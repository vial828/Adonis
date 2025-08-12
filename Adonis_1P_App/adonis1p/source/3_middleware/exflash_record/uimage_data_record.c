/**
  ******************************************************************************
  * @file    event_data_record.c
  * @author  Gison@metextech.com
  * @date    2024/08/21
  * @version V0.01
  * @brief   Brief description.
  *
  *   Detailed description starts here.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 SMOORE TECHNOLOGY CO.,LTD.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  * Change Logs:
  * Date            Version    Author                       Notes
  * 2024-08-21     V0.01      Gison@metextech.com    the first version
  *
  ******************************************************************************
  */
#include <stdlib.h>
#include "platform_io.h"
#include "sm_log.h"
#include "system_status.h"
#include "data_base_info.h"
#include "cy_serial_flash_qspi.h"
#include "FreeRTOS.h"

#include "uimage_decode.h"
#include "uimage_encode.h"

#include "uimage_data_record.h"

/*
 * Function Name: uimage_record_payload_upgrade
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return BOOL  BLE GATT status (FALSE or TRUE)
 */ 
bool uimage_record_payload_upgrade(uint32_t uimage_writeAddr, uint8_t* pBuf, size_t len)
{
    if ((uimage_writeAddr < UIMAGE_START_ADDR)||(uimage_writeAddr >= UIMAGE_END_ADDR)||(pBuf == NULL))//addr range judge
    {
        return FALSE;
    }

    //判断数据是否被写满(4K)
    if (uimage_writeAddr % 4096 == 0)
    {
        uimage_record_erase_sector(uimage_writeAddr,4096);//擦除 4 K, 将要写入的新扇区
    }

    sm_log(SM_LOG_DEBUG, "%s uimage_writeAddr:0x%08x len:%ld\n", __FUNCTION__, uimage_writeAddr, len);

    // ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    // Qspiflash_t ptQspiflash;

    // ptQspiflash.addr = uimage_writeAddr;//UI IMAGE SAVED ADDR
    // ptQspiflash.data = pBuf;
    // ptQspiflash.len = len;
    // qspiflashDev->write( (uint8_t*)&ptQspiflash, len);

    cy_serial_flash_qspi_write(uimage_writeAddr, len, pBuf);
	
	  return TRUE;
}

/*
 * Function Name: uimage_record_payload_read
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return BOOL  BLE GATT status (FALSE or TRUE)
 */ 
bool uimage_record_payload_read(uint32_t uimage_readAddr, uint8_t* pBuf, size_t len)
{
    if ((uimage_readAddr < UIMAGE_START_ADDR)||(uimage_readAddr >= UIMAGE_END_ADDR)||(pBuf == NULL))//addr range judge
    {
        return FALSE;
    }

    sm_log(SM_LOG_DEBUG, "%s uimage_readAddr:0x%08x len:%ld\n", __FUNCTION__, uimage_readAddr, len);

    cy_serial_flash_qspi_read(uimage_readAddr, len, pBuf);
	
	return TRUE;
}

int uimage_record_erase_sector(uint32_t address, uint16_t size)
{

    // sm_log(SM_LOG_INFO, "%s \r\n", __FUNCTION__);
    
    if (address == USER_UIMAGE_INTRO_START_ADDR)  //bug1966591
    {
        Intro_unicode_chars_len    = 0;
        memset(Intro_unicode_chars, 0, 40);
    }
    else if (address == USER_UIMAGE_GREETING_START_ADDR)
    {
        Greeting_unicode_chars_len = 0;
        memset(Greeting_unicode_chars, 0, 40);
    }
    else  if(address == USER_UIMAGE_OUTRO_START_ADDR)
    {
        Outro_unicode_chars_len    = 0;
        memset(Outro_unicode_chars, 0, 40);
    }

    cy_serial_flash_qspi_erase(address, size);

	return 0;
}

int uimage_record_erase_all(void) // 清除所有uimage 数据记录
{
  
    Intro_unicode_chars_len    = 0;
    Greeting_unicode_chars_len = 0;
    Outro_unicode_chars_len    = 0;
    memset(Intro_unicode_chars, 0, 40);
    memset(Greeting_unicode_chars, 0, 40);
    memset(Outro_unicode_chars, 0, 40);
    
    sm_log(SM_LOG_INFO, "%s \n \
                            Intro_unicode_chars_len: %d\n,\
                            Greeting_unicode_chars_len: %d\n,\
                            Outro_unicode_chars_len: %d\r\n", __FUNCTION__, Intro_unicode_chars_len, Greeting_unicode_chars_len, Outro_unicode_chars_len);

    cy_serial_flash_qspi_erase(USER_UIMAGE_INTRO_START_ADDR, 4096);//改为只擦除 4K
    cy_serial_flash_qspi_erase(USER_UIMAGE_GREETING_START_ADDR, 4096);//改为只擦除 4K
    cy_serial_flash_qspi_erase(USER_UIMAGE_OUTRO_START_ADDR, 4096);//改为只擦除 4K

	return 0;
}

/*============= =========== ======== ===== ==================*/

