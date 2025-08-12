/**
  ******************************************************************************
  * @file    system_interaction_logic.c
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

#include "system_interaction_logic.h"
#include "public_typedef.h"
#include "platform_io.h"
#include "system_status.h"
#include "err_code.h"
#include "task_ui_service.h" // 包含关系需要优化
#include "data_base_info.h"
#include "heat_safety.h"
#include "task_charge_service.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "task_system_service.h"
#include "ota.h"

static bool bDispUICharg = false;		// 充电后需要显示电量，但由于某些高优先级蓝牙正在显示，在显示完成后，需要显示充电电量
static bool bDispUILock = false;		// Bug 2012331 需要显示完电量后才显示Lock
static uint32_t timetick;

static bool bDlyDisplayEOL = false;
//static uint16_t g_eol_delay_handle_bits;
//#define	B_EOL_LOCK				0
//#define	B_EOL_UNLOCK			1
//#define	B_EOL_PARING_QRCODE		2
//#define	B_EOL_PARING			3
////#define	B_EOL_FIND_MY_GLO		4
//#define	B_EOL_PARING_CANCLE		5
//#define	B_EOL_PARI_OK			6
//#define	B_EOL_PARI_FAIL			7
//#define	B_EOL_LOADING			8
//#define	B_EOL_LOADING_STOP		9
//#define	B_EOL_LOADING_OK		10
//#define	B_EOL_UIMAGE_CUSTOMIZE	11
//#define	B_EOL_HEATING_PROFILES	12
//#define	B_EOL_HEATING_PROFILES_BOOST	13

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

volatile ExternalEven_e g_externalEven = 0; 

UiTaskDetailStatus_u get_current_ui_detail_status(void);
void set_current_ui_detail_status(UiTaskDetailStatus_u status);
void system_error_recover_on_chargeing_proc(void);


/**
  * @brief  获取系统外部事件
  * @param  None
  * @return 返回事件
  * @note   None
  */
ExternalEven_e get_system_external_even(void)
{
    return g_externalEven;
}

/**
  * @brief  设置系统外部事件
  * @param  None
  * @return 返回事件
  * @note   None
  */
void set_system_external_even(ExternalEven_e even)
{
    g_externalEven = even;
}

/**
  * @brief  设置显示任务
  * @param  None
  * @return 返回事件
  * @note   None
  */
