/******************************************************************************
* File Name:   app_bt_bonding.c
*
* Description: This is the source code for bonding implementation using kv-store
*              library.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2021-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
*        Header Files
*******************************************************************************/
#include "wiced_bt_stack.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include <FreeRTOS.h>
#include <task.h>
#include "cycfg_bt_settings.h"
#include "app_bt_bonding.h"
#include "mtb_kvstore.h"
#include "app_bt_utils.h"
#include "cy_serial_flash_qspi.h"
// #include "app_bt_event_handler.h"
#include <inttypes.h>
#include "stdio.h"
#include "string.h"
#include "driver_param.h"
#include "kv_store_flash.h"

#include "sm_log.h"

#include "ota.h"
#include "app_bt_utils.h"
#include "app_bt_char.h"
#include "app_bt_char_adapter.h"

#include "cycfg_gap.h"

/*******************************************************************
 * Variable Definitions
 ******************************************************************/
/*Kvstore block device*/
mtb_kvstore_bd_t    block_device;
mtb_kvstore_t  kvstore_obj;
bond_info_t    bond_info;
wiced_bt_local_identity_keys_t identity_keys;
uint16_t       peer_cccd_data[BOND_INDEX_MAX];

/*******************************************************************************
 *                              FUNCTION DEFINITIONS
 ******************************************************************************/

/**
* Function Name:
* app_kv_store_init
*
* Function Description:
* @brief   This function initializes the block device and kv-store library
*
* @param   None
*
* @return  None
*/

void app_kv_store_init(void)
{
    cy_rslt_t rslt;
    uint32_t  start_addr, length;

    get_kvstore_init_params(&length, &start_addr);

    sm_log(SM_LOG_DEBUG, "%s \n", __FUNCTION__);
#if 0
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_BAD_PARAM_ERROR: %d\n", MTB_KVSTORE_BAD_PARAM_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_ALIGNMENT_ERROR: %d\n", MTB_KVSTORE_ALIGNMENT_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_MEM_ALLOC_ERROR: %d\n", MTB_KVSTORE_MEM_ALLOC_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_INVALID_DATA_ERROR: %d\n", MTB_KVSTORE_INVALID_DATA_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_ERASED_DATA_ERROR: %d\n", MTB_KVSTORE_ERASED_DATA_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_ITEM_NOT_FOUND_ERROR: %d\n", MTB_KVSTORE_ITEM_NOT_FOUND_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_STORAGE_FULL_ERROR: %d\n", MTB_KVSTORE_STORAGE_FULL_ERROR);
    sm_log(SM_LOG_DEBUG, "MTB_KVSTORE_BUFFER_TOO_SMALL: %d\n", MTB_KVSTORE_BUFFER_TOO_SMALL);
#endif

    /*Initialize kv-store library*/
    rslt = mtb_kvstore_init(&kvstore_obj, start_addr, length, &block_device);

    sm_log(SM_LOG_DEBUG, "mtb_kvstore_init().rslt: %d\n", rslt);
    /*Check if the kv-store initialization was successful*/
    if (CY_RSLT_SUCCESS != rslt)
    {
        sm_log(SM_LOG_DEBUG, "failed to initialize kv-store \n");
        CY_ASSERT(0);
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "succeed to initialize kv-store \n");
    }
}

/**
* Function Name:
* app_kv_store_deinit
*
* Function Description:
* @brief   
*
* @param   None
*
* @return  None
*/
void app_kv_store_deinit(void)
{
    /*deinit kv-store library*/
    mtb_kvstore_deinit(&kvstore_obj);
}

