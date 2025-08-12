/**
  ******************************************************************************
  * @file    task_heatting_service.h
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

#ifndef __TASK_HEATTING_SERVICE_H
#define __TASK_HEATTING_SERVICE_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include "data_base_info.h"
#include "heat_safety.h"

#define MAX_POWER  18.0f  // 最大加热功率






typedef struct pid_sturct
{
	float SetPoint;
	float ActualPoint;
	float Err;
	float ErrOlder;
	float ErrOldest;
	float Kp;
	float Ki;
	float Kd;
	float Increment;
} PID_TYPE;


typedef struct {
    double   ut;
    double   kp1,ki1,kp2,ki2,kd;
    double   ek,ek1,ek2;
}str_increase_pid;


void heat_stop(uint16_t stopType);
uint32_t get_sessions_total(void);
uint32_t get_sessions_self_clean_prompt(void);			// 高于20根烟，需要提示清洁

void clr_sessions_total(void);
void heat_show_heat_start_log(void);
void heat_test_voltage_start(float testVoltage,uint32_t heatTime);
void heat_test_power_start(float testPower,uint32_t heatTime);
void heat_test_temp_start(float testTemp,uint32_t heatTime);


void init_pid_param(void);
void fun_pid_ek_clear(float clear_ut);
TaskHandle_t* get_task_heatting_handle(void);
void task_heatting_service(void *pvParam);
void test_heating_proc(void);
HEATER* get_heat_manager_info_handle(void);
bool procotol_get_heat_sta(uint8_t* rValue);
unsigned int get_heating_time(void);
bool procotol_DCDC_opt(uint16_t optVol);
bool procotol_DCDC_adjust(uint16_t adjustVol, uint16_t adjustCur);
bool get_update_session_time_sum_status(void);
void set_update_session_time_sum_status(bool status);

#endif



