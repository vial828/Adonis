#include "task_usb_service.h"
#include "platform_io.h"

#include "task_heatting_service.h"
#include "task_system_service.h"
#include "sm_log.h"
#include "system_status.h"
#include "data_base_info.h"
#include "system_param.h"
//#include "SEGGER_RTT.h"
//#include "serial_line.h"
#include "shell_cmd_handle.h"

#include "task_bt_service.h"


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

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "ota.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#include <inttypes.h>
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include "protocol_usb.h"

#include "event_data_record.h"


/**
 *  private variable
*/
static volatile uint8_t log_event_state=0;
static volatile uint8_t log_session_state=0;

TaskHandle_t task_usb_handle;

TaskHandle_t* get_task_usb_handle(void)
{
    return &task_usb_handle;
}

TaskHandle_t get_task_usb(void)
{
    return task_usb_handle;
}

int format_data(uint8_t *buf,const char *format, ...)
{
    uint32_t length;
    va_list args;
    va_start(args, format);
    length = vsnprintf((char*)buf, 255, (char*)format, args);
    va_end(args);
    return length;
}

int log_param_format(ComtLogParam_t logInfo, ComtLogTime_t comtLogTime, uint8_t *pBuf)
{
    int i = 0;
    int tmpLen = 0;
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    //uint32_t msTicks = msTickDev->read( (uint8_t*)&msTicks, 4);
    SysStatus_u sysStaus = get_system_status();
    uint32_t msTicks = get_heating_time();
    
    // 类别
    pBuf[0] = 0;
    i = 1;
    memcpy((char*)&pBuf[i], (uint8_t *)&msTicks, 4); // 时间戳
    i += 4;
    //g_tImageSecondRam[i] = ',';
   // i++;
   if (CHARGING == sysStaus) {
       tmpLen = sprintf((char*)&pBuf[i], "charge");
//       tmpLen = format_data(&g_tImageSecondRam[i], "charge");
       i += tmpLen;
   } else if (HEATTING_STANDARD == sysStaus || HEATTING_BOOST == sysStaus || HEATTING_CLEAN == sysStaus) {
       tmpLen = sprintf((char*)&pBuf[i], "heat");
       i += tmpLen;
   }else {
       tmpLen = sprintf((char*)&pBuf[i], "idle");
       i += tmpLen;
   }
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf((char*)&pBuf[i], "%d", logInfo.sp);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.pv);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.batv);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.bata);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d%%", logInfo.resoc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.dciv);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.dcov);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.dcoa);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%.3f", logInfo.dcow);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%.3f", logInfo.htres);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d%%", logInfo.soc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.batt);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.coldt);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.usbt);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.usbv);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.usba);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%.3f", logInfo.power_J);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.chg_state);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.partab);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    if (logInfo.ble) {
        tmpLen = sprintf(&pBuf[i], "enable");
        i += tmpLen;
    } else {
        tmpLen = sprintf(&pBuf[i], "disable");
        i += tmpLen;
    }
    return i;
}

int log_fuel_param_format(ComtFuelLogParam_t logInfo, ComtLogTime_t comtLogTime, uint8_t *pBuf)
{
    int i = 0;
    int tmpLen = 0;
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    //uint32_t msTicks = msTickDev->read( (uint8_t*)&msTicks, 4);
    SysStatus_u sysStaus = get_system_status();
    uint32_t msTicks = get_heating_time();
    
    // 类别
    pBuf[0] = 0;
    i = 1;
    memcpy((char*)&pBuf[i], (uint8_t *)&msTicks, 4); // 时间戳
    i += 4;
    //g_tImageSecondRam[i] = ',';
   // i++;
   if (CHARGING == sysStaus) {
       tmpLen = sprintf((char*)&pBuf[i], "charge");
//       tmpLen = format_data(&g_tImageSecondRam[i], "charge");
       i += tmpLen;
   } else if (HEATTING_STANDARD == sysStaus || HEATTING_BOOST == sysStaus || HEATTING_CLEAN == sysStaus) {
       tmpLen = sprintf((char*)&pBuf[i], "heat");
       i += tmpLen;
   }else {
       tmpLen = sprintf((char*)&pBuf[i], "idle");
       i += tmpLen;
   }
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf((char*)&pBuf[i], "%d", logInfo.batv);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.bati);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.batt);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d%%", logInfo.soc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d%%", logInfo.truesoc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.truerm);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.fltrm);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.truefcc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.fltfcc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.passchg);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.qmax);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.dod0);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.dodateoc);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.paredod);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.ctlsta);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "%d", logInfo.flags);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "na");
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "na");
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "na");
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    tmpLen = sprintf(&pBuf[i], "na");
    i += tmpLen;
    return i;
}