void set_up_ui_display(UiInfo_t *pUiInfo)
{
    UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
    if (uiCurrent == pUiInfo->uiStatus ) {
        return;
    }
    if (pUiInfo->even == ERR_NONE) { // 无错误

    } else {

    }
}
#define MAX_PRE_HEAT_TBL_LEN 18
// 加热前需要判断的错误
const UiInfo_t gc_PreHeatTabl[MAX_PRE_HEAT_TBL_LEN] = {
    // Critical Error
	{FLT_DE_BAT_DAMAGE, 	                NON_RECOVER, 	UI_ERR_CRITICAL}, \
	{FLT_DE_CIC_OUTPUT_VOLTAGE,             NON_RECOVER, 	UI_ERR_CRITICAL}, \
	// Reboot
	{FLT_DE_CIC_CONFIG_ERROR, 				RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, \
	{FLT_DE_BAT_DISCHARGE_CURRENT_OVER, 	RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, \
	{FLT_DE_BAT_CHARGE_CURRENT_OVER, 	    RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, \
	{FLT_DE_TC_SPIKE, 	                    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
    // Wait
	{FLT_DE_BAT_HOT, 		                WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \
	{FLT_DE_BAT_COLD, 		                WAIT_RECOVER, 	UI_ERR_COLD_WAIT}, \
	{FLT_DE_CO_JUNC_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \
	{FLT_DE_TC_ZONE_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \

	//{FLT_DE_BAT_COLD_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_COLD_WAIT},
	{FLT_DE_BAT_HOT_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
    {FLT_DE_BAT_HOT_PRE_SES,                ONE_SHORT_ERR,  UI_ERR_HOT_WAIT},
	{FLT_DE_TC_ZONE_HOT_PRE_SES, 		    ONE_SHORT_ERR, 	UI_ERR_HOT_WAIT},
	{FLT_DE_BAT_I_SENSE_DAMAGE,		        ONE_SHORT_ERR, 	UI_ERR_GER_WAIT},//
    {FLT_DE_THERMOCOUPLE_ERR,               ONE_SHORT_ERR,   UI_ERR_HOT_WAIT},
    //  Connector too hot
    {FLT_DE_USB_HOT,                        WAIT_RECOVER,   UI_ERR_USB_HOT},
    // Low Battery
	{FLT_DE_BAT_LOW, 		                ONE_SHORT_ERR, 	UI_ERR_BAT_LOW},
    // Exceeding operational voltage
	{FLT_DE_BAT_VOLTAGE_OVER,		        WAIT_RECOVER, 	UI_ERR_BAT_OV},//UI_ERR_BAT_OV

//	{TIP_DE_BAT_LEVEL, 		                NONE_ERR, 	    UI_LEVEL}, \
//	{TIP_DE_HEAT, 		                    NONE_ERR, 	    UI_HEATTING}, \
//	{TIP_DE_CLEAN, 		                    NONE_ERR, 	    UI_CLEAN_PRO}, \
//	{TIP_DE_SHIPPING, 		                NONE_ERR, 	    UI_SHIPPING}, \
//	{TIP_DE_REBOOT, 		                NONE_ERR, 	    UI_BOOT_TIP},\
//	{TIP_DE_REBOOTING, 		            	NONE_ERR, 	    UI_ERR_REBOOTING},
//	{ERR_NONE, 		                    	NONE_ERR, 	    UI_NONE},
};

/**
  * @brief  
  * @param  None
  * @return bool: true:可以加热，false:不可以加热
  * @note   None
  */
bool pre_heat_check_error(UiTaskDetailStatus_u uiCurrent)
{
    int i = 0;
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    int16_t* ini_p = get_ini_val_info_handle();

    uint8_t start_check = safety_check_before_heating();//加热前检查系统信息

    // 优先查找错误
    for (i = 0; i< MAX_PRE_HEAT_TBL_LEN; i++) {
        if (gc_PreHeatTabl[i].even == find_error_even(gc_PreHeatTabl[i].even)) {
            break;
        }
    }
    if (i < MAX_PRE_HEAT_TBL_LEN) {
        if (uiCurrent < gc_PreHeatTabl[i].uiStatus) {
            log_err_code(gc_PreHeatTabl[i].even);
			
			add_error_even_log(gc_PreHeatTabl[i].even);	// Bug 1928243 
			
            set_current_ui_detail_status(gc_PreHeatTabl[i].uiStatus);
            if (gc_PreHeatTabl[i].even != FLT_DE_BAT_LOW) { // 启动加热时，电量低不振动
                motor_set2(HAPTIC_ERR); // 加热错误，需要振动
            }
        }
        
        if (uiCurrent > UI_NONE && uiCurrent <= UI_ERR_BAT_LOW && gc_PreHeatTabl[i].even == FLT_DE_BAT_LOW) { // 正在显示低级别UI，重新显示
             set_current_ui_detail_status(UI_NONE);
             vTaskDelay(50);
             set_current_ui_detail_status(UI_ERR_BAT_LOW);
         } else {
         }
        sm_log(SM_LOG_INFO, "pre_heat_check_error! uiCurrent:%d, gc_PreHeatTabl[i].uiStatus:%d\r\n", uiCurrent, gc_PreHeatTabl[i].uiStatus);
        return false;
    }
    #if 0
    // 在没有其他错误的前提下，判断有没有one short err, one short err  暂时记录在表， 在自恢复流程把错误清除
    // 加热前判断 电芯温度是否过高1，启动加热时电量低2,启动加热时发热体过温2 
    if(p_tMontorDataInfo->bat.temperature > ini_p[FLT_BAT_HOT_PRE_SES]) { // 启动加热时电芯温度过温判断
        set_current_ui_detail_status(UI_ERR_HOT_WAIT);
        add_error_even(FLT_DE_BAT_HOT_PRE_SES);
        return false;
    } else if (p_tMontorDataInfo->bat.voltage < (float)ini_p[WAR_BAT_LOW]) { // 启动加热时电压低，POC先按电压，后改为电量%6， 
        set_current_ui_detail_status(UI_LEVEL); // 需改为低电量UI
        add_error_even(FLT_DE_BAT_LOW);
        return false;
    } else if (p_tMontorDataInfo->det.heat_K_temp > (float)ini_p[FLT_TC_ZONE_HOT_PRE_SES] && ini_p[DB_FLT_TC_CONT] == 1) { // 200 , 启动加热时发热体过温(TBD)
        set_current_ui_detail_status(UI_ERR_HOT_WAIT);
        add_error_even(FLT_DE_TC_ZONE_HOT_PRE_SES);
        return false;
    }
    #endif
    if (p_tMontorDataInfo->bat.remap_soc < ini_p[WAR_BAT_LOW_SOC]) { // 显示电量，不加热
        if (uiCurrent <= UI_ERR_BAT_LOW) {
            set_current_ui_detail_status(UI_NONE);
            vTaskDelay(20);
            set_current_ui_detail_status(UI_ERR_BAT_LOW);
        }
        return false;
    }
    if (uiCurrent >= UI_ERR_BAT_LOW && uiCurrent != UI_ERR_CHARGE_TIMEOUT) { // 正在显示错误UI，不进行加热,充电超时UI例外
        sm_log(SM_LOG_INFO, "pre_heat_check_error! uiCurrent:%d\r\n",  uiCurrent);
        return false;
    }
    return true;
}

#define MAX_HEAT_TBL_LEN 22
// 加热中需要判断的错误
const UiInfo_t gc_HeatTabl[MAX_HEAT_TBL_LEN] = {
    // Critical Error
	{FLT_DE_BAT_DAMAGE, 	                NON_RECOVER, 	UI_ERR_CRITICAL}, \
	{FLT_DE_CIC_OUTPUT_VOLTAGE,             NON_RECOVER, 	UI_ERR_CRITICAL}, \
	// Reboot
	{FLT_DE_CIC_CONFIG_ERROR, 				RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, \
	{FLT_DE_BAT_DISCHARGE_CURRENT_OVER, 	RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, \
	{FLT_DE_BAT_CHARGE_CURRENT_OVER, 	    RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, \
	{FLT_DE_TC_SPIKE, 	                    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
    // Wait
    {FLT_DE_HARDWARE_ERR,		            ONE_SHORT_ERR, 	UI_ERR_GER_WAIT},\
    {FLT_DE_THERMOCOUPLE_ERR,       ONE_SHORT_ERR,  UI_ERR_GER_WAIT},\
    {FLT_DE_BAT_HOT_PRE_SES,                WAIT_RECOVER,   UI_ERR_HOT_WAIT}, \
	{FLT_DE_BAT_HOT, 		                WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \
	{FLT_DE_BAT_COLD, 		                WAIT_RECOVER, 	UI_ERR_COLD_WAIT}, \
	{FLT_DE_CO_JUNC_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \
	{FLT_DE_TC_ZONE_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \
	{FLT_DE_TC_ZONE_HOT_PRE_SES, 		    WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, \
	{FLT_DE_POWER_OVERLOAT, 		        ONE_SHORT_ERR, 	UI_ERR_HOT_WAIT},
	{FLT_DE_TARGET_TEMP_DIFF,		        ONE_SHORT_ERR, 	UI_ERR_GER_WAIT},//

//	{FLT_DE_BAT_COLD_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_COLD_WAIT},
	{FLT_DE_BAT_HOT_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
//	{FLT_DE_CHG_VBUS,		                NON_RECOVER, 	UI_ERR_USB_OV},可以加热
	{FLT_DE_BAT_I_SENSE_DAMAGE,		        ONE_SHORT_ERR, 	UI_ERR_GER_WAIT},//
    {FLT_DE_THERMOCOUPLE_ERR,               ONE_SHORT_ERR,  UI_ERR_HOT_WAIT},
	// Connector too hot
	{FLT_DE_USB_HOT,		                WAIT_RECOVER, 	UI_ERR_USB_HOT},
	// Low Battery
    {FLT_DE_BAT_EMPTY,                      ONE_SHORT_ERR,  UI_ERR_BAT_LOW},
    // Exceeding operational voltage
	{FLT_DE_BAT_VOLTAGE_OVER,		        WAIT_RECOVER, 	UI_ERR_BAT_OV},//UI_ERR_BAT_OV


//	{TIP_DE_BAT_LEVEL, 		                NONE_ERR, 	    UI_LEVEL}, \
//	{TIP_DE_HEAT, 		                    NONE_ERR, 	    UI_HEATTING}, \
//	{TIP_DE_CLEAN, 		                    NONE_ERR, 	    UI_CLEAN_PRO}, \
//	{TIP_DE_SHIPPING, 		                NONE_ERR, 	    UI_SHIPPING}, \
//	{TIP_DE_REBOOT, 		                NONE_ERR, 	    UI_BOOT_TIP},\
//	{TIP_DE_REBOOTING, 		            	NONE_ERR, 	    UI_ERR_REBOOTING},
//	{ERR_NONE, 		                    	NONE_ERR, 	    UI_NONE},
};

/**
  * @brief  
  * @param  None
  * @return bool: true:可以加热，false:不可以加热
  * @note   None
  */
bool heat_check_error(UiTaskDetailStatus_u uiCurrent)
{
    int i = 0;
    for (i = 0; i< MAX_HEAT_TBL_LEN; i++) {
        if (gc_HeatTabl[i].even == find_error_even(gc_HeatTabl[i].even)) {
            break;
        }
    }
    if (i < MAX_HEAT_TBL_LEN) {
        if (uiCurrent < gc_HeatTabl[i].uiStatus) {
            set_current_ui_detail_status(gc_HeatTabl[i].uiStatus);
            log_err_code(gc_HeatTabl[i].even);
		
//			add_error_even_log(gc_PreHeatTabl[i].even);	// Bug 1928243		加热部分处理
			
            if (FLT_DE_BAT_EMPTY != gc_HeatTabl[i].even) { // 低电不振动
                motor_set2(HAPTIC_ERR); // 加热错误，需要振动
            }
        }
        sm_log(SM_LOG_INFO, "heat_check_error! uiCurrent:%d, gc_HeatTabl[i].uiStatus:%d\r\n", uiCurrent, gc_HeatTabl[i].uiStatus);
		heat_stop(gc_HeatTabl[i].even);		// 发生错误，把错误码发送到Stop，Stop判断是否已经Star
		return false;
    }
    if (get_task_heatting_heartbeat_ticks() == 0 && (HEATTING_STANDARD == get_system_status() || HEATTING_BOOST == get_system_status() || HEATTING_CLEAN == get_system_status())) { // 正常加热结束判断
        sm_log(SM_LOG_INFO, "heating complete!\r\n");
        set_system_status(IDLE);
    }
    return true;
}


#define MAX_PRE_CHARGE_TBL_LEN 16
// 充电前需要判断的错误
const UiInfo_t gc_PreChargeTabl[MAX_PRE_CHARGE_TBL_LEN] = {
    // Critical Error
	{FLT_DE_BAT_DAMAGE, 	                NON_RECOVER, 	UI_ERR_CRITICAL}, // OK
	{FLT_DE_CIC_OUTPUT_VOLTAGE,             NON_RECOVER, 	UI_ERR_CRITICAL},
	// Reboot
	{FLT_DE_CIC_CONFIG_ERROR, 				RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_BAT_DISCHARGE_CURRENT_OVER, 	RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_BAT_CHARGE_CURRENT_OVER, 	    RESET_RECOVER, 	UI_ERR_REBOOT_TIP}, // ok
	{FLT_DE_TC_SPIKE, 	                    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	// Wait
	{FLT_DE_BAT_HOT, 		                WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, //OK
	{FLT_DE_BAT_COLD, 		                WAIT_RECOVER, 	UI_ERR_COLD_WAIT}, // OK
	{FLT_DE_CO_JUNC_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT},// OK
	{FLT_DE_TC_ZONE_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_BAT_COLD_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_COLD_WAIT}, // OK
	{FLT_DE_BAT_HOT_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_HOT_WAIT}, // OK
	// Wrong Charger
	{FLT_DE_CHG_VBUS,		                WAIT_RECOVER, 	UI_ERR_USB_OV}, // OK
	{FLT_DE_CIC_CHARGE_TIMEOUT,		        WAIT_RECOVER, 	UI_ERR_CHARGE_TIMEOUT},// OK
    // Connector too hot
	{FLT_DE_USB_HOT,		                WAIT_RECOVER, 	UI_ERR_USB_HOT}, //ok
    // Exceeding operational voltage
	{FLT_DE_BAT_VOLTAGE_OVER,		        WAIT_RECOVER, 	UI_ERR_BAT_OV}, // OK
};

/**
  * @brief  
  * @param  None
  * @return bool: true:可以充电，false:不可以充电
  * @note   None
  */
bool pre_charge_check_error(UiTaskDetailStatus_u uiCurrent)
{
    int i = 0;
    for (i = 0; i< MAX_PRE_CHARGE_TBL_LEN; i++) {
        if (gc_PreChargeTabl[i].even == find_error_even(gc_PreChargeTabl[i].even)) {
            sm_log(SM_LOG_INFO, "pre_charge_check_error! gc_PreChargeTabl[i].even:%d, i:%d\r\n", gc_PreChargeTabl[i].even, i);
            break;
        }
    }
    if (i < MAX_PRE_CHARGE_TBL_LEN) {
        if (uiCurrent < gc_PreChargeTabl[i].uiStatus) {
            set_current_ui_detail_status(gc_PreChargeTabl[i].uiStatus);
            log_err_code(gc_PreChargeTabl[i].even);
		
			add_error_even_log(gc_PreChargeTabl[i].even);	// bug 1916088
			
            if (gc_PreChargeTabl[i].even != FLT_DE_CIC_CHARGE_TIMEOUT) {
                motor_set2(HAPTIC_ERR);
            }
        }
        sm_log(SM_LOG_INFO, "pre_charge_check_error! uiCurrent:%d, gc_PreChargeTabl[i].uiStatus:%d\r\n", uiCurrent, gc_PreChargeTabl[i].uiStatus);
        return false;
    }
    return true;
}

volatile bool g_chargFullStatus = false; // true:充满，false:未满
/**
  * @brief  获取充电满状态
  * @param  None
  * @return None
  * @note   None
  */
bool get_charge_full_status(void)
{
    return g_chargFullStatus;
}

/**
  * @brief  设置充电满状态
  * @param  None
  * @return None
  * @note   None
  */
void set_charge_full_status(bool status)
{
    g_chargFullStatus = status;
}

#define MAX_CHARGING_TBL_LEN 16
// 充电前需要判断的错误
const UiInfo_t gc_ChargingTabl[MAX_CHARGING_TBL_LEN] = {
    // Critical Error
	{FLT_DE_BAT_DAMAGE, 	                NON_RECOVER, 	UI_ERR_CRITICAL},
	{FLT_DE_CIC_OUTPUT_VOLTAGE,             NON_RECOVER, 	UI_ERR_CRITICAL},
	// Reboot
	{FLT_DE_CIC_CONFIG_ERROR, 				RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_BAT_DISCHARGE_CURRENT_OVER, 	RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_BAT_CHARGE_CURRENT_OVER, 	    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	{FLT_DE_TC_SPIKE, 	                    RESET_RECOVER, 	UI_ERR_REBOOT_TIP},
	// Wait
	{FLT_DE_BAT_HOT, 		                WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_BAT_COLD, 		                WAIT_RECOVER, 	UI_ERR_COLD_WAIT},
	{FLT_DE_CO_JUNC_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_TC_ZONE_HOT, 		            WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	{FLT_DE_BAT_COLD_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_COLD_WAIT},
	{FLT_DE_BAT_HOT_CHARGE, 			    WAIT_RECOVER, 	UI_ERR_HOT_WAIT},
	// Wrong Charger
	{FLT_DE_CHG_VBUS,		                WAIT_RECOVER, 	UI_ERR_USB_OV},
	{FLT_DE_CIC_CHARGE_TIMEOUT,		        WAIT_RECOVER, 	UI_ERR_CHARGE_TIMEOUT},//
    // Connector too hot
	{FLT_DE_USB_HOT,		                WAIT_RECOVER, 	UI_ERR_USB_HOT},
    // Exceeding operational voltage
	{FLT_DE_BAT_VOLTAGE_OVER,		        WAIT_RECOVER, 	UI_ERR_BAT_OV},
};

/**
  * @brief  
  * @param  None
  * @return bool: true:可以充电，false:不可以充电
  * @note   None
  */
bool charging_check_error(UiTaskDetailStatus_u uiCurrent)
{
    int i = 0;
    for (i = 0; i< MAX_CHARGING_TBL_LEN; i++) {
        if (gc_ChargingTabl[i].even == find_error_even(gc_ChargingTabl[i].even)) {
            break;
        }
    }
    if (i < MAX_CHARGING_TBL_LEN) {
        if (uiCurrent < gc_ChargingTabl[i].uiStatus) {
            set_current_ui_detail_status(gc_ChargingTabl[i].uiStatus);
            log_err_code(gc_ChargingTabl[i].even);
		
//			add_error_even_log(gc_PreChargeTabl[i].even);	// bug 1916088		// 检测事件会处理
			
            if (gc_ChargingTabl[i].even != FLT_DE_CIC_CHARGE_TIMEOUT) {
                motor_set2(HAPTIC_ERR);
            }
            // 如果是one short err 需要删除该错误，{
//                delete_error_even(gc_ChargingTabl[i].even);
//            }
//            if (gc_ChargingTabl[i].actionType == ONE_SHORT_ERR) 
        }
        sm_log(SM_LOG_INFO, "charging_check_error! uiCurrent:%d, gc_ChargingTabl[i].uiStatus:%d\r\n", uiCurrent, gc_ChargingTabl[i].uiStatus);
        return false;
    }
    if (true == get_charge_full_status()) {
        sm_log(SM_LOG_INFO, "charging_check_error! full\r\n");
        if (uiCurrent == UI_CHARGE_LEVEL || uiCurrent < UI_LEVEL) {
            set_current_ui_detail_status(UI_LEVEL);
        }
        return false;
    }
    return true;
}

/**
  * @brief  
  * @param  None
  * @return bool: true:可以充电，false:不可以充电
  * @note   None
  */
bool charging_only_check_error(void)
{
    int i = 0;
    for (i = 0; i< MAX_CHARGING_TBL_LEN; i++) {
        if (gc_ChargingTabl[i].even == find_error_even(gc_ChargingTabl[i].even)) {
            break;
        }
    }
    if (i < MAX_CHARGING_TBL_LEN) {
        return false;
    }
    return true;
}


bool get_battery_eol_status(void)
{
	int16_t* ini_p = get_ini_val_info_handle();
#if 0
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    uint32_t sessionCnt = p_tHeatParamInfo->smokeTotalTime / 300;
#else
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
#endif

    if (p_tMontorDataInfo->session >= ini_p[STEP3_SESSION]) {
        return true;
    }
    if (FLT_DE_END_OF_LIFE == find_error_even(FLT_DE_END_OF_LIFE)) {
        return true;
    }
    return false;
}

bool bad_battery_check(void)
{
    static uint16_t checkCnt[4] = {0};
	static uint16_t checkStatus = 0;
    int16_t *p_iniVal = get_ini_val_info_handle();
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();

	if ((p_tMontorDataInfo->chg.reg20_state & PG_STAT_MASK) &&  // Vbus input
		(p_tMontorDataInfo->chg.reg16_ctrl1 & CHG_CFG_MASK)) {  // charge enable
		// Vbat波形持续低于2.0V超过2s以上判断为死电池
		if (p_tMontorDataInfo->chg.bat_volt <= (uint16_t)p_iniVal[WAR_BAT_VOLT_DAMAGE]) { // 0.9~2.0V
			if (checkCnt[0] <= 2000) { // 该错误判断消抖不能太快
				checkCnt[0]++;
			}
			if (checkCnt[0] == 2000) { // 不重复触发
				sm_log(SM_LOG_DEBUG, "Vcbat:%d\r\n", p_tMontorDataInfo->chg.bat_volt);
				return true;
			}
		} else {
			checkCnt[0] = 0;
		}
		// 电池电压 < 0.9V, 锂保完全断路，Vbat波形震荡连续超过4次判断为死电池
		switch (checkStatus) {
			case 0:
				if (p_tMontorDataInfo->chg.bat_volt <= (uint16_t)p_iniVal[WAR_BAT_VOLT_DAMAGE]) { // 2.0V
					checkCnt[1]++;
					if (checkCnt[1] >= 25) {
						checkCnt[1] = 0;
						checkStatus++;
					}
				} else {
					checkCnt[1] = 0;
				}
				break;

			case 1:
				if (p_tMontorDataInfo->chg.bat_volt > (PROT_BAT_LOW_VOLT_THRESHOLD + 500)) { // 3.0V
					checkCnt[1]++;
					if (checkCnt[1] >= 25) {
						checkCnt[1] = 0;
						checkStatus++;
					}
				} else {
					checkCnt[1] = 0;
				}
				break;

			case 2:
				if (checkCnt[2] <= 4) {
					checkCnt[2]++;
				}
				sm_log(SM_LOG_DEBUG, "V_checkCnt:%d\r\n", checkCnt[2]);
				if (checkCnt[2] == 4) { // 不重复触发,约4*450ms
					return true;
				}
				checkStatus = 0;
				break;

			default:
				checkStatus = 0;
				break;
		}
		// 如果Vbat达到正常工作电压，并持续2s以上，清死电池判断状态位，防止误报
		if (p_tMontorDataInfo->chg.bat_volt > PROT_BAT_LOW_VOLT_THRESHOLD) { // fuel gauge work voltage > 2.5V
			checkCnt[3]++;
			if (checkCnt[3] >= 1000) {
				checkCnt[3] = 0;
				checkCnt[2] = 0;
				checkStatus = 0;
			}
		} else {
			checkCnt[3] = 0;
		}
    } else {
    	if (checkCnt[0] != 0) {
			for (uint8_t i=0; i<4; i++) {
				checkCnt[i] = 0;
			}
			checkStatus = 0;
    	}
    }

	return false;
}


void system_anytime_error_code_check(void)
{
    static uint8_t anytimeFilterHeaterOverTempCnt = 0;
    static uint8_t anytimeFilterDischargeOverCurrentCnt = 0;
    static uint8_t anytimeFilterBatHotCnt = 0;
    static uint8_t anytimeFilterBatCodeCnt = 0;
    static uint8_t anytimeFilterPcbOverTmpCnt = 0;
    static uint8_t anytimeFilterUsbOverTmpCnt = 0;
    static uint8_t anytimeFilterVbusOverVolCnt = 0;
    static uint8_t anytimeFilterBatEolCnt = 0;

    int16_t *p_iniVal = get_ini_val_info_handle();
    // 任何时候都检测的变量 anytime
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    SysStatus_u sysStatus = get_system_status();

    /* 上位CLI清除err code后，同步清anytime保护的滤波计数器 */
    if (true == get_delete_all_err_flag()) {
    	anytimeFilterHeaterOverTempCnt = 0;
    	anytimeFilterDischargeOverCurrentCnt = 0;
    	anytimeFilterBatHotCnt = 0;
    	anytimeFilterBatCodeCnt = 0;
    	anytimeFilterPcbOverTmpCnt = 0;
    	anytimeFilterUsbOverTmpCnt = 0;
    	anytimeFilterVbusOverVolCnt = 0;
    	anytimeFilterBatEolCnt = 0;
    	set_delete_all_err_flag(false);
    }

    // discharge over current 放电电流是负值
    if (p_tMontorDataInfo->bat.current <= p_iniVal[FLT_BAT_DISCHARGE_CURR_OVER]) {
        if (anytimeFilterDischargeOverCurrentCnt < 5) {
            anytimeFilterDischargeOverCurrentCnt++;
            if (anytimeFilterDischargeOverCurrentCnt == 5) {
                add_error_even(FLT_DE_BAT_DISCHARGE_CURRENT_OVER);
                log_err_code(FLT_DE_BAT_DISCHARGE_CURRENT_OVER);
            }
        }
    } else {
       anytimeFilterDischargeOverCurrentCnt = 0;
    }

    // bad battery detected
	if (true == bad_battery_check()) {
		if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
			p_tMontorDataInfo->state = CHG_STATE_FAULT;
			p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
		}
		add_error_even(FLT_DE_BAT_DAMAGE);
		log_err_code(FLT_DE_BAT_DAMAGE);
	}

    // 发热体过温大于500
    if (p_tMontorDataInfo->det.heat_K_temp >= (float)p_iniVal[FLT_TC_ZONE_HOT]) {
        if (anytimeFilterHeaterOverTempCnt < 5) {
            anytimeFilterHeaterOverTempCnt++;
            if (anytimeFilterHeaterOverTempCnt == 5) {
            	if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
            		p_tMontorDataInfo->state = CHG_STATE_FAULT;
            		p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_HOT;
            	}
                add_error_even(FLT_DE_TC_ZONE_HOT);
                log_err_code(FLT_DE_TC_ZONE_HOT);
            }
        }
    } else {
       anytimeFilterHeaterOverTempCnt = 0;
    }
    // battery hot
	
#if 1 // 更新session对应的配置和比较参数
    if (p_tMontorDataInfo->bat.temperature >= p_iniVal[STEP1_FLT_BAT_HOT + get_iniTable_session_index(p_tMontorDataInfo->session)]) {
        if (anytimeFilterBatHotCnt < 1) {
            anytimeFilterBatHotCnt++;
            if (anytimeFilterBatHotCnt == 1) {
            	if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
            		p_tMontorDataInfo->state = CHG_STATE_TEMP;
            		p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_HOT;
            	}
                add_error_even(FLT_DE_BAT_HOT);
                log_err_code(FLT_DE_BAT_HOT);
            }
        }
    } else {
       anytimeFilterBatHotCnt = 0;
    }
#else
    if (p_tMontorDataInfo->bat.temperature >= p_iniVal[FLT_BAT_HOT]) {
        if (anytimeFilterBatHotCnt < 5) {
            anytimeFilterBatHotCnt++;
            if (anytimeFilterBatHotCnt == 5) {
				p_tMontorDataInfo->chg.state = CHG_STATE_TEMP;
				p_tMontorDataInfo->chg.partab = CHG_PARTAB_SUSPENDED_BY_HOT;
                add_error_even(FLT_DE_BAT_HOT);
                log_err_code(FLT_DE_BAT_HOT);
            }
        }
    } else {
       anytimeFilterBatHotCnt = 0;
    }
#endif
    // 电芯低温
    if (p_tMontorDataInfo->bat.temperature <= p_iniVal[FLT_BAT_COLD]) {
        if (anytimeFilterBatCodeCnt < 1) {
            anytimeFilterBatCodeCnt++;
            if (anytimeFilterBatCodeCnt == 1) {
            	if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
            		p_tMontorDataInfo->state = CHG_STATE_TEMP;
            		p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_COLD;
            	}
                add_error_even(FLT_DE_BAT_COLD);
                log_err_code(FLT_DE_BAT_COLD);
            }
        }
    } else {
       anytimeFilterBatCodeCnt = 0;
    }
    // PCB过温 PCB over temperature
    if (p_tMontorDataInfo->det.heat_K_cood_temp >= (float)p_iniVal[FLT_CO_JUNC_HOT]) {
        if (anytimeFilterPcbOverTmpCnt < 5) {
            anytimeFilterPcbOverTmpCnt++;
            if (anytimeFilterPcbOverTmpCnt == 5) {
            	if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
            		p_tMontorDataInfo->state = CHG_STATE_FAULT;
            		p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_PCBA_HOT;
            	}
                add_error_even(FLT_DE_CO_JUNC_HOT);
                log_err_code(FLT_DE_CO_JUNC_HOT);
            }
        }
    } else {
       anytimeFilterPcbOverTmpCnt = 0;
    }
    // USB口过温USB port overheating 
    if (true == get_usb_status() && p_tMontorDataInfo->det.usb_port_temp >= (float)p_iniVal[FLT_USB_HOT_TEMP]) {
        if (anytimeFilterUsbOverTmpCnt < 1) { // 来不及滤波，滤波会先进入充电状态
            anytimeFilterUsbOverTmpCnt++;
            if (anytimeFilterUsbOverTmpCnt == 1) {
            	if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
            		p_tMontorDataInfo->state = CHG_STATE_FAULT;
            		p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_USB_HOT;
            	}
                add_error_even(FLT_DE_USB_HOT);
                log_err_code(FLT_DE_USB_HOT);
            }
        }
    } else {
       anytimeFilterUsbOverTmpCnt = 0;
    }
    // VBUS过压
    if (true == get_usb_status() && p_tMontorDataInfo->chg.bus_volt >= (uint16_t)p_iniVal[WAR_CHG_VBUS]) {
        if (anytimeFilterVbusOverVolCnt < 1) { // 来不及滤波，滤波会先进入充电状态
            anytimeFilterVbusOverVolCnt++;
            if (anytimeFilterVbusOverVolCnt == 1) {
            	if (sysStatus == CHARGING) { // 只有系统状态为充电时，才更新充电状态位
            		p_tMontorDataInfo->state = CHG_STATE_FAULT;
            		p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
            	}
                add_error_even(FLT_DE_CHG_VBUS);
                log_err_code(FLT_DE_CHG_VBUS);
            }
        }
    } else {
       anytimeFilterVbusOverVolCnt = 0;
    }
    
    if (true == get_battery_eol_status()) {
        if (anytimeFilterBatEolCnt < 1) { //
            anytimeFilterBatEolCnt++;
            if (anytimeFilterBatEolCnt == 1) {
				if (get_subSystem_status() == 0)	// 当有蓝牙事件时，不产生Log
				{
	                add_error_even(FLT_DE_END_OF_LIFE);
	                log_err_code(FLT_DE_END_OF_LIFE);
				}
            }
        }
    } else {
       anytimeFilterBatEolCnt = 0;
    }
}


#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
void	stop_find_my_glo(void)
{
	if (get_subSystem_status() == SUBSTATUS_FIND_ME)
	{
		set_subSystem_status(SUBSTATUS_IDLE);
		if (get_current_ui_detail_status() < UI_HEATTING)
		{
			set_current_ui_detail_status(UI_NONE);
		}
		motor_stop();
		beep_stop();
		bt_adapter_findmy_device_clr(); // Bug. 1905763
	}
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)


/**
  * @brief  检测是否低电错误，并正在显示BLE相关，需要调整显示优先级
  * @param  
  * @return bool
  * @note   None
  */
bool check_ble_ui_when_low_vol(UiTaskDetailStatus_u uiCurrent, UiTaskDetailStatus_u uiStatus)
{
	KeyPressCnt_t *p_key_cnt_handle = get_key_cnt_handle();

	if ((UI_ERR_BAT_LOW == uiStatus) && (uiCurrent >= UI_BLE_FAILED) && (uiCurrent <= UI_CUSTOMIZE))
	{
		return false;
	}
	else if ((UI_EOL == uiStatus) && get_subSystem_status() != 0)	// Gison 增加若有蓝牙相关显示，则不更新EOL显示
	{
		return false;
	}
	else if ((UI_EOL == uiStatus) && (p_key_cnt_handle->bChecking))	// Gison 若正在检测按键，则不更新显示EOL
	{
		return false;
	}

	else if ((UI_ERR_BAT_LOW == uiStatus) && get_subSystem_status() != 0)	// Gison 增加若有蓝牙相关显示，则不更新低压显示
	{
		return false;
	}
	else
	{
		return true;
	}
}

/**
  * @brief  系统空闲状态的交互逻辑处理
  * @param  etEven 外部事件输入
  * @return None
  * @note   None
  */
void system_idle_interaction_logic_proc(ExternalEven_e etEven, UiInfo_t *pUiInfo)
{
    UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
    TaskHandle_t *temp_handle;
    if (pUiInfo->uiStatus != 0) {
 //        sm_log(SM_LOG_INFO, "pUiInfo->uiStatus 0! %d, %d\r\n", pUiInfo->even, pUiInfo->uiStatus);
    }
    switch (etEven) {
        case EXT_EVENT_NONE:
            break;
        case EXT_EVENT_KEY_STANDARD_SHORT_PRESS: // 短按
            if (uiCurrent != pUiInfo->uiStatus  && pUiInfo->uiStatus > uiCurrent && uiCurrent!= UI_NONE) { // 当前正显示错误或其他显示任务，有更高的优先级错误，则更新
				if (check_ble_ui_when_low_vol(uiCurrent, pUiInfo->uiStatus))		// bug 1953170 
				{
	                set_current_ui_detail_status(pUiInfo->uiStatus);
				}

                if (UI_ERR_CRITICAL == pUiInfo->uiStatus || UI_ERR_REBOOT_TIP == pUiInfo->uiStatus || UI_ERR_USB_HOT == pUiInfo->uiStatus ||
                    UI_ERR_GER_WAIT == pUiInfo->uiStatus ||UI_ERR_HOT_WAIT == pUiInfo->uiStatus ||UI_ERR_COLD_WAIT == pUiInfo->uiStatus ||
                    UI_ERR_BAT_OV == pUiInfo->uiStatus || UI_ERR_USB_OV == pUiInfo->uiStatus || UI_ERR_CHARGE_TIMEOUT == pUiInfo->uiStatus ||
                    UI_EOL == pUiInfo->uiStatus) {
//					if (UI_EOL == pUiInfo->uiStatus && get_subSystem_status() == 0) 	// 有蓝牙事件时不产生UI，不记录Log
					if (UI_EOL == pUiInfo->uiStatus) 	// EOL时延后处理
					{
						bDlyDisplayEOL = true;
					}
					else
                    {
	                    log_err_code(pUiInfo->even);
						add_error_even_log(pUiInfo->even);	// bug 1916088
						
	                    if (UI_ERR_CHARGE_TIMEOUT != pUiInfo->uiStatus) {
	                        motor_set2(HAPTIC_ERR);
	                    }
                    }
                }
                sm_log(SM_LOG_INFO, "pUiInfo->uiStatus 1! %d\r\n", pUiInfo->uiStatus);
            } else if (pUiInfo->uiStatus <= uiCurrent && pUiInfo->uiStatus != UI_NONE) { // 当前正显示错误，但是与当前显示的错误相同或更低优先级，不更新
                
            } else if (uiCurrent == UI_NONE && pUiInfo->uiStatus != UI_NONE) {  // 当前无显示， 有错误，则更新
				if (check_ble_ui_when_low_vol(uiCurrent, pUiInfo->uiStatus))		// bug 1953170 
				{
	                set_current_ui_detail_status(pUiInfo->uiStatus);
				}
                if (UI_ERR_CRITICAL == pUiInfo->uiStatus || UI_ERR_REBOOT_TIP == pUiInfo->uiStatus || UI_ERR_USB_HOT == pUiInfo->uiStatus ||
                    UI_ERR_GER_WAIT == pUiInfo->uiStatus ||UI_ERR_HOT_WAIT == pUiInfo->uiStatus ||UI_ERR_COLD_WAIT == pUiInfo->uiStatus ||
                    UI_ERR_BAT_OV == pUiInfo->uiStatus || UI_ERR_USB_OV == pUiInfo->uiStatus || UI_ERR_CHARGE_TIMEOUT == pUiInfo->uiStatus ||
                    UI_EOL == pUiInfo->uiStatus) {
//					if (UI_EOL == pUiInfo->uiStatus && get_subSystem_status() == 0) 	// 有蓝牙事件时不产生UI，不记录Log
					if (UI_EOL == pUiInfo->uiStatus) 	// EOL时延后处理
					{
						bDlyDisplayEOL = true;
					}
					else
                    {
	                    log_err_code(pUiInfo->even);
						add_error_even_log(pUiInfo->even);	// bug 1916088
						
	                    if (UI_ERR_CHARGE_TIMEOUT != pUiInfo->uiStatus) {
	                        motor_set2(HAPTIC_ERR);
	                    }
                    }
                }
                 sm_log(SM_LOG_INFO, "pUiInfo->uiStatus2! %d\r\n", pUiInfo->uiStatus);
            } else if (uiCurrent!= UI_NONE && pUiInfo->uiStatus == UI_NONE) { // 当前有显示，无错误，不更新

            } else if (uiCurrent == UI_NONE && pUiInfo->uiStatus == UI_NONE) { // 无显示，无错误 ，显示电量
	            if (true == get_battery_eol_status() && get_subSystem_status() == 0) {
//					motor_set2(HAPTIC_ERR);
//					set_current_ui_detail_status(UI_EOL);
					bDlyDisplayEOL = true;
					break;
	       		} 
			
                if (true == get_usb_status()) { // && (false == get_charge_full_status() 客户要求继续显示电量图标
                    set_current_ui_detail_status(UI_CHARGE_LEVEL);
                } else {
                   	set_current_ui_detail_status(UI_LEVEL);
                }
            }
            break;
        case EXT_EVENT_KEY_BOOST_SHORT_PRESS: // 短按
            if (uiCurrent != pUiInfo->uiStatus  && pUiInfo->uiStatus > uiCurrent && uiCurrent!= UI_NONE) { // 当前正显示错误或其他显示任务，有更高的优先级错误，则更新
				if (check_ble_ui_when_low_vol(uiCurrent, pUiInfo->uiStatus))		// bug 1953170 
				{
	                set_current_ui_detail_status(pUiInfo->uiStatus);
				}
                if (UI_ERR_CRITICAL == pUiInfo->uiStatus || UI_ERR_REBOOT_TIP == pUiInfo->uiStatus || UI_ERR_USB_HOT == pUiInfo->uiStatus ||
                    UI_ERR_GER_WAIT == pUiInfo->uiStatus ||UI_ERR_HOT_WAIT == pUiInfo->uiStatus ||UI_ERR_COLD_WAIT == pUiInfo->uiStatus ||
                    UI_ERR_BAT_OV == pUiInfo->uiStatus || UI_ERR_USB_OV == pUiInfo->uiStatus || UI_ERR_CHARGE_TIMEOUT == pUiInfo->uiStatus ||
                    UI_EOL == pUiInfo->uiStatus) {
//					if (UI_EOL == pUiInfo->uiStatus && get_subSystem_status() == 0) 	// 有蓝牙事件时不产生UI，不记录Log
					if (UI_EOL == pUiInfo->uiStatus) 	// EOL时延后处理
					{
						bDlyDisplayEOL = true;
					}
					else
                    {
	                    log_err_code(pUiInfo->even);
						add_error_even_log(pUiInfo->even);	// bug 1916088
						
	                    if (UI_ERR_CHARGE_TIMEOUT != pUiInfo->uiStatus) {
	                        motor_set2(HAPTIC_ERR);
	                    }
                    }
                }
            } else if (pUiInfo->uiStatus <= uiCurrent && pUiInfo->uiStatus != UI_NONE) { // 当前正显示错误，但是与当前显示的错误相同或更低优先级，不更新
                
            } else if (uiCurrent == UI_NONE && pUiInfo->uiStatus != UI_NONE) {  // 当前无显示， 有错误，则更新
				if (check_ble_ui_when_low_vol(uiCurrent, pUiInfo->uiStatus))		// bug 1953170 
				{
	                set_current_ui_detail_status(pUiInfo->uiStatus);
				}
                if (UI_ERR_CRITICAL == pUiInfo->uiStatus || UI_ERR_REBOOT_TIP == pUiInfo->uiStatus || UI_ERR_USB_HOT == pUiInfo->uiStatus ||
                    UI_ERR_GER_WAIT == pUiInfo->uiStatus ||UI_ERR_HOT_WAIT == pUiInfo->uiStatus ||UI_ERR_COLD_WAIT == pUiInfo->uiStatus ||
                    UI_ERR_BAT_OV == pUiInfo->uiStatus || UI_ERR_USB_OV == pUiInfo->uiStatus || UI_ERR_CHARGE_TIMEOUT == pUiInfo->uiStatus ||
                    UI_EOL == pUiInfo->uiStatus) {
//					if (UI_EOL == pUiInfo->uiStatus && get_subSystem_status() == 0)		// 有蓝牙事件时不产生UI，不记录Log
					if (UI_EOL == pUiInfo->uiStatus) 	// EOL时延后处理
                    {
						bDlyDisplayEOL = true;
					}
					else
                    {
	                    log_err_code(pUiInfo->even);
						add_error_even_log(pUiInfo->even);	// bug 1916088
						
	                    if (UI_ERR_CHARGE_TIMEOUT != pUiInfo->uiStatus) {
	                        motor_set2(HAPTIC_ERR);
	                    }
                    }
                }
            } else if (uiCurrent!= UI_NONE && pUiInfo->uiStatus == UI_NONE) { // 当前有显示，无错误，不更新

            } else if (uiCurrent == UI_NONE && pUiInfo->uiStatus == UI_NONE) { // 无显示，无错误 ，显示电量
	            if (true == get_battery_eol_status() && get_subSystem_status() == 0) {
//					motor_set2(HAPTIC_ERR);
//					set_current_ui_detail_status(UI_EOL);
					bDlyDisplayEOL = true;
					break;
	       		} 

				if (true == get_usb_status()) { // && (false == get_charge_full_status() 客户要求继续显示电量图标
                    set_current_ui_detail_status(UI_CHARGE_LEVEL);
                } else {
                   	set_current_ui_detail_status(UI_LEVEL);
                }
            }
            break;
        case EXT_EVENT_KEY_STANDARD_HEAT_PRESS: // 加热, 查找是否有不能加热的错误，如果有，则不加热并显示对应UI， 如果没有，则加热
            if (pre_heat_check_error(uiCurrent)) {
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_system_status(HEATTING_STANDARD);
                set_heat_type(HEAT_BY_KEY);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_HEATTING);
                motor_set2(HAPTIC_1);
            }
            break;
        case EXT_EVENT_KEY_BOOST_HEAT_PRESS: // 加热, 查找是否有不能加热的错误，如果有，则不加热并显示对应UI， 如果没有，则加热
            if (pre_heat_check_error(uiCurrent)) {
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_system_status(HEATTING_BOOST);
                set_heat_type(HEAT_BY_KEY);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_HEATTING);
                motor_set2(HAPTIC_5);
            }
            break;
        case EXT_EVENT_KEY_CLEAN_HEAT_PRESS: // 干烧清洁
            if (pre_heat_check_error(uiCurrent)) {
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_system_status(HEATTING_CLEAN);
                set_heat_type(HEAT_BY_KEY);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_CLEAN_PRO);
                motor_set2(HAPTIC_2);
            }
            break;
        case EXT_EVENT_KEY_STANDARD_STOP_HEAT_PRESS: // 停止加热, 器具正常情况不会有此事件
            break;
        case EXT_EVENT_KEY_BOOST_STOP_HEAT_PRESS: // 停止加热, 器具正常情况不会有此事件
            break;
        case EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS: // 退出船运模式
            set_current_ui_detail_status(UI_SHIPPING);
            break;
        case EXT_EVENT_KEY_BT_PAIRED_PRESS: // 蓝牙配对，有错误（除了严重错误）或电量显示，则切换显示
            break;
        case EXT_EVENT_PCTOOL_CMD_TEST_HEAT: // 指令测试模式加热
            set_system_status(HEATTING_TEST);
            temp_handle = get_task_heatting_handle();
            xTaskNotifyGive(*temp_handle);
            motor_set2(HAPTIC_1);
            break;
        case EXT_EVENT_PCTOOL_CMD_BOOST_HEAT: // 指令boost模式加热
            if (pre_heat_check_error(uiCurrent)) { 
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_system_status(HEATTING_BOOST);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_5);
            }
            break;
        case EXT_EVENT_PCTOOL_CMD_STANDARD_HEAT: // 指令STANDARD模式加热
            if (pre_heat_check_error(uiCurrent)) { 
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_system_status(HEATTING_STANDARD);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_1);
            }
            break;
        case EXT_EVENT_PCTOOL_CMD_CLEAN_HEAT: // 干烧清洁
            if (pre_heat_check_error(uiCurrent)) {
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_system_status(HEATTING_CLEAN);
                set_heat_type(HEAT_BY_PCTOOL);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_CLEAN_PRO);
                motor_set2(HAPTIC_2);
            }
            break;
        case EXT_EVENT_PCTOOL_CMD_REF_BASE_HEAT:
            if (pre_heat_check_error(uiCurrent)) { 
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                //设置 REF 加热标志位
                set_ref_heat_sta(HEATTING_STANDARD);
                set_system_status(HEATTING_STANDARD);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_1);
            }
        break;
        case EXT_EVENT_PCTOOL_CMD_REF_BOOST_HEAT:
            if (pre_heat_check_error(uiCurrent)) { 
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_ref_heat_sta(HEATTING_BOOST);
                set_system_status(HEATTING_BOOST);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_5);
            }
        break;
        case EXT_EVENT_PCTOOL_CMD_STOP_HEAT: // 指令 停止加热, 已在空闲状态，无需停止
            break;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		case EXT_EVENT_USB_PLUG_HAL:		// bug 1968519
		case EXT_EVENT_USB_UNPLUG_HAL:
			if (true == app_bt_service_char_findmy_device_is_found())
			{
				stop_find_my_glo();
			}
			break;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

		case EXT_EVENT_USB_PLUG: // USB PLUG事件 充电
            if (pre_charge_check_error(uiCurrent)) {
                // 此处添加启动充电
                set_system_status(CHARGING);
                // 添加显示充电UI
                temp_handle = get_task_charge_handle();
                xTaskNotifyGive(*temp_handle);
                // 添加显示充电UI
	            if (true == get_battery_eol_status() && get_subSystem_status() == 0) {	// 当有蓝牙事件时，不产生相应的UI和Log
                    if (uiCurrent == UI_EOL) { // 如果正在显示，则不再显示和振动
						motor_set2(HAPTIC_1);
                    } else {               
						log_err_code(FLT_DE_END_OF_LIFE);
						add_error_even_log(pUiInfo->even);	// Bug 1928243 
                        motor_set2(HAPTIC_ERR);
                        set_current_ui_detail_status(UI_EOL);
                    }
					break;
	       		}

            	motor_set2(HAPTIC_1);
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)				
				if (uiCurrent < UI_CHARGE_LEVEL)
				{
            		set_current_ui_detail_status(UI_CHARGE_LEVEL);// 如果是在加热中切换到充电需要UI显示完再进行充电，不然UI和振动会不同步,需要事件延迟处理
					bDispUICharg = false;
				}
				else
				{
					bDispUICharg = true;
				}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            } else {
            	chg_set_chg_en(0); // 发生异常关闭充电
            }
            break;
        case EXT_EVENT_USB_UNPLUG: // 已在空闲状态，无需停止充电,需显示电量
            set_charge_full_status(false);
            if (pUiInfo->uiStatus == UI_ERR_BAT_OV || pUiInfo->uiStatus == UI_ERR_USB_OV || pUiInfo->uiStatus == UI_ERR_USB_HOT  || pUiInfo->uiStatus == UI_ERR_CHARGE_TIMEOUT ) { // 拔出就恢复的提示不需要再提示,
                if (uiCurrent == UI_NONE) { // 无显示 显示电量
                    set_current_ui_detail_status(UI_LEVEL);
                }
                break;
            }
            if (uiCurrent != pUiInfo->uiStatus  && pUiInfo->uiStatus > uiCurrent && uiCurrent!= UI_NONE) { // 当前正显示错误或其他显示任务，有更高的优先级错误，则更新
				if (check_ble_ui_when_low_vol(uiCurrent, pUiInfo->uiStatus))		// bug 1953170 
				{
	                set_current_ui_detail_status(pUiInfo->uiStatus);
				}
                if (UI_ERR_CRITICAL == pUiInfo->uiStatus || UI_ERR_REBOOT_TIP == pUiInfo->uiStatus || UI_ERR_USB_HOT == pUiInfo->uiStatus ||
                    UI_ERR_GER_WAIT == pUiInfo->uiStatus ||UI_ERR_HOT_WAIT == pUiInfo->uiStatus ||UI_ERR_COLD_WAIT == pUiInfo->uiStatus ||
                    UI_ERR_BAT_OV == pUiInfo->uiStatus || UI_ERR_USB_OV == pUiInfo->uiStatus) {
                    log_err_code(pUiInfo->even);
					
					add_error_even_log(pUiInfo->even);	// bug 1916088
					
                    motor_set2(HAPTIC_ERR);
                }
                 sm_log(SM_LOG_INFO, "pUiInfo->uiStatus 1! %d\r\n", pUiInfo->uiStatus);
            } else if (pUiInfo->uiStatus <= uiCurrent && pUiInfo->uiStatus != UI_NONE) { // 当前正显示错误，但是与当前显示的错误相同或更低优先级，不更新
                
            } else if (uiCurrent == UI_NONE && pUiInfo->uiStatus != UI_NONE) {  // 当前无显示， 有错误，则更新
				if (check_ble_ui_when_low_vol(uiCurrent, pUiInfo->uiStatus))		// bug 1953170 
				{
	                set_current_ui_detail_status(pUiInfo->uiStatus);
				}
                if (UI_ERR_CRITICAL == pUiInfo->uiStatus || UI_ERR_REBOOT_TIP == pUiInfo->uiStatus || UI_ERR_USB_HOT == pUiInfo->uiStatus ||
                    UI_ERR_GER_WAIT == pUiInfo->uiStatus ||UI_ERR_HOT_WAIT == pUiInfo->uiStatus ||UI_ERR_COLD_WAIT == pUiInfo->uiStatus ||
                    UI_ERR_BAT_OV == pUiInfo->uiStatus || UI_ERR_USB_OV == pUiInfo->uiStatus) {
                    log_err_code(pUiInfo->even);
					
					add_error_even_log(pUiInfo->even);	// bug 1916088
					
                    motor_set2(HAPTIC_ERR);
                }
                 sm_log(SM_LOG_INFO, "pUiInfo->uiStatus2! %d\r\n", pUiInfo->uiStatus);
            } else if (uiCurrent!= UI_NONE && pUiInfo->uiStatus == UI_NONE) { // 当前有显示，无错误，不更新

            } else if (uiCurrent == UI_NONE && pUiInfo->uiStatus == UI_NONE) { // 无显示，无错误 ，显示电量
                set_current_ui_detail_status(UI_LEVEL);
            }
            break;
        case EXT_EVENT_BOOT: // 开机事件
            set_current_ui_detail_status(UI_BOOT_TIP);
            motor_set2(HAPTIC_1);
            break;
        case EXT_EVENT_BOOT_TIP: // 用户重启倒计时事件
            if (pUiInfo->uiStatus == UI_ERR_REBOOT_TIP) {
             //   set_current_ui_detail_status(UI_ERR_REBOOTING);
            } else if (pUiInfo->uiStatus == UI_ERR_CRITICAL){ // 严重错误不能重启
                
            } else { // 其他UI,用户主动重启，开始倒计时
                set_current_ui_detail_status(UI_ERR_REBOOTING);
            }
            break;
        default:
            break;
    }
    set_system_external_even(EXT_EVENT_NONE); // 清除事件，避免重复执行该事件动作
}


/**
  * @brief  系统加热状态的交互逻辑处理
  * @param  etEven 外部事件输入
  * @return None
  * @note   在加热状态中，除了要处理外部事件的停止加热，还需要处理错误码来判断是否需要停止加热，如有错误码直接显示错误码，否则停止加热后显示拔烟
  */
void system_heating_interaction_logic_proc(ExternalEven_e etEven, UiInfo_t *pUiInfo)
{
    UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
    if (false == heat_check_error(uiCurrent)) { // 发生不可加热的错误
        set_system_status(IDLE);
        vTaskDelay(50);
        return;
    }
    switch (etEven) {
        case EXT_EVENT_NONE:
            break;
        case EXT_EVENT_KEY_STANDARD_SHORT_PRESS: // 短按加热状态，不需要响应
            break;
        case EXT_EVENT_KEY_BOOST_SHORT_PRESS: // 短按，加热状态不需要响应

            break;
        case EXT_EVENT_KEY_STANDARD_HEAT_PRESS: // 加热状态不需要响应

            break;
        case EXT_EVENT_KEY_BOOST_HEAT_PRESS:  // 加热状态，不需要响应

            break;
        case EXT_EVENT_KEY_STANDARD_STOP_HEAT_PRESS: // 停止加热, 判断是否需要充电，如需要则切换：改为先致空闲，再由主任务的另一个接口来判断是否恢复充电（即USB重置重新插入事件）
            set_system_status(IDLE);
            vTaskDelay(50); // 让加热任务有足够时间停止加热
            break;
        case EXT_EVENT_KEY_BOOST_STOP_HEAT_PRESS: // 停止加热， 判断是否需要充电，如需要则切换：改为先致空闲，再由主任务的另一个接口来判断是否恢复充电（即USB重置重新插入事件）
            set_system_status(IDLE);
            vTaskDelay(50);// 让加热任务有足够时间停止加热
            break;
        case EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS: // 退出船运模式, 加热状态，不需要响应
            break;
        case EXT_EVENT_KEY_BT_PAIRED_PRESS: // 蓝牙配对，有错误（除了严重错误）或电量显示，则切换显示
            break;
        case EXT_EVENT_PCTOOL_CMD_TEST_HEAT: // 指令测试模式加热，已在加热状态，不需要响应
            break;
        case EXT_EVENT_PCTOOL_CMD_BOOST_HEAT: // 指令boost模式加热，已在加热状态，不需要响应
            break;
        case EXT_EVENT_PCTOOL_CMD_STANDARD_HEAT: // 指令STANDARD模式加热，已在加热状态，不需要响应
            break;
        case EXT_EVENT_PCTOOL_CMD_STOP_HEAT: // 指令 停止加热 判断是否需要充电，如需要则切换
            set_system_status(IDLE);
            vTaskDelay(50);// 让加热任务有足够时间停止加热
            break;
        case EXT_EVENT_BOOT_TIP: // 重启倒计时事件，manual reset
            set_system_status(IDLE);
            vTaskDelay(50);// 让加热任务有足够时间停止加热
            set_current_ui_detail_status(UI_ERR_REBOOTING);
            break;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		case EXT_EVENT_USB_PLUG_HAL:
		case EXT_EVENT_USB_UNPLUG_HAL:
			if (true == app_bt_service_char_findmy_device_is_found())
			{
				stop_find_my_glo();
			}
			break;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			
        default:
            break;
    }
    set_system_external_even(EXT_EVENT_NONE); // 清除事件，避免重复执行该事件动作
}

/**
  * @brief  系统充电状态的交互逻辑处理
  * @param  etEven 外部事件输入
  * @return None
  * @note   None
  */
void system_charging_interaction_logic_proc(ExternalEven_e etEven, UiInfo_t *pUiInfo)
{
    UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
    TaskHandle_t *temp_handle;
    if (false == charging_check_error(uiCurrent)) { // 发生不可充电的错误或者充电满
        set_system_status(IDLE);
        vTaskDelay(50);// 让充电任务有足够时间停止充电
        return;
    }
    system_error_recover_on_chargeing_proc();
    switch (etEven) {
        case EXT_EVENT_NONE:
            break;
        case EXT_EVENT_KEY_STANDARD_SHORT_PRESS: // 短按充电状态，显示电量
            if (uiCurrent < UI_CHARGE_LEVEL) {
                set_current_ui_detail_status(UI_CHARGE_LEVEL);
            }
            break;
        case EXT_EVENT_KEY_BOOST_SHORT_PRESS: // 短按充电状态，显示电量
            if (uiCurrent < UI_CHARGE_LEVEL) {
                set_current_ui_detail_status(UI_CHARGE_LEVEL);
            }
            break;
        case EXT_EVENT_KEY_STANDARD_HEAT_PRESS: // 需要切换到加热状态
            if (pre_heat_check_error(uiCurrent)) { 
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
                // 此处添加启动加热
                set_system_status(HEATTING_STANDARD);
                set_heat_type(HEAT_BY_KEY);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_HEATTING);
                motor_set2(HAPTIC_1);
            }
            break;
        case EXT_EVENT_KEY_BOOST_HEAT_PRESS:  // 需要切换到加热状态
            if (pre_heat_check_error(uiCurrent)) { 
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
                // 此处添加启动加热
                set_system_status(HEATTING_BOOST);
                set_heat_type(HEAT_BY_KEY);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_HEATTING);
                motor_set2(HAPTIC_5);
            }
            break;
        case EXT_EVENT_KEY_CLEAN_HEAT_PRESS: // 干烧清洁
            if (pre_heat_check_error(uiCurrent)) {
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
                // 此处添加启动加热
                set_system_status(HEATTING_CLEAN);
                set_heat_type(HEAT_BY_KEY);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                set_current_ui_detail_status(UI_CLEAN_PRO);
                motor_set2(HAPTIC_2);
            }
            break;
        case EXT_EVENT_KEY_STANDARD_STOP_HEAT_PRESS: // 停止加热, 充电状态，无需响应

            break;
        case EXT_EVENT_KEY_BOOST_STOP_HEAT_PRESS: // 停止加热, 充电状态，无需响应

            break;
        case EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS: // 退出船运模式, 充电状态，不需要响应
            break;
        case EXT_EVENT_KEY_BT_PAIRED_PRESS: // 蓝牙配对，有错误（除了严重错误）或电量显示，则切换显示
            break;
        case EXT_EVENT_PCTOOL_CMD_TEST_HEAT: // 指令测试模式加热， 工厂测试，暂时不考虑充电过程切换到加热
            set_system_status(IDLE);
            vTaskDelay(50);// 让充电任务有足够时间停止充电
            set_system_status(HEATTING_TEST);
            temp_handle = get_task_heatting_handle();
            xTaskNotifyGive(*temp_handle);
            break;
        case EXT_EVENT_PCTOOL_CMD_BOOST_HEAT: // 指令boost模式加热，需要切换到加热状态
            if (pre_heat_check_error(uiCurrent)) {
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
                // 此处添加启动加热
                set_system_status(HEATTING_BOOST);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_5);
            }
            break;
        case EXT_EVENT_PCTOOL_CMD_STANDARD_HEAT: // 指令STANDARD模式加热，需要切换到加热状态
            if (pre_heat_check_error(uiCurrent)) {
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
                // 此处添加启动加热
                set_system_status(HEATTING_STANDARD);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_1);
            }
            break;
        case EXT_EVENT_PCTOOL_CMD_CLEAN_HEAT: // 干烧清洁
            if (pre_heat_check_error(uiCurrent)) {
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
                set_system_status(HEATTING_CLEAN);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_CLEAN_PRO);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_2);
            }
            break;
        case EXT_EVENT_PCTOOL_CMD_REF_BASE_HEAT:
            if (pre_heat_check_error(uiCurrent)) { 
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                //设置 REF 加热标志位
                set_ref_heat_sta(HEATTING_STANDARD);
                set_system_status(HEATTING_STANDARD);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_1);
            }
        break;
        case EXT_EVENT_PCTOOL_CMD_REF_BOOST_HEAT:
            if (pre_heat_check_error(uiCurrent)) { 
                // 此处添加启动加热
                if (uiCurrent == UI_HEATTING) { // 上一次显示UI有延迟，如果在拔烟开始之后可以提前加热
                    set_current_ui_detail_status(UI_NONE);
                    while (0 != get_ui_display_status()) {
                        vTaskDelay(50);
                    }
                }
                set_ref_heat_sta(HEATTING_BOOST);
                set_system_status(HEATTING_BOOST);
                set_heat_type(HEAT_BY_PCTOOL);
                set_current_ui_detail_status(UI_HEATTING);
                temp_handle = get_task_heatting_handle();
                xTaskNotifyGive(*temp_handle);
                motor_set2(HAPTIC_5);
            }
        break;
        case EXT_EVENT_PCTOOL_CMD_STOP_HEAT: // 指令 停止加热 , 充电状态，不需要响应
            break;
        case EXT_EVENT_BOOT_TIP: // 重启倒计时事件，manual reset
            //set_system_status(IDLE);
            //vTaskDelay(50);// 让充电任务有足够时间退出
            set_current_ui_detail_status(UI_ERR_REBOOTING);
            break;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		case EXT_EVENT_USB_PLUG_HAL:
		case EXT_EVENT_USB_UNPLUG_HAL:
			if (true == app_bt_service_char_findmy_device_is_found())
			{
				stop_find_my_glo();
			}
			break;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

        case EXT_EVENT_USB_UNPLUG: // USB拔出事件,如果是充电状态则置为空闲状态,接上USB是运行加热的
            set_charge_full_status(false);
            if (CHARGING == get_system_status()) {
                set_system_status(IDLE);
                vTaskDelay(50);// 让充电任务有足够时间停止充电
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				bDispUICharg = false;
				if (uiCurrent <= UI_CHARGE_LEVEL) // 充电拔出会打断蓝牙部分；充电中UI显示时，拔出USB要显示电量页面-Bug 1934953
				{
					set_current_ui_detail_status(UI_LEVEL);
				}
#else
				set_current_ui_detail_status(UI_LEVEL);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            }
            break;
        default:
            break;
    }
    set_system_external_even(EXT_EVENT_NONE); // 清除事件，避免重复执行该事件动作
}

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)

