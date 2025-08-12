/**
  ******************************************************************************
  * @file    task_charge_service.c
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

#include "task_charge_service.h"
#include "task_system_service.h"
#include "task_ui_service.h"

#include "system_status.h"
#include "sm_log.h"
#include "platform_io.h"

#include "driver_charge_ic.h"
#include "data_base_info.h"
#include "err_code.h"
#include "system_interaction_logic.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "event_data_record.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#define PM_WORK_RUN_INTERVAL       			(20) // ms
#define PM_CHECK_500MS						(500 / PM_WORK_RUN_INTERVAL)
#define PM_CHECK_3S							(3000 / PM_WORK_RUN_INTERVAL)
#define PM_CHECK_5S							(5000 / PM_WORK_RUN_INTERVAL)
#define PM_CHG_TIMEOUT_NUM					(6 * 60 * 60) // 6H

/**************************************************************/
#define CHG_LOG		SM_LOG_DEBUG

typedef union {
	uint16_t uword;
	struct {
		uint16_t initTrig:1;
		uint16_t powerNoGood:1;
		uint16_t full:1;
		uint16_t timeout:1;

		uint16_t batBad:1;
		uint16_t batOVP:1;
		uint16_t batOC:1;
		uint16_t batOV:1;

		uint16_t batLimit:1;
		uint16_t usbHot:1;
		uint16_t busAbnor:1;
		uint16_t pcbaHot:1;

		uint16_t batHot:1;
		uint16_t batCold:1;
		uint16_t batLV:1;
		uint16_t preChg:1;
	}ubit;
} ChgFlag_u;

typedef struct ChargeInfo_t {
	uint32_t tick; // charge time
	uint32_t secTime;
	uint32_t minTime;

	uint8_t status;
	uint8_t regCheck;
	 int8_t slowChgState;

	ChgFlag_u flag;
	uint16_t cnt[16];
	uint16_t tmp;
} ChargeInfo_t;

typedef enum {
	CHECK_PG = 0,
	CHECK_BAT_BAD,
	CHECK_BAT_OVP,
	CHECK_BAT_OC,

	CHECK_BAT_OV,
	CHECK_BUS_ABNOR,
	CHECK_USB_HOT,
	CHECK_PCBA_HOT,

	CHECK_BAT_FULL,
	CHECK_BAT_TIMEOUT,
	CHECK_BAT_LV,
	CHECK_BAT_HOT,

	CHECK_BAT_COLD,
	CHECK_TIMEOUT,
	CHECK_TEMP,
	CHECK_ALL,
} CheckType_e;

static ChargeInfo_t chg;
static EolInfo_t eol;

/**
  * @brief  user func
  * @param
  * @return
  * @note   None
  */

bool get_chg_limit_en(void);
uint8_t get_chg_limit_sta(void);
void set_chg_limit_sta(uint8_t val);
uint16_t get_chg_limit_soc(void);
uint16_t get_chg_limit_volt(void);

bool get_debug_chg_en(void);

int chg_set(uint8_t chgSet, uint16_t dat)
{
	ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
	uint8_t cfgBuf[8] = {0};
	uint8_t len = 0;

	cfgBuf[0] = chgSet;
	switch (cfgBuf[0]) {
	case CHG_SET_CTRL: // enable or disable charge
	case CHG_SET_INIT: // init para
		if (false == get_debug_chg_en()) { // 上位机控制禁止充电
			cfgBuf[1] = CHG_DIS;
		} else {
			cfgBuf[1] = (uint8_t)(dat);
		}
		len = 2;
		break;

	case CHG_SET_HIZ: // enable or disable HIZ mode
		cfgBuf[1] = (uint8_t)(dat);
		len = 2;
		break;

	case CHG_SET_IBAT: // setup battery curr
	case CHG_SET_VBAT: // setup battery volt
		cfgBuf[1] = (uint8_t)(dat >> 8);
		cfgBuf[2] = (uint8_t)(dat);
		len = 3;
		break;

	default :
		cfgBuf[1] = CHG_DIS;
		len = 2;
		break;
	}
	return chgDev->write((uint8_t *)&cfgBuf, len);
}

bool debounce_check(uint16_t *cnt, uint16_t time)
{
    if (cnt == NULL) {
        return false;
    }

	if ((*cnt) < time) {
		(*cnt)++;
		if ((*cnt) == time) {
			return true;
		}
	}

	return false;
}

/**
 * @brief FreeRTOS variable to store handle of task created to update and send dummy
   values of charge
 */
TaskHandle_t task_charge_handle;

TaskHandle_t* get_task_charge_handle(void)
{
    return &task_charge_handle;
}

extern void fg_save_data(uint8_t sta);
/**
  * @brief  充电服务任务处理函数
  * @param  pvParam:    任务入参
  * @return None
  * @note   None
  */
