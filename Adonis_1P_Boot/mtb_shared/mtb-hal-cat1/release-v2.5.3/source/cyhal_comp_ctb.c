/***************************************************************************/ /**
* \file cyhal_comp_ctb.c
*
* \brief
* Provides an implementation of the comp HAL on top of the CTB opamps.
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
#include "cyhal_comp_ctb.h"
#include "cyhal_gpio.h"
#include "cyhal_analog_common.h"
#include "cyhal_hwmgr.h"
#include "cyhal_irq_impl.h"
#include "cyhal_system_impl.h"

#if (_CYHAL_DRIVER_AVAILABLE_COMP_CTB)

#if defined(__cplusplus)
extern "C"
{
#endif

#define _CYHAL_OPAMP_PER_CTB (2u)
CY_NOINIT static cyhal_comp_t* _cyhal_comp_ctb_config_structs[_CYHAL_CTB_INSTANCES * _CYHAL_OPAMP_PER_CTB];
static bool _cyhal_comp_ctb_arrays_initialized = false;

#if !(CY_IP_MXS22LPPASS)

static const cy_stc_ctb_opamp_config_t _cyhal_comp_ctb_default_config =
{
    /* oaPower is specified in init */
#if defined(CY_IP_MXS40PASS_CTB_INSTANCES)
    .deepSleep    = CY_CTB_DEEPSLEEP_ENABLE,
    .oaMode       = CY_CTB_MODE_COMP,
    .oaPump       = _CYHAL_CTB_PUMP_ENABLE,
    .oaCompEdge   = CY_CTB_COMP_EDGE_DISABLE,
    .oaCompLevel  = CY_CTB_COMP_DSI_TRIGGER_OUT_LEVEL,
    .oaCompBypass = _CYHAL_COMP_CTB_DEFAULT_BYPASS,
    /* oaCompHyst is specified in init */
    .oaCompIntrEn = true,
#else
    .outputMode = CY_CTB_MODE_COMP,
    .pump       = _CYHAL_CTB_PUMP_ENABLE,
    .compEdge   = CY_CTB_COMP_EDGE_DISABLE,
    .compLevel  = CY_CTB_COMP_TRIGGER_OUT_LEVEL,
    .compBypass = _CYHAL_COMP_CTB_DEFAULT_BYPASS,
    /* compHyst is specified in init */
    .compIntrEn = true,
#endif
};


static const _cyhal_system_irq_t _cyhal_ctb_irq_n[] =
{
#if (CY_IP_MXS40PASS_CTB_INSTANCES == 1)
    pass_interrupt_ctbs_IRQn,
#elif (CY_IP_M0S8PASS4A_CTB_INSTANCES == 1)
    pass_0_interrupt_ctbs_IRQn,
#elif (_CYHAL_CTB_INSTANCES == 2)
    pass_0_interrupt_ctbs_IRQn,
    pass_1_interrupt_ctbs_IRQn,
#else
    #error Unhandled CTB instance count
#endif
};

/** Get the comp config struct for the opamp that caused the current interrupt */
static cyhal_comp_t* _cyhal_ctb_get_interrupt_source(void)
{
    uint32_t ctb_num = 0;
#if (_CYHAL_CTB_INSTANCES > 1)
    _cyhal_system_irq_t irqn = _cyhal_irq_get_active();
    for (uint32_t i = 0; i < sizeof(_cyhal_ctb_irq_n) / sizeof(_cyhal_system_irq_t); i++)
    {
        if (_cyhal_ctb_irq_n[i] == irqn)
        {
            ctb_num = i;
        }
    }
#endif

    CTBM_Type *ctbm = _cyhal_ctb_base[ctb_num];
    for(uint8_t oa_num = 0; oa_num < _CYHAL_OPAMP_PER_CTB; ++oa_num)
    {
        if(Cy_CTB_GetInterruptStatusMasked(ctbm, _cyhal_opamp_convert_sel(oa_num)))
        {
            cyhal_comp_t* inst = _cyhal_comp_ctb_config_structs[(ctb_num * _CYHAL_OPAMP_PER_CTB) + oa_num];
            if (NULL != inst)
            {
                return inst;
            }
        }
    }

    return NULL;
}
#endif


