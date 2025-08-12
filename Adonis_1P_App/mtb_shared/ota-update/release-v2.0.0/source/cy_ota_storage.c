/*
 * Copyright 2022, Cypress Semiconductor Corporation (an Infineon company)
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
 */

/*
 * Cypress OTA Download Storage abstraction
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "cy_ota_api.h"
#include "cy_ota_internal.h"

#include "flash_map_backend.h"

#include "cy_log.h"
#include "sm_log.h"

/***********************************************************************
 *
 * defines & enums
 *
 * For more info on locations within slots, please see MCUBootApp
 * bootutils_misc.c, bootutils_private.h, bootutils.h
 *
 **********************************************************************/

/***********************************************************************
 *
 * Macros
 *
 **********************************************************************/

/***********************************************************************
 *
 * Structures
 *
 **********************************************************************/

/***********************************************************************
 *
 * Variables
 *
 **********************************************************************/

/***********************************************************************
 *
 * functions
 *
 **********************************************************************/
/**
 * @brief Open Storage area for download
 *
 * NOTE: Typically, this erases Secondary Slot
 *
 * @param[in]   ctx_ptr - pointer to OTA agent context @ref cy_ota_context_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_OPEN_STORAGE
 */
cy_rslt_t cy_ota_storage_open(cy_ota_context_ptr ctx_ptr)
{
    const struct flash_area *fap;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);
    cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s()\n", __func__);

    sm_log(SM_LOG_DEBUG, "FLASH_AREA_IMAGE_SECONDARY(0): %d\r\n", FLASH_AREA_IMAGE_SECONDARY(0));
    if (flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0), &fap) != 0)
    {
        sm_log(SM_LOG_DEBUG,"%s() flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0) ) failed\n", __func__);
        cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0) ) failed\n", __func__);
        return CY_RSLT_OTA_ERROR_OPEN_STORAGE;
    }

    sm_log(SM_LOG_DEBUG,"Erase secondary image slot fap->fa_off: 0x%08lx, size: 0x%08lx\n", fap->fa_off, fap->fa_size);
    cy_log_msg(CYLF_OTA, CY_LOG_NOTICE, "Erase secondary image slot fap->fa_off: 0x%08lx, size: 0x%08lx\n", fap->fa_off, fap->fa_size);
    if (flash_area_erase(fap, 0, fap->fa_size) != 0)
    {
        sm_log(SM_LOG_DEBUG,"%s() flash_area_erase(fap, 0) failed\r\n", __func__);
        cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_erase(fap, 0) failed\r\n", __func__);
        return CY_RSLT_OTA_ERROR_OPEN_STORAGE;
    }

    ctx->storage_loc = (void *)fap;

    return CY_RSLT_SUCCESS;
}

#define EXT_FLASH_UI1_START_ADDRESS 0x18000000
#define EXT_FLASH_UI1_SIZE 0xEAF000
#define EXT_FLASH_UI2_START_ADDRESS 0x18EAF000
#define EXT_FLASH_UI2_SIZE 0xEAF000
#define EXT_FLASH_UI_FLAG_START_ADDRESS 0x19D5E000

/*wanggankun@2024-11-14, add, for get flash area size*/
uint32_t cy_ota_up_bin_area_size()
{
    return flash_area_size(FLASH_AREA_IMAGE_SECONDARY(0));
}

/*wanggankun@2024-11-14, add, for ui image flash open*/
char cy_ota_ui_storage_open(cy_ota_context_ptr ctx_ptr)
{
    uint8_t buffer = 0;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    ext_flash_read(EXT_FLASH_UI_FLAG_START_ADDRESS, &buffer, 1);
    sm_log(SM_LOG_DEBUG,"cy_ota_ui_storage_open: %d\r\n", buffer);

    if(buffer == CY_OTA_IMAGE_USE_UI1 || buffer == 0xFF )
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI2_START_ADDRESS: 0x%x\r\n", EXT_FLASH_UI2_START_ADDRESS);
        ctx->ota_image_flag = CY_OTA_IMAGE_USE_UI2;
        ctx->ota_image_address = EXT_FLASH_UI2_START_ADDRESS;
        return CY_OTA_IMAGE_USE_UI2;
    }
    else if(buffer == CY_OTA_IMAGE_USE_UI2)
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI1_START_ADDRESS: 0x%x\r\n", EXT_FLASH_UI1_START_ADDRESS);
        ctx->ota_image_flag = CY_OTA_IMAGE_USE_UI1;
        ctx->ota_image_address = EXT_FLASH_UI1_START_ADDRESS;
        return CY_OTA_IMAGE_USE_UI1;
    }
    else
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI1_START_ADDRESS: unknow\r\n");
        ctx->ota_image_flag = CY_OTA_IMAGE_USE_UNKNOW;
        ctx->ota_image_address = CY_OTA_IMAGE_USE_UNKNOW;
    	return CY_OTA_IMAGE_USE_UNKNOW;
    }
}

