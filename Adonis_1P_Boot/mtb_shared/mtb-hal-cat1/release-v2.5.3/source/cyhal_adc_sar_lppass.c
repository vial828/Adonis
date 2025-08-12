/***************************************************************************/ /**
* \file cyhal_adc_sar_pass.c
*
* \brief
* Provides an implementation of SAR ADC using PASS IP.
*
********************************************************************************
* \copyright
* Copyright 2023 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/**
 * \addtogroup group_hal_impl_adc ADC (Analog Digital Converter)
 * \ingroup group_hal_impl
 * \{
 * \section cyhal_adc_sar_lppass_impl_features Features
 * TODO
 * \} group_hal_impl_adc
 */

#include <cmsis_compiler.h>

#include "cyhal_adc.h"
#include "cyhal_analog_common.h"
#include "cyhal_lppass_common.h"
#include "cyhal_clock.h"
#include "cyhal_gpio.h"
#include "cyhal_irq_impl.h"
#include "cyhal_syspm.h"
#include <string.h>

#if defined(CY_IP_MXS22LPPASS_SAR_INSTANCES)

#include "cyhal_adc_sar_lppass.h"

#if defined(__cplusplus)
extern "C"
{
#endif

const uint8_t _CYHAL_ADC_NUM_TRIGGER_OUTPUTS = 8;

static const cyhal_adc_config_t _CYHAL_ADC_DEFAULT_CONFIG =
{
    .resolution          = _CYHAL_ADC_RESOLUTION,
    .average_count       = 1,
    .average_mode_flags  = CYHAL_ADC_AVG_MODE_AVERAGE,
    .continuous_scanning = false,
    .vneg                = CYHAL_ADC_VNEG_VSSA,
    .vref                = CYHAL_ADC_REF_INTERNAL,
    .ext_vref            = NC,
    .ext_vref_mv         = 0u,
    .is_bypassed         = true,
    .bypass_pin          = NC,
};

// Note: Several of the traditional arrays are omitted here:
// - _cyhal_adc_base: none of the LPPASS APIs takes a base address pointer
// - _cyhal_adc_irq_n: There is a single shared IRQ for all of the lppass; it is
//                     managed in cyhal_lppass_common
// - _cyhal_adc_sar_get_block_from_irqn: There is only one LPPASS block

const cyhal_source_t _cyhal_adc_tr_out[] =
{
    // These triggers are shared among all lppass users. The HAL will need to chose from among them
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT0_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT1_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT2_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT3_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT4_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT5_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT6_EDGE,
    CYHAL_TRIGGER_PASS_TR_LPPASS_OUT7_EDGE,
};

/*******************************************************************************
*       Internal helper functions
*******************************************************************************/
uint8_t _cyhal_adc_sar_get_block_from_irqn(_cyhal_system_irq_t irqn)
{
    CY_UNUSED_PARAMETER(irqn);
    return 0;
}

void _cyhal_adc_sar_update_interrupt_mask(const cyhal_adc_t* obj)
{
    bool needs_eos = (obj->async_scans_remaining > 0) /* Async transfer in progress */
                    || (false == obj->conversion_complete) /* ISR needs to flag that a conversion finished */
                    || (0u != (CYHAL_ADC_EOS & obj->user_enabled_events)); /* User requested EOS event */

    uint32_t current_mask_pass = Cy_AutAnalog_GetInterruptMask(); // Interrupt mask across the all blocks in the PASS
    uint32_t current_mask_adc = current_mask_pass & _CYHAL_LPPASS_ADC_INTR_MASK; // SAR-specific portion of the interrupt mask
    uint32_t new_mask_adc;
    if(needs_eos)
    {
        // We want EOS, not done. The two are similar, but DONE will not fire if we finish a scan and
        // the adc is in continuous mode
        new_mask_adc = current_mask_adc | CY_AUTANALOG_INT_SAR0_EOS;
    }
    else
    {
        new_mask_adc = current_mask_adc & ~CY_AUTANALOG_INT_SAR0_EOS;
    }

    uint32_t new_mask_pass = (current_mask_pass & (~_CYHAL_LPPASS_ADC_INTR_MASK)) | new_mask_adc;

    Cy_AutAnalog_ClearInterrupt(new_mask_pass & (~current_mask_pass));
    Cy_AutAnalog_SetInterruptMask(new_mask_pass);
}

static cy_en_autanalog_sar_pin_hs_t _cyhal_adc_sar_get_pin_select(uint8_t pin_index)
{
    static const cy_en_autanalog_sar_pin_hs_t mux_lookup[] =
    {
        CY_AUTANALOG_SAR_PIN_GPIO0,
        CY_AUTANALOG_SAR_PIN_GPIO1,
        CY_AUTANALOG_SAR_PIN_GPIO2,
        CY_AUTANALOG_SAR_PIN_GPIO3,
        CY_AUTANALOG_SAR_PIN_GPIO4,
        CY_AUTANALOG_SAR_PIN_GPIO5,
        CY_AUTANALOG_SAR_PIN_GPIO6,
        CY_AUTANALOG_SAR_PIN_GPIO7,
    };
    CY_ASSERT(pin_index < sizeof(mux_lookup)/sizeof(mux_lookup[0]));
    return mux_lookup[pin_index];
}

static cy_en_autanalog_sar_vref_t _cyhal_adc_sar_convert_vref(cyhal_adc_vref_t vref)
{
    switch(vref)
    {
        case CYHAL_ADC_REF_INTERNAL:
            return CY_AUTANALOG_SAR_VREF_VBGR;
        case CYHAL_ADC_REF_EXTERNAL:
            return CY_AUTANALOG_SAR_VREF_EXT;
        case CYHAL_ADC_REF_VDDA:
            return CY_AUTANALOG_SAR_VREF_VDDA;
        case CYHAL_ADC_REF_VDDA_DIV_2:
            return CY_AUTANALOG_SAR_VREF_VDDA_BY_2;
        default:
            CY_ASSERT(false);
            return CY_AUTANALOG_SAR_VREF_VBGR;
    }
}

static cy_rslt_t _cyhal_adc_sar_convert_average_count(uint32_t hal_count, cy_en_autanalog_sar_acc_cnt_t* pdl_count)
{
    switch(hal_count)
    {
        case 1: /* Average count of 1 is achieved by disabling averaging for all channels */
        case 2:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT2;
            return CY_RSLT_SUCCESS;
        case 4:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT4;
            return CY_RSLT_SUCCESS;
        case 8:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT8;
            return CY_RSLT_SUCCESS;
        case 16:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT16;
            return CY_RSLT_SUCCESS;
        case 32:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT32;
            return CY_RSLT_SUCCESS;
        case 64:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT64;
            return CY_RSLT_SUCCESS;
        case 128:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT128;
            return CY_RSLT_SUCCESS;
        case 256:
            *pdl_count = CY_AUTANALOG_SAR_ACC_CNT256;
            return CY_RSLT_SUCCESS;
        default:
            return CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }
}

static uint32_t _cyhal_adc_sar_get_vref_mv(const cyhal_adc_config_t* hal_config)
{
    switch(hal_config->vref)
    {
        case CYHAL_ADC_REF_INTERNAL:
            return _CYHAL_ADC_INTERNAL_VREF_MV;
        case CYHAL_ADC_REF_EXTERNAL:
            CY_ASSERT(hal_config->ext_vref_mv > 0); // Should have been error checked already
            return hal_config->ext_vref_mv;
        case CYHAL_ADC_REF_VDDA_DIV_2:
            return cyhal_syspm_get_supply_voltage(CYHAL_VOLTAGE_SUPPLY_VDDA) / 2;
        default:
            CY_ASSERT(CYHAL_ADC_REF_VDDA == hal_config->vref);
            return cyhal_syspm_get_supply_voltage(CYHAL_VOLTAGE_SUPPLY_VDDA);
    }
}

// Not currently used but may be needed later. Disabling temporarily to build without warnings.
#if 0
static uint8_t _cyhal_adc_sar_first_enabled(const cyhal_adc_t* obj) /* Or first channel, if no channel is enabled */
{
    uint8_t first_enabled = 0u;
    for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
    {
        if(NULL != obj->channel_config[i] && (0u != obj->chan_seq_config[i].chanEn))
        {
            first_enabled = i;
            break;
        }
    }

    return first_enabled;
}
#endif

static uint8_t _cyhal_adc_sar_last_enabled(const cyhal_adc_t* obj) /* Or last channel, if no channel is enabled */
{
    uint8_t last_enabled = _CYHAL_SAR_MAX_NUM_CHANNELS - 1u;
    /* NOT uint, or the loop will never terminate */
    for(int i = _CYHAL_SAR_MAX_NUM_CHANNELS - 1; i >= 0; --i)
    {
        // One channel per sequencer state, so chanEn is either (1u << channelIdx)) or 0.
        if(NULL != obj->channel_config[i] && (0u != obj->chan_seq_config[i].chanEn))
        {
            last_enabled = i;
            break;
        }
    }

    return last_enabled;
}

static cy_rslt_t _cyhal_adc_sar_reload_channel_config(cyhal_adc_t *obj)
{
    uint8_t num_channels = _cyhal_adc_sar_last_enabled(obj) + 1;
    // The array's length is actually MAX_CHANNELS, but there's no point in loading sequencer configs
    // that will never be used because they correspond to channels that have not been enabled.
    cy_rslt_t result = Cy_AutAnalog_SAR_LoadHSseqTable(obj->resource.block_num, num_channels, obj->chan_seq_config);
    return result;
}

static cy_rslt_t _cyhal_adc_sar_reload_config(cyhal_adc_t *obj)
{
    // No need to reload top_adc_config; none of the values in it can change after the ADC is initialized
    cy_rslt_t result = Cy_AutAnalog_SAR_LoadStaticConfig(obj->resource.block_num, &(obj->static_config));
    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_adc_sar_reload_channel_config(obj);
    }

    return result;
}