#if (CY_IP_MXS22LPPASS)
/** Get the comp config struct for the opamp that caused the current interrupt */
static cyhal_comp_t* _cyhal_ctb_get_interrupt_source(void)
{
    // Return only the first comp even if multiple are active.
    // If there's another one, it will be handled the next time the
    // interrupt comes around
    uint32_t intr_cause = Cy_AutAnalog_GetInterruptCause();
#if (PASS_NR_CTBLS) > 0
    if(0u != (intr_cause & CY_AUTANALOG_INT_CTBL0_COMP0))
    {
        return _cyhal_comp_ctb_config_structs[0];
    }
    if(0u != (intr_cause & CY_AUTANALOG_INT_CTBL0_COMP1))
    {
        return _cyhal_comp_ctb_config_structs[1];
    }
#endif
#if (PASS_NR_CTBLS) > 1
    if(0u != (intr_cause & CY_AUTANALOG_INT_CTBL1_COMP0))
    {
        return _cyhal_comp_ctb_config_structs[2];
    }
    if(0u != (intr_cause & CY_AUTANALOG_INT_CTBL1_COMP1))
    {
        return _cyhal_comp_ctb_config_structs[3];
    }
#endif
#if (PASS_NR_CTBLS) > 2
        #error "Unhandled CTBL instance count"
#endif

    CY_ASSERT(false); // LPPASS Common should only call us if one of the comps has an interrupt
    return NULL;
}


static cyhal_comp_event_t _cyhal_comp_ctb_get_enabled_events(cyhal_comp_t * obj)
{
    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);
    cy_en_autanalog_ctb_comp_int_t edge_config_val = (0u == obj->resource.channel_num)
        ? (static_cfg->intComp0)
        : (static_cfg->intComp1);

    switch(edge_config_val)
    {
        case CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED:
            return CYHAL_COMP_EDGE_NONE;
        case CY_AUTANALOG_CTB_COMP_INT_EDGE_RISING:
            return CYHAL_COMP_RISING_EDGE;
        case CY_AUTANALOG_CTB_COMP_INT_EDGE_FALLING:
            return CYHAL_COMP_FALLING_EDGE;
        case CY_AUTANALOG_CTB_COMP_INT_EDGE_BOTH:
            return (cyhal_comp_event_t)(CYHAL_COMP_RISING_EDGE | CYHAL_COMP_FALLING_EDGE);
        default:
            CY_ASSERT(false);
            return CYHAL_COMP_EDGE_NONE;
    }
}

static cy_en_autanalog_ctb_comp_int_t _cyhal_comp_ctb_convert_hal_event(cyhal_comp_event_t event)
{
    switch((uint8_t)event)
    {
        case 0u:
            return CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED;
        case (uint8_t)CYHAL_COMP_RISING_EDGE:
            return CY_AUTANALOG_CTB_COMP_INT_EDGE_RISING;
        case (uint8_t)CYHAL_COMP_FALLING_EDGE:
            return     CY_AUTANALOG_CTB_COMP_INT_EDGE_FALLING;
        case (uint8_t)(CYHAL_COMP_RISING_EDGE | CYHAL_COMP_FALLING_EDGE):
            return CY_AUTANALOG_CTB_COMP_INT_EDGE_BOTH;
        default:
            CY_ASSERT(false);
            return CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED;
    }
}


static uint32_t _cyhal_comp_ctb_get_interrupt_mask(cyhal_resource_inst_t* ctb_loc)
{
    static const uint32_t mask_per_block[] =
    {
#if (PASS_NR_CTBLS) > 0
        CY_AUTANALOG_INT_CTBL0_COMP0,
        CY_AUTANALOG_INT_CTBL0_COMP1,
#endif
#if (PASS_NR_CTBLS) > 1
        CY_AUTANALOG_INT_CTBL1_COMP0,
        CY_AUTANALOG_INT_CTBL1_COMP1,
#endif
#if (PASS_NR_CTBLS) > 2
        #error "Unhandled CTB instance count"
#endif
    };

    return mask_per_block[((ctb_loc->block_num << 1) + ctb_loc->channel_num)];
}

#else

