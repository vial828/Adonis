#include "platform_io.h"
#include "heat_safety.h"
#include "driver_detector.h"
#include "data_base_info.h"
#include "sm_log.h"
#include "driver_fuel_gauge.h"
#include "err_code.h"
#include "system_status.h"
#include "task_charge_service.h"
#include "heat_hot_cold.h"
#include <stdio.h>
#include <string.h>

//#define NONE_HEATING_PROTECT 1
float g_power_i = 0;
 
uint8_t flag_power_high    = 0;
uint32_t pretect_power_cnt = 0;
uint32_t power_high_cnt = 0;
uint32_t power_low_cnt  = 0;
#define POWER_I_NUM  30
float power_table[POWER_I_NUM]   = {0};
uint32_t protect_cnt =0;
uint32_t power_protect_cnt = 0;

uint32_t step_4puff=0,puff_t1 = 0,puff_t2 = 0;
uint32_t old_puff   = 0;
uint32_t puff_indv  = 0;
uint8_t power_i_div = 0;


#define PWM_POWER_MIN   (4.2f) // 保护期间最小值
#define PWM_POWER_MAX   (6.8f) // 保护期间最大值

#define PWM_PERRIO     	100 // 周期 2秒
#define PWM_DUTY       	50  // 占空比 50%
uint32_t pwm_ccr = 0;
float    pwm_power = 0;

 
uint8_t g_flag_protect = 0;
uint16_t pretect_power_index = 0;
uint32_t protect_tick_indv = 0;
extern float g_protect_power_i;


//获取温度变化率 k=(y1-y2)/(x1-x2)
static float heater_temp_buf[3]={0};
static float heater_temp_k = 0.0f;
static float temp_k_add_sum = 0.0f;
static uint8_t heater_temp_k_index = 0;
static uint8_t add_index = 0;

static uint8_t flag_pcba_temp_high = 0;
//////////////////////////////////////////////////////////////////////////////////////////
const float ref_bk_temp_buf[5]={56.6f,89.0f,127.6f,161.4f,196.9f};//对比buf
//断针加热相关判断参数结构体
#define HCK_T_BUF_LEN      4 // 断针保护采样深度 ，5个数据，1秒1个
typedef struct Heater_ck_t
{  
    float startTemp;    // 开启加热时的T0 时刻温度
    uint16_t tempIndex; //当前温度索引
    float TempBuf[HCK_T_BUF_LEN]; // 前5秒的 buf ，一秒一个值
}Heater_ck_t;
Heater_ck_t hck_t;//
typedef struct Threshold_t
{
    float  envTemp; //环境温度
    float  kTemp;   //热电偶温度
} Threshold_t;
typedef struct TimeTemp_t
{
    float  time;  //时间 S秒
    float  temp;  //温度 ℃摄氏度
} TimeTemp_t;
typedef struct heating_linear_t
{
    float  temp_k;  //时间 S秒
    float  temp_b;  //温度 ℃摄氏度
} heating_linear_t;
//---------------断针相关-检测变量------------------------//
heating_linear_t hlt;
heating_linear_t hlt_adj;
heating_linear_t hlt_0_1s;
TimeTemp_t hbk_tt_tbl[HCK_T_BUF_LEN];
TimeTemp_t hbk_tt_adj_tbl[HCK_T_BUF_LEN];
uint32_t div_func1;
uint16 func1_index;
TimeTemp_t func1_tbl[30];
uint32_t tc_bk_indv = 0;
DEV_T start_heat_t;
#define DEF_BK_REF_MAGIC (0X55AA)
typedef struct FDB_pin_bk_float_t
{ 
    // Solution 2 断针相关 归一化处理的 数值放大1000倍
    //BASE
    uint16_t    magicBase;          // magic字段，存储标志
    float    baseDryHeatSlope1;  // 空烧 0.479 - 1s 
    float    baseDryHeatSlope2;  // 空烧 2-5秒斜率
    float    baseThreshold_0_1_low;     // 判断阈值1
    float    baseThreshold_0_1_high;     // 判断阈值2
    float    baseThreshold_2_5_low;     // 判断阈值3
    float    baseThreshold_2_5_high;     // 判断阈值4
    float    baseRev1;           // 保留4字节
    float    baseRev2;           // 保留4字节
    //BOOST
    uint16_t    magicBoost;         // magic字段，存储标志
    float    boostDryHeatSlope1; // 空烧 0.479 - 1s 斜率
    float    boostDryHeatSlope2; // 空烧 2-5秒斜率
    float    boostThreshold_0_1_low;      // 判断阈值1
    float    boostThreshold_0_1_high;     // 判断阈值2
    float    boostThreshold_2_5_low;      // 判断阈值3
    float    boostThreshold_2_5_high;     // 判断阈值4 
    float    boostRev1;          // 保留4字节
    float    boostRev2;          // 保留4字节
}FDB_pin_bk_float_t;
static FDB_pin_bk_float_t pin_break_threshold_t;
static uint8_t flag_pin_break = 0;
//---------------断针相关-检测变量 end------------------------//
//最小二乘法 线性回归 线性拟合
// static void sync_temp_linear( void )
// {	
// 	double sum_x, sum_y, sum_xy, sum_xx, t, r;	
// 	uint32_t i;
	
// 	sum_x = sum_y = sum_xy = sum_xx = 0;
// 	for( i = 0; i < HCK_T_BUF_LEN; i++ )
// 	{		
// 		t = (float)( hbk_tt_tbl[i].time);// /1000.0f void overflow when linear
// 		r = (float)( hbk_tt_tbl[i].temp);
// 		sum_x  = sum_x + t;     // x axis - Res
// 		sum_y  = sum_y + r;     // y axis - Temp
// 		sum_xy = sum_xy + r*t;
// 		sum_xx = sum_xx + t*t;		
// 	}	

// 	hlt.temp_k = ( HCK_T_BUF_LEN * sum_xy - sum_x * sum_y ) / ( HCK_T_BUF_LEN * sum_xx - (sum_x)*sum_x );
// 	hlt.temp_b = (( sum_y * sum_xx  - sum_x * sum_xy ) / ( HCK_T_BUF_LEN * sum_xx - (sum_x)*sum_x));		
// }
void get_temp_linear(heating_linear_t *ret_val,TimeTemp_t *input_tbl,uint8_t tbl_len)
{
	double sum_x, sum_y, sum_xy, sum_xx, t, r;	
	uint32_t i;
	
	sum_x = sum_y = sum_xy = sum_xx = 0;
	for( i = 0; i < tbl_len; i++ )
	{		
		t = (float)( input_tbl[i].time);// /1000.0f void overflow when linear
		r = (float)( input_tbl[i].temp);
		sum_x  = sum_x + t;     // x axis - Res
		sum_y  = sum_y + r;     // y axis - Temp
		sum_xy = sum_xy + r*t;
		sum_xx = sum_xx + t*t;		
	}	

	ret_val->temp_k = (tbl_len * sum_xy - sum_x * sum_y) / (tbl_len * sum_xx - (sum_x)*sum_x);
	ret_val->temp_b = ((sum_y * sum_xx  - sum_x * sum_xy) / (tbl_len * sum_xx - (sum_x)*sum_x));
}
//////////////////////////////////////////////////////////////////////////////////////////
float heat_get_temp_k(void)
{
    return heater_temp_k;
}


