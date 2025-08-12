/**
  ******************************************************************************
  * @file    err_code.c
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

#include "err_code.h"
#include "task_ui_service.h"
#include "uthash.h"
#include "sm_log.h"
#include "data_base_info.h"
#include "system_status.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "cycfg_gatt_db.h"
#include "session_data_record.h"
#include "event_data_record.h"
#include "app_bt_char_adapter.h"
static debug_service_last_error_char_t * gpErrCode;		// Last Error
bool	bLastErrorChange = false;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

typedef struct {
	int even;
	uint8_t pro;
	UT_hash_handle hh;
}t_HashEven;
t_HashEven *pt_hashEven = NULL;

volatile EN_ERROR_CODE g_esysErrUiStaus = ERR_NONE; // UI当前要显示的错误码
volatile EN_ERROR_CODE g_esysErrStaus = ERR_NONE; // 系统错误码
// ErrInfo_t g_tErrInfoTbl[ERR_INFO_TBL_LEN];

#define MAX_EVEN 33
UiInfo_t gt_uiNewHighProEvenInfo = {0}; // 最新最高优先级事件信息

const UiInfo_t gc_UiStatusTabl[MAX_EVEN] = {
    // Critical Error
	{FLT_DE_BAT_DAMAGE, 	                NON_RECOVER, 	UI_ERR_CRITICAL},
	{FLT_DE_CIC_OUTPUT_VOLTAGE,             NON_RECOVER, 	UI_ERR_CRITICAL},

    // Reboot
	{FLT_DE_CIC_CONFIG_ERROR, 				RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_BAT_DISCHARGE_CURRENT_OVER, 	RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_BAT_CHARGE_CURRENT_OVER, 	    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_TC_SPIKE, 	                    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
    {TIP_DE_REBOOTING,                      NONE_ERR,       UI_ERR_REBOOTING},

    // Wait
    {FLT_DE_HARDWARE_ERR,		            ONE_SHORT_ERR, 	 UI_ERR_GER_WAIT},
    {FLT_DE_THERMOCOUPLE_ERR,               ONE_SHORT_ERR,   UI_ERR_HOT_WAIT}, //Eason.c add 加热温控异常 (TBD)根据发热体适配 &热偶开路/短路3  OK one shot error
    {FLT_DE_BAT_HOT_PRE_SES,                ONE_SHORT_ERR,   UI_ERR_HOT_WAIT}, //ONE_SHORT_ERR 统一管理记录在表，设置显示一次后删除
	{FLT_DE_BAT_HOT, 		                WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_BAT_COLD, 		                WAIT_RECOVER, 	UI_ERR_COLD_WAIT},
	{FLT_DE_CO_JUNC_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_TC_ZONE_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_TC_ZONE_HOT_PRE_SES, 		    WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_POWER_OVERLOAT, 		        ONE_SHORT_ERR, 	UI_ERR_HOT_WAIT},

	{FLT_DE_BAT_COLD_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_COLD_WAIT},
	{FLT_DE_BAT_HOT_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_BAT_I_SENSE_DAMAGE,		        ONE_SHORT_ERR, 	UI_ERR_GER_WAIT},//
	{FLT_DE_TARGET_TEMP_DIFF,		        ONE_SHORT_ERR, 	UI_ERR_GER_WAIT},//

    // Wrong Charger
	{FLT_DE_CIC_CHARGE_TIMEOUT,		        WAIT_RECOVER, 	UI_ERR_CHARGE_TIMEOUT},//
	{FLT_DE_CHG_VBUS,		                WAIT_RECOVER, 	UI_ERR_USB_OV},

    // Connector too hot
	{FLT_DE_USB_HOT,		                WAIT_RECOVER, 	UI_ERR_USB_HOT},

    // Low Battery
    {FLT_DE_BAT_EMPTY,                      ONE_SHORT_ERR,   UI_ERR_BAT_LOW}, // 需更改为电量低提示图，加热中，需要加到表中,显示一次后删除
    {FLT_DE_BAT_LOW,                        ONE_SHORT_ERR,   UI_ERR_BAT_LOW},  // 需更改为电量低提示图 ,需要区分加热完成后剩%6电量的提示， 加热完成后也需要记录在表，显示一次后删除

    // Battery EOL
	{FLT_DE_END_OF_LIFE,		            	NON_RECOVER, 	UI_EOL},

    // Exceeding operational voltage
	{FLT_DE_BAT_VOLTAGE_OVER,		        WAIT_RECOVER, 	UI_ERR_BAT_OV},//UI_ERR_BAT_OV

    // 提示类
	{TIP_DE_REBOOT, 		                NONE_ERR, 	    UI_BOOT_TIP},
	{TIP_DE_CLEAN, 		                    NONE_ERR, 	    UI_CLEAN_PRO},
	{TIP_DE_SHIPPING, 		                NONE_ERR, 	    UI_SHIPPING},
	{TIP_DE_HEAT, 		                    NONE_ERR, 	    UI_HEATTING},
	{TIP_DE_BAT_LEVEL, 		                NONE_ERR, 	    UI_LEVEL},
	{ERR_NONE, 		                    	NONE_ERR, 	    UI_NONE},
};

/**
  * @brief  打印错误码函数
  * @param  None
  * @return 返回事件
  * @note   None
  */
