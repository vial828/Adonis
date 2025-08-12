/***************************************************************************//**
* \file cyhal_lppass_common.h
*
* \brief
* Provides common functionality that needs to be shared among all drivers that
* interact with the Low Power Programmable Analog Subsystem.
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

#pragma once

#include <stdint.h>
#include "cyhal_hw_types.h"
#include "cyhal_general_types.h"
#include "cyhal_gpio.h"
#include "cy_result.h"

#if (CY_IP_MXS22LPPASS)
#include "cy_autanalog.h"

#define _CYHAL_PRB_TAP_DIVIDER     (100)
#define _CYHAL_LPPASS_VBGR_VALUE   (900) // VBGR is at 900mV
typedef enum 
{
    // The shared LPPASS IRQ handler was caused because of an event in the ADC
    _CYHAL_LPPASS_IRQ_ADC = 0,
    _CYHAL_LPPASS_IRQ_CTB = 1,
    _CYHAL_LPPASS_IRQ_PTC = 2,
    _CYHAL_LPPASS_IRQ_MAX = _CYHAL_LPPASS_IRQ_PTC // Alias for the highest value in this enum
} _cyhal_lppass_irq_t;

// STT entries are primarily based on ADC usage
typedef enum
{
    _CYHAL_LPPASS_STT_START = 0,
    _CYHAL_LPPASS_STT_TRIGGER = 1,
    _CYHAL_LPPASS_STT_ACQUISITION = 2,
    _CYHAL_LPPASS_STT_LOOP = 3,
    _CYHAL_LPPASS_STT_MAX = _CYHAL_LPPASS_STT_LOOP
} _cyhal_lppass_stt_t;

typedef enum
{
    _CYHAL_LPPASS_DRIVER_ADC    = 0,
    _CYHAL_LPPASS_DRIVER_CTB    = 1,
    _CYHAL_LPPASS_DRIVER_DAC    = 2,
    _CYHAL_LPPASS_DRIVER_PRB    = 3,
    _CYHAL_LPPASS_DRIVER_PTCOMP = 4,
    _CYHAL_LPPASS_DRIVER_MAX    = _CYHAL_LPPASS_DRIVER_PTCOMP
} _cyhal_lppass_drivers_t;

static const uint32_t _CYHAL_LPPASS_ADC_INTR_MASK =
    CY_AUTANALOG_INT_SAR0_DONE
    | CY_AUTANALOG_INT_SAR0_EOS
    | CY_AUTANALOG_INT_SAR0_RESULT
    | CY_AUTANALOG_INT_SAR0_RANGE0
    | CY_AUTANALOG_INT_SAR0_RANGE1
    | CY_AUTANALOG_INT_SAR0_RANGE2
    | CY_AUTANALOG_INT_SAR0_RANGE3
    | CY_AUTANALOG_INT_SAR0_FIR0_RESULT
    | CY_AUTANALOG_INT_SAR0_FIR1_RESULT;


    static const uint32_t _CYHAL_LPPASS_CTB_COMP_INTR_MASK = 
    CY_AUTANALOG_INT_CTBL0_COMP0
    | CY_AUTANALOG_INT_CTBL0_COMP1
    | CY_AUTANALOG_INT_CTBL1_COMP0
    | CY_AUTANALOG_INT_CTBL1_COMP1;

static const uint32_t _CYHAL_LPPASS_PTC_INTR_MASK =
    CY_AUTANALOG_INT_PTC0_COMP0
    | CY_AUTANALOG_INT_PTC0_COMP1;


extern const cyhal_clock_t* _cyhal_lppass_hf_clock;

// Internal helper for initializing the LPPASS. This is intended to be called
// from _cyhal_analog_init and therefore does not replicate the reference
// tracking performed within that function
cy_rslt_t _cyhal_lppass_init();

//Internal helper to update specific elements of the LPPASS. It can be called several times
// and will only modify and reload the updated stts 
cy_rslt_t _cyhal_lppass_enable_ctb(_cyhal_lppass_drivers_t driver, uint32_t instance, uint32_t channel, bool enable);

// Internal helper for freeing the LPPASS. This is intended to be called
// from _cyhal_analog_free and therefore does not replicate the reference
// tracking performed within that function
void _cyhal_lppass_free();

typedef void (*_cyhal_lppass_irq_handler_t)();

// Register a handler to be called when the shared LPPASS IRQ handler was
// caused due to an event in the specified subblock. To unregister, pass a null handler
void _cyhal_lppass_register_irq_handler(_cyhal_lppass_irq_t irq, _cyhal_lppass_irq_handler_t handler);

void _cyhal_lppass_set_irq_priority(uint8_t intr_priority);

// Allocates an unused trigger input and provides it to the caller
uint8_t _cyhal_lppass_allocate_trigger();

// Marks a particular trigger input as no longer used
uint8_t _cyhal_lppass_free_trigger(uint8_t trigger);

cy_stc_autanalog_stt_t* _cyhal_lppass_get_stt_entry(_cyhal_lppass_stt_t stt_user);
cy_rslt_t _cyhal_lppass_apply_state(_cyhal_lppass_stt_t stt_user);

/**
  * Converts a PDL PRB TAP level enum to a millivolt multiplier value
  *
  * @param[in] prb_tap position of tap to select required vref
  * @return the equivalent multiplier
  */
uint32_t _cyhal_lppass_prb_convert_tap(cy_en_autanalog_prb_tap_t prb_tap);


/**
  * Converts a PDL PRB SRC enum to a millivolt value
  *
  * @param[in] prb_src source voltage for the PRB
  * @return the equivalent millivolt value
  */
uint32_t _cyhal_lppass_prb_convert_src(cy_en_autanalog_prb_src_t prb_src);


/**
  * Returns point to shared CTB static config structure
  *
  * @return the pointer to the CTB static config structure
  */
cy_stc_autanalog_ctb_sta_t* _cyhal_lppass_get_ctb_static_cfg( uint32_t index);

/**
  * Returns point to specified CTB dynamic config structure
  *
  * @param[in] index index of the dynamic configuration requested
  * @return the pointer to the CTB dynamic config structure
  */
cy_stc_autanalog_ctb_dyn_t* _cyhal_lppass_get_ctb_dynamic_cfg( uint32_t index );

#endif

