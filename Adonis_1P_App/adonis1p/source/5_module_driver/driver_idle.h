/**
  ******************************************************************************
  * @file    driver_idle.h
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

#ifndef __DRIVER_IDLE_H
#define __DRIVER_IDLE_H

#include "stdint.h"

int driver_idle_init(void);
int driver_idle_write(uint8_t *pBuf, uint16_t len);
int driver_idle_read(uint8_t *pBuf, uint16_t len);
#endif