int log_event_param_format(uint8_t *pBuf)
{
    // sm_log(SM_LOG_INFO, "%s\r\n", __FUNCTION__);
    int i = 0;
    int tmpLen = 0;
    debug_service_event_log_char_t event_log_char;

    switch (log_event_state)
    {
		case 0:			// Idle
		{
			return 0;
		}
		case 1:			// star
		 {
			if (event_log_get(&event_log_char))
			{
				log_event_state++;
			}
			else
			{
				log_event_state = 3;
				return 0;
			}
			break;
        }
		case 2:			// continue
		{
			if (!event_log_get_next(&event_log_char))
			{
				log_event_state = 3;
				return 0;
			}
			break;
        }
		case 3:			// Idle
		default:
		{
			return 0;
		}
    }

    // 类别
    pBuf[0] = 0;
    i = 1;
    uint32_t msTicks = get_heating_time();
    memcpy((char*)&pBuf[i], (uint8_t *)&msTicks, 4); // 时间戳
    i += 4;
    // count
    // memcpy((char*)&pBuf[i], (uint8_t *)&event_log_char.count, 4);
    // i += 4;
    tmpLen = sprintf(&pBuf[i], "%u", event_log_char.count);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // timestamp
    // memcpy((char*)&pBuf[i], (uint8_t *)&event_log_char.timestamp, 4);
    // i += 4;
    tmpLen = sprintf(&pBuf[i], "%u", event_log_char.timestamp);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // event_code
    // memcpy((char*)&pBuf[i], (uint8_t *)&event_log_char.event_code, 1);
    // i += 1;
    tmpLen = sprintf(&pBuf[i], "%u", event_log_char.event_code);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // event_data
    for (uint8_t k=0; k<15; k++)
    {
        tmpLen = sprintf(&pBuf[i], "%02x", event_log_char.event_data[k]);
        i += tmpLen;
    }
    // memcpy((char*)&pBuf[i], (uint8_t *)&event_log_char.event_data, 15);
    // i += 15;

    return i;
}

int log_session_param_format(uint8_t *pBuf)
{
    // sm_log(SM_LOG_INFO, "%s\r\n", __FUNCTION__);
    int i = 0;
    int tmpLen = 0;
    session_service_records_char_t session_log_char;

    switch (log_session_state)
    {
		case 0:			// Idle
		{
			return 0;
		}
		case 1:			// star
		 {
			if (session_log_get(&session_log_char))
			{
				log_session_state++;
			}
			else
			{
				log_session_state = 3;
				return 0;
			}
			break;
        }
		case 2:			// continue
		{
			if (!session_log_get_next(&session_log_char))
			{
				log_session_state = 3;
				return 0;
			}
			break;
        }
		case 3:			// Idle
		default:
		{
			return 0;
		}
    }


    // 类别
    pBuf[0] = 0;
    i = 1;
    uint32_t msTicks = get_heating_time();
    memcpy((char*)&pBuf[i], (uint8_t *)&msTicks, 4); // 时间戳
    i += 4;
    // count
    // memcpy((char*)&pBuf[i], (uint8_t *)&session_log_char.count, 4);
    // i += 4;
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.count);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // timestamp
    // memcpy((char*)&pBuf[i], (uint8_t *)&session_log_char.timestamp, 4);
    // i += 4;
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.time_stamp);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // duration
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.duration);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // session_exit_code
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.session_exit_code);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // mode
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.mode);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // heatingProfile
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.heatingProfile);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // z1_max_temp
    tmpLen = sprintf(&pBuf[i], "%d", session_log_char.z1_max_temp);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // battery_max_temp
    tmpLen = sprintf(&pBuf[i], "%d", session_log_char.battery_max_temp);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // trusted
    tmpLen = sprintf(&pBuf[i], "%u", session_log_char.trusted);
    i += tmpLen;
    
    return i;
}

