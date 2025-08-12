/**
  ******************************************************************************
  * @file    task_heatting_service.c
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
#include "task_heatting_service.h"
#include "system_status.h"
#include "sm_log.h"
#include <stdio.h>
#include <string.h>
#include "timers.h"
#include "driver_heater.h"
#include "cyhal.h"
#include "driver_detector.h"
#include "system_param.h"

#include "shell_cmd_handle.h"
#include "task_system_service.h"
#include "heat_safety.h"
#include "heat_hot_cold.h"
#include "heat_puff.h"
#include "driver_fuel_gauge.h"
#include "err_code.h"
#include "system_interaction_logic.h"
#include "app_bt_char_adapter.h"
//#include "driver_mcu_eeprom.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "cycfg_gatt_db.h"
#include "event_data_record.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

//-------------全局变量区-----------------//
HEATER stHeatManager;
TaskHandle_t task_heatting_handle;
static DetectorInfo_t heat_t;
ptIoDev msTickDev;
//-------------私有变量区-----------------//
extern uint32_t test_20ms_adc_Convert_cnt;
static uint8_t old_HeatState  = 0;
static uint8_t old_puff = 0;
//-------------------------------------//
/**
  * @brief  获取任务指针
  * @param  None
  * @return 返回指针
  * @note   None
  */
TaskHandle_t* get_task_heatting_handle(void)
{
    return &task_heatting_handle;
}

/**
  * @brief  获取加热信息指针
  * @param  None
  * @return 返回指针
  * @note   None
  */
HEATER* get_heat_manager_info_handle(void)
{
    return &stHeatManager;
}

 /**
   * @brief  设置DCDC输出功率
   * @param  power 需要设置的功率
   * @return 无
   * @note   None
   */
void heat_set_power(float power)
{
    //计算公式：P = U*U/R ---> U = sqrt(P*R)
    if(stHeatManager.powerAdjK > 1.1)stHeatManager.powerAdjK=1.1;
    if(stHeatManager.powerAdjK < 0.8)stHeatManager.powerAdjK=0.8;

    power = power*stHeatManager.powerAdjK + stHeatManager.powerAdjB;
    if(power < 0.0f)power = 0.0f;// 滤掉小于0的设置功率值
    // stHeatManager.SetVotage = sqrt(power*stHeatManager.CurrResister);
    // stHeatManager.SetVotage = sqrt(power/stHeatManager.CurrResister)*(stHeatManager.CurrResister + stHeatManager.nmos_res);//额外加一个5mR的补偿值
    stHeatManager.SetVotage = ((stHeatManager.CurrResister + stHeatManager.nmos_res)/stHeatManager.CurrResister)*sqrt(power*stHeatManager.CurrResister);
    // stHeatManager.SetVotage = sqrt(power/stHeatManager.CurrResister)*(R1+R2);
    // sm_log(SM_LOG_NOTICE, "SetPower:%0.3f,CurrResister:%0.3f,SetVotage:%0.3f\r\n",stHeatManager.PreHeatTime,stHeatManager.CurrResister,stHeatManager.TotalHeatTime);
    hal_tps55288_set_out_v(stHeatManager.SetVotage);
}

// 开启加热  ，计划重新读取功率表 ，
void heat_start(void)
{//获取INI配置开关、数据
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint8_t event_data[1];
	uint8_t sessionMode = get_system_status();
	if (sessionMode == HEATTING_STANDARD)
	{
		event_data[0] = 0;
	}
	else if (sessionMode == HEATTING_BOOST)
	{
		event_data[0] = 1;
	}
#if defined(DEF_DRY_CLEAN_EN)
	else if (sessionMode == HEATTING_CLEAN)
	{
		event_data[0] = 2;
	}
#endif
	else
	{
		event_data[0] = 0xff;
	}
//	event_data[0] = get_system_status() - 3;
	event_record_generate(EVENT_CODE_SESSION_START, event_data, 1);

    ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
    rtcDev->read((uint8_t *)&(stHeatManager.time_stamp), 4);//update timestamp
	stHeatManager.time_stamp_trusted = Get_Rtc_Trusted();
	stHeatManager.bRunning = 1;								// 正在运行

    stHeatManager.HeatMode = get_system_status();
	if (stHeatManager.HeatMode == HEATTING_STANDARD)
	{
		stHeatManager.heatingProfile = stHeatManager.heatingProfile_base;
	}
	else if (stHeatManager.HeatMode == HEATTING_BOOST)
	{
		stHeatManager.heatingProfile = stHeatManager.heatingProfile_boost;
	}
	else
	{
		stHeatManager.heatingProfile = 0xff;				// 末定义曲线
	}
	stHeatManager.z1_max_temp = 0x8000;
	stHeatManager.battery_max_temp = 0x8000;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    HeatTemp_t *P_tHeatTemp = &p_tHeatParamInfo->tHeatBaseTemp[0];
    stHeatManager.PreHeatTime   = 5000;// 单位MS

    stHeatManager.HeatTick      = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4);
    stHeatManager.HeatingTime   = 0;
    stHeatManager.CurrResister  = 0.650f;//初设为0.65R
    stHeatManager.nmos_res = 0.008f;//默认NMOS 内阻是 0.008R
    stHeatManager.HeatMode = get_system_status();
    stHeatManager.HeatPuff = 15;
    stHeatManager.Heating_J = 0.0f;
    // 载入校准值   
    stHeatManager.tempAdjK = p_fdb_b_info->fdb_b1_t.tempAdjK;
    stHeatManager.tempAdjB = (float)p_fdb_b_info->fdb_b1_t.tempAdjB/100;

    stHeatManager.powerAdjK = p_fdb_b_info->fdb_b1_t.powerAdjK;
    stHeatManager.powerAdjB = (float) p_fdb_b_info->fdb_b1_t.powerAdjB/100;

    sm_log2(SM_LOG_NOTICE, "CurAdjK:%0.4f,CuradjB:%dVoltageAdjK:%0.4fVoltageAdjB:%d\r\n",\
            p_fdb_b_info->fdb_b1_t.adcCurrAdjK,\
            p_fdb_b_info->fdb_b1_t.adcCurrAdjB,\
            p_fdb_b_info->fdb_b1_t.adcOutVdjK,\
            p_fdb_b_info->fdb_b1_t.adcOutVdjB);
    // 限幅滤波 
    if(stHeatManager.tempAdjK > 1.200f)stHeatManager.tempAdjK = 1.200f;
    if(stHeatManager.tempAdjK < 0.800f)stHeatManager.tempAdjK = 0.800f;

   // sm_log(SM_LOG_NOTICE, "tempAdjK = %f ,tempAdjB = %0.3f\r\n",stHeatManager.tempAdjK,stHeatManager.tempAdjB);
   // sm_log(SM_LOG_NOTICE, "smokeSuctionSum:%d ,smokeTotalTime:%d S\r\n",p_tHeatParamInfo->smokeSuctionSum,p_tHeatParamInfo->smokeTotalTime);
    switch (stHeatManager.HeatMode)
    {
        case HEATTING_TEST:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatTestTemp[0];
        break;
        case HEATTING_STANDARD:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatBaseTemp[0];
        break;
        case HEATTING_BOOST:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatBoostTemp[0];
        break;
#if defined(DEF_DRY_CLEAN_EN)
        case HEATTING_CLEAN:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatCleanTemp[0];
        break;
#endif
        default:
            break;
    }

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)						// 防止自定义加热曲线后面填0的情况
		uint8_t i = 14;
		while(i)
		{
			if (P_tHeatTemp[i].time != 0 && P_tHeatTemp[i].time != 0xFFFF)
			{
				break;
			}
			i--;
		}
		stHeatManager.TotalHeatTime = P_tHeatTemp[i].time*10; 	// 使用 最后一个非0 值做加热总时间// 单位MS 