void log_err_code(EN_ERROR_CODE even)
{
    switch (even) {
        case FLT_DE_BAT_DAMAGE:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_DAMAGE\r\n");
            break;
        case FLT_DE_CIC_OUTPUT_VOLTAGE:
            sm_log(SM_LOG_ERR, "FLT_DE_CIC_OUTPUT_VOLTAGE\r\n");
            break;
        case FLT_DE_CIC_CONFIG_ERROR:
            sm_log(SM_LOG_ERR, "FLT_DE_CIC_CONFIG_ERROR\r\n");
            break;
        case FLT_DE_BAT_DISCHARGE_CURRENT_OVER:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_DISCHARGE_CURRENT_OVER\r\n");
            break;
        case FLT_DE_BAT_CHARGE_CURRENT_OVER:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_CHARGE_CURRENT_OVER\r\n");
            break;
        case FLT_DE_BAT_HOT_PRE_SES:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_HOT_PRE_SES\r\n");
            break;
        case FLT_DE_BAT_EMPTY:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_EMPTY\r\n");
            break;
        case FLT_DE_BAT_LOW:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_LOW\r\n");
            break;
        case FLT_DE_BAT_HOT:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_HOT\r\n");
            break;
        case FLT_DE_BAT_COLD:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_COLD\r\n");
            break;
        case FLT_DE_CO_JUNC_HOT:
            sm_log(SM_LOG_ERR, "FLT_DE_CO_JUNC_HOT\r\n");
            break;
        case FLT_DE_TC_ZONE_HOT:
            sm_log(SM_LOG_ERR, "FLT_DE_TC_ZONE_HOT\r\n");
            break;
        case FLT_DE_TC_ZONE_HOT_PRE_SES:
            sm_log(SM_LOG_ERR, "FLT_DE_TC_ZONE_HOT_PRE_SES\r\n");
            break;
        case FLT_DE_BAT_COLD_CHARGE:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_COLD_CHARGE\r\n");
            break;
        case FLT_DE_BAT_HOT_CHARGE:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_HOT_CHARGE\r\n");
            break;
        case FLT_DE_USB_HOT:
            sm_log(SM_LOG_ERR, "FLT_DE_USB_HOT\r\n");
            break;
        case FLT_DE_CHG_VBUS:
            sm_log(SM_LOG_ERR, "FLT_DE_CHG_VBUS\r\n");
            break;
        case FLT_DE_BAT_VOLTAGE_OVER:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_VOLTAGE_OVER\r\n");
            break;
        case FLT_DE_BAT_I_SENSE_DAMAGE:
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_I_SENSE_DAMAGE\r\n");
            break;
        case FLT_DE_CIC_CHARGE_TIMEOUT:
            sm_log(SM_LOG_ERR, "FLT_DE_CIC_CHARGE_TIMEOUT\r\n");
            break;
        case FLT_DE_TARGET_TEMP_DIFF:
            sm_log(SM_LOG_ERR, "FLT_DE_TARGET_TEMP_DIFF\r\n");
            break;
        case FLT_DE_END_OF_LIFE:
            sm_log(SM_LOG_ERR, "FLT_DE_END_OF_LIFE\r\n");
            break;
        case FLT_DE_POWER_OVERLOAT:
            sm_log(SM_LOG_ERR, "FLT_DE_POWER_OVERLOAT\r\n");
            break;
        case FLT_DE_TC_SPIKE:
            sm_log(SM_LOG_ERR, "FLT_DE_TC_SPIKE\r\n");
            break;
        default: break;
    }
}

