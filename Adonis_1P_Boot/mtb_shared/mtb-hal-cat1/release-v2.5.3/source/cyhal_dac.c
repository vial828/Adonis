/*******************************************************************************
* \file cyhal_dac.c
*
* \brief
* Provides a high level interface for interacting with the Infineon Digital/Analog converter.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low level functions
* can be used directly.
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

#include <limits.h>
#include <math.h>
#include <string.h> // For memset

#include "cyhal_analog_common.h"
#if defined(COMPONENT_CAT1D)
#include "cyhal_lppass_common.h"
#endif
#include "cyhal_dac.h"
#include "cyhal_gpio.h"
#include "cyhal_hwmgr.h"
#include "cyhal_utils.h"
#include "cyhal_syspm.h"
#include "cy_pdl.h"

/**
* \addtogroup group_hal_impl_dac DAC (Digital to Analog Converter)
* \ingroup group_hal_impl
*
* \section group_hal_impl_dac_power Power Level Mapping
* The following table shows how the HAL-defined power levels map to the hardware-specific power levels
* when cyhal_dac uses output pin buffered mode (with opamp). Unbuffered mode only supports ON and OFF.
* | HAL Power Level                | Opamp Power Level   |
* | ------------------------------ | ------------------- |
* | @ref CYHAL_POWER_LEVEL_HIGH    | CY_CTB_POWER_HIGH   |
* | @ref CYHAL_POWER_LEVEL_MEDIUM  | CY_CTB_POWER_MEDIUM |
* | @ref CYHAL_POWER_LEVEL_LOW     | CY_CTB_POWER_LOW    |
* | @ref CYHAL_POWER_LEVEL_DEFAULT | CY_CTB_POWER_MEDIUM |
*
* cyhal_dac automatically choose between buffered and unbuffered mode by selecting pin.
* Unbuffered mode - dac pin, buffered - opamp pin.
* Buffered mode take care of reserving and configuring the opamp (OA0).
* If AREF voltage reference source is selected cyhal_dac takes care of reserving and configuring the opamp (OA1).
* By default cyhal_dac use VDDA voltage reference source. Use @ref cyhal_dac_set_reference() to change
* between @ref CYHAL_DAC_REF_VDDA and @ref CYHAL_DAC_REF_VREF voltage reference sources.
*
* \note When initializing the DAC via @ref cyhal_dac_init_cfg, if opamps are required (either for buffered output
* or for buffering the AREF output when @ref CYHAL_DAC_REF_VREF is used) then they must be separately configured
* via @ref cyhal_opamp_init_cfg before the DAC is initialized. However, once the DAC is initialized, the
* @ref cyhal_dac_set_power function will update the power mode for the opamp(s) in the same manner that it
* does for DAC instances initialized via @ref cyhal_dac_init.
* \note When the DAC has been initialized via @ref cyhal_dac_init_cfg, the @ref cyhal_dac_set_reference function
* is not supported and will return @ref CYHAL_DAC_RSLT_INVALID_CONFIGURATOR. This is because the @ref
* cyhal_dac_set_reference function needs to manipulate the configuration and routing for OA1, and in this scenario
* that configuration and routing is owned by the configurator.
*
* \section group_hal_impl_dac_lppass_restictions Restrictions for reference voltage inputs for LPPASS
* The new IP block LPPASS introduced on CAT1D devices allows a lot more flexibility when defining the 
* reference for the voltage input of the DAC driver. The voltage can be drived by: 
* * VDDA
* * VBGR
* * PRB0
* * PRB1
* * CTB0_OA0
* * CTB0_OA1
* * CTB1_OA0
* * CTB1_OA1
* Otaining the information of the reference voltage for the first 2 cases is straightforward, the same cannot
* be said for the PRB and OPAMP cases. Especially in the HAL context where each driver is initialized separately. 
* For this reason, the support for OPAMP has only been partially implemented in HAL even when initializing the driver 
* through the configurator. It is possible to only call cyhal_dac_write but not cyhal_dac_write_mv when using the 
* OpAmps outputs as reference voltage.
* In the case of PRB, obtaining the voltage information knowing its configuration is easy but for the first implementation
* of DAC that is limited to having knowledge of only its own settings this information cannot be retrieved. 
* Therefore selecting PRB as reference voltage input is possible but there is no support for cyhal_dac_write_mv .  
* If a user wishes to take advantage of these configurations, they will need to configure the analog drivers directly using the PDL.
*/


#if (CYHAL_DRIVER_AVAILABLE_DAC)

