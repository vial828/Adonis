/***************************************************************************//**
* \file cy_graphics.c
* \version 1.0
*
* Provides an API implementation for Graphics Driver
*
********************************************************************************
* \copyright
* Copyright 2021-2022 Cypress Semiconductor Corporation
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

#include "cy_device.h"
#include <string.h>
#if defined(CY_IP_MXS22GFXSS)

#include "cy_graphics.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define CLEAR_INTERRUPT      (0xFFFFFFFF)
#define GPU_IDLE_MASK        (0x00000b05)
#define CY_GFXSS_WAIT_1_UNIT             (1U)
#define CY_GFXSS_TOTAL_NO_OF_RETRIES     (100U)
#define CLK_SYS_FREQ          (100U)

static void cy_gfxss_update_display_rect_params(viv_display_size_type display_size, viv_dc_rect *dc_rect);
static uint8_t cy_gfxss_get_bpp_from_format(viv_input_format_type        format);
static uint32_t cy_gfxss_dc_init(GFXSS_Type *base, cy_stc_gfx_dc_config_t *config, cy_stc_mipidsi_config_t *mipi_dsi_cfg);


static void cy_gfxss_update_display_rect_params(viv_display_size_type display_size, viv_dc_rect *dc_rect)
{

    dc_rect->x = 0;
    dc_rect->y = 0;

    switch(display_size)
    {
        case vivDISPLAY_320_480_60:
            dc_rect->w = 320;
            dc_rect->h = 480;
            break;
    
        case vivDISPLAY_480_800_60:
            dc_rect->w = 480;
            dc_rect->h = 800;
            break;
    
        case vivDISPLAY_480_864_60:
            dc_rect->w = 480;
            dc_rect->h = 864;
            break;
    
        case vivDISPLAY_640_480_60:
            dc_rect->w = 640;
            dc_rect->h = 480;
            break;
    
        case vivDISPLAY_720_480_60:
            dc_rect->w = 720;
            dc_rect->h = 480;
            break;
    
        case vivDISPLAY_800_480_60:
            dc_rect->w = 800;
            dc_rect->h = 480;
            break;
    
        case vivDISPLAY_1024_600_60:
            dc_rect->w = 1024;
            dc_rect->h = 600;
            break;
    
        case vivDISPLAY_1024_768_60:
            dc_rect->w = 1024;
            dc_rect->h = 768;
            break;
    
        case vivDISPLAY_1280_720_60:
            dc_rect->w = 1280;
            dc_rect->h = 720;
            break;
    
        case vivDISPLAY_1920_1080_60:
            dc_rect->w = 1920;
            dc_rect->h = 1080;
            break;
    
        default:
            dc_rect->w = 0;
            dc_rect->h = 0;
        }
}

/** Get number of bits per pixel from the given display format */
static uint8_t cy_gfxss_get_bpp_from_format(viv_input_format_type format)
{
    uint8_t bpp = 0;

    switch(format)
    {
      case vivARGB4444:
      case vivABGR4444:
      case vivRGBA4444:
      case vivBGRA4444:
      case vivXRGB4444:
      case vivXBGR4444:
      case vivRGBX4444:
      case vivBGRX4444:
      case vivARGB1555:
      case vivABGR1555:
      case vivRGBA1555:
      case vivBGRA1555:
      case vivXRGB1555:
      case vivXBGR1555:
      case vivRGBX1555:
      case vivBGRX1555:
      case vivRGB565:
      case vivBGR565:
           {
               bpp = 16;
           }
           break;
      case vivARGB8888:
      case vivABGR8888:
      case vivRGBA8888:
      case vivBGRA8888:
      case vivXRGB8888:
      case vivXBGR8888:
      case vivRGBX8888:
      case vivBGRX8888:
      case vivARGB2101010:
           {
               bpp = 32;
           }
           break;
      default:
           {
               bpp = 0;
           }

    }

    return bpp;
}

