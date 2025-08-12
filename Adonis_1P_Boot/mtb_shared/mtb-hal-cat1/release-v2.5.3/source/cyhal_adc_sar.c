/***************************************************************************/ /**
* \file cyhal_adc_sar.c
*
* \brief
* Provides a high level interface for interacting with the Cypress Analog/Digital
* converter. This interface abstracts out the chip specific details. If any chip
* specific functionality is necessary, or performance is critical the low level
* functions can be used directly.
*
********************************************************************************
* \copyright
* Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company) or
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

#include <cmsis_compiler.h>

#include "cyhal_adc.h"
#include "cyhal_clock.h"
#include "cyhal_dma.h"
#include "cyhal_gpio.h"
#include "cyhal_hwmgr.h"
#include "cyhal_utils.h"
#include "cyhal_irq_impl.h"
#include "cyhal_interconnect.h"
#include "cyhal_system.h"
#include <string.h>

#if defined(CY_IP_MXS22LPPASS_SAR_INSTANCES)
#include "cyhal_adc_sar_lppass.h"
#else
#include "cyhal_adc_sar_pass.h"
#endif

#if (_CYHAL_DRIVER_AVAILABLE_ADC_SAR)

#if defined(__cplusplus)
extern "C"
{
#endif

/* Defined in IP specific implementation */
#if !defined(_CYHAL_ADC_SAR_SKIP_BASE)
extern _CYHAL_ADC_SAR_Type *const _cyhal_adc_base[];
#endif
extern const en_clk_dst_t _cyhal_adc_clock[];
extern const cyhal_source_t _cyhal_adc_tr_out[];
extern const cyhal_dest_t _cyhal_adc_tr_in[];

cyhal_adc_t* _cyhal_adc_config_structs[_CYHAL_ADC_SAR_INSTANCES];


/*******************************************************************************
*       Internal helper functions
*******************************************************************************/

static uint8_t _cyhal_adc_max_configured_channel(const cyhal_adc_t* obj)
{
    uint8_t max = 0;
    for(uint8_t i = 0; i < _CYHAL_SAR_MAX_NUM_CHANNELS; ++i)
    {
        if(NULL != obj->channel_config[i])
        {
            max = i;
        }
    }
    return max;
}

void _cyhal_adc_irq_handler(void)
{
    /* The only enabled event is scan finished */
    cyhal_adc_event_t hal_event = CYHAL_ADC_EOS;

    _cyhal_system_irq_t irqn = _cyhal_irq_get_active();
    uint8_t block = _cyhal_adc_sar_get_block_from_irqn(irqn);
    cyhal_adc_t* obj = _cyhal_adc_config_structs[block];
    _cyhal_adc_sar_clear_interrupt(obj);
    obj->conversion_complete = true;
#if defined (_CYHAL_ADC_STOP_AFTER_SCAN)
    if(obj->stop_after_scan)
    {
        _cyhal_adc_sar_stop_convert(obj);
    }
#endif
    if(obj->async_scans_remaining > 0)
    {
        uint8_t num_channels = _cyhal_adc_max_configured_channel(obj) + 1;
        /* Can't read millivolts out via DMA */
        if(CYHAL_ASYNC_SW == obj->async_mode || obj->async_transfer_in_uv)
        {
            for(uint8_t i = 0; i < num_channels; ++i)
            {
                int32_t counts = 0;
                #if defined(_CYHAL_ADC_VBG_CHANNEL_IDX)
                if (_CYHAL_ADC_VBG_CHANNEL_IDX == i)
                {
                    obj->vbg_last_value = _cyhal_adc_sar_get_result(obj, _CYHAL_ADC_VBG_CHANNEL_IDX);
                    continue;
                }
                else
                {
                    counts = _cyhal_adc_sar_get_result(obj, i);
                }
                #else
                counts = _cyhal_adc_sar_get_result(obj, i);
                #endif
                *obj->async_buff_next = obj->async_transfer_in_uv ? _cyhal_adc_sar_counts_to_uvolts(obj, i, counts) : counts;
                ++obj->async_buff_next;
            }
            --(obj->async_scans_remaining);
            if(0 == obj->async_scans_remaining)
            {
                obj->async_buff_next = NULL;
                obj->async_buff_orig = NULL;
                hal_event |= CYHAL_ADC_ASYNC_READ_COMPLETE;
            }
            else if(false == obj->continuous_scanning)
            {
                _cyhal_adc_sar_start_convert(obj);
            }
            /* If we're continously scanning, another scan will be kicked off automatically
             * so we don't need to do anything */
        }
        else
        {
            CY_ASSERT(CYHAL_ASYNC_DMA == obj->async_mode);

#if _CYHAL_ADC_DMA_SUPPORTED
            cyhal_dma_cfg_t dma_config =
            {
                .src_addr = (uint32_t)_CYHAL_ADC_SAR_DMA_SRC(obj),
                .src_increment = 1u,
                .dst_addr = (uint32_t)obj->async_buff_next,
                .dst_increment = 1u,
                .transfer_width = 32u,
                .length = num_channels,
                .burst_size = 0u,
                .action = CYHAL_DMA_TRANSFER_FULL
            };

            // Configure needs to happen after we've manipulated the descriptor config
            cy_rslt_t result = cyhal_dma_configure(&(obj->dma), &dma_config);
            if(CY_RSLT_SUCCESS == result)
            {
                result = cyhal_dma_enable(&(obj->dma));
            }
            if(CY_RSLT_SUCCESS == result)
            {
                result = cyhal_dma_start_transfer(&(obj->dma));
            }
            CY_ASSERT(CY_RSLT_SUCCESS == result);

            /* Don't increment the buffer here - do that when the DMA completes */
            if(false == obj->continuous_scanning)
            {
                _cyhal_adc_sar_start_convert(obj);
            }
#else
            CY_ASSERT(false); // DMA not supported on the current device
#endif //_CYHAL_ADC_DMA_SUPPORTED
        }
    }

    _cyhal_adc_sar_update_interrupt_mask(obj);

    if(0 != (hal_event & ((cyhal_adc_event_t)obj->user_enabled_events)))
    {
        cyhal_adc_event_callback_t callback = (cyhal_adc_event_callback_t)obj->callback_data.callback;
        if(NULL != callback)
        {
            callback(obj->callback_data.callback_arg, (cyhal_adc_event_t)(hal_event & obj->user_enabled_events));
        }
    }

}

