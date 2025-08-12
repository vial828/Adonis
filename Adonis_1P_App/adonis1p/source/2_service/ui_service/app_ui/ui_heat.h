/**
  ******************************************************************************
  * @file    ui_heat.h
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

#ifndef _UI_HEAT_H
#define _UI_HEAT_H

#include "stdint.h"
#include "system_status.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

typedef enum {
	CUSTOMIZE_UI_HI = 0,
	CUSTOMIZE_UI_NAME,
	CUSTOMIZE_UI_BYE,
	CUSTOMIZE_UI_HI_DEFAULT,
	CUSTOMIZE_UI_BYE_DEFAULT,
} CustomizeUI_e;

typedef enum {
	DISP_HEAT = 0,
	DISP_OTA,
} DispMode_e;


void amoled_display_heat_hi(void);
void amoled_display_hi_to_preheat(SysStatus_u sysStatus);
void amoled_display_preheat(SysStatus_u sysStatus);
void amoled_display_preheat_to_consume(SysStatus_u sysStatus);
void amoled_display_heat_consume_all(SysStatus_u sysStatus);
void amoled_display_heat_end(SysStatus_u sysStatus);
void amoled_display_heat_dispose(void);
void amoled_display_dispose_to_bye(void);
void amoled_display_bye(void);
void amoled_display_customize(DispMode_e mode, CustomizeUI_e page, uint32_t time, uint8_t clear);

#endif