static uint32_t cy_gfxss_configure_layer(cy_stc_gfx_layer_config_t *layer_config)
{
    gctUINT bpp = 0, stride = 0;
    viv_dc_buffer buffer = {0};
    uint32_t ret = 0;
    viv_dc_rect display_rect = {0};
 
    /* select layer */
    viv_dc_select_layer(layer_config->layer_type);
 
    /* enable layer */
    viv_layer_enable(vivTRUE);
 
    bpp = cy_gfxss_get_bpp_from_format(layer_config->input_format_type);
 
    stride = layer_config->width * bpp / 8;
 
    /* config the buffer's phyAddr/format/tilemode/bufferWidth/bufferHeight/stride to kernel */
    buffer.phyAddress[0] = (gctADDRESS) layer_config->buffer_address;
    buffer.stride[0] = stride;
    buffer.format = layer_config->input_format_type;
    buffer.tiling = layer_config->tiling_type;
    buffer.width = layer_config->width;
    buffer.height = layer_config->height;
    viv_layer_set(&buffer);

    /* config display region on panel */
    display_rect.x = 0;
    display_rect.y = 0;
    display_rect.w = layer_config->width;
    display_rect.h = layer_config->height;

    viv_layer_scale(&display_rect, vivFILTER_H3_V3);

    viv_layer_set_position(layer_config->pos_x, layer_config->pos_y);
 
    viv_layer_zorder(layer_config->zorder);
 
    viv_layer_set_display(vivDISPLAY_0);
 
    return ret;
}
/** Display controller initialization */
static uint32_t cy_gfxss_dc_init(GFXSS_Type *base, cy_stc_gfx_dc_config_t *config, cy_stc_mipidsi_config_t *mipi_dsi_cfg)
{
    gctINT ret = 0;
    viv_dc_rect display_rect = {0};
    viv_output display_output = {0};

    /* open device */
    ret = viv_dc_init();

    if(ret)
        return ret;

    /* reset DC */
    viv_dc_reset();

    if ((config->display_type == GFX_DISP_TYPE_DPI) || (config->display_type == GFX_DISP_TYPE_DSI_DPI))
    {
        viv_conf_display_set_custom_size(dcCore, vivDISPLAY_0,
        config->gfx_layer_config->width,
        config->gfx_layer_config->width + mipi_dsi_cfg->display_params->hfp,
        config->gfx_layer_config->width + mipi_dsi_cfg->display_params->hfp + mipi_dsi_cfg->display_params->hsync_width,
        config->gfx_layer_config->width + mipi_dsi_cfg->display_params->hfp + mipi_dsi_cfg->display_params->hsync_width + mipi_dsi_cfg->display_params->hbp,
        config->gfx_layer_config->height,
        config->gfx_layer_config->height + mipi_dsi_cfg->display_params->vfp,
        config->gfx_layer_config->height + mipi_dsi_cfg->display_params->vfp + mipi_dsi_cfg->display_params->vsync_width,
        config->gfx_layer_config->height + mipi_dsi_cfg->display_params->vfp + mipi_dsi_cfg->display_params->vsync_width + mipi_dsi_cfg->display_params->vbp
        );
    }
    else
    {
        viv_conf_display_set_custom_size(dcCore, vivDISPLAY_0,
        config->gfx_layer_config->width,
        0,
        config->gfx_layer_config->width,
        config->gfx_layer_config->width,
        config->gfx_layer_config->height,
        0,
        config->gfx_layer_config->height,
        config->gfx_layer_config->height
        );
    }

    /* Configure Graphics/Video Layer */
    if(config->gfx_layer_config != NULL && config->gfx_layer_config->layer_enable )
    {
        cy_gfxss_configure_layer(config->gfx_layer_config);
    }

    /* Configure Overlay 0 Layer */
    if(config->ovl0_layer_config != NULL && config->ovl0_layer_config->layer_enable )
    {
        cy_gfxss_configure_layer(config->ovl0_layer_config);
    }

    /* Configure Overlay 1 Layer */
    if(config->ovl1_layer_config != NULL && config->ovl1_layer_config->layer_enable )
    {
        cy_gfxss_configure_layer(config->ovl1_layer_config);
    }


    /* Get display size from viv_display_size_type */
    cy_gfxss_update_display_rect_params(config->display_size, &display_rect);

    viv_set_display_size(vivDISPLAY_0, config->display_size);

    display_output.format = config->display_format;

    if ((config->display_type == GFX_DISP_TYPE_DPI) || (config->display_type == GFX_DISP_TYPE_DSI_DPI))
    {
        display_output.type = vivDPI;
        viv_set_output(vivDISPLAY_0, &display_output, vivTRUE);
    }
    else
    {
        display_output.type = vivDBI;
        viv_set_output_dbi(vivDISPLAY_0, config->display_type);
        viv_set_output(vivDISPLAY_0, &display_output, vivTRUE);    
        viv_set_commit(1);
    }


    return ret;
}