static void _cyhal_adc_update_seq_actions(cyhal_adc_t* obj)
{
    uint8_t last_channel = _cyhal_adc_sar_last_enabled(obj);
    // We deliberately skip the last channel in this loop, becaue it needs special handling
    // depending on whether we are in continuous scanning config or not
    for(int i = 0; i < last_channel; ++i)
    {
        obj->chan_seq_config[i].nextAction = CY_AUTANALOG_SAR_NEXT_ACTION_GO_TO_NEXT;
    }

    obj->chan_seq_config[last_channel].nextAction = obj->continuous_scanning
        ? CY_AUTANALOG_SAR_NEXT_ACTION_GO_TO_ENTRY_ADDR : CY_AUTANALOG_SAR_NEXT_ACTION_STATE_STOP;
}

/* Populates the PDL config struct with settings from the ADC config struct */
static void _cyhal_adc_sar_populate_default_config(cyhal_adc_t* adc)
{
    adc->top_adc_config = (cy_stc_autanalog_sar_t)
    {
        .sarStaCfg = &(adc->static_config),
        .hsSeqTabNum = 0, // Sequencer tables will be loaded as channels are initialized
        .hsSeqTabArr = adc->chan_seq_config,
        .lpSeqTabNum = 0u,
        .lpSeqTabArr = NULL,
        .firNum = 0u,
        .firCfg = NULL,
        .fifoCfg = NULL,
    };

    adc->static_config = (cy_stc_autanalog_sar_sta_t)
    {
        .lpStaCfg = &(adc->lp_config),
        .hsStaCfg = &(adc->hs_config),
        .posBufPwr = CY_AUTANALOG_SAR_BUF_PWR_OFF, // Buffered inputs not exposed through the HAL
        .negBufPwr = CY_AUTANALOG_SAR_BUF_PWR_OFF,
        // Note: This setting chooses  "accumulate and dump" vs. "interleaved" for channels where averaging is enabled.
        // The selection for "accumulate" vs. "accumlate and divide" is tracked by adc->average_is_accumulate
        // and is applied in the hardware on a per-channel basis
        .accMode = CY_AUTANALOG_SAR_ACC_DISABLED,
        .startupCal = CY_AUTANALOG_SAR_CAL_DISABLED,
        .chanID = false, // We don't use the FIFO features
        .shiftMode = false, // When accShift is set for a channel, shift back down to 12 bits
        .intMuxChan = { NULL }, // We don't expose mux channels
        .limitCond = { NULL }, // We don't expose the range detection
        .muxResultMask = 0u, // We don't expose mux channels
        .firResultMask = 0u, // We don't expose mux channels
    };

    adc->lp_config = (cy_stc_autanalog_sar_sta_lp_t)
    {
        .lpVref       = CY_AUTANALOG_SAR_VREF_VDDA,
        .lpDiffEn     = false,
        .lpSampleTime = {0U, 0U, 0U, 0U},
    };

    adc->hs_config = (cy_stc_autanalog_sar_sta_hs_t)
    {
        .hsVref = CY_AUTANALOG_SAR_VREF_VDDA, // Will be updated during ADC configuration
        .hsSampleTime = { 0u }, // Will be updated during channel configuration
        .hsGpioChan = { 0u }, // Will be updated during channel initialization
        .hsGpioResultMask = 0u, // Will be updated durign channel intialization
    };
}
static cy_rslt_t _cyhal_adc_sar_populate_global_config(const cyhal_adc_config_t* hal_config, cyhal_adc_t* adc)
{
    if((hal_config->resolution != _CYHAL_ADC_RESOLUTION) /* SAR does not support configurable resolution */
        || (false == hal_config->is_bypassed) /* No option to disable the bypass pad */ // TODO note this in documentation
        || (CYHAL_ADC_VNEG_VSSA != hal_config->vneg)) /* VSSA is hard-wired to VNEG */
    {
        return CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    adc->hs_config.hsVref = _cyhal_adc_sar_convert_vref(hal_config->vref);
    adc->continuous_scanning = hal_config->continuous_scanning;
    adc->vref_mv = _cyhal_adc_sar_get_vref_mv(hal_config);
    adc->average_is_accumulate = (0u != (hal_config->average_mode_flags & CYHAL_ADC_AVG_MODE_ACCUMULATE));

    cy_en_autanalog_sar_acc_cnt_t pdl_avg;
    cy_rslt_t result = _cyhal_adc_sar_convert_average_count(hal_config->average_count, &pdl_avg);
    if(CY_RSLT_SUCCESS == result)
    {
        adc->average_count = pdl_avg;
    }
    return result;
}

int32_t _cyhal_adc_sar_counts_to_uvolts(cyhal_adc_t* obj, uint8_t channel, int32_t counts)
{
    // The "sarSequencer" argument refers to the index in the sequencer table. In our case,
    // that matches the channel index
    return Cy_AutAnalog_SAR_CountsTo_uVolts(obj->resource.block_num, false, channel, CY_AUTANALOG_SAR_INPUT_GPIO, obj->channel_config[channel]->pdl_cfg.posPin, obj->vref_mv, counts);
}

/*******************************************************************************
*       SAR ADC functions
*******************************************************************************/

cy_rslt_t _cyhal_adc_sar_populate_acquisition_timers(cyhal_adc_t* obj)
{
    const uint32_t ACQUISITION_CLOCKS_MIN = 1;
    const uint32_t ACQUISITION_CLOCKS_MAX = 1024;

    cy_rslt_t result = CY_RSLT_SUCCESS;

    uint32_t clock_frequency_hz = cyhal_clock_get_frequency(_cyhal_lppass_hf_clock);
    uint32_t clock_period_ns = (clock_frequency_hz > 0)
        ? (_CYHAL_UTILS_NS_PER_SECOND / clock_frequency_hz)
        : 0;
    uint16_t sample_timer_ns[] = { 0u, 0u, 0u, 0u };
    uint8_t assigned_timer[_CYHAL_SAR_MAX_NUM_CHANNELS];
    if (clock_period_ns == 0)
    {
        result = CYHAL_ADC_RSLT_FAILED_CLOCK;
    }
    if (result == CY_RSLT_SUCCESS)
    {
        for(uint8_t channel = 0; channel < _CYHAL_SAR_MAX_NUM_CHANNELS; ++channel)
        {
            cyhal_adc_channel_t* chan_config = obj->channel_config[channel];
            assigned_timer[channel] = 0u;
            /* If the channel isn't in use, what we select doesn't matter */
            if(NULL != chan_config)
            {
                bool found = false;
                for(uint8_t timer = 0; !found && timer < sizeof(sample_timer_ns) / sizeof(sample_timer_ns[0]); ++timer)
                {
                    if(chan_config->minimum_acquisition_ns == sample_timer_ns[timer])
                    {
                        /* Matched a pre-existing timer; use that */
                        assigned_timer[channel] = timer;
                        found = true;
                    }
                    else if(0 == sample_timer_ns[timer])
                    {
                        /* Found a free timer - allocate and use that */
                        sample_timer_ns[timer] = chan_config->minimum_acquisition_ns;
                        assigned_timer[channel] = timer;
                        found = true;
                    }
                }

                if(false == found)
                {
                    /* Ran out of acquisition timers */
                    result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
                }
            }
        }
    }

    if(CY_RSLT_SUCCESS == result)
    {
        for(uint8_t i = 0; i < sizeof(obj->hs_config.hsSampleTime) / sizeof(obj->hs_config.hsSampleTime[0]); ++i)
        {
            /* Convert from nanoseconds to clock cycles, rounding up */
            uint32_t clock_cycles = (sample_timer_ns[i] + (clock_period_ns - 1)) / clock_period_ns;
            if(clock_cycles < ACQUISITION_CLOCKS_MIN)
            {
                clock_cycles = ACQUISITION_CLOCKS_MIN;
            }
            else if(clock_cycles > ACQUISITION_CLOCKS_MAX)
            {
                clock_cycles = ACQUISITION_CLOCKS_MAX;
            }
            /* Per the register map, this should be one less than the actual desired sampling cycle count */
            obj->hs_config.hsSampleTime[i] = clock_cycles - 1;
        }

        for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
        {
            obj->chan_seq_config[i].sampleTime = (cy_en_autanalog_sar_sample_time_t)assigned_timer[i];
        }
    }

    return result;
}

static uint16_t _cyhal_adc_sar_get_average_count(cyhal_adc_t* obj, uint8_t channel_idx)
{
    if (false == obj->chan_seq_config[channel_idx].accEn)
    {
        return 1u;
    }
    else
    {
        switch(obj->chan_seq_config[channel_idx].accCount)
        {

            case CY_AUTANALOG_SAR_ACC_CNT2:
                return 2;
            case CY_AUTANALOG_SAR_ACC_CNT4:
                return 4;
            case CY_AUTANALOG_SAR_ACC_CNT8:
                return 8;
            case CY_AUTANALOG_SAR_ACC_CNT16:
                return 16;
            case CY_AUTANALOG_SAR_ACC_CNT32:
                return 32;
            case CY_AUTANALOG_SAR_ACC_CNT64:
                return 64;
            case CY_AUTANALOG_SAR_ACC_CNT128:
                return 128;
            case CY_AUTANALOG_SAR_ACC_CNT256:
                return 256;
            default:
                CY_ASSERT(false); // Should never get here
                return 1u;
        }
    }
    CY_ASSERT(false); // Should never get here
    return 1u;
}

uint32_t _cyhal_adc_sar_compute_actual_sample_rate(cyhal_adc_t* obj)
{
    /* Assumes that the acquisition timers and clock frequency are already set */
    uint32_t clock_frequency_hz = cyhal_clock_get_frequency(_cyhal_lppass_hf_clock);
    uint32_t clock_period_ns = (clock_frequency_hz > 0)
        ? (_CYHAL_UTILS_NS_PER_SECOND / clock_frequency_hz)
        : 0;
    CY_ASSERT(clock_period_ns > 0);

    uint32_t total_sample_time_ns = 0;

    for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
    {
        cy_stc_autanalog_sar_seq_tab_hs_t* channelSequencerEntry = &(obj->chan_seq_config[i]);
        // One channel per sequencer state, so chanEn is eiher (1u << gpioNum) or 0.
        if (0u == channelSequencerEntry->chanEn)
        {
            continue;
        }

        uint8_t sample_time_idx = (uint8_t)(channelSequencerEntry->sampleTime);
        /* Per the pdl docuentation, the struct value is one less than the actual cycle number. */
        uint32_t sample_clock_cycles = obj->hs_config.hsSampleTime[sample_time_idx] + 1;
        uint32_t total_cycles = sample_clock_cycles + _CYHAL_ADC_CONVERSION_CYCLES;
        uint32_t sample_time_ns = total_cycles * clock_period_ns;
        sample_time_ns *= _cyhal_adc_sar_get_average_count(obj, i);
        total_sample_time_ns += sample_time_ns;
    }
    uint32_t sample_frequency_hz = (total_sample_time_ns > 0)
        ? (_CYHAL_UTILS_NS_PER_SECOND / total_sample_time_ns)
        : 0;
    return sample_frequency_hz;
}

void _cyhal_adc_sar_start_convert(cyhal_adc_t* obj)
{
    CY_UNUSED_PARAMETER(obj);
    Cy_AutAnalog_FwTrigger(CY_AUTANALOG_FW_TRIGGER0);
}

void _cyhal_adc_sar_stop_convert(cyhal_adc_t* obj)
{
    // No direct way to stop conversion. Best we can do is set every state to "stop after this state",
    // wait for it to finish, then revert them to their original values.
    uint8_t last_channel = _cyhal_adc_sar_last_enabled(obj);
    for(int i = 0; i <= last_channel; ++i)
    {
        obj->chan_seq_config[i].nextAction = CY_AUTANALOG_SAR_NEXT_ACTION_STATE_STOP;
    }

    _cyhal_adc_sar_reload_channel_config(obj);

    while(Cy_AutAnalog_SAR_IsBusy(obj->resource.block_num)) { }

    _cyhal_adc_update_seq_actions(obj);
    _cyhal_adc_sar_reload_channel_config(obj);
}

void _cyhal_adc_sar_clear_interrupt(cyhal_adc_t *obj)
{
    CY_UNUSED_PARAMETER(obj);
    Cy_AutAnalog_ClearInterrupt(_CYHAL_LPPASS_ADC_INTR_MASK);
}

void _cyhal_adc_sar_update_interrupt_priority(cyhal_adc_t *obj, uint8_t intr_priority)
{
    CY_UNUSED_PARAMETER(obj);
    _cyhal_lppass_set_irq_priority(intr_priority);
}

cy_rslt_t _cyhal_adc_sar_init(cyhal_adc_t *obj, cyhal_gpio_t pin, const cyhal_clock_t *clk)
{
    // Unlike other devices, the PDL config struct is stored persistently on the HAL object struct.
    // This call populates settings that are global in the hardware into the PDL config struct,
    // and populates settings that need to be applied per-channel into the ADC object struct
    _cyhal_adc_sar_populate_default_config(obj);
    cy_rslt_t result =  _cyhal_adc_sar_populate_global_config(&_CYHAL_ADC_DEFAULT_CONFIG, obj);

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_adc_configurator_t config;
        config.resource = NULL;
        config.config = &(obj->top_adc_config);
        config.clock = clk;
        config.num_channels = 0u;

        result = _cyhal_adc_config_hw(obj, &config, pin, false);
    }

    return result;
}

