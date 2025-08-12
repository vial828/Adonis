/**
  ******************************************************************************
  * @file    task_ui_service.h
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

#ifndef __TASK_UI_SERVICE_H
#define __TASK_UI_SERVICE_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include "err_code.h"
UiTaskDetailStatus_u get_current_ui_detail_status(void);
void set_current_ui_detail_status(UiTaskDetailStatus_u status);
TaskHandle_t* get_task_ui_handle(void);
void task_ui_service(void *pvParam);
#endif
