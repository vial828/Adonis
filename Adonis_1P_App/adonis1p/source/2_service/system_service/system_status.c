/**
  ******************************************************************************
  * @file    system_status.c
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

#include "system_status.h"
#include "public_typedef.h"
#include "platform_io.h"
#include "data_base_info.h"
#include "err_code.h"

volatile SysStatus_u g_usysStaus = 0;
static volatile uint32_t idl_runTimeMs = 0;
static volatile uint8_t gs_uiDisplayStatus = 0;
volatile bool g_bUserSysReset = false;
#define SECONDS_4    2000
#define MS_100    50

//static volatile bool gs_cleanStatus = false;

/**
  * @brief  获取系统状态
  * @param  None
  * @return 返回系统状态
  * @note   None
  */
SysStatus_u get_system_status(void)
{
    return g_usysStaus;
}

/**
  * @brief  设置系统状态
  * @param  usysStatus： 要设定的系统状态
  * @return None
  * @note   None
  */
void set_system_status(SysStatus_u usysStatus)
{
    g_usysStaus = usysStatus;
    sm_log(SM_LOG_INFO, "set_system_status! %d\r\n", g_usysStaus);
}

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
volatile SysSubStatus_u g_subSysStaus = 0;

SysSubStatus_u get_subSystem_status(void)
{
    return g_subSysStaus;
}

void set_subSystem_status(SysSubStatus_u status)
{
    g_subSysStaus = status;
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

volatile SysHeatType_e g_eSysHeatType = HEAT_BY_KEY;

void set_heat_type(SysHeatType_e heatType)
{
    g_eSysHeatType = heatType;
}

SysHeatType_e get_heat_type(void)
{
    return g_eSysHeatType;
}
#if 0
bool procotol_get_charge_sta(uint8_t* rValue)
{
    if(g_usysStaus == CHARGING)
    {
        rValue[0]=0x01;//充电状态
    }else{
        rValue[0]=0x00;//非充电状态，
    }
    return true;
}
#endif

bool get_user_reset_status(void)
{
    return g_bUserSysReset;
}

void set_user_reset_status(bool status)
{
    g_bUserSysReset = status;
}

/**
  * @brief  获取系统信息
  * @param  pBuf：要更新的入参：RTC时间+系统状态+蓝牙状态+错误码
  * @return true: 更新成功，false:更新失败
  * @note   更新参数后期如需增加，在参数后追加即可
  */
bool procotol_sys_info_get(uint8_t* pBuf)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    uint32_t msTicks = msTickDev->read( (uint8_t*)&msTicks, 4);
    uint8_t sysStatus = (uint8_t)get_system_status();
    UiInfo_t *pUiInfo = get_error_high_pro_even();
    uint16_t errCode = 0;
    // 系统时间
    memcpy(pBuf, (uint8_t *)&msTicks, 4);
    // 系统状态,需要转换状态， 与上位机统一，与pen case统一
    switch (sysStatus) {
        case IDLE:
            sysStatus = 0;
            break;
        case SLEEP:
            sysStatus = 0;
            break;
        case HEATTING_TEST:
            sysStatus = 0;
            break;
        case HEATTING_STANDARD:
            sysStatus = 1;
            break;
        case HEATTING_BOOST:
            sysStatus = 1;
            break;
        case HEATTING_CLEAN:
            sysStatus = 1;
            break;
        case HEAT_MODE_TEST_VOLTAGE:
            sysStatus = 0;
            break;
        case HEAT_MODE_TEST_POWER:
            sysStatus = 0;
            break;
        case HEAT_MODE_TEST_TEMP:
            sysStatus = 0;
            break;
        case CHARGING:
            sysStatus = 2;
            break;
        case CHIPPING_MODE_EXITTING:
            sysStatus = 0;
            break;
        default: break;
    }
    pBuf[4] = sysStatus;
    // 蓝牙状态
    pBuf[5] = bt_get_adv_en();
    // 当前错误码
    errCode = (uint16_t)pUiInfo->even;
    memcpy(&pBuf[6], (uint8_t *)&errCode, 2);
    return true;
}

void update_idl_delay_time(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    idl_runTimeMs = msTickDev->read( (uint8_t*)&idl_runTimeMs, 4);
}

uint32_t get_idl_delay_time(void)
{
    return idl_runTimeMs;
}

void set_ui_display_status(uint8_t status)
{
    gs_uiDisplayStatus = status;
}