void app_kv_store_test(void)
{
    #if 1   //for test
    cy_rslt_t rslt = CY_RSLT_SUCCESS;
    
    uint8_t data_buf[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t temp[10] = {0};
    uint32_t data_size = sizeof(data_buf);

    //create and write
    // rslt = mtb_kvstore_write(&kvstore_obj, "test", (uint8_t *)data_buf, sizeof(data_buf));
    // if (CY_RSLT_SUCCESS != rslt)
    //     sm_log(SM_LOG_DEBUG, "failed to write :%d\n", rslt);
    //read
    rslt = mtb_kvstore_read(&kvstore_obj, "test", (uint8_t *)temp, &data_size);
    sm_log(SM_LOG_DEBUG, "read data: ");
    for(uint8_t i=0; i<10; i++)
        sm_log(SM_LOG_DEBUG, " %02x", temp[i]);
    if (CY_RSLT_SUCCESS != rslt)
        sm_log(SM_LOG_DEBUG, "failed to read :%d\n", rslt);
    //delete
    rslt = mtb_kvstore_delete(&kvstore_obj, "test");
    if (rslt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "Identity keys remove failure in the flash!, deinit kv_store.\n");
        
        /*kv_store deinit*/
        app_kv_store_deinit();
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "Identity keys remove success in the flash!\n");
    }
    #endif 
}

/**
* Function Name:
* app_bt_restore_bond_data
*
* Function Description:
* @brief  This function restores the bond information from the Flash
*
* @param   None
*
* @return  cy_rslt_t: CY_RSLT_SUCCESS if the restoration was successful,
*                         an error code otherwise.
*
*/
cy_rslt_t app_bt_restore_bond_data(void)
{
    /* Read and restore contents of flash */
    uint32_t data_size = sizeof(bond_info);
    cy_rslt_t rslt = mtb_kvstore_read(&kvstore_obj, "bond_data", (uint8_t *)&bond_info, &data_size);
    if (rslt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "\n Bond data not present in the flash!\r\n");
        return rslt;
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "\n Bond data present in the flash, bond_info.link_keys[0].key_data(%d): \r\n",sizeof(wiced_bt_device_sec_keys_t));
        print_array(&bond_info.link_keys[0].key_data, sizeof(wiced_bt_device_sec_keys_t));

        sm_log(SM_LOG_DEBUG, "\n bond_info.link_keys[0].key_data.le_keys.pltk(%d): \r\n",sizeof(bond_info.link_keys[0].key_data.le_keys.pltk));
        print_array(&bond_info.link_keys[0].key_data.le_keys.pltk, sizeof(bond_info.link_keys[0].key_data.le_keys.pltk));
        sm_log(SM_LOG_DEBUG, "\n bond_info.link_keys[0].key_data.le_keys.lltk(%d): \r\n",sizeof(bond_info.link_keys[0].key_data.le_keys.lltk));
        print_array(&bond_info.link_keys[0].key_data.le_keys.lltk, sizeof(bond_info.link_keys[0].key_data.le_keys.lltk));
        sm_log(SM_LOG_DEBUG, "\r\n");
    }
    return rslt;
}


/**
* Function Name:
* app_bt_update_bond_data
*
* Function Description:
* @brief This function updates the bond information in the Flash
*
* @param   None
*
* @return  cy_rslt_t: CY_RSLT_SUCCESS if the update was successful,
*              an error code otherwise.
*
**/
cy_rslt_t app_bt_update_bond_data(void)
{
#if 1   //debug
    sm_log(SM_LOG_DEBUG, "\n update_bond_data, bond_info(%d): \r\n", sizeof(wiced_bt_device_sec_keys_t));
    print_array(&bond_info.link_keys[0].key_data, sizeof(wiced_bt_device_sec_keys_t));
#endif

    cy_rslt_t rslt = CY_RSLT_TYPE_ERROR;
    rslt = mtb_kvstore_write(&kvstore_obj, "bond_data", (uint8_t *)&bond_info, sizeof(bond_info));
    if (CY_RSLT_SUCCESS != rslt)
    {
        sm_log(SM_LOG_DEBUG, "Flash Write Error,Error code: %" PRIu32 "\n", rslt);
    }
    return rslt;
}

