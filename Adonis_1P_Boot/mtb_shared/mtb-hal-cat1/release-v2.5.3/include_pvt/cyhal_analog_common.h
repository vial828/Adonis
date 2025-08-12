/***************************************************************************//**
* \file cyhal_analog_common.h
*
* \brief
* Provides common functionality that needs to be shared among all drivers that
* interact with the Programmable Analog Subsystem.
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
#include "cyhal_lppass_common.h"
#endif

#if (_CYHAL_DRIVER_AVAILABLE_COMP_CTB)
#if defined(_CYHAL_DRIVER_AVAILABLE_COMP_CTB_PASS)
#include "cy_ctb.h"
#endif


/** \addtogroup group_hal_results_analog Analog HAL Results
 *  Analog specific return codes
 *  \ingroup group_hal_results
 *  \{ *//**

 */
/** Incorrect clock frequency */
#define CYHAL_ANALOG_ERROR_INCORRECT_CLOCK_FREQ \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_ANALOG, 0))

/**
 * \}
 */

#if defined(CY_IP_MXS40PASS_CTB_INSTANCES)
    #define _CYHAL_CTB_INSTANCES                CY_IP_MXS40PASS_CTB_INSTANCES
    #define _CYHAL_CTB_SW_OPEN                  CY_CTB_SWITCH_OPEN
    #define _CYHAL_CTB_SW_CLOSE                 CY_CTB_SWITCH_CLOSE
    #define _CYHAL_CTB_PUMP_ENABLE              CY_CTB_PUMP_ENABLE
    #define _CYHAL_COMP_CTB_DEFAULT_BYPASS      CY_CTB_COMP_BYPASS_NO_SYNC
    #define _CYHAL_COMP_CTB_HIST(hysteresis)    ((hysteresis) ? CY_CTB_COMP_HYST_10MV : CY_CTB_COMP_HYST_DISABLE)
#elif defined(CY_IP_M0S8PASS4A_CTB_INSTANCES)
    #define _CYHAL_CTB_INSTANCES                CY_IP_M0S8PASS4A_CTB_INSTANCES
    #define _CYHAL_CTB_SW_OPEN                  false
    #define _CYHAL_CTB_SW_CLOSE                 true
    #define _CYHAL_CTB_PUMP_ENABLE              true
    #define _CYHAL_COMP_CTB_DEFAULT_BYPASS      false
    #define _CYHAL_COMP_CTB_HIST(hysteresis)    (hysteresis)
#elif defined(CY_IP_MXS22LPPASS_CTB_INSTANCES)
    #define _CYHAL_CTB_INSTANCES                CY_IP_MXS22LPPASS_CTB_INSTANCES
    #define _CYHAL_CTB_SW_OPEN                  0 // TBD
    #define _CYHAL_CTB_SW_CLOSE                 0 // TBD
    #define _CYHAL_CTB_PUMP_ENABLE              0 // TBD
    #define _CYHAL_COMP_CTB_DEFAULT_BYPASS      0 // TBD
    #define _CYHAL_COMP_CTB_HIST(hysteresis)    0 // TBD
#else
    #error Unhandled PASS IP Block
#endif
#endif

#if (_CYHAL_DRIVER_AVAILABLE_PASS) || (_CYHAL_DRIVER_AVAILABLE_LPPASS)

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * Initialize the programmable analog. This utilizes reference counting to avoid
 * repeatedly initializing the analog subsystem when multiple analog blocks are in use
 * */
cy_rslt_t _cyhal_analog_init(void);


/**
 * Uninitialize the programmable analog. This utilizes reference counting to avoid
 * disabling the analog subsystem until all blocks which require it have been freed.
 */
void _cyhal_analog_free(void);

#if (_CYHAL_DRIVER_AVAILABLE_COMP_CTB)
#if (_CYHAL_DRIVER_AVAILABLE_COMP_CTB_PASS)

/**
 * Initialize the programmable analog for CTB. This utilizes reference counting to avoid
 * repeatedly initializing the analog subsystem when multiple analog blocks are in use
 *
 * @param[in] base   CTB(m) base address
 * */
void cyhal_analog_ctb_init(CTBM_Type *base);

/**
 * Uninitialize the programmable analog. This utilizes reference counting to avoid
 * disabling the analog subsystem until all blocks which require it have been freed.
 *
 * @param[in] base   CTB(m) base address
 */
void cyhal_analog_ctb_free(CTBM_Type *base);

/** Base address for each CTB */
extern CTBM_Type *const _cyhal_ctb_base[];

/**
  * Computes the switch mask to use for a set of opamp pins
  *
  * @param[in]  opamp_num Opamp number (index within CTB(m))
  * @param[in]  vin_p     Non-inverting input pin. Must be specified.
  * @param[in]  vin_m     Inverting input pin. May be NC.
  * @param[in]  vout      Opamp output pin (analog). May be NC.
  * @return the switch mask to use
  */
uint32_t _cyhal_opamp_pin_to_mask(uint8_t opamp_num, cyhal_gpio_t vin_p, cyhal_gpio_t vin_m, cyhal_gpio_t vout);
#endif /* (_CYHAL_DRIVER_AVAILABLE_COMP_CTB_PASS) */

