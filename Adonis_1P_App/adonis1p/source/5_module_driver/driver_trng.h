/**
  ******************************************************************************
  * @file    driver_trng.h
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

#ifndef __DRIVER_TRNG_H
#define __DRIVER_TRNG_H

#include "stdint.h"

int driver_trng_init(void);
int driver_trng_deinit(void);
int driver_trng_write(uint8_t *pBuf, uint16_t len);
int driver_trng_read(uint8_t *pBuf, uint16_t len);

#endif