cy_en_gfx_status_t Cy_GFXSS_Init(GFXSS_Type *base, cy_stc_gfx_config_t *config, cy_stc_gfx_context_t *context)
{

/* Configures:-

 - Clock and power
 - Enable IP Block
 - Call vg_lite_init() and vg_lite_allocate() with video/graphics layer resolution.
 - Sets the user provided interrupt mask setting.  */
    CY_ASSERT(base != NULL);
    CY_ASSERT(config != NULL);
    CY_ASSERT(context != NULL);

    cy_en_gfx_status_t result = CY_GFX_SUCCESS;
    GFXSS_GPU_Type *gpu_base = &(base->GFXSS_GPU);
    GFXSS_DC_Type *dc_base = &(base->GFXSS_DC);
    GFXSS_MIPIDSI_Type *mipidsi_base = &(base->GFXSS_MIPIDSI);

    cy_stc_gfx_dc_config_t *dc_cfg = config->dc_cfg;
    cy_stc_mipidsi_config_t *mipi_dsi_cfg = config->mipi_dsi_cfg;

    /* Assume Clock and Power are already available by this moment */

    /* Enable GPU if user provided configuration */
    if (gpu_base != NULL)
    {
        gpu_base->MXGPU.CTL |= GFXSS_GPU_MXGPU_CTL_ENABLED_Msk;
    }

    /* Configure Display Controller */
    if (dc_base != NULL)
    { 
        dc_base->MXDC.CTL |= GFXSS_DC_MXDC_CTL_ENABLED_Msk;
        dc_base->MXDC.INTR_MASK |= _VAL2FLD(GFXSS_DC_MXDC_INTR_MASK_CORE_MASK, 1U) | _VAL2FLD(GFXSS_DC_MXDC_INTR_MASK_ADDR0_MASK, 1U)
                                | _VAL2FLD(GFXSS_DC_MXDC_INTR_MASK_ADDR1_MASK, 1U) | _VAL2FLD(GFXSS_DC_MXDC_INTR_MASK_ADDR2_MASK, 1U)
                                | _VAL2FLD(GFXSS_DC_MXDC_INTR_MASK_ADDR3_MASK, 1U) | _VAL2FLD(GFXSS_DC_MXDC_INTR_MASK_RLAD_ERROR_MASK, 1U);

        switch(config->dc_cfg->display_type)
        {
            case GFX_DISP_TYPE_DPI:
            {
                dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_DPI_ENABLED_Msk;
            }
            break;
            case GFX_DISP_TYPE_DBI_A:
            case GFX_DISP_TYPE_DBI_B:
            case GFX_DISP_TYPE_DBI_C:
            {
                 dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_DBI_ENABLED_Msk;

                 /* SPI Interface */
                 if (config->dc_cfg->display_type == GFX_DISP_TYPE_DBI_C)
                 {
                     dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_SPI_ENABLED_Msk;
                 }
            }
            break;
            case GFX_DISP_TYPE_DSI_DBI:
            case GFX_DISP_TYPE_DSI_DPI:
            {
                if(mipidsi_base == NULL)
                {
                    return CY_GFX_BAD_PARAM;
                }

                dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_DPI_ENABLED_Msk;

                /* DSI specific Configuration */
                mipidsi_base->MXMIPIDSI.CLK_CTL |= GFXSS_MIPIDSI_MXMIPIDSI_CLK_CTL_CLK_CFG_Msk;
                mipidsi_base->MXMIPIDSI.CTL |= GFXSS_MIPIDSI_MXMIPIDSI_CTL_ENABLED_Msk;
                mipidsi_base->MXMIPIDSI.PHY_FREQ_RANGE = _VAL2FLD(GFXSS_MIPIDSI_MXMIPIDSI_PHY_FREQ_RANGE_CFG_CLK_FREQ_RANGE, (((CLK_SYS_FREQ / 2)   - 17U) * 4U));
                mipidsi_base->MXMIPIDSI.INTR_MASK = _VAL2FLD(GFXSS_MIPIDSI_MXMIPIDSI_INTR_MASK_CORE_MASK, 1U) | _VAL2FLD(GFXSS_MIPIDSI_MXMIPIDSI_INTR_MASK_DBI_TE_MASK, 1U) | _VAL2FLD(GFXSS_MIPIDSI_MXMIPIDSI_INTR_MASK_DPI_HALT_MASK, 1U);

                if (config->dc_cfg->display_type == GFX_DISP_TYPE_DSI_DPI)
                {
                    mipidsi_base->MXMIPIDSI.DBI_CMD = 1;
                }
                else
                {
                    mipidsi_base->MXMIPIDSI.DPI_CMD = 1; /* Default Polarity */
                }
            }

            break;
            default:
            {
                 /* Unsupported Display type */
            }
        }

        if (cy_gfxss_dc_init(base,dc_cfg,mipi_dsi_cfg) != 0U)
        {
            result = CY_GFX_BAD_PARAM;
        }else
        {
            if(dc_cfg->gfx_layer_config == NULL) {
                context->dc_context.gfx_layer_config.layer_enable = false;
            }else
            {
                memcpy(&context->dc_context.gfx_layer_config, dc_cfg->gfx_layer_config, sizeof(cy_stc_gfx_layer_config_t));
            }
            if(dc_cfg->ovl0_layer_config == NULL) {
                context->dc_context.ovl0_layer_config.layer_enable = false;
            }else
            {
                memcpy(&context->dc_context.ovl0_layer_config, dc_cfg->ovl0_layer_config, sizeof(cy_stc_gfx_layer_config_t));
            }
            if(dc_cfg->ovl1_layer_config == NULL) {
                context->dc_context.ovl1_layer_config.layer_enable = false;
            }else
            {
                memcpy(&context->dc_context.ovl1_layer_config, dc_cfg->ovl1_layer_config, sizeof(cy_stc_gfx_layer_config_t));
            }
            if(dc_cfg->rlad_config == NULL) {
                context->dc_context.rlad_config.enable = false;
            }else
            {
                memcpy(&context->dc_context.rlad_config, dc_cfg->rlad_config, sizeof(cy_stc_gfx_rlad_cfg_t));
                Cy_GFXSS_RLAD_SetImage(base, dc_cfg->rlad_config, context);
                if(context->dc_context.rlad_config.enable == 1U){
                    Cy_GFXSS_RLAD_Enable(base, context);
                }
            }
            if(dc_cfg->cursor_config == NULL) {
                context->dc_context.cursor_config.enable = false;
            }else
            {
                memcpy(&context->dc_context.cursor_config, dc_cfg->cursor_config, sizeof(cy_stc_gfx_cursor_config_t));
            }
            context->dc_context.display_type = dc_cfg->display_type;         
            context->dc_context.display_format = dc_cfg->display_format;        
            context->dc_context.display_size = dc_cfg->display_size;        
            context->dc_context.display_width = dc_cfg->display_width;        
            context->dc_context.display_height = dc_cfg->display_height;      
            context->dc_context.interrupt_mask = dc_cfg->interrupt_mask;      
        }

    }

    /* Configure MIPI DSI */
    if (((config->dc_cfg->display_type == GFX_DISP_TYPE_DSI_DBI) ||
         (config->dc_cfg->display_type == GFX_DISP_TYPE_DSI_DPI)) &&
        (mipi_dsi_cfg != NULL) && (mipidsi_base != NULL) && (result == CY_GFX_SUCCESS))
    {
        result = Cy_MIPIDSI_Init(mipidsi_base, mipi_dsi_cfg, &context->mipidsi_context);

        context->mipidsi_context.virtual_ch = mipi_dsi_cfg->virtual_ch;             /**< Display controller configuration */
        context->mipidsi_context.num_of_lanes = mipi_dsi_cfg->num_of_lanes;           /**< GPU configuration is optional */
        context->mipidsi_context.per_lane_mbps = mipi_dsi_cfg->per_lane_mbps;          /**< per lane speed in mbps */
        context->mipidsi_context.dpi_fmt = mipi_dsi_cfg->dpi_fmt;                /**< MIPI DSI configuration */
        context->mipidsi_context.max_phy_clk = mipi_dsi_cfg->max_phy_clk;            /**< device max DPHY clock in MHz unit */
        context->mipidsi_context.dsi_mode = mipi_dsi_cfg->dsi_mode;               /**< Command mode/Video mode */
        context->mipidsi_context.mode_flags = mipi_dsi_cfg->mode_flags;             /**< Additional mode information */
        context->mipidsi_context.enable = true;
        
        memcpy(&context->mipidsi_context.display_params, mipi_dsi_cfg->display_params, sizeof(cy_stc_mipidsi_display_params_t));        /**< Display parameters */   
    }else{
         context->mipidsi_context.enable = false;
    }

    return result;

}

