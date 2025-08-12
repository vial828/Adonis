/**
  ******************************************************************************
  * @file    task_charge_service.h
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

#ifndef __TASK_CHARGE_SERVICE_H
#define __TASK_CHARGE_SERVICE_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

// Charge related parameters
#define PROT_USB_OVER_VOLT_THRESHOLD	(11000) // mV, use init table
#define PROT_USB_UNDER_VOLT_THRESHOLD	(4600) // mV

#define PROT_BAT_LOW_VOLT_THRESHOLD		(2500) // mV,??????????????

#define BAT_CHG_LOWER_TEMP_THRESHOLD			(15) // ?
#define BAT_CHG_LOWER_TEMP_CURR_LIMIT			(400) // mA

#if 0
#define BAT_CHG_UPPER_TEMP_THRESHOLD_STEP1		(45) // ?
#define BAT_CHG_UPPER_TEMP_RELEASE_STEP1		(41) // ?
#define BAT_CHG_UPPER_TEMP_THRESHOLD_STEP23		(43) // ?
#define BAT_CHG_UPPER_TEMP_RELEASE_STEP23		(39) // ?
#define BAT_CHG_UPPER_TEMP_CURR_LIMIT_STEP12	(800) // mA
#define BAT_CHG_UPPER_TEMP_CURR_LIMIT_STEP3		(320) // mA

#define BAT_CHG_OVER_TEMP_THRESHOLD_STEP1		(55) // ?
#define BAT_CHG_OVER_TEMP_RELEASE_STEP1			(50) // ?
#define BAT_CHG_OVER_TEMP_THRESHOLD_STEP23		(53) // ?
#define BAT_CHG_OVER_TEMP_RELEASE_STEP23		(48) // ?
#endif

// The current state of the charger
typedef enum {
	CHG_STATE_SETUP		= 0,
	CHG_STATE_CHARGE	= 1,
	CHG_STATE_TEMP		= 2,
	CHG_STATE_FULL		= 3,
	CHG_STATE_DELAY		= 4,
	CHG_STATE_FAULT		= 5,
	CHG_STATE_LOCKOUT	= 6,
	CHG_STATE_USB_HOT	= 7,
	CHG_STATE_TIMEOUT	= 8,
	CHG_STATE_OTHER,
} ChargeState_e;

typedef enum {
	CHG_PARTAB_SUSPENDED_BY_COLD		= 0,
	CHG_PARTAB_SUSPENDED_BY_HOT			= 1,
	CHG_PARTAB_SLOW_BY_COLD				= 2,
	CHG_PARTAB_SLOW_BY_HOT				= 3,
	CHG_PARTAB_NORMAL_TEMP				= 4,
	CHG_PARTAB_SUSPENDED_BY_USB_HOT		= 5,
	CHG_PARTAB_SUSPENDED_BY_PCBA_HOT	= 6,
	CHG_PARTAB_OTHER,
} ChargePartab_e;


typedef struct EolTnfo_t {
	uint16_t session;

	uint8_t index;
	uint8_t indexBackup;

	uint16_t chgVolt;

#if 0
	uint16_t chgTermCurr;
	uint16_t chgCurrLimitLT; // lower temp	
	uint16_t chgCurrNormal;
	uint16_t chgCurrLimitUT; // upper temp
	
	int16_t chgLowerTempThreshold; // Fast charge lower temperature threshold('C) 
	int16_t chgUpperTempThreshold; // Fast charge upper temperature threshold('C) 
	int16_t chgUpperTempRelease; // Fast charge upper temperature release('C)	

	int16_t chgOverTempThreshold; // Over temperature protection threshold ('C)	
	int16_t chgOverTempRelease; // Over temperature protection release ('C)	

	int16_t overTempThreshold; // Over temperature protection threshold ('C)
	//uint16_t overTempRelease; // Over temperature protection release ('C)
#endif
} EolInfo_t;


TaskHandle_t* get_task_charge_handle(void);
void task_charge_service(void *pvParam);
bool procotol_get_charge_sta(uint8_t* rValue);

bool get_charge_full(void);
void set_charge_full(bool status);
bool check_full_remap_soc(void);

int chg_get_timeout_flag(void);

int chg_set_hiz_mode(uint8_t en);
bool is_vbus_hiz_mode(void);
int chg_set_chg_en(uint8_t en);
void chg_debug_clear(void);
void charge_debug_log(void);
uint16_t get_iniTable_session_index(int16_t session);

#endif


