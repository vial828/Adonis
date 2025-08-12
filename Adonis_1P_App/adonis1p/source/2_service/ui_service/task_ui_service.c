/**
  ******************************************************************************
  * @file    task_ui_service.c
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

#include "task_ui_service.h"
#include "platform_io.h"
#include "data_base_info.h"
#include "sm_log.h"
#include <stdlib.h>
#include "ui_heat.h"
#include "system_status.h"
//#include "err_code.h"
#include "ui_battery.h"
#include "ui_bluetooth.h"
#include "ui_clean.h"
#include "ui_err_critical.h"
#include "ui_err_reboot.h"
#include "ui_err.h"
#include "ui_shipping_mode.h"
#include "ui_err_wait.h"
#include "system_interaction_logic.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "app_bt_char_adapter.h"
#include "event_data_record.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#define SESSION_AVERAGE_TIME 305

/**
 * @brief FreeRTOS variable to store handle of task created to update and send dummy
   values of ui
 */
TaskHandle_t task_ui_handle;
UiTaskDetailStatus_u g_eUitaskDetailStatus = UI_NONE;

/**
  * @brief  返回当前正在显示的任务
  * @param  None
  * @return 返回指针
  * @note   None
  */
UiTaskDetailStatus_u get_current_ui_detail_status(void)
{
    return g_eUitaskDetailStatus;
}

/**
  * @brief  设置当前显示的任务
  * @param  None
  * @return 返回指针
  * @note   None
  */
void set_current_ui_detail_status(UiTaskDetailStatus_u status)
{
     g_eUitaskDetailStatus = status;
     if (g_eUitaskDetailStatus == UI_NONE) {
        sm_log(SM_LOG_INFO, "set_current_ui_detail_status 0!\r\n");
        return;
     }
     TaskHandle_t *temp_handle;
     temp_handle = get_task_ui_handle();
     xTaskNotifyGive(*temp_handle);
     sm_log(SM_LOG_INFO, "set_current_ui_detail_status %d!\r\n", g_eUitaskDetailStatus);
}

/**
  * @brief  获取任务指针
  * @param  None
  * @return 返回指针
  * @note   None
  */
TaskHandle_t* get_task_ui_handle(void)
{
    return &task_ui_handle;
}

extern int driver_amoled_reinit(void);
/**
  * @brief  ui服务任务处理函数
  * @param  pvParam:    任务入参
  * @return None
  * @note   None
  */
