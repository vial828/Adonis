/**
  ******************************************************************************
  * @file    driver_idle.c
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
#include "driver_idle.h"
#include "cy_pdl.h"

#define ENABLE_DRIVER_IDLE
#ifdef ENABLE_DRIVER_IDLE

#endif

/**
  * @brief  待机初始化
  * @param  None
  * @return None
  * @note   预留设计，暂没用
  */
int driver_idle_init(void)
{
    return 0;
}

/**
  * @brief  进入待机休眠
  * @param  pBuf:   要写入的数据
            len:    要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_idle_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_IDLE
    // 调用休眠API
    // 这里需要区分是普通待机模式还是船运模式
#endif
    return 0;
}

int driver_idle_read(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_IDLE
    if (CY_SYSLIB_RESET_HIB_WAKEUP == Cy_SysLib_GetResetReason()) {
        pBuf[0] = 1;
    } else {
        pBuf[0] = 0;
    }
#endif
    return 0;
}

//#include "cy_pdl.h"
#define SFLASH_NAR_ADDR_OFFSET	    (0x1A00)
//#define SFLASH_UASER_ADDR_OFFSET	(0x800)

void cy_disable_debug(void)
{
	 cy_en_flashdrv_status_t fstatus;
	 uint8_t databuf[512];
	 const uint32_t baseSFlashAddress	= CY_SFLASH_BASE;
	 uint32_t address = baseSFlashAddress + SFLASH_NAR_ADDR_OFFSET;

	 memset(databuf, 0, 512);
     memcpy(databuf, (const void *)address, 512);

     databuf[0] = 0x07;
     databuf[1] = 0x00;

     fstatus =  Cy_Flash_WriteRow(address, (uint32_t*)databuf);
     if(fstatus != CY_FLASH_DRV_SUCCESS)
     {

     }
}

