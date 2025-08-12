/**
  ******************************************************************************
  * @file    driver_detector.c
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

#include "stdio.h"
#include "string.h"
#include "driver_detector.h"
#include "sm_log.h"
#include "driver_heater.h"
#include "driver_mcu_eeprom.h"
#include "data_base_info.h"
//#define BOARD_V2   
#define BOARD_V3   

// NTC R - T 表
const TR_INFO NTC2_TEMP[] =
{
	//NTC 温度 阻值表
	{20,  68915},
	{15,  54166},
	{10,  42889},
	{5,   34196}, //以上为负温度
	{0,   27445},
	{5,   22165}, //以下为正温度
	{10,  18010},
	{15,  14720},
	{20,  12099},
	{25,  10000},
	{30,  8308},
	{35,  6939},
	{40,  5824},
	{45,  4911},
	{50,  4160},
	{55,  3540},
	{60,  3023},
	{65,  2593},
	{70,  2232},
	{75,  1929},
	{80,  1672},
	{85,  1455},
	{90,  1270},
	{95,  1111},
	{100, 976},
};

// 温度-电压查表
const float temperature_voltage_table[][2] =
{
    {-270,   -6.458},
    {-80,    -2.92},
    {-60,    -2.243},
    {-40,    -1.527},
    {-20,    -0.778},
    {0,      0},
    {20,     0.798},
    {40,     1.612},
    {60,     2.436},
    {80,     3.267},
    {100,    4.096},
    {120,    4.92},
    {140,    5.735},
    {160,    6.54},
    {180,    7.34},
    {200,    8.138},
    {250,    10.153},
    {300,    12.209},
    {350,    14.293},
    {400,    16.397},
    {450,    18.516},
    {500,    20.644},
    {550,    22.776},
    {600,    24.905},
    {650,    27.025},
    {700,    29.129},
    {800,    33.275},
    {900,    37.326},
    {1000,   41.276},
    {1200,   48.838},
    // 添加更多温度-电压对，根据实际情况
};

// 在查表中线性插值计算电压
float interpolate_voltage(float temperature)
{
    int i;
    for (i = 0; i < sizeof(temperature_voltage_table) / sizeof(temperature_voltage_table[0]) - 1; i++)
    {
        if (temperature_voltage_table[i][0] <= temperature && temperature <= temperature_voltage_table[i + 1][0])
        {
            float slope = (temperature_voltage_table[i + 1][1] - temperature_voltage_table[i][1]) / (temperature_voltage_table[i + 1][0] - temperature_voltage_table[i][0]);
            float voltage = temperature_voltage_table[i][1] + slope * (temperature - temperature_voltage_table[i][0]);
            return voltage;
        }
    }
    return 0; // 如果超出表格范围，返回0
}

// 在查表中线性插值计算温度
float interpolate_temperature(float voltage)
{
    int i;
    for (i = 0; i < sizeof(temperature_voltage_table) / sizeof(temperature_voltage_table[0]) - 1; i++)
    {
        if (temperature_voltage_table[i][1] <= voltage && voltage <= temperature_voltage_table[i + 1][1])
        {
            float slope = (temperature_voltage_table[i + 1][0] - temperature_voltage_table[i][0]) / (temperature_voltage_table[i + 1][1] - temperature_voltage_table[i][1]);
            float temperature = temperature_voltage_table[i][0] + slope * (voltage - temperature_voltage_table[i][1]);
            return temperature;
        }
    }
    return 0; // 如果超出表格范围，返回0
}

float hal_get_heat_k_temp(uint32_t opa_volt,uint32_t ntc_mV)
{

    float rts = 0.0f, volt = 0.0f,volt_cood = 0.0f;
	/* 热电偶电压=运放电压/100  运放的放大倍数是100 */
	volt = ((float)opa_volt) / 100.0f;	//热电偶电压，单位mV
    /*热电偶电压转换成温度*/
	volt_cood = interpolate_voltage(hal_get_k_cood_temp(ntc_mV));
    rts = interpolate_temperature(volt + volt_cood) ;

	return rts;//rts;

}

static DetectorInfo_t gs_tDetectorInfo;
uint8_t flag_adc_done = 0;
uint32_t gs_adc_val_tbl[16]={0};