void task_charge_service(void *pvParam)
{
    SysStatus_u sysStatus = 0;
    uint8_t buf[3] = {0};

    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ptIoDev fgDev = io_dev_get_dev(DEV_FULE_GAUGE);
	int16_t* p_iniVal = get_ini_val_info_handle();
	MonitorDataInfo_t* p_tMontorDataInfo = get_monitor_data_info_handle();
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	LifeCycle_t *p_tLifeCycleHandle = get_life_cycle_handle();
	static bool bNewChrg = false;
	uint8_t event_data[1];
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    while(true) {
    	switch (chg.status) {
		/**************************************************************************************/
		case 0:
			clr_task_charge_heartbeat_ticks();
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //waiting for notify
            set_task_charge_heartbeat_ticks();
			sm_log(SM_LOG_INFO, "TaskNotifyTakeCharging!!!\r\n");
			/***********************************************************/
			chg.tick = msTickDev->read((uint8_t*) &chg.tick, 4);
			/***********************************************************/
			chg.status++;
			break;
		/**************************************************************************************/
		case 1:
			sysStatus = get_system_status();
			if (sysStatus != CHARGING) {
				chg.status = 3; //exit charging
				sm_log(CHG_LOG, "Chg goto sta:%d\r\n", chg.status);
				break;
			}
			// 更新session对应的ini偏移量，用于参数配置和比较判断
			eol.index = get_iniTable_session_index(p_tMontorDataInfo->session); // eol
			if ((p_tMontorDataInfo->det.heat_K_temp >= (float)p_iniVal[FLT_TC_ZONE_HOT]) || // 发热体过温 ≥500℃
				(p_tMontorDataInfo->bat.temperature >= p_iniVal[STEP1_FLT_BAT_HOT + eol.index]) || // 电芯过热 ≥ 58℃
				(p_tMontorDataInfo->bat.temperature <= p_iniVal[FLT_BAT_COLD]) || // 电芯过冷 ≤ -8℃
				(p_tMontorDataInfo->det.heat_K_cood_temp >= (float)p_iniVal[FLT_CO_JUNC_HOT]) || // PCBA过热 ≥80℃
				(p_tMontorDataInfo->det.usb_port_temp >= (float)p_iniVal[FLT_USB_HOT_TEMP]) || // USB过热 ≥70℃
				(p_tMontorDataInfo->chg.bus_volt >= (uint16_t)p_iniVal[WAR_CHG_VBUS])) {// VBUS过压 ≥ 11V
				//(p_tMontorDataInfo->chg.bat_volt <= (uint16_t)p_iniVal[WAR_BAT_VOLT_DAMAGE])) { // 取消电压判断，防止低压锂保未接通导致无法复充
				if (msTickDev->read((uint8_t*) &chg.tick, 4) >= (chg.tick + 5000)) { // 异常超过5s 系统仍未切换状态
					chg.tick = msTickDev->read((uint8_t*) &chg.tick, 4);
					chg.status = 3; // exit charging
					sm_log(CHG_LOG, "Waiting overtime, exit!\r\n");
					break;
				}
			} else {
				if (msTickDev->read((uint8_t*) &chg.tick, 4) >= (chg.tick + 300)) { // delay for abnor checking
					chg.tick = msTickDev->read((uint8_t*) &chg.tick, 4);
					chg.secTime = 0;
					chg.minTime = 0;
					chg.regCheck = 0;
					chg.slowChgState = 4; // slow pre charge
					chg.flag.uword = 0; // clean all flag bit
					chg.flag.ubit.initTrig = 1;
					for (uint8_t i=0; i<CHECK_ALL; i++) {
						chg.cnt[i] = 0;
					}
					/**********************************************************************
					 * 电芯EOL配置:检测session和当前系统电压,决定充电IC截止电压 && 电量计Taper voltage.
					 * V_taper = V_chrg-0.04V,设置比充电IC电压低些,防止充不满,fg配置文件默认(4.36V).
					 * 每次充电启动时,调用一次; 电芯当前电压必须低于配置的满电电压,否则会引起SOC跳变和电芯过压保护.
					 **********************************************************************/
					eol.chgVolt = p_iniVal[STEP1_CHG_VOLT + eol.index];
					if (((p_iniVal[STEP1_CHG_VOLT]-40)/100) == (p_tMontorDataInfo->bat.taper_volt/100)) { // if clear session
						eol.indexBackup = (STEP2_SESSION - STEP1_SESSION) * 0;
					} else {
						if (((p_iniVal[STEP2_CHG_VOLT]-40)/100) == (p_tMontorDataInfo->bat.taper_volt/100)) { // if clear session
							eol.indexBackup = (STEP2_SESSION - STEP1_SESSION) * 1;
						} else {
							eol.indexBackup = (STEP2_SESSION - STEP1_SESSION) * 2;
						}
					}
					//sm_log(CHG_LOG, "index:%d,indexBackup:%d\r\n", eol.index, eol.indexBackup);
					if (eol.indexBackup != eol.index) {
						if ((eol.indexBackup > eol.index) || // eol 可能被修改和清除，直接修改
							(p_tMontorDataInfo->bat.soc <= 25) || // 实际SOC < 25%
							(p_tMontorDataInfo->bat.voltage <= (eol.chgVolt - 100))) {
							fg_save_data(1); // 禁止PD处理
							sys_task_security_ms_delay(PM_WORK_RUN_INTERVAL, TASK_CHARGE); // 喂狗延时
							chg.tmp = eol.chgVolt - 40; // mV
							buf[0] = FG_SET_TAPER_VOLT;
							buf[1] = (uint8_t)(chg.tmp >> 8);
							buf[2] = (uint8_t)(chg.tmp);
							if (RET_SUCCESS == fgDev->write((uint8_t *)&(buf), 3)) {
								sys_task_security_ms_delay(PM_WORK_RUN_INTERVAL, TASK_CHARGE); // 喂狗延时再备份
								// 每次设置过电量计参数，都同步更新备份一次电量计配置文件
								buf[0] = FG_SET_SAVE_GMFS;
								fgDev->write((uint8_t *)&(buf), 1);
								eol.indexBackup = eol.index; // 设置成功,更新备份值
								p_tMontorDataInfo->bat.taper_volt = chg.tmp; // 刷新taper_volt
							} else {
								eol.chgVolt = p_iniVal[STEP1_CHG_VOLT + eol.indexBackup];
							}
							fg_save_data(0); // 恢复PD处理
						} else { // 当前电芯电压 > (EOL_V - 0.1V), 使用上一档位充电电压值
							eol.chgVolt = p_iniVal[STEP1_CHG_VOLT + eol.indexBackup];
						}
					}
					sm_log(CHG_LOG, "chgVolt:%d,TaperVolt:%d\r\n", eol.chgVolt, p_tMontorDataInfo->bat.taper_volt);
					chg_set(CHG_SET_VBAT, eol.chgVolt); // pre set charge Vbat
					chg_set(CHG_SET_IBAT, PRE_INIT_IBAT); // pre set charge Ibat
					chg_set(CHG_SET_INIT, CHG_EN); // set charge init, enable charge

					p_tMontorDataInfo->state = CHG_STATE_CHARGE;
					p_tMontorDataInfo->partab = CHG_PARTAB_NORMAL_TEMP;
					set_chg_limit_sta(0); // 进入充电，清掉此前充电限制状态，重新判断
					sm_log(CHG_LOG, "Chg init, session:%d, dbg chg en:%d\r\n", \
							p_tMontorDataInfo->session, get_debug_chg_en());
					chg.status++;
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
					bNewChrg = true;
					event_record_generate(EVENT_CODE_CHARGE_START, event_data, 0);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				}
			}
			break;
		/**************************************************************************************/
		case 2:
			sysStatus = get_system_status();
			if (sysStatus != CHARGING) {
				chg.status++;
				break;
			}
			/***************************************************************************/
			// Vbus & usb pg check
			if (((p_tMontorDataInfo->chg.reg20_state & PG_STAT_MASK) == 0) ||
				((p_tMontorDataInfo->chg.reg21_state & VBUS_STAT_MASK) == 0)) {
				if (chg.flag.ubit.powerNoGood != 1) {
					chg.flag.ubit.powerNoGood = 1;
				}
				if (chg.cnt[CHECK_PG] != 0) {
					chg.cnt[CHECK_PG] = 0;
				}
			} else {
				chg.flag.ubit.powerNoGood = 0; // power good
				chg.regCheck = (p_tMontorDataInfo->chg.reg21_state & CHG_STAT_MASK) >> CHG_STAT_SHIFT;
				if ((chg.regCheck <= CHG_STAT_TAPER_CHARGING) && // charging
					(p_tMontorDataInfo->chg.bat_volt > (uint16_t)p_iniVal[WAR_BAT_VOLT_DAMAGE]) && /* 扩大电压判断范围，模拟电压测试会无法触发, 无电压限制影响死电池判断*/
					(p_tMontorDataInfo->chg.bat_curr < p_iniVal[STEP1_GAUGE_TERM_CURR + eol.index] / 2)) { // 电流出现了异常
					chg.cnt[CHECK_PG]++;
					if (chg.cnt[CHECK_PG] == PM_CHECK_500MS) {
						chg_set(CHG_SET_CTRL, CHG_DIS); // set disable charge
						/* 取消此处 check pg log 打印，周期性打印影响上位机内容解析
						if ((true == get_debug_chg_en()) || // 如果老化测试设置禁止充电，不打印此log
							(false == get_chg_limit_en())) { // 如果使能充电电量限制，不打印此log
							sm_log(CHG_LOG, "Chg PG check!\r\n");
						}*/
					}
					else if (chg.cnt[CHECK_PG] >= PM_CHECK_3S) {
						chg.cnt[CHECK_PG] = 0;
						if (false == get_chg_limit_en()) { // 如果使能充电电量限制，不执行恢复充电操作
							chg_set(CHG_SET_CTRL, CHG_EN); // set enable charge
						}
					}
				} else {
					if (chg.cnt[CHECK_PG] != 0) {
						chg.cnt[CHECK_PG] = 0;
						if (false == get_chg_limit_en()) { // 如果使能充电电量限制，不执行恢复充电操作
							chg_set(CHG_SET_CTRL, CHG_EN); // set enable charge
						}
					}
				}
			}
			// hardware over voltage detected:4.4 * 105%
			chg.regCheck = p_tMontorDataInfo->chg.reg22_fault & BAT_FAULT0_MASK;
			if ((chg.regCheck != 0) && (p_tMontorDataInfo->chg.bat_volt > eol.chgVolt)) {
				if (debounce_check(&chg.cnt[CHECK_BAT_OVP], PM_CHECK_500MS)) {
					chg.flag.ubit.batOVP = 1;
					p_tMontorDataInfo->state = CHG_STATE_FAULT;
					p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
					add_error_even(FLT_DE_CIC_OUTPUT_VOLTAGE);
					sm_log(CHG_LOG, "Vbat over:reg22:0x%02x\r\n", p_tMontorDataInfo->chg.reg22_fault);
					chg.status++;
					break; // exit charging state
				}
			} else {
				chg.cnt[CHECK_BAT_OVP] = 0;
				chg.flag.ubit.batOVP = 0;
			}
			// over current check
			if ((p_tMontorDataInfo->bat.current >= p_iniVal[FLT_BAT_CHARGE_CURR_OVER]) ||
				(p_tMontorDataInfo->chg.bat_curr >= p_iniVal[FLT_BAT_CHARGE_CURR_OVER])) {
				if (debounce_check(&chg.cnt[CHECK_BAT_OC], PM_CHECK_500MS)) {
					chg.flag.ubit.batOC = 1;
					p_tMontorDataInfo->state = CHG_STATE_FAULT;
					p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
					add_error_even(FLT_DE_BAT_CHARGE_CURRENT_OVER);
					sm_log(CHG_LOG, "Ibat over:%d\r\n", p_tMontorDataInfo->chg.bat_curr);
					chg.status++;
					break; // exit charging state
				}
			} else {
				chg.cnt[CHECK_BAT_OC] = 0;
				chg.flag.ubit.batOC = 0;
			}
			// software over voltage detected:4.45
			if (p_tMontorDataInfo->chg.bat_volt >= (uint16_t)p_iniVal[FLT_BAT_VOLTAGE_OVER]) {
				if (debounce_check(&chg.cnt[CHECK_BAT_OV], PM_CHECK_500MS)) {
					chg.flag.ubit.batOV = 1;
					p_tMontorDataInfo->state = CHG_STATE_FAULT;
					p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
					add_error_even(FLT_DE_BAT_VOLTAGE_OVER);
					sm_log(CHG_LOG, "Vbat over:%d\r\n", p_tMontorDataInfo->chg.bat_volt);
					chg.status++;
					break; // exit charging state
				}
			} else {
				chg.cnt[CHECK_BAT_OV] = 0;
				chg.flag.ubit.batOV = 0;
			}
			// usb voltage check:>11V
			if ((chg.flag.ubit.powerNoGood == 0) && \
				(p_tMontorDataInfo->chg.bus_volt >= (uint16_t)p_iniVal[WAR_CHG_VBUS])) {
				if (debounce_check(&chg.cnt[CHECK_BUS_ABNOR], PM_CHECK_500MS)) {
					chg.flag.ubit.busAbnor = 1;
					p_tMontorDataInfo->state = CHG_STATE_FAULT;
					p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
					add_error_even(FLT_DE_CHG_VBUS);
					sm_log(CHG_LOG, "Vusb abnor:%d\r\n", p_tMontorDataInfo->chg.bus_volt);
					chg.status++;
					break; // exit charging state
				}
			} else {
				chg.cnt[CHECK_BUS_ABNOR] = 0;
				chg.flag.ubit.busAbnor = 0;
			}
			/***************************************************************************/
			// battery low voltage detected, fuel gauge lock voltage:2.5V
			if ((p_tMontorDataInfo->chg.bat_volt <= PROT_BAT_LOW_VOLT_THRESHOLD)) {
				chg.flag.ubit.batLV = 1;
				chg.cnt[CHECK_BAT_LV] = 0;
				//sm_log(CHG_LOG, "chg bat low, Vbat:%d\r\n", p_tMontorDataInfo->chg.bat_volt);
			} else {
				if (chg.cnt[CHECK_BAT_LV] <= PM_CHECK_500MS) {
					chg.cnt[CHECK_BAT_LV]++;
					if (chg.cnt[CHECK_BAT_LV] == PM_CHECK_500MS) {
						chg.flag.ubit.batLV = 0;
					}
				}
			}

			if (chg.flag.ubit.batLV == 0) {
				// bat temperature > 55℃ 或 53℃
				if (p_tMontorDataInfo->bat.temperature >= p_iniVal[STEP1_FLT_BAT_HOT_CHARGING + eol.index]) {
					if (debounce_check(&chg.cnt[CHECK_BAT_HOT], PM_CHECK_500MS)) {
						p_tMontorDataInfo->state = CHG_STATE_TEMP;
						p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_HOT;
						add_error_even(FLT_DE_BAT_HOT_CHARGE);
						sm_log(CHG_LOG, "Tbat hot:%d\r\n", p_tMontorDataInfo->bat.temperature);
						chg.status++;
						break; // exit charging state
					}
					chg.flag.ubit.batHot = 1;
				} else {
					chg.cnt[CHECK_BAT_HOT] = 0;
					chg.flag.ubit.batHot = 0;
				}
				// bat temperature < 2℃
				if (p_tMontorDataInfo->bat.temperature <= (float)p_iniVal[FLT_BAT_COLD_CHARGE]) {
					if (debounce_check(&chg.cnt[CHECK_BAT_COLD], PM_CHECK_500MS)) {
						p_tMontorDataInfo->state = CHG_STATE_TEMP;
						p_tMontorDataInfo->partab = CHG_PARTAB_SUSPENDED_BY_COLD;
						add_error_even(FLT_DE_BAT_COLD_CHARGE);
						sm_log(CHG_LOG, "Tbat cold:%d\r\n", p_tMontorDataInfo->bat.temperature);
						chg.status++;
						break; // exit charging state
					}
					chg.flag.ubit.batCold = 1;
				} else {
					chg.cnt[CHECK_BAT_COLD] = 0;
					chg.flag.ubit.batCold = 0;
				}
			} else {
				chg.cnt[CHECK_BAT_HOT] = 0;
				chg.cnt[CHECK_BAT_COLD] = 0;
				chg.flag.ubit.batHot = 0;
				chg.flag.ubit.batCold = 0;
			}
			/***************************************************************************/
			// charge state check
			chg.regCheck = (p_tMontorDataInfo->chg.reg21_state & CHG_STAT_MASK) >> CHG_STAT_SHIFT;
			if ((chg.regCheck <= CHG_STAT_PRE_CHARGING) && 
				(chg.regCheck != CHG_STAT_NOT_CHARGING)) { // precharge check
				if (chg.flag.ubit.preChg != 1) {
					chg.flag.ubit.preChg = 1;
				}
			} else {
				if (chg.flag.ubit.preChg != 0) {
					chg.flag.ubit.preChg = 0;
				}
			}

			if (chg.regCheck >= CHG_STAT_TERMINATION_DONE) { // charge full
				if (debounce_check(&chg.cnt[CHECK_BAT_FULL], PM_CHECK_5S)) {
					chg.flag.ubit.full = 1;
					p_tMontorDataInfo->state = CHG_STATE_FULL;
					p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
					if (p_tMontorDataInfo->bat.soc != 100) {
						set_charge_full(true); // 充满电,如果soc不等于100%,更新soc
					}
					sm_log(CHG_LOG, "Chg full,reg21:0x%02x\r\n",p_tMontorDataInfo->chg.reg21_state);
					//chg.status++; // 充满电，不触发IDLE状态，
					//break; // exit charging state
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
					if (bNewChrg)
					{
						bNewChrg = false;
						(p_tLifeCycleHandle->batFullChargeCnt)++;
						set_update_lifecycle_status(1);
					}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				}
			} else {
				chg.cnt[CHECK_BAT_FULL] = 0;
				chg.flag.ubit.full = 0;
			}

			// 出厂充电限制
			if (true == get_chg_limit_en()) {
				if ((p_tMontorDataInfo->bat.remap_soc >= get_chg_limit_soc()) && (get_chg_limit_soc() > 0)) { // 不做滤波
					if (chg.flag.ubit.batLimit != 1) {
						chg.flag.ubit.batLimit = 1;
						set_chg_limit_sta(1);
						sm_log(CHG_LOG, "Chg limit:%d%%, SOC:%d%%\r\n", get_chg_limit_soc(), p_tMontorDataInfo->bat.remap_soc);
						p_tMontorDataInfo->state = CHG_STATE_OTHER;
						p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
						chg_set(CHG_SET_CTRL, CHG_DIS); // disable charge
					}
				}

				if ((p_tMontorDataInfo->bat.voltage >= get_chg_limit_volt()) && (get_chg_limit_volt() > 0)) {
					if (chg.flag.ubit.batLimit != 1) {
						chg.flag.ubit.batLimit = 1;
						set_chg_limit_sta(2);
						sm_log(CHG_LOG, "Chg limit:%dmV, Vbat:%dmV\r\n", get_chg_limit_volt(), p_tMontorDataInfo->bat.voltage);
						p_tMontorDataInfo->state = CHG_STATE_OTHER;
						p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
						chg_set(CHG_SET_CTRL, CHG_DIS); // disable charge
					}
				}
			} else {
				if (chg.flag.ubit.batLimit != 0) {
					chg.flag.ubit.batLimit = 0;
					chg_set(CHG_SET_CTRL, CHG_EN); // enable charge
					set_chg_limit_sta(0);
				}
			}

			// charge timeout check
			if (msTickDev->read((uint8_t*) &chg.tick, 4) >= (chg.tick + 1000)) {
				chg.tick = msTickDev->read((uint8_t*) &chg.tick, 4);
				if ((chg.flag.ubit.full == 0) &&
					(chg.flag.ubit.batLimit == 0) &&
					(chg.flag.ubit.timeout == 0)) {
					chg.secTime++;
				}
				if (chg.secTime >= 60) {
					chg.secTime = 0;
					chg.minTime++;
					if (chg.minTime >= p_iniVal[FLT_CIC_CHARGE_TIMEOUT]) {
						chg.flag.ubit.timeout = 1;
						//chg.secTime = 0; //不清0
						//chg.minTime = 0; //不清0
						p_tMontorDataInfo->state = CHG_STATE_TIMEOUT;
						p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
						add_error_even(FLT_DE_CIC_CHARGE_TIMEOUT);
						sm_log(CHG_LOG, "Chg timeout:%dmin %dsec\r\n", chg.minTime,chg.secTime);
						chg.status++;
						break; // exit charging state
					}
				}
				//sm_log(CHG_LOG, "Chg time:%dmin %dsec!\r\n", chg.minTime,chg.secTime); // debug
				// Fast charge, trickle charge and pre-charge timer status
				if (p_tMontorDataInfo->chg.reg20_state & SAFETY_TMR_STAT_MASK) {
					if (debounce_check(&chg.cnt[CHECK_TIMEOUT], 5)) { // 5s
						chg.flag.ubit.timeout = 1;
						//chg.secTime = 0; //不清0
						//chg.minTime = 0; //不清0
						p_tMontorDataInfo->state = CHG_STATE_TIMEOUT;
						p_tMontorDataInfo->partab = CHG_PARTAB_OTHER;
						add_error_even(FLT_DE_CIC_CHARGE_TIMEOUT);
						sm_log(CHG_LOG, "Chg timeout:reg20:0x%02x\r\n", p_tMontorDataInfo->chg.reg20_state);
						chg.status++;
						break; // exit charging state
					}
				} else {
					chg.cnt[CHECK_TIMEOUT] = 0;
				}
			}
			/***************************************************************************/
			// check bat temperature normal and set Ibat
			if ((chg.flag.ubit.powerNoGood == 0) && // power is good
				(chg.flag.ubit.batLV == 0) &&
				(chg.flag.ubit.batHot == 0) &&
				(chg.flag.ubit.batCold == 0) &&
				(chg.flag.ubit.full == 0) &&
				(chg.flag.ubit.batLimit == 0) &&
				(chg.flag.ubit.preChg == 0) &&
				(chg.flag.ubit.timeout == 0)) {
				if (p_tMontorDataInfo->bat.temperature > p_iniVal[STEP1_UPPER_TEMP_CHARGING + eol.index]) {
					if (chg.slowChgState != 1) {
						chg.slowChgState = 1;
						chg.cnt[CHECK_TEMP] = 0;
					}
				} else if (p_tMontorDataInfo->bat.temperature <= BAT_CHG_LOWER_TEMP_THRESHOLD) {
					if (chg.slowChgState != 2) {
						chg.slowChgState = 2;
						chg.cnt[CHECK_TEMP] = 0;
					}
				} else {
					if (p_tMontorDataInfo->bat.temperature <= p_iniVal[STEP1_UPPER_TEMP_CHARGE_CLEAR + eol.index]) {
						if (chg.slowChgState != 3) {
							chg.slowChgState = 3;
							chg.cnt[CHECK_TEMP] = 0;
						}
					}
				}

				switch (chg.slowChgState) {
				case 1:
					if (debounce_check(&chg.cnt[CHECK_TEMP], PM_CHECK_500MS)) {
						if (chg.flag.ubit.initTrig != 0) {
							chg.flag.ubit.initTrig = 0;
						}
						chg_set(CHG_SET_IBAT, p_iniVal[STEP1_UPPER_TEMP_CHARGE_CURR + eol.index]); // 45~55℃ max:800mA
						chg_set(CHG_SET_CTRL, CHG_EN); // set enable charge
						p_tMontorDataInfo->state = CHG_STATE_CHARGE;
						p_tMontorDataInfo->partab = CHG_PARTAB_SLOW_BY_HOT;
						//sm_log(CHG_LOG, "chg slow1, %d\r\n", p_tMontorDataInfo->bat.temperature);
					}
					break;

				case 2:
					if (debounce_check(&chg.cnt[CHECK_TEMP], PM_CHECK_500MS)) {
						if (chg.flag.ubit.initTrig != 0) {
							chg.flag.ubit.initTrig = 0;
						}
						chg_set(CHG_SET_IBAT, BAT_CHG_LOWER_TEMP_CURR_LIMIT); //  2~15℃ max:400mA
						chg_set(CHG_SET_CTRL, CHG_EN); // set enable charge
						p_tMontorDataInfo->state = CHG_STATE_CHARGE;
						p_tMontorDataInfo->partab = CHG_PARTAB_SLOW_BY_COLD;
						//sm_log(CHG_LOG, "chg slow2, %d\r\n", p_tMontorDataInfo->bat.temperature);
					}
					break;

				case 3:
					if (debounce_check(&chg.cnt[CHECK_TEMP], PM_CHECK_500MS)) {
						if (chg.flag.ubit.initTrig != 0) {
							chg.flag.ubit.initTrig = 0;
						}
						chg_set(CHG_SET_IBAT, p_iniVal[STEP1_CHG_CURR + eol.index]); // 15~45℃ max:2560mA
						chg_set(CHG_SET_CTRL, CHG_EN); // set enable charge
						p_tMontorDataInfo->state = CHG_STATE_CHARGE;
						p_tMontorDataInfo->partab = CHG_PARTAB_NORMAL_TEMP;
						//sm_log(CHG_LOG, "chg normal, %d\r\n", p_tMontorDataInfo->bat.temperature);
					}
					break;

				default:
					if (debounce_check(&chg.cnt[CHECK_TEMP], PM_CHECK_5S)) { // 延时退出初始电流限制状态
						if (chg.flag.ubit.initTrig != 0) {
							chg.flag.ubit.initTrig = 0;
							sm_log(CHG_LOG, "chg flag:%04x\r\n", chg.flag.uword);
						}
						chg_set(CHG_SET_IBAT, p_iniVal[STEP1_CHG_CURR + eol.index]); // 15~45℃ max:2560mA
						chg_set(CHG_SET_CTRL, CHG_EN); // set enable charge
						p_tMontorDataInfo->state = CHG_STATE_CHARGE;
						p_tMontorDataInfo->partab = CHG_PARTAB_NORMAL_TEMP;
						//sm_log(CHG_LOG, "chg normal, %d\r\n", p_tMontorDataInfo->bat.temperature);
					}
					break;
				}
			} else {
				if (chg.slowChgState != 0) {
					chg.slowChgState = 0;
					sm_log(CHG_LOG, "chg flag:%04x\r\n", chg.flag.uword);
				}
				chg.cnt[CHECK_TEMP] = 0;
			}
			break;
		/**************************************************************************************/
		default:
			chg_set(CHG_SET_CTRL, CHG_DIS); // disable charge
			// chg_debug_clear(); // USB移除强制清除调试参数？
			sm_log(CHG_LOG, "End of charge!\r\n");
			chg.status = 0;
			
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			(p_tLifeCycleHandle->batChargeTimeTotal) += (chg.minTime * 60 + chg.secTime);
			set_update_lifecycle_status(1);
			event_record_generate(EVENT_CODE_CHARGE_STOP, event_data, 0);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	
			break;
    	}
        sys_task_security_ms_delay(PM_WORK_RUN_INTERVAL, TASK_CHARGE);
    }
}