cy_en_gfx_status_t  Cy_GFXSS_SleepCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    /* To be implemented */
    return CY_GFX_SUCCESS;
}

cy_en_syspm_status_t  Cy_GFXSS_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{

    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;
    
    CY_ASSERT_L1(NULL != callbackParams);

    GFXSS_Type *locBase = (GFXSS_Type *) callbackParams->base;
    cy_stc_gfx_context_t *locContext = (cy_stc_gfx_context_t *) callbackParams->context;

    GFXSS_GPU_Type *gpu_base = &(locBase->GFXSS_GPU);
    GFXSS_DC_Type *dc_base = &(locBase->GFXSS_DC);
    GFXSS_MIPIDSI_Type *mipidsi_base = &(locBase->GFXSS_MIPIDSI);

    switch(mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            bool checkFail = (gpu_base->GCNANO.AQHIIDLE & GPU_IDLE_MASK ) != GPU_IDLE_MASK;
            if (checkFail)
            {
                retStatus = CY_SYSPM_FAIL;
            }
            else
            {
                dc_base->DCNANO.GCREGPANELFUNCTION = _VAL2FLD(GFXSS_DC_DCNANO_GCREGPANELFUNCTION_GCREGPANELFUNCTION_OUTPUT, 0U);
                dc_base->DCNANO.GCREGFRAMEBUFFERCONFIG = _VAL2FLD(GFXSS_DC_DCNANO_GCREGFRAMEBUFFERCONFIG_GCREGFRAMEBUFFERCONFIG_ENABLE, 0U);
                dc_base->DCNANO.GCREGOVERLAYCONFIG = _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYCONFIG_GCREGOVERLAYCONFIG_ENABLE, 0U);
                dc_base->DCNANO.GCREGOVERLAYCONFIG1 = _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYCONFIG1_GCREGOVERLAYCONFIG1_ENABLE, 0U);
                dc_base->DCNANO.GCREGCURSORCONFIG = _VAL2FLD(GFXSS_DC_DCNANO_GCREGCURSORCONFIG_GCREGCURSORCONFIG_FORMAT, 0U);
                dc_base->DCNANO.GCREGPANELCONTROL = _VAL2FLD(GFXSS_DC_DCNANO_GCREGPANELCONTROL_GCREGPANELCONTROL_VALID, 1U);

                while (_FLD2VAL(GFXSS_DC_DCNANO_GCREGPANELCONTROL_GCREGPANELCONTROL_VALID, dc_base->DCNANO.GCREGPANELCONTROL) == 1U);
                Cy_GFXSS_DeInit(locBase, locContext);
                retStatus = CY_SYSPM_SUCCESS;
            }


        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        case CY_SYSPM_AFTER_TRANSITION:
        {
            /* Enable GPU if user provided configuration */
            gpu_base->MXGPU.CTL |= GFXSS_GPU_MXGPU_CTL_ENABLED_Msk;
            
            /* Configure Display Controller */
            dc_base->MXDC.CTL |= GFXSS_DC_MXDC_CTL_ENABLED_Msk;
            switch(locContext->dc_context.display_type)
            {
                case GFX_DISP_TYPE_DPI:
                {
                    dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_DBI_ENABLED_Msk;
                }
                break;
                case GFX_DISP_TYPE_DBI_A:
                case GFX_DISP_TYPE_DBI_B:
                case GFX_DISP_TYPE_DBI_C:
                {
                    dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_DBI_ENABLED_Msk;
                    /* SPI Interface */
                    if (locContext->dc_context.display_type == GFX_DISP_TYPE_DBI_C)
                    {
                        dc_base->MXDC.IO_CTL |= GFXSS_DC_MXDC_IO_CTL_SPI_ENABLED_Msk;
                    }
                }
                break;
                case GFX_DISP_TYPE_DSI_DBI:
                case GFX_DISP_TYPE_DSI_DPI:
                {
                    /* DSI specific Configuration */
                    mipidsi_base->MXMIPIDSI.CTL |= GFXSS_MIPIDSI_MXMIPIDSI_CTL_ENABLED_Msk;

                    if (locContext->dc_context.display_type == GFX_DISP_TYPE_DSI_DBI)
                    {
                        mipidsi_base->MXMIPIDSI.DBI_CMD = 0;
                    }
                    else
                    {
                        mipidsi_base->MXMIPIDSI.DPI_CMD = 0; /* Default Polarity */
                    }
                }

                break;
            }

            cy_stc_gfx_dc_config_t dccfg;
            cy_stc_mipidsi_config_t dsicfg;

            if(locContext->dc_context.gfx_layer_config.layer_enable==false){
                dccfg.gfx_layer_config = NULL;
            }else{
                dccfg.gfx_layer_config = &locContext->dc_context.gfx_layer_config;
            }
            if(locContext->dc_context.ovl0_layer_config.layer_enable==false){
                dccfg.ovl0_layer_config = NULL;
            }else{
                dccfg.ovl0_layer_config = &locContext->dc_context.ovl0_layer_config;
            }
            if(locContext->dc_context.ovl1_layer_config.layer_enable==false){
                dccfg.ovl1_layer_config = NULL;
            }else{
                dccfg.ovl1_layer_config = &locContext->dc_context.ovl1_layer_config;
            }
            if(locContext->dc_context.rlad_config.enable==false){
                dccfg.rlad_config = NULL;
            }else{
                dccfg.rlad_config = &locContext->dc_context.rlad_config;
            }
            if(locContext->dc_context.cursor_config.enable==false){
                dccfg.cursor_config = NULL;
            }else{
                dccfg.cursor_config = &locContext->dc_context.cursor_config;
            }
            dccfg.display_type = locContext->dc_context.display_type;         
            dccfg.display_format = locContext->dc_context.display_format;        
            dccfg.display_size = locContext->dc_context.display_size;        
            dccfg.display_width = locContext->dc_context.display_width;        
            dccfg.display_height = locContext->dc_context.display_height;      
            dccfg.interrupt_mask = locContext->dc_context.interrupt_mask;      

            /* Configure MIPI DSI */
            if(locContext->mipidsi_context.enable != false){
                dsicfg.virtual_ch = locContext->mipidsi_context.virtual_ch;
                dsicfg.num_of_lanes = locContext->mipidsi_context.num_of_lanes;
                dsicfg.per_lane_mbps = locContext->mipidsi_context.per_lane_mbps;
                dsicfg.dpi_fmt = locContext->mipidsi_context.dpi_fmt;
                dsicfg.max_phy_clk = locContext->mipidsi_context.max_phy_clk;
                dsicfg.dsi_mode = locContext->mipidsi_context.dsi_mode;
                dsicfg.mode_flags = locContext->mipidsi_context.mode_flags;
                dsicfg.display_params = &locContext->mipidsi_context.display_params;  
                Cy_MIPIDSI_Init(mipidsi_base, &dsicfg, &locContext->mipidsi_context);
            }
            cy_gfxss_dc_init(locBase, &dccfg, &dsicfg);

        }
        break;

        case CY_SYSPM_BEFORE_TRANSITION:
        {
            /* This code executes inside critical section and enabling active
            * interrupt source makes interrupt pending in the NVIC. However
            * interrupt processing is delayed until code exists critical
            * section. The pending interrupt force WFI instruction does
            * nothing and device remains in the active mode.
            */
        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
            break;
    }

    return (retStatus);
}

