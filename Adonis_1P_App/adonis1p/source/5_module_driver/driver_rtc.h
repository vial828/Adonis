/**
  ******************************************************************************
  * @file    driver_tick.h
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
  * 2024-07-30      V0.01      vincent.he@metextech.com    the first version
  *
  ******************************************************************************
  */

#ifndef __DRIVER_RTC_H
#define __DRIVER_RTC_H

#include "stdint.h"
#include "sm_log.h"

#define DRV_RTC_LOG_INFO SM_LOG_INFO //SM_LOG_OFF SM_LOG_INFO

int driver_rtc_init(void);
int driver_rtc_read(uint8_t *pBuf, uint16_t len);
int driver_rtc_deinit(void);
int driver_rtc_write(uint8_t *pBuf, uint16_t len);
#endif 