int log_lifecycle_param_format(uint8_t *pBuf)
{
    // sm_log(SM_LOG_INFO, "%s\r\n", __FUNCTION__);
    int i = 0;
    int tmpLen = 0;
	LifeCycle_t * pLifeCycle = get_life_cycle_handle();
    debug_service_lifecycle_data_opcode01_rsp_t lifecycle_data;

	memcpy((uint8_t*)&lifecycle_data, (uint8_t*)(&(pLifeCycle->batTempMin)), sizeof(debug_service_lifecycle_data_opcode01_rsp_t));

    // 类别
    pBuf[0] = 0;
    i = 1;
    uint32_t msTicks = get_heating_time();
    memcpy((char*)&pBuf[i], (uint8_t *)&msTicks, 4); // 时间戳
    i += 4;
    // Min_battery_temperature
    tmpLen = sprintf(&pBuf[i], "%d", lifecycle_data.Min_battery_temperature);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Max_battery_temperature
    tmpLen = sprintf(&pBuf[i], "%d", lifecycle_data.Max_battery_temperature);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Min_battery_voltage
    tmpLen = sprintf(&pBuf[i], "%u", lifecycle_data.Min_battery_voltage);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Max_battery_voltage
    tmpLen = sprintf(&pBuf[i], "%u", lifecycle_data.Max_battery_voltage);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Max_battery_charge_current
    tmpLen = sprintf(&pBuf[i], "%d", lifecycle_data.Max_battery_charge_current);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Max_battery_discharge_current
    tmpLen = sprintf(&pBuf[i], "%d", lifecycle_data.Max_battery_discharge_current);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Total_charge_time
    tmpLen = sprintf(&pBuf[i], "%u", lifecycle_data.Total_charge_time);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // No_of_fully_charged_count
    tmpLen = sprintf(&pBuf[i], "%u", lifecycle_data.No_of_fully_charged_count);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // No_of_complete_session
    tmpLen = sprintf(&pBuf[i], "%u", lifecycle_data.No_of_complete_session);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // No_of_incomplete_session
    tmpLen = sprintf(&pBuf[i], "%u", lifecycle_data.No_of_incomplete_session);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Min_Zone1_temperature
    tmpLen = sprintf(&pBuf[i], "%d", lifecycle_data.Min_Zone1_temperature);
    i += tmpLen;
    pBuf[i] = ',';
    i++;
    // Max_Zone1_temperature
    tmpLen = sprintf(&pBuf[i], "%d", lifecycle_data.Max_Zone1_temperature);
    i += tmpLen;

    return i;
}