#if defined(__cplusplus)
extern "C"
{
#endif

#define _CYHAL_DAC_VALUE_SCALING_FACTOR (UINT16_MAX / CY_CTDAC_UNSIGNED_MAX_CODE_VALUE)
#define _CYHAL_DAC_WRITE_TOLERANCE      (5u)

#if defined(CY_IP_MXS22LPPASS)

/* Configuration of range conditions for DAC */
static cy_stc_autanalog_dac_ch_limit_t _cyhal_lppass_dac_ch_cfg =
{
    .cond = CY_AUTANALOG_DAC_CH_LIMIT_OUTSIDE,
    .low  = 0x0,
    .high = 0xFFFF,
};

static cy_stc_autanalog_dac_ch_t _cyhal_lppass_dac_channel_config =
{
    .startAddr     = 0U,                                /* start from beginning */
    .endAddr       = 1U,                                /* one value is used  */
    .operMode      = CY_AUTANALOG_DAC_LUT_OS_ONE_QUAD,  /* One shot one quadrant */
    .sampleAndHold = false,                             /* sample mode (with de-glitch) */
    .stepSel       = CY_AUTANALOG_DAC_STEP_SEL_DISABLED,   /* step value is 1 */
    .statSel       = CY_AUTANALOG_DAC_STATUS_SEL_DISABLED, /* not used and not configured */
};


static const cy_stc_autanalog_dac_sta_t _CYHAL_DAC_STATIC_CONFIG_DEFAULT =
{
    .lpDivDac = 0,
    .topology = CY_AUTANALOG_DAC_TOPO_BUFFERED_EXTERNAL, 
    .deGlitch = false, 
    .bottomSel = false,
    .disabledMode = false,
    .refBuffPwr = CY_AUTANALOG_DAC_BUF_PWR_LOW_RAIL,
    .outBuffPwr = CY_AUTANALOG_DAC_BUF_PWR_LOW_RAIL,
    .vrefSel = CY_AUTANALOG_DAC_VREF_VDDA,
    .vrefMux = CY_AUTANALOG_DAC_VREF_MUX_VBGR, /* Not used when vrefSel is VDDA */
    .sign = false,
    .sampleTime = 0,
    .stepVal[0] = 0,
    .deGlitchTime = 0,
    .chCfg[0] = &_cyhal_lppass_dac_channel_config,
    .chLimitCfg[0] = &_cyhal_lppass_dac_ch_cfg,
};

#else

static CTDAC_Type *const _cyhal_dac_base[] = {
#if (CY_IP_MXS40PASS_CTDAC_INSTANCES > 0)
    CTDAC0,
#endif
#if (CY_IP_MXS40PASS_CTDAC_INSTANCES > 1)
    CTDAC1,
#endif
#if (CY_IP_MXS40PASS_CTDAC_INSTANCES > 2)
#warning Unhandled CTDAC instance count
#endif
};

static const cy_stc_ctdac_config_t _CYHAL_DAC_DEFAULT_CONFIG =
{
    .refSource = CY_CTDAC_REFSOURCE_VDDA,
    .formatMode = CY_CTDAC_FORMAT_UNSIGNED,
    .updateMode = CY_CTDAC_UPDATE_DIRECT_WRITE,
    .deglitchMode = CY_CTDAC_DEGLITCHMODE_UNBUFFERED,
    .outputMode = CY_CTDAC_OUTPUT_VALUE,
    //.outputBuffer is configured automatically depending on pin choice
    .deepSleep = CY_CTDAC_DEEPSLEEP_ENABLE,
    .deglitchCycles = 0,
    .value = 0,
    .nextValue = 0,
    .enableInterrupt = true,
    .configClock = false,
    // The following values are simply placeholders because configClock is false
    .dividerType = CY_SYSCLK_DIV_8_BIT,
    .dividerNum = 0,
    .dividerIntValue = 0,
    .dividerFracValue = 0,
};

#if defined(CY_IP_MXS40PASS_CTB)
static const cy_stc_ctb_opamp_config_t cyhal_opamp_default_config =
{
    .deepSleep    = CY_CTB_DEEPSLEEP_ENABLE,
    .oaPower      = CY_CTB_POWER_MEDIUM,
    .oaMode       = CY_CTB_MODE_OPAMP1X,
    .oaPump       = CY_CTB_PUMP_ENABLE,
    .oaCompEdge   = CY_CTB_COMP_EDGE_DISABLE,
    .oaCompLevel  = CY_CTB_COMP_DSI_TRIGGER_OUT_LEVEL,
    .oaCompBypass = CY_CTB_COMP_BYPASS_SYNC,
    .oaCompHyst   = CY_CTB_COMP_HYST_DISABLE,
    .oaCompIntrEn = true,
};

/* We can safely assume these indices even if we're owned by a configurator, because
 * the hardware does not support any other connections to the vout and ref in terminals */
static const uint8_t OPAMP_IDX_OUTPUT = 0;
static const uint8_t OPAMP_IDX_REF    = 1;
#endif

#if defined(CY_IP_MXS40PASS_CTB)
static bool _cyhal_dac_is_output_buffered(const cyhal_dac_t *obj)
{
    /* C06 enables the voutsw terminal on the CTDAC block, which is hard-wired to a pin */
    return (0u == (obj->base_dac->CTDAC_SW & CTDAC_CTDAC_SW_CTDO_CO6_Msk));
}
#endif /* defined(CY_IP_MXS40PASS_CTB) */

static bool _cyhal_dac_is_external_reference(const cyhal_dac_t *obj)
{
    /* CVD connects the DAC reference input to VDDA. It will be opened if the DAC is driven
     * by an external reference (buffered through OA1) instead */
    return (0u == (obj->base_dac->CTDAC_SW & CTDAC_CTDAC_SW_CTDD_CVD_Msk));
}

static uint32_t _cyhal_dac_convert_reference(cyhal_dac_ref_t ref)
{
    switch(ref)
        {
            case CYHAL_DAC_REF_VDDA:
                return CY_CTDAC_REFSOURCE_VDDA;
            case CYHAL_DAC_REF_VREF:
                return CY_CTDAC_REFSOURCE_EXTERNAL;
            default:
                CY_ASSERT(false);
                return CY_CTDAC_REFSOURCE_VDDA;
        }
}

#endif /* defined(CY_IP_MXS22LPPASS) */

#if defined(CY_IP_MXS40PASS_CTB)
static cy_rslt_t _cyhal_dac_configure_oa0(cyhal_dac_t *obj, bool init)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    CY_ASSERT(false == obj->owned_by_configurator);
    if (init && (CYHAL_RSC_INVALID != obj->resource_opamp.type))
    {
        /* Configure OA0 for buffered output */
        /* OA0 require non defaut CY_CTB_MODE_OPAMP10X */
        cy_stc_ctb_opamp_config_t config = cyhal_opamp_default_config;
        config.oaMode = CY_CTB_MODE_OPAMP10X;
        result = Cy_CTB_OpampInit(obj->base_opamp, CY_CTB_OPAMP_0, &config);
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_OA0_SW, CY_CTB_SW_OA0_NEG_OUT_MASK | CY_CTB_SW_OA0_OUT_SHORT_1X_10X_MASK, CY_CTB_SWITCH_CLOSE);
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_CTD_SW, CY_CTB_SW_CTD_OUT_CHOLD_MASK | CY_CTB_SW_CTD_CHOLD_OA0_POS_MASK, CY_CTB_SWITCH_CLOSE);
        cyhal_analog_ctb_init(obj->base_opamp);
    }
    else
    {
        /* Open switches OA0 if used */
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_OA0_SW, CY_CTB_SW_OA0_NEG_OUT_MASK | CY_CTB_SW_OA0_OUT_SHORT_1X_10X_MASK, CY_CTB_SWITCH_OPEN);
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_CTD_SW, CY_CTB_SW_CTD_OUT_CHOLD_MASK | CY_CTB_SW_CTD_CHOLD_OA0_POS_MASK, CY_CTB_SWITCH_OPEN);
        cyhal_analog_ctb_free(obj->base_opamp);
    }
    return result;
}