volatile bool chargFull = false; // true:充满，false:未满
volatile uint32_t checkFullTick = 0;
volatile uint32_t checkFullTime = 0;
bool get_charge_full(void) // 获取充电满状态
{
    return chargFull;
}

void set_charge_full(bool sta)
{
	chargFull = sta;
}

bool check_full_remap_soc(void)
{
	SysStatus_u tempSysStatus = get_system_status();
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

	if (true == get_charge_full()) {
		if ((HEATTING_STANDARD == tempSysStatus) ||
			(HEATTING_BOOST == tempSysStatus) ||
			(HEATTING_TEST == tempSysStatus) ||
			(HEATTING_CLEAN == tempSysStatus) ||
			(HEAT_MODE_TEST_VOLTAGE == tempSysStatus) ||
			(HEAT_MODE_TEST_POWER == tempSysStatus) ||
			(HEAT_MODE_TEST_TEMP == tempSysStatus)) {
			set_charge_full(false); // 只要加热放电，就解除
			checkFullTime = 0;
		} else if (IDLE == tempSysStatus) {
			if (msTickDev->read((uint8_t*) &checkFullTick, 4) >= (checkFullTick + 1000)) {
				checkFullTick = msTickDev->read((uint8_t*) &checkFullTick, 4);
				checkFullTime++;
			}
			if (checkFullTime >= 2700) { // 空闲状态累计大于45min放电，就解除
				checkFullTime = 0;
				set_charge_full(false);
			}
		}
		return true;
	} else {
		if (checkFullTime != 0) {
			checkFullTime = 0;
		}
		checkFullTick = msTickDev->read((uint8_t*) &checkFullTick, 4);
	}

	return false;
}