#else
        stHeatManager.TotalHeatTime = P_tHeatTemp[14].time*10; // 使用 最后一个 值做加热总时间// 单位MS  
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)	
    
    heat_puff_init();
    old_HeatState  = HEAT_FLOW_STATE_PREHEAT;
    init_pid_param();
    heating_io_init();
    sys_task_security_ms_delay(2, TASK_HEATTING);
    hal_tps55288_init();
    sys_task_security_ms_delay(10, TASK_HEATTING);
    cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN, 1);
    hal_tps55288_set_out_v(0.0);//设置一个初始电压
    hal_tps55288_set_enable(1);
    stHeatManager.StartTime = msTickDev->read( (uint8_t*)&stHeatManager.StartTime, 4);
}
void heat_show_heat_start_log(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    switch (stHeatManager.HeatMode)
    {
        case HEATTING_STANDARD:
            sm_log(SM_LOG_NOTICE, "HEATTING_STANDARD heating... \r\n");
        break;
        case HEATTING_BOOST:
            sm_log(SM_LOG_NOTICE, "HEATTING_BOOST heating... \r\n");
        break;
        case HEATTING_CLEAN:
            sm_log(SM_LOG_NOTICE, "HEATTING_CLEAN heating... \r\n");
        break;
        case HEAT_MODE_TEST_VOLTAGE:
            sm_log(SM_LOG_NOTICE, "HEAT_MODE_TEST_VOLTAGE heating... \r\n");
        break;
        case HEAT_MODE_TEST_POWER:
            sm_log(SM_LOG_NOTICE, "HEAT_MODE_TEST_POWER heating... \r\n");
        break;
        case HEAT_MODE_TEST_TEMP:
            sm_log(SM_LOG_NOTICE, "HEAT_MODE_TEST_TEMP heating... \r\n");
        break;
        default:
            break;
    }
    //log 加热时长
    sm_log(SM_LOG_NOTICE, "preHeatTime:%d,maxHeatTime:%d\r\n",stHeatManager.PreHeatTime,stHeatManager.TotalHeatTime);
    sm_log(SM_LOG_NOTICE, "tempAdjK = %f ,tempAdjB = %0.3f\r\n",stHeatManager.tempAdjK,stHeatManager.tempAdjB);
    sm_log(SM_LOG_NOTICE, "smokeSuctionSum:%d ,smokeBaseTotalTime:%d, smokeBoostTotalTime:%d\r\n",p_tHeatParamInfo->smokeSuctionSum,p_tHeatParamInfo->smokeBaseTotalTime, p_tHeatParamInfo->smokeBoostTotalTime);
}
void heat_test_voltage_start(float testVoltage,uint32_t heatTime)
{
    stHeatManager.PreHeatTime   = 5000;// 单位MS
    stHeatManager.TotalHeatTime = heatTime;// 单位MS
    stHeatManager.HeatTick      = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4);
    stHeatManager.HeatingTime   = 0;
    stHeatManager.CurrResister  = 0.650f;//初设为0.65R
    stHeatManager.nmos_res = 0.008f;//默认NMOS 内阻是 0.008R
    stHeatManager.HeatMode  = HEAT_MODE_TEST_VOLTAGE;
    stHeatManager.SetVotage = testVoltage;
    stHeatManager.bRunning = 1; // 正在运行
    set_system_status(HEAT_MODE_TEST_VOLTAGE);
    motor_set2(HAPTIC_1);
    init_pid_param();
    heating_io_init();
    sys_task_security_ms_delay(2, TASK_USB);
    hal_tps55288_init();
    sys_task_security_ms_delay(10, TASK_USB);
    cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN, 1);
    hal_tps55288_set_out_v(stHeatManager.SetVotage);//设置c测试电压
    hal_tps55288_set_enable(1);
    stHeatManager.StartTime = msTickDev->read( (uint8_t*)&stHeatManager.StartTime, 4);
}
void heat_test_power_start(float testPower,uint32_t heatTime)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    stHeatManager.PreHeatTime   = 5000;// 单位MS
    stHeatManager.TotalHeatTime = heatTime * 10;// 单位MS
    stHeatManager.HeatTick      = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4);
    stHeatManager.HeatingTime   = 0;
    stHeatManager.CurrResister  = 0.650f;//初设为0.65R
    stHeatManager.nmos_res = 0.008f;//默认NMOS 内阻是 0.008R
    stHeatManager.HeatMode  = HEAT_MODE_TEST_POWER;
    stHeatManager.SetPower = testPower;
    // 载入校准值   
    stHeatManager.tempAdjK = p_fdb_b_info->fdb_b1_t.tempAdjK;
    stHeatManager.tempAdjB = (float)p_fdb_b_info->fdb_b1_t.tempAdjB/100;
    stHeatManager.powerAdjK = p_fdb_b_info->fdb_b1_t.powerAdjK;
    stHeatManager.powerAdjB = (float) p_fdb_b_info->fdb_b1_t.powerAdjB/100;

    sm_log2(SM_LOG_NOTICE, "CurAdjK:%0.4f,CuradjB:%dVoltageAdjK:%0.4fVoltageAdjB:%d\r\n",\
        p_fdb_b_info->fdb_b1_t.adcCurrAdjK,\
        p_fdb_b_info->fdb_b1_t.adcCurrAdjB,\
        p_fdb_b_info->fdb_b1_t.adcOutVdjK,\
        p_fdb_b_info->fdb_b1_t.adcOutVdjB);

    // 限幅滤波 
    if(stHeatManager.tempAdjK > 1.200f)stHeatManager.tempAdjK = 1.200f;
    if(stHeatManager.tempAdjK < 0.800f)stHeatManager.tempAdjK = 0.800f;
    stHeatManager.bRunning = 1; // 正在运行
    set_system_status(HEAT_MODE_TEST_POWER);
	//sm_log(SM_LOG_NOTICE, "test testPower:%0.3f,TotalHeatTime:%d\r\n",testPower,stHeatManager.TotalHeatTime);

    motor_set2(HAPTIC_1);
    init_pid_param();
    heating_io_init();
    sys_task_security_ms_delay(2, TASK_USB);
    hal_tps55288_init();
    sys_task_security_ms_delay(10, TASK_USB);
    cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN, 1);
    heat_set_power(stHeatManager.SetPower );
    hal_tps55288_set_enable(1);
    stHeatManager.StartTime = msTickDev->read( (uint8_t*)&stHeatManager.StartTime, 4);
}

void heat_test_temp_start(float testTemp,uint32_t heatTime)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    stHeatManager.PreHeatTime   = 5000;// 单位MS
    stHeatManager.TotalHeatTime = heatTime * 10;// 单位MS
    stHeatManager.HeatTick      = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4);
    stHeatManager.HeatingTime   = 0;
    stHeatManager.CurrResister  = 0.650f;//初设为0.65R
    stHeatManager.nmos_res = 0.008f;//默认NMOS 内阻是 0.008R
    stHeatManager.HeatMode  = HEAT_MODE_TEST_TEMP;
    stHeatManager.CurrTargetTemp = testTemp ;
    // 载入校准值   
    stHeatManager.tempAdjK = p_fdb_b_info->fdb_b1_t.tempAdjK;
    stHeatManager.tempAdjB = (float)p_fdb_b_info->fdb_b1_t.tempAdjB/100;
    stHeatManager.powerAdjK = p_fdb_b_info->fdb_b1_t.powerAdjK;
    stHeatManager.powerAdjB = (float) p_fdb_b_info->fdb_b1_t.powerAdjB/100;
    // 限幅滤波 
    if(stHeatManager.tempAdjK > 1.200f)stHeatManager.tempAdjK = 1.200f;
    if(stHeatManager.tempAdjK < 0.800f)stHeatManager.tempAdjK = 0.800f;
    stHeatManager.bRunning = 1; // 正在运行
    set_system_status(HEAT_MODE_TEST_TEMP);
    motor_set2(HAPTIC_1);
    init_pid_param();
    heating_io_init();
    sys_task_security_ms_delay(2, TASK_USB);
    hal_tps55288_init();
    sys_task_security_ms_delay(10, TASK_USB);
    cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN, 1);
    heat_set_power(0.0f);
    hal_tps55288_set_enable(1);
    stHeatManager.StartTime = msTickDev->read( (uint8_t*)&stHeatManager.StartTime, 4);
}