cy_rslt_t _cyhal_adc_sar_init_hw(cyhal_adc_t *obj, const cyhal_adc_configurator_t* cfg)
{
    cy_rslt_t result = _cyhal_analog_init();
    if(CY_RSLT_SUCCESS == result)
    {
        obj->analog_init_completed = true;
        result = Cy_AutAnalog_SAR_LoadConfig(obj->resource.block_num, cfg->config);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        _cyhal_adc_sar_update_interrupt_mask(obj);
        _cyhal_lppass_register_irq_handler(_CYHAL_LPPASS_IRQ_ADC, _cyhal_adc_irq_handler);
    }
    
    return result;
}

void _cyhal_adc_sar_deinit(cyhal_adc_t *obj)
{
    _cyhal_lppass_register_irq_handler(_CYHAL_LPPASS_IRQ_ADC, NULL);
    Cy_AutAnalog_SetInterruptMask(Cy_AutAnalog_GetInterruptMask() & (~_CYHAL_LPPASS_ADC_INTR_MASK));
    _cyhal_adc_sar_disable(obj);
    if(obj->analog_init_completed == true)
    {
        _cyhal_analog_free();
    }
}

void _cyhal_adc_sar_enable(cyhal_adc_t *obj)
{
    CY_UNUSED_PARAMETER(obj);
    cy_stc_autanalog_stt_t* adc_stt = _cyhal_lppass_get_stt_entry(_CYHAL_LPPASS_STT_START);
    if (!adc_stt->sar[obj->resource.block_num]->enable)
    {
        adc_stt->sar[obj->resource.block_num]->enable = true;
        _cyhal_lppass_apply_state(_CYHAL_LPPASS_STT_START);
    }
}