uint8_t get_ui_display_status(void)
{
    return gs_uiDisplayStatus;
}


volatile uint32_t g_taskBtHeartBeatTicks = 0; //task bt心跳记录，0：挂起，存活则累加, 异常则值非零并且不变

volatile uint32_t g_taskBtHeartBeatBackupTicks = 0; //task bt心跳记录备份

uint32_t get_task_bt_heartbeat_backup_ticks(void)
{
    return g_taskBtHeartBeatBackupTicks;
}

void set_task_bt_heartbeat_backup_ticks(uint32_t ticks)
{
    g_taskBtHeartBeatBackupTicks = ticks;
}

uint32_t get_task_bt_heartbeat_ticks(void)
{
    return g_taskBtHeartBeatTicks;
}

void set_task_bt_heartbeat_ticks(void)
{
    g_taskBtHeartBeatTicks++;
}

void clr_task_bt_heartbeat_ticks(void)
{
    g_taskBtHeartBeatTicks = 0;
}


volatile uint32_t g_taskChargeHeartBeatTicks = 0; //task charge心跳记录，0：挂起，存活则累加, 异常则值非零并且不变

volatile uint32_t g_taskChargeHeartBeatBackupTicks = 0; //task charge心跳记录备份

uint32_t get_task_charge_heartbeat_backup_ticks(void)
{
    return g_taskChargeHeartBeatBackupTicks;
}

void set_task_charge_heartbeat_backup_ticks(uint32_t ticks)
{
    g_taskChargeHeartBeatBackupTicks = ticks;
}

uint32_t get_task_charge_heartbeat_ticks(void)
{
    return g_taskChargeHeartBeatTicks;
}

void set_task_charge_heartbeat_ticks(void)
{
    g_taskChargeHeartBeatTicks++;
}

void clr_task_charge_heartbeat_ticks(void)
{
    g_taskChargeHeartBeatTicks = 0;
}

volatile uint32_t g_taskHeattingHeartBeatTicks = 0; //task heatting心跳记录，0：挂起，存活则累加, 异常则值非零并且不变

volatile uint32_t g_taskHeattingHeartBeatBackupTicks = 0; //task heatting心跳记录备份

uint32_t get_task_heatting_heartbeat_backup_ticks(void)
{
    return g_taskHeattingHeartBeatBackupTicks;
}

void set_task_heatting_heartbeat_backup_ticks(uint32_t ticks)
{
    g_taskHeattingHeartBeatBackupTicks = ticks;
}

uint32_t get_task_heatting_heartbeat_ticks(void)
{
    return g_taskHeattingHeartBeatTicks;
}

void set_task_heatting_heartbeat_ticks(void)
{
    g_taskHeattingHeartBeatTicks++;
}

void clr_task_heatting_heartbeat_ticks(void)
{
    g_taskHeattingHeartBeatTicks = 0;
}


volatile uint32_t g_taskUiHeartBeatTicks = 0; //task ui心跳记录，0：挂起，存活则累加, 异常则值非零并且不变

volatile uint32_t g_taskUiHeartBeatBackupTicks = 0; //task ui心跳记录备份

uint32_t get_task_ui_heartbeat_backup_ticks(void)
{
    return g_taskUiHeartBeatBackupTicks;
}

void set_task_ui_heartbeat_backup_ticks(uint32_t ticks)
{
    g_taskUiHeartBeatBackupTicks = ticks;
}

uint32_t get_task_ui_heartbeat_ticks(void)
{
    return g_taskUiHeartBeatTicks;
}

void set_task_ui_heartbeat_ticks(void)
{
    g_taskUiHeartBeatTicks++;
}

void clr_task_ui_heartbeat_ticks(void)
{
    g_taskUiHeartBeatTicks = 0;
}


volatile uint32_t g_taskUsbHeartBeatTicks = 0; //task usb心跳记录，0：挂起，存活则累加, 异常则值非零并且不变

volatile uint32_t g_taskUsbHeartBeatBackupTicks = 0; //task usb心跳记录备份

uint32_t get_task_usb_heartbeat_backup_ticks(void)
{
    return g_taskUsbHeartBeatBackupTicks;
}

void set_task_usb_heartbeat_backup_ticks(uint32_t ticks)
{
    g_taskUsbHeartBeatBackupTicks = ticks;
}

uint32_t get_task_usb_heartbeat_ticks(void)
{
    return g_taskUsbHeartBeatTicks;
}

void set_task_usb_heartbeat_ticks(void)
{
    g_taskUsbHeartBeatTicks++;
}

