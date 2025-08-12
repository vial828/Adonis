/***************************************************************************/ /**
* \file cyhal_comp_ptc.c
*
* \brief
* Provides an implementation of the comp HAL on top of the Programmable Threshold
* comparator (PTC).
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

#include <string.h>
#include "cyhal_comp_ptc.h"
#include "cyhal_gpio.h"
#include "cyhal_analog_common.h"
#include "cyhal_hwmgr.h"

#if (_CYHAL_DRIVER_AVAILABLE_COMP_PTC)
#include "cyhal_lppass_common.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define _CYHAL_COMP_PER_PTC (2u)

static bool _cyhal_comp_ptc_arrays_initialized = false;
CY_NOINIT static cyhal_comp_t* _cyhal_comp_ptc_config_structs[PASS_NR_PTCS * _CYHAL_COMP_PER_PTC];

static int _cyhal_comp_ptc_get_flat_index(uint8_t block_num, uint8_t comp_num)
{
    return (block_num * _CYHAL_COMP_PER_PTC) + comp_num;
}

/** Get the comp config struct for the ptc that caused the current interrupt */
static cyhal_comp_t* _cyhal_ptc_get_interrupt_source(void)
{
    // Return only the first comp even if multiple are active.
    // If there's another one, it will be handled the next time the
    // interrupt comes around
    uint32_t intr_cause = Cy_AutAnalog_GetInterruptCause();
#if (PASS_NR_PTCS) > 0
    if(0u != (intr_cause & CY_AUTANALOG_INT_PTC0_COMP0))
    {
        return _cyhal_comp_ptc_config_structs[0];
    }
    if(0u != (intr_cause & CY_AUTANALOG_INT_PTC0_COMP1))
    {
        return _cyhal_comp_ptc_config_structs[1];
    }
#endif
#if (PASS_NR_PTCS) > 1
        #error "Unhandled PTC instance count"
#endif

    CY_ASSERT(false); // LPPASS Common should only call us if one of the comps has an interrupt
    return NULL;
}

static cy_en_autanalog_ptcomp_comp_int_t _cyhal_comp_ptc_convert_hal_event(uint32_t event)
{
    switch(event)
    {
        case 0u:
            return CY_AUTANALOG_PTCOMP_COMP_INT_DISABLED;
        case (uint32_t)CYHAL_COMP_RISING_EDGE:
            return CY_AUTANALOG_PTCOMP_COMP_INT_EDGE_RISING;
        case (uint32_t)CYHAL_COMP_FALLING_EDGE:
            return CY_AUTANALOG_PTCOMP_COMP_INT_EDGE_FALLING;
        case (uint32_t)(CYHAL_COMP_RISING_EDGE | CYHAL_COMP_FALLING_EDGE):
            return CY_AUTANALOG_PTCOMP_COMP_INT_EDGE_BOTH;
        default:
            CY_ASSERT(false);
            return CY_AUTANALOG_PTCOMP_COMP_INT_DISABLED;
    }
}

static uint32_t _cyhal_comp_ptc_get_interrupt_mask(cyhal_resource_inst_t* ptc_loc)
{
    static const uint32_t mask_per_block[] =
    {
#if (PASS_NR_PTCS) > 0
        CY_AUTANALOG_INT_PTC0_COMP0,
        CY_AUTANALOG_INT_PTC0_COMP1,
#endif
#if (PASS_NR_PTCS) > 1
        #error "Unhandled PTC instance count"
#endif
    };

    return mask_per_block[_cyhal_comp_ptc_get_flat_index(ptc_loc->block_num, ptc_loc->channel_num)];
}

static cy_en_autanalog_ptcomp_comp_pwr_t _cyhal_comp_ptc_convert_power(cyhal_power_level_t hal_power)
{
    switch(hal_power)
    {
        default:
            CY_ASSERT(false);
            __attribute__((fallthrough));
        case CYHAL_POWER_LEVEL_OFF:
            return CY_AUTANALOG_PTCOMP_COMP_PWR_OFF;
        case CYHAL_POWER_LEVEL_LOW:
            return CY_AUTANALOG_PTCOMP_COMP_PWR_ULP;
        case CYHAL_POWER_LEVEL_MEDIUM:
        case CYHAL_POWER_LEVEL_DEFAULT:
            return CY_AUTANALOG_PTCOMP_COMP_PWR_LP;
        case CYHAL_POWER_LEVEL_HIGH:
            return CY_AUTANALOG_PTCOMP_COMP_PWR_NORMAL;
    }
}