#if _CYHAL_ADC_DMA_SUPPORTED
static void _cyhal_adc_dma_handler(void* arg, cyhal_dma_event_t event)
{
    CY_ASSERT(CYHAL_DMA_TRANSFER_COMPLETE == event);
    CY_UNUSED_PARAMETER(event);
    cyhal_adc_t* obj = (cyhal_adc_t*)arg;
    CY_ASSERT(CYHAL_ASYNC_DMA == obj->async_mode);

    uint8_t num_channels = _cyhal_adc_max_configured_channel(obj) + 1;
    CY_ASSERT(false == obj->async_transfer_in_uv);
    obj->async_buff_next += num_channels;
    --(obj->async_scans_remaining);
    _cyhal_adc_sar_update_interrupt_mask(obj);

    if(0 == obj->async_scans_remaining)
    {
        // DMA doesn't sign extend when we copy from 16 to 32 bits, so do the sign extension
        // ourselves once all channel scans are complete.
        while(obj->async_buff_orig != obj->async_buff_next)
        {
            // Mask off the upper two bytes because those contain mirrored status bits which
            // are not part of the ADC counts
            int16_t sar_result = (int16_t)(0xFFFF & *(obj->async_buff_orig));
            *(obj->async_buff_orig) = sar_result;
            ++(obj->async_buff_orig);
        }
        obj->async_buff_next = NULL;
        obj->async_buff_orig = NULL;
        if(0 != (CYHAL_ADC_ASYNC_READ_COMPLETE & ((cyhal_adc_event_t)obj->user_enabled_events)))
        {
            cyhal_adc_event_callback_t callback = (cyhal_adc_event_callback_t)obj->callback_data.callback;
            if(NULL != callback)
            {
                callback(obj->callback_data.callback_arg, CYHAL_ADC_ASYNC_READ_COMPLETE);
            }
        }
    }
}
#endif


/*******************************************************************************
*       ADC HAL Functions
*******************************************************************************/

