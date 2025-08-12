/**
  ******************************************************************************
  * @file    task_system_service.c
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

#include "platform_io.h"
#include "task_system_service.h"
#include "sm_log.h"
#include "system_status.h"
#include "task_heatting_service.h"
#include "data_base_info.h"
#include "system_param.h"
//#include "SEGGER_RTT.h"
//#include "serial_line.h"
#include "shell_cmd_handle.h"
#include "protocol_usb.h"


#include "task_bt_service.h"
#include "task_ui_service.h"
#include "task_charge_service.h"


#include "stdlib.h"
/* Header file includes */
#include <string.h>
#include "cyhal.h"
#include "cybsp.h"

#include "cybt_platform_trace.h"
#include "cycfg_gatt_db.h"
#include "cycfg_bt_settings.h"
#include "app_bt_utils.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_uuid.h"
#include "wiced_memory.h"
#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "cyhal_gpio.h"
#include "wiced_bt_l2c.h"
#include "cyabs_rtos.h"
#include "sm_log.h"
#include "stdlib.h"
#include "ota.h"
#include <inttypes.h>
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include "driver_beep.h"
#include "err_code.h"
#include "system_interaction_logic.h"
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "app_bt_char_adapter.h"
#include "app_bt_char.h"
#include "event_data_record.h"
#include "session_data_record.h"
#include "app_bt_char.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#include "ota_context.h"
#if (OTA_SERVICE_SUPPORT == 1)
/* OTA related header files */
#include "cy_ota_api.h"
#endif

#define SHORT_PRESS     	20u // 短按时间
#define LONG_PRESS      	2000u // 长按
#define LONG_PRESS_CLEAN    4000u // 长按
#define EXIT_SHIPPING_MODE_PRESS      	3000u // 长按, 相当于断电重启，
#define USB_JITTER_INSERT_DELAY		300u // USB 抖动延时 ms
#define USB_JITTER_PULLOUT_DELAY	1000u // USB 抖动延时 ms
#define MULTIPLES_SHORT_PRESS   500u // 按键连续短按最大时间间隔 ms
#define TIME_WAKEUP (24*60*60) // s

/**
 * @brief FreeRTOS variable to store handle of task created to update and send dummy
   values of system
 */
TaskHandle_t task_system_handle;
bool pctool_idl_cmd = false;
uint32_t g_device_init_time = 0;
volatile uint32_t g_multiplesShortPressTicks = 0;
volatile uint8_t g_multiplesShortPressCnt = 0;
bool g_swStatus = true; // true sw打开，false 关闭

volatile uint32_t pollCnt = 0;
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
static KeyPressCnt_t g_keyPressCnt_t = {0};

//bool g_updateMotorEvent = false;
bool g_updateLifeCycleStatus = false;
bool get_update_lifecycle_status(void)
{
    return g_updateLifeCycleStatus;
}
void set_update_lifecycle_status(bool status)
{
    g_updateLifeCycleStatus = status;
}

KeyPressCnt_t *get_key_cnt_handle(void)
{
	return &g_keyPressCnt_t;
}

void clr_key_cnt(void)
{
	memset(&g_keyPressCnt_t, 0, sizeof(g_keyPressCnt_t));
}

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

static bool bOTACancel_WaitKeyRelease=0; //bug1966591
static bool bFindMe_WaitKeyRelease = 0;	//Bug 2012331 

bool is_sw_on(void)
{
    return g_swStatus;
}

void disable_swd(void)
{
#ifdef DEF_BLE_APP_GROUP_EN // 蓝牙app团队 升级不要按按键

#else //for the moment, remove this SWD restriction while burning fw on BLE versions
    Cy_GPIO_SetHSIOM(GPIO_PRT6, 7UL, HSIOM_SEL_GPIO);
    Cy_GPIO_SetDrivemode(GPIO_PRT6, 7UL, CY_GPIO_DM_ANALOG);
    Cy_GPIO_SetHSIOM(GPIO_PRT6, 6UL, HSIOM_SEL_GPIO);
    Cy_GPIO_SetDrivemode(GPIO_PRT6, 6UL, CY_GPIO_DM_ANALOG);
#endif

    g_swStatus = false;
}

void enable_swd(void)
{
//#ifndef DE_RDP_ON_EN // 如果没有定义则打开
    Cy_GPIO_SetHSIOM(GPIO_PRT6, 7UL, P6_7_CPUSS_SWJ_SWCLK_TCLK);
    Cy_GPIO_SetDrivemode(GPIO_PRT6, 7UL, CY_GPIO_DM_PULLDOWN);

    Cy_GPIO_SetHSIOM(GPIO_PRT6, 6UL, P6_6_CPUSS_SWJ_SWDIO_TMS);
    Cy_GPIO_SetDrivemode(GPIO_PRT6, 6UL, CY_GPIO_DM_PULLUP);
//#endif
    g_swStatus = true;
}

volatile int g_longPressResetFlag = 0;

void sys_set_reset_flag(int val)
{
	g_longPressResetFlag = val;
}

int sys_get_reset_flag(void)
{
	return g_longPressResetFlag;
}

/**
  * @brief  获取任务指针
  * @param  None
  * @return 返回指针
  * @note   None
  */
TaskHandle_t* get_task_system_handle(void)
{
    return &task_system_handle;
}

volatile bool g_usbStatus = false; // USB接入状态，USB接入：true, USB拔出：false
bool get_usb_status(void)
{
    return g_usbStatus;
}

void set_usb_status(bool status)
{
    g_usbStatus = status;
}

/**
  * @brief  获取开机时的按键状态，如果按键一直是按下的，不进行加热，只进行退出船运模式，退出船运模式后，也不进行加热，可以进入睡眠，知道按键弹起才解锁该按键的功能
  * @param  None
  * @return None
  * @note   None
  */
void sys_start_key_status(void)
{
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    uint8_t buf[3];
    keyDev->read( (uint8_t*)&buf, 3);
    KeyLockStatus_t *pt_keyLockStatus = get_key_lock_status();
    pt_keyLockStatus->baseKey = buf[0]; // 按下就是锁定
    pt_keyLockStatus->boostKey = buf[1]; // 按下就是锁定
    pt_keyLockStatus->usb = buf[2]; // USB插入， 暂时没用
}

static uint32_t baseKeyPressTime = 0;
static uint32_t boostKeyPressTime = 0;

/**
  * @brief  base按键按下计时计算
  * @param  None
  * @return None
  * @note   None
  */
void base_key_press_time_calculator(void) // BUG 1934401
{
    static uint8_t status = 0;
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    uint8_t buf[3];
    keyDev->read( (uint8_t*)&buf, 3);
    static uint32_t msTick = 0;
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    switch (status) {
        case 0:
            if (buf[0] == 1) {
                msTick = msTickDev->read( (uint8_t*)&msTick, 4);
                status++;
            } else {
                baseKeyPressTime = 0;
            }
            break;
        case 1:
            if (buf[0] == 1) {
                baseKeyPressTime = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
            } else {
                baseKeyPressTime = 0;
                status = 0;
            }
            break;
        default: status = 0;
            break;
    }
}

/**
  * @brief  按键按下计时计算
  * @param  None
  * @return None
  * @note   None
  */
void boost_key_press_time_calculator(void) // BUG 1934401
{
    static uint8_t status = 0;
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    uint8_t buf[3];
    keyDev->read( (uint8_t*)&buf, 3);
    static uint32_t msTick = 0;
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    switch (status) {
        case 0:
            if (buf[1] == 1) {
                msTick = msTickDev->read( (uint8_t*)&msTick, 4);
                status++;
            } else {
                boostKeyPressTime = 0;
            }
            break;
        case 1:
            if (buf[1] == 1) {
                boostKeyPressTime = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
            } else {
                boostKeyPressTime = 0;
                status = 0;
            }
            break;
        default: status = 0;
            break;
    }
}

/**
  * @brief  base按键按下计时获取
  * @param  None
  * @return None
  * @note   None
  */
uint32_t get_base_key_press_time(void) // BUG 1934401
{
    return baseKeyPressTime;
}

/**
  * @brief  boost按键按下计时获取
  * @param  None
  * @return None
  * @note   None
  */
uint32_t get_boost_key_press_time(void) // BUG 1934401
{
    return boostKeyPressTime;
}