void HalSafetyInit(float startTemp,uint8_t heat_mode)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    FDB_AREA_F_u  fdb_f_u;
    flag_pin_break = 0;
    app_param_read(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
    if(fdb_f_u.fdb_f_t.pinBreakParam.magicBase == DEF_BK_REF_MAGIC )
    {
        pin_break_threshold_t.magicBase                 = fdb_f_u.fdb_f_t.pinBreakParam.magicBase;          
        pin_break_threshold_t.baseDryHeatSlope1         = (float)(fdb_f_u.fdb_f_t.pinBreakParam.baseDryHeatSlope1)/1000.0f;  
        pin_break_threshold_t.baseDryHeatSlope2         = (float)(fdb_f_u.fdb_f_t.pinBreakParam.baseDryHeatSlope2)/1000.0f;  
        pin_break_threshold_t.baseThreshold_0_1_low     = (float)(fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_0_1_low)/1000.0f;     
        pin_break_threshold_t.baseThreshold_0_1_high    = (float)(fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_0_1_high)/1000.0f;     
        pin_break_threshold_t.baseThreshold_2_5_low     = (float)(fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_2_5_low)/1000.0f;     
        pin_break_threshold_t.baseThreshold_2_5_high    = (float)(fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_2_5_high)/1000.0f;   
    }else{
        pin_break_threshold_t.magicBase = 0;
    }
    if(fdb_f_u.fdb_f_t.pinBreakParam.magicBoost == DEF_BK_REF_MAGIC )
    {
        pin_break_threshold_t.magicBoost                = fdb_f_u.fdb_f_t.pinBreakParam.magicBoost ;         
        pin_break_threshold_t.boostDryHeatSlope1        = (float)(fdb_f_u.fdb_f_t.pinBreakParam.boostDryHeatSlope1)/1000.0f;  
        pin_break_threshold_t.boostDryHeatSlope2        = (float)(fdb_f_u.fdb_f_t.pinBreakParam.boostDryHeatSlope2)/1000.0f;  
        pin_break_threshold_t.boostThreshold_0_1_low    = (float)(fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_0_1_low )/1000.0f;     
        pin_break_threshold_t.boostThreshold_0_1_high   = (float)(fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_0_1_high)/1000.0f;     
        pin_break_threshold_t.boostThreshold_2_5_low    = (float)(fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_2_5_low )/1000.0f;     
        pin_break_threshold_t.boostThreshold_2_5_high   = (float)(fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_2_5_high)/1000.0f;     
    }else{
        pin_break_threshold_t.magicBoost = 0;
    }
    power_protect_cnt = 0;
    step_4puff =0;
    g_power_i = 0;
    pretect_power_cnt = 0;
    puff_indv = 300*1000;
    power_i_div = 0;
    for(int i=0;i<POWER_I_NUM;i++)//积分窗口buf clear
    {
        power_table[i] = 0.0; //功率累加和 清0
    }
    // 热电偶温度斜率K 
    heater_temp_buf[0]=0.0f;
    heater_temp_buf[1]=0.0f;
    heater_temp_buf[2]=0.0f;
    heater_temp_k = 0.0f;
    heater_temp_k_index = 0;
    temp_k_add_sum = 0.0f;
    add_index = 0;
	g_flag_protect = 0;
    // 断针相关参数初始化
    hck_t.startTemp = startTemp;
    hck_t.tempIndex = 0;
    tc_bk_indv = 0;
    //归一化 判定
    if(heat_mode == HEATTING_STANDARD)
    {
 
        sm_log(SM_LOG_NOTICE, "base  heat. ref_0_1s_k[%0.3f],ref_2_5s_k[%0.3f],[%0.3f][%0.3f][%0.3f][%0.3f]\r\n",\
                pin_break_threshold_t.baseDryHeatSlope1 ,\    
                pin_break_threshold_t.baseDryHeatSlope2 ,\    
                pin_break_threshold_t.baseThreshold_0_1_low ,\
                pin_break_threshold_t.baseThreshold_0_1_high,\
                pin_break_threshold_t.baseThreshold_2_5_low ,\
                pin_break_threshold_t.baseThreshold_2_5_high);
    }else{

        sm_log(SM_LOG_NOTICE, "boost heat. ref_0_1s_k[%0.3f],ref_2_5s_k[%0.3f],[%0.3f][%0.3f][%0.3f][%0.3f]\r\n",\
                pin_break_threshold_t.boostDryHeatSlope1,\     
                pin_break_threshold_t.boostDryHeatSlope2,\     
                pin_break_threshold_t.boostThreshold_0_1_low,\ 
                pin_break_threshold_t.boostThreshold_0_1_high,\
                pin_break_threshold_t.boostThreshold_2_5_low,\
                pin_break_threshold_t.boostThreshold_2_5_high);
    }
    for(int i = 0;i < HCK_T_BUF_LEN;i++)
    {
        hbk_tt_tbl[i].temp = 0.0f;
        hbk_tt_adj_tbl[i].temp = 0.0f;
    }
    div_func1   = 0;
    func1_index = 0;
}

// 加热前 检测发热体温度、冷端补偿（PCB板）温度、电池温度
uint8_t safety_check_before_heating(void)
{
    uint8_t rts = 0;
    DetectorInfo_t heat_start;
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);   
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    TaskHandle_t *temp_handle;
#ifdef NONE_HEATING_PROTECT
    return 0; // 去掉保护
#endif
    //INI 相关
    int16_t* ini_p = get_ini_val_info_handle();
    adcDev->read((uint8_t*)&heat_start,sizeof(heat_start));//获取ADC采集数据

    if(ini_p[DB_FLT_TC_CONT] == 1)//连续开机保护（1->使能， 0->禁止）
    {
        // 检测发热体温度，大于预设阈值 不开启加热
        if(heat_start.heat_K_temp >= (float)ini_p[FLT_TC_ZONE_HOT_PRE_SES] || \
           p_tMontorDataInfo->det.heat_K_temp >= (float)ini_p[FLT_TC_ZONE_HOT_PRE_SES]) // 200 , 启动加热时发热体过温(TBD)
        {
            rts |= 0X1;
            add_error_even(FLT_DE_TC_ZONE_HOT_PRE_SES);
            sm_log(SM_LOG_ERR, "FLT_DE_TC_ZONE_HOT_PRE_SES\r\n");
        } else{
           // sm_log(SM_LOG_NOTICE, " DB_FLT_TC_CONT = %d ,FLT_TC_ZONE_HOT_PRE_SES = %d PASS\r\n",ini_p[DB_FLT_TC_CONT],ini_p[FLT_TC_ZONE_HOT_PRE_SES]);
        } 
    }else{
      // sm_log(SM_LOG_NOTICE, " DB_FLT_TC_CONT = %d \r\n",ini_p[DB_FLT_TC_CONT]); 
    }
    
    if(ini_p[DB_FLT_TC_EMPTY] == 1)//加热空烧保护（1->使能， 0->禁止）
    {
       // sm_log(SM_LOG_NOTICE, " DB_FLT_TC_EMPTY = %d \r\n",ini_p[DB_FLT_TC_EMPTY]);
    }else{
      //  sm_log(SM_LOG_NOTICE, " DB_FLT_TC_EMPTY = %d \r\n",ini_p[DB_FLT_TC_EMPTY]);
    }

    if(ini_p[DB_FLT_TC_BREAK] == 1)// 断针保护（1->使能， 0->禁止）
    {
       // sm_log(SM_LOG_NOTICE, " DB_FLT_TC_BREAK = %d \r\n",ini_p[DB_FLT_TC_BREAK]);
    }else{
       // sm_log(SM_LOG_NOTICE, " DB_FLT_TC_BREAK = %d \r\n",ini_p[DB_FLT_TC_BREAK]);
    }

    if(ini_p[PFUN_AUTO_CLEAN] == 1)// 自清洁功能（1->使能， 0->禁止）
    {
       // sm_log(SM_LOG_NOTICE, " PFUN_AUTO_CLEAN = %d \r\n",ini_p[PFUN_AUTO_CLEAN]);
    }else{
       // sm_log(SM_LOG_NOTICE, " PFUN_AUTO_CLEAN = %d \r\n",ini_p[PFUN_AUTO_CLEAN]);
    }

    if(ini_p[DB_SESSIONS_CLEAN] == 1)// 启动自清洁session值
    {
       // sm_log(SM_LOG_NOTICE, " DB_SESSIONS_CLEAN = %d \r\n",ini_p[DB_SESSIONS_CLEAN]);
    }else{
      //  sm_log(SM_LOG_NOTICE, " DB_SESSIONS_CLEAN = %d \r\n",ini_p[DB_SESSIONS_CLEAN]);
    }

    // 检测冷端补偿（PCB板）温度，大于预设阈值 不开启加热
    if(flag_pcba_temp_high == 1)
    {
        if(heat_start.heat_K_cood_temp > (float)ini_p[FLT_CO_JUNC_HOT_CLEAR] ||\
           p_tMontorDataInfo->det.heat_K_cood_temp > (float)ini_p[FLT_CO_JUNC_HOT_CLEAR])// 60 , PCBA过温恢复
        {
            rts |= 0x02;
            add_error_even(FLT_DE_CO_JUNC_HOT);
            sm_log(SM_LOG_ERR, " FLT_DE_CO_JUNC_HOT!\r\n");
        }else{
            flag_pcba_temp_high = 0;
            }
    }
    

	// 检测电池温度，大于预设阈值 不开启加热