bool g_updateErrcodeStatus = false;
bool get_update_errcode_status(void)
{
    return g_updateErrcodeStatus;
}
void set_update_errcode_status(bool status)
{
    g_updateErrcodeStatus = status;
}

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
void add_error_even_log(EN_ERROR_CODE errCode)
{
	sm_log(SM_LOG_INFO, "\r\n add_error_even_log %d", errCode);	// debug
// 错误码
	uint8_t event_data[2];
	event_data[1] = errCode;				// 大端传输
	event_data[0] = errCode >> 8;
//	event_record_generate(EVENT_CODE_ERROR, event_data, 2); 	// 避免两者时间戳不一致
// 时间戳
	uint32_t ts;
	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->read((uint8_t *)&ts, 4);
// 写log
	debug_service_event_log_char_t record;
	memset(&(record.event_data), 0, 15);
	record.timestamp = ts;
	record.event_code = EVENT_CODE_ERROR;
	memcpy(&(record.event_data), event_data, 2);
	event_record_insert(&record);

// last err
	gpErrCode->error_code = errCode;
	gpErrCode->timestamp = ts;
	set_update_last_error(true);
	bLastErrorChange = true;
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

extern const uint16_t cErrCodeKeyNum[ERRCODE_MAX_LEN];
void add_error_even(EN_ERROR_CODE errCode)
{
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	sm_log(SM_LOG_INFO, "\r\n add_error_even_log %d", errCode);	// debug
// 错误码
	uint8_t event_data[2];
	event_data[1] = errCode;				// 大端传输
	event_data[0] = errCode >> 8;
//	event_record_generate(EVENT_CODE_ERROR, event_data, 2); 	// 避免两者时间戳不一致
// 时间戳
	uint32_t ts;
	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->read((uint8_t *)&ts, 4);
// 写log
	debug_service_event_log_char_t record;
	memset(&(record.event_data), 0, 15);
	record.timestamp = ts;
	record.event_code = EVENT_CODE_ERROR;
	memcpy(&(record.event_data), event_data, 2);
	event_record_insert(&record);

// last err
	gpErrCode->error_code = errCode;
	gpErrCode->timestamp = ts;
	set_update_last_error(true);
	bLastErrorChange = true;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

	t_HashEven *s;
    int even = (int)errCode;
    uint16_t *pErrcodeTbl = get_errCode_val_info_handle();
  //  sm_log(SM_LOG_INFO, "\r\n add_error_even %d", even);
	HASH_FIND_INT(pt_hashEven, &even, s);
	if (s == NULL) {
		s = (t_HashEven *)malloc(sizeof(t_HashEven));
		s->even = even;
		HASH_ADD_INT(pt_hashEven, even, s);
	}
    for (int i = 0; i < ERRCODE_MAX_LEN; i++) {
        if (errCode == cErrCodeKeyNum[i]) {
            if (pErrcodeTbl[i] < 0xFFFF) {
                pErrcodeTbl[i]++;
                // 在休眠的时候保存一次
                set_update_errcode_status(true);
            }
            break;
        }
    }
}



EN_ERROR_CODE find_error_even(EN_ERROR_CODE errCode)
{
	t_HashEven *s;
    int even = (int)errCode;
	HASH_FIND_INT(pt_hashEven, &even, s);
	if (s != NULL) {
		return s->even;
	}
	return 0;
}

void delete_error_even(EN_ERROR_CODE errCode)
{
	t_HashEven *s;
    int even = (int)errCode;
	HASH_FIND_INT(pt_hashEven, &even, s);
	if (s != NULL) {
		HASH_DEL(pt_hashEven, s);
		free(s);
	}	
}
// 用于上位CLI清除err code后，同步清anytime保护的滤波计数器
static bool gs_deleteAllErrFlg = false;
bool get_delete_all_err_flag(void)
{
    return gs_deleteAllErrFlg;
}

void set_delete_all_err_flag(bool flag)
{
	gs_deleteAllErrFlg = flag;
}
void delete_all_error_even(void)
{
    uint8_t i;
    int even = 0;
    for (i = 0; i < MAX_EVEN; i++) {
         delete_error_even(gc_UiStatusTabl[i].even);
    }
    gt_uiNewHighProEvenInfo.even = ERR_NONE;
    gt_uiNewHighProEvenInfo.actionType = NONE_ERR;
    gt_uiNewHighProEvenInfo.uiStatus = UI_NONE;
    set_delete_all_err_flag(true);
}

// 获取当前最高优先级even
UiInfo_t* get_error_high_pro_even(void)
{
	uint8_t i;
	int even = 0;
	for (i = 0; i < MAX_EVEN; i++) {
		even = find_error_even(gc_UiStatusTabl[i].even);
		if(even != 0) {
          //  sm_log(SM_LOG_INFO, "\r\n get_error_high_pro_even %d", even);
			break;
		}
	}
//    #if 0
	if (i == (MAX_EVEN) && even == 0) { // hash表里已没有事件
	    gt_uiNewHighProEvenInfo.even = ERR_NONE;
        gt_uiNewHighProEvenInfo.actionType = NONE_ERR;
        gt_uiNewHighProEvenInfo.uiStatus = UI_NONE;
        return &gt_uiNewHighProEvenInfo;
	}
//	if (even != gt_uiNewHighProEvenInfo.even) {
//	}
    
    gt_uiNewHighProEvenInfo.even = gc_UiStatusTabl[i].even;
    gt_uiNewHighProEvenInfo.actionType = gc_UiStatusTabl[i].actionType;
    gt_uiNewHighProEvenInfo.uiStatus = gc_UiStatusTabl[i].uiStatus;
	return &gt_uiNewHighProEvenInfo;
}

UiTaskDetailStatus_u get_pro_even(void)
{
	uint8_t i;
	EN_ERROR_CODE even = 0;
	for (i = 0; i < MAX_EVEN; i++) {
		if(g_esysErrStaus == gc_UiStatusTabl[i].even) {
			break;
		}
	}
	return gc_UiStatusTabl[i].uiStatus;
}
#if 0
/**
  * @brief  获取当前系统的错误码
  * @param  None
  * @return 返回错误码
  * @note   None
  */
EN_ERROR_CODE get_system_ui_err_status(void)
{  
    return g_esysErrStaus;
}

EN_ERROR_CODE set_system_ui_err_status(void)
{  
     g_esysErrUiStaus = 0;
//    sm_log(SM_LOG_INFO, "\r\n clr g_esysErrUiStaus;");
}
/**
  * @brief  设置系统的错误码
  * @param  esysErrStaus 要设定的系统错误码
  * @return None
  * @note   None
  */
void set_system_err_war_tip_status(EN_ERROR_CODE esysErrStaus)
{
    g_esysErrStaus = esysErrStaus;
    g_esysErrUiStaus = esysErrStaus;
    sm_log(SM_LOG_INFO, "\r\n set_system_err_war_tip_status g_esysErrUiStaus %d;", g_esysErrUiStaus);
}
#endif

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
void init_errCode_info_handle()					// 指向正确的RAM位置
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	gpErrCode = (debug_service_last_error_char_t *)(&(pBleData->last_error[4]));			// 前4字节保存Magic字段
}

debug_service_last_error_char_t * get_errCode_info_handle(void)
{
	return gpErrCode;
}

bool get_last_error_status_change()
{
	if (bLastErrorChange)
	{
		bLastErrorChange = false;
		return true;
	}
	else
	{
		return false;
	}
}

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)