void task_ui_service(void *pvParam)
{
    static  SysStatus_u sysStatus;
    static uint8_t status = 0;
    static UiTaskDetailStatus_u eUitaskDetailStatus = UI_NONE;
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		uint8_t event_data[1];
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    while(true)
    {
//        vTaskDelay(1);
        sys_task_security_ms_delay(1, TASK_UI);
        switch (status) {
            case 0: // 等待显示UI的通知
                clr_task_ui_heartbeat_ticks();
                set_ui_display_status(0); // 无显示任务

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				event_data[0] = EVENT_CODE_LCD_OFF;
				event_record_generate(EVENT_CODE_LED_BEHAVIOUR, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待任务通知
                set_task_ui_heartbeat_ticks();
                set_ui_display_status(1); // 有显示任务
                // 判断需要显示的大类：1加热，2充电，3显示电量, 4显示错误码信息， 5蓝牙相关
                sysStatus = get_system_status();
               // eUitaskDetailStatus = get_current_ui_detail_status();
                UiTaskDetailStatus_u uiStatus = get_current_ui_detail_status();
               // set_system_ui_err_status();
                #if 0
                if (uiInfo->uiStatus == UI_LEVEL) {
                    delete_led_even(uiInfo->even);
                    if (CHARGING == sysStatus) {
                        status = 2;
                        set_ui_display_status(2); // 充电显示
                    } else {
                        status = 3; //
                        set_ui_display_status(3); // 电量显示
                    }
                } else if (uiInfo->uiStatus == UI_HEATTING) {
                    delete_led_even(uiInfo->even);
                    status = 1;
                    set_ui_display_status(1); // 加热显示
                } else if (uiInfo->uiStatus == UI_SHIPPING) {
                    delete_led_even(uiInfo->even);
                    status = 5; //
                } else if (uiInfo->uiStatus == UI_CLEAN_PRO) {
                    delete_led_even(uiInfo->even);
                    status = 4; //
                    set_ui_display_status(5); // 显示清洁过程
                }else if (uiInfo->uiStatus == UI_ERR_WAIT) {
                    delete_led_even(uiInfo->even);
                }else if (uiInfo->uiStatus == UI_ERR_REBOOT_TIP) {
                    status = 6;
                }else if (uiInfo->uiStatus == UI_ERR_REBOOTING) {
                    status = 7;
                }else if (uiInfo->uiStatus == UI_ERR_CRITICAL) {
                    status = 8;
                } else {
                    status = 0;
                }
                #endif
                if (uiStatus == UI_LEVEL) {
                    status = 3; //
                    set_ui_display_status(3); // 电量显示
                } else if (uiStatus == UI_CHARGE_LEVEL) {
                    status = 2;
                    set_ui_display_status(2); // 充电显示
                } else if (uiStatus == UI_HEATTING) {
                    status = 1;
                    set_ui_display_status(1); // 加热显示
                } else if (uiStatus == UI_SHIPPING) {
                    status = 5; //
                } else if (uiStatus == UI_CLEAN_PRO) {
                    status = 4; //
                    //set_ui_display_status(5); // 显示清洁过程
                }else if (uiStatus == UI_ERR_REBOOT_TIP) {
                    status = 6;
                }else if (uiStatus == UI_ERR_REBOOTING) {
                    status = 7;
                }else if (uiStatus == UI_ERR_CRITICAL) {
                    status = 8;
        		}  else if (uiStatus == UI_ERR_GER_WAIT) {
                    status = 9;
                }  else if (uiStatus == UI_BOOT_TIP) {
                    status = 10;
                }
                /**************************************/
                else if (uiStatus == UI_ERR_COLD_WAIT) {
                    status = 11;
                } else if (uiStatus == UI_ERR_HOT_WAIT) {
                    status = 12;
                }  else if (uiStatus == UI_ERR_USB_HOT) {
                	status = 13;
                }  else if (uiStatus == UI_ERR_USB_OV) {
                    status = 14;
                }  else if (uiStatus == UI_ERR_BAT_OV) {
                    status = 15;
                } else if (uiStatus == UI_ERR_BAT_LOW) {
                    status = 16;
                }  else if (uiStatus == UI_EOL) {
                    status = 17;
                } else if (uiStatus == UI_ERR_CHARGE_TIMEOUT) {
                    status = 18;
               } else if (uiStatus == UI_BLE_FAILED) {
                	status = 19;
                } else if (uiStatus == UI_BLE_PAIRED) {
                	status = 20;
                } else if (uiStatus == UI_BLE_PAIRING) {
                	status = 21;
                } else if (uiStatus == UI_BLE_FIND_ME) {
                	status = 22;
                } else if (uiStatus == UI_LOADING) {
                	status = 23;
                } else if (uiStatus == UI_BLE_LOCK) {
                	status = 24;
                } else if (uiStatus == UI_BLE_UNLOCK) {
                    status = 25;
                } else if (uiStatus == UI_BLE_DOWNLOAD) {
                	status = 26;
                } else if (uiStatus == UI_SET_PROFILE) {
                	status = 27;
                } else if (uiStatus == UI_SET_PROFILE_BOOST) {
                    status = 28;
                } else if (uiStatus == UI_CUSTOMIZE) {
                    status = 29;
                } else {
                    status = 0;
                }
                sm_log(SM_LOG_INFO, "task_ui_service run status %d!\r\n", status);
				
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				event_data[0] = EVENT_CODE_LCD_ON; 
				event_record_generate(EVENT_CODE_LED_BEHAVIOUR, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

                if (status > 0 && status <= 29) {
                 //   ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
                 //   amoledDev->init();
                    driver_amoled_reinit();
                }
                break;

            case 1:// 加热显示
                if (sysStatus != HEATTING_STANDARD && sysStatus != HEATTING_BOOST) {
                    if (UI_HEATTING == get_current_ui_detail_status()) {
                        set_current_ui_detail_status(UI_NONE);
                    }
                    status = 0;
                    break;
                }
                if (UI_HEATTING < get_current_ui_detail_status()) { // 加热前发生
                    status = 0;
                    break;
                }
				/* uimage display photo debug */
				if (MyName_uImage_Intro_isExisted()) { // hi page is existed
					if (MyName_uImage_Greeting_isExisted()) { // name page is existed
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_HI, 1000, 0); // 1s
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_NAME, 1000, 1); // 1s
					} else {
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_HI, 2000, 1); // 2s
					}
				} else {
					if (MyName_uImage_Greeting_isExisted()) { // name page is existed
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_HI_DEFAULT, 1000, 0); // 1s
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_NAME, 1000, 1); // 1s
					} else if (MyName_uImage_Outro_isExisted()) { // bye page is existed
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_HI_DEFAULT, 2000, 1); // 2s
					} else {
						amoled_display_heat_hi(); // 2s
					}
				}
                if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
                    status = 0;
                    break;
                }
				
               	amoled_display_hi_to_preheat(sysStatus);
                if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
                    status = 0;
                    break;
                }
				
                amoled_display_preheat(sysStatus);
                if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
                    status = 0;
                    break;
                }

                if (sysStatus == get_system_status()) { // 如果状态不等，则跳过抽吸
                    amoled_display_preheat_to_consume(sysStatus);
                    if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
                        status = 0;
                        break;
                    }
                    amoled_display_heat_consume_all(sysStatus); // 抽吸过程发生
                    if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
                        status = 0;
                        break;
                    }
                    amoled_display_heat_end(sysStatus);
                    if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
                        status = 0;
                        break;
                    }
                }
                if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
                    status = 0;
                    break;
                }
				
                amoled_display_heat_dispose();
                if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
                    status = 0;
                    break;
                }