void key_usb_scan_process(void)
{
    static uint8_t statusStandard = 0;
    static uint8_t statusBoost = 0;
    static uint8_t statusUsb = 0;
    static uint8_t statusDouble = 0;

    static uint32_t tickStandard = 0;
    static uint32_t tickBoost = 0;
    static uint32_t tickUsb = 0;
    static uint32_t tickDouble = 0;

    SysStatus_u tempSysStatus;
    //INI 相关
    int16_t* ini_p = get_ini_val_info_handle();
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ptIoDev idleDev = io_dev_get_dev(DEV_IDLE);
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    KeyLockStatus_t *pt_keyLockStatus = get_key_lock_status();
    int16_t* pIniVal = get_ini_val_info_handle();
    uint8_t buf[3];
    keyDev->read( (uint8_t*)&buf, 3);
    TaskHandle_t *temp_handle;
    wiced_result_t result;
    static uint8_t rebootFlag = 1;
    static uint16_t bootSwapTime = 0;
    static uint8_t lowBatFlag = 0;
    static uint8_t doubleKeyDown = 0;
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	SysSubStatus_u subSystemStatus = get_subSystem_status();
	uint8_t event_data[1];
	static uint8_t key_buf[3];
// 记录按键1
	if (buf[0])
	{
		if (key_buf[0] == 0)
		{
			key_buf[0] = 1;
			event_data[0] = EVENT_CODE_KEY1;
			event_record_generate(EVENT_CODE_BUTTON_PRESS, event_data, 1);
		}
	}
	else if (key_buf[0])
	{
		key_buf[0] = 0;
	}
// 记录按键2
	if (buf[1])
	{
		if (key_buf[1] == 0)
		{
			key_buf[1] = 1;
			event_data[0] = EVENT_CODE_KEY2;
			event_record_generate(EVENT_CODE_BUTTON_PRESS, event_data, 1);
		}
	}
	else if (key_buf[1])
	{
		key_buf[1] = 0;
	}

	uint8_t usbKey;
	if (p_tMontorDataInfo->chg.bus_volt > 3000)
	{
		usbKey = 1;
	}
	else
	{
		usbKey = 0;
	}
	
	if (usbKey != key_buf[2])					// bug 1968519	不能简单使用buf[2]判断，因为加热时，buf[2]读出是零
												// 加热过程中，拔出USB，目前测试会延迟20秒才能检测到释放，跟电压泄放硬件相关
	{
		key_buf[2] = usbKey;
		if (usbKey)
		{
			sm_log(SM_LOG_NOTICE, "Charg is plug!\r\n");
			set_system_external_even(EXT_EVENT_USB_PLUG_HAL);
		}
		else
		{
			sm_log(SM_LOG_NOTICE, "Charg is unplug!\r\n");
			set_system_external_even(EXT_EVENT_USB_UNPLUG_HAL);
		}
	}	
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//	if (bOTACancel_WaitKeyRelease && (buf[0] == 0) && (buf[1] == 0))		// 先处理完按键释放后再更改，否则可能会点亮屏幕
//	{
//		bOTACancel_WaitKeyRelease = 0;
//		sm_log(SM_LOG_NOTICE, "000!\r\n");
//	}

    // 添加开机动画
	if (rebootFlag == 1) {
		rebootFlag = 0;
		disable_swd(); // 进入主任务，先关闭SWD，防止硬件DPDM干扰影响系统运行
		if (get_shipping_mode() == 0) {
            if (BleCongested_Read() == false) {
                set_system_external_even(EXT_EVENT_BOOT);
            } else {
                BleCongested_Write(0);
//                BleData_update_to_flash();
            }
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			event_record_generate(EVENT_CODE_HARDWARE_RESET, event_data, 0);
			clr_key_cnt();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    	} else { // 如果是shippingmode,立即退出该模式
    		//set_system_external_even(EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS);
            if (BleCongested_Read() == false) {
        		set_current_ui_detail_status(UI_SHIPPING);
            } else {
                BleCongested_Write(0);
//                BleData_update_to_flash();
            }
    	}
    }

// 按键长按，复位前做存储动作
	static bool bNeedCheckLongPress;
	if (get_base_key_press_time() > 7000 || get_boost_key_press_time() > 7000)
	{
		if (bNeedCheckLongPress)
		{
			bNeedCheckLongPress = 0;
			flash_data_save_change(0);
		}
	}
	else
	{
		bNeedCheckLongPress = 1;
	}

    if (pt_keyLockStatus->baseKey == KEY_LOCK && buf[0] == false) {
        pt_keyLockStatus->baseKey = KEY_UNLOCK;
    }
    if (pt_keyLockStatus->boostKey == KEY_LOCK && buf[1] == false) {
        pt_keyLockStatus->boostKey = KEY_UNLOCK;
    }
    if (pt_keyLockStatus->baseKey == KEY_LOCK || pt_keyLockStatus->boostKey == KEY_LOCK) {
        bootSwapTime = 2100;
    } else {
        bootSwapTime = 0;
    }

	if (app_bt_ota_upgrade_is_running() && (SUBSTATUS_LOADING == subSystemStatus))	// OTA长按取消
	{
		if (get_base_key_press_time() > 2000 || get_boost_key_press_time() > 2000)
		{
			set_system_external_even(EXT_EVENT_LOADING_STOP);
            bOTACancel_WaitKeyRelease = 1; //bug 1966591
            /* restore key detect*/
            set_update_ui_timer_cnt(1000);
            /* restore ntf_loop_exit_flag*/
            app_bt_char_bt_service_task_ntf_loop_exit_flag_set(0);
		}
		return;
	}
	
    if (get_update_ui_timer_cnt() > 0) { // 升级UI、APP 、BOOT、INI时，不响应按键, 升级APP和boot时也不响应按键
        return;
    }
    if (buf[0] == true && buf[1] == true && sys_get_reset_flag() == 0) { // 增加长按判断，是因为长按重启，再按另一个按键，导致异常bug:1831277
        statusStandard = 0;
        statusBoost = 0;
        doubleKeyDown = 1;

        switch (statusDouble) {
            case 0:
                if (pt_keyLockStatus->baseKey == KEY_UNLOCK && pt_keyLockStatus->boostKey == KEY_UNLOCK) {
                    tickDouble = msTickDev->read( (uint8_t*)&tickDouble, 4);
                    statusDouble++;
                }
                break;
            case 1:
                if (msTickDev->read( (uint8_t*)&tickDouble, 4) > tickDouble + LONG_PRESS) {
                    tempSysStatus = get_system_status();
                    if (HEATTING_CLEAN == tempSysStatus) { // 两个都按的情况，暂时不响应

                    }
                    statusDouble++;
                }
                break;
            case 2:
                if (msTickDev->read( (uint8_t*)&tickDouble, 4) > tickDouble + LONG_PRESS_CLEAN) {
                    tempSysStatus = get_system_status();
					if (get_lock_mode()) // 锁定状态，暂时末处理
					{
						update_idl_delay_time();
						set_system_external_even(EXT_EVENT_LOCK);
					} else if (((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) && (SUBSTATUS_IDLE == subSystemStatus)) {
						if (bOTACancel_WaitKeyRelease == 0)
						{
#ifdef DEF_DRY_CLEAN_EN							
							set_system_external_even(EXT_EVENT_KEY_CLEAN_HEAT_PRESS);
#endif
						}
                    }
                    statusDouble++;
                }
                break;
            default: break;
        }
    }
    if (doubleKeyDown == 1) {
        statusStandard = 0;
        statusBoost = 0;
    }
    if (buf[0] == false || buf[1] == false) {
        statusDouble = 0;
    }
    if (buf[0] == false && buf[1] == false) { // 两个按键都松开，再重新开始
        doubleKeyDown = 0;
        sys_set_reset_flag(0);
    }

	if (bFindMe_WaitKeyRelease && statusStandard == 0 && statusBoost == 0)
	{
		bFindMe_WaitKeyRelease = 0;
	}
	
    switch (statusStandard) {
        case 0:
            if (buf[0] == true) { // true: 处于按下状态， false：处于弹起状态
                tickStandard = msTickDev->read( (uint8_t*)&tickStandard, 4);
                statusStandard++;
            }
            break;
        case 1:
            if (buf[0] == false) {
                statusStandard = 0;
            } else {
                if (msTickDev->read( (uint8_t*)&tickStandard, 4) > tickStandard + SHORT_PRESS) {
               // 	sm_log(SM_LOG_NOTICE, "keyStandard is pressed!\r\n");
                //	sys_set_reset_flag(0);
					if (IDLE ==get_system_status()) {
                		enable_swd(); // IDLE status 按键按下使能一次SWD，IDLE状态下防止硬件DPDM干扰影响系统运行
					}
                	statusStandard++;
                }
            }
            break;
        case 2:
            if (buf[0] == false && get_shipping_mode() == 0) {
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				if (bFindMe_WaitKeyRelease == 0)
				{
					g_keyPressCnt_t.bChecking = true;
					g_keyPressCnt_t.timeTick = msTickDev->read(NULL, 4);
					if (g_keyPressCnt_t.cnt2)
					{
						g_keyPressCnt_t.cnt1 = 0;
						g_keyPressCnt_t.cnt2 = 0;
					}
					if (g_keyPressCnt_t.cnt1 < 0xff)
					{
						g_keyPressCnt_t.cnt1++;
					}
				}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
                // 短按弹起事件，如果是空闲状态，或者充电状态，则显示电量，
                if ((IDLE ==get_system_status() || CHARGING ==get_system_status() || BT_SERVICE_CHAR_COMM ==get_system_status()) && get_shipping_mode() == 0) {
                    
                    update_idl_delay_time(); // 更新休眠定时器
            		if (true == is_sw_on()) {
            			disable_swd();// 按键弹起,禁止SWD，IDLE状态下防止硬件DPDM干扰影响系统运行
            		}
					if (bOTACancel_WaitKeyRelease == 0 && bFindMe_WaitKeyRelease == 0)
					{
                    	set_system_external_even(EXT_EVENT_KEY_STANDARD_SHORT_PRESS);
					}
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//					event_data[0] = EVENT_CODE_KEY1;
//					event_record_generate(EVENT_CODE_BUTTON_PRESS, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

// debug qr test
#if 0
			#include "task_ui_service.h"
			#include "err_code.h"
			static uint8_t keyCnt = 0;
			keyCnt++;
			switch (keyCnt%3) {
			case 0:
				set_current_ui_detail_status(UI_EOL);
			break;

			case 1:
				set_current_ui_detail_status(UI_ERR_CRITICAL);
			break;

			default:
				set_current_ui_detail_status(UI_BLE_DOWNLOAD);
			break;
			}
#endif

                    sm_log(SM_LOG_INFO, "soc: %d, re_soc: %d\r\n", p_tMontorDataInfo->bat.soc, p_tMontorDataInfo->bat.remap_soc);
                    if (buf[1] == false) { // 两个按键之一按下，两个同时按下不算
                        if (msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4) - g_multiplesShortPressTicks  < MULTIPLES_SHORT_PRESS) {
                            g_multiplesShortPressCnt++;
                        } else {
                            g_multiplesShortPressCnt = 1;
                        }
                        g_multiplesShortPressTicks = msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4);
                    }
                }
             //   sm_log(SM_LOG_NOTICE, "keyStandard is released!\r\n");
                g_device_init_time = 0;
                statusStandard = 0;
            } else {
                if (msTickDev->read( (uint8_t*)&tickStandard, 4) > tickStandard + LONG_PRESS - g_device_init_time - bootSwapTime) {
                	// tickStandard = msTickDev->read( (uint8_t*)&tickStandard, 4);// 长按事件, 添加发送消息
				//	sm_log(SM_LOG_NOTICE, "long time press keyStandard!!!\r\n");
					if (get_shipping_mode() == 0 && pt_keyLockStatus->baseKey == KEY_UNLOCK) { // 按键没用被锁定并且不在船运模式才可以加热
                        tempSysStatus = get_system_status();
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
						clr_key_cnt();
						if (get_lock_mode())	// 锁定状态，暂时末处理
						{
							update_idl_delay_time();
							set_system_external_even(EXT_EVENT_LOCK);
						}
//						else if ((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) { // 空闲状态或充电则加热
						else if (subSystemStatus == SUBSTATUS_PARING || subSystemStatus == SUBSTATUS_PARING_QRCODE)
						{
							set_system_external_even(EXT_EVENT_PARING_CANCLE);
						}
						else if (((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) && (SUBSTATUS_IDLE == subSystemStatus)) { // 空闲状态或充电则加热
#else						
                        if ((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) { // 空闲状态或充电则加热
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
							if (bOTACancel_WaitKeyRelease == 0)
							{
                           		set_system_external_even(EXT_EVENT_KEY_STANDARD_HEAT_PRESS);
							}
                    		//if (true == is_sw_on()) { // USB判断中 已有禁止操作
                    		//	disable_swd();// 进入加热,禁止SWD，IDLE状态下防止硬件DPDM干扰影响系统运行
                    		//}
                        }else if (HEATTING_TEST == tempSysStatus || HEATTING_STANDARD == tempSysStatus || HEATTING_BOOST == tempSysStatus || HEATTING_CLEAN == tempSysStatus) { // 加热状态则停止加热
							// set_system_status(IDLE);
                            set_system_external_even(EXT_EVENT_KEY_STANDARD_STOP_HEAT_PRESS);
							update_idl_delay_time(); // 更新休眠定时器
                    	} else {;}// 如果是充电状态并且足够一支烟电量，则先停止充电，再启动加热， 后续优化细化处理
					}
                    statusStandard++;
                }
            }
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            if (true == app_bt_service_char_findmy_device_is_found())
            {
                beep_stop();
				set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
				bFindMe_WaitKeyRelease = 1;
            }
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            break;
        case 3:
            if (buf[0] == false) { // 长按弹起
        		if (true == is_sw_on()) {
        			disable_swd();// 按键弹起,禁止SWD，IDLE状态下防止硬件DPDM干扰影响系统运行
        		}
                g_device_init_time = 0;
                statusStandard = 0;
            } else {
                if (get_shipping_mode() != 0) {
                	//if (msTickDev->read( (uint8_t*)&tickStandard, 4) > tickStandard + EXIT_SHIPPING_MODE_PRESS - g_device_init_time - bootSwapTime) {
                	if (msTickDev->read( (uint8_t*)&tickStandard, 4) > tickStandard + 100) { // 只要信号能保持到系统启动完成，消抖即可触发退出shipmode，时间约3s(0.7s+2.1s+0.1s)
                	//	tickStandard = msTickDev->read( (uint8_t*)&tickStandard, 4);
					//	sm_log(SM_LOG_NOTICE, "exit shipping mode!\r\n");
					//	set_shipping_mode(0);
                        //set_system_external_even(EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS);
//						clr_time_stamp_trush();			// 退出船运模式后，Dummy 读一次RTC，让Trush字段变无效
						statusStandard++;
					}
                } else if (pt_keyLockStatus->baseKey == KEY_UNLOCK){ // 一直长按，关闭加热，并触发reset
                	if (msTickDev->read( (uint8_t*)&tickStandard, 4) > tickStandard + LONG_PRESS+ LONG_PRESS - g_device_init_time) {
                		tickStandard = msTickDev->read( (uint8_t*)&tickStandard, 4);
                        sys_set_reset_flag(1);
                        update_idl_delay_time(); 
                        set_system_external_even(EXT_EVENT_BOOT_TIP);
                        statusStandard++;
                        // 保存错误码数据
// 移至长按时间判断处理                        flash_data_save_change(0);
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//						SysSubStatus_u subSystemStatus = get_subSystem_status();
						if (SUBSTATUS_PARING_QRCODE == subSystemStatus || SUBSTATUS_PARING == subSystemStatus)
						{
//							set_system_external_even(EXT_EVENT_PARING_CANCLE);
							set_subSystem_status(SUBSTATUS_IDLE);
							app_bt_adv_stop();
						}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)			
                	}
                }
            }
            break;
        case 4:
            if (buf[0] == false) {// 长按弹起
        		if (true == is_sw_on()) {
        			disable_swd();// 按键弹起,禁止SWD，IDLE状态下防止硬件DPDM干扰影响系统运行
        		}
				update_idl_delay_time(); // 更新休眠定时器
                g_device_init_time = 0;
              //  sys_set_reset_flag(0);
                statusStandard = 0;
            } else {

            }
            break;
        default:
        	statusStandard = 0;
        	break;
    }

    switch (statusBoost) {
        case 0:
            if (buf[1] == true) {
                tickBoost = msTickDev->read( (uint8_t*)&tickBoost, 4);
                statusBoost++;
            }
            break;
        case 1:
            if (buf[1] == false) {
                statusBoost = 0;
            } else {
                if (msTickDev->read( (uint8_t*)&tickBoost, 4) > tickBoost + SHORT_PRESS) {
                //	sys_set_reset_flag(0);
                    statusBoost++; //
                }
            }
            break;
        case 2:
            if (buf[1] == false && get_shipping_mode() == 0) {
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				if (bFindMe_WaitKeyRelease == 0)
				{
					g_keyPressCnt_t.bChecking = true;
					g_keyPressCnt_t.timeTick = msTickDev->read(NULL, 4);
					if (g_keyPressCnt_t.cnt1)
					{
						g_keyPressCnt_t.cnt1 = 0;
						g_keyPressCnt_t.cnt2 = 0;
					}
					if (g_keyPressCnt_t.cnt2 < 0xff)
					{
						g_keyPressCnt_t.cnt2++;
					}
				}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)	
                // 短按弹起事件，如果是空闲状态，或者充电状态，则显示电量，
                if ((IDLE ==get_system_status() || CHARGING ==get_system_status()) && get_shipping_mode() == 0) {
                    update_idl_delay_time(); // 更新休眠定时器
					if (bOTACancel_WaitKeyRelease == 0 && bFindMe_WaitKeyRelease == 0)
					{
                    	set_system_external_even(EXT_EVENT_KEY_BOOST_SHORT_PRESS);
					}
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//					event_data[0] = EVENT_CODE_KEY2;
//					event_record_generate(EVENT_CODE_BUTTON_PRESS, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

// debug
#if 0



#endif 
		
                    sm_log(SM_LOG_INFO, "soc: %d, re_soc: %d\r\n", p_tMontorDataInfo->bat.soc, p_tMontorDataInfo->bat.remap_soc);
                    if (buf[0] == false) { // 两个按键之一按下，两个同时按下分开弹起，只算一个，不考虑同时弹起的情况
                        if (msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4) - g_multiplesShortPressTicks  < MULTIPLES_SHORT_PRESS) {
                            g_multiplesShortPressCnt++;
                        } else {
                            g_multiplesShortPressCnt = 1;
                        }
                        g_multiplesShortPressTicks = msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4);
                    }
                }
             //   sm_log(SM_LOG_NOTICE, "keyBoost is released!\r\n");
                g_device_init_time = 0;
                statusBoost = 0;
            } else {
                if (msTickDev->read( (uint8_t*)&tickBoost, 4) > tickBoost + LONG_PRESS - g_device_init_time - bootSwapTime) {
                //	tickBoost = msTickDev->read( (uint8_t*)&tickBoost, 4);
				//	sm_log(SM_LOG_NOTICE, "long time press keyBoost!!!\r\n");// 长按事件， 添加发送消息
					if (get_shipping_mode() == 0 && pt_keyLockStatus->boostKey == KEY_UNLOCK) {
						tempSysStatus = get_system_status();
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
						clr_key_cnt();
						if (get_lock_mode())	// 锁定状态，暂时末处理
						{
							update_idl_delay_time();
							set_system_external_even(EXT_EVENT_LOCK);
						}
//						else if ((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) { // 空闲状态或充电则加热
						else if (subSystemStatus == SUBSTATUS_PARING || subSystemStatus == SUBSTATUS_PARING_QRCODE)
						{
							set_system_external_even(EXT_EVENT_PARING_CANCLE);
						}
						else if (((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) && (SUBSTATUS_IDLE == subSystemStatus)) { // 空闲状态或充电则加热
#else
						if ((IDLE == tempSysStatus) || (CHARGING == tempSysStatus)) { // 空闲状态或充电则加热
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
						if (bOTACancel_WaitKeyRelease == 0)
						{
                        	set_system_external_even(EXT_EVENT_KEY_BOOST_HEAT_PRESS);
						}
						}else if (HEATTING_TEST == tempSysStatus || HEATTING_STANDARD == tempSysStatus || HEATTING_BOOST == tempSysStatus || HEATTING_CLEAN == tempSysStatus) { // 加热状态则停止加热
                            set_system_external_even(EXT_EVENT_KEY_BOOST_STOP_HEAT_PRESS);
							update_idl_delay_time(); // 更新休眠定时器
                        }
                    }
                    statusBoost++;
                }
            }
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            if (true == app_bt_service_char_findmy_device_is_found())
            {
                beep_stop();
				set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
				bFindMe_WaitKeyRelease = 1;
            }
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            break;
        case 3:
            if (buf[1] == false) { // 长按弹起
                statusBoost = 0;
                g_device_init_time = 0;
            } else {
                if (get_shipping_mode() != 0) {
                	//if (msTickDev->read( (uint8_t*)&tickBoost, 4) > tickBoost + EXIT_SHIPPING_MODE_PRESS - g_device_init_time - bootSwapTime) {
                	if (msTickDev->read( (uint8_t*)&tickBoost, 4) > tickBoost + 100) { // 只要信号能保持到系统启动完成，消抖即可触发退出shipmode，时间约3s(0.7s+2.1s+0.1s)
					//sm_log(SM_LOG_NOTICE, "exit shipping mode!\r\n");
					//	set_shipping_mode(0);
                        //set_system_external_even(EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS);
//						clr_time_stamp_trush();			// 退出船运模式后，Dummy 读一次RTC，让Trush字段变无效
						statusBoost++;
                	}
                } else if (pt_keyLockStatus->boostKey == KEY_UNLOCK){ // 一直长按，关闭加热，触发reset
                	if (msTickDev->read( (uint8_t*)&tickBoost, 4) > tickBoost + LONG_PRESS + LONG_PRESS - g_device_init_time) {
                		tickBoost = msTickDev->read( (uint8_t*)&tickBoost, 4);
                		sys_set_reset_flag(1);
						update_idl_delay_time(); // 更新休眠定时器
                        set_system_external_even(EXT_EVENT_BOOT_TIP);
						statusBoost++;
                        // 保存错误码数据
// 移至长按时间判断处理						 flash_data_save_change(0);
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//						SysSubStatus_u subSystemStatus = get_subSystem_status();
						if (SUBSTATUS_PARING_QRCODE == subSystemStatus || SUBSTATUS_PARING == subSystemStatus)
						{
//							set_system_external_even(EXT_EVENT_PARING_CANCLE);
							set_subSystem_status(SUBSTATUS_IDLE);
							app_bt_adv_stop();
						}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)			

                	}
                }
            }
            break;
        case 4:
            if (buf[1] == false) {// 长按弹起
				update_idl_delay_time(); // 更新休眠定时器
                g_device_init_time = 0;
              //  sys_set_reset_flag(0);
                statusBoost = 0;
            } else {

            }
            break;
        default:
        	statusBoost = 0;
        	break;
    }