void copy_data_to_log(ComtLogParam_t *logInfo, ComtLogTime_t *comtLogTime)
{
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    SysStatus_u tempSysStatus = get_system_status();
    IniInfo_t *p_tIniInfo =  get_iniInfo_handle();
    int16_t *piniValTbl = get_ini_val_info_handle();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    
    // 发热体相关
    if (tempSysStatus == HEATTING_BOOST || tempSysStatus == HEATTING_STANDARD || tempSysStatus == HEATTING_CLEAN) {
        
        logInfo->pv = (int16_t)((p_tMontorDataInfo->heaterInfo.CurrDetectTemp - (float)p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f)/p_fdb_b_info->fdb_b1_t.tempAdjK);
        logInfo->sp = (int16_t)(p_tMontorDataInfo->heaterInfo.CurrTargetTemp);
    } else {
        logInfo->pv = (int16_t)p_tMontorDataInfo->det.heat_K_temp;
        logInfo->sp = logInfo->pv;
    }
    // 电量计相关
    logInfo->batv = p_tMontorDataInfo->bat.voltage;
    logInfo->bata = p_tMontorDataInfo->bat.current;
    logInfo->resoc = p_tMontorDataInfo->bat.remap_soc;
    logInfo->soc = p_tMontorDataInfo->bat.soc;
    // 加热相关
    logInfo->dciv = p_tMontorDataInfo->bat.voltage; 
    if (tempSysStatus == HEATTING_BOOST || tempSysStatus == HEATTING_STANDARD || tempSysStatus == HEATTING_CLEAN) {
        logInfo->dcov = (int16_t)(p_tMontorDataInfo->heaterInfo.SetVotage * 1000);
        logInfo->dcoa = (int16_t)(p_tMontorDataInfo->det.heat_I * 1000);
        logInfo->power_J = p_tMontorDataInfo->heaterInfo.Heating_J;
    } else {
        logInfo->dcoa = 0;
        logInfo->dcov = 0;
        logInfo->power_J = 0;
    }
    logInfo->dcow = p_tMontorDataInfo->det.heat_P;
    logInfo->htres = p_tMontorDataInfo->det.heat_R;
    //   logInfo.ses_J = ;
    logInfo->batt = p_tMontorDataInfo->bat.temperature;
    // 充电相关
    logInfo->coldt = (int16_t)p_tMontorDataInfo->det.heat_K_cood_temp;
    logInfo->usbt = (int16_t)p_tMontorDataInfo->det.usb_port_temp;
    logInfo->usbv = p_tMontorDataInfo->chg.bus_volt;
    logInfo->usba = p_tMontorDataInfo->chg.bus_curr;
    if (tempSysStatus == HEATTING_BOOST || tempSysStatus == HEATTING_STANDARD || tempSysStatus == HEATTING_CLEAN) {
		logInfo->chg_state = 0; // clear state
		logInfo->partab = 0; // clear state
    } else {
		logInfo->chg_state = p_tMontorDataInfo->state;
		logInfo->partab = p_tMontorDataInfo->partab;
    }
    logInfo->ble = piniValTbl[PFUN_BLE_EN];
}

void copy_fuel_gauge_data_to_log(ComtFuelLogParam_t *logInfo, ComtLogTime_t *comtLogTime)
{
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    SysStatus_u tempSysStatus = get_system_status();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    logInfo->batv = p_tMontorDataInfo->bat.voltage;
    logInfo->bati = p_tMontorDataInfo->bat.current;
    logInfo->batt = p_tMontorDataInfo->bat.temperature;;
    logInfo->soc = p_tMontorDataInfo->bat.soc;
    logInfo->truesoc = p_tMontorDataInfo->bat.true_soc;
    logInfo->truerm = p_tMontorDataInfo->bat.ture_rm;
    logInfo->fltrm = p_tMontorDataInfo->bat.filtered_rm;
    logInfo->truefcc = p_tMontorDataInfo->bat.ture_fcc;
    logInfo->fltfcc = p_tMontorDataInfo->bat.filtered_fcc;
    logInfo->passchg = p_tMontorDataInfo->bat.passed_charge;
    logInfo->qmax = p_tMontorDataInfo->bat.qmax;
    logInfo->dod0 = p_tMontorDataInfo->bat.dod0;
    logInfo->dodateoc = p_tMontorDataInfo->bat.dodateoc;
    logInfo->paredod = p_tMontorDataInfo->bat.paresent_dod;
    logInfo->ctlsta = p_tMontorDataInfo->bat.ctrl_status;
    logInfo->flags = p_tMontorDataInfo->bat.flags;
}

