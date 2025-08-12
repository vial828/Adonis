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

#if defined(CY_IP_MXS40PASS_SAR_INSTANCES) || defined(CY_IP_MXS40EPASS_ESAR_INSTANCES) || defined(CY_IP_M0S8PASS4A_SAR_INSTANCES)

#include "cyhal_irq_impl.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(CY_IP_MXS40PASS_SAR_INSTANCES)
#define _CYHAL_ADC_SAR_INSTANCES CY_IP_MXS40PASS_SAR_INSTANCES
#elif defined(CY_IP_MXS40EPASS_ESAR_INSTANCES)
#define _CYHAL_ADC_SAR_INSTANCES CY_IP_MXS40EPASS_ESAR_INSTANCES
#elif defined(CY_IP_M0S8PASS4A_SAR_INSTANCES)
#define _CYHAL_ADC_SAR_INSTANCES CY_IP_M0S8PASS4A_SAR_INSTANCES
#endif

/* ESAR only sign extends to 16 bits, but we return 32-bit signed values. This means that
 * while we can technically read the ADC results using DMA, doing so doesn't bring any
 * real value because still have to post-process the results using the CPU to achieve
 * proper sign extension. We do this on existing CAT1A and CAT2 devices because we had
 * already released DMA support before the issue was realized and we can't remove it
 * without breaking BWC, but extending this behavior to new devices would just be misleading */
#if (CYHAL_DRIVER_AVAILABLE_DMA && !defined(CY_IP_MXS40EPASS_ESAR_INSTANCES))
    #define _CYHAL_ADC_DMA_SUPPORTED (1u)
#else
    #define _CYHAL_ADC_DMA_SUPPORTED (0u)
#endif

// The PDL for M0S8 PASS doesn't take the register as an argument; it always writes to MUX_SWITCH0
#if defined(CY_IP_M0S8PASS4A_SAR_INSTANCES)
    #define _CYHAL_SAR_MAX_NUM_CHANNELS   CY_SAR_SEQ_NUM_CHANNELS
    #define _CYHAL_ADC_SARSEQ_STATE(state) (state)
    #define _CYHAL_ADC_SWITCH_STATE(state) (state)
    #define _CYHAL_ADC_SET_SWITCH(base, mask, state) Cy_SAR_SetAnalogSwitch((base), (mask), (state))
#else
    #define _CYHAL_SAR_MAX_NUM_CHANNELS   CY_SAR_MAX_NUM_CHANNELS
    #define _CYHAL_ADC_SWITCH_STATE(state) ((state) ? CY_SAR_SWITCH_CLOSE : CY_SAR_SWITCH_OPEN)
    #define _CYHAL_ADC_SARSEQ_STATE(state) ((state) ? CY_SAR_SWITCH_SEQ_CTRL_ENABLE : CY_SAR_SWITCH_SEQ_CTRL_DISABLE)
    #define _CYHAL_ADC_SET_SWITCH(base, mask, state) Cy_SAR_SetAnalogSwitch((base), CY_SAR_MUX_SWITCH0, (mask), _CYHAL_ADC_SWITCH_STATE((state)))
#endif

#if defined(CY_IP_MXS40EPASS_ESAR)
    #define _CYHAL_ADC_MIN_ACQUISITION_TIME_NS  300UL
#else
    #define _CYHAL_ADC_MIN_ACQUISITION_TIME_NS  167UL
#endif

#if (CY_IP_MXS40PASS_SAR_INSTANCES == 1) && !defined(CY_DEVICE_PSOC6A256K)
    #define _CYHAL_ADC_SINGLE_UNSUFFIXED
#endif

#if defined(CY_IP_MXS40EPASS_ESAR)
// Expected to be lowest value possible
#define _CYHAL_ADC_VBG_CHANNEL_IDX          (0)
/* We don't support differential channels */
#define _CYHAL_ADC_SINGLE_ENDED             (1)
/* No external bypass or external vref option available */
#define _CYHAL_ADC_EXTERNAL_VREF_SUPPORTED  (0)
#else
#define _CYHAL_ADC_SINGLE_ENDED             (0)
#define _CYHAL_ADC_EXTERNAL_VREF_SUPPORTED  (1)
/* Allow stopping after scan in the ISR */
#define _CYHAL_ADC_STOP_AFTER_SCAN          (1)
#endif /* defined(CY_IP_MXS40EPASS_ESAR) */

#if defined(CY_IP_M0S8PASS4A_SAR_INSTANCES)
#define _CYHAL_ADC_RELEASE_BYPASS_VREF      (1)
#define _CYHAL_ADC_EVEN_VPLUS_PIN           (1)
#endif

#define _CYHAL_ADC_RESOLUTION 12u
#define _CYHAL_ADC_INTERNAL_VREF_MV 1200UL
#define _CYHAL_ADC_CONVERSION_CYCLES (_CYHAL_ADC_RESOLUTION + 2)
#define _CYHAL_ADC_SAR_TR_IN_SUPPORTED (1u)

#if defined(CY_IP_MXS40EPASS_ESAR_INSTANCES)
typedef PASS_SAR_Type _CYHAL_ADC_SAR_Type;
#else
typedef SAR_Type _CYHAL_ADC_SAR_Type;
#endif

#define _CYHAL_ADC_SAR_DMA_SRC(obj) ((obj)->base->CHAN_RESULT)


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

#if !defined(CY_IP_MXS40EPASS_ESAR_INSTANCES)
extern int32_t _cyhal_adc_sar_counts_to_uvolts(cyhal_adc_t* obj, uint8_t channel, int32_t counts);
#else
int32_t _cyhal_adc_sar_counts_to_uvolts(cyhal_adc_t* obj, uint8_t channel, uint32_t counts);
#endif /* defined(CY_IP_MXS40EPASS_ESAR_INSTANCES) */
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

#endif /* defined(CY_IP_MXS40PASS_SAR_INSTANCES) || defined(CY_IP_MXS40EPASS_ESAR_INSTANCES) || defined(CY_IP_M0S8PASS4A_SAR_INSTANCES) */