void _cyhal_adc_sar_disable(cyhal_adc_t *obj)
{
    CY_UNUSED_PARAMETER(obj);
    // No direct way to stop conversion. Best we can do is set every state to "stop after this state".
    uint8_t last_channel = _cyhal_adc_sar_last_enabled(obj);
    for(int i = 0; i <= last_channel; ++i)
    {
        obj->chan_seq_config[i].nextAction = CY_AUTANALOG_SAR_NEXT_ACTION_STATE_STOP;
    }
}

// Transfer adc-wide configuration settings to the individual channels where they are stored on this hardware
void _cyhal_adc_sar_channel_apply_global_config(cyhal_adc_channel_t *obj)
{
    cy_stc_autanalog_sar_seq_tab_hs_t* seq_entry = &obj->adc->chan_seq_config[obj->channel_idx];
    obj->pdl_cfg.accShift = seq_entry->accEn && obj->adc->average_is_accumulate;
    seq_entry->accCount = obj->adc->average_count;
}

// For situations where the global HAL-level ADC config has changed, because some of the
// settings that are global from the HAL perspective
// Note: This only updates the channel config structs, it does not reload the cofiguration
// in the hardware (to avoid duplicated work in cases where the caller needs to do additional
// work before applying the config)
void _cyhal_adc_sar_configure_all_channels(cyhal_adc_t *obj)
{
    for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
    {
        if(obj->channel_config[i] != NULL)
        {
            _cyhal_adc_sar_channel_apply_global_config(obj->channel_config[i]);
        }
    }
}