int chg_get_timeout_flag(void)
{
	//sm_log(SM_LOG_INFO, "Charge full check!\r\n");
	return (int)(chg.flag.ubit.timeout);
}

bool g_vbusHizModeStatus = false;
// 获取VBUS模式， 0：正常模式，1高阻模式
bool is_vbus_hiz_mode(void)
{
	MonitorDataInfo_t* p_tMontorDataInfo = get_monitor_data_info_handle();

    if ((p_tMontorDataInfo->chg.reg16_ctrl1 & HIZ_MODE_MASK) == HIZ_MODE_ENABLE)  {
        return true;
    }

    return false;
}
// 开始加热前设置为高阻，加热结束后设置为正常状态
int chg_set_hiz_mode(uint8_t en) 
{
	return chg_set(CHG_SET_HIZ, en);
}

int chg_set_chg_en(uint8_t en)
{
	return chg_set(CHG_SET_CTRL, en);
}

/*****************************************************************************/
/* 用于上位机老化测试功能：上位机控制是否充电，默认使能，协议解除或设备复位后清除 */
static bool dbgChgEn = true;

bool get_debug_chg_en(void)
{
	return dbgChgEn;
}

bool procotol_chg_set_en(uint8_t en)
{
	if (en > 0) { // 0x01 使能充电
		dbgChgEn = true;
		chg_set(CHG_SET_CTRL, CHG_EN);
	} else { // 0x00 停止充电
		dbgChgEn = false; // 复位后状态可恢复
		chg_set(CHG_SET_CTRL, CHG_DIS);
	}

	return true;
} // CHG DBG