/***************************************************************
* 函数名称: NTC_R2T
* 函数功能: NTC2电阻换算成温度，返回值为浮点型
* 输入参数: R: NTC电阻值
* 返回结果: 对应的温度值
****************************************************************/
float hal_ntc3880_R2T(unsigned int R)
{
	unsigned int i = 0;
	float rts;
	float T1,T2,R1,R2;

	if(R <= NTC2_TEMP[0].Resistor)	//温度大于-20℃，找点
	{
		for(i = 0; i < 24; i++)
		{
			if(NTC2_TEMP[i].Resistor >= R && NTC2_TEMP[i + 1].Resistor < R) break;
		}
	}
	else 	//温度小于-20℃则用-20和-15℃两个点的延长线来描曲线
	{
		T1 = NTC2_TEMP[0].Tempeture;
		T2 = NTC2_TEMP[1].Tempeture;
		R1 = NTC2_TEMP[0].Resistor;
		R2 = NTC2_TEMP[1].Resistor;
		rts = ((T2-T1)/(R1-R2))*(R-R1)-20.0;
		return rts;
	}

	if(i < 4)	//前4组为负温度
	{
		T1 = NTC2_TEMP[i].Tempeture;
		T2 = NTC2_TEMP[i + 1].Tempeture;
		R1 = NTC2_TEMP[i].Resistor;
		R2 = NTC2_TEMP[i + 1].Resistor;
		rts = (((T1-T2)/(R2-R1))*(R-R1)-T1);
	}
	else if(i < 24)	//0-100℃
	{
		T1 = NTC2_TEMP[i].Tempeture;
		T2 = NTC2_TEMP[i + 1].Tempeture;
		R1 = NTC2_TEMP[i].Resistor;
		R2 = NTC2_TEMP[i + 1].Resistor;
		rts = (((T2-T1)/(R2-R1))*(R-R1)+T1);
	}
	else	//大于100℃用95和100℃两个点的延长线描曲线
	{
		T1 = NTC2_TEMP[23].Tempeture;
		T2 = NTC2_TEMP[24].Tempeture;
		R1 = NTC2_TEMP[23].Resistor;
		R2 = NTC2_TEMP[24].Resistor;
		rts = (((T2-T1)/(R2-R1))*(R-R1)+100.0);
	}

	return rts;
}

/***************************************************************
* 函数名称: GetK_cood_temp
* 函数功能: 获取热电偶冷端温度
* 输入参数: 无
* 返回结果: 对应的温度值
****************************************************************/
float hal_get_k_cood_temp(uint32_t ntc_mV)
{
	float rts,ntc2,volt;
	float Vref = 2.800f;
	/*热电偶冷端NTC2*/
    float ntc_R =0.0f,nct_I=0.0,ntc_V=0.0;
    ntc_V = ((float) ntc_mV)/1000;
    nct_I = ((Vref - ntc_V))/10000.0f;
    ntc_R = ntc_V/nct_I;
    ntc2 = hal_ntc3880_R2T((uint16_t)ntc_R);	//冷端温度：单位℃
	return ntc2;//rts;
}
/***************************************************************
* 函数名称: GetK_cood_temp
* 函数功能: 获取热电偶冷端温度
* 输入参数: 无
* 返回结果: 对应的温度值
****************************************************************/
float hal_usb_port_temp(uint32_t ntc_mV)
{
	float rts,ntc2,volt;
	float Vref = 2.800f;
	/*热电偶冷端NTC2*/
    float ntc_R =0.0f,nct_I=0.0,ntc_V=0.0;
    ntc_V = ((float) ntc_mV)/1000;
    nct_I = ((Vref - ntc_V))/10000.0f;
    ntc_R = ntc_V/nct_I;
    ntc2 = hal_ntc3880_R2T((uint16_t)ntc_R);	//冷端温度：单位℃
	return ntc2;//rts;
}
//-----------------------------------ADC 相关-------------------------------------

/*******************************************************************************
* Global Variables
********************************************************************************/
#define SAR_IRQ_PRIORITY 	3
const cy_stc_sysint_t SAR_IRQ_cfg = {
    .intrSrc        	= pass_interrupt_sar_IRQn,
    .intrPriority   	= SAR_IRQ_PRIORITY
};

static cy_stc_syspm_callback_params_t SARDeepSleepCallbackParams =
{
	/* .base    */ SAR,
	/* .context */ NULL
};

/* Attach the Cy_SAR_DeepSleepCallback function and set the callback parameters. */
static cy_stc_syspm_callback_t SARDeepSleepCallbackStruct =
{
	/* .callback        */ (Cy_SysPmCallback)(&Cy_SAR_DeepSleepCallback),
	/* .type            */ CY_SYSPM_DEEPSLEEP,
	/* .skipMode        */ 0UL,
	/* .callbackParams  */ &SARDeepSleepCallbackParams,
	/* .prevItm         */ NULL,
	/* .nextItm         */ NULL,
	/* .order           */ 0
};
/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
void SAR_Interrupt(void);