cy_rslt_t _cyhal_adc_sar_configure(cyhal_adc_t *obj, const cyhal_adc_config_t *config)
{
    cy_rslt_t result = _cyhal_adc_sar_populate_global_config(config, obj);
    if(CY_RSLT_SUCCESS == result)
    {
        _cyhal_adc_update_seq_actions(obj);
        _cyhal_adc_sar_configure_all_channels(obj);
        result = _cyhal_adc_sar_reload_config(obj);
    }

    return result;
}


/*******************************************************************************
*       ADC Channel HAL Functions
*******************************************************************************/

cy_rslt_t _cyhal_adc_sar_channel_init(cyhal_adc_channel_t *obj, cyhal_adc_t* adc, cyhal_gpio_t vplus, cyhal_gpio_t vminus, const cyhal_adc_channel_config_t* cfg)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    const cyhal_resource_pin_mapping_t *vplus_map = _cyhal_utils_get_resource(vplus, cyhal_pin_map_pass_sarmux_pads,
        sizeof(cyhal_pin_map_pass_sarmux_pads)/sizeof(cyhal_pin_map_pass_sarmux_pads[0]), NULL, false);
    const cyhal_resource_pin_mapping_t *vminus_map = (NC == vminus) ? NULL :
        _cyhal_utils_get_resource(vminus, cyhal_pin_map_pass_sarmux_pads,
        sizeof(cyhal_pin_map_pass_sarmux_pads)/sizeof(cyhal_pin_map_pass_sarmux_pads[0]), NULL, false);
    
    if(NULL == vplus_map || (NC != vminus && NULL == vminus_map))
    {
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->pdl_cfg = (cy_stc_autanalog_sar_hs_chan_t)
        {
            // The sarmux input index is stored in the channel_num field for ADC input pins
            .posPin = _cyhal_adc_sar_get_pin_select(vplus_map->channel_num),
            .hsDiffEn = (NC != vminus),
            .sign = true, // ADC HAL is signed by default
            .posCoeff = CY_AUTANALOG_SAR_CH_COEFF_DISABLED, // Not exposed in the HAL
            .negPin = (NC == vminus) ? CY_AUTANALOG_SAR_PIN_GPIO0 /* placeholder value */ : _cyhal_adc_sar_get_pin_select(vminus_map->channel_num),
            // accShift will be updated during _cyhal_adc_update_channel_config
            .negCoeff = CY_AUTANALOG_SAR_CH_COEFF_DISABLED, // Not exposed in the HAL
            .hsLimit = CY_AUTANALOG_SAR_LIMIT_STATUS_DISABLED, // Not exposed in the HAL
            .fifoSel = CY_AUTANALOG_FIFO_DISABLED // Not exposed in the HAL
        };

        adc->chan_seq_config[obj->channel_idx] = (cy_stc_autanalog_sar_seq_tab_hs_t)
        {
            // chanEn will be updated by _cyhal_adc_update_channel_config
            .muxMode = CY_AUTANALOG_SAR_CHAN_MASK_GPIO_DISABLED, // We don't use muxed inputs, only dedicated GPIOs
            .mux0Sel = 0u,
            .mux1Sel = 0u,
            .sampleTimeEn = 1u,
            .sampleTime = CY_AUTANALOG_SAR_SAMPLE_TIME0, // Will be updated by_cyhal_adc_sar_populate_acquisition_timers
            // accShift and average_count will be updated during _cyhal_adc_update_channel_config
            .calReq = CY_AUTANALOG_SAR_CAL_DISABLED,
            .nextAction = CY_AUTANALOG_SAR_NEXT_ACTION_STATE_STOP, // Will be updated in _cyhal_adc_sar_update_seq_actions
        };

        adc->hs_config.hsGpioChan[obj->channel_idx] = &(obj->pdl_cfg);
    }

    // Will take care of updating the configuration in the ADC
    result = cyhal_adc_channel_configure(obj, cfg);

    return result;
}