static cy_rslt_t _cyhal_dac_configure_oa1(cyhal_dac_t *obj, bool init)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    CY_ASSERT(false == obj->owned_by_configurator);

    if (init && (CYHAL_RSC_INVALID != obj->resource_aref_opamp.type))
    {
        /* Configure OA1 for buffered (AREF) voltage reference source */
        result = Cy_CTB_OpampInit(obj->base_opamp, CY_CTB_OPAMP_1, &cyhal_opamp_default_config);
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_OA1_SW, CY_CTB_SW_OA1_NEG_OUT_MASK | CY_CTB_SW_OA1_POS_AREF_MASK, CY_CTB_SWITCH_CLOSE);
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_CTD_SW, CY_CTB_SW_CTD_REF_OA1_OUT_MASK, CY_CTB_SWITCH_CLOSE);
        cyhal_analog_ctb_init(obj->base_opamp);
    }
    else
    {
        /* Open switches OA1 if used */
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_OA1_SW, CY_CTB_SW_OA1_NEG_OUT_MASK | CY_CTB_SW_OA1_POS_AREF_MASK, CY_CTB_SWITCH_OPEN);
        Cy_CTB_SetAnalogSwitch(obj->base_opamp, CY_CTB_SWITCH_CTD_SW, CY_CTB_SW_CTD_REF_OA1_OUT_MASK, CY_CTB_SWITCH_OPEN);
        cyhal_analog_ctb_free(obj->base_opamp);
    }
    return result;
}
#endif

