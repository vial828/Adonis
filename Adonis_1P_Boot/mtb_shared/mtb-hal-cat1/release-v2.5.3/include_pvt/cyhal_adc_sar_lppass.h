/*******************************************************************************
* File Name: cyhal_adc_sar_pass.h
*
* Description:
* Device specific implementation for ADC SAR PASS API.
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

#pragma once

#include "cyhal_adc.h"

#if defined(CY_IP_MXS22LPPASS_SAR_INSTANCES)

#include "cyhal_irq_impl.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define _CYHAL_ADC_SAR_DMA_SRC(obj) (SAR->STA.GPIO_CHAN_RESULT)
#define _CYHAL_ADC_SAR_INSTANCES (CY_IP_MXS22LPPASS_SAR_INSTANCES)

//TODO add this on
//#define _CYHAL_ADC_DMA_SUPPORTED (1u)

// ADC clock is always shared with the rest of the PASS
#define _CYHAL_ADC_SHARED_CLOCK (1u)

// The PDL for M0S8 PASS doesn't take the register as an argument; it always writes to MUX_SWITCH0
#define _CYHAL_SAR_MAX_NUM_CHANNELS (PASS_SAR_SAR_GPIO_CHANNELS)

#define _CYHAL_ADC_MIN_ACQUISITION_TIME_NS  20UL

// Technically this ADC only supports single-ended, but it
// has a simultaneous-sampling based pseudo-differential mode
// that hides this fact from the application
#define _CYHAL_ADC_SINGLE_ENDED             (0)
#define _CYHAL_ADC_EXTERNAL_VREF_SUPPORTED  (1)

#define _CYHAL_ADC_RESOLUTION 12u
#define _CYHAL_ADC_INTERNAL_VREF_MV 900UL // TODO check and define this value
#define _CYHAL_ADC_CONVERSION_CYCLES (_CYHAL_ADC_RESOLUTION + 4) // Note: differential counts twice TODO update sample rate calculationt to reflect

// No mapping for _CYHAL_ADC_SAR_TYPE; none of the APIs require a base address
#define _CYHAL_ADC_SAR_SKIP_BASE                (1)
#define _CYHAL_ADC_SAR_SKIP_PCLK                (1)
#define _CYHAL_ADC_SAR_CUSTOM_SOURCE_CALC 	(1)

/*******************************************************************************
*       Function prototypes
*******************************************************************************/

cy_rslt_t _cyhal_adc_config_hw(cyhal_adc_t *obj, const cyhal_adc_configurator_t* cfg, cyhal_gpio_t pin, bool owned_by_configurator);
void _cyhal_adc_irq_handler(void);

cy_rslt_t _cyhal_adc_sar_populate_acquisition_timers(cyhal_adc_t* obj);
void _cyhal_adc_sar_get_sample_times(cyhal_adc_t* obj, uint32_t* min_acquisition_ns, uint32_t* conversion_clock_cycles);
uint32_t _cyhal_adc_sar_compute_actual_sample_rate(cyhal_adc_t* obj);

uint8_t _cyhal_adc_sar_get_block_from_irqn(_cyhal_system_irq_t irqn);
void _cyhal_adc_sar_clear_interrupt(cyhal_adc_t *obj);
void _cyhal_adc_sar_update_interrupt_priority(cyhal_adc_t *obj, uint8_t intr_priority);
void _cyhal_adc_sar_update_interrupt_mask(const cyhal_adc_t* obj);

cy_rslt_t _cyhal_adc_sar_init(cyhal_adc_t *obj, cyhal_gpio_t pin, const cyhal_clock_t *clk);
cy_rslt_t _cyhal_adc_sar_init_hw(cyhal_adc_t *obj, const cyhal_adc_configurator_t* cfg);
void _cyhal_adc_sar_deinit(cyhal_adc_t *obj);
void _cyhal_adc_sar_enable(cyhal_adc_t *obj);
void _cyhal_adc_sar_disable(cyhal_adc_t *obj);
cy_rslt_t _cyhal_adc_sar_configure(cyhal_adc_t *obj, const cyhal_adc_config_t *config);
void _cyhal_adc_sar_start_convert(cyhal_adc_t* obj);
void _cyhal_adc_sar_stop_convert(cyhal_adc_t* obj);

int32_t _cyhal_adc_sar_counts_to_uvolts(cyhal_adc_t* obj, uint8_t channel, int32_t counts);
int32_t _cyhal_adc_sar_get_result(cyhal_adc_t* obj, uint8_t channel);

cy_rslt_t _cyhal_adc_sar_channel_init(cyhal_adc_channel_t *obj, cyhal_adc_t* adc, cyhal_gpio_t vplus, cyhal_gpio_t vminus, const cyhal_adc_channel_config_t* cfg);
cy_rslt_t _cyhal_adc_sar_channel_config(cyhal_adc_channel_t *obj, const cyhal_adc_channel_config_t *config);
void _cyhal_adc_sar_channel_deinit(cyhal_adc_channel_t *obj);
uint16_t _cyhal_adc_sar_counts_to_u16(const cyhal_adc_channel_t *obj, int32_t signed_result);
int32_t _cyhal_adc_sar_read(const cyhal_adc_channel_t *obj);

void _cyhal_adc_sar_connect_digital(cyhal_adc_t *obj, cyhal_source_t source, cyhal_adc_input_t input);
cy_rslt_t _cyhal_adc_sar_enable_output(cyhal_adc_t *obj, cyhal_adc_output_t output, cyhal_source_t *source);
void _cyhal_adc_sar_disconnect_digital(cyhal_adc_t *obj, cyhal_source_t source, cyhal_adc_input_t input);
void _cyhal_adc_sar_disable_output(cyhal_adc_t *obj, cyhal_adc_output_t output);


/*******************************************************************************
*       Inlined functions
*******************************************************************************/
static inline bool _cyhal_adc_is_conversion_complete(const cyhal_adc_channel_t *obj)
{
    return obj->adc->conversion_complete;
}

#define cyhal_adc_is_conversion_complete(obj)  _cyhal_adc_is_conversion_complete(obj)

static inline cy_rslt_t _cyhal_adc_read_latest(const cyhal_adc_channel_t* obj, int32_t *result)
{
    if (obj->adc->conversion_complete)
    {
        *result = _cyhal_adc_sar_get_result(obj->adc, obj->channel_idx);
        return CY_RSLT_SUCCESS;
    }
    else
    {
        return CYHAL_ADC_RSLT_ERR_BUSY;
    }
}

#define cyhal_adc_read_latest(obj, result) _cyhal_adc_read_latest(obj, result)

static inline cy_rslt_t _cyhal_adc_start_convert(cyhal_adc_t *obj)
{
    obj->conversion_complete = false;
    _cyhal_adc_sar_start_convert(obj);
    return CY_RSLT_SUCCESS;
}

#define cyhal_adc_start_convert(obj) _cyhal_adc_start_convert(obj)

#if defined(__cplusplus)
}
#endif

#endif /* defined(CY_IP_MXS22LPPASS_SAR_INSTANCES) */