cy_rslt_t _cyhal_adc_sar_channel_config(cyhal_adc_channel_t *obj, const cyhal_adc_channel_config_t *config)
{
    cy_stc_autanalog_sar_seq_tab_hs_t* seq_entry = &obj->adc->chan_seq_config[obj->channel_idx];
    seq_entry->chanEn = config->enabled ? (1u << obj->channel_idx) : 0u;
    if (config->enabled)
    {
        obj->adc->hs_config.hsGpioResultMask |= (1u << obj->channel_idx);
    }
    else
    {
        obj->adc->hs_config.hsGpioResultMask &= ~((1u << obj->channel_idx));
    }
    seq_entry->accEn = config->enable_averaging;
    seq_entry->accCount = obj->adc->average_count;

    _cyhal_adc_sar_channel_apply_global_config(obj);
    cy_rslt_t result = _cyhal_adc_sar_populate_acquisition_timers(obj->adc);

    if(CY_RSLT_SUCCESS == result)
    {
        _cyhal_adc_update_seq_actions(obj->adc);
        // Not just reload channel config; things like the pin selection for a channel are included in
        // the static config that has to be reloaded as a single chunk.
        result = _cyhal_adc_sar_reload_config(obj->adc);
    }
    return result;
}

void _cyhal_adc_sar_channel_deinit(cyhal_adc_channel_t *obj)
{
    obj->adc->hs_config.hsGpioChan[obj->channel_idx] = NULL;
    obj->adc->chan_seq_config[obj->channel_idx].chanEn = 0u;
    _cyhal_adc_update_seq_actions(obj->adc);
    _cyhal_adc_sar_reload_channel_config(obj->adc);
}