// 返回1 : 跑完加热时间，0：没有跑完加热时间
uint8_t heat_stop_state_check(uint32_t Heat_Time)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    uint32_t time = 0; //加热段时间
    if(stHeatManager.HeatMode == HEATTING_STANDARD)// base 模式
	{
//		time =  p_tHeatParamInfo->tHeatBaseTemp[14].time*10;
		time = stHeatManager.TotalHeatTime;
	}else if(stHeatManager.HeatMode == HEATTING_BOOST)// base 模式{//boost 延后15个数据
	{
//		time =  p_tHeatParamInfo->tHeatBoostTemp[14].time*10;
		time = stHeatManager.TotalHeatTime;
	}else if(stHeatManager.HeatMode == HEATTING_CLEAN)// base 模式{//boost 延后15个数据
	{
//		time =  p_tHeatParamInfo->tHeatBoostTemp[14].time*10;
		time = stHeatManager.TotalHeatTime;
	}
    sm_log(SM_LOG_NOTICE, "Heat_Time:%d s,Temp[14].time:%d \r\n",Heat_Time,time);
    if(Heat_Time >= time)
    {
        return 1;
    }else{
        return 0;
    }
}

bool g_updateSessionTimeSumStatus = false;
bool get_update_session_time_sum_status(void)
{
    return g_updateSessionTimeSumStatus;
}
void set_update_session_time_sum_status(bool status)
{
    g_updateSessionTimeSumStatus = status;
}

void heat_stop(uint16_t stopType)
{
	if (stHeatManager.bRunning == 0)		// 已经停止
	{
		return;
	}
	stHeatManager.bRunning = 0;
//结束加热
    hal_tps55288_set_out_v(0.0f);
    hal_tps55288_set_enable(0);
    driver_heater_deinit();

    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    SessionTimeSum_v2_t *p_tSessionTimeSum = get_session_time_sum_info_handle();
// 自清洁烟支计算
    if (stHeatManager.HeatMode == HEATTING_STANDARD)
	{
        p_tSessionTimeSum->baseTimeSumClean  += stHeatManager.HeatingTime/1000;// 单位为秒
    } 
	else if (stHeatManager.HeatMode == HEATTING_BOOST)
	{
        p_tSessionTimeSum->boostTimeSumClean  += stHeatManager.HeatingTime/1000;// 单位为秒
    }
	else if (stHeatManager.HeatMode == HEATTING_CLEAN)
	{
		if (heat_stop_state_check(stHeatManager.HeatingTime))			// 完整完成清楚才清零
		{
	        p_tSessionTimeSum->boostTimeSumClean = 0;
			p_tSessionTimeSum->baseTimeSumClean = 0;
		}
    }

    if (stHeatManager.HeatMode == HEATTING_STANDARD) {
        p_tHeatParamInfo->smokeBaseTotalTime  += stHeatManager.HeatingTime/1000;// 单位为秒
    } else {
        p_tHeatParamInfo->smokeBoostTotalTime  += stHeatManager.HeatingTime/1000;// 单位为秒
    }
    p_tHeatParamInfo->smokeSuctionSum += 1; // 总加热次数 
    
    p_tSessionTimeSum->suctionSum = p_tHeatParamInfo->smokeSuctionSum;
    p_tSessionTimeSum->baseTimeSum = p_tHeatParamInfo->smokeBaseTotalTime;
    p_tSessionTimeSum->boostTimeSum = p_tHeatParamInfo->smokeBoostTotalTime;
    set_update_session_time_sum_status(true);

//	if(p_tHeatParamInfo->smokeSuctionSum%10 == 0)
//	{
////		paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
////		paramDev->read( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
////		sm_log(SM_LOG_NOTICE, "flash write param:%d Bytes \r\n",\  
////		p_tHeatParamInfo->smokeTotalTime,\
////		p_tHeatParamInfo->smokeSuctionSum,\
////		 sizeof(HeatParamInfo_t));
//        set_update_session_time_sum_status(false);
//        session_time_sum_update_to_flash();
//	}

    sm_log(SM_LOG_NOTICE, "smokeBaseTotalTime:%d s, smokeBoostTotalTime:%d s, smokeSuctionSum:%d \r\n",\  
        p_tHeatParamInfo->smokeBaseTotalTime,\
        p_tHeatParamInfo->smokeBoostTotalTime,\
        p_tHeatParamInfo->smokeSuctionSum);
    
    if (stopType == 0) { // 正常的停止，振动马达，stopType ==1 错误的停止，应由系统错误处理流程驱动 错误态的马达模式
        motor_set2(HAPTIC_4);
    }


#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
// 检测LifeCycle
	stHeatManager.HeatState = HEAT_FLOW_STATE_NONE;

	LifeCycle_t *p_tLifeCycleHandle = get_life_cycle_handle();
	if (heat_stop_state_check(stHeatManager.HeatingTime))
	{
		(p_tLifeCycleHandle->sessionCompleteCnt)++;
	}
	else
	{
		(p_tLifeCycleHandle->sessionIncompleteCnt)++;
		if (stopType == 0)	{stopType++;}						// 末完成烟支，错误码为1而非0
	}
	set_update_lifecycle_status(1);

	uint8_t event_data[2];
	event_data[1] = stopType;			// 大端传输
	event_data[0] = stopType >> 8;
	event_record_generate(EVENT_CODE_SESSION_STOP, event_data, 2);

	session_service_records_char_t session_records_char;
	session_records_char.count				 = p_tHeatParamInfo->smokeSuctionSum;
	session_records_char.time_stamp 		 = stHeatManager.time_stamp;
	session_records_char.duration			 = stHeatManager.HeatingTime/1000;
	session_records_char.session_exit_code	 = stopType;

	if (stHeatManager.HeatMode == HEATTING_STANDARD)	session_records_char.mode = 0;
	else if (stHeatManager.HeatMode == HEATTING_BOOST)	session_records_char.mode = 1;
	else if (stHeatManager.HeatMode == HEATTING_CLEAN)	session_records_char.mode = 2;		// 增加清洁模式
	else												session_records_char.mode = 0xff;
	
	session_records_char.heatingProfile		 = stHeatManager.heatingProfile;
	session_records_char.z1_max_temp		 = stHeatManager.z1_max_temp;
//	session_records_char.z2_max_temp		 = 0x0000;
	session_records_char.battery_max_temp	 = stHeatManager.battery_max_temp;
	session_records_char.trusted			= stHeatManager.time_stamp_trusted;
	session_record_insert(&session_records_char);
    sm_log(SM_LOG_NOTICE, "incomp:%d, comp: %d \r\n",p_tLifeCycleHandle->sessionIncompleteCnt ,(p_tLifeCycleHandle->sessionCompleteCnt));
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	
}

uint32_t get_sessions_total(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    uint32_t sum = (p_tHeatParamInfo->smokeBaseTotalTime / 315) + (p_tHeatParamInfo->smokeBoostTotalTime / 255);
    return  sum;// 315 is base mode total heatting time
}

uint32_t get_sessions_self_clean_prompt(void)			// 高于20根烟，需要提示清洁
{
    SessionTimeSum_v2_t *p_tSessionTimeSum = get_session_time_sum_info_handle();

    uint32_t sum = (p_tSessionTimeSum->baseTimeSumClean / 315) + (p_tSessionTimeSum->boostTimeSumClean / 255);
    return  sum;// 315 is base mode total heatting time
}