//bool ble_interaction_delay_handle_for_eol(void)		// 因为显示EOL而延迟处理的事件，无，则返回false
//{
//	UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
//	SysSubStatus_u subSysStatus = get_subSystem_status();
//	ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
//
//	// 查找中，优先处理
//   if ((SUBSTATUS_FIND_ME == subSysStatus) && (uiCurrent == UI_ERR_BAT_LOW || uiCurrent <= UI_CHARGE_LEVEL || uiCurrent == UI_EOL || uiCurrent == UI_BLE_FIND_ME))
//   {
//	   set_current_ui_detail_status(UI_BLE_FIND_ME);
//	   g_eol_delay_handle_bits = 0;
//	   return true;
//   }
//
//// 仅支持缓冲一次，显示优先级顺序为 qr_code, Find
//// 最低优先级为Lock、unLock
//	if ((g_eol_delay_handle_bits & (1 << B_EOL_PARING_QRCODE)) && (subSysStatus == SUBSTATUS_PARING_QRCODE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)	// Bug 1964706 若UI不显示QR_Code，则不广播
//		{
//			set_current_ui_detail_status(UI_BLE_DOWNLOAD);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}	
//	}
////	else if (g_eol_delay_handle_bits & (1 << B_EOL_FIND_MY_GLO))	// 因为收到BLE指令后Beep就动作，所以停用该处
////	{
////		motor_strength_set(100);
////		amoled_brigth_set(100);
////
////		uint32_t temp;
////		temp = amoled_brigth_get();
////		temp *= 255;
////		temp /= 100;
////		driver_rm69600_write_command(0x51);
////		driver_rm69600_write_data((uint8_t)temp);
////	
////		timetick = msTickDev->read( NULL, 4);
////		set_subSystem_status(SUBSTATUS_FIND_ME);
////		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
////		{
////			set_current_ui_detail_status(UI_BLE_FIND_ME);
////		}
////		motor_set2(HAPTIC_FIND_ME);			// 查找过程，就算Session状态无相应显示，也需要振动马达，
////	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_PARING)) && (subSysStatus == SUBSTATUS_PARING))
//	{
//		if ((uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL))
//		{
//			set_current_ui_detail_status(UI_BLE_PAIRING);			
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_UIMAGE_CUSTOMIZE)) && (subSysStatus == SUBSTATUS_IDLE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL) // 低电量的时候需要显示蓝牙
//		{
//			set_current_ui_detail_status(UI_CUSTOMIZE);// Display Uimage Upgrade UI
//			motor_set2(HAPTIC_1);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_HEATING_PROFILES)) && (subSysStatus == SUBSTATUS_IDLE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL) // 低电量的时候需要显示蓝牙
//		{
//			set_current_ui_detail_status(UI_SET_PROFILE);// Display Set_Profile Upgrade UI
//			motor_set2(HAPTIC_1);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}	
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_HEATING_PROFILES_BOOST)) && (subSysStatus == SUBSTATUS_IDLE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL) // 低电量的时候需要显示蓝牙
//		{
//			set_current_ui_detail_status(UI_SET_PROFILE_BOOST);// Display Set_Profile Upgrade UI
//			motor_set2(HAPTIC_1);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_LOADING_OK)) && (subSysStatus == SUBSTATUS_IDLE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL) // 低电量的时候需要显示蓝牙
//		{
//			set_current_ui_detail_status(UI_SHIPPING);
//			motor_set2(HAPTIC_1);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_LOADING)) && (subSysStatus == SUBSTATUS_LOADING))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
//		{
//			set_current_ui_detail_status(UI_LOADING);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_PARI_OK)) && (subSysStatus == SUBSTATUS_IDLE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
//		{
//			set_current_ui_detail_status(UI_BLE_PAIRED);
//			motor_set2(HAPTIC_1);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_PARI_FAIL)) && (subSysStatus == SUBSTATUS_IDLE))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
//		{
//			set_current_ui_detail_status(UI_BLE_FAILED);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_UNLOCK)) && (get_lock_mode() == false))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
//		{
//			set_current_ui_detail_status(UI_BLE_UNLOCK);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else if ((g_eol_delay_handle_bits & (1 << B_EOL_LOCK)) && (get_lock_mode() == true))
//	{
//		if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
//		{
//			set_current_ui_detail_status(UI_BLE_LOCK);
//			g_eol_delay_handle_bits = 0;
//			return true;
//		}
//		else
//		{
//			g_eol_delay_handle_bits = 0;
//			return false;
//		}
//	}
//	else
//	{
//		g_eol_delay_handle_bits = 0;
//		return false;
//	}
//	
//	g_eol_delay_handle_bits = 0;
//	return true;
//}