// 处理完按键释放后，再更新OTA按键状态
	if (bOTACancel_WaitKeyRelease && (buf[0] == 0) && (buf[1] == 0))		// 先处理完按键释放后再更改，否则可能会点亮屏幕
	{
		bOTACancel_WaitKeyRelease = 0;
	}

    if (buf[0] == true && buf[1] == true) { // 两个按键已弹起
        if (msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4) - g_multiplesShortPressTicks  >= MULTIPLES_SHORT_PRESS) {
            if (g_multiplesShortPressCnt == 5) { // 自清洁
            } else if (g_multiplesShortPressCnt == 3) { // 蓝牙配对

            }
            g_multiplesShortPressCnt = 0;
            g_multiplesShortPressTicks = msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4);
        }
    }
    /*************************************************************************/
	tempSysStatus = get_system_status();
	if ((HEATTING_STANDARD == tempSysStatus) ||
		(HEATTING_BOOST == tempSysStatus) ||
		(HEATTING_TEST == tempSysStatus) ||
		(HEATTING_CLEAN == tempSysStatus) ||
		(HEAT_MODE_TEST_VOLTAGE == tempSysStatus) ||
		(HEAT_MODE_TEST_POWER == tempSysStatus) ||
		(HEAT_MODE_TEST_TEMP == tempSysStatus)) { // 只要加热就进行SWD禁止和SUB插入判断
		if (true == is_sw_on()) {
			disable_swd(); // 加热、充电和休眠时关闭SWD, idle后 有按键按下才打开
		}
        if (statusUsb != 4) {// 延时恢复USB判断,和加热后低电量显示
            statusUsb = 4;
        }
        // 加热过程，发现USB被拉低，则设置为高阻态，防止VBUS给VSYS供电
		if (buf[2] == true) { // USB低电平，设置为高阻态
			if (false == is_vbus_hiz_mode()) {
				chg_set_hiz_mode(1);
			}
		}
		// 进入加热过程，无论USB移除或VBUS为高阻模式,USB状态设定为false,等待加热结束，需重新判断USB状态
		if (true == get_usb_status()) {
			set_usb_status(false);
			p_tMontorDataInfo->state = 0; // clear state
			p_tMontorDataInfo->partab = 0; // clear state
		}
		return;
	}

	UiTaskDetailStatus_u uiCurrent = get_current_ui_detail_status();
	if ((UI_HEATTING == uiCurrent) || // 等待加热UI显示结束
		(UI_ERR_REBOOTING == uiCurrent) ||
		(UI_BOOT_TIP == uiCurrent) || // 等待开机UI显示结束
		(UI_CLEAN_PRO == uiCurrent) || // 等待清洁UI显示结束
		(UI_SHIPPING == uiCurrent)) { // 等待ship UI显示结束
		return;
	}

	if (is_vbus_hiz_mode()) { // VBUS高阻模式，充电IC的PG管脚即USB管脚信号不可信, 只有加热的时候设置为高阻模式，加热过程如果拔出USB，则无法识别USB拔出
		chg_set_hiz_mode(0); // 解除高阻态
		statusUsb = 4; // 延时恢复USB判断
	}

	// IDLE状态下 只要USB插入就禁掉SWD
	if (buf[2] == true) {
		if (true == is_sw_on()) {
			disable_swd();
		}
	}

 	switch (statusUsb) {
 	case 0:
 		if ((buf[2] == true) && (p_tMontorDataInfo->chg.bus_volt >= PROT_USB_UNDER_VOLT_THRESHOLD)) { // PG触发，并且Vbus >= 4.6V
			tickUsb = msTickDev->read((uint8_t*) &tickUsb, 4);
			statusUsb++;
		}
 		break;

 	case 1:
 		if ((buf[2] == false) || (p_tMontorDataInfo->chg.bus_volt < PROT_USB_UNDER_VOLT_THRESHOLD)) {// 未进入充电前，只要USB PG信号或USB电压出现了抖动，跳转状态0重新判断
			statusUsb = 0;
		} else {
        	if (get_shipping_mode() != 0) {
             	//if (msTickDev->read( (uint8_t*)&tickUsb, 4) > tickUsb + EXIT_SHIPPING_MODE_PRESS - g_device_init_time- 2100) { // 2100 swap跳转时间 user flow 没有USB插入退出船运，需要客户确认
             	if (msTickDev->read( (uint8_t*)&tickUsb, 4) > tickUsb + 100) { // 只要信号能保持到系统启动完成，消抖即可触发退出shipmode，时间约3s(0.7s+2.1s+0.1s)
                	//set_system_external_even(EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS);
//                	clr_time_stamp_trush();			// 退出船运模式后，Dummy 读一次RTC，让Trush字段变无效
         			statusUsb = 0;
             	}
        	} else {
         		if (msTickDev->read((uint8_t*) &tickUsb, 4) > (tickUsb + USB_JITTER_INSERT_DELAY)) {
         			tickUsb = msTickDev->read((uint8_t*) &tickUsb, 4);
         		    sm_log(SM_LOG_NOTICE, "Type-c is plug!\r\n");
                    set_system_external_even(EXT_EVENT_USB_PLUG);
                    set_usb_status(true);
         			statusUsb = 2;
         		}
        	}
		}
 		break;

 	case 2: // 已在充电状态，判断PG和Vbus是否符合移除条件 // 降低USB移除的Vbus电压判断值，防止劣质电源输入，充电IC ICO 波动导致的误动作
         if ((buf[2] == false) || (p_tMontorDataInfo->chg.bus_volt < (PROT_USB_UNDER_VOLT_THRESHOLD-100))) {
            tickUsb = msTickDev->read((uint8_t*) &tickUsb, 4);
            statusUsb = 3;
         }
 		break;

	case 3:
		if ((buf[2] == false) || (p_tMontorDataInfo->chg.bus_volt < (PROT_USB_UNDER_VOLT_THRESHOLD-100))) {
        	if (msTickDev->read((uint8_t*) &tickUsb, 4) > (tickUsb + USB_JITTER_PULLOUT_DELAY)) { // USB移除消抖判断加长，否则ESD测试会引起充电中断
        		update_idl_delay_time(); // 更新休眠定时器
                set_usb_status(false);
				p_tMontorDataInfo->state = 0; // clear state
				p_tMontorDataInfo->partab = 0; // clear state
				sm_log(SM_LOG_NOTICE, "Type-c is removed!\r\n");
                set_system_external_even(EXT_EVENT_USB_UNPLUG);
        		statusUsb = 0;
        	}
        } else {
            statusUsb = 2; // 已在充电状态，如果PG和Vbus出现过抖动，跳回状态2重新判断
        }
		break;

	case 4:
		tickUsb = msTickDev->read((uint8_t*) &tickUsb, 4); // 加热完全结束，更新定时，以防无初始化状态
		statusUsb = 5;
 		break;

 	case 5:
        if (UI_ERR_BAT_LOW <= get_current_ui_detail_status()) {
            if (UI_ERR_BAT_LOW == get_current_ui_detail_status()) {
                lowBatFlag = 1;
            }
        	break;
        }
 		if (msTickDev->read((uint8_t*) &tickUsb, 4) > (tickUsb + 100)) {
 			if (buf[2] == false) { // USB 移除
                // 如果电量低，则提示电量低
                 // 获取电池电量、
                if (lowBatFlag == 0) {
                    uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;
                    if (0 == sys_get_reset_flag() && batteryLevel < ini_p[WAR_BAT_LOW_SOC]) { // 没长按复位，USB没有接入 提示电量低
                         if (get_current_ui_detail_status() < UI_ERR_BAT_LOW) {
                              set_current_ui_detail_status(UI_ERR_BAT_LOW);
                         }
                    }
                } else {
                    lowBatFlag = 0;
                }
			}
 			statusUsb = 0;
 		}
 		break;
 	default:
 		statusUsb = 0;
 		break;
 	}

 	static uint8_t statusUsbOld = 0;
 	if (statusUsbOld != statusUsb) {
 		sm_log(SM_LOG_NOTICE, "Usb status:%d\r\n", statusUsb);
 		statusUsbOld = statusUsb;
 	}
}