void clr_sessions_total(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//    ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
    p_tHeatParamInfo->smokeBoostTotalTime = 0;// 单位为秒
    p_tHeatParamInfo->smokeBaseTotalTime = 0;// 单位为秒
//    p_tHeatParamInfo->smokeSuctionSum = 0; // 总加热次数 

	SessionTimeSum_v2_t *p_tSessionTimeSum = get_session_time_sum_info_handle();
	p_tSessionTimeSum->baseTimeSum = p_tHeatParamInfo->smokeBoostTotalTime;
	p_tSessionTimeSum->boostTimeSum = p_tHeatParamInfo->smokeBaseTotalTime;
	p_tSessionTimeSum->baseTimeSumClean = 0;
	p_tSessionTimeSum->boostTimeSumClean = 0;
//	app_param_write(INDEX_D_3, (uint8_t *)&g_tSessionTimeSum, sizeof(g_tSessionTimeSum));
	set_update_session_time_sum_status(true);
//    paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
//    paramDev->read( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
}

//输出 0-1000 转换成 0-20瓦 
float range_1000_0_to_power(float pid_in,float p_max)
{
    float re_power = 0.0;
    if(p_max > MAX_POWER )p_max = MAX_POWER;
	if(pid_in<0.0)pid_in=0.0;
    if(pid_in>1000.0)pid_in = 1000.0;
    re_power = pid_in*(p_max/1000.0);
    return re_power;
}
//-------------------------------PID 相关 START-----------------------------------//
str_increase_pid g_increase_pid;    //增量式PID控制句柄
uint8_t g_adj_test_work = 1;
/**
 * @brief      PID parameter init
 * @param[in]  current Heater's thermocouple temperature unit (℃)
 * @param[out] none
 * @retval     none
 * @note   None
*/
void init_pid_param(void)
{
    //初始化PID 参数
    g_increase_pid.ek1 = 0;
    g_increase_pid.ek2 = 0;
    g_increase_pid.ek  = 0;
    
    if(g_adj_test_work == 1){//校准测试模式下的PID 参数
        g_increase_pid.kp1 = 8;//10.5//12.5
        g_increase_pid.ki1 = 0.05;//0.03//0.003;//10.5
        g_increase_pid.kd  = 0.0;
    }else{//正常抽吸模式下的PID 参数
        g_increase_pid.kp1 = 15.5;//10.5//12.5
        g_increase_pid.ki1 = 0.05;//0.03//0.003;//10.5
        g_increase_pid.kd  = 0.0;
    }
 
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);
    adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));  //获取ADC采集数据
    DEV_T en_temp;

    en_temp.bord_ntc_temp  = heat_t.heat_K_cood_temp;// 板子NTC温度 
    en_temp.Heat_k_temp    = heat_t.heat_K_temp ;    // 热电偶温度
    en_temp.TypeC_ntc_temp = heat_t.usb_port_temp;   // 
    stHeatManager.Heat_Hot_K =  hal_get_hot_k(en_temp);

    if(en_temp.bord_ntc_temp < en_temp.TypeC_ntc_temp)
    {
        HalSafetyInit(en_temp.bord_ntc_temp,stHeatManager.HeatMode); 
    }else{
        HalSafetyInit(en_temp.TypeC_ntc_temp,stHeatManager.HeatMode); 
    }
    // stHeatManager.Heat_Hot_K =  1;  // T调试，取消冷热机用 
    sm_log(SM_LOG_NOTICE, "st:%0.3f,HOT: %0.3f \r\n",en_temp.Heat_k_temp ,stHeatManager.Heat_Hot_K);
    // sm_log(SM_LOG_NOTICE, "kP:%0.6f,kI:%0.6f,kD:%0.6f \r\n",g_increase_pid.kp1,g_increase_pid.ki1,g_increase_pid.kd);
}

/**
 * @brief      PID deviation clear
 * @param[in]  none
 * @param[out] none
 * @retval     none
 * @note   None
*/
void fun_pid_ek_clear(float clear_ut)
{
    g_increase_pid.ek1 = 0;
    g_increase_pid.ek2 = 0;
    g_increase_pid.ek  = 0;
    g_increase_pid.ut  = clear_ut * 1000.0 / MAX_POWER;
}
/**
 * @brief      get power with time 
 * @param[in]  time ,uint (ms)
 * @param[out] none
 * @retval     power,uint (Watt)
 * @note   调用此函数时 需要保证冷热机比例值： stHeatManager.Heat_Hot_K 必须已经赋值
*/
float get_time_power(unsigned int time)//time 单位：ms
{
    float Re_power=0.0;
    uint8_t i = 0;
    //从功率表读取功率
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();

    HeatPower_t *P_HeatPower_t = &p_tHeatParamInfo->tHeatBasePower[0];
    time = time/10;
    if(stHeatManager.HeatMode == HEATTING_TEST)// test 模式
    {
    	P_HeatPower_t = &p_tHeatParamInfo->tHeatTestPower[0];
    }else if(stHeatManager.HeatMode == HEATTING_STANDARD)// base 模式
    {
    	P_HeatPower_t = &p_tHeatParamInfo->tHeatBasePower[0];
    }else if(stHeatManager.HeatMode == HEATTING_BOOST)// base 模式{//boost 延后15个数据
    {
    	P_HeatPower_t = &p_tHeatParamInfo->tHeatBoostPower[0];
    }else if(stHeatManager.HeatMode == HEATTING_CLEAN)
    {
        P_HeatPower_t = &p_tHeatParamInfo->tHeatCleanPower[0];
    }


    for(i = 0;i < 12;i++){
        if(time <= P_HeatPower_t[i].time)break;
    }
    Re_power = P_HeatPower_t[i].power/100.0f;
    Re_power = Re_power * 1000.0 / MAX_POWER*stHeatManager.Heat_Hot_K; //转换输出为 0- 1000 20240513 添加冷热机
    return Re_power;
}
/**
 * @brief      get power with time 
 * @param[in]  time ,uint (ms)
 * @param[out] none
 * @retval     power,uint (Watt)
 * @note   None
*/
float get_time_temp(unsigned int time)//time 单位：ms
{

    unsigned int i;
	float rts;
	float t1,t2,T1,T2;
    //从功率表读取功率
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    time = time/10;
    HeatTemp_t *P_tHeatTemp = &p_tHeatParamInfo->tHeatBaseTemp[0];// 默认
    switch (stHeatManager.HeatMode)
    {
        case HEATTING_TEST:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatTestTemp[0];
        break;
        case HEATTING_STANDARD:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatBaseTemp[0];
        break;
        case HEATTING_BOOST:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatBoostTemp[0];
        break;
        case HEATTING_CLEAN:
            P_tHeatTemp = &p_tHeatParamInfo->tHeatCleanTemp[0];
        break;
        default:
            break;
    }
 
    for(i = 0; i < MAX_TEMP_CTRL_POINT - 2; i++)
	{
		if(P_tHeatTemp[i].time <= time
        && P_tHeatTemp[i+1].time > time) break;
	}
    if((MAX_TEMP_CTRL_POINT - 2) == i) i = MAX_TEMP_CTRL_POINT - 2;

    t1 = (float)P_tHeatTemp[i].time;
	t2 = (float)P_tHeatTemp[i+1].time;
	T1 = (float)P_tHeatTemp[i].tempeture;
	T2 = (float)P_tHeatTemp[i+1].tempeture;
	
 
	rts = (((T2 - T1) / (t2 - t1)) * (time - t1) + T1);
	if(rts < 0) rts = 0;

	return rts;
}

/**
 * @brief     Incremental PID calculation
 * @param[in]  target:target temperature; pv:current temperature
 * @param[out] none
 * @retval     0-1000
 * @note   None
*/