static cyhal_comp_event_t _cyhal_comp_ctb_get_enabled_events(cyhal_comp_t * obj)
{
    uint32_t edge_config_val = (0u == obj->resource.channel_num)
        ? (CTBM_OA_RES0_CTRL(obj->base_ctb) & CTBM_OA_RES0_CTRL_OA0_COMPINT_Msk)
        : (CTBM_OA_RES1_CTRL(obj->base_ctb) & CTBM_OA_RES1_CTRL_OA1_COMPINT_Msk);

    switch(edge_config_val)
    {
        case CY_CTB_COMP_EDGE_DISABLE:
            return (cyhal_comp_event_t)0u;
        case CY_CTB_COMP_EDGE_RISING:
            return CYHAL_COMP_RISING_EDGE;
        case CY_CTB_COMP_EDGE_FALLING:
            return CYHAL_COMP_FALLING_EDGE;
        case CY_CTB_COMP_EDGE_BOTH:
            return (cyhal_comp_event_t)(CYHAL_COMP_RISING_EDGE | CYHAL_COMP_FALLING_EDGE);
        default:
            CY_ASSERT(false);
            return (cyhal_comp_event_t)0u;
    }
}

static cy_en_ctb_comp_edge_t _cyhal_comp_ctb_convert_hal_event(cyhal_comp_event_t event)
{
    switch((uint8_t)event)
    {
        case 0u:
            return CY_CTB_COMP_EDGE_DISABLE;
        case (uint8_t)CYHAL_COMP_RISING_EDGE:
            return CY_CTB_COMP_EDGE_RISING;
        case (uint8_t)CYHAL_COMP_FALLING_EDGE:
            return CY_CTB_COMP_EDGE_FALLING;
        case (uint8_t)(CYHAL_COMP_RISING_EDGE | CYHAL_COMP_FALLING_EDGE):
            return CY_CTB_COMP_EDGE_BOTH;
        default:
            CY_ASSERT(false);
            return CY_CTB_COMP_EDGE_DISABLE;
    }
}
#endif


static void _cyhal_comp_ctb_irq_handler(void)
{
    cyhal_comp_t* obj = _cyhal_ctb_get_interrupt_source();
    #if (CY_IP_MXS22LPPASS)
    Cy_AutAnalog_ClearInterrupt(_cyhal_comp_ctb_get_interrupt_mask(&(obj->resource)));
    #else
    Cy_CTB_ClearInterrupt(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num));
    #endif
    cyhal_comp_event_callback_t callback = (cyhal_comp_event_callback_t)obj->callback_data.callback;
    if(NULL != callback)
    {
        // The CTB hardware doesn't directly capture the event, so just pass on the converted mask
        cyhal_comp_event_t event = _cyhal_comp_ctb_get_enabled_events(obj);
        callback(obj->callback_data.callback_arg, event);
    }

}

static void _cyhal_comp_ctb_cfg_init(void)
{
    if (!_cyhal_comp_ctb_arrays_initialized)
    {
        for (uint8_t i = 0; i < _CYHAL_CTB_INSTANCES * _CYHAL_OPAMP_PER_CTB; i++)
        {
            _cyhal_comp_ctb_config_structs[i] = NULL;
        }
        _cyhal_comp_ctb_arrays_initialized = true;
    }
}