typedef struct AppDetectorInfo_t
{
    uint32_t heatR;
    uint32_t heatI;
    uint32_t heatP;
} AppDetectorInfo_t;

void detector_idle_scan(void)
{
    AppDetectorInfo_t appDetectorInfo;
    ptIoDev detectorDev = io_dev_get_dev(DEV_ADC);
    detectorDev->read( (uint8_t*)&appDetectorInfo, sizeof(AppDetectorInfo_t));
    sm_log(SM_LOG_NOTICE, "I = %lu, R = %lu, P = %lu\r\n",appDetectorInfo.heatI, appDetectorInfo.heatR, appDetectorInfo.heatP);
}


static ptIoDev msTickDev;
static MotorInfo_t motorInfo, motorInfo_1;
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
static uint8_t motor_strength = 100;
static uint8_t beep_mode = 0;
//-------------------------------------//
/**
  * @brief  振动马达设置
  * @param  None
  * @return 返回 无
  * @note   None
  */
uint8_t motor_strength_get(void)
{
	return motor_strength;
}
void motor_strength_set(uint8_t dat)
{
	motor_strength = dat;
}

uint8_t beep_mode_get(void)
{
	return beep_mode;
}
void beep_mode_set(uint8_t dat)
{
	beep_mode = dat;
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

//-------------------------------------//
/**
  * @brief  振动马达设置
  * @param  None
  * @return 返回 无
  * @note   None
  */
void motor_set2(uint8_t setHAPTIC)
{
	if(setHAPTIC > HAPTIC_TATAL)return; // 非法数据
	if (motorInfo.runTimes != 0) { // 有振动正在发生

	if (setHAPTIC != HAPTIC_FIND_ME)
	{		// FindMe振动覆盖其它
	
		if (motorInfo.HAPTICs == HAPTIC_ERR)return; // 如果正在振高优先级的振动，则不覆盖
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		if (setHAPTIC < HAPTIC_ERR)return; // 如果设置的不是高优先级振动，则不覆盖
#else
		if (setHAPTIC != HAPTIC_ERR)return; // 如果设置的不是高优先级振动，则不覆盖
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	}

		motorInfo.runIndv  = 0;
		motorInfo.stopIndv = 0;
		motorInfo.runTimes = 0;
		motorInfo.duty = 0;
		motorInfo.hz   = 4000;
		vTaskDelay(10); // 等待振动停止
	}

    ptIoDev motorDev = io_dev_get_dev(DEV_MOTOR);
    int16_t* pIniVal = get_ini_val_info_handle();
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    if (pIniVal[HAPTIC_VOLT] != 0 && p_tMontorDataInfo->bat.voltage != 0) { // 非零判断
        uint16_t setRmsV = pIniVal[HAPTIC_VOLT];
        motorInfo.dutyOfRmsV = (setRmsV*setRmsV) *100 / (p_tMontorDataInfo->bat.voltage * p_tMontorDataInfo->bat.voltage);
		//	sm_log(SM_LOG_DEBUG, "\r\n setRmsV:%d ,bat.voltage:%d mV\r\n",setRmsV,p_tMontorDataInfo->bat.voltage);
        if(motorInfo.dutyOfRmsV > 99)motorInfo.dutyOfRmsV = 99;
    } else {
        motorInfo.dutyOfRmsV = 99;
    }
	
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	motorInfo.dutyOfRmsV = motorInfo.dutyOfRmsV * motor_strength / 100;		// 强度百分比

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	FindMe_t * sFind = (FindMe_t *)pBleData->bt_find_me;

	uint8_t event_data[2];
		
	switch (setHAPTIC)
	{
		case HAPTIC_1:			{event_data[0] = 1;event_data[1] = 1;break;}
		case HAPTIC_2:			{event_data[0] = 2;event_data[1] = 1;break;}
		case HAPTIC_3:			{event_data[0] = 3;event_data[1] = 1;break;}
		case HAPTIC_4:			{event_data[0] = 4;event_data[1] = 1;break;}
		case HAPTIC_5:			{event_data[0] = 5;event_data[1] = 1;break;}
		case HAPTIC_ERR:		{event_data[0] = 0;event_data[1] = 1;break;}
		case HAPTIC_FIND_ME:	{event_data[0] = 6;event_data[1] = sFind->dat[1];break;}	// 新UI改为6
		default:				{event_data[0] = 0xff;event_data[1] = 0;break;}
	}
	
	event_record_generate(EVENT_CODE_HAPTIC_BEHAVIOUR, event_data, 2);	
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    motorInfo.duty = motorInfo.dutyOfRmsV;
    motorInfo.hz   = pIniVal[HAPTIC_PWM_FREQ] * 1000;
    motorInfo.motorTick = msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4);
 
   //sm_log(SM_LOG_DEBUG, "motor_set2(%d) duty:%d ,Frequency:%d Hz\r\n",setHAPTIC,motorInfo.duty,motorInfo.hz);
    motorDev->write( (uint8_t*)&motorInfo, sizeof(MotorInfo_t));
    motorInfo.runIndv  = 0;
    motorInfo.stopIndv = 0;
    motorInfo.HAPTICs = setHAPTIC;
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	if (setHAPTIC == HAPTIC_FIND_ME)
	{
		motorInfo.runTimes = sFind->dat[1];//60000 / MOTOR_HAPTIC_FIND_P2;
	}
	else
	{
		motorInfo.runTimes = 3;// 错误振动次数
	}
#else
    motorInfo.runTimes = 3; // 放最后，同时当更新标志，以防止主任务切换运行motor_proc2， 导致马达无法停止
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}
/**
  * @brief  振动马达 停止
  * @param  None
  * @return 返回 无
  * @note   None
  */
void motor_stop(void){
    ptIoDev motorDev = io_dev_get_dev(DEV_MOTOR);
    motorInfo.runIndv  = 0;
    motorInfo.stopIndv = 0;
    motorInfo.runTimes = 0;
    motorInfo.duty = 0;
    motorInfo.hz   = 4000;
    motorDev->write( (uint8_t*)&motorInfo, sizeof(MotorInfo_t)); 
}

void motor_set(uint16_t runIndvMs, uint16_t stopIndvMs, uint8_t duty, uint16_t loopTimes)
{
   ptIoDev motorDev = io_dev_get_dev(DEV_MOTOR);

   if (duty > 100) {duty = 100;} // MAX: 100
   if (duty < 0) {duty = 0;}     // MIN: 0

   motorInfo_1.runIndv  = runIndvMs;
   motorInfo_1.stopIndv = stopIndvMs;
   motorInfo_1.runTimes = loopTimes;
   motorInfo_1.duty = duty;
   motorInfo_1.hz   = 4000;
   motorInfo_1.motorTick = msTickDev->read( (uint8_t*)&motorInfo_1.motorTick, 4);
   motorDev->write( (uint8_t*)&motorInfo_1, sizeof(MotorInfo_t));
}

void motor_proc(void)
{
   static uint8_t oldDuty = 0;

   ptIoDev motorDev = io_dev_get_dev(DEV_MOTOR);

	if (motorInfo_1.runTimes > 0)
   {
		if (msTickDev->read( (uint8_t*)&motorInfo_1.motorTick, 4) - motorInfo_1.motorTick < motorInfo_1.runIndv)//
		{
			motorInfo_1.duty = 70;
		}else 
       {
			motorInfo_1.duty = 0;
		}

		if ((msTickDev->read( (uint8_t*)&motorInfo_1.motorTick, 4) - motorInfo_1.motorTick) >= (motorInfo_1.runIndv + motorInfo_1.stopIndv))// 
		{
			motorInfo_1.motorTick = msTickDev->read( (uint8_t*)&motorInfo_1.motorTick, 4);
			motorInfo_1.runTimes--;
		}

       if (oldDuty != motorInfo_1.duty)
       {
           oldDuty = motorInfo_1.duty;
           motorDev->write( (uint8_t*)&motorInfo_1, sizeof(MotorInfo_t));
       }
	}
}

//5种模式 相关时间定义
#define MOTOR_HAPTIC1_P1    250     //单位ms

#define MOTOR_HAPTIC2_P1    200     //单位ms
#define MOTOR_HAPTIC2_P2    380     //单位ms
#define MOTOR_HAPTIC2_P3    830     //单位ms

#define MOTOR_HAPTIC3_P1    500     //单位ms

#define MOTOR_HAPTIC4_P1    450     //单位ms
#define MOTOR_HAPTIC4_P2    630     //单位ms
#define MOTOR_HAPTIC4_P3    830     //单位ms

#define MOTOR_HAPTIC5_P1    200     //单位ms
#define MOTOR_HAPTIC5_P2    380     //单位ms
#define MOTOR_HAPTIC5_P3    580     //单位ms

#define MOTOR_HAPTIC_ERR_P1    750     //单位ms
#define MOTOR_HAPTIC_ERR_P2    750*2     //单位ms

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#define MOTOR_HAPTIC_FIND_P1    750//250     //单位ms
#define MOTOR_HAPTIC_FIND_P2    1000     //单位ms
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