cy_rslt_t cy_ota_storage_ui_write(cy_ota_context_ptr ctx_ptr, cy_ota_storage_write_info_t *chunk_info)
{
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);
    //sm_log(SM_LOG_DEBUG, "%s() buf:%p len:%ld off: 0x%lx (%ld)\n", __func__, chunk_info->buffer, chunk_info->size, chunk_info->offset, chunk_info->offset);

    uint32_t data_will_be_written = ctx->total_bytes_written + chunk_info->size;

    if(ctx->total_bytes_written % 4096 == 0 || 
       ((data_will_be_written % 4096) != 0 && (data_will_be_written/4096 - ctx->total_bytes_written/4096) > 0 ))
    {
        ctx->ota_image_earse_total++;
        ext_flash_earse( ctx->ota_image_address + (data_will_be_written/4096)*4096, 4096);
    }

    if (ext_flash_write(ctx->ota_image_address + chunk_info->offset, chunk_info->buffer, chunk_info->size) != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "ext_flash_write() failed\n");
        return CY_RSLT_OTA_ERROR_WRITE_STORAGE;
    }

    return CY_RSLT_SUCCESS;
}

/**
 * @brief Read from storage area
 *
 * @param[in]       ctx_ptr     - pointer to OTA agent context @ref cy_ota_context_t
 * @param[in][out]  chunk_info  - pointer to chunk information, buffer pointer used for the read
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_READ_STORAGE
 */
cy_rslt_t cy_ota_storage_read(cy_ota_context_ptr ctx_ptr, cy_ota_storage_write_info_t *chunk_info)
{
    const struct flash_area     *fap;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);

    cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() buf:%p len:%ld off: 0x%lx (%ld)\n", __func__,
                             chunk_info->buffer, chunk_info->size,
                             chunk_info->offset, chunk_info->offset);

    /* Always read from secondary slot */
    fap = (const struct flash_area *)ctx->storage_loc;
    if (fap != NULL)
    {
        /* read into the chunk_info buffer */
        if (flash_area_read(fap, chunk_info->offset, chunk_info->buffer, chunk_info->size) != CY_RSLT_SUCCESS)
        {
            cy_log_msg(CYLF_OTA, CY_LOG_ERR, "flash_area_read() failed \n");
            return CY_RSLT_OTA_ERROR_READ_STORAGE;
        }
        return CY_RSLT_SUCCESS;
    }

    cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area invalid\r\n", __func__);
    return CY_RSLT_OTA_ERROR_OPEN_STORAGE;
}

/**
 * @brief Write to storage area
 *
 * @param[in]   ctx_ptr     - pointer to OTA agent context @ref cy_ota_context_t
 * @param[in]   chunk_info  - pointer to chunk information
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_WRITE_STORAGE
 */
cy_rslt_t cy_ota_storage_write(cy_ota_context_ptr ctx_ptr, cy_ota_storage_write_info_t *chunk_info)
{
    const struct flash_area *fap;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);

    cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() buf:%p len:%ld off: 0x%lx (%ld)\n", __func__,
                             chunk_info->buffer, chunk_info->size,
                             chunk_info->offset, chunk_info->offset);

    /* write to secondary slot */
    fap = (const struct flash_area *)ctx->storage_loc;
    if (fap != NULL)
    {
        if (flash_area_write(fap, chunk_info->offset, chunk_info->buffer, chunk_info->size) != CY_RSLT_SUCCESS)
        {
            cy_log_msg(CYLF_OTA, CY_LOG_ERR, "flash_area_write() failed\n");
            return CY_RSLT_OTA_ERROR_WRITE_STORAGE;
        }
        return CY_RSLT_SUCCESS;
    }

    cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() no fap!\n", __func__);
    return CY_RSLT_OTA_ERROR_OPEN_STORAGE;
}

