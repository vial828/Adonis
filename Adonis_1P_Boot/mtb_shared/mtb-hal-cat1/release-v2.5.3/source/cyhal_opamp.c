/***************************************************************************/ /**
* \file cyhal_opamp.c
*
* \brief
* Provides a high level interface for interacting with the Infineon Continuous
* Time Block. This interface abstracts out the chip specific details. If any chip
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

#include <string.h>
#include "cyhal_opamp.h"
#include "cyhal_gpio.h"
#include "cyhal_analog_common.h"
#include "cyhal_hwmgr.h"
#include "cy_ctb.h"
#include "cyhal_system_impl.h"

/**
* \addtogroup group_hal_impl_opamp Opamp (Operational Amplifier)
* \ingroup group_hal_impl
*
* \section group_hal_impl_opamp_power Power Level Mapping
* The following table shows how the HAL-defined power levels map to the hardware-specific power levels
* | HAL Power Level                | Opamp Power Level   |
* | ------------------------------ | ------------------- |
* | @ref CYHAL_POWER_LEVEL_HIGH    | CY_CTB_POWER_HIGH   |
* | @ref CYHAL_POWER_LEVEL_MEDIUM  | CY_CTB_POWER_MEDIUM |
* | @ref CYHAL_POWER_LEVEL_LOW     | CY_CTB_POWER_LOW    |
* | @ref CYHAL_POWER_LEVEL_DEFAULT | CY_CTB_POWER_MEDIUM |
*
*/

#if (CYHAL_DRIVER_AVAILABLE_OPAMP)

#if !(CY_IP_MXS22LPPASS)

static const cy_stc_ctb_opamp_config_t _cyhal_opamp_default_config =
{
#if defined(CY_IP_MXS40PASS_CTB_INSTANCES)
    .deepSleep    = CY_CTB_DEEPSLEEP_ENABLE,
    .oaPower      = CY_CTB_POWER_OFF,
    .oaMode       = CY_CTB_MODE_OPAMP10X,
    .oaPump       = _CYHAL_CTB_PUMP_ENABLE,
    .oaCompEdge   = CY_CTB_COMP_EDGE_DISABLE,
    .oaCompLevel  = CY_CTB_COMP_DSI_TRIGGER_OUT_LEVEL,
    .oaCompBypass = CY_CTB_COMP_BYPASS_SYNC,
    .oaCompHyst   = CY_CTB_COMP_HYST_DISABLE,
    .oaCompIntrEn = true,
#else
    .power      = CY_CTB_POWER_OFF,
    .outputMode = CY_CTB_MODE_OPAMP_EXTERNAL,
    .pump       = _CYHAL_CTB_PUMP_ENABLE,
    .compEdge   = CY_CTB_COMP_EDGE_DISABLE,
    .compLevel  = CY_CTB_COMP_TRIGGER_OUT_LEVEL,
    .compBypass = false,
    .compHyst   = false,
    .compIntrEn = true,
#endif
};

#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#if (CY_IP_MXS22LPPASS)