#if defined(CY_IP_MXS22LPPASS)
uint32_t _cyhal_dac_convert_power(cyhal_power_level_t hal_power)
{
    switch(hal_power)
    {
        case CYHAL_POWER_LEVEL_OFF:
            return (uint32_t)
                CY_AUTANALOG_DAC_BUF_PWR_OFF;
        case CYHAL_POWER_LEVEL_LOW:
            return (uint32_t)
                CY_AUTANALOG_DAC_BUF_PWR_LOW_RAIL;
        case CYHAL_POWER_LEVEL_MEDIUM:
        case CYHAL_POWER_LEVEL_DEFAULT:
            return (uint32_t)
                CY_AUTANALOG_DAC_BUF_PWR_MEDIUM_RAIL;
        case CYHAL_POWER_LEVEL_HIGH:
            return (uint32_t)
                CY_AUTANALOG_DAC_BUF_PWR_HIGH_RAIL;
        default:
            return (uint32_t)
                CY_AUTANALOG_DAC_BUF_PWR_MEDIUM_RAIL;
    }
}

#endif
/*******************************************************************************
*       DAC HAL Functions
*******************************************************************************/
cy_rslt_t _cyhal_dac_init_hw(cyhal_dac_t *obj, const cyhal_dac_configurator_t *cfg)
{
    #if defined(CY_IP_MXS22LPPASS_INSTANCES)

    cy_rslt_t result = CY_RSLT_SUCCESS;
        
    if (CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_analog_init();
    }
    
    if (CY_RSLT_SUCCESS == result)
    {
        obj->analog_init_completed = true;
        result = (cy_rslt_t)Cy_AutAnalog_DAC_LoadStaticConfig(obj->resource_dac.block_num, cfg->static_cfg); 
    }

    #else 
    obj->base_dac = _cyhal_dac_base[obj->resource_dac.block_num];
    #if (_CYHAL_DRIVER_AVAILABLE_COMP_CTB)
    obj->base_opamp = _cyhal_ctb_base[obj->resource_dac.block_num];
    #endif // _CYHAL_DRIVER_AVAILABLE_COMP_CTB
    cy_rslt_t result = (cy_rslt_t)Cy_CTDAC_Init(obj->base_dac, cfg->config);

    /* We deliberately don't initialize the opamp(s), if any, here. In the configurator
     * flow, these are initialized by the application via separate calls to
     * cyhal_opamp_init_cfg */

    if (CY_RSLT_SUCCESS == result)
    {
        _cyhal_analog_init();
        Cy_CTDAC_Enable(obj->base_dac);
    }
    #endif

    return result;
}

