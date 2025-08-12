/**
  ******************************************************************************
  * @file    system_status.h
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

#ifndef __SYSTEM_STATUS_H
#define __SYSTEM_STATUS_H

#include "stdint.h"
#include "stdbool.h"
typedef enum SysStatus_u {
    IDLE                = (0),
    SLEEP               ,
    HEATTING_TEST       ,
    HEATTING_STANDARD   ,
    HEATTING_BOOST      ,
    HEATTING_CLEAN      ,
    HEAT_MODE_TEST_VOLTAGE ,
    HEAT_MODE_TEST_POWER   ,
    HEAT_MODE_TEST_TEMP    ,
    CHARGING               ,
    CHIPPING_MODE_EXITTING     ,
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    BT_SERVICE_CHAR_COMM   ,//prevent go to sleep while ntf communication.
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
} SysStatus_u;

typedef enum SysHeatType_e {
    HEAT_BY_KEY                = (0),
    HEAT_BY_PCTOOL               ,
} SysHeatType_e;


typedef enum
{
    TASK_NONE = 0,
    TASK_BT,
    TASK_CHARGE,
    TASK_HEATTING,
    TASK_UI,
    TASK_USB,
}EN_TASK_TYPE;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
typedef enum SysSubStatus_u 
{
    SUBSTATUS_IDLE = (0),
	SUBSTATUS_PARING_QRCODE,
	SUBSTATUS_PARING,
	SUBSTATUS_FIND_ME,
	SUBSTATUS_LOADING,
} SysSubStatus_u;
SysSubStatus_u get_subSystem_status(void);
void set_subSystem_status(SysSubStatus_u status);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

SysStatus_u get_system_status(void);
void set_system_status(SysStatus_u usysStatus);
bool get_user_reset_status(void);
void set_user_reset_status(bool status);
void update_idl_delay_time(void);
uint32_t get_idl_delay_time(void);
void set_ui_display_status(uint8_t status);
uint8_t get_ui_display_status(void);

void set_task_bt_heartbeat_ticks(void);
void clr_task_bt_heartbeat_ticks(void);
void set_task_charge_heartbeat_ticks(void);
void clr_task_charge_heartbeat_ticks(void);
void set_task_heatting_heartbeat_ticks(void);
void clr_task_heatting_heartbeat_ticks(void);
void set_task_ui_heartbeat_ticks(void);
void clr_task_ui_heartbeat_ticks(void);
void set_task_usb_heartbeat_ticks(void);
void clr_task_usb_heartbeat_ticks(void);
void sys_task_security_ms_delay(uint32_t mstick, EN_TASK_TYPE appTask);
void sys_wdg_pro(void);
void set_heat_type(SysHeatType_e heatType);
SysHeatType_e get_heat_type(void);
void sys_start_shipping_mode_pro(void);

#endif
 
 