bool procotol_chg_get_en(void)
{
    return dbgChgEn;
}

/*****************************************************************************/
/* 用于出厂充电限制，默认不限制，协议解除或设备复位后清除 */
typedef struct ChgLimitInfo_t {
	bool en;
	uint8_t sta;

	uint16_t soc;
	uint16_t volt;
} ChgLimitInfo_t;

ChgLimitInfo_t chgLimit;

bool get_chg_limit_en(void)
{
	return chgLimit.en;
}

uint8_t get_chg_limit_sta(void)
{
	return chgLimit.sta;
}

void set_chg_limit_sta(uint8_t val)
{
	chgLimit.sta = val;
}

uint16_t get_chg_limit_soc(void)
{
	return chgLimit.soc;
}

uint16_t get_chg_limit_volt(void)
{
	return chgLimit.volt;
}

bool payload_chg_limit(uint8_t en, uint8_t soc, uint16_t volt)
{
	chgLimit.en = en > 0? true : false;
	
	if (soc > 100) { // 充电soc限制值
		soc = 100;
	}
	chgLimit.soc = (uint16_t)soc;
	
	if (volt > 4400) { // 充电volt限制值
		volt = 4400;
	}
	if (volt < 2000) {
		volt = 0;
	}
	chgLimit.volt = volt;

	return true;
}