cy_rslt_t cyhal_dac_init(cyhal_dac_t *obj, cyhal_gpio_t pin)
{
    CY_ASSERT(NULL != obj);

    /* Initial values */
    cy_rslt_t result = CY_RSLT_SUCCESS;
    memset(obj, 0, sizeof(cyhal_dac_t));
    obj->resource_dac.type = CYHAL_RSC_INVALID;
    obj->pin = CYHAL_NC_PIN_VALUE;
#if !(CY_IP_MXS22LPPASS)
    obj->resource_opamp.type = CYHAL_RSC_INVALID;
    obj->resource_aref_opamp.type = CYHAL_RSC_INVALID;
#endif
    cyhal_dac_configurator_t config;

    const cyhal_resource_pin_mapping_t *opamp_map = NULL;

    #if defined(CYHAL_PIN_MAP_DRIVE_MODE_DAC_CTDAC_VOUTSW)
    const cyhal_resource_pin_mapping_t *dac_map = _CYHAL_UTILS_GET_RESOURCE(pin, cyhal_pin_map_dac_ctdac_voutsw);
    #elif defined(CYHAL_PIN_MAP_DRIVE_MODE_PASS_DAC_PADS)
    const cyhal_resource_pin_mapping_t *dac_map = _CYHAL_UTILS_GET_RESOURCE(pin, cyhal_pin_map_pass_dac_pads);
    #else
    const cyhal_resource_pin_mapping_t *dac_map = NULL;
    #endif

#if !(CY_IP_MXS22LPPASS)
    if (NULL == dac_map)
    {
        /* Try to get buffered output pin if unbuffered is not specified.  */
        #ifdef CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_OUT_10X
        opamp_map = _CYHAL_UTILS_GET_RESOURCE(pin, cyhal_pin_map_opamp_out_10x);
        #endif
    }
#endif

    /* Check if mapping is successful */
    if ((NULL == dac_map) && (NULL == opamp_map))
    {
        result = CYHAL_DAC_RSLT_BAD_ARGUMENT;
    }

#if defined(CY_IP_MXS40PASS_CTB)
    /* Verify if opamp instance 0 is selected, buffered output can be connected to OA0 */
    if ((NULL != opamp_map) && (OPAMP_IDX_OUTPUT != (opamp_map->channel_num)))
    {
        result = CYHAL_DAC_RSLT_BAD_ARGUMENT;
    }
#endif

    cyhal_resource_inst_t dac_instance;

#if !(CY_IP_MXS22LPPASS)
    cyhal_resource_inst_t opamp_instance;
#endif

#if defined(CY_IP_MXS40PASS_CTB)
    if (NULL != opamp_map)
    {
        dac_instance.type = CYHAL_RSC_DAC;
        dac_instance.block_num = opamp_map->block_num;
        dac_instance.channel_num = 0;
    }
    else if (CY_RSLT_SUCCESS == result)
    {
        _CYHAL_UTILS_ASSIGN_RESOURCE(dac_instance, CYHAL_RSC_DAC, dac_map);
    }
#endif
#if (CY_IP_MXS22LPPASS)
    if (CY_RSLT_SUCCESS == result)
    {
        _CYHAL_UTILS_ASSIGN_RESOURCE(dac_instance, CYHAL_RSC_DAC, dac_map);
    }
#endif
    if (CY_RSLT_SUCCESS == result)
    {
        result = cyhal_hwmgr_reserve(&dac_instance);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        obj->resource_dac = dac_instance;
    }

#if !(CY_IP_MXS22LPPASS) 
    if ((NULL != opamp_map) && (CY_RSLT_SUCCESS == result))
    {
        _CYHAL_UTILS_ASSIGN_RESOURCE(opamp_instance, CYHAL_RSC_OPAMP, opamp_map);
        result = cyhal_hwmgr_reserve(&opamp_instance);
        if (CY_RSLT_SUCCESS == result)
        {
            obj->resource_opamp = opamp_instance;
        }
    }
#endif

    if (CY_RSLT_SUCCESS == result)
    {
#if defined(CYHAL_PIN_MAP_DRIVE_MODE_DAC_CTDAC_VOUTSW)
        if (NULL != dac_map)
        {
            result = _cyhal_utils_reserve_and_connect(dac_map, CYHAL_PIN_MAP_DRIVE_MODE_DAC_CTDAC_VOUTSW);
        }
#elif defined(CYHAL_PIN_MAP_DRIVE_MODE_PASS_DAC_PADS)
        if (NULL != dac_map)
        {
            result = _cyhal_utils_reserve_and_connect(dac_map, CYHAL_PIN_MAP_DRIVE_MODE_PASS_DAC_PADS);
        }
#endif
#if defined(CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_OUT_10X) 
        if (NULL != opamp_map)
        {
            result = _cyhal_utils_reserve_and_connect(opamp_map, CYHAL_PIN_MAP_DRIVE_MODE_OPAMP_OUT_10X);
        }
#endif

        if (CY_RSLT_SUCCESS == result)
        {
            obj->pin = pin;
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
#if !(CY_IP_MXS22LPPASS)        
        /* Verify is output buffered or not */
        cy_stc_ctdac_config_t cfg;
        memset(&cfg, 0, sizeof(cy_stc_ctdac_config_t));
        memset(&config, 0, sizeof(cyhal_dac_configurator_t));
        cfg = _CYHAL_DAC_DEFAULT_CONFIG;
        cfg.outputBuffer = (obj->resource_opamp.type != CYHAL_RSC_INVALID) ? CY_CTDAC_OUTPUT_BUFFERED : CY_CTDAC_OUTPUT_UNBUFFERED;
        config.config = &cfg;
#else
        /* Configure the lppass*/
        if(CY_RSLT_SUCCESS == result)
        {
            //Populate a configurator object with the needed elements
            cy_stc_autanalog_dac_sta_t cfg;
            memset(&cfg, 0, sizeof(cy_stc_autanalog_dac_sta_t));
            memset(&config, 0, sizeof(cyhal_dac_configurator_t));
            config.resource = NULL;
            config.static_cfg = &_CYHAL_DAC_STATIC_CONFIG_DEFAULT;            
        }

#endif
        result = _cyhal_dac_init_hw(obj, &config);
    }

#if defined(CY_IP_MXS40PASS_CTB)
    if ((CY_RSLT_SUCCESS == result) && (obj->resource_opamp.type != CYHAL_RSC_INVALID))
    {
        /* Init OA0 for buffered output, don't touch OA1 it may be used by opamp or comp */
        result = _cyhal_dac_configure_oa0(obj, true);
    }
#endif

#if (CY_IP_MXS22LPPASS)
    if(result == CY_RSLT_SUCCESS)
    {
        obj->static_cfg = *(config.static_cfg);
    }
#endif

    if(CY_RSLT_SUCCESS != result)
    {
        /* Freeup resources in case of failure */
        cyhal_dac_free(obj);
    }
    return result;
}

 cy_rslt_t cyhal_dac_init_cfg(cyhal_dac_t *obj, const cyhal_dac_configurator_t *cfg)
 {
    memset(obj, 0, sizeof(cyhal_dac_t));
    obj->owned_by_configurator = true;
    obj->resource_dac = *cfg->resource;
#if defined(CY_IP_MXS40PASS_CTB)
    obj->resource_opamp.type = CYHAL_RSC_INVALID;
    obj->resource_aref_opamp.type = CYHAL_RSC_INVALID;
#endif
    obj->pin = CYHAL_NC_PIN_VALUE;
    cy_rslt_t result = _cyhal_dac_init_hw(obj, cfg);
#if (CY_IP_MXS22LPPASS)
    if(CY_RSLT_SUCCESS == result)
    {
        obj->static_cfg = *(cfg->static_cfg);
    }
#endif
    if(CY_RSLT_SUCCESS != result)
    {
        cyhal_dac_free(obj);
    }
    return result;
}

void cyhal_dac_free(cyhal_dac_t *obj)
{
#if (CY_IP_MXS40PASS)
    if (NULL != obj->base_dac)
    {
#if defined(CY_IP_MXS40PASS_CTB)
        /* Power off OA1 if used */
        if (_cyhal_dac_is_external_reference(obj))
        {
            Cy_CTB_SetPower(obj->base_opamp, _cyhal_opamp_convert_sel(OPAMP_IDX_REF), (cy_en_ctb_power_t)_cyhal_opamp_convert_power(CYHAL_POWER_LEVEL_OFF), CY_CTB_PUMP_ENABLE);
            if(false == obj->owned_by_configurator)
            {
                (void)_cyhal_dac_configure_oa1(obj, false);
            }
        }

        /* Power off OA0 if used */
        if (_cyhal_dac_is_output_buffered(obj))
        {
            Cy_CTB_SetPower(obj->base_opamp, _cyhal_opamp_convert_sel(OPAMP_IDX_OUTPUT), (cy_en_ctb_power_t)_cyhal_opamp_convert_power(CYHAL_POWER_LEVEL_OFF), CY_CTB_PUMP_ENABLE);
            if(false == obj->owned_by_configurator)
            {
                (void)_cyhal_dac_configure_oa0(obj, false);
            }
        }
#endif   

        _cyhal_analog_free();

        Cy_CTDAC_Disable(obj->base_dac);

        if(false == obj->owned_by_configurator)
        {
            cyhal_hwmgr_free(&obj->resource_dac);
            if(CYHAL_RSC_INVALID != obj->resource_opamp.type)
            {
                cyhal_hwmgr_free(&obj->resource_opamp);
            }
            if(CYHAL_RSC_INVALID != obj->resource_aref_opamp.type)
            {
                cyhal_hwmgr_free(&obj->resource_aref_opamp);
            }

            _cyhal_utils_release_if_used(&(obj->pin));
        }

        obj->base_dac = NULL;
        obj->base_opamp = NULL;
    }
#elif (CY_IP_MXS22LPPASS)
        if(obj->analog_init_completed == true)
        {
            _cyhal_analog_free();
        }
        if(false == obj->owned_by_configurator)
        {
            cyhal_hwmgr_free(&obj->resource_dac);
        }
        _cyhal_utils_release_if_used(&(obj->pin));
#endif
}

void cyhal_dac_write(const cyhal_dac_t *obj, uint16_t value)
{
    #if defined(CY_IP_MXS22LPPASS)
    obj->static_cfg.chLimitCfg[0]->low = value - _CYHAL_DAC_WRITE_TOLERANCE;
    obj->static_cfg.chLimitCfg[0]->high = value + _CYHAL_DAC_WRITE_TOLERANCE;

    cy_rslt_t result = (cy_rslt_t) Cy_AutAnalog_DAC_LoadStaticConfig(obj->resource_dac.block_num, &obj->static_cfg);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    CY_UNUSED_PARAMETER(result);

    Cy_AutAnalog_DAC_Data_WriteSample(obj->resource_dac.block_num, 0, value);
    #else
    uint16_t scaled_value = value / _CYHAL_DAC_VALUE_SCALING_FACTOR;
    Cy_CTDAC_SetValue(obj->base_dac, scaled_value);
    #endif
}

cy_rslt_t cyhal_dac_write_mv(const cyhal_dac_t *obj, uint16_t value)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t reference_voltage_mv = 0;

#if defined(CY_IP_MXS22LPPASS)

    if(obj->static_cfg.vrefSel == CY_AUTANALOG_DAC_VREF_VDDA)
    {
        /* VDDA voltage in millivolts */
        reference_voltage_mv = cyhal_syspm_get_supply_voltage(CYHAL_VOLTAGE_SUPPLY_VDDA);
    }
    else if (obj->static_cfg.vrefSel == CY_AUTANALOG_DAC_VREF_MUX_OUT)
    {
        if(obj->static_cfg.vrefMux == CY_AUTANALOG_DAC_VREF_MUX_VBGR)
        {
            /* VBGR voltage in mV*/
            reference_voltage_mv = _CYHAL_LPPASS_VBGR_VALUE;
        }
        /* In case of configurator object we might get a reference from the PRB
        we need a way to access the configuration of the PRBs to obtain the values. 
        It would look something like this but with no way to currently access the 
        config other than maintaining global copies this needs to be revised.*/

        // else if(obj->static_cfg.vrefMux == CY_AUTANALOG_DAC_VREF_MUX_PRB_VREF0 )
        // {
        //     uint32_t vref_value = _cyhal_lppass_prb_convert_src(_cyhal_autanalog_cfg.prb[0].src);
        // uint32_t multiplier = 0;
        // /* Based on the prbVref0Fw use the value stored in the configuration or in the 
        //  STT directly.  */
        // if(_cyhal_lppass_stt.prb.prbVref0Fw)
        // {
        //     multiplier = _cyhal_lppass_prb_convert_tap(_cyhal_autanalog_cfg.prb[0].tap);
        // }
        // else
        // {
        //     multiplier = _cyhal_lpppass_prb_convert_tap(_cyhal_lppass_stt.prb.prbVref0Tap);
        // }
        //     reference_voltage_mv = (vref_value * multiplier) / _CYHAL_PRB_TAP_DIVIDER;
        // }
        // else if(obj->static_cfg.vrefMux ==  CY_AUTANALOG_DAC_VREF_MUX_PRB_VREF1 )
        // {
        //     uint32_t vref_value = _cyhal_prb_convert_src(_cyhal_autanalog_cfg.prb[1].src);
        // uint32_t multiplier = 0;
        // /* Based on the prbVref0Fw use the value stored in the configuration or in the 
        //  STT directly.  */
        // if(_cyhal_lppass_stt.prb.prbVref0Fw)
        // {
        //     multiplier = _cyhal_lppass_prb_convert_tap(_cyhal_autanalog_cfg.prb[1].tap);
        // }
        // else
        // {
        //     multiplier = _cyhal_lppass_prb_convert_tap(_cyhal_lppass_stt.prb.prbVref1Tap);
        // }
        //     reference_voltage_mv = (vref_value * multiplier) / _CYHAL_PRB_TAP_DIVIDER;
        // }
        // For the first implementation let's just return error in case of reference voltage
        // different from VDDA or VBGR.
        else
        {
            result = CYHAL_DAC_RSLT_BAD_REF_VOLTAGE;
        }
    }

    uint32_t count =  (value << 12) / reference_voltage_mv;

    cyhal_dac_write(obj, count);

#else

    if (_cyhal_dac_is_external_reference(obj))
    {
        reference_voltage_mv = cyhal_syspm_get_supply_voltage(CYHAL_VOLTAGE_SUPPLY_VDDA);

        if (0 == reference_voltage_mv)
        {
            result = CYHAL_DAC_RSLT_BAD_REF_VOLTAGE;
        }
    }
    else
    {
        /* AREF voltage in millivolts */
        reference_voltage_mv = 1200;
    }

    if (result == CY_RSLT_SUCCESS)
    {
        uint32_t count =  (value << 12) / reference_voltage_mv;
        Cy_CTDAC_SetValue(obj->base_dac, count);
    }
#endif
    return result;
}