extern void app_bt_adv_stop(void);
/**
* Function Name:
* app_bt_delete_bond_info
*
* Function Description:
* @brief  This deletes the bond information from the Flash
*
* @param  None
*
* @return  cy_rslt_t: CY_RSLT_SUCCESS if the deletion was successful,
*              an error code otherwise.
*
*/
cy_rslt_t app_bt_delete_bond_info(void)
{
    cy_rslt_t rslt = CY_RSLT_SUCCESS;

    if (app_server_context.bt_conn_id != 0) //没有连接，才能删除绑定信息
    {
        rslt = CY_RSLT_TYPE_ERROR;
        return rslt;
    }

    sm_log(SM_LOG_DEBUG,"%s, bond_info.slot_data[NUM_BONDED]: %d\r\n", __FUNCTION__, bond_info.slot_data[NUM_BONDED]);

    for (uint8_t i = 0; i < bond_info.slot_data[NUM_BONDED]; i++)
    {
        wiced_result_t result = app_bt_delete_device_info(i);
        if (WICED_BT_SUCCESS != result)
        {
            rslt = CY_RSLT_TYPE_ERROR;
            return rslt;
        }
    }

    /*Update the slot data*/
    bond_info.slot_data[NUM_BONDED]=0;
    bond_info.slot_data[NEXT_FREE_INDEX]=0;

    /*Update bond information*/
    rslt = app_bt_update_bond_data();

    /*delete bond_data obj*/
    rslt = mtb_kvstore_delete(&kvstore_obj, "bond_data");
    if (rslt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "Identity keys remove failure in the flash!, deinit kv_store.\n");
        
        /*kv_store deinit*/
        app_kv_store_deinit();
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "Identity keys remove success in the flash, stop adv and clear remote mac!\n");
        //stop adv
        app_bt_adv_stop(); //bug2016679
        //clear pairing mac
        memset(cy_bt_remote_address, 0, sizeof(cy_bt_remote_address));
    }
    return rslt;
}

/**
* Function Name:
* app_bt_delete_device_info
*
* Function Description:
* @brief  This function deletes the bond information of the device from the RAM
*         and address resolution database.
*
* @param  index: Index of the device whose data is to be deleted
*
* @return  wiced_result_t: WICED_BT_SUCCESS if the deletion was successful,
*                   an error code otherwise.
*
*/
wiced_result_t app_bt_delete_device_info(uint8_t index)
{
    wiced_result_t result = WICED_BT_SUCCESS;
    
    sm_log(SM_LOG_DEBUG,"%s\r\n", __FUNCTION__);
    
    /* Remove device from address resolution database */
    result = wiced_bt_dev_remove_device_from_address_resolution_db(&(bond_info.link_keys[index]));
    if (WICED_BT_SUCCESS != result)
    {
        sm_log(SM_LOG_DEBUG," wiced_bt_dev_remove_device_from_address_resolution_db failure!\r\n");
        return result;
    }
    else
    {
        sm_log(SM_LOG_DEBUG," wiced_bt_dev_remove_device_from_address_resolution_db successful!\r\n");
    }

    /* Remove from the bonded device list */
    result = wiced_bt_dev_delete_bonded_device(bond_info.link_keys[index].bd_addr);
    if(WICED_BT_SUCCESS != result)
    {
        sm_log(SM_LOG_DEBUG," wiced_bt_dev_delete_bonded_device failure!\r\n");
        return result;
    }
    else
    {
        sm_log(SM_LOG_DEBUG," wiced_bt_dev_delete_bonded_device successful!\r\n");
    }

    /* Remove bonding information in RAM */
    peer_cccd_data[index]=0;
    bond_info.privacy_mode[index]=0;
    memset(&bond_info.link_keys[index], 0, sizeof(wiced_bt_device_link_keys_t));
    memset(&bond_info, 0, sizeof(bond_info));

    return result;
}

