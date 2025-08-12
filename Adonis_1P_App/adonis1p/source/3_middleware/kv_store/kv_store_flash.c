/*******************************************************************************
 * File Name: app_serial_flash.c
 *
 * Description: This file contains block device function implementations
 *              required by kv-store library
 *
 * Related Document: See README.md
 *
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
 ******************************************************************************/

/*******************************************************************************
 * Header files
 ******************************************************************************/
#ifndef USE_INTERNAL_FLASH

#include "cybsp.h"
#include "mtb_kvstore.h"
#include "kv_store_flash.h"
#include "cy_retarget_io.h"

#include "data_base_info.h"
#include "platform_io.h"
#include "app_bt_char_adapter.h"

#define	ADDR_OF_BONDING_DATA		2048		// ??????,??2K??
#define	SIZE_OF_BONDING_DATA		4096		// ???? 2K
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
static uint32_t bd_read_size(void* context, uint32_t addr);
static uint32_t bd_program_size(void* context, uint32_t addr);
static uint32_t bd_erase_size(void* context, uint32_t addr);
static cy_rslt_t bd_read(void* context, uint32_t addr,
                         uint32_t length, uint8_t* buf);
static cy_rslt_t bd_program(void* context, uint32_t addr,
                            uint32_t length, const uint8_t* buf);
static cy_rslt_t bd_erase(void* context, uint32_t addr, uint32_t length);

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

/**
 * Function Name: bd_read_size
 *
 * Function Description:
 *   @brief Function to get the read size of the block device
 *          for a specific address.
 *
 *   @param  void* context : Context object that is passed into mtb_kvstore_init
             uint32_t addr : Address for which the read size is queried.
             This address is passed in as start_addr + offset.
 *
 *
 *   @return uint32_t: Read size of the memory device.
 *
 */
static uint32_t bd_read_size(void* context, uint32_t addr)
{
    (void)context;
	if (addr >= ADDR_OF_BONDING_DATA && addr <= (ADDR_OF_BONDING_DATA + SIZE_OF_BONDING_DATA))
	{
//		return SIZE_OF_BONDING_DATA - (addr - ADDR_OF_BONDING_DATA);
		return 512;
	}
	else
	{
		return 0;
	}
}

/**
 * Function Name: bd_program_size
 *
 * Function Description:
 *   @brief Function to get the program size of the block device
 *          for a specific address.
 *
 *   @param  void* context: Context object that is passed into mtb_kvstore_init
             uint32_t addr: Address for which the program size is queried.
             This address is passed in as start_addr + offset.
 *
 *
 *   @return uint32_t: Program size of the memory device.
 *
 */
static uint32_t bd_program_size(void* context, uint32_t addr)
{
	(void)context;
	if (addr >= ADDR_OF_BONDING_DATA && addr <= (ADDR_OF_BONDING_DATA + SIZE_OF_BONDING_DATA))
	{
//		return SIZE_OF_BONDING_DATA - (addr - ADDR_OF_BONDING_DATA);
		return 512;
	}
	else
	{
		return 0;
	}
}


/**
 * Function Name: bd_erase_size
 *
 * Function Description:
 *   @brief Function prototype to get the erase size of the block device
 *          for a specific address.
 *
 *   @param  void* context: Context object that is passed into mtb_kvstore_init
             uint32_t addr: Address for which the program size is queried.
             This address is passed in as start_addr + offset.
 *
 *
 *   @return uint32_t Erase size of the memory device.
 *
 */
static uint32_t bd_erase_size(void* context, uint32_t addr)
{
    (void)context;
	(void)addr;
	return 512;
}

/**
 * Function Name: bd_read
 *
 * Function Description:
 *   @brief Function for reading data from the block device.
 *
 *   @param void* context   : Context object that is passed into mtb_kvstore_init
 *   @param uint32_t addr   : Address to read the data from the block device
 *          This address is passed in as start_addr + offset
 *   @param uint32_t length : Length of the data to be read into the buffer
 *   @param uint8_t* buf    : Buffer to read the data.
 *
 *
 *   @return cy_rslt_t: Result of the read operation.
 *
 */
static cy_rslt_t bd_read(void* context, uint32_t addr,
                         uint32_t length, uint8_t* buf)
{
    (void)context;
    // len~{R*4sSZ~}0
	if (NULL == buf || 0 == length)
    {
        return -1;
    }
		
	if (addr >= ADDR_OF_BONDING_DATA && (addr + length) <= (ADDR_OF_BONDING_DATA + SIZE_OF_BONDING_DATA))
	{
//		ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
//		E2PDataType_t e2pTemp;
		
//		e2pTemp.addr = addr;
//		e2pTemp.pDate = buf;
//		e2pTemp.size = length;
//		e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));

		Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
		uint8_t * pBuf = pBleData->ble_bonding_data;
		pBuf += (addr - ADDR_OF_BONDING_DATA);
		memcpy(buf, pBuf, length);
	    return 0;
	}

	return -1;
	
}