void Cy_GFXSS_Clear_DC_Interrupt(GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
  CY_ASSERT(base != NULL);
  CY_ASSERT(context != NULL);
  
  GFXSS_DC_Type *dc_base = &(base->GFXSS_DC);

  /* DC */
  if (dc_base != NULL)
  {
      (void)(dc_base->DCNANO.GCREGDISPLAYINTR);
      dc_base->MXDC.INTR = CLEAR_INTERRUPT;
  }
}

void  Cy_GFXSS_Interrupt(GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
  CY_ASSERT(base != NULL);
  CY_ASSERT(context != NULL);
  
  GFXSS_GPU_Type *gpu_base = &(base->GFXSS_GPU);
  GFXSS_DC_Type *dc_base = &(base->GFXSS_DC);
  GFXSS_MIPIDSI_Type *mipidsi_base = &(base->GFXSS_MIPIDSI);

  /* Clear interrupts */

  /* GPU */
  if (gpu_base != NULL)
  {
      gpu_base->MXGPU.INTR = CLEAR_INTERRUPT;
  }

  /* DC */
  if (dc_base != NULL)
  {
      dc_base->MXDC.INTR = CLEAR_INTERRUPT;
  }

  /* MIPI DSI */
  if (mipidsi_base != NULL)
  {
      mipidsi_base->MXMIPIDSI.INTR = CLEAR_INTERRUPT;
  }
}