/**
 * @brief Close Storage area for download
 *
 * @param[in]   ctx_ptr - pointer to OTA agent context @ref cy_ota_context_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_CLOSE_STORAGE
 */
cy_rslt_t cy_ota_storage_close(cy_ota_context_ptr ctx_ptr)
{
    const struct flash_area *fap;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);

    cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s()\n", __func__);

    /* close secondary slot */
    fap = (const struct flash_area *)ctx->storage_loc;
    if (fap == NULL)
    {
        return CY_RSLT_OTA_ERROR_CLOSE_STORAGE;
    }
    flash_area_close(fap);

    return CY_RSLT_SUCCESS;
}

/**
 * @brief Verify download signature
 *
 * @param[in]   ctx_ptr - pointer to OTA agent context @ref cy_ota_context_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_storage_verify(cy_ota_context_ptr ctx_ptr)
{
    cy_rslt_t               result = CY_RSLT_SUCCESS;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);
    (void)ctx;

    cy_log_msg(CYLF_OTA, CY_LOG_NOTICE, "%s()\n", __func__);
    if (flash_area_boot_set_pending(0, (ctx->agent_params.validate_after_reboot == 0) ) != CY_RSLT_SUCCESS)
    {
        cy_log_msg(CYLF_OTA, CY_LOG_ERR, "VERIFY flash_area_boot_set_pending() Failed \n");
        result = CY_RSLT_OTA_ERROR_VERIFY;
    }
    return result;
}

cy_rslt_t cy_ota_storage_ui_verify(cy_ota_context_ptr ctx_ptr)
{
    cy_rslt_t               result = CY_RSLT_SUCCESS;
    cy_ota_context_t *ctx = (cy_ota_context_t *)ctx_ptr;
    CY_OTA_CONTEXT_ASSERT(ctx);

    uint8_t buffer[10]={0};

    if (ctx->ota_image_flag == CY_OTA_IMAGE_USE_UI1)
    {
        sm_log(SM_LOG_DEBUG, "cy_ota_storage_ui_verify: UI1, 0x%x\r\n", ctx->ota_image_flag);
        memset(buffer, 0, 10);
    }
    else if  (ctx->ota_image_flag == CY_OTA_IMAGE_USE_UI2)
    {
        sm_log(SM_LOG_DEBUG, "cy_ota_storage_ui_verify: UI2, 0x%x\r\n", ctx->ota_image_flag);
        memset(buffer, 1, 10);
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "cy_ota_storage_ui_verify: UI1, 0x%x\r\n", ctx->ota_image_flag);
        memset(buffer, 0xFF, 10);
    }

    ext_flash_earse(EXT_FLASH_UI_FLAG_START_ADDRESS, 4096);

    if (ext_flash_write(EXT_FLASH_UI_FLAG_START_ADDRESS, &buffer[0], 10) != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "cy_ota_storage_ui_verify() failed\n");
        return CY_RSLT_OTA_ERROR_VERIFY;
    }
    return result;
}


/**
 * @brief App has validated the new OTA Image
 *
 * This call needs to be after reboot and MCUBoot has copied the new Application
 *      to the Primary Slot.
 *
 * @param   N/A
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_storage_validated(void)
{
    /* Mark Image in Primary Slot as valid */
    cy_rslt_t               result = CY_RSLT_SUCCESS;

    /* we copy this to a RAM buffer so that if we are running in XIP from external flash, the write routine won't fail */
    if (flash_area_boot_set_confirmed() != CY_RSLT_SUCCESS)
    {
        cy_log_msg(CYLF_OTA, CY_LOG_ERR, "VERIFY flash_area_boot_set_confirmed() Failed \n");
        result = CY_RSLT_OTA_ERROR_VERIFY;
    }

    return result;
}

#ifdef FW_DATBLOCK_SEPARATE_FROM_APPLICATION

#define CY_OTA_IMGTOOL_HEADER_SIZE              0x100
#define CY_OTA_SEPARATE_INTERNAL_HEADER_SIZE    0x100

static cy_ota_fw_data_block_header_t cy_ota_fwdb_info;