void PDL_ADC_Init(void)
{
    cy_en_sar_status_t 	sarStatus;
   // Cy_SysAnalog_DeInit();
	/* Enable analog reference block needed by the SAR. */
	Cy_SysAnalog_Init(&Cy_SysAnalog_Fast_Local);
	Cy_SysAnalog_Enable();

	/* Configure and enable the SAR interrupt. */
	(void)Cy_SysInt_Init(&SAR_IRQ_cfg, SAR_Interrupt);
	NVIC_EnableIRQ(SAR_IRQ_cfg.intrSrc);

	/* Initialize and enable the SAR to Config0. */
	sarStatus = Cy_SAR_Init(Heat_adc_HW, &Heat_adc_config);
	if(sarStatus != CY_SAR_SUCCESS)
	{
		sm_log(SM_LOG_INFO,"SAR ADC init failed, err:%x\r\n", sarStatus);
	}
	Cy_SAR_Enable(Heat_adc_HW);

	/* Register the callback before entering Deep Sleep mode. */
	Cy_SysPm_RegisterCallback(&SARDeepSleepCallbackStruct);

	/* Initiate continuous conversions. */
	Cy_SAR_StartConvert(Heat_adc_HW, CY_SAR_START_CONVERT_CONTINUOUS);
}

uint32_t test_20ms_adc_Convert_cnt = 0;
#define  AVG_CNT_NUM	(20) //采集20次做平均
uint32_t avg_filter = 0;
uint32_t avg_add_buf[7]={0};
uint32_t chanResults[7]={0};
void SAR_Interrupt(void)
{
    uint32_t intr_status = 0u;
    int32_t chanResults[7];
    intr_status = Cy_SAR_GetInterruptStatus(SAR);
    if ((intr_status & (uint32_t) CY_SAR_INTR_EOS) == (uint32_t) CY_SAR_INTR_EOS)
    {
        /* An end of scan occured, retrieve the ADC result and do something with it here. */
    	avg_add_buf[0] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 0, Cy_SAR_GetResult16(Heat_adc_HW, 0));
    	avg_add_buf[1] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 1, Cy_SAR_GetResult16(Heat_adc_HW, 1));
    	avg_add_buf[2] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 2, Cy_SAR_GetResult16(Heat_adc_HW, 2));
    	avg_add_buf[3] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 3, Cy_SAR_GetResult16(Heat_adc_HW, 3));
    	avg_add_buf[4] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 4, Cy_SAR_GetResult16(Heat_adc_HW, 4));
    	avg_add_buf[5] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 5, Cy_SAR_GetResult16(Heat_adc_HW, 5));
        avg_add_buf[6] += Cy_SAR_CountsTo_uVolts(Heat_adc_HW, 6, Cy_SAR_GetResult16(Heat_adc_HW, 6));
    	if(++avg_filter >= AVG_CNT_NUM){
    		avg_filter = 0;
    		chanResults[0] = (avg_add_buf[0]/AVG_CNT_NUM);
    		chanResults[1] = (avg_add_buf[1]/AVG_CNT_NUM);
    		chanResults[2] = (avg_add_buf[2]/AVG_CNT_NUM);
    		chanResults[3] = (avg_add_buf[3]/AVG_CNT_NUM);
    		chanResults[4] = (avg_add_buf[4]/AVG_CNT_NUM);
    		chanResults[5] = (avg_add_buf[5]/AVG_CNT_NUM);
            chanResults[6] = (avg_add_buf[6]/AVG_CNT_NUM);
    		memcpy(gs_adc_val_tbl,chanResults,sizeof(chanResults));
    		memset(avg_add_buf,0x00,sizeof(avg_add_buf));
    		flag_adc_done = 1;
    		test_20ms_adc_Convert_cnt++;
    	}
    }
    Cy_SAR_ClearInterrupt(SAR, intr_status);

}


//-------------------------------------------------------------------------------
/**
  * @brief  初始化探测器：探测器以ADC为主
  * @param  None
  * @return None
  * @note   None
  */