void motor_proc2(void)
{
    static uint8_t oldDuty = 0;
    ptIoDev motorDev = io_dev_get_dev(DEV_MOTOR);
	if(motorInfo.runTimes > 0)
    {
        switch (motorInfo.HAPTICs)
        {
        case  HAPTIC_1:
           if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC1_P1)//250ms
            {
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else {
                motorInfo.duty = 0;
                motorInfo.runTimes = 0;
            //    sm_log(SM_LOG_DEBUG, "motorInfo HAPTIC_1\r\n");

//#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//				g_updateMotorEvent = true;
//#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				
            }
            break;
        case  HAPTIC_2:
           if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC2_P1)//
            {
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC2_P2){
                motorInfo.duty = 0;
            }else if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC2_P3){
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else{
                motorInfo.duty = 0;
                motorInfo.runTimes = 0;
            //    sm_log(SM_LOG_DEBUG, "motorInfo HAPTIC_2\r\n");
				
//#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//				g_updateMotorEvent = true;
//#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

            }
            break;

		case  HAPTIC_3:
		  if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC3_P1)//500ms
		   {
			   motorInfo.duty = motorInfo.dutyOfRmsV;
		   }else {
			   motorInfo.duty = 0;
			   motorInfo.runTimes = 0;   
		   }
		   break;
			
        case  HAPTIC_4:
           if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC4_P1)//
            {
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC4_P2){
                motorInfo.duty = 0;
            }else if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC4_P3){
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else{
                motorInfo.duty = 0;
                motorInfo.runTimes = 0;
            //    sm_log(SM_LOG_DEBUG, "motorInfo HAPTIC_4\r\n");

//#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//				g_updateMotorEvent = true;
//#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

            }
            break;
        case  HAPTIC_5:
           if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC5_P1)//
            {
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC5_P2){
                motorInfo.duty = 0;
            }else if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick < MOTOR_HAPTIC5_P3){
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else{
                motorInfo.duty = 0;
                motorInfo.runTimes = 0;
               // sm_log(SM_LOG_DEBUG, "motorInfo HAPTIC_5\r\n");

//#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//				g_updateMotorEvent = true;
//#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

            }
            break;
        case  HAPTIC_ERR:
           if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick <MOTOR_HAPTIC_ERR_P1)//750ms
            {
                motorInfo.duty = motorInfo.dutyOfRmsV;
            }else {
                motorInfo.duty = 0;
            }
            if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick >= MOTOR_HAPTIC_ERR_P2)//750*2ms
            {
                motorInfo.motorTick = msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4);
                motorInfo.runTimes--;
                if(motorInfo.runTimes == 0)
                {
                   // sm_log(SM_LOG_DEBUG, "motorInfo HAPTIC_ERR\r\n");

//#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//					g_updateMotorEvent = true;
//#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
					
                }
            }
            break;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		case  HAPTIC_FIND_ME:
		   if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick <MOTOR_HAPTIC_FIND_P1)//750ms
			{
				motorInfo.duty = motorInfo.dutyOfRmsV;
			}else {
				motorInfo.duty = 0;
			}
			if(msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4) - motorInfo.motorTick >= MOTOR_HAPTIC_FIND_P2)//750*2ms
			{
				motorInfo.motorTick = msTickDev->read( (uint8_t*)&motorInfo.motorTick, 4);
				motorInfo.runTimes--;
				if(motorInfo.runTimes == 0)
				{
				   // sm_log(SM_LOG_DEBUG, "motorInfo HAPTIC_ERR\r\n");

//					g_updateMotorEvent = true;
					
				}
			}
			break;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			
        default:
            break;
        }
    }
    if(oldDuty != motorInfo.duty )
    {
        oldDuty = motorInfo.duty;
        motorDev->write( (uint8_t*)&motorInfo, sizeof(MotorInfo_t));
        //sm_log(SM_LOG_DEBUG, "motorInfo update duty:%d ,Frequency:%d Hz\r\n",motorInfo.duty,motorInfo.hz);
    }
}

//================================================================//
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)

typedef struct
{
	uint8_t mode;				// 0停止，1固有频率，2自定义音频
	uint32_t runTime;			// 时间控制
	uint32_t duration;			// 总运行时间
	uint32_t tickBegin;			// 开始时间
	beepFrq_t freque[MAX_BEEP_CTRL_POINT];
	BeepInfo_t tBeepInfo;
} Beep_t;

static Beep_t beepInfo;

//void beep_test_set(uint16_t ring_hz,uint16_t runIndvMs,uint16_t stopIndvMs,uint16_t loopTimes)
//{
//	uint32_t timecnt;
//	timecnt = msTickDev->read( (uint8_t*)&timecnt, 4);
//
//    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);
//
//	beepInfo.mode = 1;
//	beepInfo.runTime = timecnt;
//
//    beepInfo.tBeepInfo.runIndv  = runIndvMs;
//    beepInfo.tBeepInfo.stopIndv = stopIndvMs;
//    beepInfo.tBeepInfo.runTimes = loopTimes;
//    beepInfo.tBeepInfo.duty = 50;
//    beepInfo.tBeepInfo.hz   = ring_hz;
//    beepInfo.tBeepInfo.beepTick = timecnt;
//    beepDev->write( (uint8_t*)&(beepInfo.tBeepInfo), sizeof(BeepInfo_t));
//}

void beep_set(uint16_t runIndvMs,uint16_t stopIndvMs,uint16_t loopTimes)
{
	uint32_t timecnt;
	timecnt = msTickDev->read( (uint8_t*)&timecnt, 4);
	
    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);
	
	beepInfo.mode = 1;
 	beepInfo.tickBegin = timecnt;

    beepInfo.tBeepInfo.runIndv  = runIndvMs;
    beepInfo.tBeepInfo.stopIndv = stopIndvMs;
    beepInfo.tBeepInfo.runTimes = loopTimes;
    beepInfo.tBeepInfo.duty = 50;
    beepInfo.tBeepInfo.hz   = 4000;
    beepInfo.tBeepInfo.beepTick = timecnt;
    beepDev->write( (uint8_t*)&(beepInfo.tBeepInfo), sizeof(BeepInfo_t));
}
void beep_stop(void)
{
	beepInfo.mode = 0;

    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);

    beepInfo.tBeepInfo.runIndv  = 0;
    beepInfo.tBeepInfo.stopIndv = 0;
    beepInfo.tBeepInfo.runTimes = 0;
    beepInfo.tBeepInfo.duty = 100;
 
    beepDev->write( (uint8_t*)&(beepInfo.tBeepInfo), sizeof(BeepInfo_t));
}

extern const beepFrq_t acstDefBeepFrqTbl[3][MAX_BEEP_CTRL_POINT];
void beep_set_tune(uint32_t duration)
{
	uint32_t timecnt;
	timecnt = msTickDev->read( (uint8_t*)&timecnt, 4);

	beepInfo.mode = 2;
	beepInfo.tickBegin = timecnt;
	beepInfo.duration = duration;
	
	beepInfo.tBeepInfo.runTimes = 0xffff;	
	memcpy(beepInfo.freque, &acstDefBeepFrqTbl[beep_mode][0], sizeof(beepInfo.freque));	

	beepInfo.tBeepInfo.beepTick = timecnt;
	beepInfo.tBeepInfo.stopIndv = 0;
	beepInfo.tBeepInfo.duty = 50;
//	beepInfo.tBeepInfo.hz	= 4000;
//	ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);
//	beepDev->write( (uint8_t*)&(beepInfo.tBeepInfo), sizeof(BeepInfo_t));
}



void beep_proc(void)
{
    static uint8_t oldDuty = 0;
	static uint16_t oldHz = 0;
    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);

	if (2 == beepInfo.mode)
	{
		int frqIndex = 0;

		beepInfo.runTime =	msTickDev->read( (uint8_t*)&beepInfo.tickBegin, 4) - beepInfo.tickBegin;	
		if (beepInfo.runTime < beepInfo.duration)
//		if (msTickDev->read( (uint8_t*)&beepInfo.duration, 4) < beepInfo.duration)
		{
			if(beepInfo.tBeepInfo.runTimes > 0)
			{
				// 获取beep run时间
				beepInfo.runTime =	msTickDev->read( (uint8_t*)&beepInfo.tBeepInfo.beepTick, 4) - beepInfo.tBeepInfo.beepTick;
				if (beepInfo.runTime < beepInfo.freque[MAX_BEEP_CTRL_POINT-1].time)
				{
					for(frqIndex = 0; frqIndex < MAX_BEEP_CTRL_POINT; frqIndex++)
					{
						if(beepInfo.runTime < beepInfo.freque[frqIndex].time)
						{
							break;
						}
					}
					if(frqIndex > 0)
					{
						beepInfo.tBeepInfo.hz = beepInfo.freque[frqIndex -1].frequency;  
					}
					else
					{
					   beepInfo.tBeepInfo.hz = beepInfo.freque[0].frequency;  
					}
				}
				else
				{ 
					beepInfo.tBeepInfo.hz = 0;//停止鸣叫
					if(beepInfo.runTime > beepInfo.freque[MAX_BEEP_CTRL_POINT-1].time +	beepInfo.tBeepInfo.stopIndv)
					{
//						beepInfo.tBeepInfo.runTimes -- ;
						beepInfo.tBeepInfo.beepTick = msTickDev->read( (uint8_t*)&beepInfo.tBeepInfo.beepTick, 4); // 更新起始鸣叫时间
					}
				}
			
				if(oldHz != beepInfo.tBeepInfo.hz )
				{ 
	 				oldHz = beepInfo.tBeepInfo.hz;
					beepDev->write( (uint8_t*)&beepInfo.tBeepInfo, sizeof(BeepInfo_t));
					update_idl_delay_time();				// 防止休眠而关闭Buz
				}
			}
			else
			{ 
				beep_stop();
				set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
				bFindMe_WaitKeyRelease = 0;
			}
		}
		else
		{
			beep_stop();
			set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
			bFindMe_WaitKeyRelease = 0;
		}
	}
	else
	{
		if(beepInfo.tBeepInfo.runTimes > 0){
			if(msTickDev->read( (uint8_t*)&beepInfo.tBeepInfo.beepTick, 4) - beepInfo.tBeepInfo.beepTick < beepInfo.tBeepInfo.runIndv)//
			{
				beepInfo.tBeepInfo.duty = 50;
			}else {
				beepInfo.tBeepInfo.duty = 100;
			}
			if(msTickDev->read( (uint8_t*)&beepInfo.tBeepInfo.beepTick, 4) - beepInfo.tBeepInfo.beepTick >= beepInfo.tBeepInfo.runIndv + beepInfo.tBeepInfo.stopIndv)//
			{
				beepInfo.tBeepInfo.beepTick = msTickDev->read( (uint8_t*)&beepInfo.tBeepInfo.beepTick, 4);
				beepInfo.tBeepInfo.runTimes--;
			}
	        if(oldDuty != beepInfo.tBeepInfo.duty )
	        {
	            oldDuty = beepInfo.tBeepInfo.duty;
				beepDev->write( (uint8_t*)&(beepInfo.tBeepInfo), sizeof(BeepInfo_t));
	        }
		}
	}
}

// 更新lifeCycle的Z1温度
void update_lifeCycle_Coil_Temp(int16_t Temp)
{
	LifeCycle_t *p_tLifeCycleHandle = get_life_cycle_handle();
	if (Temp > p_tLifeCycleHandle->tempZ1Max)
	{
		p_tLifeCycleHandle->tempZ1Max = Temp;
		set_update_lifecycle_status(1);
	}
	if (Temp < p_tLifeCycleHandle->tempZ1Min)
	{
		p_tLifeCycleHandle->tempZ1Min = Temp;
		set_update_lifecycle_status(1);
	}
}
#else
//================================================================//

static BeepInfo_t beepInfo;
//-------------------------------------//
//void beep_test_set(uint16_t ring_hz,uint16_t runIndvMs,uint16_t stopIndvMs,uint16_t loopTimes)
//{
//    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);
//
//    beepInfo.runIndv  = runIndvMs;
//    beepInfo.stopIndv = stopIndvMs;
//    beepInfo.runTimes = loopTimes;
//    beepInfo.duty = 50;
//    beepInfo.hz   = ring_hz;
//    beepInfo.beepTick = msTickDev->read( (uint8_t*)&beepInfo.beepTick, 4);
//    beepDev->write( (uint8_t*)&beepInfo, sizeof(BeepInfo_t));
//}
/**
  * @brief  蜂鸣器设置
  * @param  None
  * @return 返回 无
  * @note   None
  */