#if (CY_IP_MXS22LPPASS)
static cy_rslt_t _cyhal_comp_update_sta_dyn_cfg(cyhal_comp_t *obj, bool hysteresis)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);

    uint32_t saved_intr = cyhal_system_critical_section_enter();

    if(obj->resource.channel_num == 0)
    {
        static_cfg->pwrOpamp0 = CY_AUTANALOG_CTB_OA_PWR_OFF;
        static_cfg->topologyOpamp0 = (hysteresis == true) ? CY_AUTANALOG_CTB_OA_TOPO_HYST_COMPARATOR : CY_AUTANALOG_CTB_OA_TOPO_COMPARATOR;
        static_cfg->intComp0 = CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED;
        static_cfg->capFbOpamp0 = CY_AUTANALOG_CTB_OA_FB_CAP_0_pF;
        static_cfg->capCcOpamp0 = CY_AUTANALOG_CTB_OA_CC_CAP_0_1_pF;
    }
    else
    {
        static_cfg->pwrOpamp1 = CY_AUTANALOG_CTB_OA_PWR_OFF;
        static_cfg->topologyOpamp1 = (hysteresis == true) ? CY_AUTANALOG_CTB_OA_TOPO_HYST_COMPARATOR : CY_AUTANALOG_CTB_OA_TOPO_COMPARATOR;
        static_cfg->intComp1 = CY_AUTANALOG_CTB_COMP_INT_EDGE_DISABLED;
        static_cfg->capFbOpamp1 = CY_AUTANALOG_CTB_OA_FB_CAP_0_pF;
        static_cfg->capCcOpamp1 = CY_AUTANALOG_CTB_OA_CC_CAP_0_1_pF;
    }

    cyhal_system_critical_section_exit(saved_intr);
    
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
    cy_stc_autanalog_ctb_dyn_t * dynamic_config = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + another_oa_idx);

    // No config was found for plus pin
    if ((!p_pin_config_found) ||
        // No config was found for minus pin
        (((!m_pin_config_found) ||
        // OA0 and OA1 pin configs cannot be same
        ((uint32_t)ninv_cfg_to_set == (uint32_t)inv_cfg_to_set))))
    {
        result = CYHAL_COMP_RSLT_ERR_BAD_ARGUMENT;
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
        dynamic_config->outToPin = false;
        cyhal_system_critical_section_exit(saved_intr);
    }
         
    return result;

}


cy_rslt_t _cyhal_comp_ctb_init_hw(cyhal_comp_t *obj, const cyhal_comp_configurator_t* cfg)
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

    _cyhal_lppass_register_irq_handler(_CYHAL_LPPASS_IRQ_CTB, _cyhal_comp_ctb_irq_handler);

    _cyhal_comp_ctb_config_structs[(obj->resource.block_num  << 1 ) + obj->resource.channel_num] = obj;

    return result;
}
#else
cy_rslt_t _cyhal_comp_ctb_init_hw(cyhal_comp_t *obj, const cy_stc_ctb_opamp_config_t* cfg)
{
    obj->base_ctb = _cyhal_ctb_base[obj->resource.block_num];
    cy_rslt_t result = Cy_CTB_OpampInit(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num), cfg);
    if(CY_RSLT_SUCCESS == result)
    {
        /* Initialize the programmable analog */
        cyhal_analog_ctb_init(obj->base_ctb);

        _cyhal_irq_register(_cyhal_ctb_irq_n[obj->resource.block_num], CYHAL_ISR_PRIORITY_DEFAULT, _cyhal_comp_ctb_irq_handler);
        _cyhal_irq_enable(_cyhal_ctb_irq_n[obj->resource.block_num]);
        _cyhal_comp_ctb_config_structs[(obj->resource.block_num * _CYHAL_OPAMP_PER_CTB) + obj->resource.channel_num] = obj;
    }
    else
    {
        obj->base_ctb = NULL;
    }

    return result;
}
#endif

cy_rslt_t _cyhal_comp_ctb_init(cyhal_comp_t *obj, cyhal_gpio_t vin_p, cyhal_gpio_t vin_m, cyhal_gpio_t output, cyhal_comp_config_t *cfg)
{
    CY_ASSERT(NULL != obj);

    /* Initial values */
    cy_rslt_t result = CY_RSLT_SUCCESS;
    memset(obj, 0, sizeof(cyhal_comp_t));
    obj->resource.type = CYHAL_RSC_INVALID;

    /* CTB configuration objects initialization */
    _cyhal_comp_ctb_cfg_init();

    /* Validate pins. vin_p and vin_m are mandatory pins, vout is optional. */
    if ((NC == vin_p) || (NC == vin_m))
    {
        result = CYHAL_COMP_RSLT_ERR_INVALID_PIN;
    }

#if CY_IP_MXS22LPPASS
    //Output pins are controlled through the AC and at this stage we don't want to enable the output pin overall as it 
    //might impact other drivers
    if( NC != output)
    {
        result = CYHAL_COMP_RSLT_ERR_INVALID_PIN;
    }
#endif

    /* Allocate resources */
    if(CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_opamp_init_common(&(obj->resource), CYHAL_COMP_RSLT_ERR_INVALID_PIN, vin_p, vin_m, NC /* vout unused by comparator */, output);
    }

    /* Configure the opamp */
    if (result == CY_RSLT_SUCCESS)
    {
        obj->pin_vin_p = vin_p;
        obj->pin_vin_m = vin_m;
        obj->pin_out = output;
#if CY_IP_MXS22LPPASS
        /* Configure the lppass*/
        if(CY_RSLT_SUCCESS == result)
        {
            result = _cyhal_comp_update_sta_dyn_cfg(obj, cfg->hysteresis);
        }
        //Populate a configurator object with the needed elements
        cyhal_comp_configurator_t config;
        memset(&config, 0, sizeof(cyhal_comp_configurator_t));

        //Static configuration
        config.static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);

        //Dynamic configuration
        config.dynamic_cfg_num = CY_IP_MXS22LPPASS_CTB_INSTANCES*_CYHAL_OPAMP_OA_NUM_PER_CTB;
        config.dynamic_cfg = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + obj->resource.channel_num);

        result = _cyhal_comp_ctb_init_hw(obj, &config);