static cy_rslt_t _cyhal_opamp_config_lppass(cyhal_opamp_t *obj)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);

    //Determine Topology
    cy_en_autanalog_ctb_oa_topo_t opamp_topology = CY_AUTANALOG_CTB_OA_TOPO_COMPARATOR;

    if (obj->resource.channel_num == 0)
    {
        // OA 0 specific settings
        if(obj->pin_vin_m != NC)
        {
            // Opamp mode // vin_p is guaranteed to not be NC by init_common
            opamp_topology = CY_AUTANALOG_CTB_OA_TOPO_OPEN_LOOP_OPAMP;
        }
        else if (obj->pin_vin_m == NC)
        {
            // Follower mode
            opamp_topology = CY_AUTANALOG_CTB_OA_TOPO_BUFFER;
        }
    }
    else if (obj->resource.channel_num == 1)
    {
        // OA 1 specific settings
        if(obj->pin_vin_m != NC)
        {
            // Opamp mode
            opamp_topology = CY_AUTANALOG_CTB_OA_TOPO_OPEN_LOOP_OPAMP;
        }
        else if (obj->pin_vin_m == NC)
        {
            // Follower mode
            opamp_topology = CY_AUTANALOG_CTB_OA_TOPO_BUFFER;
        }
    }

    if (obj->resource.channel_num == 0)
    {
        uint32_t saved_intr = cyhal_system_critical_section_enter();
        static_cfg->pwrOpamp0 = CY_AUTANALOG_CTB_OA_PWR_OFF;
        static_cfg->topologyOpamp0 = opamp_topology;
        static_cfg->intComp0 = CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED;
        static_cfg->capFbOpamp0 = CY_AUTANALOG_CTB_OA_FB_CAP_0_pF;
        static_cfg->capCcOpamp0 = CY_AUTANALOG_CTB_OA_CC_CAP_0_1_pF;
        cyhal_system_critical_section_exit(saved_intr);
    }
    else if (obj->resource.channel_num == 1)
    {
        uint32_t saved_intr = cyhal_system_critical_section_enter();
        static_cfg->pwrOpamp1 = CY_AUTANALOG_CTB_OA_PWR_OFF;
        static_cfg->topologyOpamp1 = opamp_topology;
        static_cfg->intComp1 = CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED;
        static_cfg->capFbOpamp1 = CY_AUTANALOG_CTB_OA_FB_CAP_0_pF;
        static_cfg->capCcOpamp1 = CY_AUTANALOG_CTB_OA_CC_CAP_0_1_pF;
        cyhal_system_critical_section_exit(saved_intr);
    }
    
    
    bool p_pin_config_found = false;
    bool m_pin_config_found = false;

    cy_en_autanalog_ctb_oa_ninv_pin_t ninv_cfg_to_set = CY_AUTANALOG_CTB_OA_NINV_PIN_DISCONNECT;
    cy_en_autanalog_ctb_oa_inv_pin_t inv_cfg_to_set = CY_AUTANALOG_CTB_OA_INV_PIN_DISCONNECT;


    // Non-Inv pin
    #if defined (CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_VIN_P0)
    if (_CYHAL_UTILS_GET_RESOURCE(obj->pin_vin_p, cyhal_pin_map_opamp_vin_p0) != NULL)
    {
        p_pin_config_found = true;
        ninv_cfg_to_set = CY_AUTANALOG_CTB_OA_NINV_PIN_OA0_P0_OA1_P5;
    }
    #endif
    #if defined (CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_VIN_P1)
    if (!p_pin_config_found && (NULL != _CYHAL_UTILS_GET_RESOURCE(obj->pin_vin_p,
            cyhal_pin_map_opamp_vin_p1)))
    {
        p_pin_config_found = true;
        ninv_cfg_to_set = CY_AUTANALOG_CTB_OA_NINV_PIN_OA0_P1_OA1_P4;
    }
    #endif

    // Inv pin
    #if defined (CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_VIN_M0)
    if (_CYHAL_UTILS_GET_RESOURCE(obj->pin_vin_m, cyhal_pin_map_opamp_vin_m0) != NULL)
    {
        m_pin_config_found = true;
        inv_cfg_to_set = CY_AUTANALOG_CTB_OA_INV_PIN_OA0_P0_OA1_P5;
    }
    #endif
    #if defined (CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_VIN_M1)
    if (!m_pin_config_found && (NULL != _CYHAL_UTILS_GET_RESOURCE(obj->pin_vin_m,
            cyhal_pin_map_opamp_vin_m1)))
    {
        m_pin_config_found = true;
        inv_cfg_to_set = CY_AUTANALOG_CTB_OA_INV_PIN_OA0_P1_OA1_P4;
    }
    #endif

    // Index of OA, that shares same block with the one, that is being configured
    uint8_t another_oa_idx = (obj->resource.channel_num == 0) ? 1 : 0;
    cy_stc_autanalog_ctb_dyn_t * dynamic_config = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) +  another_oa_idx);

    // No config was found for plus pin
    if ((!p_pin_config_found) ||
        // Minus pin is present
        ((NC != obj->pin_vin_m) &&
            // No config was found for minus pin
            ((!m_pin_config_found) ||
            // OA0 and OA1 pin configs cannot be same
            ((uint32_t)ninv_cfg_to_set == (uint32_t)inv_cfg_to_set))
        ) ||
    // Both OA pins should be configured in a same way
        // nInv pin configuration exists for another OA in the block
        ((dynamic_config->ninvInpPin != CY_AUTANALOG_CTB_OA_NINV_PIN_DISCONNECT) &&
        // And it differs from the one, that we are going to set
        (dynamic_config->ninvInpPin != ninv_cfg_to_set)) ||
        // Inv pin configuration exists for another OA in the block
        ((dynamic_config->invInpPin != CY_AUTANALOG_CTB_OA_INV_PIN_DISCONNECT) &&
        // And it differs from the one, that we are going to set
        (dynamic_config->invInpPin != inv_cfg_to_set)))
    {
        result = CYHAL_OPAMP_RSLT_BAD_ARGUMENT;
    }
    else
    {
        dynamic_config = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + obj->resource.channel_num);
        uint32_t saved_intr = cyhal_system_critical_section_enter();
        dynamic_config->ninvInpPin = ninv_cfg_to_set;
        dynamic_config->invInpPin = inv_cfg_to_set;
        dynamic_config->ninvInpRef = CY_AUTANALOG_CTB_OA_NINV_REF_DISCONNECT;
        dynamic_config->resInpPin = CY_AUTANALOG_CTB_OA_RES_PIN_DISCONNECT;
        dynamic_config->resInpRef = CY_AUTANALOG_CTB_OA_RES_REF_DISCONNECT;
        dynamic_config->sharedMuxIn = CY_AUTANALOG_CTB_OA_MUX_IN_DISCONNECT;
        dynamic_config->sharedMuxOut = CY_AUTANALOG_CTB_OA_MUX_OUT_DISCONNECT;
        dynamic_config->outToPin = true;
        cyhal_system_critical_section_exit(saved_intr);
    }
         
    return result;

}


