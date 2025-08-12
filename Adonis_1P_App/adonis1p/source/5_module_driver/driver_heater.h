/**
  ******************************************************************************
  * @file    driver_heater.h
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

#ifndef __DRIVER_HEATER_H
#define __DRIVER_HEATER_H

#include "stdint.h"
#include "cy_result.h"

//-------------加热相关IO口定义-----------------//
#define CYBSP_DCDCEN_PIN 	    (P8_6)
#define CYBSP_HEAT_NMOS_PIN 	(P0_5)

void heating_io_init(void);
cy_rslt_t hal_tps55288_write_reg(uint8_t reg_addr,uint8_t *dat,uint8_t len);

cy_rslt_t hal_tps55288_read_reg(uint8_t reg_addr,uint8_t *out_dat,uint8_t len);

cy_rslt_t hal_tps55288_set_enable(uint8_t en);

cy_rslt_t hal_tps55288_set_out_v(float set_v);

cy_rslt_t hal_tps55288_init(void);

int driver_heater_init(void);

int driver_heater_deinit(void);

int driver_heater_write(uint8_t *pBuf, uint8_t len);

#endif