#else
        cy_stc_ctb_opamp_config_t config = _cyhal_comp_ctb_default_config;
#if defined(CY_IP_MXS40PASS_CTB_INSTANCES)
        config.oaPower = (cy_en_ctb_power_t)_cyhal_opamp_convert_power(cfg->power);
        config.oaCompHyst = _CYHAL_COMP_CTB_HIST(cfg->hysteresis);
#else
        config.power = (cy_en_ctb_power_t)_cyhal_opamp_convert_power(cfg->power);
        config.compHyst = _CYHAL_COMP_CTB_HIST(cfg->hysteresis);
#endif
        result = _cyhal_comp_ctb_init_hw(obj, &config);
#endif
    }
#if !(CY_IP_MXS22LPPASS)
    if (result == CY_RSLT_SUCCESS)
    {
        /* OPAMP Routing. Close input switches for OA0 or OA1. */
        Cy_CTB_SetAnalogSwitch(obj->base_ctb, _cyhal_opamp_convert_switch(obj->resource.channel_num), _cyhal_opamp_pin_to_mask(obj->resource.channel_num, vin_p, vin_m, NC), _CYHAL_CTB_SW_CLOSE);
        _cyhal_opamp_set_isolation_switch(obj->resource.channel_num, obj->base_ctb, true);
    }
#endif
    /* Free OPAMP in case of failure */
    if (result != CY_RSLT_SUCCESS)
    {
        _cyhal_comp_ctb_free(obj);
    }
    return result;
}

cy_rslt_t _cyhal_comp_ctb_init_cfg(cyhal_comp_t *obj, const cyhal_comp_configurator_t *cfg)
{
    /* CTB configuration objects initialization */
    _cyhal_comp_ctb_cfg_init();

#if (CY_IP_MXS22LPPASS)

    cy_rslt_t result = _cyhal_comp_ctb_init_hw(obj, cfg);

    //Update shared configurations to reflect inputs coming from device configurator
    cy_stc_autanalog_ctb_dyn_t * dynamic_config = _cyhal_lppass_get_ctb_dynamic_cfg((obj->resource.block_num << 1) + obj->resource.channel_num);
    memcpy(dynamic_config, cfg->dynamic_cfg, sizeof(cy_stc_autanalog_ctb_dyn_t));

    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);
    CY_UNUSED_PARAMETER(static_cfg);

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
#else
    cy_rslt_t result = _cyhal_comp_ctb_init_hw(obj, cfg->opamp);
#endif
    
    if(CY_RSLT_SUCCESS != result)
    {
        _cyhal_comp_ctb_free(obj);
    }

    return result;
}