int driver_detector_init(void)
{
	cy_rslt_t result;
	//IO初始化
	result = cyhal_gpio_init(CYBSP_DCDCEN_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
	result = cyhal_gpio_init(CYBSP_HEAT_NMOS_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);

	//cyhal_gpio_write(CYBSP_DCDCEN_PIN, 0);    // 为 0 则使能
	//cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN, 1); // 打开NMOS

	// ADC 初始化
	PDL_ADC_Init();
    return 0;
}

/**
  * @brief  去初始化探测器：以ADC为主
  * @param  None
  * @return None
  * @note   None
  */
int driver_detector_deinit(void)
{
#ifdef ENABLE_DRIVER_DETECTOR

#endif
    return 0;
}

/**
  * @brief  对探测器读操作
  * @param  buf:要读入的数据
            len:要读入数据的长度
  * @return 0：成功，-1：失败
  * @note   把结构体g_readVal数据拷贝到buf中
  */
int driver_detector_read(uint8_t *pBuf, uint16_t len)
{
    // P10_0----CH0----MS   电芯厂家检测
    // P10_1----CH1----VCDC 电流检测
    // P10_2----CH2----NTC_USB    USB温度检测
    // P10_3----CH3----IS4   热电偶冷端补偿
    // P10_4----CH4----IS3   热电偶检测温度
    // P10_5----CH5----IS2   DCDC-->发热体端 电压
    // P10_6----CH6----IS1   发热体低端电压

    // len要大于0
    if(len==0)  return -1;
    if(pBuf == NULL)return -1;
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	flag_adc_done =0;
	float VCDC,heating_I,heating_V,heating_R,heating_p;
	float VIS1,VIS2;

    if(p_fdb_b_info->fdb_b1_t.adcCurrAdjK < 0.95)
    {
        p_fdb_b_info->fdb_b1_t.adcCurrAdjK = 0.95;
    }
    if(p_fdb_b_info->fdb_b1_t.adcCurrAdjK > 1.05)
    {
        p_fdb_b_info->fdb_b1_t.adcCurrAdjK = 1.05;
    }

    if(p_fdb_b_info->fdb_b1_t.adcOutVdjK < 0.95)
    {
        p_fdb_b_info->fdb_b1_t.adcOutVdjK = 0.95;
    }
    if(p_fdb_b_info->fdb_b1_t.adcOutVdjK > 1.05)
    {
        p_fdb_b_info->fdb_b1_t.adcOutVdjK = 1.05;
    }

    gs_tDetectorInfo.battTypeVoltage = (float)gs_adc_val_tbl[0]/1000000;// uV转换为V

	VCDC = (float)gs_adc_val_tbl[1]/1000;// uV转换为10mVdanwei
	VIS1 = (float)gs_adc_val_tbl[6]/1000;// uV转换为mV
	VIS2 = (float)gs_adc_val_tbl[5]/1000;// uV转换为mV

	heating_I = 10*VCDC;	//电流是mA

    heating_I = heating_I * p_fdb_b_info->fdb_b1_t.adcCurrAdjK + (float)p_fdb_b_info->fdb_b1_t.adcCurrAdjB/100.0f;//校准值参与计算
    heating_I /=1000;       //电流是A

    heating_V = 2*VIS1- VIS2;//mV
   // heating_V = heating_V * p_fdb_b_info->fdb_b1_t.adcOutVdjK + (float)p_fdb_b_info->fdb_b1_t.adcOutVdjB/100.0f;//校准值参与计算
    heating_V /=1000;//V

    VIS2 = VIS2/1000;//转换为V
    if(heating_I < 0.001){//避免除数为0 ；电流很小时去计算电阻 
        heating_R = 0.62f;
    }else{
        heating_R = heating_V/heating_I;	// 计算出的单位是Ω  
    }

	heating_p = heating_V*heating_I;

	gs_tDetectorInfo.heat_V = heating_V;
	gs_tDetectorInfo.heat_I = heating_I;
	gs_tDetectorInfo.heat_R = heating_R;
	gs_tDetectorInfo.heat_P = heating_p;

    gs_tDetectorInfo.opa_adc_val      = gs_adc_val_tbl[4]/100;// opa 运放 adc采集的原始值 opa_adc_val 单位mV
	gs_tDetectorInfo.heat_K_cood_temp = hal_get_k_cood_temp(gs_adc_val_tbl[3]/1000);// V03
	gs_tDetectorInfo.usb_port_temp    = hal_usb_port_temp(gs_adc_val_tbl[2]/1000);  // v03
	gs_tDetectorInfo.heat_K_temp      = hal_get_heat_k_temp(gs_adc_val_tbl[4]/1000,gs_adc_val_tbl[3]/1000); // 03
    gs_tDetectorInfo.nmos_res         = VIS2 / heating_I ; //计算 NMOS 的内阻
    memcpy(pBuf, (uint8_t*)&gs_tDetectorInfo, sizeof(DetectorInfo_t));
    return 0;
}

/**
  * @brief  对探测器写操作
  * @param  buf:要写入的数据
            len:要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   此函数保留暂时没有对探测器写操作
  */
int driver_detector_write(uint8_t *pBuf, uint16_t len)
{
    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
    if(len==0 )  return -1;
    if(pBuf == NULL)     return -1;
    return 0;
}

