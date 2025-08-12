/**
  ******************************************************************************
  * @file    task_system_service.h
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

#ifndef __TASK_SYSTEM_SERVICE_H
#define __TASK_SYSTEM_SERVICE_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
//#define HIBERNATE_MODE  1
TaskHandle_t* get_task_system_handle(void);
void task_system_service(void *pvParam);
void shell_init(void);

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
typedef struct KeyPressCnt_t
{
	uint8_t cnt1;		// 按键1次数，防止2键交叉计数
	uint8_t cnt2;		// 按键2次数
	uint32_t timeTick;	// 最后一次接下的时间戳，时间大于0.6秒后处理
	bool	bChecking;	// 正在检测
} KeyPressCnt_t;

KeyPressCnt_t *get_key_cnt_handle(void);
void clr_key_cnt(void);

uint8_t motor_strength_get(void);
void motor_strength_set(uint8_t dat);
uint8_t beep_mode_get(void);
void beep_mode_set(uint8_t dat);
bool get_update_lifecycle_status(void);
void set_update_lifecycle_status(bool status);
void beep_set_tune(uint32_t duration);
void motor_set(uint16_t runIndvMs, uint16_t stopIndvMs, uint8_t duty, uint16_t loopTimes);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

void motor_stop(void);
void motor_proc(void);

void motor_set2(uint8_t setHAPTIC);
void motor_proc2(void);

void beep_set(uint16_t runIndvMs,uint16_t stopIndvMs,uint16_t loopTimes);
//void beep_test_set(uint16_t ring_hz,uint16_t runIndvMs,uint16_t stopIndvMs,uint16_t loopTimes);
void beep_stop(void);

void disable_swd(void);
void enable_swd(void);
bool get_usb_status(void);

void sys_set_reset_flag(int val);
int sys_get_reset_flag(void);
void sys_start_key_status(void);
bool get_pctool_idle_cmd_status(void);
uint8_t get_pc_tool_log_en(void);
void update_log_pollCnt(void);
uint32_t get_base_key_press_time(void);
uint32_t get_boost_key_press_time(void);
#endif