uint16_t cyhal_dac_read(cyhal_dac_t *obj)
{
#if !defined(CY_IP_MXS22LPPASS)
    uint16_t value = (uint16_t)obj->base_dac->CTDAC_VAL;
    uint16_t scaled_value = value * _CYHAL_DAC_VALUE_SCALING_FACTOR;
    return scaled_value;
#else
    uint16_t value = (uint16_t)Cy_AutAnalog_DAC_Data_ReadSample(obj->resource_dac.block_num , 0);
    return value;
#endif
    
}

cy_rslt_t cyhal_dac_set_reference(cyhal_dac_t *obj, cyhal_dac_ref_t ref)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

#if !defined(CY_IP_MXS22LPPASS)
    if(false == obj->owned_by_configurator)
    {
        if (CYHAL_DAC_REF_VDDA == ref)
        {
#if defined(CY_IP_MXS40PASS_CTB) /* If no opamps we just need to check that ref is VDDA, the only supported value */
            /* Unreserve OA1, not needed for VDDA */
            if (obj->resource_aref_opamp.type != CYHAL_RSC_INVALID)
            {
                cyhal_hwmgr_free(&obj->resource_aref_opamp);
                obj->resource_aref_opamp.type = CYHAL_RSC_INVALID;

                /* Freeup OA1. Not needed when VDDA reference is set  */
                result = _cyhal_dac_configure_oa1(obj, false);
            }
        }
        else if (CYHAL_DAC_REF_VREF == ref)
        {
            if (obj->resource_aref_opamp.type == CYHAL_RSC_INVALID)
            {
                /* Reserve OA1 to be able connect to AREF voltage source */
                obj->resource_aref_opamp.type = CYHAL_RSC_OPAMP;
                obj->resource_aref_opamp.block_num = obj->resource_dac.block_num;
                obj->resource_aref_opamp.channel_num = OPAMP_IDX_REF;

                result = cyhal_hwmgr_reserve(&obj->resource_aref_opamp);
                if (CY_RSLT_SUCCESS != result)
                {
                    obj->resource_aref_opamp.type = CYHAL_RSC_INVALID;
                }
                else
                {
                    /* Init OA1 to be able connect to AREF voltage source. OA0 is untouched */
                    result = _cyhal_dac_configure_oa1(obj, true);
                }
            }
#endif
        }
        else
        {
            result = CYHAL_DAC_RSLT_BAD_REF_VOLTAGE;
        }

        if (result == CY_RSLT_SUCCESS)
        {
            Cy_CTDAC_SetRef(obj->base_dac, (cy_en_ctdac_ref_source_t)_cyhal_dac_convert_reference(ref));
        }
    }
    else
    {
        /* We don't own the configuration and routing of OA1, so we can't init/free it and open/close
         * switches to it, as would be required to change the reference */
        result = CYHAL_DAC_RSLT_INVALID_CONFIGURATOR;
    }

    return result;