static cy_en_autanalog_ptcomp_comp_hyst_t _cyhal_comp_ptc_convert_hysteresis(bool hyst_on)
{
    return hyst_on ? CY_AUTANALOG_PTCOMP_COMP_HYST_ON : CY_AUTANALOG_PTCOMP_COMP_HYST_OFF;
}

static cy_en_autanalog_ptcomp_comp_mux_t _cyhal_comp_ptc_convert_input(uint8_t pin_index)
{
    const cy_en_autanalog_ptcomp_comp_mux_t MUX_VALUES[] =
    {
        CY_AUTANALOG_PTCOMP_COMP_GPIO0,
        CY_AUTANALOG_PTCOMP_COMP_GPIO1,
        CY_AUTANALOG_PTCOMP_COMP_GPIO2,
        CY_AUTANALOG_PTCOMP_COMP_GPIO3,
        CY_AUTANALOG_PTCOMP_COMP_GPIO4,
        CY_AUTANALOG_PTCOMP_COMP_GPIO5,
        CY_AUTANALOG_PTCOMP_COMP_GPIO6,
        CY_AUTANALOG_PTCOMP_COMP_GPIO7,
    };

    return MUX_VALUES[pin_index];
}

static void _cyhal_comp_ptc_irq_handler(void)
{
    cyhal_comp_t* obj = _cyhal_ptc_get_interrupt_source();
    Cy_AutAnalog_ClearInterrupt(_cyhal_comp_ptc_get_interrupt_mask(&(obj->resource)));

    cyhal_comp_event_callback_t callback = (cyhal_comp_event_callback_t)obj->callback_data.callback;
    if(NULL != callback)
    {
        // The PTC hardware doesn't directly capture the event, so assume that whatever
        // event is enabled is the one that caused this (which should always be the case)
        cyhal_comp_event_t event = (cyhal_comp_event_t)(obj->irq_cause);
        callback(obj->callback_data.callback_arg, event);
    }
}

static void _cyhal_comp_ptc_cfg_init(void)
{
    if (!_cyhal_comp_ptc_arrays_initialized)
    {
        for (uint8_t i = 0; i < sizeof(_cyhal_comp_ptc_config_structs) / sizeof(_cyhal_comp_ptc_config_structs[0]); ++i)
        {
            _cyhal_comp_ptc_config_structs[i] = NULL;
        }
        _cyhal_comp_ptc_arrays_initialized = true;
    }
}

void _cyhal_comp_ptc_get_static_config(uint8_t ptc_num, cy_stc_autanalog_ptcomp_comp_sta_t *config)
{
    // The struct combines information about the configuration for both comparators within
    // this block, so we need to combine the information about both in order to
    // update the configuration
    cyhal_comp_t *comp0 = _cyhal_comp_ptc_config_structs[_cyhal_comp_ptc_get_flat_index(ptc_num, 0)];
    cyhal_comp_t *comp1 = _cyhal_comp_ptc_config_structs[_cyhal_comp_ptc_get_flat_index(ptc_num, 1)];
    memset(config, 0, sizeof(cy_stc_autanalog_ptcomp_comp_sta_t));
    if(NULL != comp0)
    {
        config->powerModeComp0 = comp0->ptc_power;
        config->compHystComp0  = _cyhal_comp_ptc_convert_hysteresis(comp0->ptc_hysteresis_enabled);
        config->compEdgeComp0  = _cyhal_comp_ptc_convert_hal_event(comp0->irq_cause);
    }
    else
    {
        config->powerModeComp0 = CY_AUTANALOG_PTCOMP_COMP_PWR_OFF;
    }
    if(NULL != comp1)
    {
        config->powerModeComp1 = comp1->ptc_power;
        config->compHystComp1  = _cyhal_comp_ptc_convert_hysteresis(comp1->ptc_hysteresis_enabled);
        config->compEdgeComp1  = _cyhal_comp_ptc_convert_hal_event(comp1->irq_cause);
    }
    else
    {
        config->powerModeComp1 = CY_AUTANALOG_PTCOMP_COMP_PWR_OFF;
    }

    config->lpDivPtcomp  = 1u; // Pick a default clock divider
    config->compPpCfgNum = 0u; // HAL doesn't expose the post-processing features
}