void beep_set(uint16_t runIndvMs,uint16_t stopIndvMs,uint16_t loopTimes)
{
    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);

    beepInfo.runIndv  = runIndvMs;
    beepInfo.stopIndv = stopIndvMs;
    beepInfo.runTimes = loopTimes;
    beepInfo.duty = 50;
    beepInfo.hz   = 4000;
    beepInfo.beepTick = msTickDev->read( (uint8_t*)&beepInfo.beepTick, 4);
    beepDev->write( (uint8_t*)&beepInfo, sizeof(BeepInfo_t));
}
void beep_stop(void)
{
    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);

    beepInfo.runIndv  = 0;
    beepInfo.stopIndv = 0;
    beepInfo.runTimes = 0;
    beepInfo.duty = 100;
 
    beepDev->write( (uint8_t*)&beepInfo, sizeof(BeepInfo_t));
}
void beep_proc(void)
{
    static uint8_t oldDuty = 0;
    ptIoDev beepDev = io_dev_get_dev(DEV_BEEP);
	if(beepInfo.runTimes > 0){
		if(msTickDev->read( (uint8_t*)&beepInfo.beepTick, 4) - beepInfo.beepTick < beepInfo.runIndv)//
		{
			beepInfo.duty = 50;
		}else {
			beepInfo.duty = 100;
		}
		if(msTickDev->read( (uint8_t*)&beepInfo.beepTick, 4) - beepInfo.beepTick >= beepInfo.runIndv + beepInfo.stopIndv)//
		{
			beepInfo.beepTick = msTickDev->read( (uint8_t*)&beepInfo.beepTick, 4);
			beepInfo.runTimes--;
		}
        if(oldDuty != beepInfo.duty )
        {
            oldDuty = beepInfo.duty;
            beepDev->write( (uint8_t*)&beepInfo, sizeof(BeepInfo_t));
        }
	}
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//================================================================//

/**
  * @brief  测试马达接口
  * @param  ctr:    马达开关， 0开，1关
  * @return None
  * @note   None
  */
bool procotol_motor_control(uint8_t ctr)
{
    if (ctr == 0) {
        motor_stop();
    } else {
        motor_set2(HAPTIC_TATAL);
    }
    return true;
}

/**
  * @brief  测试蜂鸣器接口
  * @param  ctr:    蜂鸣器开关 0关，1开
  * @return None
  * @note   None
  */
bool procotol_buzzer_control(uint8_t ctr)
{
    if (ctr == 0) {
        beep_stop();
    } else {
        beep_set(60000, 0, 1);
    }
    return true;
}

bool procotol_enter_sleep(void)
{
    pctool_idl_cmd = true;
    return true;
}
bool get_pctool_idle_cmd_status(void)
{
        return pctool_idl_cmd;
}
static uint16_t gs_logTimer = 0;//0x00 -> 关闭log输出,20~10000 -> log输出频率，单位ms

bool procotol_log_control(uint16_t optCode)
{
  //  gs_logTimer = (optCode/2);//因为2ms轮循一次
    gs_logTimer = optCode; 
    return true;
}//log
uint8_t get_pc_tool_log_en(void)
{
    if(gs_logTimer){
        return 1;
    }else{
        return 0;
    }
}
extern void get_wake_up_source(uint8_t *buf);
extern void clr_wake_up_source(void);

void data_monitor_update(void) // 退出休眠，刷新一遍数据先，为UI显示等任务准备最新数据
{
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);
    ptIoDev batDev = io_dev_get_dev(DEV_FULE_GAUGE); // 电量计 每秒更新一次 ， 其他的每20ms更新一次
    ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC); // chg <100ms更新一次 ， 其他的每20ms更新一次

    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();

	adcDev->read((uint8_t *)&(p_tMontorDataInfo->det), sizeof(DetectorInfo_t));
	HEATER *heaterInfo = get_heat_manager_info_handle();
	memcpy(&p_tMontorDataInfo->heaterInfo, heaterInfo, sizeof(HEATER));
	p_tMontorDataInfo->session = (uint16_t)get_sessions_total(); // 更新session数
	for (uint8_t i=0; i<6; i++) { // 统一刷新外设模块数据
		if (i < 2) {
			chgDev->read((uint8_t *)&(p_tMontorDataInfo->chg), sizeof(ChgInfo_t));
		}
		batDev->read((uint8_t *)&(p_tMontorDataInfo->bat), sizeof(BatInfo_t));
	}
}

extern void fg_save_data(uint8_t sta);
int fuel_gauge_check_cfg(MonitorDataInfo_t *p_tMontorDat)
{
	int ret = 0;
	uint8_t buf[1] = {FG_SET_SAVE_GMFS};
	ptIoDev fgDev = io_dev_get_dev(DEV_FULE_GAUGE);
	// 每次休眠前或者设置过电量计参数，都同步更新备份一次电量计配置文件
	fg_save_data(1);
	ret = fgDev->write((uint8_t *)&(buf), 1);
	fg_save_data(0);
	return ret;
}

extern int driver_amoled_deinit(void);

uint8_t g_monitorDataWakeUpStart = 0;
void set_monitor_start_status(uint8_t status)
{
    g_monitorDataWakeUpStart = status;
}

/**
  * @brief  应用侧低功耗处理函数
  * @param  None
  * @return None
  * @note   按键按下 唤醒 5秒之后 休眠  ---->
  */
void lo_power_proc(void)
{
    SysStatus_u sysStatus = 0;
    uint32_t tmpIdleTime = 0;
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    uint8_t buf[3];
    uint8_t wakeSourceBuf[4];
    TaskHandle_t *temp_handle;
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    int16_t* pIniVal = get_ini_val_info_handle();

    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    static uint32_t mstick = 0;
    static uint8_t bleStatus = 0;
    //进入休眠 ，step1 :挂起其他任务  step2:反初始化各个片上外设  step3:配置休眠IO状态，设置唤醒源
    //唤醒中断中开启其他任务调度
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint8_t event_data[2];
    static bool bFirstIdel = true;
// 马达停止不需要记录事件
//	if (g_updateMotorEvent)							// 该段代码不能要motor_proc2执行，因为motor_proc2在ISR里
//	{
//		g_updateMotorEvent = false;
//		event_data[0] = 0;event_data[1] = 0;		// Haptic 行为需要增加一个重复次数
//		event_record_generate(EVENT_CODE_HAPTIC_BEHAVIOUR, event_data, 2);
//	}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    if(get_system_status() == IDLE){ // 120000

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		if (bFirstIdel)
		{
			bFirstIdel = false;
			event_record_generate(EVENT_CODE_IDLE, event_data, 0);
		}

		SysSubStatus_u sysSubStatus = get_subSystem_status();
		if (sysSubStatus != SUBSTATUS_IDLE)						// 如果在配对状态，下载，Find Me 等状态，则休眠时间相应清除
		{
			if (msTickDev->read(NULL, 4) - get_idl_delay_time() >= 60000)
			{
				update_idl_delay_time();
			}
		}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    	keyDev->read( (uint8_t*)&buf, 3);
        if((msTickDev->read((uint8_t*)&tmpIdleTime, 4) - get_idl_delay_time() >= 120000 || pctool_idl_cmd) && (false == get_ui_display_status()) && (false == buf[2])) //2分钟后/CMD命令&&ui无显示任务 USB移除 开启休眠模式
        {
#if 1
			fuel_gauge_check_cfg(p_tMontorDataInfo); // 进入休眠或者shipmode前，都检查备份一次电量计的配置
#endif
            if (get_shipping_mode() == 0 && p_tMontorDataInfo->bat.voltage > pIniVal[SHIP_MODE_VOLT]) { // 电压大于INI设定值
                if (pctool_idl_cmd) {
                    pctool_idl_cmd = false;
                    procotol_fuelGauge_enter_shutdown(); // 上位机发送指令休眠增加关电量计，供工厂测试PCBA待机电流使用
                    vTaskDelay(10); // 等待串口发送完毕
                }
                set_update_ui_timer_cnt(0);
#ifdef HIBERNATE_MODE
    //            sm_log(SM_LOG_NOTICE, "Cy_SysPm_SystemEnterHibernate go!!!\r\n");
#else
                sm_log(SM_LOG_NOTICE, "Enter Deep sleep!!!\r\n");
#endif

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				event_record_generate(EVENT_CODE_SLEEP, event_data, 0);
                /* BT task Exit flag set */
                app_bt_char_bt_service_task_ntf_loop_exit_flag_set(1);
                vTaskDelay(1000); // 等待串口发送完毕, &&退出task_bt_service()
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

                // if (1 == bt_get_adv_en()) { // 第一次上电默认打开，后续关闭 POA之前采取这个策略
                //     procotol_ble_opt(0);
                // }

                if (gs_logTimer) { // 如果日志是上传状态，则先关闭日志上传
                    gs_logTimer = 0;
                }
                // 保存错误码数据
                flash_data_save_change(0);
               
                vTaskDelay(10); // 等待串口发送完毕
                set_system_status(SLEEP);
                clr_wake_up_source(); // 清唤醒源
                #if 0
                // 保存错误码数据
                if (true == get_update_errcode_status()) {
                    errcode_update_to_flash();
                    set_update_errcode_status(false);
                }
                #endif

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
// 定时唤醒
			  extern cyhal_rtc_t rtc_obj;
			  cyhal_rtc_set_alarm_by_seconds((&rtc_obj), TIME_WAKEUP);				// 暂时设置24小时
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				
              //  g_device_init_time = msTickDev->read( (uint8_t*)&g_device_init_time, 4); // 更新计时器值
                g_device_init_time = 0; // 更新计时器值
                set_monitor_start_status(1);
                device_deinit();
        		if (true == is_sw_on()) {
        			disable_swd(); // 进入休眠,禁止SWD,SLEEP状态下防止硬件DPDM干扰影响系统运行
        		}
#ifdef HIBERNATE_MODE
//*******************Hibernate****************//
                Cy_SysPm_SystemEnterHibernate();
#else
//*****************Deep sleep*****************//
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //waiting for notify
#endif
                if (get_rdp_on_mode() == 1) {
                    set_rdp_on_mode_no_save_flash(1);
                }
                // set_system_status(IDLE);
                device_init();
                //data_monitor_update(); // 取消数据刷新，set_monitor_start_status(1)已做处理
                //enable_swd();
                
                pollCnt = msTickDev->read( (uint8_t*)&pollCnt, 4);
                g_device_init_time = msTickDev->read( (uint8_t*)&g_device_init_time, 4) - g_device_init_time; // 长按情况要减去初始化的时间
                if (g_device_init_time > LONG_PRESS) {
                    g_device_init_time = LONG_PRESS;
                }

                clear_debug_data_info_handle(); // 清调试参数bit位
                update_idl_delay_time();
                set_system_status(IDLE); //setting system status to idle when periphearl init finish, modify by zshe.
                get_wake_up_source(wakeSourceBuf); // 获取按键唤醒源
                sm_log(SM_LOG_NOTICE, "Exit Deep sleep!!!\r\n");
                sm_log(SM_LOG_NOTICE, "g_device_init_time %d!!!\r\n", g_device_init_time);

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
                /* restore ntf_loop_exit_flag*/
                app_bt_char_bt_service_task_ntf_loop_exit_flag_set(0);
                
                /* 唤醒时，通知一下 bt任务，检测是否有NTF 信息上报 - add by vincent.he*/
                TaskHandle_t *temp_handle;
                temp_handle = get_task_bt_handle();
                xTaskNotifyGive(*temp_handle);

				event_record_generate(EVENT_CODE_WAKE_UP, event_data, 0);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
                keyDev->read( (uint8_t*)&buf, 3);
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				if (wakeSourceBuf[0] == 1 && buf[1] == 0 && get_shipping_mode() == 0)	// 按键按下唤醒，但执行到该位置已经弹起
				{
					event_data[0] = EVENT_CODE_KEY2;
					event_record_generate(EVENT_CODE_BUTTON_PRESS, event_data, 1);

					if (bFindMe_WaitKeyRelease == 0)
					{
						g_keyPressCnt_t.bChecking = true;
						g_keyPressCnt_t.timeTick = msTickDev->read(NULL, 4);
						if (g_keyPressCnt_t.cnt1)
						{
							g_keyPressCnt_t.cnt1 = 0;
							g_keyPressCnt_t.cnt2 = 0;
						}
						if (g_keyPressCnt_t.cnt2 < 0xff)
						{
							g_keyPressCnt_t.cnt2++;
						}
					}
				}

				if (wakeSourceBuf[1] == 1 && buf[0] == 0 && get_shipping_mode() == 0)	// 按键按下唤醒，但执行到该位置已经弹起
				{
					event_data[0] = EVENT_CODE_KEY1;
					event_record_generate(EVENT_CODE_BUTTON_PRESS, event_data, 1);
					
					if (bFindMe_WaitKeyRelease == 0)
					{
						g_keyPressCnt_t.bChecking = true;
						g_keyPressCnt_t.timeTick = msTickDev->read(NULL, 4);
						if (g_keyPressCnt_t.cnt2)
						{
							g_keyPressCnt_t.cnt1 = 0;
							g_keyPressCnt_t.cnt2 = 0;
						}
						if (g_keyPressCnt_t.cnt1 < 0xff)
						{
							g_keyPressCnt_t.cnt1++;
						}
					}
				}

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

                if (wakeSourceBuf[3] == 1) { // 两个中断同时触发 不响应

                } else if ((wakeSourceBuf[0] == 1 || wakeSourceBuf[1] == 1) && (buf[0] == 0 && buf[1] == 0)) { // 唤醒源是按键，并且按键已弹起,需要显示电量
                    set_system_external_even(EXT_EVENT_KEY_STANDARD_SHORT_PRESS);
                    g_device_init_time = 0;
                    g_multiplesShortPressTicks = msTickDev->read( (uint8_t*)&g_multiplesShortPressTicks, 4);
                    g_multiplesShortPressCnt = 1; // 算短按一次
                } else if (buf[2] == 1) { // USB 插入唤醒
                    g_device_init_time = 0;
                }
            } else { // USB插入情况无法进入ship mode
                sm_log(SM_LOG_NOTICE, "enter shipping mode!!!\r\n");
                // 保存错误码数据
                flash_data_save_change(1);		// 时间戳不需要保存
                if (get_shipping_mode() == 0) { // 低压情况要设置船运
                    set_shipping_mode(1); // 0:正常模式，1：船运模式
                }
                // 解决屏闪问题
                driver_amoled_deinit();

                procotol_fuelGauge_enter_shutdown();
                procotol_enter_transport_mode();
                vTaskDelay(25); // 等待shipmode生效
            }
        }
    } else {
        update_idl_delay_time();
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		bFirstIdel = true;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)	
    }
    if (get_user_reset_status() == true) { // 用户复位，等待

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//		set_time_stamp_to_flash();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		// flash_data_save_change(0);		// 带时间戳 bug1979300
        vTaskDelay(100); // 等待返回数据给上位机完成
        set_user_reset_status(false);
        //复位系统
        NVIC_SystemReset();
    }

    switch (bleStatus) {
        case 0:
            if (true == BleCongested_Read() && UI_NONE == get_current_ui_detail_status()) {
                mstick = msTickDev->read( (uint8_t*)&mstick, 4);
                bleStatus++;
            }
            break;
        case 1:
            if (true == BleCongested_Read() && UI_NONE == get_current_ui_detail_status()) {
                if (msTickDev->read( (uint8_t*)&mstick, 4) > (mstick + 1000)) { // 1秒UI_NONE滤波
                    flash_data_save_change(0);      // 带时间戳
                    NVIC_SystemReset();//复位系统 FOR TEST
                }
            } else {
                bleStatus = 0;
            }
            break;
        default: bleStatus = 0; break;
    }
}