cy_en_gfx_status_t Cy_GFXSS_Set_FrameBuffer(GFXSS_Type *base, uint32_t* gfx_layer_buffer, cy_stc_gfx_context_t *context)
{

    CY_ASSERT(base != NULL);
    
    if (gfx_layer_buffer != NULL)
    {
        base->GFXSS_DC.DCNANO.GCREGFRAMEBUFFERADDRESS = (uint32_t)gfx_layer_buffer;
        viv_set_commit(0x1);

        return CY_GFX_SUCCESS;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }

    return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_Set_Overaly0(GFXSS_Type *base,          uint32_t* overlay0_buffer, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);

    if (overlay0_buffer != NULL)
    {
        base->GFXSS_DC.DCNANO.GCREGOVERLAYADDRESS = (uint32_t)overlay0_buffer;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }
    
    return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_Set_Overaly1(GFXSS_Type *base,          uint32_t* overlay1_buffer, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);

    if (overlay1_buffer != NULL)
    {
        base->GFXSS_DC.DCNANO.GCREGOVERLAYADDRESS1 = (uint32_t)overlay1_buffer;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }
    
    return CY_GFX_SUCCESS;
}

uint32_t* Cy_GFXSS_Get_FrameBufferAddress(GFXSS_Type *base)
{
    CY_ASSERT(base != NULL);

    return (uint32_t *)(base->GFXSS_DC.DCNANO.GCREGFRAMEBUFFERADDRESS);

}

cy_en_gfx_status_t Cy_GFXSS_DeInit(GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);
    CY_ASSERT(context != NULL);

    GFXSS_GPU_Type *gpu_base = &(base->GFXSS_GPU);
    GFXSS_DC_Type *dc_base = &(base->GFXSS_DC);
    GFXSS_MIPIDSI_Type *mipidsi_base = &(base->GFXSS_MIPIDSI);

    /* Disable GPU */
    if (gpu_base != NULL)
    {
        gpu_base->MXGPU.CTL &= ~GFXSS_GPU_MXGPU_CTL_ENABLED_Msk;
    }

    /* Disable DC */
    if (dc_base != NULL)
    {
        dc_base->MXDC.CTL &= ~GFXSS_DC_MXDC_CTL_ENABLED_Msk;
    }

    /* MIPI DSI Base */
    if (mipidsi_base != NULL)
    {
        Cy_MIPIDSI_DeInit(mipidsi_base);
    }

    return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_RLAD_SetImage(GFXSS_Type *base,  cy_stc_gfx_rlad_cfg_t *rlad_cfg, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);
    CY_ASSERT(rlad_cfg != NULL);
    GFXSS_DC_Type *gfxss_dc = &(base->GFXSS_DC);

    if(gfxss_dc !=NULL)
    {
        /* Update Layer ID and Disable RLAD by default */
        gfxss_dc->MXDC.RLAD_CTL = _VAL2FLD(GFXSS_DC_MXDC_RLAD_CTL_RLAD_LAYER, rlad_cfg->layer_id + 1U);

        /* Image properties */
        gfxss_dc->MXDC.RLAD_IMG = (_VAL2FLD(GFXSS_DC_MXDC_RLAD_IMG_RLAD_WIDTH, rlad_cfg->image_width) |
                                    _VAL2FLD(GFXSS_DC_MXDC_RLAD_IMG_RLAD_HEIGHT, rlad_cfg->image_height) |
                                    _VAL2FLD(GFXSS_DC_MXDC_RLAD_IMG_RLAD_FORMAT, rlad_cfg->rlad_format));
        /* RLAD compression mode */
        gfxss_dc->MXDC.RLAD_ENC = _VAL2FLD(GFXSS_DC_MXDC_RLAD_ENC_RLAD_MODE, rlad_cfg->compression_mode);
        gfxss_dc->MXDC.RLAD_ENC |= _VAL2FLD(GFXSS_DC_MXDC_RLAD_ENC_RLAD_BITS_R, 8U);
        gfxss_dc->MXDC.RLAD_ENC |= _VAL2FLD(GFXSS_DC_MXDC_RLAD_ENC_RLAD_BITS_G, 8U);
        gfxss_dc->MXDC.RLAD_ENC |= _VAL2FLD(GFXSS_DC_MXDC_RLAD_ENC_RLAD_BITS_B, 8U);
        gfxss_dc->MXDC.RLAD_ENC |= _VAL2FLD(GFXSS_DC_MXDC_RLAD_ENC_RLAD_BITS_A, 8U);

        /* Compressed Image address */
        gfxss_dc->MXDC.RLAD_BUF0 = (uint32_t)rlad_cfg->image_address;

        /* Compressed Image size */
        gfxss_dc->MXDC.RLAD_BUF1 = _VAL2FLD(GFXSS_DC_MXDC_RLAD_BUF1_RLAD_SIZE, rlad_cfg->compressed_image_size);

        return CY_GFX_SUCCESS;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }
}