// Input is an array of arrays. Outer array is one element per PTC block instance.
// Inner array is one element per comparator within a PTC
// Note: this assumes that cyhal_comp_t.ptc_input_plus/ptc_input_minus have been populated already
void _cyhal_comp_ptc_populate_dynamic_configs(int block_num, cy_stc_autanalog_ptcomp_comp_dyn_t *dynCfgs)
{
    // For simplicity, each PTC is allocated its own dynamic config. There are 8 dynamic configs
    // per PTC and only 2 comparators per PTC block, so no possibility of overrunning
    for(uint8_t comp_num = 0; comp_num < _CYHAL_COMP_PER_PTC; ++comp_num)
    {
        // We always want at least to clear out the dynamic config, even if there's no PTC in use
        memset(&dynCfgs[comp_num], 0, sizeof(cy_stc_autanalog_ptcomp_comp_dyn_t));
        int flat_index = _cyhal_comp_ptc_get_flat_index(block_num, comp_num);
        if(NULL != _cyhal_comp_ptc_config_structs[flat_index])
        {
            dynCfgs[comp_num].ninvInpMux = _cyhal_comp_ptc_config_structs[flat_index]->ptc_input_plus;
            dynCfgs[comp_num].invInpMux  = _cyhal_comp_ptc_config_structs[flat_index]->ptc_input_minus;
        }
    }
}

void _cyhal_comp_ptc_update_stt(cy_stc_autanalog_stt_t *stt)
{
    for(uint8_t i = 0; i < PASS_NR_PTCS; ++i)
    {
        int comp0Index = (i * _CYHAL_COMP_PER_PTC);
        int comp1Index = (i * _CYHAL_COMP_PER_PTC) + 1;
        stt->ptcomp[i]->unlock = true;
        stt->ptcomp[i]->enableComp0 = (NULL != _cyhal_comp_ptc_config_structs[comp0Index]);
        stt->ptcomp[i]->dynCfgIdxComp0 = 0u;
        stt->ptcomp[i]->enableComp1 = (NULL != _cyhal_comp_ptc_config_structs[comp1Index]);
        stt->ptcomp[i]->dynCfgIdxComp1 = 1u;
    }
}

/* Reload the block configuration after something change in one of its comparators */
cy_rslt_t _cyhal_comp_reload_config(uint8_t block_num)
{
    cy_stc_autanalog_ptcomp_comp_sta_t staticConfig;
    cy_stc_autanalog_ptcomp_comp_dyn_t dynCfg[_CYHAL_COMP_PER_PTC];

    _cyhal_comp_ptc_populate_dynamic_configs(block_num, dynCfg);
    _cyhal_comp_ptc_get_static_config(block_num, &staticConfig);
    cy_stc_autanalog_ptcomp_t pdl_cfg =
    {
        .ptcompStaCfg    = &staticConfig,
        .ptcompDynCfgNum = sizeof(dynCfg) / sizeof(dynCfg[0]),
        .ptcompDynCfgArr = dynCfg
    };

    cy_rslt_t result = Cy_AutAnalog_PTComp_LoadConfig(block_num, &pdl_cfg);
    if(CY_RSLT_SUCCESS == result)
    {
        cy_stc_autanalog_stt_t *start_stt = _cyhal_lppass_get_stt_entry(_CYHAL_LPPASS_STT_START);
        _cyhal_comp_ptc_update_stt(start_stt);
        result = _cyhal_lppass_apply_state(_CYHAL_LPPASS_STT_START);
    }
    return result;
}

cy_rslt_t _cyhal_comp_ptc_init_hw(cyhal_comp_t *obj, const cy_stc_autanalog_ptcomp_t* cfg)
{
    cy_rslt_t result = _cyhal_analog_init();
    if(CY_RSLT_SUCCESS == result)
    {
        result = Cy_AutAnalog_PTComp_LoadConfig(obj->resource.block_num, cfg);
    }
    if(CY_RSLT_SUCCESS == result)
    {
        cy_stc_autanalog_stt_t *start_stt = _cyhal_lppass_get_stt_entry(_CYHAL_LPPASS_STT_START);
        _cyhal_comp_ptc_update_stt(start_stt);
        result = _cyhal_lppass_apply_state(_CYHAL_LPPASS_STT_START);
    }
    if(CY_RSLT_SUCCESS == result)
    {
        _cyhal_lppass_register_irq_handler(_CYHAL_LPPASS_IRQ_PTC, _cyhal_comp_ptc_irq_handler);
    }

    return result;
}