void clr_task_usb_heartbeat_ticks(void)
{
    g_taskUsbHeartBeatTicks = 0;
}

void sys_task_security_ms_delay(uint32_t mstick, EN_TASK_TYPE appTask)
{
    switch (appTask) {
        case TASK_BT:
            set_task_bt_heartbeat_ticks();
            break;
        case TASK_CHARGE:
            set_task_charge_heartbeat_ticks();
            break;
        case TASK_HEATTING:
            set_task_heatting_heartbeat_ticks();
            break;
        case TASK_UI:
            set_task_ui_heartbeat_ticks();
            break;
        case TASK_USB:
            set_task_usb_heartbeat_ticks();
            break;
        default:
            break;
    }
    vTaskDelay(mstick);
}

/**
  * @brief  Watch dog pro
  * @param  None
  * @return None
  * @note   5 seconds no feed will reset sys
  */
void sys_wdg_pro(void)
{
    ptIoDev wdgDev = io_dev_get_dev(DEV_WDG);
    static uint32_t heatingHeatbeatCnt = 0;
    static uint32_t btHeatbeatCnt = 0;
    static uint32_t usbHeatbeatCnt = 0;
    static uint32_t uiHeatbeatCnt = 0;
    static uint32_t chargeHeatbeatCnt = 0;
    int wdgTicks = 0;
    heatingHeatbeatCnt++; 
    btHeatbeatCnt++;
    usbHeatbeatCnt++;
    uiHeatbeatCnt++;
    chargeHeatbeatCnt++;
    // Comparison between Backup Value and Latest Value
    if (get_task_heatting_heartbeat_ticks()!= 0 && get_task_heatting_heartbeat_backup_ticks() == get_task_heatting_heartbeat_ticks()) {
        // The heating task has not updated heartbeat for more than 4 seconds, and there is no prompt printed every 100ms
        if (heatingHeatbeatCnt >= SECONDS_4 && heatingHeatbeatCnt % MS_100 == 0) {
            sm_log(SM_LOG_WARNING, "wdg maybe reset sys, heating task more than 4S no update heartbeat! time:%u\r\n", heatingHeatbeatCnt * 2);
        }
        return;
    } else {
        set_task_heatting_heartbeat_backup_ticks(get_task_heatting_heartbeat_ticks()); // Update backup values
        heatingHeatbeatCnt = 0;
    }
    if (get_task_bt_heartbeat_ticks()!= 0 && get_task_bt_heartbeat_backup_ticks() == get_task_bt_heartbeat_ticks()) {
        if (btHeatbeatCnt >= SECONDS_4 && btHeatbeatCnt % MS_100 == 0) {
            sm_log(SM_LOG_WARNING, "wdg maybe reset sys, bt task more than 4S no update heartbeat! time:%u\r\n", btHeatbeatCnt * 2);
        }
        return;
    } else {
        set_task_bt_heartbeat_backup_ticks(get_task_bt_heartbeat_ticks());
        btHeatbeatCnt = 0;
    }
    if (get_task_usb_heartbeat_ticks()!= 0 && get_task_usb_heartbeat_backup_ticks() == get_task_usb_heartbeat_ticks()) {
        if (usbHeatbeatCnt >= SECONDS_4 && usbHeatbeatCnt % MS_100 == 0) {
            sm_log(SM_LOG_WARNING, "wdg maybe reset sys, usb task more than 4S no update heartbeat! time:%u\r\n", usbHeatbeatCnt * 2);
        }
        return;
    } else {
        set_task_usb_heartbeat_backup_ticks(get_task_usb_heartbeat_ticks());
        usbHeatbeatCnt = 0;
    }
    if (get_task_ui_heartbeat_ticks()!= 0 && get_task_ui_heartbeat_backup_ticks() == get_task_ui_heartbeat_ticks()) {
        if (uiHeatbeatCnt >= SECONDS_4 && uiHeatbeatCnt % MS_100 == 0) {
            sm_log(SM_LOG_WARNING, "wdg maybe reset sys, ui task more than 4S no update heartbeat! time:%u\r\n", uiHeatbeatCnt * 2);
        }
        return;
    } else {
        set_task_ui_heartbeat_backup_ticks(get_task_ui_heartbeat_ticks());
        uiHeatbeatCnt = 0;
    }
    if (get_task_charge_heartbeat_ticks()!= 0 && get_task_charge_heartbeat_backup_ticks() == get_task_charge_heartbeat_ticks()) {
        if (chargeHeatbeatCnt >= SECONDS_4 && chargeHeatbeatCnt % MS_100 == 0) {
            sm_log(SM_LOG_WARNING, "wdg maybe reset sys, charge task more than 4S no update heartbeat! time:%u\r\n", chargeHeatbeatCnt * 2);
        }
        return;
    } else {
        set_task_charge_heartbeat_backup_ticks(get_task_charge_heartbeat_ticks());
        chargeHeatbeatCnt = 0;
    }
    wdgDev->write( (uint8_t*)&wdgTicks, 4); // feed watch dog
}