#if 1 // 更新session对应的配置和比较参数
    if(p_tMontorDataInfo->bat.temperature >= ini_p[STEP1_FLT_BAT_HOT_PRE_SES + get_iniTable_session_index(p_tMontorDataInfo->session)]) // 电芯过温 51 or 48 ℃
    {
        rts |= 0x04;
        add_error_even(FLT_DE_BAT_HOT_PRE_SES);
        sm_log(SM_LOG_ERR, " FLT_DE_BAT_HOT_PRE_SES!\r\n");
    }else{
        // sm_log(SM_LOG_NOTICE, " FLT_BAT_HOT_PRE_SES = %f ,%f PASS!\r\n",(float)ini_p[FLT_BAT_HOT_PRE_SES],p_tMontorDataInfo->bat.temperature);
    }
#else
    if(p_tMontorDataInfo->bat.temperature > ini_p[FLT_BAT_HOT_PRE_SES]) // 52 , 电芯过温恢复
    {
        rts |= 0x04;
      //  set_system_err_war_tip_status(FLT_DE_BAT_HOT_PRE_SES);
      //  temp_handle = get_task_ui_handle();
       // xTaskNotifyGive(*temp_handle);
        add_error_even(FLT_DE_BAT_HOT_PRE_SES);
         sm_log(SM_LOG_ERR, " FLT_DE_BAT_HOT_PRE_SES!\r\n");
    }else{
        // sm_log(SM_LOG_NOTICE, " FLT_BAT_HOT_PRE_SES = %f ,%f PASS!\r\n",(float)ini_p[FLT_BAT_HOT_PRE_SES],p_tMontorDataInfo->bat.temperature);
    }
#endif	
    // 检测电池电压，电池小于预设阈值，不开启加热
    if (p_tMontorDataInfo->bat.remap_soc < (uint16_t)ini_p[WAR_BAT_LOW_SOC]) // < 5%
    //	(p_tMontorDataInfo->bat.voltage < (uint16_t)ini_p[WAR_BAT_LOW]))
    {
        rts |= 0x08;
      //  set_system_err_war_tip_status(FLT_DE_BAT_LOW);  
       // temp_handle = get_task_ui_handle();
      //  xTaskNotifyGive(*temp_handle);
        add_error_even(FLT_DE_BAT_LOW);
        sm_log(SM_LOG_ERR, " FLT_DE_BAT_LOW!\r\n");
    }else{
        // sm_log(SM_LOG_ERR, " WAR_BAT_LOW = %d ,%d PASS!\r\n",ini_p[WAR_BAT_LOW],p_tMontorDataInfo->bat.voltage);
    }
    // sm_log(SM_LOG_NOTICE, " safety check start  heater temp:%0.2f pcb temp:%0.2f battarey temp:%d bat voltage:%d \r\n",\
    //                     heat_start.heat_K_temp,\
    //                     heat_start.heat_K_cood_temp,\
    //                     bat_t.temperature,\
    //                     bat_t.voltage);
    return rts;
}

static uint8_t div_bat_check_cnt = 0;
static uint8_t filter_thermocouple_err_cnt = 0;
static uint8_t filter_power_err_cnt = 0;
static uint8_t filter_dcdc_err_cnt = 0;
static uint8_t filter_current_err_cnt = 0;// 电流 过流滤波
static uint8_t filter_bat_hot_err_cnt = 0;
static uint8_t filter_bat_cold_err_cnt = 0;
static uint8_t filter_pcb_hot_err_cnt = 0;
static uint8_t filter_pcb_cold_err_cnt = 0;