bool ble_interaction_delay_handle_for_eol(void)		// 因为显示EOL而延迟处理的事件，无，则返回false
{
	UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
	SysSubStatus_u subSysStatus = get_subSystem_status();
	ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

//	// 查找中，优先处理
	 if (uiCurrent == UI_ERR_BAT_LOW || uiCurrent <= UI_CHARGE_LEVEL || uiCurrent == UI_EOL)
	 {
		 if (SUBSTATUS_FIND_ME == subSysStatus)
		 {
				set_current_ui_detail_status(UI_BLE_FIND_ME);
				return true;
		 }
		 else if (SUBSTATUS_LOADING == subSysStatus)
		 {
				set_current_ui_detail_status(UI_LOADING);
				return true;
		 }
		 else if (SUBSTATUS_PARING_QRCODE == subSysStatus)
		 {
				set_current_ui_detail_status(UI_BLE_DOWNLOAD);
				return true;
		 }
		else if (SUBSTATUS_PARING == subSysStatus)
		{
			set_current_ui_detail_status(UI_BLE_PAIRING);
			return true;
		}
	 }
	 return false;
}

void subSystem_idle_interaction_logic_proc(ExternalEven_e etEven, UiInfo_t *pUiInfo)
{
    UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();
	ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
	
//	static uint32_t timetick;

    if (pUiInfo->uiStatus != 0) 
	{
//        sm_log(SM_LOG_INFO, "pUiInfo->uiStatus 0! %d, %d\r\n", pUiInfo->even, pUiInfo->uiStatus);
    }

// 超时状态转换
	SysSubStatus_u subSysStatus = get_subSystem_status();


// bug 1977067 点烟完成后，若还在查找状态，则显示UI
   if (SUBSTATUS_FIND_ME == subSysStatus)
   {
	  if (uiCurrent <= UI_CHARGE_LEVEL)
	  {
		  set_current_ui_detail_status(UI_BLE_FIND_ME);
	  }
   }
   else if (SUBSTATUS_LOADING == subSysStatus)
   {
	  if (uiCurrent <= UI_CHARGE_LEVEL)
	  {
		  set_current_ui_detail_status(UI_LOADING);
	  }
   }
   else if (SUBSTATUS_PARING_QRCODE == subSysStatus)
   {
	  if (uiCurrent <= UI_CHARGE_LEVEL)
	  {
		  set_current_ui_detail_status(UI_BLE_DOWNLOAD);
	  }
   }
	else if (SUBSTATUS_PARING == subSysStatus)
	{
		if (uiCurrent <= UI_CHARGE_LEVEL)
		{
		   set_current_ui_detail_status(UI_BLE_PAIRING);
		}
	}

	uiCurrent = get_current_ui_detail_status(); 	  // 更新寄存器


	switch (subSysStatus)
	{
//		case SUBSTATUS_FIND_ME:
//		{
//			if (msTickDev->read( NULL, 4) - timetick > 60000)	// 60秒后停止
//			{
//				if (etEven == 0)
//				{
//					set_subSystem_status(SUBSTATUS_IDLE);
//					if (uiCurrent < UI_HEATTING)
//					{
//						set_current_ui_detail_status(UI_NONE);
//					}
////					motor_stop();								// 马达振动跟Beep一致，该处不Stop
//				}
//			}
//			break;
//		}
		case SUBSTATUS_PARING_QRCODE:
		{
			if (msTickDev->read( NULL, 4) - timetick > 60000)	// 60秒后进入配对
			{
				if (etEven == 0)
				{
					etEven = EXT_EVENT_PARING;
					set_subSystem_status(SUBSTATUS_PARING);
				}
			}
			break;
		}
		case SUBSTATUS_PARING:
		{
			if (msTickDev->read( NULL, 4) - timetick > 120000)	// 配对时间120秒
			{
				if (etEven == 0)
				{
					etEven = EXT_EVENT_PARI_FAIL;
					set_subSystem_status(SUBSTATUS_IDLE);

                    //配对超时，停止广播
                    app_bt_adv_stop();
				}
			}
			break;
		}
		default:
			break;		
	}

// 事件处理
    switch (etEven) 
	{
		case EXT_EVENT_LOCK:
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_LOCK);
			}
			else */ if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
			{
				set_current_ui_detail_status(UI_BLE_LOCK);
				bDlyDisplayEOL = false;
			}
			break;

		case EXT_EVENT_UNLOCK:
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_UNLOCK);
			}
			else */if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
			{
				set_current_ui_detail_status(UI_BLE_UNLOCK);
				bDlyDisplayEOL = false;
			}
			break;

		case EXT_EVENT_PARING_QRCODE:
			if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || (uiCurrent == UI_EOL))	// Bug 1964706 若UI不显示QR_Code，则不广播