cy_rslt_t _cyhal_comp_ptc_init(cyhal_comp_t *obj, cyhal_gpio_t vin_p, cyhal_gpio_t vin_m, cyhal_gpio_t output, cyhal_comp_config_t *cfg)
{
    CY_ASSERT(NULL != obj);

    /* Initial values */
    cy_rslt_t result = CY_RSLT_SUCCESS;
    memset(obj, 0, sizeof(cyhal_comp_t));
    /* Mark pins in obj NC until they are successfully reserved */
    obj->pin_vin_p = NC;
    obj->pin_vin_m = NC;
    obj->pin_out = NC;

    obj->resource.type = CYHAL_RSC_INVALID;

    /* PTC configuration objects initialization */
    _cyhal_comp_ptc_cfg_init();

    /* Validate pins. vin_p and vin_m are mandatory pins, vout is not supported. */
    CY_UNUSED_PARAMETER(output);
    if (NC != output) /* vin_p/vin_m being NC will be handled in the NULL check below */
    {
        result = CYHAL_COMP_RSLT_ERR_INVALID_PIN;
    }

    /* Both vplus and vminus choose from the same set of pins */
    const cyhal_resource_pin_mapping_t *vin_p_map  = (NC != vin_p)     ? _CYHAL_UTILS_GET_RESOURCE(vin_p, cyhal_pin_map_pass_ptc_pads)     : NULL;
    const cyhal_resource_pin_mapping_t *vin_m_map  = (NC != vin_m)     ? _CYHAL_UTILS_GET_RESOURCE(vin_m, cyhal_pin_map_pass_ptc_pads)     : NULL;

    /* Verify if mapping successful */
    if ((NULL == vin_p_map) || (NULL == vin_m_map))
    {
        result = CYHAL_COMP_RSLT_ERR_INVALID_PIN;
    }

    /* The channel field is reused to convey the PTC input index that the pin is connected to.
     * So only compare the block numbers */
    if ((CY_RSLT_SUCCESS == result) && (vin_p_map->block_num != vin_m_map->block_num))
    {
        result = CYHAL_COMP_RSLT_ERR_INVALID_PIN;
    }

    /* Reserve resources */
    cyhal_resource_inst_t rsc;
    if(CY_RSLT_SUCCESS == result)
    {
        /* There are two comparators per PTC, and all can access the same pins. The
        * channel field in the pin package is reused to convey the ptc input index
        * that the pin is connected to.
        * So attempt to use channel 0; if that does not work, try to use channel 1 */
        rsc = (cyhal_resource_inst_t) { CYHAL_RSC_PTC, vin_p_map->block_num, 0u };
        result = cyhal_hwmgr_reserve(&rsc);
        if(CY_RSLT_SUCCESS != result)
        {
            rsc.channel_num = 1u;
            result = cyhal_hwmgr_reserve(&rsc);
        }
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->resource = rsc;
        result = _cyhal_utils_reserve_and_connect(vin_p_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_PTC_PADS);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->pin_vin_p = vin_p;
        obj->ptc_input_plus = _cyhal_comp_ptc_convert_input(vin_p_map->channel_num);
        result = _cyhal_utils_reserve_and_connect(vin_m_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_PTC_PADS);
    }

    if(CY_RSLT_SUCCESS == result)
    {
        obj->pin_vin_m = vin_m;
        obj->ptc_input_minus = _cyhal_comp_ptc_convert_input(vin_m_map->channel_num);
        /* Configure the PTC */
        /* The configuration is comingled between both comparators within a PTC, so all
         * of the configuration functions operate on the structs in the config array */
        obj->ptc_hysteresis_enabled = _cyhal_comp_ptc_convert_hysteresis(cfg->hysteresis);
        obj->ptc_power = _cyhal_comp_ptc_convert_power(cfg->power);
        int flat_index = _cyhal_comp_ptc_get_flat_index(obj->resource.block_num, obj->resource.channel_num);
        _cyhal_comp_ptc_config_structs[flat_index] = obj;

        cy_stc_autanalog_ptcomp_comp_sta_t staticConfig;
        cy_stc_autanalog_ptcomp_comp_dyn_t dynCfg[_CYHAL_COMP_PER_PTC];

        memset(dynCfg, 0, sizeof(dynCfg));
        _cyhal_comp_ptc_populate_dynamic_configs(obj->resource.block_num, dynCfg);

        _cyhal_comp_ptc_get_static_config(obj->resource.block_num, &staticConfig);
        cy_stc_autanalog_ptcomp_t pdl_cfg =
        {
            .ptcompStaCfg    = &staticConfig,
            .ptcompDynCfgNum = sizeof(dynCfg) / sizeof(dynCfg[0]),
            .ptcompDynCfgArr = dynCfg
        };

        result = _cyhal_comp_ptc_init_hw(obj, &pdl_cfg);
    }

    /* Free PTC in case of failure */
    if (result != CY_RSLT_SUCCESS)
    {
        _cyhal_comp_ptc_free(obj);
    }
    return result;
}