static uint8_t cntBatLow = 0;
//返回 是否已经保护，0：未保护，1：保护
uint8_t HalSafetyProc(HEATER *Heat_t)
{
    //INI 相关
    uint8_t rts_prt_flag = 0;// 保护，停止加热标志
    int16_t* ini_p = get_ini_val_info_handle();
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    MonitorDataInfo_t *p_tMontorDataInfo =  get_monitor_data_info_handle();
    MonitorDataInfo_t *p_debugDataInfo = get_debug_data_info_handle();
    TaskHandle_t *temp_handle;
#ifdef NONE_HEATING_PROTECT
    return 0; // 去保护
#endif
    //------------------------------计算 加热温度变化斜率--------------------------------------//
   // if(++div_k >= 10){
     //   div_k = 0;
        heater_temp_buf[2] = heater_temp_buf[1];
        heater_temp_buf[1] = heater_temp_buf[0];
        heater_temp_buf[0] = Heat_t->CurrDetectTemp;
        if(heater_temp_k_index < 3){
            heater_temp_k_index++;  
        }else{
            temp_k_add_sum +=  (heater_temp_buf[0] - heater_temp_buf[2])/0.04;
        }
        if(++add_index >= 8)// 取8次平均滤波
        {
            heater_temp_k = temp_k_add_sum / 8;
           // Heat_t->HeatTempK = heater_temp_k;
            add_index = 0;
            temp_k_add_sum = 0;
        }
    //------------------------------热电偶 开路保护检测--------------------------------------//
    if(Heat_t->HeatingTime >= 5000)
    {
        if((Heat_t->CurrDetectTemp < 100.0f) || ((p_debugDataInfo->dbgBit & DEBUG_THEAT) && (p_debugDataInfo->det.heat_K_temp <= 100.0f)))
        {
            if(++filter_thermocouple_err_cnt >= 15){ //滤波
                filter_thermocouple_err_cnt = 0;
                heat_stop(FLT_DE_THERMOCOUPLE_ERR); 
                rts_prt_flag = 1;
                add_error_even(FLT_DE_THERMOCOUPLE_ERR); //
                sm_log(SM_LOG_ERR, "FLT_DE_THERMOCOUPLE_ERR\r\n");
            }
        }else{
            filter_thermocouple_err_cnt = 0;
        }
    }
    //------------------------------加热过程电压低1--------------------------------------//
    if(Heat_t->HeatingTime >= 3000)  // 3000, 加热过程电芯低压
    {
        if(((p_debugDataInfo->dbgBit & DEBUG_VBAT) && p_debugDataInfo->bat.voltage <= ini_p[WAR_BAT_EMPTY]) || \
        	(p_tMontorDataInfo->bat.voltage <= ini_p[WAR_BAT_EMPTY]))
        {
            cntBatLow++;
            if(cntBatLow>=50)
            {
                cntBatLow = 0;
                heat_stop(FLT_DE_BAT_EMPTY); 
                rts_prt_flag = 1;              
                add_error_even(FLT_DE_BAT_EMPTY);
                sm_log(SM_LOG_ERR, "FLT_DE_BAT_EMPTY\r\n");
            }
        }else{
            cntBatLow = 0;
        }   
    }
    //------------------------------电流 过流 保护--------------------------------------//
    if(Heat_t->DetectCurrent <= ((float)ini_p[FLT_BAT_DISCHARGE_CURR_OVER])/1000.0f)//DCDC实际输出电压异常/adc检测异常
    {
        if(++filter_current_err_cnt >= 15){ //滤波
            filter_current_err_cnt = 0;
            heat_stop(FLT_DE_BAT_DISCHARGE_CURRENT_OVER); 
            rts_prt_flag = 1;
            add_error_even(FLT_DE_BAT_DISCHARGE_CURRENT_OVER);
            sm_log(SM_LOG_ERR, "FLT_DE_BAT_DISCHARGE_CURRENT_OVER\r\n");
         }
    }else{
        filter_current_err_cnt = 0;
    }
    //------------------------------电池 过温 保护--------------------------------------//
#if 1 // 更新session对应的配置和比较参数
	if (p_tMontorDataInfo->bat.temperature >= ini_p[STEP1_FLT_BAT_HOT + get_iniTable_session_index(p_tMontorDataInfo->session)]) // 58 or 56℃
	{
         if(++filter_bat_hot_err_cnt >= 50){ //滤波
             filter_bat_hot_err_cnt = 0;
             heat_stop(FLT_DE_BAT_HOT); 
             rts_prt_flag = 1;
             add_error_even(FLT_DE_BAT_HOT);
             sm_log(SM_LOG_ERR, "FLT_DE_BAT_HOT\r\n");
          }
     }else{
         filter_bat_hot_err_cnt = 0;
     }
#else
     if(p_tMontorDataInfo->bat.temperature > ini_p[FLT_BAT_HOT] ||\
        p_tMontorDataInfo->bat.temperature > ini_p[FLT_BAT_HOT])// 
     {
         if(++filter_bat_hot_err_cnt >= 50){ //滤波
             filter_bat_hot_err_cnt = 0;
             heat_stop(FLT_DE_BAT_HOT); 
             rts_prt_flag = 1;
             add_error_even(FLT_DE_BAT_HOT);
             sm_log(SM_LOG_ERR, "FLT_DE_BAT_HOT\r\n");
          }
     }else{
         filter_bat_hot_err_cnt = 0;
     }
#endif
    //------------------------------电池 低温 保护--------------------------------------//
     if(p_tMontorDataInfo->bat.temperature <= ini_p[FLT_BAT_COLD])//
     {
         if(++filter_bat_cold_err_cnt >= 50){ //滤波
             filter_bat_cold_err_cnt = 0;
             heat_stop(FLT_DE_BAT_COLD); 
             rts_prt_flag = 1;
             add_error_even(FLT_DE_BAT_COLD);
             sm_log(SM_LOG_ERR, " FLT_DE_BAT_COLD\r\n");
          }
     }else{
         filter_bat_cold_err_cnt = 0;
     }
    //------------------------------PCB 过温 保护--------------------------------------//
    if(Heat_t->detectPcbTemp >= ini_p[FLT_CO_JUNC_HOT] || \
      ((p_debugDataInfo->dbgBit & DEBUG_TPCBA) && p_debugDataInfo->det.heat_K_cood_temp >= ini_p[FLT_CO_JUNC_HOT]))//
    {
        if(++filter_pcb_hot_err_cnt >= 50){ //滤波
            filter_pcb_hot_err_cnt = 0;
            flag_pcba_temp_high = 1;
            heat_stop(FLT_DE_CO_JUNC_HOT); 
            rts_prt_flag = 1;
            add_error_even(FLT_DE_CO_JUNC_HOT);
            sm_log(SM_LOG_ERR, " FLT_DE_CO_JUNC_HOT!");
         }
    }else{
        filter_pcb_hot_err_cnt = 0;
    }
    //------------------------------加热无电流/发热体开路损坏----功率校准值参与计算判断---------------------------//
    if(Heat_t->SetPower * Heat_t->powerAdjK + Heat_t->powerAdjB >= 2.0f && Heat_t->DetectCurrent <= (float)ini_p[FLT_BAT_I_SENSE_DAMAGE]/1000) // Power >= 2 W && Discharge current <= 1A
    {
        if(++filter_power_err_cnt>=50){
            filter_power_err_cnt = 0;
            heat_stop(FLT_DE_BAT_I_SENSE_DAMAGE); 
            rts_prt_flag = 1;
             add_error_even(FLT_DE_BAT_I_SENSE_DAMAGE);
            sm_log(SM_LOG_ERR, " FLT_DE_BAT_I_SENSE_DAMAGE ERR!\r\n");
        }
    }else{
        filter_power_err_cnt = 0;
    }
    //------------------------------硬件DCDC异常--------------------------------------//
    if(Heat_t->HeatIs1Voltage < Heat_t->SetVotage - 1.5 && \
       p_tMontorDataInfo->bat.voltage > ini_p[WAR_BAT_EMPTY])//DCDC实际输出电压异常/adc检测异常 输出加热电压,检测电压比设置电压小1.5v
    {
        if(++filter_dcdc_err_cnt >= 100){
            filter_dcdc_err_cnt = 0;
            heat_stop(FLT_DE_HARDWARE_ERR); 
            rts_prt_flag = 1;
            add_error_even(FLT_DE_HARDWARE_ERR);//
            sm_log(SM_LOG_ERR, " FLT_DE_HARDWARE_ERR!\r\n");
         }
    }else{
        filter_dcdc_err_cnt = 0;
    }
	
    //------------------------------功率保护--------------------------------------//
    if(ini_p[DB_FLT_TC_PWR_LIM] > 0) 
    {
        if(Heat_t->CurrPowerVal >= ((float)ini_p[DB_FLT_TC_PWR_LIM]/100 + 1.0f) || Heat_t->SetPower > (float)ini_p[DB_FLT_TC_PWR_LIM]/100){
            heat_stop(FLT_DE_POWER_OVERLOAT); 
            rts_prt_flag = 1;
            add_error_even(FLT_DE_POWER_OVERLOAT);
            sm_log(SM_LOG_ERR, "FLT_DE_POWER_OVERLOAT\r\n");
        }
    }
	//-----------------------------失控异常保护-----------------------------------//
	if( Heat_t->HeatingTime >= 30000 )// 逻辑：30s后，测得温度 大于 目标温度50度立则即停止加热
	{
		if( Heat_t->CurrDetectTemp >= (Heat_t->CurrTargetTemp*Heat_t->tempAdjK  + Heat_t->tempAdjB) + (float)ini_p[FLT_TARGET_TEMP_DIFF] || \
            Heat_t->CurrDetectTemp <= (Heat_t->CurrTargetTemp*Heat_t->tempAdjK  + Heat_t->tempAdjB) - (float)ini_p[FLT_TARGET_TEMP_DIFF] ||\
            p_tMontorDataInfo->det.heat_K_temp >= (Heat_t->CurrTargetTemp*Heat_t->tempAdjK  + Heat_t->tempAdjB) + (float)ini_p[FLT_TARGET_TEMP_DIFF]||\
            p_tMontorDataInfo->det.heat_K_temp <= (Heat_t->CurrTargetTemp*Heat_t->tempAdjK  + Heat_t->tempAdjB) - (float)ini_p[FLT_TARGET_TEMP_DIFF])  // 模拟触发保护
		{
			heat_stop(FLT_DE_TARGET_TEMP_DIFF);
            rts_prt_flag = 1; 
            add_error_even(FLT_DE_TARGET_TEMP_DIFF);
			sm_log(SM_LOG_ERR, "FLT_DE_TARGET_TEMP_DIFF!\r\n");
		}  
	}
    //--------------------------------//发热体过温-------------------------------//
    if(Heat_t->CurrDetectTemp >= (float)ini_p[FLT_TC_ZONE_HOT] ||\
      p_tMontorDataInfo->det.heat_K_temp >= (float)ini_p[FLT_TC_ZONE_HOT])
	{   
        heat_stop(FLT_DE_TC_ZONE_HOT);
        rts_prt_flag = 1;
        add_error_even(FLT_DE_TC_ZONE_HOT);
        sm_log(SM_LOG_ERR, "FLT_DE_TC_ZONE_HOT!\r\n");
    }
	//--------------------------------(温度急剧上升)断针保护原理相同-----------------------------------//
    if(ini_p[DB_FLT_TC_BREAK] && (Heat_t->HeatMode != HEATTING_CLEAN))
    {
        if(Heat_t->HeatingTime < 200)//第1个数据
        {
            if(func1_index == 0)
            {
                func1_tbl[func1_index].time = (float)Heat_t->HeatingTime/1000.0f;
                func1_tbl[func1_index].temp = Heat_t->CurrDetectTemp;
                start_heat_t.bord_ntc_temp  = Heat_t->detectPcbTemp;
                start_heat_t.Heat_k_temp    = Heat_t->CurrDetectTemp;
                // sm_log2(SM_LOG_ERR,"%0.3f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.4f\n",
                //                     (float)Heat_t->HeatingTime/1000.0f,\
                //                     Heat_t->CurrDetectTemp,\
                //                     (Heat_t->CurrDetectTemp - Heat_t->tempAdjB)/Heat_t->tempAdjK,\
                //                     Heat_t->tempAdjB,\
                //                     Heat_t->powerAdjB,\
                //                     Heat_t->Heat_Hot_K,\
                //                     Heat_t->detectPcbTemp,
                //                     Heat_t->CurrResister);
                func1_index++;   
            }
        }
        if(Heat_t->HeatingTime - div_func1 >= 200)// 200MS 第 2 ~ 25 个数据
        {
            div_func1 = Heat_t->HeatingTime;
            if(func1_index <= 25)
            {
                func1_tbl[func1_index].time = (float)Heat_t->HeatingTime/1000.0f;
                func1_tbl[func1_index].temp = Heat_t->CurrDetectTemp;
                
                // sm_log2(SM_LOG_ERR,"%0.3f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.4f\n",
                //                     (float)Heat_t->HeatingTime/1000.0f,\
                //                     Heat_t->CurrDetectTemp,\
                //                     (Heat_t->CurrDetectTemp - Heat_t->tempAdjB)/Heat_t->tempAdjK,\
                //                     Heat_t->tempAdjB,\
                //                     Heat_t->powerAdjB,\
                //                     Heat_t->Heat_Hot_K,\
                //                     Heat_t->detectPcbTemp,
                //                     Heat_t->CurrResister);
                func1_index++;
            }else{
                if(HEATTING_STANDARD == get_ref_heat_sta())//空烧 REF Base 模式 改为第6秒停止加热
                {
                    if(Heat_t->HeatingTime >= 6000)
                    {
                        set_ref_heat_sta(0);
                        heat_stop(0);
                        rts_prt_flag = 1;  
                    }
                }
            }
        }
        if(Heat_t->HeatingTime >= 2000 && hck_t.tempIndex < HCK_T_BUF_LEN )// 2-5秒 的温度，用来求斜率
        {
            if(Heat_t->HeatingTime - tc_bk_indv >= 1000)//1秒 存一个值
            {
                tc_bk_indv = Heat_t->HeatingTime;
                
                hbk_tt_tbl[hck_t.tempIndex].time = (float)Heat_t->HeatingTime/1000.0f;
                hbk_tt_tbl[hck_t.tempIndex].temp = Heat_t->CurrDetectTemp;

               // sm_log2(SM_LOG_ERR, "[%02d] %0.3f,%0.3f\r\n",hck_t.tempIndex,hbk_tt_tbl[hck_t.tempIndex].time,hbk_tt_tbl[hck_t.tempIndex].temp);
                hck_t.tempIndex++;
            }
            
            if(hck_t.tempIndex >= HCK_T_BUF_LEN)//5采集完成
            {
                float func0 = 0.95f;
                float func1;
                float func2;
                uint16 div_func1;

                get_temp_linear(&hlt,hbk_tt_tbl,HCK_T_BUF_LEN); //2-5S 的 K 
                get_temp_linear(&hlt_0_1s,func1_tbl,6);         //0-1S 的 K
//----------------------------------REF 加热存储逻辑 start---------------------------------------------------//
            if(HEATTING_STANDARD == get_ref_heat_sta())
            {
                //保存 ref 加热数据
                int16 info_p[4] = {0};
                info_p[0] = (int16)start_heat_t.bord_ntc_temp*100;
                info_p[1] = (int16)start_heat_t.Heat_k_temp*100;
                sm_log2(SM_LOG_ERR,"save Base start pcb temp:%0.2f start tc temp:%0.2f\r\n",(float)info_p[0]/100.0f,(float)info_p[1]/100.0f);
                set_ref_tbl(Heat_t->HeatMode,info_p,func1_tbl,hlt_0_1s.temp_k,hlt.temp_k);
                bool res_val = get_ref_tbl(Heat_t->HeatMode,info_p,func1_tbl);
                if(res_val == true)
                {
                        sm_log2(SM_LOG_ERR, "FDB_AREA_F_t: %d bytes\r\n",sizeof(FDB_AREA_F_t));
                        // sm_log2(SM_LOG_ERR,"read start pcb temp:%0.2f start tc temp:%0.2f\r\n",(float)info_p[0]/100.0f,(float)info_p[1]/100.0f);
                        // for(int i=0;i<30;i++)//debug
                        // {
                        //     sm_log2(SM_LOG_ERR,"read base [%d] %0.3f,%0.3f\r\n",i,func1_tbl[i].time,func1_tbl[i].temp);
                        // }
                }
                sm_log2(SM_LOG_ERR, "k0_1 = %0.3f,k2_5 = %0.3f\r\n",hlt_0_1s.temp_k,hlt.temp_k);
                
            }else if(HEATTING_BOOST == get_ref_heat_sta())
            {
                //保存 ref 加热数据
                set_ref_heat_sta(0);
                int16 info_p[4] = {0};
                info_p[0] = (int16)start_heat_t.bord_ntc_temp*100;
                info_p[1] = (int16)start_heat_t.Heat_k_temp*100;
                sm_log2(SM_LOG_ERR,"save Boost start pcb temp:%0.2f start tc temp:%0.2f\r\n",(float)info_p[0]/100.0f,(float)info_p[1]/100.0f);
                set_ref_tbl(Heat_t->HeatMode,info_p,func1_tbl,hlt_0_1s.temp_k,hlt.temp_k);
                bool res_val = get_ref_tbl(Heat_t->HeatMode,info_p,func1_tbl);
                if(res_val == true)
                {
                        sm_log2(SM_LOG_ERR, "FDB_AREA_F_t: %d bytes\r\n",sizeof(FDB_AREA_F_t));
                        // sm_log2(SM_LOG_ERR,"read start pcb temp:%0.2f start tc temp:%0.2f\r\n",(float)info_p[0]/100.0f,(float)info_p[1]/100.0f);
                        // for(int i=0;i<30;i++)
                        // {
                        //     sm_log2(SM_LOG_ERR,"read boost [%d] %0.3f,%0.3f\r\n",i,func1_tbl[i].time,func1_tbl[i].temp);
                        // }
                }
                sm_log2(SM_LOG_ERR, "k0_1 = %0.3f,k2_5 = %0.3f\r\n",hlt_0_1s.temp_k,hlt.temp_k);
               // heat_stop(0);// 工厂表示 BOOST 加热不需要自己停止，可指令下发停止命令停止加热
               // rts_prt_flag = 1;
            }else {
                set_ref_heat_sta(0);
                if(pin_break_threshold_t.magicBase != DEF_BK_REF_MAGIC || pin_break_threshold_t.magicBoost != DEF_BK_REF_MAGIC)
                {
                    flag_pin_break = 1;
                    sm_log(SM_LOG_ERR, "FLT_DE_HAVE_NO_PIN_BREAK_THRESHOLD\r\n");
                }else{
                    if(Heat_t->HeatMode == HEATTING_STANDARD){//Base 模式
                        sm_log2(SM_LOG_ERR, "Base  heat\r\n");
                        func2 = hlt_0_1s.temp_k/pin_break_threshold_t.baseDryHeatSlope1;
                        func1 = hlt.temp_k/pin_break_threshold_t.baseDryHeatSlope2;
                        float R1 = func2;
                        float R2 = func1;
                        
                        if(R1 > 1.349f)
                        {
                            // 结果1 
                            if(R2 > 1.04f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 1\r\n");
                            }
                            // 结果2
                            if(R2 >= 0.903f && R2 <= 1.04f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 1 \r\n");
                            }
                            // 结果3
                            if(R2 > 0.89f && R2 < 0.903f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 2\r\n");
                            }
                            // 结果4
                            if(R2 >= 0.4093f && R2 <= 0.89f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 2 \r\n");
                            }
                            // 结果5
                            if(R2 < 0.4093f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 3 \r\n");
                            }
                        }else{
                            // 结果6 
                            if(R2>1.04f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 4 \r\n");
                            }
                            // 结果7 
                            if(R2 >= 0.903f && R2 <= 1.04f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 3 \r\n");
                            }
                            // 结果8 
                            if(R2 >= 0.89f && R2 < 0.903f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 4 \r\n");
                            }
                            // 结果9 
                            if(R2 >= 0.4093f && R2 < 0.89f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 5 \r\n");
                            }
                            // 结果10
                            if(R2 < 0.4093f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 5\r\n");
                            }
                        }
                    }else if(Heat_t->HeatMode == HEATTING_BOOST){
                        sm_log2(SM_LOG_ERR, "Boost heat\r\n");
                        func2 = hlt_0_1s.temp_k/pin_break_threshold_t.boostDryHeatSlope1;
                        func1 = hlt.temp_k/pin_break_threshold_t.boostDryHeatSlope2;
                        float R1 = func2;
                        float R2 = func1;
                        if(R1 > 1.356f)
                        {
                            // 结果1 
                            if(R2 > 1.04f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 1\r\n");
                            }
                            // 结果2
                            if(R2 >= 0.9334f && R2 <= 1.04f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 1 \r\n");
                            }
                            // 结果3
                            if(R2 > 0.91f && R2 < 0.9334f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 2\r\n");
                            }
                            // 结果4
                            if(R2 >= 0.4093f && R2 <= 0.91f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 2 \r\n");
                            }
                            // 结果5
                            if(R2 < 0.4093f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 3 \r\n");
                            }
                        }else{
                            // 结果6 
                            if(R2>1.04f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 4 \r\n");
                            }
                            // 结果7 
                            if(R2 >= 0.9334f && R2 <= 1.04f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 3 \r\n");
                            }
                            // 结果8 
                            if(R2 >= 0.91f && R2 < 0.9334f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 4 \r\n");
                            }
                            // 结果9 
                            if(R2 >= 0.4093f && R2 < 0.91f)
                            {
                                flag_pin_break = 0;
                                sm_log2(SM_LOG_ERR, " Good Pin 5 \r\n");
                            }
                            // 结果10
                            if(R2 < 0.4093f)
                            {
                                flag_pin_break = 1;
                                sm_log2(SM_LOG_ERR, " Broken Pin 5\r\n");
                            }
                        }
                    
                    }
                }
                
                sm_log2(SM_LOG_ERR, "func1 = %0.3f,func2 = %0.3f,k0_1 = %0.3f,k2_5 = %0.3f\r\n",\
                    func1 ,func2,hlt_0_1s.temp_k,hlt.temp_k);
               // if((hlt.temp_k > heater_bk_threshold_K || hlt.temp_k < heater_bk_threshold_LK)&& HEAT_BY_KEY == get_heat_type())  // pctool 指令方式加热，不做断针保护，因工厂需要干烧测试
                if(flag_pin_break && HEAT_BY_KEY == get_heat_type())
                {
#ifdef DEF_PIN_BROKE_EN // 启用断针保护
                    heat_stop(FLT_DE_TC_SPIKE);
                    rts_prt_flag = 1;
                    add_error_even(FLT_DE_TC_SPIKE);  
                    sm_log(SM_LOG_ERR, " FLT_DE_TC_SPIKE! \r\n");
#endif
                }
            }
//----------------------------------REF 加热存储逻辑 end---------------------------------------------------//
            }
        }
    }
    
    //--------------------------------发热体开路保护-------------------------------------//
	// if( Heat_t->HeatingTime >= 8000 )
	// {
	// 	//逻辑：8s功率段后，发热体温度小于100度 判断为发热体开路、热电偶问题
	// 	if( Heat_t->CurrDetectTemp < 100.0f )
	// 	{
	// 		heat_stop(FLT_DE_THERMOCOUPLE_ERR); 
    //         rts_prt_flag = 1;
    //         add_error_even(FLT_DE_THERMOCOUPLE_ERR); //
	// 		sm_log(SM_LOG_ERR, " Heater Hardware fault:HEAT_Stop();\r\n");
	// 	}  
	// }

    //--------------------------------功率分段限制最大功率-------------------------------------//
    if(Heat_t->HeatingTime < 12*1000)	//12秒之前
    {
        if(Heat_t->SetPower > (float)ini_p[DB_FLT_TC_PWR_LIM]/100) Heat_t->SetPower = (float)ini_p[DB_FLT_TC_PWR_LIM]/100;
    }
	else if(Heat_t->HeatingTime >= 12*1000 && Heat_t->HeatingTime < 60*1000)
	{
        if(Heat_t->SetPower > 5.5f) Heat_t->SetPower = 5.5f;
    }
	else
	{
        if(Heat_t->SetPower > 4.5f) Heat_t->SetPower = 4.5f;
    }
	//--------------------------------功率分段限制最大功率-------------------------------------//
	if((Heat_t->HeatingTime > 12*1000) && (Heat_t->HeatMode != HEATTING_CLEAN))
	{
		//200ms 累加一次
		if( ++power_i_div>=10 )
		{
			power_i_div = 0;
			
			pretect_power_cnt++;
			power_table[pretect_power_cnt] = Heat_t->CurrPowerVal; //功率滑动累加 
			if(pretect_power_cnt >= POWER_I_NUM-1)	
			{
				// 200ms累加1次，30次(6秒)
				pretect_power_cnt = 0;
			}

			float g_power_i_temp = 0;
			for(int i=0; i<POWER_I_NUM; i++ )
			{
				g_power_i_temp += power_table[i];//功率累加和
			}
			g_power_i = g_power_i_temp;
			 
			if(g_power_i > HalGetProtectCtrlPowerI(Heat_t->HeatingTime/10)*POWER_I_NUM)
			{
				protect_tick_indv = Heat_t->HeatTick;
				g_flag_protect = 1;
			//	sm_log(SM_LOG_NOTICE, "g_power_i protect 1 start at %d MS!!!\r\n",Heat_t->HeatingTime);
			}		
		}  
	} 


//原始岑写的是10
#define	protect_fixed_power_time	6
	
#define propwm_duy	
#if (0)	//旧保护逻辑
    if(g_flag_protect)
    {
        if(GetSystemTick() - protect_tick_indv < 10*1000){
            if(Heat_t->HeatingTime >= 12*1000 && Heat_t->HeatingTime < 45*1000){//12锟斤拷 - 45锟斤拷之锟斤拷
							//锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷式锟斤拷锟斤拷
                if(Heat_t->SetPower > 4.3f)Heat_t->SetPower = 4.3f;
							
            }else if(Heat_t->HeatingTime >= 45*1000){
                if(Heat_t->SetPower > 2.5f)Heat_t->SetPower = 2.5f;
            }
            fun_pid_ek_clear();
            g_power_i = 0;
        }else if(GetSystemTick() - protect_tick_indv >= 20000)//10S锟斤拷锟斤拷锟剿筹拷锟斤拷锟狡ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟铰斤拷锟叫癸拷锟斤拷
        {
            for(int i=0;i<POWER_I_NUM;i++)
            {
                power_table[i] = 0.0f;//锟斤拷锟斤拷锟桔加猴拷 锟斤拷0
            }
						g_power_i = 0;
            g_flag_protect = 0;
            printf(" g_power_i protect end at %d MS!!!\r\n",Heat_t->HeatingTime);
        }
    }
#else // 新保护逻辑
	//前10秒 PWM脉冲，脉冲功率幅值不变，10-20秒 幅值逐渐增大，当温度达到目标温度提前退出保护
    if(g_flag_protect)
    {
        if(Heat_t->HeatTick - protect_tick_indv < protect_fixed_power_time*1000)
		{
			//保护后 开启脉冲式加热
			if(++pwm_ccr <= PWM_DUTY)
			{
				Heat_t->SetPower = HalGetCurrProtectCtrlPower(Heat_t->HeatingTime/10);
				// Heat_t->SetPower = PWM_POWER_MIN;
				if(Heat_t->HeatTick - protect_tick_indv < 1*1000)
				{//第一秒 为设置保护功率的一半
				  Heat_t->SetPower = Heat_t->SetPower/2;
				}
				pwm_power = Heat_t->SetPower;
			} 
			else
			{
				Heat_t->SetPower = 0.0f;
			}

			if(pwm_ccr >= PWM_PERRIO)
			{
				//完成一个PWM功率周期
				pwm_ccr = 0;
			}							
        }else if(Heat_t->HeatTick - protect_tick_indv >= protect_fixed_power_time*1000 ) //10S过后退出限制，按正常控温进行工作
		//&&GetSystemTick() - protect_tick_indv <= 20*1000
        {
			//脉冲功率缓慢增加 从 PWM_POWER_MIN---->PWM_POWER_MAX
			//保护后 开启脉冲式加热
			if(++pwm_ccr <= PWM_DUTY)
			{
				Heat_t->SetPower = pwm_power;
			} 
			else
			{
				Heat_t->SetPower = 0.0f;
			}
			
			if(pwm_ccr >= PWM_PERRIO)
			{
			//完成一个PWM功率周期 功率逐渐加大到 PWM_POWER_MAX  STEP = 0.2W 
				pwm_ccr = 0;
				if(pwm_power < PWM_POWER_MAX)
				{
					pwm_power += 0.2;
				}
				else
				{
					pwm_power = PWM_POWER_MAX;
				}

				if(Heat_t->SetPower > PWM_POWER_MAX)
				{
					Heat_t->SetPower = PWM_POWER_MAX;
				}
				//if(Heat_t->CurrDetectTemp > Heat_t->CurrTargetTemp - 1.0f)
                if(Heat_t->CurrDetectTemp > (Heat_t->CurrTargetTemp*Heat_t->tempAdjK  + Heat_t->tempAdjB) - 1.0f)
				{// 当温度上升至 目标温度-1℃是 结束保护，PID接入
					protect_proc_break();	//结束保护
				}
			}
		}
		else if(Heat_t->HeatTick - protect_tick_indv >= 30*1000)//30S过后退出限制，按正常控温进行工作
        {
            protect_proc_break();	//结束保护
        }
    }		
#endif
return rts_prt_flag;
}