static cy_rslt_t _cyhal_opamp_init_hw(cyhal_opamp_t *obj, const cyhal_opamp_configurator_t *cfg)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = _cyhal_analog_init();

    if (CY_RSLT_SUCCESS == result)
    {
            result = (cy_rslt_t)Cy_AutAnalog_CTB_LoadStaticConfig(obj->resource.block_num, cfg->static_cfg); 
    }

    if (CY_RSLT_SUCCESS == result)
    {
            result = (cy_rslt_t)Cy_AutAnalog_CTB_LoadDynamicConfig(obj->resource.block_num, ((obj->resource.block_num << 1) + obj->resource.channel_num),
                    cfg->dynamic_cfg);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_lppass_enable_ctb(_CYHAL_LPPASS_DRIVER_CTB, obj->resource.block_num, obj->resource.channel_num, true);
        
    }

    if (CY_RSLT_SUCCESS == result)
    {
        obj->is_init_success = true;
    }

    return result;
}

#else // (CY_IP_MXS22LPPASS)

static cy_rslt_t _cyhal_opamp_init_hw(cyhal_opamp_t *obj, const cyhal_opamp_configurator_t *cfg)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    obj->base = _cyhal_ctb_base[obj->resource.block_num];
    if (false == obj->owned_by_configurator)
    {
        result = Cy_CTB_OpampInit(obj->base, _cyhal_opamp_convert_sel(obj->resource.channel_num), &_cyhal_opamp_default_config);
    }
    else
    {
        result = Cy_CTB_OpampInit(obj->base, _cyhal_opamp_convert_sel(obj->resource.channel_num), cfg->config);
    }
    if(CY_RSLT_SUCCESS == result)
    {
        obj->is_init_success = true;
        /* Initialize the programmable analog */
        cyhal_analog_ctb_init(obj->base);
    }

    return result;
}
#endif