void _cyhal_comp_ctb_free(cyhal_comp_t *obj)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(_cyhal_comp_ctb_arrays_initialized); /* Should not be freeing if we never initialized anything */

    if(CYHAL_RSC_INVALID != obj->resource.type)
    {
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

            _cyhal_lppass_enable_ctb(_CYHAL_LPPASS_DRIVER_CTB, obj->resource.block_num, obj->resource.channel_num, false);
            #else
            Cy_CTB_SetAnalogSwitch(obj->base_ctb, _cyhal_opamp_convert_switch(obj->resource.channel_num), _cyhal_opamp_pin_to_mask(obj->resource.channel_num, obj->pin_vin_p, obj->pin_vin_m, NC), _CYHAL_CTB_SW_OPEN);
            _cyhal_opamp_set_isolation_switch(obj->resource.channel_num, obj->base_ctb, false);
            #endif
        }
        #if (CY_IP_MXS22LPPASS)
        _cyhal_analog_free();

        _cyhal_comp_ctb_config_structs[(obj->resource.block_num << 1) + obj->resource.channel_num] = NULL;

        #else
        cyhal_analog_ctb_free(obj->base_ctb);
  

        _cyhal_comp_ctb_config_structs[(obj->resource.block_num * _CYHAL_OPAMP_PER_CTB) + obj->resource.channel_num] = NULL;

        uint8_t ctb_num = obj->resource.block_num;
        /* If neither opamp in this ctb is in use, disable the ISR */
        if((NULL == _cyhal_comp_ctb_config_structs[ctb_num * _CYHAL_OPAMP_PER_CTB])
            && (NULL == _cyhal_comp_ctb_config_structs[(ctb_num * _CYHAL_OPAMP_PER_CTB) + 1]))
        {
            _cyhal_irq_free(_cyhal_ctb_irq_n[obj->resource.block_num]);
        }
        #endif

        if(false == obj->owned_by_configurator)
        {
            cyhal_hwmgr_free(&(obj->resource));
        }
        #if !(CY_IP_MXS22LPPASS)
        obj->base_ctb = NULL;
        #endif
        obj->resource.type = CYHAL_RSC_INVALID;
    }

    _cyhal_utils_release_if_used(&(obj->pin_vin_p));
    _cyhal_utils_release_if_used(&(obj->pin_vin_m));
    _cyhal_utils_release_if_used(&(obj->pin_out));
}

cy_rslt_t _cyhal_comp_ctb_set_power(cyhal_comp_t *obj, cyhal_power_level_t power)
{
    CY_ASSERT(NULL != obj);

    #if (CY_IP_MXS22LPPASS)
    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);

    cy_en_autanalog_ctb_oa_pwr_t * oa_x_power_cfg_field = (obj->resource.channel_num == 0) ?
            &(static_cfg->pwrOpamp0) :
            &(static_cfg->pwrOpamp1);
    uint32_t saved_intr = cyhal_system_critical_section_enter();
    *oa_x_power_cfg_field = (cy_en_autanalog_ctb_oa_pwr_t)_cyhal_opamp_convert_power(power);
    cy_rslt_t result = (cy_rslt_t)Cy_AutAnalog_CTB_LoadStaticConfig(
            obj->resource.block_num, static_cfg);
    cyhal_system_critical_section_exit(saved_intr);
    return result;
    #else
    cy_en_ctb_power_t power_level = (cy_en_ctb_power_t)_cyhal_opamp_convert_power(power);
    Cy_CTB_SetPower(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num), power_level, _CYHAL_CTB_PUMP_ENABLE);
    return CY_RSLT_SUCCESS;
    #endif
}

cy_rslt_t _cyhal_comp_ctb_configure(cyhal_comp_t *obj, cyhal_comp_config_t *cfg)
{
    CY_ASSERT(NULL != obj);
#if !defined(CY_IP_MXS22LPPASS)
#if defined(CY_IP_MXS40PASS_CTB_INSTANCES)
    cy_rslt_t result = _cyhal_comp_ctb_set_power(obj, cfg->power);
    if(CY_RSLT_SUCCESS == result)
    {
        Cy_CTB_CompSetConfig(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num),
            CY_CTB_COMP_DSI_TRIGGER_OUT_LEVEL, _CYHAL_COMP_CTB_DEFAULT_BYPASS, _CYHAL_COMP_CTB_HIST(cfg->hysteresis));
    }
#else
    cy_stc_ctb_opamp_config_t config = _cyhal_comp_ctb_default_config;
    config.power = (cy_en_ctb_power_t)_cyhal_opamp_convert_power(cfg->power);
    config.compHyst = _CYHAL_COMP_CTB_HIST(cfg->hysteresis);
    cy_rslt_t result = Cy_CTB_OpampInit(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num), &config);