/**
* Function Name:
* app_bt_update_slot_data
*
* Function Description:
* @brief  This function updates the slot data in the Flash
*
* @param  None
*
* @return cy_rslt_t: CY_RSLT_SUCCESS if the update was successful,
*              an error code otherwise.
*/
cy_rslt_t app_bt_update_slot_data(void)
{
    cy_rslt_t rslt = CY_RSLT_TYPE_ERROR;
    /* Increment number of bonded devices and next free slot and save them in Flash */
    if (BOND_INDEX_MAX > bond_info.slot_data[NUM_BONDED])
    {
        /* Increment only if the bonded devices are less than BOND_INDEX_MAX */
        bond_info.slot_data[NUM_BONDED]++;
    }
    /* Update Next Slot to be used for next incoming Device */
    bond_info.slot_data[NEXT_FREE_INDEX] = (bond_info.slot_data[NEXT_FREE_INDEX] + 1) % BOND_INDEX_MAX;
    rslt = app_bt_update_bond_data();
    return rslt;
}

/**
* Function Name:
* app_bt_save_device_link_keys
*
* Function Description:
* @brief This function saves peer device link keys to the Flash
*
* @param link_key: Save link keys of the peer device.
*
* @return cy_rslt_t: CY_RSLT_SUCCESS if the save was successful,
*              an error code otherwise.
*
*/
cy_rslt_t app_bt_save_device_link_keys(wiced_bt_device_link_keys_t *link_key)
{
    sm_log(SM_LOG_DEBUG,"%s \r\n", __FUNCTION__);
    
    cy_rslt_t rslt = CY_RSLT_TYPE_ERROR;
    uint8_t index = NUM_BONDED;//setting index default value to 0(NUM_BONDED).Just save one mobile_phone bonding information

    /* Check if there is an entry of keys for the peer BDA in NVRAM */
    index = app_bt_find_device_in_flash(link_key->bd_addr);
    if(index != BOND_INDEX_MAX)
    {
        memcpy(&bond_info.link_keys[index],//NUM_BONDED
               (uint8_t *)(link_key), sizeof(wiced_bt_device_link_keys_t));

        rslt = mtb_kvstore_write(&kvstore_obj, "bond_data", (uint8_t *)&bond_info, sizeof(bond_info));
        if (CY_RSLT_SUCCESS != rslt)
        {
            sm_log(SM_LOG_DEBUG, "Flash Write Error,Error code: %" PRIu32 "\n", rslt );
        }
    }
    /* If there is no entry of keys in NVRAM, create a fresh entry in next free slot */
    else
    {
        sm_log(SM_LOG_DEBUG, "%s ,no entry of keys in NVRAM\n", __func__);

        memcpy(&bond_info.link_keys[NUM_BONDED],//bond_info.slot_data[NEXT_FREE_INDEX]
               (uint8_t *)(link_key), sizeof(wiced_bt_device_link_keys_t));

        rslt = mtb_kvstore_write(&kvstore_obj, "bond_data", (uint8_t *)&bond_info, sizeof(bond_info));
        if (CY_RSLT_SUCCESS != rslt)
        {
            sm_log(SM_LOG_DEBUG, "Flash Write Error,Error code: %" PRIu32 "\n", rslt );
        }
    }

    return rslt;
}

/**
* Function Name:
* app_bt_save_local_identity_key
*
* Function Description:
* @briefThis function saves local device identity keys to the Flash
*
* @param id_key: Local identity keys to store in the flash.
*
* @return cy_rslt_t: CY_RSLT_SUCCESS if the save was successful,
*              an error code otherwise.
*
*/
cy_rslt_t app_bt_save_local_identity_key(wiced_bt_local_identity_keys_t id_key)
{
    memcpy(&identity_keys, (uint8_t *)&(id_key), sizeof(wiced_bt_local_identity_keys_t));
    cy_rslt_t rslt = mtb_kvstore_write(&kvstore_obj, "local_irk", (uint8_t *)&identity_keys, sizeof(wiced_bt_local_identity_keys_t));
    if (CY_RSLT_SUCCESS == rslt)
    {
        sm_log(SM_LOG_DEBUG, "Local identity Keys saved to Flash \n");
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "Flash Write Error,Error code: %" PRIu32 "\n", rslt );
    }

    return rslt;
}

