/**
  ******************************************************************************
  * @file    ui_shipping_mode.h
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

#ifndef _UI_SHIPPING_MODE_H
#define _UI_SHIPPING_MODE_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

/**
  * @brief  显示hi动画
  * @param  None
* @return None
* @note   图片页数：26， 显示时间：2s（前黑42毫秒，后黑42毫秒）42可忽略读取和发送数据需要时间， 即76ms
*/
void amoled_display_exit_shipping_mode(void);

#endif

