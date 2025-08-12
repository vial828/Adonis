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
  * 2024-03-013     V0.01      xuhua.huang@metextech.com    the first version
  *
  ******************************************************************************
  */

#ifndef __DRIVER_TICK_H
#define __DRIVER_TICK_H

#include "stdint.h"

int driver_mstick_init(void);
int driver_100ustick_init(void);
int driver_10ustick_init(void);
int driver_mstick_deinit(void);
int driver_100ustick_deinit(void);
int driver_10ustick_deinit(void);
int driver_mstick_read(uint8_t *pBuf, uint16_t len);
int driver_100ustick_read(uint8_t *pBuf, uint16_t len);
int driver_10ustick_read(uint8_t *pBuf, uint16_t len);
#endif 