/**
 * Function Name: bd_program
 *
 * Function Description:
 * @brief
 *
 * @param void* context   : Context object that is passed into mtb_kvstore_init
 * @param uint32_t addr   : Address to program the data into the block device.
 *        This address is passed in as start_addr + offset
 * @param uint32_t length : Length of the data to be written
 * @param uint8_t* buf    : Data that needs to be written
 *
 * @return cy_rslt_t      : Result of the program operation.
 *
 */
static cy_rslt_t bd_program(void* context, uint32_t addr,
                            uint32_t length, const uint8_t* buf)
{
    (void)context;
    // len~{R*4sSZ~}0
	if (NULL == buf || 0 == length)
    {
        return -1;
    }
	
//  sm_log(SM_LOG_DEBUG, "bd_program, addr:%ld, len:%ld, buf:", addr, length);
	#if 0
    for(uint32_t i=0; i<length; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", buf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	if (addr >= ADDR_OF_BONDING_DATA && (addr + length) <= (ADDR_OF_BONDING_DATA + SIZE_OF_BONDING_DATA))
	{
//		ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
//		E2PDataType_t e2pTemp;
//		
//		e2pTemp.addr = addr;
//		e2pTemp.pDate = buf;
//		e2pTemp.size = length;
//		e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));

		Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
		uint8_t * pBuf = pBleData->ble_bonding_data;
		pBuf += (addr - ADDR_OF_BONDING_DATA);
		memcpy(pBuf, buf, length);
//		app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
		set_update_BleData_status(1);

	    return 0;
	}

	return -1;
	
}


/**
 * Function Name: bd_erase
 *
 * Function Description:
 *   @brief
 *
 *   @param context        : Context object that is passed into mtb_kvstore_init
 *   @param uint32_t addr  : Address to erase the data from the device.
 *          This address is passed in as start_addr + offset
 *   @param uint32_t length: Length of the data that needs to be erased
 *
 *
 * @return cy_rslt_t       : Result of the erase operation.
 *
 */
static cy_rslt_t bd_erase(void* context, uint32_t addr, uint32_t length)
{
    (void)context;

//  sm_log(SM_LOG_DEBUG, "bd_erase, addr:%ld, len:%ld", addr, length);

	if (0 == length)
	{
		return -1;
	}
		
	if (addr >= ADDR_OF_BONDING_DATA && (addr + length) <= (ADDR_OF_BONDING_DATA + SIZE_OF_BONDING_DATA))
	{
		Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
		uint8_t * pBuf = pBleData->ble_bonding_data;
		pBuf += (addr - ADDR_OF_BONDING_DATA);
		memset(pBuf, 0xff, length);
//		app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
		set_update_BleData_status(1);

		return 0;
	}

	return -1;
}

/**
 * Function Name: app_kvstore_bd_config
 *
 * Function Description:
 *   @brief  This function provides the pointer to the implemented
 *           prototype function for the block device.
 *
 *   @param  mtb_kvstore_bd_t : Block device interface
 *
 *   @return void
 *
 */
void app_kvstore_bd_config(mtb_kvstore_bd_t* device)
{
    device->read         = bd_read;
    device->program      = bd_program;
    device->erase        = bd_erase;
    device->read_size    = bd_read_size;
    device->program_size = bd_program_size;
    device->erase_size   = bd_erase_size;
    device->context      = NULL;
}

/**
 * Function Name: app_kvstore_bd_init
 *
 * Function Description:
 *   @brief  This function initializes the underlying block device
 *           (in this case external flash).
 *
 *   @param void
 *   @return void
 *
 */
//void app_kvstore_bd_init(void)
//{
//
//}
/**
 * Function Name: get_kvstore_init_params
 *
 * Function Description:
 *   @brief  This function is used to define the bond data storage
 *         (in this case external flash).
 *
 *   @param uint32_t *start_addr: Start Address to erase the data
 *                                from the device.
 *   @param uint32_t *length    : Length of the data that needs to be erased
 *
 *   @return void
 *
 */
void get_kvstore_init_params(uint32_t *length, uint32_t *start_addr)
{
	// ~{5XV7N*Ac#,DZ2?:/J}4&@mFpJ<5XV7~}
	*start_addr = ADDR_OF_BONDING_DATA;//9.5KB
    *length = SIZE_OF_BONDING_DATA;//4096
}

#endif
/* END OF FILE [] */