//			if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)	// Bug 1964706 若UI不显示QR_Code，则不广播
			{
				if (0 == app_bt_enter_pairing_mode())//进入配对模式
				{
					timetick = msTickDev->read( NULL, 4);
					set_subSystem_status(SUBSTATUS_PARING_QRCODE);
//					if (uiCurrent == UI_EOL)							// 延迟处理
//					{
//						g_eol_delay_handle_bits |= (1 << B_EOL_PARING_QRCODE);
//					}
//					else
					{
						set_current_ui_detail_status(UI_BLE_DOWNLOAD);
						bDlyDisplayEOL = false;
					}
				}
				else
				{
					sm_log(SM_LOG_INFO, "enter pairing failure\r\n");	// debug
					set_subSystem_status(SUBSTATUS_IDLE);
//					if (uiCurrent == UI_EOL)
//					{
//						g_eol_delay_handle_bits |= (1 << B_EOL_PARI_FAIL);
//					}
//					else
					{
						set_current_ui_detail_status(UI_BLE_FAILED);
						bDlyDisplayEOL = false;
					}
				}
			}
			break;

		case EXT_EVENT_PARING:
			timetick = msTickDev->read( NULL, 4);
			set_subSystem_status(SUBSTATUS_PARING);
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_PARING);
			}
			else if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)*/
			if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
			{
				set_current_ui_detail_status(UI_BLE_PAIRING);
				bDlyDisplayEOL = false;
			}
			break;

		case EXT_EVENT_FIND_MY_GLO:
// bug 1964122	亮度、强度最大值
//			if (uiCurrent == UI_EOL)
//			{
//				g_eol_delay_handle_bits |= (1 << B_EOL_FIND_MY_GLO);
//			}
//			else
			{
				motor_strength_set(100);
				amoled_brigth_set(100);
		
				uint32_t temp;
				temp = amoled_brigth_get();
				temp *= 255;
				temp /= 100;
				driver_rm69600_write_command(0x51);
				driver_rm69600_write_data((uint8_t)temp);
			
				timetick = msTickDev->read( NULL, 4);
				set_subSystem_status(SUBSTATUS_FIND_ME);
//				if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
				if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)	// EOL完了再判断显示，否则电量显示时长不正确
				{
					set_current_ui_detail_status(UI_BLE_FIND_ME);
				}
				motor_set2(HAPTIC_FIND_ME);
			}
			break;

		case EXT_EVENT_FIND_MY_GLO_CANCLE:
// bug 1964122	亮度、强度恢复
			get_motor_strength_from_eeprom();
			get_oled_brightness_from_eeprom();

			uint32_t temp;
			temp = amoled_brigth_get();
			temp *= 255;
			temp /= 100;
			driver_rm69600_write_command(0x51);
			driver_rm69600_write_data((uint8_t)temp);
		
			stop_find_my_glo();
			break;

		case EXT_EVENT_PARING_CANCLE:
			if (subSysStatus == SUBSTATUS_PARING || subSysStatus == SUBSTATUS_PARING_QRCODE)
			{
				set_subSystem_status(SUBSTATUS_IDLE);
				if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
				{
					set_current_ui_detail_status(UI_NONE);
				}
                //关闭广播
                app_bt_adv_stop();
			}
			break;

		case EXT_EVENT_PARI_OK:
		//绑定成功后，关闭广播，发送该事件
			app_bt_adv_stop();
		
			set_subSystem_status(SUBSTATUS_IDLE);
		
//			if (uiCurrent == UI_EOL)
//			{
//				g_eol_delay_handle_bits |= (1 << B_EOL_PARI_OK);
//			}
//			else 
			{
				if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
				{
					set_current_ui_detail_status(UI_BLE_PAIRED);
					motor_set2(HAPTIC_1);
					bDlyDisplayEOL = false;
				}
			}
			break;

		case EXT_EVENT_PARI_FAIL:
            //关闭广播
            app_bt_adv_stop();

			set_subSystem_status(SUBSTATUS_IDLE);
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_PARI_FAIL);
			}
			else */ if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
			{
				set_current_ui_detail_status(UI_BLE_FAILED);
				bDlyDisplayEOL = false;
			}
			break;

		case EXT_EVENT_LOADING:
			timetick = msTickDev->read( NULL, 4);
			set_subSystem_status(SUBSTATUS_LOADING);
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_LOADING);
			}
			else */ if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW || uiCurrent == UI_EOL)
			{
				set_current_ui_detail_status(UI_LOADING);
				bDlyDisplayEOL = false;
			}
			break;

		case EXT_EVENT_LOADING_STOP:
			if (subSysStatus == SUBSTATUS_LOADING)
			{
				set_subSystem_status(SUBSTATUS_IDLE);
				if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
				{
					set_current_ui_detail_status(UI_NONE);
				}
			}
			break;

		case EXT_EVENT_LOADING_OK:
			set_subSystem_status(SUBSTATUS_IDLE);
//			if (uiCurrent == UI_EOL)
//			{
//				g_eol_delay_handle_bits |= (1 << B_EOL_LOADING_OK);
//			}
//			else
			{
				if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)	// 低电量的时候需要显示蓝牙
				{
					set_current_ui_detail_status(UI_SHIPPING);
					motor_set2(HAPTIC_1);
					bDlyDisplayEOL = false;
				}
			}
			break;
        case EXT_EVENT_UIMAGE_CUSTOMIZE:
        {
			if (subSysStatus == SUBSTATUS_LOADING)
			{
				set_subSystem_status(SUBSTATUS_IDLE);
/*				if (uiCurrent == UI_EOL)
				{
					g_eol_delay_handle_bits |= (1 << B_EOL_UIMAGE_CUSTOMIZE);
				}
				else */ if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW) // 低电量的时候需要显示蓝牙
				{
					set_current_ui_detail_status(UI_CUSTOMIZE);// Display Uimage Upgrade UI
					motor_set2(HAPTIC_1);
					bDlyDisplayEOL = false;
				}
			}