#else
        if (CYHAL_DAC_REF_VDDA == ref)
        {
            obj->static_cfg.vrefSel = CY_AUTANALOG_DAC_VREF_VDDA;
        }
        else if (CYHAL_DAC_REF_VREF == ref)
        {
            obj->static_cfg.vrefSel = CY_AUTANALOG_DAC_VREF_MUX_OUT; 
            obj->static_cfg.vrefMux = CY_AUTANALOG_DAC_VREF_MUX_VBGR;
        }
        else
        {
            result = CYHAL_DAC_RSLT_BAD_REF_VOLTAGE;
        }
    if (result == CY_RSLT_SUCCESS)
    {
        result = (cy_rslt_t)Cy_AutAnalog_DAC_LoadStaticConfig(obj->resource_dac.block_num, &obj->static_cfg);
    }
    return result;
#endif
}

cy_rslt_t cyhal_dac_set_power(cyhal_dac_t *obj, cyhal_power_level_t power)
{
#if defined(CY_IP_MXS22LPPASS)
    /* Safe convert power level from HAL (cyhal_power_level_t) to corresponding PDL value */
    uint32_t converted_power_level = _cyhal_dac_convert_power(power);
    obj->static_cfg.refBuffPwr = (cy_en_autanalog_dac_buf_pwr_t)converted_power_level;
    obj->static_cfg.outBuffPwr = (cy_en_autanalog_dac_buf_pwr_t)converted_power_level;
    cy_rslt_t result = (cy_rslt_t)Cy_AutAnalog_DAC_LoadStaticConfig(obj->resource_dac.block_num, &obj->static_cfg);
    return result;
#else
#if defined(CY_IP_MXS40PASS_CTB)
    if (_cyhal_dac_is_output_buffered(obj) || _cyhal_dac_is_external_reference(obj))
    {
        /* Safe convert power level from HAL (cyhal_power_level_t) to PDL (cy_en_ctb_power_t) */
        cy_en_ctb_power_t power_level = (cy_en_ctb_power_t)_cyhal_opamp_convert_power(power);
        if(_cyhal_dac_is_output_buffered(obj))
        {
            Cy_CTB_SetPower(obj->base_opamp, _cyhal_opamp_convert_sel(OPAMP_IDX_OUTPUT), power_level, CY_CTB_PUMP_ENABLE);
        }
        if(_cyhal_dac_is_external_reference(obj))
        {
            Cy_CTB_SetPower(obj->base_opamp, _cyhal_opamp_convert_sel(OPAMP_IDX_REF), power_level, CY_CTB_PUMP_ENABLE);
        }

        bool full_ctb_owned = _cyhal_dac_is_output_buffered(obj) || _cyhal_dac_is_external_reference(obj);
        if(full_ctb_owned)
        {
            if (CYHAL_POWER_LEVEL_OFF == power)
            {
                Cy_CTB_Disable(obj->base_opamp);
            }
            else
            {
                Cy_CTB_Enable(obj->base_opamp);
            }
        }
    }
#endif
    if (CYHAL_POWER_LEVEL_OFF == power)
    {
        Cy_CTDAC_Disable(obj->base_dac);
    }
    else
    {
        Cy_CTDAC_Enable(obj->base_dac);
    }
    return CY_RSLT_SUCCESS;
#endif
}

#if defined(__cplusplus)
}
#endif

#endif /* CYHAL_DRIVER_AVAILABLE_DAC */
