/**
  ******************************************************************************
  * @file    driver_amoled.h
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

#ifndef __DRIVER_AMOLED_H
#define __DRIVER_AMOLED_H

#include "stdint.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
uint8_t amoled_brigth_get(void);
void amoled_brigth_set(uint8_t dat);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
int driver_amoled_init(void);
int driver_amoled_deinit(void);
int driver_amoled_write(uint8_t *pBuf, uint16_t len);
int driver_amoled_read(uint8_t *pBuf, uint16_t len);

#endif

