/**
  ******************************************************************************
  * @file    err_code.h
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
#ifndef _ERR_CODE_H
#define _ERR_CODE_H

#include "stdint.h"
#include "platform_io.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "cycfg_gatt_db.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#define ERR_INFO_TBL_LEN    24;
//错误代码（根据场景定义）
typedef enum
{
    ERR_NONE                                     = 0,   // 无错误
    //系统相关错误
    //加热时触发的相关错误
    FLT_DE_BAT_HOT_PRE_SES                       = 101,   // 启动加热时电芯温度过高1 OK one shot error
    FLT_DE_BAT_EMPTY                             = 102,   // 加热过程电压低1        OK, one shot error
    FLT_DE_BAT_LOW                               = 103,   // 启动加热时电量低2      OK one shot error
    FLT_DE_TC_SPIKE                              = 104,   // 温升过快2-------------one shot error
    FLT_DE_THERMOCOUPLE_ERR                      = 105,   // 加热温控异常 (TBD)根据发热体适配 &热偶开路/短路3  OK one shot error
    FLT_DE_TC_ZONE_HOT                           = 106,   // 发热体过温8             OK one shot error
    FLT_DE_TC_ZONE_HOT_PRE_SES                   = 107,   // 启动加热时发热温度高＞200℃(TBD) ok   OK Wait Recover
    FLT_DE_BAT_I_SENSE_DAMAGE                    = 108,   // 加热无电流   OK one shot error
    FLT_DE_HARDWARE_ERR                          = 109,   // 加热硬件异常 OK one shot error
    FLT_DE_BOARD_TC_HIGH                         = 110,   // 电路板高温   OK Wait Recover (用FLT_DE_CO_JUNC_HOT代替)
    FLT_DE_POWER_OVERLOAT                        = 111,   // 功率过载     OK one shot error
    FLT_DE_TARGET_TEMP_DIFF                      = 112,   // 测得温度 大于 目标温度50(ini设定)度one shot error
    
    //充电时触发的相关错误
    FLT_DE_BAT_DAMAGE                            = 201,   // 电芯损坏 <=0.9v Non Recover 实测
    FLT_DE_CIC_OUTPUT_VOLTAGE                    = 202,   // 充电电芯过压Leve0 ＞4.63V 不可恢复 Non recover
    FLT_DE_CIC_CONFIG_ERROR                      = 203,   // 充电芯片iic通信错误 Reset recover
    FLT_DE_BAT_CHARGE_CURRENT_OVER               = 204,   // 充电过流 Reset recover
	FLT_DE_BAT_HOT_CHARGE                      	 = 205,   // 充电-电芯过温 Wait Recover
    FLT_DE_BAT_COLD_CHARGE                       = 206,    // 充电-电芯低温 Wait Recover
    FLT_DE_BAT_VOLTAGE_OVER                      = 207,    // 充电-电芯过压≥4.45V Wait Recover
    FLT_DE_CHG_VBUS                              = 208,    // 充电-Vbus电压高 show wrong charge ui and stop charging
    FLT_DE_CIC_CHARGE_TIMEOUT                    = 209,    // 充电超时＞6H

    // 任何时候触发的错误
    FLT_DE_BAT_DISCHARGE_CURRENT_OVER            = 301,     // 放电过流
    FLT_DE_BAT_HOT                               = 302,     // 电芯过温
    FLT_DE_BAT_COLD                              = 303,     // 电芯低温
	FLT_DE_END_OF_LIFE                           = 304,     // 电芯EOL ≥ 11000
    FLT_DE_CO_JUNC_HOT                           = 305,     // PCB过温≥80℃
    FLT_DE_USB_HOT                               = 306,     // USB端口过热

    TIP_DE_BAT_LEVEL                             = 601,     // 电量提示
    TIP_DE_HEAT                                  = 602,     // 加热提示
    TIP_DE_CLEAN                                 = 603,     // 清洁pro
    TIP_DE_SHIPPING                              = 604,     // 船运
    TIP_DE_REBOOT                                = 605,     // 开机启动
    TIP_DE_REBOOTING                             = 606,     // 重启计数
// dianxin  usbkou guoya  diya guoliu 
}EN_ERROR_CODE;


typedef enum {
    NONE_ERR,
    ONE_SHORT_ERR,
    WAIT_RECOVER,
    RESET_RECOVER,
    NON_RECOVER,
}EN_RECOVER_TYPE;

typedef struct {
    EN_ERROR_CODE errCode;
    uint8_t errStatus;
}ErrInfo_t;

// UI优先级，值越大，优先级越大
typedef enum UiTaskDetailStatus_u {
    UI_NONE                 = (0),
    UI_LEVEL                , // 1
    UI_CHARGE_LEVEL         , // 2

// 蓝牙相关显示优先级应该较低
	UI_BLE_FAILED, // 19
	UI_BLE_PAIRED, // 20
	UI_BLE_PAIRING, // 21
	UI_BLE_FIND_ME, // 22
	UI_LOADING, // 23
	UI_BLE_LOCK, // 24
	UI_BLE_UNLOCK, // 25
	UI_BLE_DOWNLOAD, // 26
	
	UI_SET_PROFILE, // 27 根据需要调整优先级
	UI_SET_PROFILE_BOOST, // 28
	UI_CUSTOMIZE, // 29 根据需要调整优先级

    UI_HEATTING             , // 3
    UI_SHIPPING             , // 4
    UI_CLEAN_PRO            , // 5
    UI_BOOT_TIP             , // 6
    // Exceeding operational voltage
    UI_ERR_BAT_OV           , // 7
    // Battery EOL
    UI_EOL                  , // 8

    // Low Battery
    UI_ERR_BAT_LOW           , // 9
    // Connector too hot
    UI_ERR_USB_HOT         	, // 10
    // Wrong Charger
    UI_ERR_CHARGE_TIMEOUT  	, // 11
    UI_ERR_USB_OV         	, // 12
    // Wait
    UI_ERR_COLD_WAIT         , // 13
    UI_ERR_HOT_WAIT         , // 14
    UI_ERR_GER_WAIT         , // 15
    // Reboot
    UI_ERR_REBOOT_TIP       , // 16 错误重启
    UI_ERR_REBOOTING        , // 17 用户重启
    // Critical Error
    UI_ERR_CRITICAL         , // 18

	UI_TYPE_NUM_ALL
} UiTaskDetailStatus_u;

typedef struct UiInfo_t {
    EN_ERROR_CODE even;
    EN_RECOVER_TYPE actionType; // 严重错误，重启，等待， 充电，低压
    UiTaskDetailStatus_u uiStatus;  // ui显示类型
}UiInfo_t;

//EN_ERROR_CODE get_system_ui_err_status(void);
void set_system_err_war_tip_status(EN_ERROR_CODE esysErrStaus);
UiInfo_t* get_error_high_pro_even(void);
EN_ERROR_CODE find_error_even(EN_ERROR_CODE errCode);
void add_error_even(EN_ERROR_CODE errCode);
void delete_error_even(EN_ERROR_CODE errCode);
UiTaskDetailStatus_u get_pro_even(void);
//EN_ERROR_CODE set_system_ui_err_status(void);
void log_err_code(EN_ERROR_CODE even);
bool get_update_errcode_status(void);
void set_update_errcode_status(bool status);
bool get_delete_all_err_flag(void);
void set_delete_all_err_flag(bool flag);
void delete_all_error_even(void);

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
void add_error_even_log(EN_ERROR_CODE errCode);
void init_errCode_info_handle();
debug_service_last_error_char_t * get_errCode_info_handle(void);
bool get_last_error_status_change();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)


#endif