float fun_increase_pid_algorithm(float target, float pv)
{
    double dut=0.0,Out=0.0;
	
    g_increase_pid.ek = target - pv;
    dut = g_increase_pid.kp1*(g_increase_pid.ek- g_increase_pid.ek1) \
        + g_increase_pid.ki1* g_increase_pid.ek                      \
        + g_increase_pid.kd* (g_increase_pid.ek - 2*g_increase_pid.ek1 +g_increase_pid.ek2);

    g_increase_pid.ek2 = g_increase_pid.ek1;
    g_increase_pid.ek1 = g_increase_pid.ek;
    g_increase_pid.ut =g_increase_pid.ut + dut;

    Out=g_increase_pid.ut;
    if (Out > 999.9) Out = 999.9;
    if (Out < 0) Out = 0;
	return Out;
}
/**
 * @brief     PID null calculation, error translation
 * @param[in]  target:target temperature; pv:current temperature
 * @param[out] none
 * @retval     none
 * @note   None
*/
void fun_increase_pid_dumy(float target, float pv)
{
    g_increase_pid.ek = target - pv;
    g_increase_pid.ek2 = g_increase_pid.ek1;
    g_increase_pid.ek1 = g_increase_pid.ek;
}

/**
 * @brief      Total entry function of temperature control logic
 * @param[in]  param[0]: target temperature; param[1]:current temperature; param[2]:time 
 * @param[out] none
 * @retval     none
 * @note   12S Power table ctrl and PID temperate ctrl later
*/
float fun_get_algorithm_power(float target, float pv,unsigned int time){
//整合时间、输入和目标
    
     if (time>12*1000 && time < 60*1000){
        if(pv > target - 0.1){
                g_increase_pid.ki1 = 0.05;
            }else {
                g_increase_pid.ki1 = 0.0125;
            }
    }else if (time >60*1000&& time < 120*1000){
            if (pv > target - 0.1){
                g_increase_pid.ki1 = 0.05;
            }else {
                g_increase_pid.ki1 = 0.013;
            }
    }else if (time >120*1000){
            if (pv > target - 0.1){
                g_increase_pid.ki1 = 0.05;
            }else {
                g_increase_pid.ki1 = 0.0135;
            }
    }

    if ( time < 8*1000)
    {
        g_increase_pid.ut  =  get_time_power(time);
        fun_increase_pid_dumy(target,pv);
    }else {
        g_increase_pid.ut  =  fun_increase_pid_algorithm(target,pv);
    }
     
    return g_increase_pid.ut;
}
/**
 * @brief      Total entry function of temperature control logic
 * @param[in]  param[0]: target temperature; param[1]:current temperature; param[2]:time 
 * @param[out] none
 * @retval     none
 * @note    PID temperate ctrl all the time
*/
float fun_get_algorithm_PCBA_test(float target, float pv,unsigned int time)
{
     if (time < 60*1000){
        if(pv > target - 0.1){
                g_increase_pid.ki1 = 0.05;
            }else {
                g_increase_pid.ki1 = 0.0125;
            }
    }else if (time >60*1000&& time < 120*1000){
            if (pv > target - 0.1){
                g_increase_pid.ki1 = 0.05;
            }else {
                g_increase_pid.ki1 = 0.013;
            }
    }else if (time >120*1000){
            if (pv > target - 0.1){
                g_increase_pid.ki1 = 0.05;
            }else {
                g_increase_pid.ki1 = 0.0135;
            }
    }
    g_increase_pid.ut  =  fun_increase_pid_algorithm(target,pv);
    return g_increase_pid.ut;
}
//-------------------------------PID 相关 END-------------------------------------//

static float power_sum = 0.0f;
 
/**
  * @brief  heatting服务任务处理函数
  * @param  pvParam:    任务入参
  * @return None
  * @note   None
  */
void task_heatting_service(void *pvParam)
{
    static uint8_t status = 0;
    SysStatus_u sysStatus = 0;

    static uint32_t log_intv = 0;
    static uint8_t puff_indv = 0;
    int16_t* ini_p = get_ini_val_info_handle();
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);//SYS TICK 
    msTickDev = io_dev_get_dev(DEV_MS_TICK);//SYS TICK 
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
//	LifeCycle_t *p_tLifeCycleHandle = get_life_cycle_handle();
	int16_t i16Temp;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	
   while(true)
   {
       sys_task_security_ms_delay(1, TASK_HEATTING);
       switch (status) {
           case 0: // 空闲状态 等待获取加热 消息，获得消息后，判断加热条件，更新加热所需参数，开启加热
                clr_task_heatting_heartbeat_ticks();
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待任务通知
                set_task_heatting_heartbeat_ticks();
                // 加热前条件判断、开启加热前的保护相关在此添加
                sysStatus = get_system_status();
                if(sysStatus == HEATTING_TEST ||sysStatus == HEATTING_STANDARD || sysStatus == HEATTING_BOOST || sysStatus == HEATTING_CLEAN)// 这些加热都是 带保护机制
                { 
                    uint8_t start_check = safety_check_before_heating();//加热前检查系统信息
                    start_check = 0;
                    if(start_check == 0){
                        heat_start();
                        log_intv = 0;
                        update_log_pollCnt();
                        status = 1;
                    }
                }else{
                    // PCBA_Test 开启加热
                    status = 2;
                }
               // sys_task_security_ms_delay(150, TASK_HEATTING);
               break;
           case 1:// -----------加热循环---------
                if(msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4) - stHeatManager.HeatTick >= 20)//20ms 来临
                {
                    stHeatManager.HeatTick = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4);
                    sysStatus = get_system_status();
                    if(sysStatus == IDLE || sysStatus == SLEEP || sysStatus == CHARGING){//停止加热
                        sm_log(SM_LOG_NOTICE, "\r\nsysStatu trig :heat_stop();");
                        heat_stop(0);
                        status = 0;
                        continue;
                    }
                    stHeatManager.HeatingTime = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4) - stHeatManager.StartTime;
                    adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));//获取ADC采集数据 
                    //加热状态切换判断
                    if(stHeatManager.HeatingTime < stHeatManager.PreHeatTime)
                    {
                        stHeatManager.HeatState = HEAT_FLOW_STATE_PREHEAT;//预热状态
                    } else if(stHeatManager.HeatingTime < (stHeatManager.TotalHeatTime - 30000u))
                    {
                        stHeatManager.HeatState = HEAT_FLOW_STATE_HEAT_NORMAL;
                    } else if(stHeatManager.HeatingTime < stHeatManager.TotalHeatTime)//
                    {
                        stHeatManager.HeatState = HEAT_FLOW_STATE_HEAT_STAGE_LAST_EOS;
                    } else{
                            status = 0;
                            heat_stop(0);
                            sm_log(SM_LOG_NOTICE, "\r\n times up :heat_stop();");
                    }

                    if(old_HeatState != stHeatManager.HeatState)//有状态更新
                    {
                        old_HeatState  = stHeatManager.HeatState;
                        if( stHeatManager.HeatState == HEAT_FLOW_STATE_HEAT_NORMAL &&  sysStatus != HEATTING_CLEAN){// 预热完成 5秒
                            motor_set2(HAPTIC_2);
                        }else if( stHeatManager.HeatState == HEAT_FLOW_STATE_HEAT_STAGE_LAST_EOS ){
                            if(true == get_eos_prompt()) // 如果返回1，需要Eos提醒，就是结束之前30秒 要振动一下马达
                            {
                                motor_set2(HAPTIC_3);
                            }
                        }
                    }
                    //---------------------检测量更新--------------------------
                    stHeatManager.CurrTargetTemp   = get_time_temp(stHeatManager.HeatingTime)/100.0f;
                    stHeatManager.CurrDetectTemp   = heat_t.heat_K_temp;     //max31855_get_temp();
                    stHeatManager.CurrPowerVal     = heat_t.heat_P;
                    stHeatManager.HeatIs1Voltage   = heat_t.heat_V;
                    stHeatManager.DetectCurrent    = heat_t.heat_I;
                    stHeatManager.detectPcbTemp    = heat_t.heat_K_cood_temp   ;
                    stHeatManager.opa_adc_val       = heat_t.opa_adc_val;
                    if(stHeatManager.CurrPowerVal > 2.0f){
                        stHeatManager.CurrResister   = heat_t.heat_R;
                        stHeatManager.nmos_res       = heat_t.nmos_res; 
                    }
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
					if (p_tMontorDataInfo->bat.temperature > stHeatManager.battery_max_temp)
					{
						stHeatManager.battery_max_temp = p_tMontorDataInfo->bat.temperature;
					}

					i16Temp = stHeatManager.CurrDetectTemp; // float 转换
					if (i16Temp > stHeatManager.z1_max_temp)
					{
						stHeatManager.z1_max_temp = i16Temp;
					}