uint16_t _cyhal_adc_sar_counts_to_u16(const cyhal_adc_channel_t *obj, int32_t signed_result)
{
    CY_UNUSED_PARAMETER(obj);
    const uint8_t RESULT_SCALING_FACTOR = UINT16_MAX / 0xFFF; // constant 12-bit SAR resolution

    /* An unsigned value will range from 0 to vref * 2, rather than -vref to vref. Offset accordingly */
    uint16_t unsigned_result = (uint16_t)(signed_result + 0x800); 
    /* The SAR provides a 12-bit result, but this API is defined to fill a full 16-bit range */
    uint16_t scaled_result = ((uint16_t)unsigned_result) * RESULT_SCALING_FACTOR;
    return scaled_result;
}

int32_t _cyhal_adc_sar_get_result(cyhal_adc_t* obj, uint8_t channel)
{
    return Cy_AutAnalog_SAR_ReadResult(obj->resource.block_num, CY_AUTANALOG_SAR_INPUT_GPIO, channel);
}

int32_t _cyhal_adc_sar_read(const cyhal_adc_channel_t *obj)
{
    uint32_t old_en_values[_CYHAL_SAR_MAX_NUM_CHANNELS] = { 0u };

    bool needs_reconfig = false;
    if(!obj->adc->continuous_scanning)
    {
        /* Enable the selected channel only, the set the ADC to run that conversion */
        for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
        {
            // One channel per sequencer state, so chanEn is either (1u << chanIdx) or 0.
            cyhal_adc_channel_t* current_chan = obj->adc->channel_config[i];
            if(NULL != current_chan)
            {
                old_en_values[i] = obj->adc->chan_seq_config[i].chanEn;
                obj->adc->chan_seq_config[i].chanEn = (obj->channel_idx == i) ? (1u << i) : 0u;
                needs_reconfig |= (old_en_values[i] != obj->adc->chan_seq_config[i].chanEn); // Only need to reconfigure the SAR if something changed.
            }

        }

        if(needs_reconfig)
        {
            _cyhal_adc_update_seq_actions(obj->adc);
            _cyhal_adc_sar_reload_channel_config(obj->adc);
        }
        obj->adc->conversion_complete = false; // Needs to be before update_interrupt_mask
        _cyhal_adc_sar_update_interrupt_mask(obj->adc);
        _cyhal_adc_sar_start_convert(obj->adc);
    }

    // There is a function Cy_AutAnalog_SAR_IsBusy, but it always returns true when continuous scanning is enabled,
    // so it cannot be used in all cases to determine if at least one scan has completed. That means we need to
    // maintain this state anyway, so use it consistently.
    while(!obj->adc->conversion_complete) { }

    int32_t result = _cyhal_adc_sar_get_result(obj->adc, obj->channel_idx);

    if(!obj->adc->continuous_scanning)
    {
        if(needs_reconfig)
        {
            /* Restore the original channel enabled state */
            for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
            {
                // One channel per sequencer state, so chanEn is either (1u << chanIdx) or 0.
                cyhal_adc_channel_t* current_chan = obj->adc->channel_config[i];
                if(NULL != current_chan)
                {
                    obj->adc->chan_seq_config[i].chanEn = old_en_values[i];
                }
            }

            _cyhal_adc_update_seq_actions(obj->adc);
            _cyhal_adc_sar_reload_channel_config(obj->adc);
        }
        _cyhal_adc_sar_update_interrupt_mask(obj->adc);
    }

    return result;
}