cy_rslt_t cyhal_opamp_init(cyhal_opamp_t *obj, cyhal_gpio_t vin_p, cyhal_gpio_t vin_m, cyhal_gpio_t vout)
{
    /* Check if obj is free */
    CY_ASSERT(NULL != obj);

    /* Initial values */
    cy_rslt_t result = CY_RSLT_SUCCESS;
    memset(obj, 0, sizeof(cyhal_opamp_t));

#if !(CY_IP_MXS22LPPASS)
    obj->base = NULL;
#endif
    obj->resource.type = CYHAL_RSC_INVALID;
    obj->is_init_success = false;

    /* Validate input pins. vin_p and vout are mandatory pins, vin_m is optional. */
    if ((NC == vin_p) || (NC == vout))
    {
        result = CYHAL_OPAMP_RSLT_ERR_INVALID_PIN;
    }

    /* Allocate resources */
    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_opamp_init_common(&(obj->resource), CYHAL_OPAMP_RSLT_BAD_ARGUMENT, vin_p, vin_m, vout, NC /* comp_out unused by opamp */);
    }

    /* Configure the opamp */
    if (result == CY_RSLT_SUCCESS)
    {
        obj->pin_vin_p = vin_p;
        obj->pin_vin_m = vin_m;
        obj->pin_vout = vout;

#if CY_IP_MXS22LPPASS
        /* Configure the lppass*/
        if(CY_RSLT_SUCCESS == result)
        {
            result = _cyhal_opamp_config_lppass(obj);
        }
        if(CY_RSLT_SUCCESS == result)
        {
            //Populate a configurator object with the needed elements
            cyhal_opamp_configurator_t cfg;
            memset(&cfg, 0, sizeof(cyhal_opamp_configurator_t));

            //Static configuration
            cfg.static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);

            //Dynamic configuration
        	cfg.dynamic_cfg_num = CY_IP_MXS22LPPASS_CTB_INSTANCES*_CYHAL_OPAMP_OA_NUM_PER_CTB;
        	cfg.dynamic_cfg = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + obj->resource.channel_num);

            result = _cyhal_opamp_init_hw(obj, &cfg);
        }
#else

        result = _cyhal_opamp_init_hw(obj, NULL);
#endif

    }

#if !(CY_IP_MXS22LPPASS)
    if (result == CY_RSLT_SUCCESS)
    {
        /* OPAMP Routing. Close input switches for OA0 or OA1. */
        Cy_CTB_SetAnalogSwitch(obj->base,
                _cyhal_opamp_convert_switch(obj->resource.channel_num),
                _cyhal_opamp_pin_to_mask(obj->resource.channel_num, vin_p, vin_m, vout), _CYHAL_CTB_SW_CLOSE);
        _cyhal_opamp_set_isolation_switch(obj->resource.channel_num, obj->base, true);
    }
#endif

    /* Free OPAMP in case of failure */
    if (result != CY_RSLT_SUCCESS)
    {
        cyhal_opamp_free(obj);
    }
    return result;
}

cy_rslt_t cyhal_opamp_init_cfg(cyhal_opamp_t *obj, const cyhal_opamp_configurator_t *cfg)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != cfg);

    memset(obj, 0, sizeof(cyhal_opamp_t));
    obj->owned_by_configurator = true;
    obj->resource = *cfg->resource;
    obj->is_init_success = false;
    obj->pin_vin_p = NC;
    obj->pin_vin_m = NC;
    obj->pin_vout  = NC;
    cy_rslt_t result = _cyhal_opamp_init_hw(obj, cfg);

#if (CY_IP_MXS22LPPASS)
    //Update shared configurations to reflect inputs coming from device configurator
    cy_stc_autanalog_ctb_dyn_t * dynamic_config = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + obj->resource.channel_num);
    memcpy(dynamic_config, cfg->dynamic_cfg, sizeof(cy_stc_autanalog_ctb_dyn_t));

    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);
    //first address of the static configuration specific to opamp0 or opamp1
    cy_en_autanalog_ctb_oa_pwr_t * oa_x_power_cfg_field = (obj->resource.channel_num == 0) ?
            &(static_cfg->pwrOpamp0) :
            &(static_cfg->pwrOpamp1);
    if((obj->resource.channel_num == 0) )
    {
        memcpy(oa_x_power_cfg_field, cfg->static_cfg, (sizeof(cy_stc_autanalog_ctb_sta_t)/2));
    }
    else
    {
        memcpy(oa_x_power_cfg_field, &(cfg->static_cfg->pwrOpamp1), (sizeof(cy_stc_autanalog_ctb_sta_t)/2));
    }