/**
  * @brief  监控数据更新
  * @param  None
  * @return None
  * @note   None
  */
extern void set_pd_proc_start(uint8_t sta);
static MonitorDataInfo_t p_tMontorDataInfoTmp;

void data_monitor_pro(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);
    ptIoDev batDev = io_dev_get_dev(DEV_FULE_GAUGE); // 电量计 每秒更新一次 ， 其他的每20ms更新一次
    ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC); // chg <100ms更新一次 ， 其他的每20ms更新一次
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
    MonitorDataInfo_t *p_debugDataInfo = get_debug_data_info_handle();
    int16_t *p_iniVal = get_ini_val_info_handle();

    static uint32_t ms20Ticks = 0;
    static uint32_t ms60Ticks = 0;
    static uint32_t ms100Ticks = 0;
    static uint8_t status = 0;
    static uint8_t chgErrCheck = 0;
    static uint8_t fgErrCheck = 0;
    static uint8_t fgInitCnt = 0; // 初始化电量计
    static uint8_t fgState = 0;
    static uint8_t cnt = 0;
    int16_t temp = 0;
    uint8_t i=0;

    SysStatus_u sysStatus = get_system_status();
    static SysStatus_u oldSysStaus = IDLE;
    if (g_monitorDataWakeUpStart) {
        status = 0;
        g_monitorDataWakeUpStart = 0;
    }
    if(oldSysStaus != sysStatus)
    {
        oldSysStaus = sysStatus;
        if(sysStatus == HEATTING_BOOST || sysStatus == HEATTING_STANDARD || sysStatus == HEATTING_CLEAN)
        {
            adcDev->read((uint8_t *)&(p_tMontorDataInfo->det), sizeof(DetectorInfo_t));
            HEATER *heaterInfo = get_heat_manager_info_handle();
            memcpy(&p_tMontorDataInfo->heaterInfo, heaterInfo, sizeof(HEATER));   
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			update_lifeCycle_Coil_Temp((uint16_t)p_tMontorDataInfo->det.heat_K_temp);

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)	
        }  
    }
    switch (status) {
        case 0:
            ms20Ticks = msTickDev->read( (uint8_t*)&ms20Ticks, 4);
            ms60Ticks = msTickDev->read( (uint8_t*)&ms60Ticks, 4);
            ms100Ticks = msTickDev->read( (uint8_t*)&ms100Ticks, 4);
            // 优先更新一次数据
            adcDev->read((uint8_t *)&(p_tMontorDataInfo->det), sizeof(DetectorInfo_t));
            HEATER *heaterInfo = get_heat_manager_info_handle();
            memcpy(&p_tMontorDataInfo->heaterInfo, heaterInfo, sizeof(HEATER));
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			update_lifeCycle_Coil_Temp((uint16_t)p_tMontorDataInfo->det.heat_K_temp);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)	
			p_tMontorDataInfo->session = (uint16_t)get_sessions_total(); // 更新session数
			for (i=0; i<6; i++) { // 统一刷新外设模块数据
				if (i < 2) {
					chgDev->read((uint8_t *)&(p_tMontorDataInfo->chg), sizeof(ChgInfo_t));
				}
				batDev->read((uint8_t *)&(p_tMontorDataInfo->bat), sizeof(BatInfo_t));
			}
			memcpy(&p_tMontorDataInfoTmp, p_tMontorDataInfo, sizeof(MonitorDataInfo_t)); // 初始化全局数据
			fgState = 0;
			set_pd_proc_start(0); // dev wakeup or reset, clear
            status++;
            break;

        case 1:
            if (msTickDev->read( (uint8_t*)&ms20Ticks, 4) > (ms20Ticks + 20)) { // 读取其他参数
                ms20Ticks = msTickDev->read((uint8_t*)&ms20Ticks, 4);
                adcDev->read((uint8_t *)&(p_tMontorDataInfoTmp.det), sizeof(DetectorInfo_t));
                HEATER *heaterInfo = get_heat_manager_info_handle();
                memcpy(&p_tMontorDataInfo->heaterInfo, heaterInfo, sizeof(HEATER));
                /********************* 软件模拟测试代码 **********************/
                if (p_debugDataInfo->dbgBit & DEBUG_TPCBA) {
                	p_tMontorDataInfoTmp.det.heat_K_cood_temp = p_debugDataInfo->det.heat_K_cood_temp;
                }
                if (p_debugDataInfo->dbgBit & DEBUG_TUSB) {
                	p_tMontorDataInfoTmp.det.usb_port_temp = p_debugDataInfo->det.usb_port_temp;
                }
    			if (p_debugDataInfo->dbgBit & DEBUG_THEAT) {
    				p_tMontorDataInfoTmp.det.heat_K_temp = p_debugDataInfo->det.heat_K_temp;
                }
    			memcpy(&p_tMontorDataInfo->det, &p_tMontorDataInfoTmp.det, sizeof(DetectorInfo_t));

				if (p_debugDataInfo->dbgBit & DEBUG_SESSION) {
	            	p_tMontorDataInfo->session = p_debugDataInfo->session;
	            } else {
					p_tMontorDataInfo->session = (uint16_t)get_sessions_total(); // 更新session数
	            }
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				update_lifeCycle_Coil_Temp((uint16_t)p_tMontorDataInfoTmp.det.heat_K_temp);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)	                
				/*******************************************************/
            }

			/* 60ms*2刷新一次数据，充电IC开了4路adc，每路转换时间约24ms; 刷新太慢影响死电池波形判断 */
            if (msTickDev->read((uint8_t*)&ms60Ticks, 4) > (ms60Ticks + 60)) {
                ms60Ticks = msTickDev->read((uint8_t*)&ms60Ticks, 4);
                if (RET_SUCCESS == chgDev->read((uint8_t *)&(p_tMontorDataInfoTmp.chg), sizeof(ChgInfo_t))) {
                	/********************* 软件模拟测试代码 **********************/
                    if (p_debugDataInfo->dbgBit & DEBUG_VBAT) {
                        p_tMontorDataInfoTmp.chg.bat_volt = p_debugDataInfo->chg.bat_volt;
                    }
                    if (p_debugDataInfo->dbgBit & DEBUG_IBAT) {
                        p_tMontorDataInfoTmp.chg.bat_curr = p_debugDataInfo->chg.bat_curr;
                    }
                    if (p_debugDataInfo->dbgBit & DEBUG_VBUS) {
                        p_tMontorDataInfoTmp.chg.bus_volt = p_debugDataInfo->chg.bus_volt;
                    }
                    if (p_debugDataInfo->dbgBit & DEBUG_IBUS) { // no use
                        p_tMontorDataInfoTmp.chg.bus_curr = p_debugDataInfo->chg.bus_curr;
                    }
                    memcpy(&p_tMontorDataInfo->chg, &p_tMontorDataInfoTmp.chg, sizeof(ChgInfo_t));
                    /*上位机获取寄存器信息，只执行一次*/
					charge_debug_log();
                    /*******************************************************/
					if (chgErrCheck != 0) {
						chgErrCheck = 0;
					}
                } else { // charger IIC abnormal check
                	if (chgErrCheck <= 20) {
                		chgErrCheck++;
                	}
                	if (chgErrCheck >= 20) {
                		chgErrCheck = 0;
						add_error_even(FLT_DE_CIC_CONFIG_ERROR); // IIC总线异常，从设备失控
					}
                }
            }

            /* 100ms * 6 刷新一次电量计数据 */
			if (msTickDev->read((uint8_t*)&ms100Ticks, 4) > (ms100Ticks + 100)) {
				ms100Ticks = msTickDev->read((uint8_t*)&ms100Ticks, 4);
				switch (fgState) {
				case 0:
					/* USB input 并且 BatVolt < 2.5V */
					if ((p_tMontorDataInfo->chg.reg20_state & 0x80) &&
						(p_tMontorDataInfo->chg.bat_volt <= PROT_BAT_LOW_VOLT_THRESHOLD)) {
						fgInitCnt = 0;
						fgErrCheck = 0;
						cnt = 0;
						fgState = 1;
						break;
					}
					/* 初始化失败 */
					if (p_tMontorDataInfo->bat.init == 0) {
						fgInitCnt = 0;
						fgErrCheck = 0;
						cnt = 0;
						fgState = 1;
						sm_log(SM_LOG_DEBUG, "FG init:%d\r\n", p_tMontorDataInfo->bat.init);
						break;
					}
					/* 等待船运模式完全退出 */
					if (get_shipping_mode() != 0) {
	        			for (i=0; i<3; i++) {
	        				batDev->read((uint8_t *)&(p_tMontorDataInfoTmp.bat), sizeof(BatInfo_t)); // 提高刷新频率
	        			}
						//break;
					}
					/* 读取电量计数据 */
					if (RET_SUCCESS == batDev->read((uint8_t *)&(p_tMontorDataInfoTmp.bat), sizeof(BatInfo_t))) {
						if (true == check_full_remap_soc()) {
							p_tMontorDataInfoTmp.bat.remap_soc = 100;
						}
						/*******************************************************/
						#if 1 // 在充电中，防止SOC反跳
						static uint16_t batSocTmp = 0;
						if (sysStatus == CHARGING) {
							if (p_tMontorDataInfoTmp.bat.remap_soc >= batSocTmp) {
								batSocTmp = p_tMontorDataInfoTmp.bat.remap_soc;
							} else {
								p_tMontorDataInfoTmp.bat.remap_soc = batSocTmp;
							}
						} else {
							batSocTmp = p_tMontorDataInfoTmp.bat.remap_soc;
						}
						#endif
						/*******************************************************/
						if (fgErrCheck != 0) {
							fgErrCheck = 0;
						}
					} else {
						/* 读数据失败，暂使用ADC和充电IC数据 */
						p_tMontorDataInfoTmp.bat.remap_soc = p_tMontorDataInfo->bat.remap_soc;
						p_tMontorDataInfoTmp.bat.voltage = p_tMontorDataInfo->chg.bat_volt;
						p_tMontorDataInfoTmp.bat.current = p_tMontorDataInfo->chg.bat_curr;
						p_tMontorDataInfoTmp.bat.temperature = p_tMontorDataInfo->bat.temperature;
						temp = (int16_t)(p_tMontorDataInfo->det.heat_K_cood_temp + p_tMontorDataInfo->det.usb_port_temp)>>1; //使用融合温度
						if (p_tMontorDataInfoTmp.bat.temperature > temp) {
							p_tMontorDataInfoTmp.bat.temperature--;
						} else {
							p_tMontorDataInfoTmp.bat.temperature = temp;
						}

                		fgErrCheck++;
	                	if (fgErrCheck >= 20) {
	                		fgErrCheck = 0;
							//add_error_even(FLT_DE_CIC_CONFIG_ERROR); // IIC总线异常，从设备失控
							cnt = 0;
							fgState = 1;
						}
					}
					break;
				case 1:
				 	/* 初始化失败或低电过，电压到了锂保完全恢复，重新初始化电量计 */
					if ((p_tMontorDataInfo->chg.bat_volt > (PROT_BAT_LOW_VOLT_THRESHOLD + 500)) &&
						(FLT_DE_BAT_DAMAGE != find_error_even(FLT_DE_BAT_DAMAGE))) {
						if (fgInitCnt <= 30) {fgInitCnt++;}
					} else {
						if (fgInitCnt >  0) {fgInitCnt--;}
					}

					if (fgInitCnt >= 30) { // 超时3s，重新初始化电量计
						fgInitCnt = 0;
						uint8_t buf[1] = {FG_SET_INIT};
						if (RET_SUCCESS == batDev->write((uint8_t *)&(buf), sizeof(buf))) {
							for (i=0; i<6; i++) { // 统一刷新外设模块数据
								batDev->read((uint8_t *)&(p_tMontorDataInfo->bat), sizeof(BatInfo_t));
							}
							p_tMontorDataInfo->bat.init++;
                            p_tMontorDataInfoTmp.bat.init = p_tMontorDataInfo->bat.init; // 更新初始化标志
							sm_log(SM_LOG_DEBUG, "FG reinit:%d\r\n", p_tMontorDataInfo->bat.init);
							fgErrCheck = 0;
							fgState = 0;
							break;
						} else {
	                		fgErrCheck++;
		                	if (fgErrCheck >= 10) { // 初始化一直失败，IIC总线异常，从设备失控
		                		fgErrCheck = 0;
								add_error_even(FLT_DE_CIC_CONFIG_ERROR);
							}
						}
					}

					cnt++;
					if (cnt >= 6) { // 保证bat数据刷新周期不变
						cnt = 0;
						/* 电量计主要数据暂使用ADC和充电IC数据 */
						p_tMontorDataInfoTmp.bat.remap_soc = p_tMontorDataInfo->bat.remap_soc;
						p_tMontorDataInfoTmp.bat.voltage = p_tMontorDataInfo->chg.bat_volt;
						p_tMontorDataInfoTmp.bat.current = p_tMontorDataInfo->chg.bat_curr;
						p_tMontorDataInfoTmp.bat.temperature = p_tMontorDataInfo->bat.temperature;
						temp = (int16_t)(p_tMontorDataInfo->det.heat_K_cood_temp + p_tMontorDataInfo->det.usb_port_temp)>>1;
						if (p_tMontorDataInfoTmp.bat.temperature > temp) {
							p_tMontorDataInfoTmp.bat.temperature--;
						} else {
							p_tMontorDataInfoTmp.bat.temperature = temp; // 使用融合温度
						}
					}
					break;
				default:
					fgState = 0;
					break;
				}
	            /********************* 软件模拟测试代码 **********************/
	            if (p_debugDataInfo->dbgBit & DEBUG_SOC) {
	                p_tMontorDataInfoTmp.bat.remap_soc = p_debugDataInfo->bat.remap_soc;
	            }
	            if (p_debugDataInfo->dbgBit & DEBUG_VBAT) {
	                p_tMontorDataInfoTmp.bat.voltage = p_debugDataInfo->bat.voltage;
	            }
	            if (p_debugDataInfo->dbgBit & DEBUG_IBAT) {
	                p_tMontorDataInfoTmp.bat.current = p_debugDataInfo->bat.current;
	            }
	            if (p_debugDataInfo->dbgBit & DEBUG_TBAT) {
	                p_tMontorDataInfoTmp.bat.temperature = p_debugDataInfo->bat.temperature;
	            }
	            memcpy(&p_tMontorDataInfo->bat, &p_tMontorDataInfoTmp.bat, sizeof(BatInfo_t));
				/*******************************************************/
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
					static uint8_t batSoc = 0xfe;
					uint8_t event_data[1];
					if (get_shipping_mode() == 0)			// bug 1984609
					{
						if (batSoc != p_tMontorDataInfo->bat.remap_soc)
						{
							batSoc = p_tMontorDataInfo->bat.remap_soc;
							event_data[0] = batSoc;
							event_record_generate(EVENT_CODE_BATTERY_LEVEL, event_data, 1);							
						}
					}
// 检测LifeCycle
					LifeCycle_t *p_tLifeCycleHandle = get_life_cycle_handle();
					uint16_t u16Temp;
					int16_t i16Temp;
// bat voltage
					u16Temp = p_tMontorDataInfo->bat.voltage;
					if (u16Temp > p_tLifeCycleHandle->batVolMax)
					{
						p_tLifeCycleHandle->batVolMax = u16Temp;
						set_update_lifecycle_status(1);
					}
					if (u16Temp < p_tLifeCycleHandle->batVolMin)
					{
						p_tLifeCycleHandle->batVolMin = u16Temp;
						set_update_lifecycle_status(1);
					}
// bat temperature
					i16Temp = p_tMontorDataInfo->bat.temperature;
					if (i16Temp > p_tLifeCycleHandle->batTempMax)
					{
						p_tLifeCycleHandle->batTempMax = i16Temp;
						set_update_lifecycle_status(1);
					}
					if (i16Temp < p_tLifeCycleHandle->batTempMin)
					{
						p_tLifeCycleHandle->batTempMin = i16Temp;
						set_update_lifecycle_status(1);
					}
// bat current
					i16Temp = p_tMontorDataInfo->bat.current;
					if (i16Temp > p_tLifeCycleHandle->batChargeCurrentMax)
					{
						p_tLifeCycleHandle->batChargeCurrentMax = i16Temp;
						set_update_lifecycle_status(1);
					}
					if (i16Temp < p_tLifeCycleHandle->batDischargeCurrentMax)
					{
						p_tLifeCycleHandle->batDischargeCurrentMax = i16Temp;
						set_update_lifecycle_status(1);
					}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)			
			}
            break;
        default:
            status = 0;
        break;
    }
    //加热相关

    //PM相关
    
    //判断错误码 更新UI、以及系统状态
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    //电池电量动态检测(ble notification feature)
    app_bt_service_char_ntf_monitor();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    