/**
* Function Name:
* app_bt_read_local_identity_keys
*
* Function Description:
* @brief This function reads local device identity keys from the Flash
*
* @param None
*
* @return cy_rslt_t: CY_RSLT_SUCCESS if the read was successful,
*              an error code otherwise.
*
*/
cy_rslt_t app_bt_read_local_identity_keys(void)
{
    uint32_t data_size = sizeof(identity_keys);
    cy_rslt_t rslt = mtb_kvstore_read(&kvstore_obj, "local_irk", NULL, &data_size);
    if (rslt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "New Keys need to be generated! \n");
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "Identity keys are available in the database.\n");
        rslt = mtb_kvstore_read(&kvstore_obj, "local_irk", (uint8_t *)&identity_keys, &data_size);
        sm_log(SM_LOG_DEBUG, "Local identity keys read from Flash, local_irk(%d): \n", data_size);
    }
    return rslt;
}

/**
* Function Name:
* app_bt_delete_local_identity_keys
*
* Function Description:
* @brief This function delete local device identity keys from the Flash
*
* @param None
*
* @return cy_rslt_t: CY_RSLT_SUCCESS if the read was successful,
*              an error code otherwise.
*
*/
cy_rslt_t app_bt_delete_local_identity_keys(void)
{
    cy_rslt_t rslt;
    uint32_t data_size = sizeof(identity_keys);
    uint8_t temp[100] = {0};

    sm_log(SM_LOG_DEBUG,"%s, local_irk_size: \r\n", __FUNCTION__, data_size);

    //delete
    rslt = mtb_kvstore_delete(&kvstore_obj, "local_irk");
    if (rslt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "Identity keys remove success in the flash!\n");
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "Identity keys remove failure in the flash!\n");
    }

    //read and verify
    rslt = mtb_kvstore_read(&kvstore_obj, "local_irk", (uint8_t *)temp, &data_size);
    sm_log(SM_LOG_DEBUG, "local_irk read data: ");
    for(uint8_t i=0; i<data_size; i++)
        sm_log(SM_LOG_DEBUG, " %02x", temp[i]);
    if (CY_RSLT_SUCCESS != rslt)
        sm_log(SM_LOG_DEBUG, " failed to read :%d\n", rslt);

    return rslt;
}

/**
* Function Name:
* app_bt_update_cccd
*
* Function Description:
* @brief  This function updates the CCCD data in the Flash
*
* @param  cccd: cccd value to be updated in flash
* @param  index: Index of the device in the flash
*
* @return cy_rslt_t: CY_RSLT_SUCCESS if the update was successful,
*              an error code otherwise.
*/
cy_rslt_t app_bt_update_cccd(uint16_t cccd, uint8_t index)
{
    cy_rslt_t rslt = CY_RSLT_TYPE_ERROR;
    peer_cccd_data[index]= cccd;
    sm_log(SM_LOG_DEBUG, "Updating CCCD Value to: %d \n",cccd);
    rslt = mtb_kvstore_write(&kvstore_obj, "cccd_data", (uint8_t *)&peer_cccd_data, sizeof(peer_cccd_data));
    return rslt;
}

/**
* Function Name:
* app_bt_restore_cccd
*
* Function Description:
* @brief This function restores the cccd from the Flash
*
* @param   None
*
* @return  cy_rslt_t: CY_RSLT_SUCCESS if the update was successful,
*              an error code otherwise.
*
**/
cy_rslt_t app_bt_restore_cccd(void)
{
    cy_rslt_t rslt = CY_RSLT_TYPE_ERROR;
    uint32_t data_size = sizeof(peer_cccd_data);
    rslt = mtb_kvstore_read(&kvstore_obj, "cccd_data", (uint8_t *)peer_cccd_data, &data_size);
    return rslt;
}