cy_en_gfx_status_t Cy_GFXSS_RLAD_Enable( GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);

    GFXSS_DC_Type *gfxss_dc = &(base->GFXSS_DC);

    if(gfxss_dc !=NULL)
    {
        gfxss_dc->MXDC.RLAD_CTL |= GFXSS_DC_MXDC_RLAD_CTL_RLAD_ENABLE_Msk;
        context->dc_context.rlad_config.enable = true;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }
    return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_RLAD_Disable( GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);

    GFXSS_DC_Type *gfxss_dc = &(base->GFXSS_DC);

    if(gfxss_dc !=NULL)
    {
        gfxss_dc->MXDC.RLAD_CTL &= ~GFXSS_DC_MXDC_RLAD_CTL_RLAD_ENABLE_Msk;
        context->dc_context.rlad_config.enable = false;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }

    return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_Transfer_Frame( GFXSS_Type *base, cy_stc_gfx_context_t *context){
    CY_ASSERT(base != NULL);
    CY_ASSERT(context != NULL);

    GFXSS_DC_Type *gfxss_dc = &(base->GFXSS_DC);
    uint32_t horizontal_resolution = context->dc_context.display_width;
    uint32_t vertical_resolution = context->dc_context.display_height;
    uint32_t No_of_lines = (DBI_SCLICE_LIMIT_IN_BYTES / (horizontal_resolution * RGB_16_BIT_PIXEL_FORMAT_IN_BYTES)) - ((horizontal_resolution * RGB_16_BIT_PIXEL_FORMAT_IN_BYTES) % 4); 

    uint8_t bytes_per_pixel_framebuffer = RGB_16_BIT_PIXEL_FORMAT_IN_BYTES;    
    uint8_t bytes_per_pixel_overlay = RGB_16_BIT_PIXEL_FORMAT_IN_BYTES;
    uint8_t bytes_per_pixel_overlay1 = RGB_16_BIT_PIXEL_FORMAT_IN_BYTES;
    if(_FLD2VAL(GFXSS_DC_DCNANO_GCREGFRAMEBUFFERCONFIG_GCREGFRAMEBUFFERCONFIG_FORMAT, gfxss_dc->DCNANO.GCREGFRAMEBUFFERCONFIG) == 4U)
    {
        bytes_per_pixel_framebuffer = RGB_32_BIT_PIXEL_FORMAT_IN_BYTES;
    }
    if(_FLD2VAL(GFXSS_DC_DCNANO_GCREGOVERLAYCONFIG_GCREGOVERLAYCONFIG_FORMAT, gfxss_dc->DCNANO.GCREGOVERLAYCONFIG) == 4U)
    {
        bytes_per_pixel_overlay = RGB_32_BIT_PIXEL_FORMAT_IN_BYTES;
    }
    if(_FLD2VAL(GFXSS_DC_DCNANO_GCREGOVERLAYCONFIG1_GCREGOVERLAYCONFIG1_FORMAT, gfxss_dc->DCNANO.GCREGOVERLAYCONFIG1) == 4U)
    {
        bytes_per_pixel_overlay1 = RGB_32_BIT_PIXEL_FORMAT_IN_BYTES;
    }

    for(int i=0; i < vertical_resolution; i += No_of_lines){ 
        gfxss_dc->DCNANO.GCREGFRAMEBUFFERSIZE = _VAL2FLD(GFXSS_DC_DCNANO_GCREGFRAMEBUFFERSIZE_GCREGFRAMEBUFFERSIZE_WIDTH, horizontal_resolution) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGFRAMEBUFFERSIZE_GCREGFRAMEBUFFERSIZE_HEIGHT, No_of_lines);
        gfxss_dc->DCNANO.GCREGOVERLAYSIZE = _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE_GCREGOVERLAYSIZE_WIDTH, horizontal_resolution) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE_GCREGOVERLAYSIZE_HEIGHT, No_of_lines);
        gfxss_dc->DCNANO.GCREGOVERLAYSIZE1 = _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE1_GCREGOVERLAYSIZE1_WIDTH, horizontal_resolution) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE1_GCREGOVERLAYSIZE1_HEIGHT, No_of_lines);
        gfxss_dc->DCNANO.GCREGVDISPLAY = _VAL2FLD(GFXSS_DC_DCNANO_GCREGVDISPLAY_GCREGVDISPLAY_DISPLAY_END, No_of_lines) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGVDISPLAY_GCREGVDISPLAY_TOTAL, (No_of_lines + 1));
        gfxss_dc->DCNANO.GCREGVSYNC =  _VAL2FLD(GFXSS_DC_DCNANO_GCREGVSYNC_GCREGVSYNC_START, 0U ) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGVSYNC_GCREGVSYNC_END,  No_of_lines);

        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_ADDRESS, 0x2A); // set column address 
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x01);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x3F);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_ADDRESS, 0x2B); // set row address 
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x63);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_ADDRESS, 0x2C); // send data 
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_MEM, 0x00);

        while((gfxss_dc->DCNANO.GCREGDISPLAYINTR & GFXSS_DC_DCNANO_GCREGDISPLAYINTR_GCREGDISPLAYINTR_DISP0_Msk ) != GFXSS_DC_DCNANO_GCREGDISPLAYINTR_GCREGDISPLAYINTR_DISP0_Msk );

        gfxss_dc->DCNANO.GCREGFRAMEBUFFERADDRESS += (No_of_lines * horizontal_resolution * bytes_per_pixel_framebuffer);
        gfxss_dc->DCNANO.GCREGOVERLAYADDRESS += (No_of_lines * horizontal_resolution * bytes_per_pixel_overlay);
        gfxss_dc->DCNANO.GCREGOVERLAYADDRESS1 += (No_of_lines * horizontal_resolution * bytes_per_pixel_overlay1);
    }

    No_of_lines = vertical_resolution % No_of_lines;
    if(No_of_lines > 0 )
    {
        gfxss_dc->DCNANO.GCREGFRAMEBUFFERSIZE = _VAL2FLD(GFXSS_DC_DCNANO_GCREGFRAMEBUFFERSIZE_GCREGFRAMEBUFFERSIZE_WIDTH, horizontal_resolution) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGFRAMEBUFFERSIZE_GCREGFRAMEBUFFERSIZE_HEIGHT, No_of_lines);
        gfxss_dc->DCNANO.GCREGOVERLAYSIZE = _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE_GCREGOVERLAYSIZE_WIDTH, horizontal_resolution) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE_GCREGOVERLAYSIZE_HEIGHT, No_of_lines);
        gfxss_dc->DCNANO.GCREGOVERLAYSIZE1 = _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE1_GCREGOVERLAYSIZE1_WIDTH, horizontal_resolution) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGOVERLAYSIZE1_GCREGOVERLAYSIZE1_HEIGHT, No_of_lines);
        gfxss_dc->DCNANO.GCREGVDISPLAY = _VAL2FLD(GFXSS_DC_DCNANO_GCREGVDISPLAY_GCREGVDISPLAY_DISPLAY_END, No_of_lines) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGVDISPLAY_GCREGVDISPLAY_TOTAL, (No_of_lines + 1));
        gfxss_dc->DCNANO.GCREGVSYNC =  _VAL2FLD(GFXSS_DC_DCNANO_GCREGVSYNC_GCREGVSYNC_START, 0U ) | _VAL2FLD(GFXSS_DC_DCNANO_GCREGVSYNC_GCREGVSYNC_END,  No_of_lines);

        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_ADDRESS, 0x2A); // set column address 
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x01);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x3F);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_ADDRESS, 0x2B); // set row address 
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x00);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_DATA, 0x63);
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_ADDRESS, 0x2C); // send data 
        viv_hw_display_dbi_set_command(DC_CORE->hardware, vivD8R5G6B5, vivDBI_COMMAND_MEM, 0x00);

        while((gfxss_dc->DCNANO.GCREGDISPLAYINTR & GFXSS_DC_DCNANO_GCREGDISPLAYINTR_GCREGDISPLAYINTR_DISP0_Msk ) != GFXSS_DC_DCNANO_GCREGDISPLAYINTR_GCREGDISPLAYINTR_DISP0_Msk );
    }
        return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_Enable_GPU( GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);
    uint32_t timeout = CY_GFXSS_TOTAL_NO_OF_RETRIES;
    GFXSS_GPU_Type *gpu_base = &(base->GFXSS_GPU);

    if(gpu_base != NULL)
    {
        gpu_base->MXGPU.CTL |= GFXSS_GPU_MXGPU_CTL_ENABLED_Msk;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }

    while ((_FLD2VAL(GFXSS_GPU_MXGPU_STATUS_CORE_RST, gpu_base->MXGPU.STATUS) != 0U) && ( timeout > 0u))
    {
        Cy_SysLib_DelayUs(CY_GFXSS_WAIT_1_UNIT);
        timeout--;
    }
    if(_FLD2VAL(GFXSS_GPU_MXGPU_STATUS_CORE_RST, gpu_base->MXGPU.STATUS) != 0U)
    {
        return CY_GFX_TIMEOUT;
    }
    return CY_GFX_SUCCESS;
}

