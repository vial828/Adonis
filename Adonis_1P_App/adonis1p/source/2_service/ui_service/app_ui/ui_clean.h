/**
  ******************************************************************************
  * @file    ui_clean.h
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

#ifndef _UI_CLEAN_H
#define _UI_CLEAN_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

#ifdef DEF_DRY_CLEAN_EN // 启用干烧清洁功能,不使用干烧清洁功能时，把此宏注释掉
void amoled_display_cleaning(void);
uint8_t amoled_display_clean_hint(void);
void amoled_display_clean_hint_end(uint8_t op);
#endif

#endif

