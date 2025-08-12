/**
  ******************************************************************************
  * @file    driver_wdg.c
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
#include "driver_wdg.h"

#define ENABLE_DRIVER_WDG
#include "cyhal_wdt.h"
#ifdef ENABLE_DRIVER_WDG
cyhal_wdt_t wdt_obj;
#endif

/**
  * @brief  初始化看门狗
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_wdg_init(void)
{
#ifdef ENABLE_DRIVER_WDG

//    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
//    cyhal_wdt_free(&wdt_obj);

    cy_rslt_t result;
    
    /* Initialize the WDT */
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    result = cyhal_wdt_init(&wdt_obj, 5000); // 看门狗定时5秒
#else    
    result = cyhal_wdt_init(&wdt_obj, 5000); // 看门狗定时5秒
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    
    /* WDT initialization failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
#endif
    return 0;
}

/**
  * @brief  去初始化看门狗
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_wdg_deinit(void)
{
#ifdef ENABLE_DRIVER_WDG
//    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdt_obj);
#endif
    return 0;
}

/**
  * @brief 喂狗
  * @param pBbuf:  要发送的数据
           len:    要发送数据的长度
  * @return 0-成功；-1-失败；
  * @note None
  */
int driver_wdg_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_WDG
    cyhal_wdt_kick(&wdt_obj);
#endif
    return 0;
}