//			motor_set2(HAPTIC_1);
        }
            break;
        case EXT_EVENT_HEATING_PROFILES:
        {
			if (subSysStatus == SUBSTATUS_LOADING)
			{
				set_subSystem_status(SUBSTATUS_IDLE);
			}
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_HEATING_PROFILES);
			}
			else */if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW) // 低电量的时候需要显示蓝牙
			{
				set_current_ui_detail_status(UI_SET_PROFILE);// Display Set_Profile Upgrade UI
				motor_set2(HAPTIC_1);
				bDlyDisplayEOL = false;
			}		
        }
            break;

        case EXT_EVENT_HEATING_PROFILES_BOOST:
        {
			if (subSysStatus == SUBSTATUS_LOADING)
			{
				set_subSystem_status(SUBSTATUS_IDLE);
			}
/*			if (uiCurrent == UI_EOL)
			{
				g_eol_delay_handle_bits |= (1 << B_EOL_HEATING_PROFILES_BOOST);
			}
			else */ if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW) // 低电量的时候需要显示蓝牙
			{
				set_current_ui_detail_status(UI_SET_PROFILE_BOOST);// Display Set_Profile Upgrade UI
				motor_set2(HAPTIC_1);
				bDlyDisplayEOL = false;
			}		
        }
            break;
        default:
            break;
    }
//    set_system_external_even(EXT_EVENT_NONE); // 清除事件，避免重复执行该事件动作
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

/**
  * @brief  系统交互逻辑处理
  * @param  None
  * @return None
  * @note   None
  */
void system_interaction_logic_proc(void)
{
    system_anytime_error_code_check(); // 放前面， 第一时间检测到错误，确保按键时第一时间显示

    ExternalEven_e etEven = get_system_external_even();
    SysStatus_u sysStatus = get_system_status();
    UiInfo_t *pUiInfo = get_error_high_pro_even();

	UiTaskDetailStatus_u uiCurrent= get_current_ui_detail_status();

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
// 检测按键多击情况
	static uint8_t uKeyPressCnt = 0;
	static bool bUpdate = false;

	KeyPressCnt_t *p_key_cnt_handle = get_key_cnt_handle();
	if (p_key_cnt_handle->bChecking)
	{
		ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
//	bug. 1954307 显示二维码图标需要延时一点时间，否则跟低压错误显示同时而忽略不显示QR而显示低电压
		if ((msTickDev->read( NULL, 4) - p_key_cnt_handle->timeTick > 600) || ((p_key_cnt_handle->cnt1 > 4 || p_key_cnt_handle->cnt2 > 4) && (msTickDev->read( NULL, 4) - p_key_cnt_handle->timeTick > 100)))			// 当低电压错误时，按5次，马上配对会仅显示低压错误
		{
			if (p_key_cnt_handle->cnt1 && p_key_cnt_handle->cnt2 == 0)
			{
				uKeyPressCnt = p_key_cnt_handle->cnt1;
			}
			else if (p_key_cnt_handle->cnt2 && p_key_cnt_handle->cnt1 == 0)
			{
				uKeyPressCnt = p_key_cnt_handle->cnt2;
			}
			else
			{
				uKeyPressCnt = 0;
			}
			clr_key_cnt();
			sm_log(SM_LOG_INFO, "Key Press Cnt = %d.\r\n", uKeyPressCnt);	// debug
			
			bUpdate = true;
#if 1 // Bug 1953079: [pairing] When pairing timeout, click the button to not exit the Bluetooth timeout
			if (UI_BLE_FAILED == uiCurrent) {
                set_current_ui_detail_status(UI_NONE);
                while (0 != get_ui_display_status()) {
                    vTaskDelay(10);
                }
            }
#endif
		}
	}
// 处理多击事件
	SysSubStatus_u subSysStatus = get_subSystem_status();
	switch(sysStatus)
	{
		case IDLE:
		case BT_SERVICE_CHAR_COMM:
		case CHIPPING_MODE_EXITTING:
		case CHARGING:
		{
			if (get_lock_mode())		// 锁定状态
			{
				if (uKeyPressCnt == 3)	// 解锁
				{
					if (subSysStatus == SUBSTATUS_IDLE)
					{
						uint8_t data[1];
						data[0] = 0;
						bt_adapter_device_lock_set(data, 1);
						bDlyDisplayEOL = false;
					}
				}
				else if (uKeyPressCnt)	// 显示锁状态
				{
//					set_system_external_even(EXT_EVENT_LOCK);
/*					if (uiCurrent == UI_EOL)
					{
						g_eol_delay_handle_bits |= (1 << B_EOL_LOCK);
					}
					else */ if (uiCurrent < UI_HEATTING || uiCurrent == UI_ERR_BAT_LOW)
					{
						if (uiCurrent == UI_CHARGE_LEVEL || uiCurrent == UI_LEVEL)
						{
							bDispUILock = true;
							bDlyDisplayEOL = false;
						}
						else if (uiCurrent == UI_NONE)		// 若显示其它蓝牙状态，则不显示
						{
							if ((get_battery_eol_status()))
							{
								bDispUILock = true;
								bDlyDisplayEOL = false;

								if (get_subSystem_status() == 0)		// 显示Lock之前，先显示EOL
								{
									bDlyDisplayEOL = true;
									set_current_ui_detail_status(UI_EOL);	
								}
							}
							else
							{
								set_current_ui_detail_status(UI_BLE_LOCK);
								bDispUILock = false;
								bDlyDisplayEOL = false;
							}
						}
					}
				}
			}
			else
			{
				if (uKeyPressCnt == 3)	// 加锁
				{
					if (subSysStatus == SUBSTATUS_IDLE)
					{
						uint8_t data[1];
						data[0] = 1;
						bt_adapter_device_lock_set(data, 1);							
						bDispUILock = false;
						bDlyDisplayEOL = false;
						sm_log(SM_LOG_INFO, "Dev Lock.\r\n");	// debug
					}
				}
				else if (uKeyPressCnt == 5)	// 配对
				{
                    
                    if (true == BleCongested_Read()) { // 连按五次 并且蓝牙出现问题，系统复位
                        flash_data_save_change(0);      // 带时间戳
                        NVIC_SystemReset();//复位系统
                    }

					if (subSysStatus == SUBSTATUS_PARING || subSysStatus == SUBSTATUS_PARING_QRCODE)
					{
// 改为长按2秒取消						set_system_external_even(EXT_EVENT_PARING_CANCLE);
					}
					else if (subSysStatus == SUBSTATUS_IDLE)
					{
                        if (app_server_context.bt_conn_id == 0) //没有连接，才进入配对流程
                        {
#ifdef DEF_BLE_EN
                            set_system_external_even(EXT_EVENT_PARING_QRCODE);	// 配对二维码 
#endif
                        }
                        else
                        {
                            /**
                             * if connected, dispaly pair success*/
                            set_system_external_even(EXT_EVENT_PARI_OK);
                        }
						bDispUILock = false;
						bDlyDisplayEOL = false;
					}
				}
//				else if (uKeyPressCnt == 2 && (get_subSystem_status() == SUBSTATUS_PARING_QRCODE))
//				{
//					set_system_external_even(EXT_EVENT_PARING);				// 跳过二维码		20241129 取消按2次跳过
//				}

				uKeyPressCnt = 0;		// 处理完，清除
			}
		}

		default:						// 其它状态不响应按键多击，直接清零
		{
			if (uKeyPressCnt)
			{
				uKeyPressCnt = 0;
			}
		break;
		}
	}

	if (bUpdate)
	{
		bUpdate = false;
		if (bDlyDisplayEOL)
		{
			bDlyDisplayEOL = false;
			if (get_subSystem_status() == 0)
			{
				log_err_code(FLT_DE_END_OF_LIFE);
				add_error_even_log(FLT_DE_END_OF_LIFE);	// Bug 1928243 
				motor_set2(HAPTIC_ERR);
				set_current_ui_detail_status(UI_EOL);
			}
		}
	}
	
	subSystem_idle_interaction_logic_proc(get_system_external_even(), pUiInfo);		// 必须更新事件

	uiCurrent = get_current_ui_detail_status();	// 更新UI状态

	if (bDispUICharg)
	{
		if (true == get_usb_status())
		{
			if (uiCurrent < UI_CHARGE_LEVEL)
			{
				set_current_ui_detail_status(UI_CHARGE_LEVEL);// 如果是在加热中切换到充电需要UI显示完再进行充电，不然UI和振动会不同步,需要事件延迟处理
				bDispUICharg = false;
			}
		}
		else
		{
			bDispUICharg = false;
		}
	}

	if (bDispUILock)
	{
		if (false == get_lock_mode() || uiCurrent > UI_CHARGE_LEVEL)	// 非Lock或非显示电量
		{
			if (uiCurrent != UI_EOL)
			{
				bDispUILock = false;
			}
		}
//		else if (uiCurrent == UI_NONE)
		else if (uiCurrent == UI_NONE)
		{
			set_current_ui_detail_status(UI_BLE_LOCK);
			bDispUILock = false;
		}
	}
	
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    switch(sysStatus) {
        case IDLE:
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
        case BT_SERVICE_CHAR_COMM:
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            system_idle_interaction_logic_proc(etEven, pUiInfo);
            break;
        case SLEEP:
            break;
        case HEATTING_TEST:
            break; 
        case HEATTING_STANDARD:
            system_heating_interaction_logic_proc(etEven, pUiInfo);
            break;
        case HEATTING_BOOST:
            system_heating_interaction_logic_proc(etEven, pUiInfo);
            break;
        case HEATTING_CLEAN:
            system_heating_interaction_logic_proc(etEven, pUiInfo);
            break;
        case HEAT_MODE_TEST_VOLTAGE:
            break;
        case HEAT_MODE_TEST_POWER:
            break;
        case HEAT_MODE_TEST_TEMP:
            break;
        case CHARGING:
            system_charging_interaction_logic_proc(etEven, pUiInfo);
            break;
//        case CHIPPING_MODE_EXITTING:
//            set_system_status(IDLE);
//            set_current_ui_detail_status(UI_HEATTING);
//            break;
        default:
            break;
    } 
}

/**
  * @brief  系统错误恢复判断处理-充电状态
  * @param  None
  * @return None
  * @note   
  */
void system_error_recover_on_chargeing_proc(void)
{
    ExternalEven_e etEven = get_system_external_even();
    SysStatus_u sysStatus = get_system_status();
    UiInfo_t *pUiInfo = get_error_high_pro_even();
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    int16_t* ini_p = get_ini_val_info_handle();

    // 遍历系统需要恢复的错误，针对每个错误与读到的参数对比，并做滤波（1秒）处理
     /************************Pre-Session Scan*****************************************/
    if (FLT_DE_BAT_HOT_PRE_SES == find_error_even(FLT_DE_BAT_HOT_PRE_SES)) { // one short err 启动加热时电芯过温EE 51℃
        delete_error_even(FLT_DE_BAT_HOT_PRE_SES);
    }
    if (FLT_DE_BAT_LOW == find_error_even(FLT_DE_BAT_LOW)) { // one short err 启动加热时电量低 先用电压，>3.65V则错误清除，后续用soc 电量%4
        if (p_tMontorDataInfo->bat.remap_soc >= ini_p[WAR_BAT_LOW_SOC]) {
            delete_error_even(FLT_DE_BAT_LOW);
        }
    }   
    
    if (FLT_DE_TC_ZONE_HOT_PRE_SES == find_error_even(FLT_DE_TC_ZONE_HOT_PRE_SES)) { // one short err 启动加热时发热体温度高 > 200
        delete_error_even(FLT_DE_TC_ZONE_HOT_PRE_SES);
    }

    /***********************During Session******************************************/
    if (FLT_DE_BAT_EMPTY == find_error_even(FLT_DE_BAT_EMPTY)) { //one shot error 加热过程低压≤3V 
        delete_error_even(FLT_DE_BAT_EMPTY);
    }
  //  if (FLT_DE_TC_SPIKE == find_error_even(FLT_DE_TC_SPIKE)) { // one shot error 温升过快＞20℃@16ms (TBD) 断针 reset recover
  //      delete_error_even(FLT_DE_TC_SPIKE);
  //  }
    if (FLT_DE_THERMOCOUPLE_ERR == find_error_even(FLT_DE_THERMOCOUPLE_ERR)) { //one shot error 加热温控异常 (TBD)根据发热体适配 &热偶开路/短路EE Target detla temperature 100℃
        delete_error_even(FLT_DE_THERMOCOUPLE_ERR);
    }
    if (FLT_DE_BAT_I_SENSE_DAMAGE == find_error_even(FLT_DE_BAT_I_SENSE_DAMAGE)) { // 加热无电流 one shot error
        delete_error_even(FLT_DE_BAT_I_SENSE_DAMAGE);
    }


    /**********************Anytime***************************************/
    if (FLT_DE_TC_ZONE_HOT == find_error_even(FLT_DE_TC_ZONE_HOT)) { // 发热体过温 EE 500℃（TBD）Wait Recover 停止加热和充电，什么时候恢复充电？
        if (p_tMontorDataInfo->det.heat_K_temp < (float)ini_p[FLT_TC_ZONE_HOT]) {
            delete_error_even(FLT_DE_TC_ZONE_HOT);
        }
    }
    if (FLT_DE_BAT_DISCHARGE_CURRENT_OVER == find_error_even(FLT_DE_BAT_DISCHARGE_CURRENT_OVER)) { // 放电过流 EE 7A Reset recover

    }
    if (FLT_DE_BAT_HOT == find_error_even(FLT_DE_BAT_HOT)) { // 电芯过温 EE 58℃， 释放 52℃ Wait Recover
#if 1 // 更新session对应的配置和比较参数
        if (p_tMontorDataInfo->bat.temperature <= ini_p[STEP1_FLT_BAT_HOT_CLEAR + get_iniTable_session_index(p_tMontorDataInfo->session)]) {
            delete_error_even(FLT_DE_BAT_HOT);
        }
#else
		if (p_tMontorDataInfo->bat.temperature <= ini_p[FLT_BAT_HOT_CLEAR]) {
            delete_error_even(FLT_DE_BAT_HOT);
        }
#endif
    }
    if (FLT_DE_BAT_COLD == find_error_even(FLT_DE_BAT_COLD)) { // 电芯低温 EE -8℃, 释放-6℃Wait Recover
        if (p_tMontorDataInfo->bat.temperature >= ini_p[FLT_BAT_COLD_CLEAR]) {
            delete_error_even(FLT_DE_BAT_COLD);
        }
    }
    if (FLT_DE_END_OF_LIFE == find_error_even(FLT_DE_END_OF_LIFE)) { // 电芯EOL EE 11000 ,USERFLOW 10000 The 6s EOL UI will be prompted before each battery display

    }
    if (FLT_DE_CO_JUNC_HOT == find_error_even(FLT_DE_CO_JUNC_HOT)) { // PCB过温 EE 80, 释放60 Wait Recover
        if (p_tMontorDataInfo->det.heat_K_cood_temp <= ini_p[FLT_CO_JUNC_HOT_CLEAR]) {
            delete_error_even(FLT_DE_CO_JUNC_HOT);
        }
    }
    if (FLT_DE_USB_HOT == find_error_even(FLT_DE_USB_HOT)) { // USB端口过热 EE 70℃，释放 55℃
        if (p_tMontorDataInfo->det.usb_port_temp <= ini_p[FLT_USB_HOT_TEMP_CLEAR] || false == get_usb_status()) {
            delete_error_even(FLT_DE_USB_HOT);
        }
    }
}


