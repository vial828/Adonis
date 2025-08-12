/**
  ******************************************************************************
  * @file    driver_param.c
  * @author  xuhua.huang@metextech.com
  * @date    2024/03/013
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
  * 2024-03-013     V0.01      xuhua.huang@metextech.com    the first version
  *
  ******************************************************************************
  */

#include "stdio.h"
#include "string.h"
#include "driver_param.h"

#define ENABLE_DRIVER_PARAM
#ifdef ENABLE_DRIVER_PARAM
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "driver_mcu_eeprom.h"
#include "platform_io.h"
#else
//定义参数存储地址及偏移
#define PARAM_BASE      (MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - (2 * MCU_FLASH_ROW_SIZE)) // 最后一行为文件信息，参数放在倒数第二行

#define PARAM_START_ADDR  PARAM_BASE
#define PARAM_END_ADDR    ((uint32_t)(PARAM_BASE + (2 * MCU_FLASH_ROW_SIZE))) // 大于或等于这个地址，判断为非法

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#endif

/**
  * @brief  初始化配置参数
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_param_init(void)
{
    return 0;
}

/**
  * @brief  读配置参数
  * @param  pBbuf:  要读入的数据
            len:    要读入数据的长度
  * @return 0：成功，-1：失败
  * @note
  */
int driver_param_read(uint8_t *pBbuf, uint16_t len)
{
#if 0
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    // len要大于0
    if (len==0 || len > E2P_SIZE_OF_HEAT_PARAM)
    {
        return -1;
    }
    if (pBbuf == NULL)
    {
        return -1;
    }
    // 此处添加读取EEPROM API接口

	ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
	E2PDataType_t e2pTemp;
	
	e2pTemp.addr = E2P_ADDR_OF_HEAT_PARAM;
	e2pTemp.pDate = pBbuf;
	e2pTemp.size = len;
	e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));
	
    return 0;

#else

    uint32_t programAddr = PARAM_START_ADDR;
    uint32_t i = 0;
    // len要大于0
    if (len==0)
    {
        return -1;
    }
    if (pBbuf == NULL)
    {
        return -1;
    }
    // 此处添加写进FLASH API接口
    while (i < len)
    {
        uint32_t val = *(__IO uint32_t *)programAddr;
        if (i + 4 > len) { // 防止buf访问越界
          memcpy(&pBbuf[i], (uint8_t*)&val, len - i);
        } else {
          memcpy(&pBbuf[i], (uint8_t*)&val, sizeof(uint32_t));
        }
        i += 4;
        programAddr = programAddr + 4;
    }
    return 0;    
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#endif
}

/**
  * @brief  写配置参数
  * @param  pBuf:   要写入的数据
            len:    要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   None 20240620 Eason 改为使用1K,最大不能超过1K
  */
int driver_param_write(uint8_t *pBuf, uint16_t len)
{
#if 0
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
    if (len==0 || len > E2P_SIZE_OF_HEAT_PARAM)
    {
        return -1;
    }
    if (pBuf == NULL)
    {
        return -1;
    }
    // 写 

	ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
	E2PDataType_t e2pTemp;
	
	e2pTemp.addr = E2P_ADDR_OF_HEAT_PARAM;
	e2pTemp.pDate = pBuf;
	e2pTemp.size = len;
	e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));

    return 0;		
#else

    uint8_t temBuf[MCU_FLASH_ROW_SIZE] = {0};

    const uint32_t * row_ptr = NULL;

    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
    if (len==0)
    {
        return -1;
    }
    if (pBuf == NULL)
    {
        return -1;
    }
    // 擦除 2个 PAGE
    Cy_Flash_EraseRow(PARAM_START_ADDR);
    Cy_Flash_EraseRow(PARAM_START_ADDR + MCU_FLASH_ROW_SIZE);
    // 写 
    row_ptr = (const uint32_t *) temBuf;
    if(len <= MCU_FLASH_ROW_SIZE)// 只写1个PAGE 512 字节
    {
        memcpy(temBuf,pBuf,len);
        Cy_Flash_WriteRow(PARAM_START_ADDR, row_ptr);
    }else{
        memcpy(temBuf,pBuf,MCU_FLASH_ROW_SIZE);
        Cy_Flash_WriteRow(PARAM_START_ADDR, row_ptr);// 写第一页

        memset(temBuf,0x00,sizeof(temBuf));
        memcpy(temBuf,&pBuf[MCU_FLASH_ROW_SIZE],len - MCU_FLASH_ROW_SIZE);
        Cy_Flash_WriteRow(PARAM_START_ADDR + MCU_FLASH_ROW_SIZE, row_ptr);//写第二页
    }

    return 0;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#endif
}