#endif
    if(CY_RSLT_SUCCESS != result)
    {
        cyhal_opamp_free(obj);
    }
    return result;
}

cy_rslt_t cyhal_opamp_set_power(cyhal_opamp_t *obj, cyhal_power_level_t power)
{
    /* Safe convert power level from HAL (cyhal_power_level_t) to corresponding PDL value */
    uint32_t converted_power_level = _cyhal_opamp_convert_power(power);

    #if (CY_IP_MXS22LPPASS)
    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);

    cy_en_autanalog_ctb_oa_pwr_t * oa_x_power_cfg_field = (obj->resource.channel_num == 0) ?
            &(static_cfg->pwrOpamp0) :
            &(static_cfg->pwrOpamp1);
    uint32_t saved_intr = cyhal_system_critical_section_enter();
    *oa_x_power_cfg_field = (cy_en_autanalog_ctb_oa_pwr_t)converted_power_level;
    cy_rslt_t result = (cy_rslt_t)Cy_AutAnalog_CTB_LoadStaticConfig(
            obj->resource.block_num, static_cfg);
    cyhal_system_critical_section_exit(saved_intr);
    return result;

    #else

    Cy_CTB_SetPower(obj->base, _cyhal_opamp_convert_sel(obj->resource.channel_num), (cy_en_ctb_power_t)converted_power_level,
            _CYHAL_CTB_PUMP_ENABLE);
    return CY_RSLT_SUCCESS;

    #endif // CY_IP_MXS22LPPASS or other
}

void cyhal_opamp_free(cyhal_opamp_t *obj)
{
    if (NULL != obj
#if (CY_IP_MXS40PASS)
    && NULL != obj->base
#endif
    )
    {
        if (obj->is_init_success)
        {
            cyhal_opamp_set_power(obj, CYHAL_POWER_LEVEL_OFF);

            if(false == obj->owned_by_configurator)
            {
                #if (CY_IP_MXS22LPPASS)
                cy_stc_autanalog_ctb_dyn_t * dynamic_config = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + obj->resource.channel_num);

                uint32_t saved_intr = cyhal_system_critical_section_enter();
                dynamic_config->ninvInpPin =
                        CY_AUTANALOG_CTB_OA_NINV_PIN_DISCONNECT;
                dynamic_config->invInpPin =
                        CY_AUTANALOG_CTB_OA_INV_PIN_DISCONNECT;
                cyhal_system_critical_section_exit(saved_intr);

                cy_rslt_t result = _cyhal_lppass_enable_ctb(_CYHAL_LPPASS_DRIVER_CTB, obj->resource.block_num, obj->resource.channel_num, false);
                CY_UNUSED_PARAMETER(result);
                #else
                Cy_CTB_SetAnalogSwitch(obj->base, _cyhal_opamp_convert_switch(obj->resource.channel_num),
                        _cyhal_opamp_pin_to_mask(obj->resource.channel_num, obj->pin_vin_p, obj->pin_vin_m, obj->pin_vout),
                        _CYHAL_CTB_SW_OPEN);
                _cyhal_opamp_set_isolation_switch(obj->resource.channel_num, obj->base, false);
                #endif
            }
            #if (CY_IP_MXS40PASS)
            cyhal_analog_ctb_free(obj->base);
            #elif (CY_IP_MXS22LPPASS)
            _cyhal_analog_free();
            #endif
        }
    }

    if((NULL != obj) && (CYHAL_RSC_INVALID != obj->resource.type))
    {
        if(false == obj->owned_by_configurator)
        {
            cyhal_hwmgr_free(&obj->resource);
        }
        #if (CY_IP_MXS40PASS)
        obj->base = NULL;
        #endif
        obj->resource.type = CYHAL_RSC_INVALID;
    }

    if(NULL != obj)
    {
        _cyhal_utils_release_if_used(&(obj->pin_vin_p));
        _cyhal_utils_release_if_used(&(obj->pin_vout));
        _cyhal_utils_release_if_used(&(obj->pin_vin_m));
    }
}

#if defined(__cplusplus)
}
#endif

#endif /* CYHAL_DRIVER_AVAILABLE_OPAMP */