// 检测LifeCycle
//					if (i16Temp > p_tLifeCycleHandle->tempZ1Max)
//					{
//						p_tLifeCycleHandle->tempZ1Max = i16Temp;
//						set_update_lifecycle_status(1);
//					}
//					if (i16Temp < p_tLifeCycleHandle->tempZ1Min)
//					{
//						p_tLifeCycleHandle->tempZ1Min = i16Temp;
//						set_update_lifecycle_status(1);
//					}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
                    //---------------------控温算法逻辑--------------------------
                    float heat_out = 0.0f;
                    heat_out = fun_get_algorithm_power( stHeatManager.CurrTargetTemp*stHeatManager.tempAdjK  + stHeatManager.tempAdjB,\
                                                        stHeatManager.CurrDetectTemp,\
                                                        stHeatManager.HeatingTime);
            
                    stHeatManager.SetPower = range_1000_0_to_power(heat_out,MAX_POWER);//PID输出 0 - 1000 转化为 0 - power_max W
                    stHeatManager.Heating_J += stHeatManager.CurrPowerVal/50;//1焦耳 = 1WS 
                    //---------------------保护算法逻辑--------------------------
                    if(sysStatus == HEATTING_STANDARD || sysStatus == HEATTING_BOOST || sysStatus == HEATTING_CLEAN){
                        uint8_t prt_flag = 0;
                        prt_flag = HalSafetyProc(&stHeatManager);
                        if(prt_flag){
                            sm_log(SM_LOG_NOTICE, "\r\n HalSafetyProc status = 0 :heat_stop();");
                            status = 0;// 加热任务状态标志位 置位 至IDLE  
                            continue;          
                        }
                    }
                    //---------------------设置输出功率--------------------------
                    if(status == 1){//添加条件判断，避免状态跳变的时候 ，还被设置功率
                        heat_set_power(stHeatManager.SetPower );   
                    }
                    //---------------------每200ms打印一次log-------------------
                    if(++puff_indv >=25){// 500ms 
                        puff_indv = 0;
                        heat_puff_proc(&stHeatManager.HeatPuff,stHeatManager.SetPower,stHeatManager.HeatingTime);
                        
                        stHeatManager.HeatPuff =  heat_puff_get();
                    }
                    if(stHeatManager.HeatingTime - log_intv >= 200){// 200 MS打印一次
                        log_intv = stHeatManager.HeatingTime;
                        sysStatus = get_system_status();
                            if(ini_p[BASE_LOG_SW] == 1 && get_pc_tool_log_en()==0)// BASE LOG功能（1->使能， 0->禁止）
                            {
                                // sm_log2(SM_LOG_ERR,"%0.1f,%0.1f,%0.1f,%0.2f,%0.2f,%d,%0.2f,%0.2f,%0.4f J\n",
                                // (float)stHeatManager.HeatingTime/1000,\
                                // stHeatManager.CurrTargetTemp,\
                                // (stHeatManager.CurrDetectTemp - stHeatManager.tempAdjB)/stHeatManager.tempAdjK,\
                                // stHeatManager.SetPower,\
                                // stHeatManager.CurrPowerVal,\
                                // stHeatManager.HeatPuff,\
                                // stHeatManager.Heat_Hot_K,\
                                // heat_get_temp_k(),\
                                // stHeatManager.Heating_J
                                // ); 
                                // sm_log2(SM_LOG_ERR,"%0.3f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.4f\n",
                                //     (float)stHeatManager.HeatingTime/1000,\
                                //     stHeatManager.CurrDetectTemp,\
                                //     (stHeatManager.CurrDetectTemp - stHeatManager.tempAdjB)/stHeatManager.tempAdjK,\
                                //     stHeatManager.tempAdjB,\
                                //     stHeatManager.Heat_Hot_K,\
                                //     heat_t.usb_port_temp,
                                //     heat_t.heat_K_cood_temp,
                                //     stHeatManager.CurrResister);
                            }
                                     
                        test_20ms_adc_Convert_cnt = 0;
                    }
                }
               break;
            case 2:// 串口启动加热 --------------------
                sysStatus = get_system_status();
                if(sysStatus == IDLE || sysStatus == SLEEP){//停止加热
                    heat_stop(0);
                    status = 0;
                    continue;
                }
               test_heating_proc();
        	   break;
           default:    break;
       }
   }
}
//----------------上位机协议控制加热相关对接---------------------//
bool procotol_heat_stop(void)
{
    //heat_stop();
    SysStatus_u sysStatus = 0;
    sysStatus = get_system_status();
    if(sysStatus != CHARGING || sysStatus == CHIPPING_MODE_EXITTING){//停止加热
        set_system_status(IDLE);
       // sm_log(SM_LOG_NOTICE, "procotol Heat stop !\r\n");
    }
    return true;
}

bool procotol_heat_start(uint8_t heatType)
{
    bool rts = false;
    SysStatus_u sysStatus = 0;
    sysStatus = get_system_status();
    TaskHandle_t *temp_handle;
    if(sysStatus == IDLE || sysStatus == CHARGING)
    {
        switch (heatType)
            {
                case 0:
                    clear_ref_tbl();
                    set_system_external_even(EXT_EVENT_PCTOOL_CMD_TEST_HEAT);
                    break;
                case 1: // BASE MODE
                    set_system_external_even(EXT_EVENT_PCTOOL_CMD_STANDARD_HEAT);
                    break;
                case 2: // BOOST MODE
                    set_system_external_even(EXT_EVENT_PCTOOL_CMD_BOOST_HEAT);
                    break;
#if defined(DEF_DRY_CLEAN_EN)
                case 3: // CLEAR MODE
                    set_system_external_even(EXT_EVENT_PCTOOL_CMD_CLEAN_HEAT);
                    break;
#endif
                default:
                    return false;
                    break;
            }
    }else{
        return false;
    }
    return true;
}


uint8_t test_voltage_heat_start(float ctrlVoltage)
{
	stHeatManager.SetVotage = ctrlVoltage;
	if(ctrlVoltage > 4.5)return 1;
	else return 0;
}

uint8_t test_power_heat_start(float ctrlPower)
{
	stHeatManager.SetPower = ctrlPower;
	if(ctrlPower > 16.0f)return 1;
	else return 0;
}

uint8_t test_temp_heat_start(float ctrlTemp)
{
	stHeatManager.CurrTargetTemp = ctrlTemp;
	if(ctrlTemp > 500.0f)return 1;
	else return 0;
}