/*******************************************************************************
*       Interconnect functions
*******************************************************************************/

void _cyhal_adc_sar_connect_digital(cyhal_adc_t *obj, cyhal_source_t source, cyhal_adc_input_t input)
{
    // There are four trigger inputs to the LPPASS, but there is no straightforward way to
    // route one of them to trigger the ADC. The only thing that the triggers can be used for
    // is as a wait condition to determine when the autonomous controller transitions from one
    // state to another. The ADC HAL could in theory set up such a scheme in a vacuum, but it would
    // interfere with usage of the AC by other HAL drivers which need to be able to set it to
    // arbitrary states in order to control the power state of other blocks.
    // TODO
    CY_UNUSED_PARAMETER(source);
    CY_UNUSED_PARAMETER(input);
    CY_UNUSED_PARAMETER(obj);
}

cy_rslt_t _cyhal_adc_sar_enable_output(cyhal_adc_t *obj, cyhal_adc_output_t output, cyhal_source_t *source)
{
    CY_UNUSED_PARAMETER(output);
    CY_UNUSED_PARAMETER(obj);
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint8_t foundEos = 0xFF;
    uint8_t foundEmpty = 0xFF;
    for(uint8_t i = 0; i < _CYHAL_ADC_NUM_TRIGGER_OUTPUTS; ++i)
    {
        // We support one output: EOS. So we're looking for either a trigger that already
	    // is connected to EOS, or one that is currently connected to nothing.
        cy_en_autanalog_ac_out_trigger_mask_t currentTrigger = 
            Cy_AutAnalog_GetOutputTriggerMask((cy_en_autanalog_ac_out_trigger_idx_t) i);
        if (CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY == currentTrigger)
        {
            foundEmpty = i;
        }
        else if (CY_AUTANALOG_AC_OUT_TRIG_MASK_SAR_EOS == currentTrigger)
        {
            foundEos = i;
        }

        if ((0xFF != foundEos) && (0xFF != foundEmpty))
	    {
            break;
        }
    }
    
    if (foundEos)
    {
        // Use already configured trigger
        *source = _cyhal_adc_tr_out[foundEos];
    }
    else if(foundEmpty)
    {
        // Map this trigger to the ADC, then use it
        Cy_AutAnalog_SetOutputTriggerMask((cy_en_autanalog_ac_out_trigger_idx_t)foundEmpty, CY_AUTANALOG_AC_OUT_TRIG_MASK_SAR_EOS);
        *source = _cyhal_adc_tr_out[foundEmpty];
    }
    else
    {
        result = CYHAL_ADC_RSLT_NO_CHANNELS;
    }
    return result;
}

void _cyhal_adc_sar_disconnect_digital(cyhal_adc_t *obj, cyhal_source_t source, cyhal_adc_input_t input)
{
    CY_UNUSED_PARAMETER(source);
    CY_UNUSED_PARAMETER(input);
    CY_UNUSED_PARAMETER(obj);
}

void _cyhal_adc_sar_disable_output(cyhal_adc_t *obj, cyhal_adc_output_t output)
{
    CY_UNUSED_PARAMETER(output);
    CY_UNUSED_PARAMETER(obj);

    // Find and disable any output triggers that are mapped to eos
    for(uint8_t i = 0; i < _CYHAL_ADC_NUM_TRIGGER_OUTPUTS; ++i)
    {
        cy_en_autanalog_ac_out_trigger_mask_t currentTrigger = 
            Cy_AutAnalog_GetOutputTriggerMask((cy_en_autanalog_ac_out_trigger_idx_t) i);
        if (CY_AUTANALOG_AC_OUT_TRIG_MASK_SAR_EOS == currentTrigger)
        {
            Cy_AutAnalog_SetOutputTriggerMask((cy_en_autanalog_ac_out_trigger_idx_t)i, CY_AUTANALOG_AC_OUT_TRIG_MASK_EMPTY);
        }
    }
}


#if defined(__cplusplus)
}
#endif

#endif /* defined(CY_IP_MXS22LPPASS_SAR_INSTANCES) */