#endif
#else
    cy_rslt_t result = _cyhal_comp_ctb_set_power(obj, cfg->power);
    if(CY_RSLT_SUCCESS == result)
    {
        cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);
        cy_en_autanalog_ctb_oa_topo_t * oa_x_topology_cfg_field = (obj->resource.channel_num == 0) ?
        &(static_cfg->topologyOpamp0) :
        &(static_cfg->topologyOpamp1);
        cy_en_autanalog_ctb_oa_topo_t requested_topology = (cfg->hysteresis == true) ? CY_AUTANALOG_CTB_OA_TOPO_HYST_COMPARATOR : CY_AUTANALOG_CTB_OA_TOPO_COMPARATOR;
        if(requested_topology != *oa_x_topology_cfg_field )
        {
            uint32_t saved_intr = cyhal_system_critical_section_enter();
            *oa_x_topology_cfg_field = requested_topology;
            result = (cy_rslt_t)Cy_AutAnalog_CTB_LoadStaticConfig(
                    obj->resource.block_num, static_cfg);
            cyhal_system_critical_section_exit(saved_intr);
        }
        else
        {
            /* Already in requested topology, no need to update */
        }
    }
#endif

    return result;
}

bool _cyhal_comp_ctb_read(cyhal_comp_t *obj)
{
    CY_ASSERT(NULL != obj);

    #if (CY_IP_MXS22LPPASS)
    return (1UL == Cy_AutAnalog_CTB_Comp_Read(obj->resource.block_num, obj->resource.channel_num));
    #else
    return (1UL == Cy_CTB_CompGetStatus(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num)));
    #endif
}

void _cyhal_comp_ctb_enable_event(cyhal_comp_t *obj, cyhal_comp_event_t event, uint8_t intr_priority, bool enable)
{
    CY_UNUSED_PARAMETER(intr_priority);
    cyhal_comp_event_t enabled_events = _cyhal_comp_ctb_get_enabled_events(obj);
    if(enable)
    {
        enabled_events |= event;
    }
    else
    {
        enabled_events &= (~event);
    }

    #if (CY_IP_MXS22LPPASS)
    _cyhal_lppass_set_irq_priority(intr_priority);

    cy_stc_autanalog_ctb_sta_t* static_cfg = _cyhal_lppass_get_ctb_static_cfg(obj->resource.block_num);
    cy_en_autanalog_ctb_comp_int_t pdl_event = _cyhal_comp_ctb_convert_hal_event(enabled_events);
    cy_en_autanalog_ctb_comp_int_t * oa_x_interrupt_cfg_field = (obj->resource.channel_num == 0) ?
        &(static_cfg->intComp0) :
        &(static_cfg->intComp1);
    if(pdl_event != *oa_x_interrupt_cfg_field )
    {
        uint32_t saved_intr = cyhal_system_critical_section_enter();
        *oa_x_interrupt_cfg_field = (cy_en_autanalog_ctb_comp_int_t)pdl_event;
        cy_rslt_t result = (cy_rslt_t)Cy_AutAnalog_CTB_LoadStaticConfig(
                obj->resource.block_num, static_cfg);
        CY_UNUSED_PARAMETER(result);
        cyhal_system_critical_section_exit(saved_intr);
        uint32_t newMask = _CYHAL_LPPASS_CTB_COMP_INTR_MASK & _cyhal_comp_ctb_get_interrupt_mask(&(obj->resource));
        uint32_t interruptMask = Cy_AutAnalog_GetInterruptMask();
        Cy_AutAnalog_SetInterruptMask(newMask | ((~newMask) & interruptMask));

    }
    else
    {
        /* Interrupt configuration already in desired state, no need to update */
    }
    #else
    _cyhal_system_irq_t irqn = _cyhal_ctb_irq_n[obj->resource.block_num];
    _cyhal_irq_set_priority(irqn, intr_priority);

    cy_en_ctb_comp_edge_t pdl_event = _cyhal_comp_ctb_convert_hal_event(enabled_events);
    Cy_CTB_CompSetInterruptEdgeType(obj->base_ctb, _cyhal_opamp_convert_sel(obj->resource.channel_num), pdl_event);
    #endif
}

#if defined(__cplusplus)
}
#endif

#endif /* _CYHAL_DRIVER_AVAILABLE_COMP_CTB */