void test_heating_proc(void)
{
    static uint32_t log_intv = 0;
    SysStatus_u sysStatus = 0;
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC); 
    int16_t* ini_p = get_ini_val_info_handle();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	if(msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4) - stHeatManager.HeatTick >= 20)//20ms 来临
	{
		stHeatManager.HeatTick = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4);
        //加热状态切换判断
        if(stHeatManager.HeatingTime < stHeatManager.PreHeatTime)
        {
            stHeatManager.HeatState = HEAT_FLOW_STATE_PREHEAT;//预热状态
        } else if(stHeatManager.HeatingTime < (stHeatManager.TotalHeatTime - 20000u))
        {
            stHeatManager.HeatState = HEAT_FLOW_STATE_HEAT_NORMAL;
        } else if(stHeatManager.HeatingTime < stHeatManager.TotalHeatTime)
        {
            stHeatManager.HeatState = HEAT_FLOW_STATE_HEAT_STAGE_LAST_EOS;
        } else{
 
                heat_stop(0);
                sm_log(SM_LOG_NOTICE, "\r\n times up :heat_stop();");
                set_system_status(IDLE);
        }
        stHeatManager.HeatingTime = msTickDev->read( (uint8_t*)&stHeatManager.HeatTick, 4) - stHeatManager.StartTime;
        adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));//获取ADC采集数据
        //---------------------检测量更新-------------------------
        stHeatManager.CurrDetectTemp   = heat_t.heat_K_temp;     		// max31855_get_temp();
        stHeatManager.CurrPowerVal     = heat_t.heat_P;
        stHeatManager.DetectVotage     = heat_t.heat_V;
        stHeatManager.DetectCurrent    = heat_t.heat_I;
        if(stHeatManager.CurrPowerVal > 2.0f){
            stHeatManager.CurrResister = heat_t.heat_R;
            stHeatManager.nmos_res      = heat_t.nmos_res; 
        }
		switch(stHeatManager.HeatMode){
            case HEAT_MODE_TEST_VOLTAGE:
                hal_tps55288_set_out_v(stHeatManager.SetVotage);
                break;
            case HEAT_MODE_TEST_POWER:
                heat_set_power(stHeatManager.SetPower );
                break;
            case HEAT_MODE_TEST_TEMP:
                //--------------------控温算法逻辑-------------------------
                float heat_out = 0.0f;
                heat_out = fun_get_algorithm_PCBA_test( stHeatManager.CurrTargetTemp*stHeatManager.tempAdjK  + stHeatManager.tempAdjB,\
                                                    stHeatManager.CurrDetectTemp,\
                                                    stHeatManager.HeatingTime);
                stHeatManager.SetPower = range_1000_0_to_power(heat_out,MAX_POWER); //PID输出 0 - 1000 转化为 0 - power_max W
                //---------------------保护算法逻辑------------------------
                if(stHeatManager.SetPower > (float)ini_p[DB_FLT_TC_PWR_LIM]/100)
                {
                    stHeatManager.SetPower = (float)ini_p[DB_FLT_TC_PWR_LIM]/100;
                }
                //---------------------设置输出功率------------------------
                heat_set_power(stHeatManager.SetPower);
                break;
            default:
                break;
	    }
        if(stHeatManager.HeatingTime - log_intv >= 200){//200 MS打印一次
            log_intv = stHeatManager.HeatingTime;
        sysStatus = get_system_status();
        if(ini_p[BASE_LOG_SW] == 1 && get_pc_tool_log_en()==0)// BASE LOG功能（1->使能， 0->禁止）
        {
            sm_log2(SM_LOG_ERR,"%0.1f,%0.1f,%0.1f,%0.2f,%0.2f,[%0.5f,%0.3f,%0.3f,%0.3f][%0.3f,%d,%0.3f,%d]\n",
            (float)stHeatManager.HeatingTime/1000,\
            stHeatManager.CurrTargetTemp,\
            (stHeatManager.CurrDetectTemp - stHeatManager.tempAdjB)/stHeatManager.tempAdjK,\
            stHeatManager.SetPower,\
            stHeatManager.CurrPowerVal,\

            stHeatManager.CurrResister,\
            stHeatManager.SetVotage,\
            stHeatManager.DetectVotage,\
            stHeatManager.DetectCurrent,\
            
            p_fdb_b_info->fdb_b1_t.adcCurrAdjK,\
            p_fdb_b_info->fdb_b1_t.adcCurrAdjB/100,\
            p_fdb_b_info->fdb_b1_t.adcOutVdjK,\
            p_fdb_b_info->fdb_b1_t.adcOutVdjB/100
            ); 
        }
            test_20ms_adc_Convert_cnt = 0;
        }
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（5）加热电路控制PC->Device
/*
    Payload [1]:
                0x00 关闭输出，
                0x01 设置输出电压并启动，
                0x02 设置输出功率并启动，
                0x03 设置目标温度并启动
    Payload [2~3]:电压值（mv） 或 功率值（mW）或 温度值（℃）< 16位无符号整数>
*/
bool procotol_heat_opt(uint8_t optType,uint16_t optCode)
{
    bool rts = false;
    SysStatus_u sysStatus = 0;
    sysStatus = get_system_status();
    TaskHandle_t *temp_handle;
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);
    switch(optType)
    {
        case 0x00:// 0x00 关闭输出，
            sysStatus = get_system_status();
            if(sysStatus != CHARGING || sysStatus == CHIPPING_MODE_EXITTING){//停止加热
                set_system_status(IDLE);
               // sm_log(SM_LOG_NOTICE, "procotol Heat stop !\r\n");
            }
        break;
        case 0x01:// 设置输出电压并启动，电压值（mv）
            if(sysStatus != IDLE)
            {
                return false;
            }
           
            temp_handle = get_task_heatting_handle();
            heat_test_voltage_start((float)optCode/1000.0f,600*1000);// 默认先加热最大时间 600秒
            xTaskNotifyGive(*temp_handle);
            
        break;
        case 0x02:// 设置输出功率并启动，功率值（mW）
            if(sysStatus != IDLE)
            {
                return false; 
            }
          
            temp_handle = get_task_heatting_handle();
            heat_test_power_start((float)optCode/1000.0f,600*1000);// 默认先加热最大时间 600秒
            xTaskNotifyGive(*temp_handle);

        break;
        case 0x03:// 设置目标温度并启动, 温度值（℃）
            if(sysStatus == IDLE){
                temp_handle = get_task_heatting_handle();
                heat_test_temp_start((float)optCode,600*1000);// 默认先加热最大时间 600秒
                xTaskNotifyGive(*temp_handle);
            }else if(sysStatus == HEAT_MODE_TEST_TEMP)
            {
                stHeatManager.CurrTargetTemp = (float)optCode ;// 如果已经开启加热 则直接赋值给目标值 其他不变
            }else{
                return false;
            }
            
        break;
        case 0x04:// REF BASE HEAT
            // 归一化REF 加热 开启条件：冷端温度与 发热体温度 必须范围在24摄氏度±5℃ 
            adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));//获取ADC采集数据 
            if(heat_t.heat_K_temp >= 19.0f && heat_t.heat_K_temp <= 29.0f &&\
               heat_t.heat_K_cood_temp >= 19.0f && heat_t.heat_K_cood_temp <= 29.0f) 
            {
                set_system_external_even(EXT_EVENT_PCTOOL_CMD_REF_BASE_HEAT);
            }else{
                return false;
            }
            
        break;
        case 0x05:// REF BOOST HEAT
            // 归一化REF 加热 开启条件：冷端温度与 发热体温度 必须范围在24摄氏度±5℃ 
            adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));//获取ADC采集数据 
            if(heat_t.heat_K_temp >= 19.0f && heat_t.heat_K_temp <= 29.0f &&\
               heat_t.heat_K_cood_temp >= 19.0f && heat_t.heat_K_cood_temp <= 29.0f) 
            {
                set_system_external_even(EXT_EVENT_PCTOOL_CMD_REF_BOOST_HEAT);
            }else{
                return false;
            }
        break;
        default:
        break;
    }
    return true;
}//heat


uint8_t adj_pot_now = 0;//0 :1200mV 校准，1:3000mV校准
typedef struct
{ 
    uint16_t    adcCurr;
    uint16_t    fbCurr;
    uint16_t    adcV;
    uint16_t    fbV;  //通过上位机读回的电压
} Adj_DCDC_Info_t;

//DCDC 校准联合体
typedef union{
	Adj_DCDC_Info_t  calib_t;
	uint8_t     res[sizeof(Adj_DCDC_Info_t)];
}dcdc_adj_u;