void task_usb_service(void *pvParam)
{
	ptIoDev usbDev = io_dev_get_dev(DEV_USB);
	uint32_t ulNotificationValue;
	uint8_t usbBuf[1024 +11];// UART_USB_LEN_MAX
	int tempRxLen,tempTxLen;

    while(1)
    {
        clr_task_usb_heartbeat_ticks();
        xTaskNotifyWait(0x00000000,0xffffffff,&ulNotificationValue,portMAX_DELAY);//waiting for notify 
        set_task_usb_heartbeat_ticks();
        if(ulNotificationValue & USB_RX)
        {
			tempRxLen = usbDev->read( usbBuf, sizeof(usbBuf));
            if (get_shipping_mode() == 0) { // 船运模式不处理接收数据
                /******************Protocol processing start**********************/
                tempTxLen =  frame_parse_usb(usbBuf,tempRxLen);
                if(tempTxLen > 0)
                {
                    /**
                     * added by vincent
                     */
//                    log_event_state = 0;
//                    log_session_state = 0;

                    update_idl_delay_time(); // 收到一帧合法数据才更新睡眠计时器
                    usbDev->write(usbBuf,(uint16_t)tempTxLen);
                }
                else //如果不是上面的数据帧，可能是其它数据
                {
                
#if (defined ENABLE_PCTOOL_SHELL)
                        for (int i = 0; i <tempRxLen; i++) 
                        {
                            serial_line_input_byte(usbBuf[i]);
                        }
#endif
#if 0 // 去掉透传
                    if (app_server_context.bt_conn_id)//已与手机链接
                    {
                        wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                            HDLC_SESSION_SERVICE_DEVICE_INFORMATION_VALUE,
                                                            tempRxLen,(uint8_t *)usbBuf,NULL);
                    }
#endif
                }
            }
			/******************Protocol processing end**********************/
		}
        if(ulNotificationValue & USB_LOG)//LOG 发送
        {
            uint8_t usbCopyBuf[512] = {0};
            if (get_shipping_mode() == 0)  // 船运模式不发送日志;
            { 
                ComtLogTime_t comtLogTime;
                // memset((uint8_t*)&logInfo,0x0,sizeof(logInfo));
                int len = 0;
                uint8_t log_type = get_send_pc_log_type();

                // sm_log(SM_LOG_DEBUG, "get_send_pc_log_type(): %d\n", log_type);

				if (log_event_state && log_type != LOG_EVENT)	// 当数据正在传输，转换成另外Log，终止
				{
					log_event_state = 0;
				}

				if (log_session_state && log_type != LOG_SESSION)	// 当数据正在传输，转换成另外Log，终止
				{
					log_session_state = 0;
				}

                if (log_type == LOG_COMMON) // common log
                {
                    ComtLogParam_t logInfo;
                    copy_data_to_log(&logInfo, &comtLogTime);  //获取数据
                    len = log_param_format(logInfo, comtLogTime, usbCopyBuf); // 将实体数据格式化
                } 
                else if (log_type == LOG_FUELGAGUE) // fuel log
                {
                    ComtFuelLogParam_t logFuelInfo;
                    copy_fuel_gauge_data_to_log(&logFuelInfo, &comtLogTime); //获取数据
                    len = log_fuel_param_format(logFuelInfo, comtLogTime, usbCopyBuf); // 将实体数据格式化
                }
                else if (log_type == LOG_EVENT) // event log
                {
					if (log_event_state == 0)	// 启动
					{
						log_event_state = 1;
					}
                    len = log_event_param_format(usbCopyBuf); // 将实体数据格式化
                    #if 0
                    sm_log(SM_LOG_DEBUG, "len: %d data: ", len);
                    for(uint8_t i=0; i<len; i++)
                        sm_log(SM_LOG_DEBUG, " 0x%02x", usbCopyBuf[i]);
                    sm_log(SM_LOG_DEBUG, "\n");
                    #endif
                }
                else if (log_type == LOG_SESSION) // session log
                {
					if (log_session_state == 0)	// 启动
					{
						log_session_state = 1;
					}
                    len = log_session_param_format(usbCopyBuf); // 将实体数据格式化
                }
                else if (log_type == LOG_LIFECYCLE) // lifecycle log
                {
                    len = log_lifecycle_param_format(usbCopyBuf); // 将实体数据格式化
                }
                
                if (len > 0) //added by vincent.he, 2025.2.21
                {
                    tempTxLen = frame_create_usb(usbBuf,PROC_REQUEST_LOG,usbCopyBuf,len);
                    if(tempTxLen > 0)
                    {
                        usbDev->write(usbBuf,(uint16_t)tempTxLen);
                    }
                }
            }
        }
        ulNotificationValue = 0;
	}
}