/**
  * Performs basic initialization of an opamp and its associated set of pins that is common to
  * both opamp and comparator mode.
  * This reserves resources and configures switches to connect the pins. It does not configure the
  * opamp hardware.
  * If this returns success, the opamp and all pins will be reserved. If an error is returned, all resources
  * that were successfully reserved will have been freed.
  *
  * @param[out] rsc          The opamp resource to use.
  * @param[in] bad_arg_error The driver-specific error that should be returned if an invalid pin is specified
  * @param[in]  vin_p        Non-inverting input pin. Must be specified.
  * @param[in]  vin_m        Inverting input pin. May be NC.
  * @param[in]  vout         Opamp output pin (analog). May be NC. Generally not specified in combination with comp_out
  * @param[in]  comp_out     Comparator output pin (digital). May be NC. Generally not specified in combination with vout
  * @return Whether the init operation succeeded.
  */
cy_rslt_t _cyhal_opamp_init_common(cyhal_resource_inst_t* rsc, cy_rslt_t bad_arg_error, cyhal_gpio_t vin_p, cyhal_gpio_t vin_m, cyhal_gpio_t vout, cyhal_gpio_t comp_out);

/**
  * Converts a HAL power level enum to a PDL-level opamp power enum value
  *
  * @param[in] hal_power power level as a HAL enum value
  * @return the equivalent pdl-level enum value
  */
uint32_t _cyhal_opamp_convert_power(cyhal_power_level_t hal_power);

#if (_CYHAL_DRIVER_AVAILABLE_COMP_CTB_PASS)
#if defined(COMPONENT_CAT1)
/**
  * Converts an opamp number into a PDL-level `cy_en_ctb_switch_register_sel_t` value.
  *
  * @param[in] oa_num the opamp number within its CTB(m)
  * @return the PDL-level `cy_en_ctb_switch_register_sel_t` value.
  */
__STATIC_INLINE cy_en_ctb_switch_register_sel_t _cyhal_opamp_convert_switch(uint8_t oa_num)
{
    CY_ASSERT(oa_num < 2);
    return (oa_num == 0) ? CY_CTB_SWITCH_OA0_SW : CY_CTB_SWITCH_OA1_SW;
}

#else /* (_CYHAL_DRIVER_AVAILABLE_COMP_CTB_PASS) */

/**
  * Converts an opamp number into a PDL-level `cy_en_ctb_opamp_sel_t` value.
  *
  * @param[in] oa_num the opamp number within its CTB(m)
  * @return the PDL-level `cy_en_ctb_switch_register_sel_t` value.
  */
__STATIC_INLINE cy_en_ctb_opamp_sel_t _cyhal_opamp_convert_switch(uint8_t oa_num)
{
    CY_ASSERT(oa_num < 2);
    return (oa_num == 0) ? CY_CTB_OPAMP_0 : CY_CTB_OPAMP_1;
}
#endif /* (_CYHAL_DRIVER_AVAILABLE_COMP_CTB_PASS) */

/**
  * Converts an opamp number into a PDL-level `cy_en_ctb_opamp_sel` value.
  *
  * @param[in] oa_num the opamp number within its CTB(m)
  * @return the PDL-level `cy_en_ctb_opamp_sel` value.
  */
__STATIC_INLINE cy_en_ctb_opamp_sel_t _cyhal_opamp_convert_sel(uint8_t oa_num)
{
    CY_ASSERT(oa_num < 2);
    return (oa_num == 0) ? CY_CTB_OPAMP_0 : CY_CTB_OPAMP_1;
}

/**
 * Opens or closes the isolation switch if an opamp requires it
 *
 * @param[in] oa_num The opamp number within its CTB(m)
 * @param[in] close  Whether to close (true) or open (false) the switch
 */
__STATIC_INLINE void _cyhal_opamp_set_isolation_switch(uint8_t oa_num, CTBM_Type *base, bool close)
{
#if defined(COMPONENT_CAT1)
    if(0u == oa_num)
    {
        // OA0 has an additional isolation switch (CIS) on its vplus line
        Cy_CTB_SetAnalogSwitch(base, CY_CTB_SWITCH_CTD_SW, CY_CTB_SW_CTD_CHOLD_OA0_POS_ISOLATE_MASK,
            close ? CY_CTB_SWITCH_CLOSE : CY_CTB_SWITCH_OPEN);
    }
#else
    // These devices don't have the isolation switch
    CY_UNUSED_PARAMETER(oa_num);
    CY_UNUSED_PARAMETER(base);
    CY_UNUSED_PARAMETER(close);
#endif
}
#endif // !(_CYHAL_DRIVER_AVAILABLE_LPPASS)

#endif // _CYHAL_DRIVER_AVAILABLE_COMP_CTB


#if (_CYHAL_DRIVER_AVAILABLE_LPPASS)

#define _CYHAL_PRB_TAP_DIVIDER     (100)

/**
  * Converts a PDL PRB TAP level enum to a millivolt multiplier value
  *
  * @param[in] prb_tap position of tap to select required vref
  * @return the equivalent multiplier
  */
uint32_t _cyhal_prb_convert_tap(cy_en_autanalog_prb_tap_t prb_tap);


/**
  * Converts a PDL PRB SRC enum to a millivolt value
  *
  * @param[in] prb_src source voltage for the PRB
  * @return the equivalent millivolt value
  */
uint32_t _cyhal_prb_convert_src(cy_en_autanalog_prb_src_t prb_src);
#endif

#if defined(__cplusplus)
}
#endif

#endif // _CYHAL_DRIVER_AVAILABLE_PASS