bool procotol_get_charge_sta(uint8_t* rValue)
{
    SysStatus_u sysStatus = get_system_status();

	if (chgLimit.en == false) {
 	    if (sysStatus == CHARGING) {
	        rValue[0]=0x01;//充电状态
	    }else{
	        rValue[0]=0x00;//非充电状态，
	    }
	} else {
		if (chgLimit.sta == 1) {
			rValue[0] = 0x12; // 充电限制使能,（SOC限制）非充电状态
		} else if (chgLimit.sta == 2) {
			rValue[0] = 0x13; // 充电限制使能,（VOLT限制）非充电状态
		} else {
			if (sysStatus != CHARGING) {
				rValue[0] = 0x10; // 充电限制使能，非充电状态
			}else{
				rValue[0] = 0x11; // 充电限制使能,充电状态
			}
		}
	}

    return true;
}
/*****************************************************************************/
/* 用于充电IC寄存器打印调试 */ 
volatile uint8_t chgRegPrt = 0;

bool procotol_get_chg_reg(uint8_t cmd)
{
	chgRegPrt = 0x01;
	return true;
}

void charge_debug_log(void)
{
	MonitorDataInfo_t* p_tMontorDataInfo = get_monitor_data_info_handle();

	if (chgRegPrt == 0) {
		return;
	}
	chgRegPrt = 0;

	//set_current_ui_detail_status(UI_CUSTOMIZE);	return; // debug

#if 0 // debug
	static uint8_t cnt = 0;
	if (cnt == 0)
		set_current_ui_detail_status(UI_EOL);
	else if (cnt == 1)
		set_current_ui_detail_status(UI_ERR_CRITICAL);
	else
		set_current_ui_detail_status(UI_BLE_DOWNLOAD);

	cnt++;cnt %= 3;
	return; // UI_CUSTOMIZE debug
#endif

	sm_log(SM_LOG_ERR, "SysPara - Tpcab:%0.0fC, Tusb:%0.0fC, Vbus:%dmV, Ibus:%dmA, Vcbat:%dmV, Icbat:%dmA\r\n",
		p_tMontorDataInfo->det.heat_K_cood_temp,
		p_tMontorDataInfo->det.usb_port_temp,
		p_tMontorDataInfo->chg.bus_volt,
		p_tMontorDataInfo->chg.bus_curr,
		p_tMontorDataInfo->chg.bat_volt,
		p_tMontorDataInfo->chg.bat_curr);
#if 0
	sm_log(SM_LOG_ERR, "BQ25638 - R14:%02x, R15:%02x, R16:%02x, R17:%02x, R18:%02x, R19:%02x, R1A:%02x\r\n",
		p_tMontorDataInfo->chg.reg14_ctrlTimer,
		p_tMontorDataInfo->chg.reg15_ctrl0,
		p_tMontorDataInfo->chg.reg16_ctrl1,
		p_tMontorDataInfo->chg.reg17_ctrl2,
		p_tMontorDataInfo->chg.reg18_ctrl3,
		p_tMontorDataInfo->chg.reg19_ctrl4,
		p_tMontorDataInfo->chg.reg1A_ctrl5);
#endif
	sm_log(SM_LOG_ERR, "BQ25638 - R20:%02x, R21:%02x, R22:%02x, R23:%02x, R24:%02x, R25:%02x\r\n",
		p_tMontorDataInfo->chg.reg20_state,
		p_tMontorDataInfo->chg.reg21_state,
		p_tMontorDataInfo->chg.reg22_fault,
		p_tMontorDataInfo->chg.reg23_flag,
		p_tMontorDataInfo->chg.reg24_flag,
		p_tMontorDataInfo->chg.reg25_fault);
#if 1 // runlog 已实现电量计log
	sm_log(SM_LOG_ERR, "BQ27426 - Vbat:%dmV, Ibat:%dmA, Tbat:%dC, SOC:%d%%, tureSOC:%d%%, tureRM:%dmAh, filterRM:%dmAh, tureFCC:%dmAh, filterFCC:%dmAh \r\n",
		p_tMontorDataInfo->bat.voltage,
		p_tMontorDataInfo->bat.current,
		p_tMontorDataInfo->bat.temperature,
		p_tMontorDataInfo->bat.soc,
		p_tMontorDataInfo->bat.true_soc,
		p_tMontorDataInfo->bat.ture_rm,
		p_tMontorDataInfo->bat.filtered_rm,
		p_tMontorDataInfo->bat.ture_fcc,
		p_tMontorDataInfo->bat.filtered_fcc);

	sm_log(SM_LOG_ERR, "BQ27426 - passChg:%dmAh, qmax:%d, DOD0:%d, DODatEOC:%d, paresentDOD:%d, ctlSta:%04x, flags:%04x \r\n",
		p_tMontorDataInfo->bat.passed_charge,
		p_tMontorDataInfo->bat.qmax,
		p_tMontorDataInfo->bat.dod0,
		p_tMontorDataInfo->bat.dodateoc,
		p_tMontorDataInfo->bat.paresent_dod,
		p_tMontorDataInfo->bat.ctrl_status,
		p_tMontorDataInfo->bat.flags);
#endif

	ptIoDev fgDev = io_dev_get_dev(DEV_FULE_GAUGE);
	uint8_t buf[1] = {FG_SET_DEBUG};
    fgDev->write((uint8_t *)&(buf), 1);
}

uint16_t get_iniTable_session_index(int16_t session)
{
	int16_t* p_ini = get_ini_val_info_handle();
	uint16_t index = 0;

	if (session < p_ini[STEP2_SESSION]) { // 0~8200
		index = 0;
	} else if (session < p_ini[STEP3_SESSION]) { // 8200~11000
		index = 1;
	} else { // session >= p_ini[STEP3_SESSION] // > 11000
		index = 2;
	}
	index *= (STEP2_SESSION - STEP1_SESSION); // 12

	return index;
}

/****************************************end****************************************/