static cy_ota_fw_data_block_header_t *cy_ota_fwdb_get_base_info( void )
{
    const struct flash_area         *fap;
    cy_ota_fw_data_block_header_t   *info = &cy_ota_fwdb_info;
    uint32_t                        read_offset;

    cy_log_set_facility_level(CYLF_OTA, CY_LOG_INFO);

    /* Always read from secondary slot */
    if (flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1), &fap) != 0)
    {
        cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1) ) failed\n", __func__);
        return NULL;
    }

    read_offset = CY_OTA_IMGTOOL_HEADER_SIZE;
    /* read into the info buffer */
    if (flash_area_read(fap, read_offset, (uint8_t *)info, sizeof(cy_ota_fw_data_block_header_t)) != CY_RSLT_SUCCESS)
    {
        cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_read(info) failed \n", __func__);
        flash_area_close(fap);
        info = NULL;
    }

    flash_area_close(fap);

    if ( (info->WiFi_FW_offset == 0xFFFF) && (info->BT_FW_offset == 0xFFFF) )
    {
        info = NULL;
    }

    if (info != NULL)
    {
        /* Add base address */
        if (info->WiFi_FW_offset != 0)
        {
            info->WiFi_FW_offset += CY_OTA_SEPARATE_INTERNAL_HEADER_SIZE;
        }
        if (info->CLM_blob_offset != 0)
        {
            info->CLM_blob_offset += CY_OTA_SEPARATE_INTERNAL_HEADER_SIZE;
        }
        if (info->BT_FW_offset != 0)
        {
            info->BT_FW_offset += CY_OTA_SEPARATE_INTERNAL_HEADER_SIZE;
        }

        /* For debugging */
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:WiFi_FW_version: %d %d %d %d\n", __func__,
                                          info->WiFi_FW_version[0], info->WiFi_FW_version[1], info->WiFi_FW_version[2], info->WiFi_FW_version[3] );
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:WiFi_FW_offset : 0x%x\n", __func__, info->WiFi_FW_offset);
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:WiFi_FW_size   : 0x%x\n", __func__, info->WiFi_FW_size);
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:CLM_blob_offset: 0x%x\n", __func__, info->CLM_blob_offset);
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:CLM_blob_size  : 0x%x\n", __func__, info->CLM_blob_size);

        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:BT_FW_version  : >%s<\n", __func__, info->BT_FW_version);
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:BT_FW_offset   : 0x%x\n", __func__, info->BT_FW_offset);
        cy_log_msg(CYLF_OTA, CY_LOG_INFO, "%s() info:BT_FW_size     : 0x%x\n", __func__, info->BT_FW_size);

    }

    return info;
}


/**
 * @brief When FW is stored in a separate Data Block, get WiFi FW info
 *
 * Use this call to get the external flash WiFi info
 *
 * @param   wifi_fw_info   - ptr to cy_ota_fwdb_wifi_fw_info_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_fwdb_get_wifi_fw_info(cy_ota_fwdb_wifi_fw_info_t *wifi_fw_info)
{
    cy_ota_fw_data_block_header_t *fwdb_header;

    if (wifi_fw_info == NULL)
    {
        return CY_RSLT_OTA_ERROR_GENERAL;
    }

    fwdb_header = cy_ota_fwdb_get_base_info();
    if (fwdb_header != NULL)
    {
        const struct flash_area *fap;
        int rc;

        memcpy( &wifi_fw_info->WiFi_FW_version, &fwdb_header->WiFi_FW_version, sizeof(wifi_fw_info->WiFi_FW_version));
        wifi_fw_info->WiFi_FW_addr = fwdb_header->WiFi_FW_offset;
        wifi_fw_info->WiFi_FW_size = fwdb_header->WiFi_FW_size;
        rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1), &fap);
        if (rc == 0)
        {
            flash_area_close(fap);
        }

        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() wifi_fw_info:WIFI_FW_version: >%s<\n", __func__, wifi_fw_info->WiFi_FW_version);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() wifi_fw_info:WIFI_FW_addr   : 0x%x\n", __func__, wifi_fw_info->WiFi_FW_addr);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() wifi_fw_info:WIFI_FW_size   : 0x%x\n\n", __func__, wifi_fw_info->WiFi_FW_size);
    }

    return CY_RSLT_SUCCESS;
}


/**
 * @brief Read WiFi FW from FW DataBlock
 *
 * @param[in]   offset - offset into data
 * @param[in]   size   - amount to copy
 * @param[in]   dest   - destination buffer for the read
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_READ_STORAGE
 */