#ifdef DEF_DRY_CLEAN_EN // 启用干烧清洁功能,不使用干烧清洁功能时，把此宏注释掉
				/* 20支烟抽吸后，提示自清洁动画*/
				// if (1) { // 预留 自清洁使能并且抽吸支数≥20支，SOC≥15% (仅用于调试)
				if (get_clean_prompt() && (get_sessions_self_clean_prompt() >= 20) && (p_tMontorDataInfo->bat.soc >= 15)){
					//0:无按键操作,播放至bye; 1:按boost按键启动自清洁,直接退出; 2:按base按键取消自清洁,播放bye;
					if (1 == amoled_display_clean_hint()) { // 1:按boost按键启动自清洁
						if (UI_HEATTING == get_current_ui_detail_status()) {
							set_current_ui_detail_status(UI_NONE);
						}
						set_system_external_even(EXT_EVENT_KEY_CLEAN_HEAT_PRESS);
						status = 0;
						break; // 直接退出
					}
				} else {
					amoled_display_dispose_to_bye(); // 无需自清洁提示，播放过度动画
				}
#else
				amoled_display_dispose_to_bye(); // 播放过度动画
#endif
                if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
                    status = 0;
                    break;
                }
				/* uimage display customized page*/
				if (MyName_uImage_Outro_isExisted()) { // bye page is existed
					amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_BYE, 1440, 1); // 1.44s
				} else {
					if (MyName_uImage_Greeting_isExisted() || // name page is existed
						MyName_uImage_Intro_isExisted()) { // hi page is existed
						amoled_display_customize(DISP_HEAT, CUSTOMIZE_UI_BYE_DEFAULT, 1440, 1);
					} else {
						amoled_display_bye(); // 1.44s
					}
				}

                if (UI_HEATTING == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 2:// 充电显示
                amoled_display_battery_charge_level();
                if (UI_CHARGE_LEVEL == get_current_ui_detail_status()) {//
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 3:// 空闲电量显示
               // amoled_display_idle_battery_level();
                if (true == get_battery_eol_status()) {
                    amoled_display_battery_eol_level(); // 显示电量
                } else {
                    amoled_display_battery_level(0);
                }
                if (UI_LEVEL == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 4:// 显示清洁过程
#ifdef DEF_DRY_CLEAN_EN // 启用干烧清洁功能,不使用干烧清洁功能时，把此宏注释掉
            	amoled_display_cleaning();
#endif
                if (UI_CLEAN_PRO == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 5:// 显示退出船运模式
                amoled_display_exit_shipping_mode();
                if (UI_SHIPPING == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                // 读取电量计
                for (uint8_t i=0; i<7; i++) { // 延时1.4s，等待系统SOC更新为非零 (总时间约3.3s，多10%余量)
                	sys_task_security_ms_delay(200, TASK_UI);
                	if ((p_tMontorDataInfo->bat.soc > 0) || (p_tMontorDataInfo->bat.voltage <= 3300)) {
                		//sm_log(SM_LOG_INFO, "fule gauge soc:%d,i:%d\r\n",p_tMontorDataInfo->bat.soc, i);
                		break;
                	}
                }
                set_shipping_mode(0);
                set_system_status(IDLE);
                status = 0;
                break;
            case 6://
                amoled_display_err_reboot_tip();
                if (UI_ERR_REBOOT_TIP == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 7://
                amoled_display_err_reboot_countDown();
                if (UI_ERR_REBOOTING == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 8://
                amoled_display_err_critical();
                if (UI_ERR_CRITICAL == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 9:
            	amoled_display_err_wait(3);
                if (UI_ERR_GER_WAIT == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 10:
                amoled_display_boot_tip();
                // 读取电量计
                for (uint8_t i=0; i<7; i++) { // 延时1.4s，等待系统SOC更新为非零
                	sys_task_security_ms_delay(200, TASK_UI);
                	if ((p_tMontorDataInfo->bat.soc > 0) || (p_tMontorDataInfo->bat.voltage <= 3300)) {
                		//sm_log(SM_LOG_INFO, "fule gauge soc:%d,i:%d\r\n",p_tMontorDataInfo->bat.soc, i);
                		break;
                	}
                }
                if (UI_BOOT_TIP == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                // 增加初始化电量计
                status = 0;
                break;
            case 11: // UI_ERR_COLD_WAIT
            	amoled_display_err_wait(0);
                if (UI_ERR_COLD_WAIT == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 12: // UI_ERR_HOT_WAIT
            	amoled_display_err_wait(1);
                if (UI_ERR_HOT_WAIT == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 13: // UI_ERR_USB_HOT
            	amoled_display_chg_err(0); // 0:usb过热
                if (UI_ERR_USB_HOT == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
                status = 0;
                break;
            case 14: // UI_ERR_USB_OV
				amoled_display_chg_err(1); // 1:usb过压
                if (UI_ERR_USB_OV == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;
			case 15: // UI_ERR_BAT_OV
				amoled_display_chg_err(2); // 2:bat过压复位
                if (UI_ERR_BAT_OV == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;
			case 16: // UI_ERR_BAT_LOW
				amoled_display_battery_low_level(); // 
                if (UI_ERR_BAT_LOW == get_current_ui_detail_status()) {
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;
			case 17: // UI_EOL
				amoled_display_battery_eol(sysStatus); //
				sysStatus = get_system_status();

				if (ble_interaction_delay_handle_for_eol())		// 若BLE有事件需要延迟处理，则优先处理
				{
					status = 0;
					break;
				}
		
                // 显示完二维码后，读取记录状态，如果电芯还可用，则显示电量和可抽吸烟支数量
                if (CHARGING == sysStatus) {
                    if (UI_EOL == get_current_ui_detail_status()) { //
                        set_current_ui_detail_status(UI_CHARGE_LEVEL);
                    }
                    amoled_display_battery_charge_level(); //充电
                } else {
                    if (UI_EOL == get_current_ui_detail_status()) { //
                        set_current_ui_detail_status(UI_LEVEL);
                    }
                    if (IDLE == sysStatus) { // 空闲状态 才显示电量
                    	amoled_display_battery_eol_level(); // 显示电量
                    }
                }
                if (UI_CHARGE_LEVEL == get_current_ui_detail_status() || UI_LEVEL == get_current_ui_detail_status()) { // 不能加UI_EOL 防止在显示电量的时候，由按按键了，导致只振动不执行新的UI_EOL
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;
			case 18: // UI_ERR_CHARGE_TIMEOUT
				amoled_display_battery_level(2); //
                if (UI_ERR_CHARGE_TIMEOUT == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;

			case 19:
				amoled_disp_ble_failed();
                if (UI_BLE_FAILED == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;

			case 20:
				amoled_disp_ble_paired();
                if (UI_BLE_PAIRED == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;

			case 21:
				amoled_disp_ble_pairing();
                if (UI_BLE_PAIRING == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;

			case 22:
				amoled_disp_ble_findme();
                if (UI_BLE_FIND_ME == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;

			case 23:
				amoled_disp_loading();
                if (UI_LOADING == get_current_ui_detail_status()) { //
                    set_current_ui_detail_status(UI_NONE);
                }
				status = 0;
				break;

			case 24:
				amoled_disp_ble_lock();
				if (UI_BLE_LOCK == get_current_ui_detail_status()) { //
					set_current_ui_detail_status(UI_NONE);
				}
				status = 0;
				break;

			case 25:
				amoled_disp_ble_unlock();
				if (UI_BLE_UNLOCK == get_current_ui_detail_status()) { //
					set_current_ui_detail_status(UI_NONE);
				}
				status = 0;
				break;

			case 26:
				amoled_disp_ble_download();
				if (UI_BLE_DOWNLOAD == get_current_ui_detail_status()) { //
					set_current_ui_detail_status(UI_NONE);
				}
				status = 0;
				break;

			case 27:
                // Get customized flag and play page
                amoled_display_set_profile(PROFILE_UI_STANDARD); // 如果page 为PROFILE_UI_NONE，函数内部会直接退出
				if (UI_SET_PROFILE == get_current_ui_detail_status()) { //
					set_current_ui_detail_status(UI_NONE);
				}
				status = 0;
				break;

			case 28:
				// Get customized flag and play page
				amoled_display_set_profile(PROFILE_UI_BOOST); // 如果page 为PROFILE_UI_NONE，函数内部会直接退出
				if (UI_SET_PROFILE_BOOST == get_current_ui_detail_status()) { //
					set_current_ui_detail_status(UI_NONE);
				}
				status = 0;
				break;

			case 29: /* ui display personalisation hi and name page */ // 根据USER定义的调整
				//amoled_display_confirmation(); // 先播放√页面，系统要配合震动; V17版User flow取消打钩
                //if (UI_CUSTOMIZE != get_current_ui_detail_status()) {
                //    status = 0;
                //    break;
                //}
                // Get customized flag and play page
                switch (MyName_uImage_isUpdating_Get())
                {
                case CUSTOMIZE_UI_HI:
					amoled_display_customize(DISP_OTA, CUSTOMIZE_UI_HI, 4000, 1); // 4s
                    break;
                case CUSTOMIZE_UI_NAME:
					amoled_display_customize(DISP_OTA, CUSTOMIZE_UI_NAME, 4000, 1); // 4s
                    break;
                case CUSTOMIZE_UI_BYE:
					amoled_display_customize(DISP_OTA, CUSTOMIZE_UI_BYE, 4000, 1); // 4s
                    break;

                default:
                    break;
                }
				if (UI_CUSTOMIZE == get_current_ui_detail_status()) { //
					set_current_ui_detail_status(UI_NONE);
				}
				status = 0;
				break;

            default:    break;
        }
    }
}