dcdc_adj_u calibP1;
dcdc_adj_u calibP2;
static uint8_t pcba_adj_err = 0;//0:PASS 1:FAIL   
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（10）DC/DC校准
/*
    Step1:
        Payload [1~2]: 设置输出电压
                
    Step2:
        Payload [1~2]: T40~T45 间电压值（mV）< 16位无符号整数>
        Payload [3~4]: 电流值
        
    Step3: 延时＞500ms，等待稳定，PC再次读取电压并判断是否在设置范围。
    注：Step1~Step3, 需进行两次循环，分别为设置1.2V 及 3.5V电压下。
*/
bool procotol_DCDC_opt(uint16_t optVol)
{
	if(optVol > 4500)   //临时定义最大允许输出电压小于5V
	{
		optVol = 4500;
	}
    if(optVol >= 100 && optVol < 3000)// 清除校准，或者校准第一个点
    {
        if(optVol == 100)
        {
            adj_pot_now = 0xff;
            pcba_adj_err = 0;
            FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
            // data copy dcdc votage adj
            p_fdb_b_info->fdb_b1_t.adcCurrAdjB = 0;
            p_fdb_b_info->fdb_b1_t.adcCurrAdjK = 1.0f;

            p_fdb_b_info->fdb_b1_t.adcOutVdjB = 0;
            p_fdb_b_info->fdb_b1_t.adcOutVdjK = 1.0f;
            // 保存校准值
            app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
            sm_log2(SM_LOG_NOTICE,"calibration  write default val ok!\n");
        }else{
            adj_pot_now = 0;//校准第一个点
            memset(calibP1.res,0x00,sizeof(Adj_DCDC_Info_t));
            heat_test_voltage_start((float)optVol/1000, 3*1000);
            sm_log2(SM_LOG_NOTICE,"Adj_DCDC_opt(%0.3f)\n",(float)optVol/1000);
        }
    }else if(optVol >= 3000 && optVol <= 4500)
    {
            adj_pot_now = 1;//1:3000mV校准
            memset(calibP2.res,0x00,sizeof(Adj_DCDC_Info_t));
            heat_test_voltage_start((float)optVol/1000, 3*1000);
            sm_log2(SM_LOG_NOTICE,"Adj_DCDC_opt(%0.3f)\n",(float)optVol/1000);
    }else {
        heat_stop(0);
    }
   
    return true;
}

// 校准两个点 得到一个一次函数式子，然后设置
bool procotol_DCDC_adjust(uint16_t adjustVol, uint16_t adjustCur)
{
     bool bool_rts = false;
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC); 
    adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));//获取ADC采集数据   
    switch (adj_pot_now)
    {
        case 0://校准 第一个点
            calibP1.calib_t.adcCurr = (uint16_t)(heat_t.heat_I * 1000);
            calibP1.calib_t.fbCurr  = adjustCur;
            calibP1.calib_t.adcV  = (uint16_t)(heat_t.heat_V * 1000);
            calibP1.calib_t.fbV   = adjustVol;
            if(calibP1.calib_t.adcCurr*1.1f < calibP1.calib_t.fbCurr || \
               calibP1.calib_t.adcCurr*0.9f > calibP1.calib_t.fbCurr || pcba_adj_err)//机器与治具偏差大于 ±10%  返回失败
            {
                pcba_adj_err = 1;
                bool_rts = false;
            }else
            {
                sm_log2(SM_LOG_NOTICE," dcdc adj 1# point: adcCurr:%d mA,fbCurr:%d mA,adcVoltage:%d,fbV:%d mV\n",\
                calibP1.calib_t.adcCurr,\
                calibP1.calib_t.fbCurr,\
                calibP1.calib_t.adcV,\
                calibP1.calib_t.fbV );
                bool_rts = true;
            }
        break;
        case 1://校准 第二个点
            calibP2.calib_t.adcCurr = (uint16_t)(heat_t.heat_I * 1000);
            calibP2.calib_t.fbCurr  = adjustCur;
            calibP2.calib_t.adcV  = (uint16_t)(heat_t.heat_V * 1000);
            calibP2.calib_t.fbV   = adjustVol;
              if(calibP2.calib_t.adcCurr*1.1f < calibP2.calib_t.fbCurr || \
               calibP2.calib_t.adcCurr*0.9f > calibP2.calib_t.fbCurr || pcba_adj_err)//机器与治具偏差大于 ±10%  返回失败
               {
                    bool_rts = false;
               }else{
                    sm_log2(SM_LOG_NOTICE," dcdc adj 2# point: adcCurr:%d mA,fbCurr:%d mA,adcVoltage:%d,fbV:%d mV\n",\
                            calibP2.calib_t.adcCurr,\
                            calibP2.calib_t.fbCurr,\
                            calibP2.calib_t.adcV,\
                            calibP2.calib_t.fbV );
                    float cur_k = (float)(calibP2.calib_t.fbCurr - calibP1.calib_t.fbCurr)/(calibP2.calib_t.adcCurr - calibP1.calib_t.adcCurr);
                    float cur_b = (float)calibP2.calib_t.fbCurr - (float)cur_k*calibP2.calib_t.adcCurr;
                    sm_log2(SM_LOG_NOTICE,"calibration current: k = %0.4f,b = %0.4f\n",cur_k,cur_b);

                    float votage_k = (float)(calibP2.calib_t.fbV - calibP1.calib_t.fbV)/(calibP2.calib_t.adcV - calibP1.calib_t.adcV);
                    float votage_b = (float)calibP2.calib_t.fbV - (float)votage_k*calibP2.calib_t.adcV;
                    sm_log2(SM_LOG_NOTICE,"calibration Votage: k = %0.4f,b = %0.4f\n",votage_k,votage_b);
                    if(1)// if((cur_k > 0.95 && cur_k < 1.05) && (votage_k >0.95  && votage_k<1.05))//需要在这里把值存起来
                    {
                        FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
                        // data copy dcdc votage adj
                        p_fdb_b_info->fdb_b1_t.adcCurrAdjB = (int16_t)(cur_b*100);
                        p_fdb_b_info->fdb_b1_t.adcCurrAdjK = cur_k;

                        p_fdb_b_info->fdb_b1_t.adcOutVdjB = (int16_t)(votage_b*100);
                        p_fdb_b_info->fdb_b1_t.adcOutVdjK = votage_k;
                        // 保存校准值
                        app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
                    }
                    bool_rts = true;
               }
        break;
    default:
        break;
    }  
    heat_stop(0);
    set_system_status(IDLE);
    return bool_rts;
}
float get_dcdc_adj_val(float target,float err1,float err2)
{
    float err = 0.0f;

  return err;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（11）运放校准PC->Device
/*
    Payload [1~2]:T1运放输出电压值（mV）< 16位无符号整数>
*/

bool procotol_OPA_adjust_vol(uint16_t adjustVol)
{
    uint16_t err_opa_u16;//运放输出电压 偏差值（mV）
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);
    adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));//获取ADC采集数据
    err_opa_u16 =  adjustVol - heat_t.opa_adc_val;

    return true;
}

/*
   Payload [0]:
           Error code:
           0x00 正常
           Bit0: 0->正常，1->低压
           Bit1: 0->正常，1->发热体过温
           Bit2: 0->正常，1->硬件故障
*/
bool procotol_get_heat_sta(uint8_t* rValue)
{ 
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
    rValue[0] =0x00;
    if(p_tMontorDataInfo->bat.voltage<3580u)//低压3.58v 不加热
    {
        rValue[0] |= BIT0;
    }
    if(((uint32_t)p_tMontorDataInfo->det.heat_K_temp) > 500.0f)//大于500度过温
    {
        rValue[0] |= BIT1;
    }
    //加热情况下  才知道硬件有没有故障 判断依据为设置功率 与测得功率  偏差 大于1.5W 
    if(get_system_status() == HEATTING_STANDARD && get_system_status() == HEATTING_BOOST)
    {
        if((stHeatManager.CurrPowerVal > stHeatManager.SetPower + 1.5f && stHeatManager.CurrPowerVal < stHeatManager.SetPower - 1.5f))//设置功率 与计算功率 偏差超过2W硬件故障
        {
            rValue[0] |=BIT2;
        }   
    }
    return true;
}

unsigned int get_heating_time(void)
{
    SysStatus_u sysStatus = 0;
    sysStatus = get_system_status();
    if(sysStatus == HEATTING_STANDARD || sysStatus == HEATTING_BOOST || sysStatus == HEATTING_CLEAN)
    {
        return stHeatManager.HeatingTime;
    }else{
        return 0;
    }
}