/**
  * @brief  系统错误恢复判断处理
  * @param  None
  * @return None
  * @note   在加热或者充电状态下，触发错误并显示UI后系统处于空闲状态，在系统空闲状态下，查找系统里的每个错误，根据系统参数，判断是否需要清除该错误
  */
void system_error_recover_proc(void)
{
    ExternalEven_e etEven = get_system_external_even();
    SysStatus_u sysStatus = get_system_status();
    UiInfo_t *pUiInfo = get_error_high_pro_even();
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    int16_t* ini_p = get_ini_val_info_handle();
    static uint32_t recoverChargeFilterCnt = 0;
    static uint32_t filterUsbCnt = 0;
    if (sysStatus == CHARGING || sysStatus == HEATTING_STANDARD || sysStatus == HEATTING_BOOST || sysStatus == HEATTING_CLEAN) {
        filterUsbCnt = 0;
        return;
    }
    
    // 遍历系统需要恢复的错误，针对每个错误与读到的参数对比，并做滤波（1秒）处理
     /************************Pre-Session Scan*****************************************/
    if (FLT_DE_BAT_HOT_PRE_SES == find_error_even(FLT_DE_BAT_HOT_PRE_SES)) { // one short err 启动加热时电芯过温EE 51℃
        delete_error_even(FLT_DE_BAT_HOT_PRE_SES);
    }
    if (FLT_DE_BAT_LOW == find_error_even(FLT_DE_BAT_LOW)) { // one short err 启动加热时电量低 先用电压，>3.65V则错误清除，后续用soc 电量%4
        if (p_tMontorDataInfo->bat.remap_soc >= ini_p[WAR_BAT_LOW_SOC]) {
            delete_error_even(FLT_DE_BAT_LOW);
        }
    }
    if (FLT_DE_TC_ZONE_HOT_PRE_SES == find_error_even(FLT_DE_TC_ZONE_HOT_PRE_SES)) { // one short err 启动加热时发热体温度高 > 200
        delete_error_even(FLT_DE_TC_ZONE_HOT_PRE_SES);
    }
    if (FLT_DE_TARGET_TEMP_DIFF == find_error_even(FLT_DE_TARGET_TEMP_DIFF)) { // one short err 测得温度 大于 目标温度50
        delete_error_even(FLT_DE_TARGET_TEMP_DIFF);
    }

    /***********************During Session******************************************/
    if (FLT_DE_BAT_EMPTY == find_error_even(FLT_DE_BAT_EMPTY)) { //one shot error 加热过程低压≤3V 
        delete_error_even(FLT_DE_BAT_EMPTY);
    }
  //  if (FLT_DE_TC_SPIKE == find_error_even(FLT_DE_TC_SPIKE)) { // one shot error 温升过快＞20℃@16ms (TBD)
   //     delete_error_even(FLT_DE_TC_SPIKE);
  //  }
    if (FLT_DE_THERMOCOUPLE_ERR == find_error_even(FLT_DE_THERMOCOUPLE_ERR)) { //one shot error 加热温控异常 (TBD)根据发热体适配 &热偶开路/短路EE Target detla temperature 100℃
        delete_error_even(FLT_DE_THERMOCOUPLE_ERR);
    }
    if (FLT_DE_BAT_I_SENSE_DAMAGE == find_error_even(FLT_DE_BAT_I_SENSE_DAMAGE)) { // 加热无电流 one shot error
        delete_error_even(FLT_DE_BAT_I_SENSE_DAMAGE);
    }
    if (FLT_DE_HARDWARE_ERR == find_error_even(FLT_DE_HARDWARE_ERR)) { // 加热无电流 one shot error
        delete_error_even(FLT_DE_HARDWARE_ERR);
    }
    if (FLT_DE_POWER_OVERLOAT == find_error_even(FLT_DE_POWER_OVERLOAT)) { // 加热功率过载 one shot error
        delete_error_even(FLT_DE_POWER_OVERLOAT);
    }

    /**********************During charging***************************************/
    if (FLT_DE_CIC_OUTPUT_VOLTAGE == find_error_even(FLT_DE_CIC_OUTPUT_VOLTAGE)) { // Non recover Rebrush version 充电电芯过压Leve0 ＞4.63V 不可恢复, 需要写到掉电保存位置, 重新刷固件清除

    }
    if (FLT_DE_BAT_CHARGE_CURRENT_OVER == find_error_even(FLT_DE_BAT_CHARGE_CURRENT_OVER)) { // Reset recover ,保存到RAM不能清除 充电过流＞4A

    }
    if (FLT_DE_BAT_HOT_CHARGE == find_error_even(FLT_DE_BAT_HOT_CHARGE)) { // 充电-电芯过温 EE 55℃ ，释放50℃ Wait Recover ，移除USB的时候删除该错误
#if 1 // 更新session对应的配置和比较参数
        if (p_tMontorDataInfo->bat.temperature <= ini_p[STEP1_FLT_BAT_HOT_CHARGE_CLEAR + get_iniTable_session_index(p_tMontorDataInfo->session)] || false == get_usb_status()) {
            delete_error_even(FLT_DE_BAT_HOT_CHARGE);
        }
#else
        if (p_tMontorDataInfo->bat.temperature <= ini_p[FLT_BAT_HOT_CHARGE_CLEAR] || false == get_usb_status()) {
            delete_error_even(FLT_DE_BAT_HOT_CHARGE);
        }
#endif	
    }
    if (FLT_DE_BAT_COLD_CHARGE == find_error_even(FLT_DE_BAT_COLD_CHARGE)) { // 充电-电芯低温 EE 2℃， 释放 4℃或 移除USB的时候删除该错误 可以加热
        if (p_tMontorDataInfo->bat.temperature >= ini_p[FLT_BAT_COLD_CHARGE_CLEAR] || false == get_usb_status()) {
            delete_error_even(FLT_DE_BAT_COLD_CHARGE);
        }
    }
    if (FLT_DE_BAT_VOLTAGE_OVER == find_error_even(FLT_DE_BAT_VOLTAGE_OVER)) { // 充电-电芯过压Leve1≥4.45V 满充设置4.4V  <4.35 或USB移除 Wait Recover 恢复level1
        if (p_tMontorDataInfo->bat.voltage <= ini_p[FLT_BAT_VOLTAGE_OVER_CLEAR] || false == get_usb_status()) {
            delete_error_even(FLT_DE_BAT_VOLTAGE_OVER);
        }
    }
    if (FLT_DE_CHG_VBUS == find_error_even(FLT_DE_CHG_VBUS)) { // 充电-Vbus电压高 Wait Recover Unplugg the charger or VbusVolt < 11000mv, 可以加热
        if ((p_tMontorDataInfo->chg.bus_volt < ini_p[WAR_CHG_VBUS]) || false == get_usb_status()) {
            delete_error_even(FLT_DE_CHG_VBUS);
            if (p_tMontorDataInfo->chg.bus_volt < ini_p[WAR_CHG_VBUS] && true == get_usb_status()) { // 恢复充电，可能是USB拔出有滤波，但vbus电压先降，恢复充电需要延时
                recoverChargeFilterCnt = 550; // 1100 /2 task_system_service一个循环2ms
            }
        }
    }
    if (FLT_DE_CIC_CHARGE_TIMEOUT == find_error_even(FLT_DE_CIC_CHARGE_TIMEOUT)) { // 充电超时＞6H Unplugg the charger Wait Recover
        if (false == get_usb_status() && UI_HEATTING != get_current_ui_detail_status()) { // usb在加热的时候会设置为false 不可信， 加热结束后，usb重新检测，需要滤波
            filterUsbCnt++;
            if (filterUsbCnt > 250) {
                filterUsbCnt = 0;
                delete_error_even(FLT_DE_CIC_CHARGE_TIMEOUT);
            }
        } else {
            filterUsbCnt = 0;
        }
    }
    
    /**********************when the charger is plugged in***************************************/
    if (FLT_DE_BAT_DAMAGE == find_error_even(FLT_DE_BAT_DAMAGE)) { // 电芯损坏不可以恢复电压0.9V  Non Recover Rebrush version

    }

    /**********************device awake***************************************/
    if (FLT_DE_CIC_CONFIG_ERROR == find_error_even(FLT_DE_CIC_CONFIG_ERROR)) { // 充电芯片iic通信错误  Reset recover

    }

    /**********************Anytime***************************************/
    if (FLT_DE_TC_ZONE_HOT == find_error_even(FLT_DE_TC_ZONE_HOT)) { // 发热体过温 EE 500℃（TBD）Wait Recover 停止加热和充电，什么时候恢复充电？
        if (p_tMontorDataInfo->det.heat_K_temp < (float)ini_p[FLT_TC_ZONE_HOT]) {
            delete_error_even(FLT_DE_TC_ZONE_HOT);
        }
    }
    if (FLT_DE_BAT_DISCHARGE_CURRENT_OVER == find_error_even(FLT_DE_BAT_DISCHARGE_CURRENT_OVER)) { // 放电过流 EE 7A Reset recover

    }
    if (FLT_DE_BAT_HOT == find_error_even(FLT_DE_BAT_HOT)) { // 电芯过温 EE 58℃， 释放 52℃ Wait Recover
#if 1 // 更新session对应的配置和比较参数
        if (p_tMontorDataInfo->bat.temperature <= ini_p[STEP1_FLT_BAT_HOT_CLEAR + get_iniTable_session_index(p_tMontorDataInfo->session)]) {
            delete_error_even(FLT_DE_BAT_HOT);
        }
#else
        if (p_tMontorDataInfo->bat.temperature <= ini_p[FLT_BAT_HOT_CLEAR]) {
            delete_error_even(FLT_DE_BAT_HOT);
        }
#endif	
    }
    if (FLT_DE_BAT_COLD == find_error_even(FLT_DE_BAT_COLD)) { // 电芯低温 EE -8℃, 释放-6℃Wait Recover
        if (p_tMontorDataInfo->bat.temperature >= ini_p[FLT_BAT_COLD_CLEAR]) {
            delete_error_even(FLT_DE_BAT_COLD);
        }
    }
    if (FLT_DE_END_OF_LIFE == find_error_even(FLT_DE_END_OF_LIFE)) { // 电芯EOL EE 11000 ,USERFLOW 10000 The 6s EOL UI will be prompted before each battery display

    }
    if (FLT_DE_CO_JUNC_HOT == find_error_even(FLT_DE_CO_JUNC_HOT)) { // PCB过温 EE 80, 释放60 Wait Recover
        if (p_tMontorDataInfo->det.heat_K_cood_temp <= ini_p[FLT_CO_JUNC_HOT_CLEAR]) {
            delete_error_even(FLT_DE_CO_JUNC_HOT);
        }
    }
    if (FLT_DE_USB_HOT == find_error_even(FLT_DE_USB_HOT)) { // USB端口过热 EE 70℃，释放 55℃
        if (p_tMontorDataInfo->det.usb_port_temp <= ini_p[FLT_USB_HOT_TEMP_CLEAR] || false == get_usb_status()) {
            delete_error_even(FLT_DE_USB_HOT);
        }
    }
//    int usb = get_usb_status();
//    int err = charging_only_check_error();
//    int chargeFull = get_charge_full_status();
//    sm_log(SM_LOG_INFO, "system_error_recover_proc! recover charge! %d, %d, %d, %d\r\n", usb, err, chargeFull, p_tMontorDataInfo->bat.voltage);
    // USB如果是接入状态，并且没有对应错误，并且没有充满(判断充满条件：充满标志 充电IC截止电压> 4.4V,截止电流150ma)，则需要恢复充电
    // 考虑充电中切换到加热，usb状态已为false，因此不会加热结束后马上恢复充电,
    if (recoverChargeFilterCnt != 0) {
        recoverChargeFilterCnt--;
    }
    if (true == get_usb_status() && true == charging_only_check_error() && recoverChargeFilterCnt == 0 && (UI_HEATTING != get_current_ui_detail_status()) && (UI_CLEAN_PRO != get_current_ui_detail_status()) && (false == get_charge_full_status())) { // USB接入的时候，要不要休眠, p_tMontorDataInfo->bat.voltage
    	set_charge_full_status(false);
        set_system_external_even(EXT_EVENT_USB_PLUG);
        sm_log(SM_LOG_INFO, "sys recover chg!\r\n");
    }
}