/**
* Function Name:
* app_bt_find_device_in_flash
*
* Function Description:
* @brief This function searches provided bd_addr in bonded devices list
*
* @param *bd_addr: pointer to the address of the device to be searched
*
* @return uint8_t: Index of the device in the bond data stored in the flash if found,
*            else returns  BOND_INDEX_MAX to indicate the device was not found.
*
*/
uint8_t app_bt_find_device_in_flash(uint8_t *bd_addr)
{
    uint8_t index =  BOND_INDEX_MAX; /*Return out of range value if device is not found*/
    for (uint8_t count = 0; count < bond_info.slot_data[NUM_BONDED]; count++)
    {
        if (0 == memcmp(&(bond_info.link_keys[count].bd_addr), bd_addr, sizeof(wiced_bt_device_address_t)))
        {
            sm_log(SM_LOG_DEBUG, "Found device in the flash!\n");
            index = count;
            break; /* Exit the loop since we found what we want */
        }
    }
    return(index);
}

uint8_t app_bt_find_device_in_flash_RPA(uint8_t *bd_addr)
{
    uint8_t index =  BOND_INDEX_MAX; /*Return out of range value if device is not found*/
    for (uint8_t count = 0; count < bond_info.slot_data[NUM_BONDED]; count++)
    {
        if (0 == wiced_ble_private_device_address_resolution(bd_addr, bond_info.link_keys[count].key_data.le_keys.irk))//判断该地址是否为可解析私有地址
        {
            sm_log(SM_LOG_DEBUG, "Found device index(%d) in the flash_RPA!\n", count);
            index = count;
            break; /* Exit the loop since we found what we want */
        }
    }
    return(index);
}

/**
* Function Name:
* app_bt_add_devices_to_address_resolution_db
*
* Function Description:
* @brief This function adds the bonded devices to address resolution database
*
* @param  None
*
* @return None
*
*/
void app_bt_add_devices_to_address_resolution_db(void)
{
    sm_log(SM_LOG_DEBUG, "%s, bond_info.slot_data[NUM_BONDED]:%d\n", __FUNCTION__, bond_info.slot_data[NUM_BONDED]);

    /* Copy in the keys and add them to the address resolution database */
    for (uint8_t i = 0; (i < bond_info.slot_data[NUM_BONDED]) && (i < BOND_INDEX_MAX); i++)
    {
        /* Add device to address resolution database */
        wiced_result_t result = wiced_bt_dev_add_device_to_address_resolution_db(&bond_info.link_keys[i]);
        if (WICED_BT_SUCCESS == result)
        {
            sm_log(SM_LOG_DEBUG, "Device added to address resolution database: ");
            print_bd_address((uint8_t *)&bond_info.link_keys[i].bd_addr);
            memcpy(cy_bt_remote_address, (uint8_t *)&bond_info.link_keys[i].bd_addr, 6);//when device reboot
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "Error adding device to address resolution database, Error Code %d \n", result);
        }
    }
}

/**
* Function Name:
* print_bond_data
*
* Function Description:
* @brief This function prints the bond data stored in the Flash
*
* @param None
*
* @return None
*
*/
void print_bond_data()
{
    for (uint8_t i = 0; i < bond_info.slot_data[NUM_BONDED]; i++)
    {
        sm_log(SM_LOG_DEBUG, "Slot: %d",i+1);
        sm_log(SM_LOG_DEBUG, "Device Bluetooth Address: ");
        print_bd_address(bond_info.link_keys[i].bd_addr);
        sm_log(SM_LOG_DEBUG, "Device Keys: \n");
        print_array(&(bond_info.link_keys[i].key_data), sizeof(wiced_bt_device_sec_keys_t));
        sm_log(SM_LOG_DEBUG, "\n");
    }
}

wiced_bool_t app_bt_is_existed_bonded_devices(void)
{
	return (bond_info.slot_data[NUM_BONDED] !=0) ;
}

/* END OF FILE [] */