extern uint32_t get_key1_mstick(void);
extern uint32_t get_key2_mstick(void);
extern uint32_t get_usb_mstick(void);

void sys_start_shipping_mode_pro(void)
{
    ptIoDev wdgDev = io_dev_get_dev(DEV_WDG);
    int wdgTicks = 0;
    uint32_t i = 0;
    if (0 == get_shipping_mode()) {
        return;
    }
    if (0 != get_usb_mstick()) { // USB接入直接退出船运
        return;
    }
    if (0 == get_key1_mstick() && 0 ==get_key2_mstick()) { // 系统起来后没有按键按下，继续船运
        // 解决屏闪问题
        driver_amoled_deinit();
//        uint8_t cfg = CHG_SET_POWER_RST;
//        ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
//        chgDev->write((uint8_t *)&cfg, 1);
        procotol_fuelGauge_enter_shutdown();
        procotol_enter_transport_mode();
        vTaskDelay(25); // 等待shipmode生效
        procotol_fuelGauge_enter_shutdown(); // 重新尝试一次
        procotol_enter_transport_mode();
        vTaskDelay(1000); 
        NVIC_SystemReset();
        while(1);
    }

    if (0 != get_key1_mstick() && 0 !=get_key2_mstick()) {
        // 解决屏闪问题
        driver_amoled_deinit();
//        uint8_t cfg = CHG_SET_POWER_RST;
//        ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
//        chgDev->write((uint8_t *)&cfg, 1);
        procotol_fuelGauge_enter_shutdown();
        procotol_enter_transport_mode();
        vTaskDelay(25); // 等待shipmode生效
        procotol_fuelGauge_enter_shutdown(); // 重新尝试一次
        procotol_enter_transport_mode();
        vTaskDelay(1000); 
        NVIC_SystemReset();
        while(1);
    }

    if (0 == get_key1_mstick()) {
        while (0 !=get_key2_mstick() && 3000 > get_key2_mstick()) {
            vTaskDelay(10);
            wdgDev->write( (uint8_t*)&wdgTicks, 4); // feed watch dog
        }
        if (0 ==get_key2_mstick()) { // 长按时长不足3S
            // 解决屏闪问题
            driver_amoled_deinit();
//            uint8_t cfg = CHG_SET_POWER_RST;
//            ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
//            chgDev->write((uint8_t *)&cfg, 1);
            procotol_fuelGauge_enter_shutdown();
            procotol_enter_transport_mode();
            vTaskDelay(25); // 等待shipmode生效
            procotol_fuelGauge_enter_shutdown(); // 重新尝试一次
            procotol_enter_transport_mode();
            vTaskDelay(1000); 
            NVIC_SystemReset();
            while(1);
        }
        sm_log(SM_LOG_NOTICE, "shipping mode out\r\n");
        return; // 按键时长足够3S，返回
    }

    if (0 == get_key2_mstick()) {
        while (0 !=get_key1_mstick() && 3000 > get_key1_mstick()) {
            wdgDev->write( (uint8_t*)&wdgTicks, 4); // feed watch dog
            vTaskDelay(10);
        }
        if (0 ==get_key1_mstick()) { // 长按时长不足3S
            // 解决屏闪问题
            driver_amoled_deinit();
//            uint8_t cfg = CHG_SET_POWER_RST;
 //           ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
//            chgDev->write((uint8_t *)&cfg, 1);
            procotol_fuelGauge_enter_shutdown();
            procotol_enter_transport_mode();
            vTaskDelay(25); // 等待shipmode生效
            procotol_fuelGauge_enter_shutdown(); // 重新尝试一次
            procotol_enter_transport_mode();
            vTaskDelay(1000);
            NVIC_SystemReset();
            while(1);
        }
        sm_log(SM_LOG_NOTICE, "shipping mode out\r\n");
        return; // 按键时长足够3S，返回
    }
}