cy_rslt_t _cyhal_adc_config_hw(cyhal_adc_t *obj, const cyhal_adc_configurator_t* cfg, cyhal_gpio_t pin, bool owned_by_configurator)
{
#if !defined(_CYHAL_ADC_SAR_SKIP_PCLK)
    const uint32_t DESIRED_DIVIDER = 8000000u; // 8 MHz. Required range is 1.7 - 18
#endif

    CY_ASSERT(NULL != obj);

    cy_rslt_t result = CY_RSLT_SUCCESS;

#if !defined(_CYHAL_ADC_SAR_SKIP_PCLK)
    obj->clock.reserved = false;
#endif
    obj->resource.type = CYHAL_RSC_INVALID;
    obj->async_mode = CYHAL_ASYNC_SW;
    obj->source = CYHAL_TRIGGER_CPUSS_ZERO;

    obj->owned_by_configurator = owned_by_configurator;

    if(NULL == cfg->resource && (NC != pin))
    {
        const cyhal_resource_pin_mapping_t* map = _CYHAL_UTILS_TRY_ALLOC(pin, CYHAL_RSC_ADC, cyhal_pin_map_pass_sarmux_pads);

        if (NULL == map)
        {
            result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
        }
        else
        {
            /* No need to reserve - try_alloc did so for us already */
            obj->resource.type = CYHAL_RSC_ADC;
            obj->resource.block_num = map->block_num;
            obj->resource.channel_num = 0; // Force channel to 0, since we don't use channels and the sarmux_pads overload that item
        }
    }
    else if(NULL != cfg->resource)
    {
        obj->resource = *cfg->resource;
    }
    else
    {
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

#if !defined(_CYHAL_ADC_SAR_SKIP_PCLK)
    en_clk_dst_t pclk = (en_clk_dst_t)0;
#endif
    if (CY_RSLT_SUCCESS == result)
    {
#if !defined(_CYHAL_ADC_SAR_SKIP_BASE)
        obj->base = _cyhal_adc_base[obj->resource.block_num];
#endif
#if !defined(_CYHAL_ADC_SAR_SKIP_PCLK)
        pclk = _cyhal_adc_clock[obj->resource.block_num];
        if (NULL != cfg->clock)
        {
            obj->clock = *cfg->clock;
            obj->dedicated_clock = false;
        }
        else
        {
            result = _cyhal_utils_allocate_clock(&(obj->clock), &(obj->resource), CYHAL_CLOCK_BLOCK_PERIPHERAL_16BIT, true);
            if (CY_RSLT_SUCCESS == result)
            {
                obj->dedicated_clock = true;
            }
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        if (CY_SYSCLK_SUCCESS != _cyhal_utils_peri_pclk_assign_divider(pclk, &(obj->clock)))
            result = CYHAL_ADC_RSLT_FAILED_CLOCK;
    }

    if (CY_RSLT_SUCCESS == result)
    {
        if(obj->dedicated_clock)
        {
            uint32_t source_hz = _cyhal_utils_get_peripheral_clock_frequency(&(obj->resource));
            uint32_t div = source_hz / DESIRED_DIVIDER;
            if (0 == div ||
                (CY_SYSCLK_SUCCESS != _cyhal_utils_peri_pclk_set_divider(pclk, &(obj->clock), div - 1)) ||
                (CY_SYSCLK_SUCCESS != _cyhal_utils_peri_pclk_enable_divider(pclk, &(obj->clock))))
            {
                result = CYHAL_ADC_RSLT_FAILED_CLOCK;
            }
        }
#endif
    }

    if (result == CY_RSLT_SUCCESS)
    {
        result = _cyhal_adc_sar_init_hw(obj, cfg);
    }

    if (result == CY_RSLT_SUCCESS)
    {
        _cyhal_adc_config_structs[obj->resource.block_num] = obj;
        _cyhal_adc_sar_enable(obj);
    }
    else
    {
        cyhal_adc_free(obj);
    }
    
    return result;
}

cy_rslt_t cyhal_adc_init_cfg(cyhal_adc_t *adc, cyhal_adc_channel_t** channels, uint8_t* num_channels,
                                const cyhal_adc_configurator_t *cfg)
{
    // We can't do this at a lower level helper such as config_hw because some implementations
    // need to be able to store information on the ADC object before that function is called.
    memset(adc, 0, sizeof(cyhal_adc_t));

    cy_rslt_t result = CY_RSLT_SUCCESS;
    if(*num_channels < cfg->num_channels)
    {
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    if(CY_RSLT_SUCCESS == result)
    {
        *num_channels = cfg->num_channels;
        result = _cyhal_adc_config_hw(adc, cfg, NC, true);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        /* config_hw will have initialized the channels in the ADC HW and the configurator will
         * have set up the routing, but we need to initialize the channel structs */
        for(uint8_t i = 0; i < *num_channels; ++i)
        {
            cyhal_adc_channel_t* channel = channels[i];
            memset(channel, 0, sizeof(cyhal_adc_channel_t));
            channel->adc = adc;
            channel->channel_idx = i;
            /* Nothing in this flow needs to know what the pins are - and the inputs aren't even
             * necesssarily pins. The configurator takes care of resource reservation and routing for us */
            channel->vplus = NC;
#if !(_CYHAL_ADC_SINGLE_ENDED)
            channel->vminus = NC;
#endif
            /* The configurator takes care of initial solving, but store this so that we behave properly
             * if the user changes any of the configuration at runtime */
            channel->minimum_acquisition_ns = cfg->achieved_acquisition_time[i];
        }
    }
    return result;
}

cy_rslt_t cyhal_adc_init(cyhal_adc_t *obj, cyhal_gpio_t pin, const cyhal_clock_t *clk)
{
    // We can't do this at a lower level helper such as config_hw because some implementations
    // need to be able to store information on the ADC object before that function is called.
    memset(obj, 0, sizeof(cyhal_adc_t));
    return _cyhal_adc_sar_init(obj, pin, clk);
}

void cyhal_adc_free(cyhal_adc_t *obj)
{
    if (NULL != obj 
#if !defined(_CYHAL_ADC_SAR_SKIP_BASE)
        && NULL != obj->base
#endif
       )
    {
        _cyhal_adc_config_structs[obj->resource.block_num] = NULL;

        cy_rslt_t rslt;
        rslt = cyhal_adc_disable_output(obj, CYHAL_ADC_OUTPUT_SCAN_COMPLETE);
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);
        if (CYHAL_TRIGGER_CPUSS_ZERO != obj->source)
        {
            rslt = cyhal_adc_disconnect_digital(obj, obj->source, CYHAL_ADC_INPUT_START_SCAN);
            CY_ASSERT(CY_RSLT_SUCCESS == rslt);
        }
        (void)rslt; // Disable compiler warning in release build

        _cyhal_adc_sar_deinit(obj);

        if(false == obj->owned_by_configurator)
        {
#if !defined(_CYHAL_ADC_SAR_SKIP_PCLK)
            if(obj->dedicated_clock)
            {
                _cyhal_utils_peri_pclk_disable_divider(_cyhal_adc_clock[obj->resource.block_num], &(obj->clock));
                cyhal_clock_free(&obj->clock);
            }
#endif

#if defined(_CYHAL_ADC_RELEASE_BYPASS_VREF)
            _cyhal_utils_release_if_used(&(obj->ext_vref));
            _cyhal_utils_release_if_used(&(obj->bypass_pin));
#endif
            if(CYHAL_RSC_INVALID != obj->resource.type)
            {
                cyhal_hwmgr_free(&obj->resource);
            }
        }
#if !defined(_CYHAL_ADC_SAR_SKIP_BASE)
        obj->base = NULL;
#endif
    }
}

cy_rslt_t cyhal_adc_configure(cyhal_adc_t *obj, const cyhal_adc_config_t *config)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    obj->continuous_scanning = config->continuous_scanning;

#if !(_CYHAL_ADC_EXTERNAL_VREF_SUPPORTED)
    /* No external bypass or external vref option available */
    if (config->is_bypassed
        || CYHAL_ADC_REF_INTERNAL != config->vref
        || CYHAL_ADC_VNEG_VSSA != config->vneg
        || config->average_count < 1u
        || config->average_count > (UINT8_MAX + 1)) /* 8-bit field stores (desired_count - 1) */
    {
        /* No need to check here that the other vref/bypass related parameters are consistent with these
         * selections, there are already checks further down that do that */
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    /* Even though average count can technically be any value between 1 and 256, when we're operating in "true"
     * average mode (i.e. not accumulate) we're limited to powers of 2 because those are the only division
     * ratios that can be achieved through the "shiftRight" functionality.
     */

     obj->average_is_accumulate = (0u != (config->average_mode_flags & CYHAL_ADC_AVG_MODE_ACCUMULATE));
     if(false == obj->average_is_accumulate)
     {
         bool saw_one = false;
         for(uint8_t i = 0; i < 16u; ++i)
         {
             if(0u != (config->average_count & (1u << i)))
             {
                 if(saw_one) /* Two bits set, not a power of 2 */
                 {
                    result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
                    break;
                 }
                 saw_one = true;
             }
         }
     }
#endif
#if defined(CYHAL_PIN_MAP_DRIVE_MODE_PASS_SAR_EXT_VREF0)
    if(NC != config->ext_vref)
    {
        // If this pin wasn't used in the previous config for either vref or bypass, reserve it
        if((NC == obj->ext_vref) && (config->ext_vref != obj->bypass_pin))
        {
            const cyhal_resource_pin_mapping_t* ext_vref_map =
                _cyhal_utils_get_resource(config->ext_vref, cyhal_pin_map_pass_sar_ext_vref0,
                    sizeof(cyhal_pin_map_pass_sar_ext_vref0)/sizeof(cyhal_pin_map_pass_sar_ext_vref0[0]), &(obj->resource),
                    false);

            if (NULL == ext_vref_map)
            {
                result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(ext_vref_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_SAR_EXT_VREF0);
            }

            if(CY_RSLT_SUCCESS == result)
            {
                obj->ext_vref = config->ext_vref;
            }
        }
    }
    else
    {
        if(NC != obj->ext_vref) // We used to have an external vref pin - free it
        {
            // If the same pin was used as bypass, mark it freed now too
            if(obj->ext_vref == obj->bypass_pin)
            {
                obj->bypass_pin = NC;
            }
            // It is okay to do this without checking if the pin is still used for bypass,
            // because in that case we will just re-reserve the pin below
            cyhal_gpio_free(obj->ext_vref);
            obj->ext_vref = NC;
        }

        // If external vref exists as a GPIO, it's an error to set vref to external without passing in the pin
        if(CYHAL_ADC_REF_EXTERNAL == config->vref)
        {
            result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
        }
    }

    if(NC != config->bypass_pin)
    {
        if(CY_RSLT_SUCCESS == result)
        {
            // Bypass and ext_vref share the same hard-wired IO connection
            const cyhal_resource_pin_mapping_t* bypass_map =
                _cyhal_utils_get_resource(config->bypass_pin, cyhal_pin_map_pass_sar_ext_vref0,
                    sizeof(cyhal_pin_map_pass_sar_ext_vref0)/sizeof(cyhal_pin_map_pass_sar_ext_vref0[0]), &(obj->resource),
                    false);

            if (NULL == bypass_map)
            {
                result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
            }
            else if(config->bypass_pin != config->ext_vref) // It's valid to use the same pin for both ext_vref and bypass
            {
                result = _cyhal_utils_reserve_and_connect(bypass_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_SAR_EXT_VREF0);
            }
        }

        if(CY_RSLT_SUCCESS == result)
        {
            obj->bypass_pin = config->bypass_pin;
        }
    }
    else
    {
        // We used to have an external vref pin - free it, unless it's still used for ext_vref
        if((NC != obj->bypass_pin) && (obj->ext_vref != obj->bypass_pin))
        {
            cyhal_gpio_free(obj->bypass_pin);
            obj->bypass_pin = NC;
        }

        // If bypass exists as a GPIO, it's an error to enable bypass without passing in the pin
        if(config->is_bypassed)
        {
            result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
        }
    }
#else
    /* No GPIO pins for VREF - it must be using a dedicated pad */
    if(config->bypass_pin != NC || config->ext_vref != NC)
    {
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }
#endif

    if(false == ((CYHAL_ADC_REF_EXTERNAL == config->vref) ^ (0u == config->ext_vref_mv)))
    {
        /* Must have exactly one of: ext vref selected, ext vref voltage unspecified */
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_adc_sar_configure(obj, config);
    }
    
    if(obj->continuous_scanning)
    {
        obj->conversion_complete = false;
        _cyhal_adc_sar_update_interrupt_mask(obj);
        _cyhal_adc_sar_start_convert(obj);
    }
    return result;
}

cy_rslt_t cyhal_adc_set_power(cyhal_adc_t *obj, cyhal_power_level_t power)
{
    // The SAR doesn't have selectable power levels in the same way that the opamps do.
    if(CYHAL_POWER_LEVEL_OFF == power)
    {
        _cyhal_adc_sar_disable(obj);
    }
    else
    {
        _cyhal_adc_sar_enable(obj);
    }
    return CY_RSLT_SUCCESS;
}

#if !(CYHAL_ADC_SHARED_CLOCK)
uint32_t _cyhal_adc_calc_optimal_clock_rate(cyhal_adc_t* obj, uint32_t target_sample_hz)
{
    /* From the architecture TRM */
    const uint32_t ADC_CLOCK_MAX_HZ = 60000000;
    const uint32_t ADC_CLOCK_MIN_HZ = 1000000;

    uint32_t sample_period_ns = _CYHAL_UTILS_NS_PER_SECOND / target_sample_hz;
    uint32_t total_acquisition_ns, conversion_clock_cycles;
    _cyhal_adc_sar_get_sample_times(obj, &total_acquisition_ns, &conversion_clock_cycles);

    uint32_t conversion_budget_ns;
    if(sample_period_ns < total_acquisition_ns)
    {
        // Requested sampling rate is impossible - go as fast as we can.
        conversion_budget_ns = 1;
    }
    else
    {
        conversion_budget_ns = sample_period_ns - total_acquisition_ns;
    }

    uint32_t target_period_ns = conversion_budget_ns / conversion_clock_cycles;
    uint32_t target_clock_hz = _CYHAL_UTILS_NS_PER_SECOND / target_period_ns;
    if(target_clock_hz > ADC_CLOCK_MAX_HZ)
    {
        target_clock_hz = ADC_CLOCK_MAX_HZ;
    }
    else if(target_clock_hz < ADC_CLOCK_MIN_HZ)
    {
        target_clock_hz = ADC_CLOCK_MIN_HZ;
    }

    return target_clock_hz;
}
#endif

cy_rslt_t cyhal_adc_set_sample_rate(cyhal_adc_t* obj, uint32_t desired_sample_rate_hz, uint32_t* achieved_sample_rate_hz)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    /* If we don't own the clock, the caller needs to adjust it and/or the acquisition times to achive the desired rate */
#if !(CYHAL_ADC_SHARED_CLOCK) && !defined(_CYHAL_ADC_SAR_SKIP_PCLK)
    if(obj->dedicated_clock)
    {
        uint32_t desired_hz = _cyhal_adc_calc_optimal_clock_rate(obj, desired_sample_rate_hz);
        result = cyhal_clock_set_frequency(&(obj->clock), desired_hz, NULL);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_adc_sar_populate_acquisition_timers(obj);
    }
#else
    // If we can't adjust the clock ever, we can't change the
    // sample rate, we can just report what we have
    CY_UNUSED_PARAMETER(desired_sample_rate_hz);
#endif

    if(CY_RSLT_SUCCESS == result)
    {
        *achieved_sample_rate_hz = _cyhal_adc_sar_compute_actual_sample_rate(obj);
    }
    else
    {
        *achieved_sample_rate_hz = 0u;
    }
    return result;
}


/*******************************************************************************
*       ADC Channel HAL Functions
*******************************************************************************/

cy_rslt_t cyhal_adc_channel_init_diff(cyhal_adc_channel_t *obj, cyhal_adc_t* adc, cyhal_gpio_t vplus, cyhal_gpio_t vminus, const cyhal_adc_channel_config_t* cfg)
{
    CY_ASSERT(obj != NULL);
    CY_ASSERT(adc != NULL);

    cy_rslt_t result = CY_RSLT_SUCCESS;

    memset(obj, 0, sizeof(cyhal_adc_channel_t));
    obj->vplus = NC;
#if !(_CYHAL_ADC_SINGLE_ENDED)
    obj->vminus = NC;
    const cyhal_resource_pin_mapping_t *vminus_map = NULL;
#endif

    // Check for invalid pin or pin belonging to a different SAR
    const cyhal_resource_pin_mapping_t *vplus_map = _cyhal_utils_get_resource(vplus, cyhal_pin_map_pass_sarmux_pads,
        sizeof(cyhal_pin_map_pass_sarmux_pads)/sizeof(cyhal_pin_map_pass_sarmux_pads[0]), NULL, false);

    if(NULL == vplus_map || adc->resource.block_num != vplus_map->block_num)
    {
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    uint8_t chosen_channel = 0;

    if (CY_RSLT_SUCCESS == result)
    {
        // Find the first available channel
        #if defined(_CYHAL_ADC_VBG_CHANNEL_IDX)
        for(chosen_channel = _CYHAL_ADC_VBG_CHANNEL_IDX + 1; chosen_channel < _CYHAL_SAR_MAX_NUM_CHANNELS; ++chosen_channel)
        #else
        for(chosen_channel = 0; chosen_channel < _CYHAL_SAR_MAX_NUM_CHANNELS; ++chosen_channel)
        #endif /* defined(_CYHAL_ADC_VBG_CHANNEL_IDX) */
        {
            if(NULL == adc->channel_config[chosen_channel])
            {
                break;
            }
        }
        if (chosen_channel >= _CYHAL_SAR_MAX_NUM_CHANNELS) // No channels available
            result = CYHAL_ADC_RSLT_NO_CHANNELS;
    }

    if(CY_RSLT_SUCCESS == result)
    {
        // Don't set the ADC until here so that free knows whether we have allocated
        // the channel on the parent ADC instance (and therefore doesn't try to free it if
        // something fails further up)
        obj->adc = adc;
        obj->channel_idx = chosen_channel;
        obj->adc->channel_config[chosen_channel] = obj;
        obj->minimum_acquisition_ns = (cfg->min_acquisition_ns > _CYHAL_ADC_MIN_ACQUISITION_TIME_NS)
                                       ? cfg->min_acquisition_ns : _CYHAL_ADC_MIN_ACQUISITION_TIME_NS;
    }

    if((CY_RSLT_SUCCESS == result) && (CYHAL_ADC_VNEG != vminus))
    {
        #if (_CYHAL_ADC_SINGLE_ENDED)
        /* We don't support differential channels */
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
        #else
        vminus_map = _cyhal_utils_get_resource(vminus, cyhal_pin_map_pass_sarmux_pads,
            sizeof(cyhal_pin_map_pass_sarmux_pads)/sizeof(cyhal_pin_map_pass_sarmux_pads[0]), NULL, false);
        if (NULL == vminus_map || adc->resource.block_num != vminus_map->block_num)
        {
            result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
        }
        #if defined(_CYHAL_ADC_EVEN_VPLUS_PIN)
        // For PSoC™ 4A devices, vplus must be an even number pin, and vminus the following odd numbered pin
        else if (((vplus & 1) != 0) || ((vplus + 1) != vminus))
        {
            result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
        }
        #endif
        #endif /* (_CYHAL_ADC_SINGLE_ENDED) */
    }

    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_utils_reserve_and_connect(vplus_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_SARMUX_PADS);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->vplus = vplus;
#if !(_CYHAL_ADC_SINGLE_ENDED)
        if(CYHAL_ADC_VNEG != vminus)
        {
            result = _cyhal_utils_reserve_and_connect(vminus_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_SARMUX_PADS);
        }
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->vminus = vminus;
#endif
        result = _cyhal_adc_sar_populate_acquisition_timers(obj->adc);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_adc_sar_channel_init(obj, adc, vplus, vminus, cfg);
    }

    if(CY_RSLT_SUCCESS != result)
    {
        cyhal_adc_channel_free(obj);
    }

    return result;
}

cy_rslt_t cyhal_adc_channel_configure(cyhal_adc_channel_t *obj, const cyhal_adc_channel_config_t *config)
{
    CY_ASSERT(NULL != obj);

    obj->minimum_acquisition_ns = (config->min_acquisition_ns > _CYHAL_ADC_MIN_ACQUISITION_TIME_NS)
                                   ? config->min_acquisition_ns : _CYHAL_ADC_MIN_ACQUISITION_TIME_NS;

    return _cyhal_adc_sar_channel_config(obj, config);
}

void cyhal_adc_channel_free(cyhal_adc_channel_t *obj)
{
    if(obj->adc != NULL)
    {
        // Disable the channel, the unconfigure it
        obj->adc->channel_config[obj->channel_idx] = NULL;

        if(false == obj->adc->owned_by_configurator)
        {
            _cyhal_utils_release_if_used(&(obj->vplus));
#if !(_CYHAL_ADC_SINGLE_ENDED)
            _cyhal_utils_release_if_used(&(obj->vminus));
#endif
        }

        _cyhal_adc_sar_channel_deinit(obj);
        obj->adc = NULL;
    }
}

uint16_t cyhal_adc_read_u16(const cyhal_adc_channel_t *obj)
{
    int32_t signed_result = cyhal_adc_read(obj);
    return _cyhal_adc_sar_counts_to_u16(obj, signed_result);
}

int32_t cyhal_adc_read(const cyhal_adc_channel_t *obj)
{
    return _cyhal_adc_sar_read(obj);
}

int32_t cyhal_adc_read_uv(const cyhal_adc_channel_t *obj)
{
    CY_ASSERT(NULL != obj);

    int32_t counts = cyhal_adc_read(obj);
    return _cyhal_adc_sar_counts_to_uvolts(obj->adc, obj->channel_idx, counts);
}

void _cyhal_adc_start_async_read(cyhal_adc_t* obj, size_t num_scan, int32_t* result_list)
{
    CY_ASSERT(NULL == obj->async_buff_next); /* Transfer already in progress */
    uint32_t savedIntrStatus = cyhal_system_critical_section_enter();
    obj->async_scans_remaining = num_scan;
    obj->async_buff_next = result_list;
    obj->async_buff_orig = result_list;
    _cyhal_adc_sar_update_interrupt_mask(obj);

    if(false == obj->continuous_scanning)
    {
        _cyhal_adc_sar_start_convert(obj);
    }
    cyhal_system_critical_section_exit(savedIntrStatus);
}

cy_rslt_t cyhal_adc_read_async(cyhal_adc_t* obj, size_t num_scan, int32_t* result_list)
{
    CY_ASSERT(NULL != obj);
    obj->async_transfer_in_uv = false;
    _cyhal_adc_start_async_read(obj, num_scan, result_list);
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cyhal_adc_read_async_uv(cyhal_adc_t* obj, size_t num_scan, int32_t* result_list)
{
    CY_ASSERT(NULL != obj);
    obj->async_transfer_in_uv = true;
    _cyhal_adc_start_async_read(obj, num_scan, result_list);
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cyhal_adc_set_async_mode(cyhal_adc_t *obj, cyhal_async_mode_t mode, uint8_t dma_priority)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL == obj->async_buff_next); /* Can't swap mode while a transfer is running */

    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(mode == CYHAL_ASYNC_DMA)
    {
#if _CYHAL_ADC_DMA_SUPPORTED
        result = cyhal_dma_init(&(obj->dma), CYHAL_DMA_PRIORITY_DEFAULT, CYHAL_DMA_DIRECTION_PERIPH2MEM);
        if(CY_RSLT_SUCCESS == result)
        {
            cyhal_dma_register_callback(&(obj->dma), &_cyhal_adc_dma_handler, obj);
            cyhal_dma_enable_event(&(obj->dma), CYHAL_DMA_TRANSFER_COMPLETE, dma_priority, true);
        }
#else
        CY_UNUSED_PARAMETER(dma_priority);
        result = CYHAL_ADC_RSLT_BAD_ARGUMENT; // DMA not supported
#endif
    }
    else
    {
#if _CYHAL_ADC_DMA_SUPPORTED
        /* Free the DMA instances if we reserved them but don't need them anymore */
        if(CYHAL_RSC_INVALID != obj->dma.resource.type)
        {
            cyhal_dma_free(&obj->dma);
            obj->dma.resource.type = CYHAL_RSC_INVALID;
        }
#endif
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->async_mode = mode;
    }
    return CY_RSLT_SUCCESS;
}


/*******************************************************************************
*       Event functions
*******************************************************************************/

void cyhal_adc_register_callback(cyhal_adc_t *obj, cyhal_adc_event_callback_t callback, void *callback_arg)
{
    CY_ASSERT(NULL != obj);

    uint32_t savedIntrStatus = cyhal_system_critical_section_enter();
    obj->callback_data.callback = (cy_israddress) callback;
    obj->callback_data.callback_arg = callback_arg;
    cyhal_system_critical_section_exit(savedIntrStatus);
}

void cyhal_adc_enable_event(cyhal_adc_t *obj, cyhal_adc_event_t event, uint8_t intr_priority, bool enable)
{
    if(enable)
    {
        obj->user_enabled_events |= event;
    }
    else
    {
        obj->user_enabled_events &= ~event;
    }

    _cyhal_adc_sar_update_interrupt_mask(obj);
    _cyhal_adc_sar_update_interrupt_priority(obj, intr_priority);
}


/*******************************************************************************
*       Interconnect functions
*******************************************************************************/

#if defined(_CYHAL_ADC_SAR_TR_IN_SUPPORTED)
static cyhal_dest_t _cyhal_adc_calculate_dest(uint8_t block_num)
{
    CY_ASSERT(block_num < _CYHAL_ADC_SAR_INSTANCES);
    return _cyhal_adc_tr_in[block_num];
}
#endif

#if !defined(_CYHAL_ADC_SAR_CUSTOM_SOURCE_CALC)
static cyhal_source_t _cyhal_adc_calculate_source(cyhal_adc_t *obj)
{
    CY_ASSERT(obj->resource.block_num < _CYHAL_ADC_SAR_INSTANCES);
    return _cyhal_adc_tr_out[obj->resource.block_num];
}
#endif

cy_rslt_t cyhal_adc_connect_digital(cyhal_adc_t *obj, cyhal_source_t source, cyhal_adc_input_t input)
{
#if defined(_CYHAL_ADC_SAR_TR_IN_SUPPORTED)
    if(input == CYHAL_ADC_INPUT_START_SCAN)
    {
        _cyhal_adc_sar_connect_digital(obj, source, input);
        cyhal_dest_t dest = _cyhal_adc_calculate_dest(obj->resource.block_num);
        return _cyhal_connect_signal(source, dest);
    }
#else
    CY_UNUSED_PARAMETER(obj);
    CY_UNUSED_PARAMETER(source);
    CY_UNUSED_PARAMETER(input);
#endif

    return CYHAL_ADC_RSLT_BAD_ARGUMENT;
}

cy_rslt_t cyhal_adc_enable_output(cyhal_adc_t *obj, cyhal_adc_output_t output, cyhal_source_t *source)
{
    cy_rslt_t result = CYHAL_ADC_RSLT_BAD_ARGUMENT;
    if(output == CYHAL_ADC_OUTPUT_SCAN_COMPLETE)
    {
        result = _cyhal_adc_sar_enable_output(obj, output, source);
#if !defined(_CYHAL_ADC_SAR_CUSTOM_SOURCE_CALC)
	// If custom source calculation is required, it will have been performed
	// in _cyhal_adc_sar_enable_output
        *source = _cyhal_adc_calculate_source(obj);
#endif
    }

    return result;
}

cy_rslt_t cyhal_adc_disconnect_digital(cyhal_adc_t *obj, cyhal_source_t source, cyhal_adc_input_t input)
{
#if defined(_CYHAL_ADC_SAR_TR_IN_SUPPORTED)
    if(input == CYHAL_ADC_INPUT_START_SCAN)
    {
        _cyhal_adc_sar_disconnect_digital(obj, source, input);
        cyhal_dest_t dest = _cyhal_adc_calculate_dest(obj->resource.block_num);
        return _cyhal_disconnect_signal(source, dest);
    }
#else
    CY_UNUSED_PARAMETER(obj);
    CY_UNUSED_PARAMETER(source);
    CY_UNUSED_PARAMETER(input);
#endif

    return CYHAL_ADC_RSLT_BAD_ARGUMENT;
}

cy_rslt_t cyhal_adc_disable_output(cyhal_adc_t *obj, cyhal_adc_output_t output)
{
    if(output != CYHAL_ADC_OUTPUT_SCAN_COMPLETE)
    {
        return CYHAL_ADC_RSLT_BAD_ARGUMENT;
    }

    _cyhal_adc_sar_disable_output(obj, output);
    return CY_RSLT_SUCCESS;
}

#if defined(__cplusplus)
}
#endif

#endif /* (_CYHAL_DRIVER_AVAILABLE_ADC_SAR) */