cy_rslt_t _cyhal_comp_ptc_init_cfg(cyhal_comp_t *obj, const cyhal_comp_configurator_t *cfg)
{
    /* The resource assignment is handled by generic cyhal_comp layer */
    // Save these for future updates of other PTC instances
    cy_stc_autanalog_ptcomp_comp_sta_t* static_cfg = cfg->ptc->ptcompStaCfg;
    bool isComp0 = (0u == cfg->resource->channel_num);
    obj->ptc_hysteresis_enabled = isComp0 ? static_cfg->compHystComp0 : static_cfg->compHystComp1;
    obj->ptc_power = isComp0 ? static_cfg->powerModeComp0 : static_cfg->powerModeComp1;
    cy_rslt_t result = _cyhal_comp_ptc_init_hw(obj, cfg->ptc);
    if(CY_RSLT_SUCCESS != result)
    {
        _cyhal_comp_ptc_free(obj);
    }

    return result;
}

void _cyhal_comp_ptc_free(cyhal_comp_t *obj)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(_cyhal_comp_ptc_arrays_initialized); /* Should not be freeing if we never initialized anything */

    if(CYHAL_RSC_INVALID != obj->resource.type)
    {
        _cyhal_analog_free();

        _cyhal_comp_ptc_config_structs[_cyhal_comp_ptc_get_flat_index(obj->resource.block_num, obj->resource.channel_num)] = NULL;

        _cyhal_comp_reload_config(obj->resource.block_num);

        if(false == obj->owned_by_configurator)
        {
            cyhal_hwmgr_free(&(obj->resource));
        }
        obj->resource.type = CYHAL_RSC_INVALID;
    }

    _cyhal_utils_release_if_used(&(obj->pin_vin_p));
    _cyhal_utils_release_if_used(&(obj->pin_vin_m));
}

cy_rslt_t _cyhal_comp_ptc_set_power(cyhal_comp_t *obj, cyhal_power_level_t power)
{
    CY_ASSERT(NULL != obj);
    obj->ptc_power = _cyhal_comp_ptc_convert_power(power);
    return _cyhal_comp_reload_config(obj->resource.block_num);
}

cy_rslt_t _cyhal_comp_ptc_configure(cyhal_comp_t *obj, cyhal_comp_config_t *cfg)
{
    CY_ASSERT(NULL != obj);
    obj->ptc_hysteresis_enabled = _cyhal_comp_ptc_convert_hysteresis(cfg->hysteresis);
    obj->ptc_power = _cyhal_comp_ptc_convert_power(cfg->power);

    return _cyhal_comp_reload_config(obj->resource.block_num);
}

bool _cyhal_comp_ptc_read(cyhal_comp_t *obj)
{
    CY_ASSERT(NULL != obj);

    return Cy_AutAnalog_PTComp_Read(obj->resource.block_num, obj->resource.channel_num);
}

void _cyhal_comp_ptc_enable_event(cyhal_comp_t *obj, cyhal_comp_event_t event, uint8_t intr_priority, bool enable)
{
    _cyhal_lppass_set_irq_priority(intr_priority);
    bool existing_events_enabled = (0u != obj->irq_cause);
    if(enable)
    {
        obj->irq_cause |= (uint32_t)event;
    }
    else
    {
        obj->irq_cause &= ~((uint32_t)event);
    }

    /* There are not separate events for different edge types, so there's no need to do the
     * usual process of figuring out what events were new. We only care about whether we're
     * changing from no events enabled to some events enabled. That is the only case in which
     * this function can cause a stale interrupt to fire if not cleared. */
    if(enable && false == existing_events_enabled)
    {
        Cy_AutAnalog_ClearInterrupt(_cyhal_comp_ptc_get_interrupt_mask(&(obj->resource)));
    }
    _cyhal_comp_reload_config(obj->resource.block_num);
}

#if defined(__cplusplus)
}
#endif

#endif /* _CYHAL_DRIVER_AVAILABLE_COMP_PTC */