cy_en_gfx_status_t Cy_GFXSS_Disable_GPU( GFXSS_Type *base, cy_stc_gfx_context_t *context)
{
    CY_ASSERT(base != NULL);
    uint32_t timeout = CY_GFXSS_TOTAL_NO_OF_RETRIES;
    GFXSS_GPU_Type *gpu_base = &(base->GFXSS_GPU);

    if(gpu_base != NULL)
    {
        while((gpu_base->GCNANO.AQHIIDLE & GPU_IDLE_MASK ) != GPU_IDLE_MASK );
        gpu_base->MXGPU.CTL &= ~GFXSS_GPU_MXGPU_CTL_ENABLED_Msk;
    }
    else
    {
        return CY_GFX_BAD_PARAM;
    }

    while ((_FLD2VAL(GFXSS_GPU_MXGPU_STATUS_CORE_RST, gpu_base->MXGPU.STATUS) != 1U) && ( timeout > 0u))
    {
        Cy_SysLib_DelayUs(CY_GFXSS_WAIT_1_UNIT);
        timeout--;
    }
    if(_FLD2VAL(GFXSS_GPU_MXGPU_STATUS_CORE_RST, gpu_base->MXGPU.STATUS) != 1U)
    {
        return CY_GFX_TIMEOUT;
    }
    return CY_GFX_SUCCESS;
}

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXS22GFXSS */
/* [] END OF FILE */
