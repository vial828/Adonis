/**
  ******************************************************************************
  * @file    ui_test.h
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

#ifndef _UI_BLUETOOTH_H
#define _UI_BLUETOOTH_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

typedef enum {
	PROFILE_UI_NONE = 0,
	PROFILE_UI_STANDARD,
	PROFILE_UI_BOOST
} ProfileUI_e;

void amoled_disp_ble_failed(void);
void amoled_disp_ble_paired(void);
void amoled_disp_ble_download(void);
void amoled_disp_ble_pairing(void);
void amoled_disp_ble_findme(void);
void amoled_disp_ble_lock(void);
void amoled_disp_ble_unlock(void);
void amoled_disp_loading(void);

void amoled_display_set_profile(ProfileUI_e page);
void amoled_display_confirmation(void);

#endif