cy_rslt_t cy_ota_fwdb_get_wifi_fw_data(uint32_t offset, uint32_t size, uint8_t *dest)
{
    cy_rslt_t                   result = CY_RSLT_SUCCESS;
    cy_ota_fwdb_wifi_fw_info_t  wifi_fw_info;
    const struct flash_area     *fap;


    result = cy_ota_fwdb_get_wifi_fw_info(&wifi_fw_info);
    if (result == CY_RSLT_SUCCESS)
    {
        /* Always read from secondary image of primary slot */
        if (flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1), &fap) != 0)
        {
            cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1) failed\r\n", __func__);
            return CY_RSLT_OTA_ERROR_OPEN_STORAGE;
        }

        /* read into the chunk_info buffer */
        if (flash_area_read(fap, offset, dest, size) != CY_RSLT_SUCCESS)
        {
            cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_read() failed \n", __func__);
            result = CY_RSLT_OTA_ERROR_READ_STORAGE;
        }
        flash_area_close(fap);
    }
    return result;
}
/**
 * @brief When FW is stored in a separate Data Block, get WiFi CLM blob info
 *
 * Use this call to get the external flash WiFi CLM blob info
 *
 * @param   clm_blob_info   - ptr to cy_ota_fwdb_clm_blob_info
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_fwdb_get_clm_blob_info(cy_ota_fwdb_clm_blob_info_t *clm_blob_info)
{
    cy_ota_fw_data_block_header_t *fwdb_header;

    if (clm_blob_info == NULL)
    {
        return CY_RSLT_OTA_ERROR_GENERAL;
    }

    fwdb_header = cy_ota_fwdb_get_base_info();
    if (fwdb_header != NULL)
    {
        const struct flash_area *fap;
        int rc;

        clm_blob_info->CLM_blob_addr = fwdb_header->CLM_blob_offset;
        clm_blob_info->CLM_blob_size = fwdb_header->CLM_blob_size;
        rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1), &fap);
        if (rc == 0)
        {
            flash_area_close(fap);
        }

        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() clm_blob_info:CLM_blob_addr   : 0x%x\n", __func__, clm_blob_info->CLM_blob_addr);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() clm_blob_info:CLM_blob_size   : 0x%x\n\n", __func__, clm_blob_info->CLM_blob_size);
    }

    return CY_RSLT_SUCCESS;

    return CY_RSLT_OTA_ERROR_GENERAL;
}

/**
 * @brief When FW is stored in a separate Data Block, get the BT FW Patch
 *
 * Use this call to get the external flash BT FW info
 *
 * @param   bt_fw_info   - ptr to cy_ota_fwdb_bt_fw_info_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_fwdb_get_bt_fw_info(cy_ota_fwdb_bt_fw_info_t *bt_fw_info)
{
    cy_ota_fw_data_block_header_t *fwdb_header;

    if (bt_fw_info == NULL)
    {
        return CY_RSLT_OTA_ERROR_GENERAL;
    }

    fwdb_header = cy_ota_fwdb_get_base_info();
    if (fwdb_header != NULL)
    {
        const struct flash_area *fap;
        int rc;

        bt_fw_info->BT_FW_version = fwdb_header->BT_FW_version;
        bt_fw_info->BT_FW_addr = fwdb_header->BT_FW_offset;
        bt_fw_info->BT_FW_size = fwdb_header->BT_FW_size;
        rc = flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1), &fap);
        if (rc == 0)
        {
            bt_fw_info->BT_FW_addr += fap->fa_off;
            flash_area_close(fap);
        }

        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() bt_fw_info:BT_FW_version: >%s<\n", __func__, bt_fw_info->BT_FW_version);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() bt_fw_info:BT_FW_addr    : 0x%x\n", __func__, bt_fw_info->BT_FW_addr);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() bt_fw_info:BT_FW_size   : 0x%x\n\n", __func__, bt_fw_info->BT_FW_size);
    }

    return CY_RSLT_SUCCESS;
}

/**
 * @brief Get BT FW for transfer to BT module
 *
 * Use this call to get the external flash BT FW Patch info
 * NOTES: This allocates RAM, Expected to be < 48k
 *        The User must call cy_ota_fwdb_free_bt_fw() after use.
 *
 * @param   bt_fw   - ptr to cy_ota_fwdb_bt_fw_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_fwdb_get_bt_fw(cy_ota_fwdb_bt_fw_t *bt_fw)
{
    cy_rslt_t result = CY_RSLT_OTA_ERROR_GENERAL;
    uint32_t  fw_buffer = 0x00;

    cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() bt_fw:%p\n", __func__, bt_fw);

    if (bt_fw == 0x00)
    {
        return CY_RSLT_OTA_ERROR_GENERAL;
    }
    memset(bt_fw, 0x00, sizeof(cy_ota_fwdb_bt_fw_t));

    cy_ota_fw_data_block_header_t *fwdb_header;
    fwdb_header = cy_ota_fwdb_get_base_info();
    if ( (fwdb_header != 0x00) && (fwdb_header->BT_FW_offset != 0x00) && (fwdb_header->BT_FW_size > 0) &&
         (fwdb_header->BT_FW_offset != 0xFF) && (fwdb_header->BT_FW_size != 0xffffffff) )
    {
        uint32_t                    malloc_size;
        const struct flash_area     *fap;
        uint32_t                    read_offset;

        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() fwdb_header:BT_FW_offset : 0x%x\n", __func__, fwdb_header->BT_FW_offset);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%s() fwdb_header:BT_FW_size   : 0x%x\n", __func__, fwdb_header->BT_FW_size);

        /* Round up size a bit */
        malloc_size = fwdb_header->BT_FW_size + (4 - (fwdb_header->BT_FW_size & 3));
        fw_buffer  =(uint32_t)mlloc(malloc_size);
        if (fw_buffer == 0x00)
        {
            return CY_RSLT_OTA_ERROR_OUT_OF_MEMORY;
        }
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "%d:%s() buf:%p\n", __LINE__, __func__, fw_buffer);
        cy_log_msg(CYLF_OTA, CY_LOG_DEBUG, "        off:0x%x malloc:0x%x sz:0x%lx\n", fwdb_header->BT_FW_offset, malloc_size, fwdb_header->BT_FW_size);

        memset( (char *)fw_buffer, 0xDE, malloc_size);

        read_offset = fwdb_header->BT_FW_offset;

        /* Always read from secondary slot */
        if (flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1), &fap) != 0)
        {
            cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_open(FLASH_AREA_IMAGE_PRIMARY(1) ) failed\r\n", __func__);
            result = CY_RSLT_OTA_ERROR_GENERAL;
        }
        else
        {
            /* read the bt fw patch into the buffer */
            if (flash_areaf_read(fap, read_offset, (uint8_t *)fw_buffer, fwdb_header->BT_FW_size) != CY_RSLT_SUCCESS)
            {
                cy_log_msg(CYLF_OTA, CY_LOG_ERR, "%s() flash_area_read() failed \n", __func__);
                result = CY_RSLT_OTA_ERROR_GENERAL;
            }
            else
            {
                cy_log_msg(CYLF_OTA, CY_LOG_DEBUG4, "%s() flash_area_read() success\n", __func__);
                result = CY_RSLT_SUCCESS;
            }
            flash_area_close(fap);
        }
    }

    if (result == CY_RSLT_SUCCESS)
    {
        bt_fw->BT_FW_version = fwdb_header->BT_FW_version;
        bt_fw->BT_FW_buffer  = (uint8_t *)fw_buffer;
        bt_fw->BT_FW_size    = fwdb_header->BT_FW_size;
    }
    else
    {
        if (fw_buffer != 0x00)
        {
            vPortFree((uint8_t *)fw_buffer);
        }
    }
    return result;
}

/**
 * @brief Free BT FW for transfer to BT module
 *
 * Use this call to free the external flash BT FW Patch info
 * NOTES: This frees RAM, Expected to be < 48k
 *
 * @param   bt_fw   - ptr to cy_ota_fwdb_bt_fw_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_OTA_ERROR_GENERAL
 */
cy_rslt_t cy_ota_fwdb_free_bt_fw(cy_ota_fwdb_bt_fw_t *bt_fw)
{
    if (bt_fw == NULL)
    {
        return CY_RSLT_OTA_ERROR_GENERAL;
    }
    if (bt_fw->BT_FW_buffer != 0x00)
    {
        cy_log_msg(CYLF_OTA, CY_LOG_NOTICE, "%d:%s() free :%p\n", __LINE__, __func__, bt_fw->BT_FW_buffer);
        vPortFree(bt_fw->BT_FW_buffer);
        bt_fw->BT_FW_buffer = 0x00;
    }

    return CY_RSLT_SUCCESS;
}

#endif