#if 0
//岑成原始功率设定
const HeatPower_t astDefProtectPOWERTbl[MAX_POWER_CTRL_POINT] = 
{
	//时间-功率
	{6000 ,420},
	{12000,420},
	{18000,400},
	{24000,400},
	{30000,400},
	{36000,400},
	{42000,400},
	{48000,400},
	{54000,400},
	{60000,400},
	{66000,400},
	{72000,400},
};
#else
const HeatPower_t astDefProtectPOWERTbl[MAX_POWER_CTRL_POINT] = 
{
	//时间-功率
	{6000 ,420},
	{12000,420},
	{15000,410},
	{18000,400},
	{24000,400},
	{30000,400},
	{36000,400},
	{42000,400},
	{54000,400},
	{60000,400}
};

#endif

#if 0
//岑成原始功率设定
//功率积分保护阈值 时间-阀值表   实际保护时 阈值时 功率*30 30累加次数
const HeatPower_t astDefProtectPOWER_I_Tbl[MAX_POWER_CTRL_POINT] = 
{
	//时锟斤拷-锟斤拷锟斤拷
	{4000 ,500},
	{5000 ,475},
	{6000 ,420},
	{7000 ,400},
	{8000 ,380},
	{9000 ,350},
	{15000,350},
	{18000,350},
	{24000,350},
	{30000,350},
	{36000,350},
	{60000,350},
};
#else
//功率积分保护阈值 时间-阀值表   实际保护时 阈值时 功率*30 30累加次数
const HeatPower_t astDefProtectPOWER_I_Tbl[MAX_POWER_CTRL_POINT] = 
{
	//时间-功率
	{4000 ,500},//485
	{5000 ,470},
	{6000 ,420},
	{7000 ,400},
	{8000 ,380},
	{9000 ,380},
	{15000,375},
	{18000,360},
	{24000,350},
	{27000,345},
	{30000,345},
	{60000,350},
};
#endif
/***************************************************************
* 函数名称: HalGetCurrCtrlPower
* 函数功能: 按照功率表计算下一时间的目标温度值
* 输入参数: HeatTime: 当前加热时间 单位：10ms
* 返回结果: 0.0 - 16.0(W)
****************************************************************/
float HalGetCurrProtectCtrlPower(unsigned int HeatTime)
{
	float rts;
	int i = 0;
	HeatPower_t* pTempTbl;
	
	pTempTbl = (HeatPower_t*)&astDefProtectPOWERTbl[0];
	for(i = 0; i < MAX_POWER_CTRL_POINT; i++)
	{
		if(HeatTime <= pTempTbl[i].time)	break;
	}
	rts = (float)pTempTbl[i].power / 100.0f;
	return rts;
}
/***************************************************************
* 函数名称: HalGetCurrCtrlPower
* 函数功能: 按照功率表计算下一时间的目标温度值
* 输入参数: HeatTime: 当前加热时间 单位：10ms
* 返回结果: 0.0 - 16.0(W)
****************************************************************/
float HalGetProtectCtrlPowerI(unsigned int HeatTime)
{
	float rts;
	int i = 0;
	HeatPower_t* pTempTbl;
	
	pTempTbl = (HeatPower_t*)&astDefProtectPOWER_I_Tbl[0];
	for(i = 0; i < MAX_POWER_CTRL_POINT; i++)
	{
		if(HeatTime <= pTempTbl[i].time)	break;
	}
	rts = (float)pTempTbl[i].power / 100.0f;
	return rts;
}