//    IDLE -> HEAT   BREFORE() -> GIVE
}
void update_log_pollCnt(void)
{
    pollCnt = msTickDev->read( (uint8_t*)&pollCnt, 4);
}

uint8_t g_pdStartProc = 0;
void set_pd_proc_start(uint8_t sta)
{
	g_pdStartProc = sta;
}

uint8_t g_fgSaveData = 0;
void fg_save_data(uint8_t sta)
{
	g_fgSaveData = sta;
}

void pd_proc(void) {
	ptIoDev pdDev = io_dev_get_dev(DEV_PD_PHY);
	static uint8_t pdBuf[1] = {PD_SET_PROC};
	static uint8_t checkCnt = 0;
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
	// if the fuel gauge is in SLEEP mode, clear the pd process flag
	if ((p_tMontorDataInfo->bat.ctrl_status & BQ27426_STATUS_SLEEP) || (g_fgSaveData != 0)) {
		g_pdStartProc = 0; // 屏蔽PD任务，以免影响电量计唤醒
		checkCnt = 0;
	} else {
		if (checkCnt <= 250) {
			checkCnt++;
		}
		if (checkCnt == 250) { // 延时0.5s恢复PD任务处理 (系统周期2ms)
			g_pdStartProc = 1;
			sm_log(SM_LOG_INFO, "Recover pd proc\r\n");
		}
	}

	if (g_pdStartProc != 0) {
		pdDev->write((uint8_t *)&(pdBuf), 1);
	}
}
/**
  * @brief  system服务任务处理函数
  * @param  pvParam:    任务入参
  * @return None
  * @note   None
  */
void task_system_service(void *pvParam)
{
    msTickDev = io_dev_get_dev(DEV_MS_TICK);//SYS TICK 
    pollCnt = msTickDev->read( (uint8_t*)&pollCnt, 4);
    while(true)
    {
       // pollCnt++;
        // 1. 添加按键扫描，按键事件消息发送; 添加USB插拔检测，USB插拔事件消息发送
        key_usb_scan_process();
        base_key_press_time_calculator(); // BUG 1934401
        boost_key_press_time_calculator(); // BUG 1934401
        // 2. 检测串口是否接收到数据， 如有收到数据，向PCtool任务发送消息
       // get_usb_rec_data();
        // 3. 添加待机判断
        // 4. 增加看门狗喂狗
        // detector_idle_scan();
        // 震动马达 进程
        // motor_proc2();

        // 蜂鸣器 进程
        beep_proc();
        pd_proc();
        if(gs_logTimer!= 0 && get_update_ui_timer_cnt() == 0 && get_rdp_on_mode_no_save_flash() != 1) // 升级UI和读取INI时不上传日志
        {
            uint32_t tmpTick = msTickDev->read( (uint8_t*)&tmpTick, 4);
            if(tmpTick > pollCnt && (tmpTick - pollCnt >= gs_logTimer) ) // 需要把任务切换和vTaskDelay算上,
            {
                tmpTick = tmpTick - pollCnt;
                if (tmpTick > gs_logTimer && tmpTick < (2 * gs_logTimer)) {
                    tmpTick = tmpTick - gs_logTimer;
                } else {
                    tmpTick = 0;
                }
                pollCnt = msTickDev->read( (uint8_t*)&pollCnt, 4);
                if (pollCnt >= tmpTick) {
                    pollCnt = pollCnt - tmpTick;  // 执行减法后，下次if判断需要tmpTick > pollCnt在进行定时判断
                }
                xTaskNotify(get_task_usb(),USB_LOG,eSetBits);//usb的LOG
            }
        } else {
            pollCnt = msTickDev->read( (uint8_t*)&pollCnt, 4);
        }
        data_monitor_pro();
        system_error_recover_proc(); // 先恢复，防止先报错后恢复导致多报错一次
        system_interaction_logic_proc();
        sys_wdg_pro();
        lo_power_proc();
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		session_record_sync_proc();
		event_record_sync_proc();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
        vTaskDelay(2);
    }	
}