void protect_proc_break(void)
{
	fun_pid_ek_clear(pwm_power/2*0.9);	//测试发下 切换至PID 时 初始ut不能太大

	for(int i=0;i<POWER_I_NUM;i++)
	{
		power_table[i] = 0.0f;//功率累加和 清0
	}
	g_power_i = 0;
	g_flag_protect = 0;
//	sm_log(SM_LOG_NOTICE, "g_power_i protect end !!!\r\n");
}


#define DEF_BASE_THD_0_1_LOW      (400) 
#define DEF_BASE_THD_0_1_HIGH     (1349)   
#define DEF_BASE_THD_2_5_LOW      (410) 
#define DEF_BASE_THD_2_5_HIGH     (903) 


#define DEF_BOOST_THD_0_1_LOW     (400)
#define DEF_BOOST_THD_0_1_HIGH    (1560) 
#define DEF_BOOST_THD_2_5_LOW     (410) 
#define DEF_BOOST_THD_2_5_HIGH    (934)
void set_ref_tbl(uint8_t mode,int16 *info_p,RefTemp_t *refTemp_p ,float ref_k_0_1s,float ref_k_2_5s)
{
    FDB_AREA_F_u  fdb_f_u;
    if(mode == HEATTING_STANDARD)
    {
        app_param_read(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
        fdb_f_u.fdb_f_t.flag_base = DEF_BK_REF_MAGIC;
        memcpy(fdb_f_u.fdb_f_t.base_info,info_p,sizeof(fdb_f_u.fdb_f_t.base_info));
        memcpy(fdb_f_u.fdb_f_t.base_ref,refTemp_p,sizeof(func1_tbl));

        fdb_f_u.fdb_f_t.pinBreakParam.magicBase = DEF_BK_REF_MAGIC;
        fdb_f_u.fdb_f_t.pinBreakParam.baseDryHeatSlope1 = (uint32_t)(ref_k_0_1s*1000);
        fdb_f_u.fdb_f_t.pinBreakParam.baseDryHeatSlope2 = (uint32_t)(ref_k_2_5s*1000);
        fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_0_1_low = DEF_BASE_THD_0_1_LOW;     //0.01
        fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_0_1_high = DEF_BASE_THD_0_1_HIGH;   //1.089
        fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_2_5_low = DEF_BASE_THD_2_5_LOW;     //0.479
        fdb_f_u.fdb_f_t.pinBreakParam.baseThreshold_2_5_high = DEF_BASE_THD_2_5_HIGH;   //0.903
        app_param_write(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
    }else if(mode == HEATTING_BOOST)
    {
        app_param_read(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
        fdb_f_u.fdb_f_t.flag_boost = DEF_BK_REF_MAGIC;
        memcpy(fdb_f_u.fdb_f_t.boost_info,info_p,sizeof(fdb_f_u.fdb_f_t.boost_info));
        memcpy(fdb_f_u.fdb_f_t.boost_ref,refTemp_p,sizeof(func1_tbl));
        fdb_f_u.fdb_f_t.pinBreakParam.magicBoost = DEF_BK_REF_MAGIC;
        fdb_f_u.fdb_f_t.pinBreakParam.boostDryHeatSlope1 = (uint32_t)(ref_k_0_1s*1000);
        fdb_f_u.fdb_f_t.pinBreakParam.boostDryHeatSlope2 = (uint32_t)(ref_k_2_5s*1000);
        fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_0_1_low  = DEF_BOOST_THD_0_1_LOW;  //0.01
        fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_0_1_high = DEF_BOOST_THD_0_1_HIGH; //1.096
        fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_2_5_low  = DEF_BOOST_THD_2_5_LOW;  //0.479
        fdb_f_u.fdb_f_t.pinBreakParam.boostThreshold_2_5_high = DEF_BOOST_THD_2_5_HIGH; //0.933
        app_param_write(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
    }
}

bool get_ref_tbl(uint8_t mode,int16 *info_p,RefTemp_t *refTemp_p )
{
    bool ret_val = false;
    FDB_AREA_F_u  fdb_f_u;
    app_param_read(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
    if(mode == HEATTING_STANDARD)
    {
        if(fdb_f_u.fdb_f_t.flag_base == DEF_BK_REF_MAGIC)
        {
            memcpy(info_p,fdb_f_u.fdb_f_t.base_info,sizeof(fdb_f_u.fdb_f_t.base_info));
            memcpy(refTemp_p,fdb_f_u.fdb_f_t.base_ref,sizeof(func1_tbl));
            ret_val = true;
        }else{
            memset(info_p,0,sizeof(fdb_f_u.fdb_f_t.base_info));
            memset(refTemp_p,0,sizeof(func1_tbl));
            ret_val = false;
        }
    }else if(mode == HEATTING_BOOST)
    {
        if(fdb_f_u.fdb_f_t.flag_boost == DEF_BK_REF_MAGIC)
        {
            memcpy(info_p,fdb_f_u.fdb_f_t.boost_info,sizeof(fdb_f_u.fdb_f_t.boost_info));
            memcpy(refTemp_p,fdb_f_u.fdb_f_t.boost_ref,sizeof(func1_tbl));
            ret_val = true;
        }else{
            memset(info_p,0,sizeof(fdb_f_u.fdb_f_t.boost_info));
            memset(refTemp_p,0,sizeof(func1_tbl));
            ret_val = false;
        }
    }
    return ret_val;
}
bool clear_ref_tbl(void)
{
    bool ret_val = false;
    FDB_AREA_F_u  fdb_f_u;
    memset(fdb_f_u.res,0,sizeof(FDB_AREA_F_u));
    app_param_write(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
}
// 工厂 REF 加热标志位
static uint8_t flag_ref_heat_sta = 0;

uint8_t get_ref_heat_sta(void)
{
    return flag_ref_heat_sta;
}

void set_ref_heat_sta(uint8_t sta)
{
    flag_ref_heat_sta = sta;
}







