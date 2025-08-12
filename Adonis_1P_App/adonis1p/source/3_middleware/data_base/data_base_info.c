/**
  ******************************************************************************
  * @file    data_base_info.c
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "data_base_info.h"
#include "sm_log.h"
#include "platform_io.h"
#include "err_code.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "app_bt_char_adapter.h"
#include "system_status.h"
#include "system_interaction_logic.h"

LifeCycle_t g_tLifeCycle;
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

HeatParamInfo_t g_tHeatParamInfo;

ImageHeaderInfo_t g_tImageHeaderInfo; // 定义image头部全局结构体变量
 
IniInfo_t g_iniInfo;
uint32_t g_shippingMode = 0;
uint32_t g_rdpOnMode = 0;
MonitorDataInfo_t g_tMonitorDataInfo;
uint8_t funcSetup[8]; // 用于产品配置,掉电复位
KeyLockStatus_t g_keyLockStatus;
FDB_area_b1_u  g_fdb_b_u;
SessionTimeSum_u g_tSessionTimeSum;

//PCB_TEST 功率表
const HeatPower_t astDefTestPOWERTbl[MAX_POWER_CTRL_POINT] = 
{
    {100,1500},
    {200,1380},
    {300,1340},
    {400,1180},
    {500,1080},
    {600,1030},
    {700,880},
    {800,825},
    {900,650},
    {1000,600},
    {1100,600},
    {1200,550}
};
//BASE0 功率表
const HeatPower_t astDefBase0POWERTbl[MAX_POWER_CTRL_POINT] = 
{
    {100 ,1500},
    {200 ,1100},
    {300 ,1500},
    {400 ,1100},
    {500 ,1500},
    {600 ,1100},
    {700 ,1500},
    {800 ,900 },
    {900 ,700 },
    {1000,600 },
    {1100,500 },
    {1200,400 }
};

//BASE1 功率表
const HeatPower_t astDefBase1POWERTbl[MAX_POWER_CTRL_POINT] = 
{
    {100 ,1500},
    {200 ,1100},
    {300 ,1500},
    {400 ,1100},
    {500 ,1500},
    {600 ,1100},
    {700 ,1500},
    {800 ,900 },
    {900 ,700 },
    {1000,600 },
    {1100,500 },
    {1200,400 }
};

//BASE2 功率表
const HeatPower_t astDefBase2POWERTbl[MAX_POWER_CTRL_POINT] = 
{
    {100 ,1500},
    {200 ,1100},
    {300 ,1500},
    {400 ,1100},
    {500 ,1500},
    {600 ,1100},
    {700 ,1500},
    {800 ,900 },
    {900 ,700 },
    {1000,600 },
    {1100,500 },
    {1200,400 }
};

//BOOST 功率表
const HeatPower_t astDefBoostPOWERTbl[MAX_POWER_CTRL_POINT] = 
{
    {100, 1500},
    {200, 1500},
    {300, 1150},
    {400, 1500},
    {500, 1500},
    {600, 1150},
    {700, 1500},
    {800, 1500},
    {900, 700 },
    {1000,600 },
    {1100,500 },
    {1200,400 }

};

//清洁模式功率表 
const HeatPower_t astDefCleanPOWERTbl[MAX_POWER_CTRL_POINT] = 
{
    {100 ,1500},//20241105
    {200 ,1500},
    {300 ,1400},
    {400 ,1250},
    {500 ,900 },
    {600 ,850 },
    {700 ,700 },
    {800 ,500 },
    {900 ,500 },
    {1000,500 },
    {1100,500 },
    {1200,500 }
};
// TR 备用
const TrInfo_t astDefTRTbl[MAX_TR_TBL_GR] = 
{
    {15000,2550},
    {20000,2806},
    {25000,3079},
    {26000,3141},
    {28000,3256},
    {30000,3371},
    {32000,3469},
    {34000,3568},
    {36000,3696},
    {37000,3753},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};
//TEST 默认温控曲线
const HeatTemp_t acstDefHeatTestTempTbl[MAX_TEMP_CTRL_POINT] = 
{
    //时间(10毫秒)，温度(0.01摄氏度)
    {0   , 19900}, //BASE
    {1200, 19900},
    {2000, 19900},
    {4200, 19900},
    {4300, 22100},
    {6200, 22100},
    {6300, 24200},
    {8200, 24200},
    {2300, 0},
    {30000,0},
    {30000,0},
    {30000,0},
    {30000,0},
    {30000,0},
    {30000,0}
};
//base0 默认温控曲线
const HeatTemp_t acstDefHeatBase0TempTbl[MAX_TEMP_CTRL_POINT] = 
{
    //时间(10毫秒)，温度(0.01摄氏度)
    {0   , 20800}, //BASE - 20250114
    {1200, 20800},
    {2000, 20800},
    {4000, 20800},
    {7000, 21100},
    {10000,21300},
    {13000,21800},
    {16000,22300},
    {19000,22500},
    {22000,22700},
    {25000,22800},
    {27000,23100},
    {28500,23300},
    {30000,23500},
    {31500,23900}
};

//base1 默认温控曲线
const HeatTemp_t acstDefHeatBase1TempTbl[MAX_TEMP_CTRL_POINT] = 
{
    //时间(10毫秒)，温度(0.01摄氏度)
    {0   , 20800}, //BASE Medium
    {1200, 20800},
    {2000, 20800},
    {4000, 20800},
    {7000, 21100},
    {10000,21300},
    {13000,21800},
    {16000,22300},
    {19000,22500},
    {22000,22700},
    {24500,22800},
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};

//base2 默认温控曲线
const HeatTemp_t acstDefHeatBase2TempTbl[MAX_TEMP_CTRL_POINT] = 
{
    //时间(10毫秒)，温度(0.01摄氏度)
    {0   , 20800}, //BASE1 Short
    {1200, 20800},
    {2000, 20800},
    {4000, 20800},
    {7000, 21100},
    {10000,21300},
    {13000,21800},
    {16000,22300},
    {18500,22500},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};
//BOOST 默认温控曲线
const HeatTemp_t acstDefHeatBoostTempTbl[MAX_TEMP_CTRL_POINT] = 
{   
    //时间(10毫秒)，温度(0.01摄氏度)
    {0   , 22000}, //BOOST - 20250214
    {1200, 22200},
    {2000, 22000},
    {4000, 21500},
    {6000, 20700},
    {8000, 20900},
    {10000,21400},
    {12000,21900},
    {14000,22100},
    {16000,22300},
    {19000,22400},
    {21000,22700},
    {22500,22900},
    {24000,23100},
    {25500,23400}
};

//Clean 默认温控曲线
const HeatTemp_t acstDefHeatCleanTempTbl[MAX_TEMP_CTRL_POINT] = 
{   
    //时间(10毫秒)，温度(0.01摄氏度)
    {0   ,28200}, //20241105
    {800 ,28600},
    {900 ,28600},
    {1000,28600},
    {1200,28900},
    {1300,29100},
    {1400,29300},
    {1500,29700},
    {1700,30100},
    {2000,30600},
    {2400,31100},
    {3000,31600},
    {4000,32300},
    {5000,32700},
    {6000,33000}
};
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
const beepFrq_t acstDefBeepFrqTbl[3][MAX_BEEP_CTRL_POINT] = 
{
	// 第一条
	0	 ,2093, 
	500, 0,
	625, 4186,
	750, 3520,
	875, 4186,
	1000, 4699,
	1125, 0,
	1250, 0,
	1375, 4186,
	1500, 3520,
	1625, 4186,
	1750, 4699,
	1875, 0,
	2000, 0,
	2125, 0,
	2500	,2093, 
	3000, 0,
	3125, 4186,
	3250, 3520,
	3375, 4186,
	3500, 4699,
	3625, 0,
	3750, 0,
	3875, 4186,
	4000, 3520,
	4125, 4186,
	4250, 4699,
	4375, 0,
	4500, 0,
	4625, 0,

	// 第二条
	0	 ,2093, 
	500, 0,
	625, 4186,
	750, 3520,
	875, 4186,
	1000, 4699,
	1125, 0,
	1250, 0,
	1375, 4186,
	1500, 3520,
	1625, 4186,
	1750, 4699,
	1875, 0,
	2000, 0,
	2125, 0,
	2500	,2093, 
	3000, 0,
	3125, 4186,
	3250, 3520,
	3375, 4186,
	3500, 4699,
	3625, 0,
	3750, 0,
	3875, 4186,
	4000, 3520,
	4125, 4186,
	4250, 4699,
	4375, 0,
	4500, 0,
	4625, 0,

	// 第三条
	0	 ,2093, 
	500, 0,
	625, 4186,
	750, 3520,
	875, 4186,
	1000, 4699,
	1125, 0,
	1250, 0,
	1375, 4186,
	1500, 3520,
	1625, 4186,
	1750, 4699,
	1875, 0,
	2000, 0,
	2125, 0,
	2500	,2093, 
	3000, 0,
	3125, 4186,
	3250, 3520,
	3375, 4186,
	3500, 4699,
	3625, 0,
	3750, 0,
	3875, 4186,
	4000, 3520,
	4125, 4186,
	4250, 4699,
	4375, 0,
	4500, 0,
	4625, 0,


/*
    //时间(1毫秒)，frequency(1500Hz - 20000Hz)
    0   ,523, 
    500, 0,
    625, 1047,
    750, 880,
    875, 1047,
    1000, 1175,
    1125, 0,
    1250, 0,
    1375, 1047,
    1500, 880,
    1625, 1047,
    1750, 1175,
    1875, 0,
    2000, 0,
    2125, 0,
    2500, 523, 
    3000, 0,
    3125, 1047,
    3250, 880,
    3375, 1047,
    3500, 1175,
    3625, 0,
    3750, 0,
    3875, 1047,
    4000, 880,
    4125, 1047,
    4250, 1175,
    4375, 0,
    4500, 0,
    4625, 0,

    //时间(1毫秒)，frequency(1500Hz - 20000Hz)
    0   ,523, 
    500, 0,
    625, 1175,
    750, 1047,
    875, 880,
    1000, 1047,
    1125, 0,
    1250, 0,
    1375, 1175,
    1500, 1047,
    1625, 880,
    1750, 1047,
    1875, 0,
    2000, 0,
    2125, 0,
    2500   ,523, 
    3000, 0,
    3125, 1175,
    3250, 1047,
    3375, 880,
    3500, 1047,
    3625, 0,
    3750, 0,
    3875, 1175,
    4000, 1047,
    4125, 880,
    4250, 1047,
    4375, 0,
    4500, 0,
    4625, 0,

    //时间(1毫秒)，frequency(1500Hz - 20000Hz)
    0   ,4000, 
    500, 0,
    625, 4000,
    750, 0,
    875, 4000,
    1000, 0,
    1125, 4000,
    1250, 0,
    1375, 4000,
    1500, 0,
    1625, 4000,
    1750, 0,
    1875, 4000,
    2000, 0,
    2125, 4000,
    2500   ,0, 
    3000, 4000,
    3125, 0,
    3250, 4000,
    3375, 0,
    3500, 4000,
    3625, 0,
    3750, 4000,
    3875, 0,
    4000, 4000,
    4125, 0,
    4250, 4000,
    4375, 0,
    4500, 4000,
    4625, 0
*/
};
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

const StepPuffInfo_t acstDefStepPuffTbl[MAX_STEP_PUFF_POINT] = 
{
    {0,0},
};

const int16_t cIniValTbl[INI_VAL_TBL_LEN] = 
{
#if 0 // 202405 list
    3600  ,// 3600 (0~3600)mv, 震动马达RMS电压
    20    ,// 20 (0~1200)kHz , 震动马达驱动频率
    11000 ,// 11000 (mv)     , 充电Vbus最大电压
    5     ,// 5 (1~5)        , EOL降额分段设置
    1     ,// 1 (1~5)        , 下一个降额段
    2000     ,// 0              , cycle STEP1_SESSION
    2560  ,// 2500           , 充电电流  （设置到充电IC）
    100   ,// 100            , 截止电流  （电量计）
    4400  ,// 4400           , 满充电压  （电量计）
    4000  ,// 8200           , cycle STEP2_SESSION
    2000  ,// 2000           , 充电电流
    100   ,// 100            , 截止电流
    4320  ,// 4320           , 满充电压
    6000  ,// 8000           , cycle STEP3_SESSION
    2000  ,// 2000           , 充电电流
    100   ,// 100            , 截止电流
    4320  ,// 4320           , 满充电压
    8000  ,// 9000           , cycle STEP4_SESSION
    2000  ,// 2000           , 充电电流
    100   ,// 100            , 截止电流
    4320  ,// 4320           , 满充电压
    10000 ,// 10000          , cycle STEP5_SESSION
    400   ,// 400            , 充电电流
    200   ,// 200            , 预充电流
    100   ,// 100            , 截止电流
    3840  ,// 3840           , 满充电压
    51    ,// 51             , 启动加热时电芯过温
    55    ,// 55             , 充电过温
    50    ,// 50             , 充电过温恢复
    2     ,// 2              , 充电低温
    4     ,// 4              , 充电低温恢复
    58    ,// 58             , 电芯过温
    52    ,// 52             , 电芯过温恢复
    -8    ,// -8             , 电芯低温
    -6    ,// -6             , 电芯低温恢复
    3000  ,// 3000           , 加热过程电芯低压
    3580  ,// 3580 (3001~4000, 启动加热时电压低 (POC 先用电压）
    5     ,// 6 (0~99)       , 启动加热时电量低
    2000  ,// 2000           , 电芯NG电压，不可恢复 （电压改到2.0）
    500   ,// 500            , 发热体过温 (TBD)
    200   ,// 200            , 启动加热时发热体过温(TBD)
    4450  ,// 4450           , 电芯过压
    4350  ,// 4350           , 电芯过压恢复
    -7000  ,// 7000           , 加热放电过流 应该为负值
    4000  ,// 4000           , 充电过流
    80    ,// 80             , PCBA过温 (RT1-thermocouple cold NTC )
    60    ,// 60             , PCBA过温恢复
    1000  ,// 1000           , 加热无电流
    360   ,// 360            , 充电超时，单位分钟 (6h)
    70    ,// 70             , USB端口过热
    55    ,// 55             , USB端口过热恢复
    3000  ,// 3000           , case加热低压
    3150  ,// 3150           , case给pen充电低压
    1800  ,// 1800           , 功率保护，单位0.01W（设置为0则禁止该功能）
    1     ,// 1              , 连续开机保护（1->使能， 0->禁止）
    1     ,// 1              , 加热空烧保护（1->使能， 0->禁止）
    1     ,// 1              , 断针保护（1->使能， 0->禁止）
    0     ,// 0              , 自清洁功能（1->使能， 0->禁止）
    0     ,// 0              , BLE使能（1->使能， 0->禁止）
    100   ,// 100            , 启动自清洁session值
    50    ,// 50            , 加热过程中 采集温度偏离目标温差
    2900,   // 2900            , 休眠时进入shiping mode电压
    0       // 0，开启原始打印log 方便调烟、内部测试
#else // 20240803 list
    3600, // 震动马达RMS电压 (0~3600)mv
    20, // 震动马达驱动频率(0~1200)kHz
    11000, // 充电Vbus最大电压 (mv)
    3, // EOL降额分段设置
    0, // cycle
    2560, // 充电电流（设置到充电IC）
    100, // 截止电流（电量计）
    4400, // 满充电压（电量计）
	800, // 充电上限温度限流值
    45, // 充电上限温度阈值
    41, // 充电上限温度恢复值
    55, // 充电过温
    50, // 充电过温恢复
    51, // 启动加热时电芯过温
    58, // 电芯过温
    52, // 电芯过温恢复
    8200, // cycle
    2000, // 充电电流（设置到充电IC）
    100, // 截止电流
    4320, // 满充电压
	800, // 充电上限温度限流值
    43, // 充电上限温度阈值
    39, // 充电上限温度恢复值
    53, // 充电过温
    48, // 充电过温恢复
    48, // 启动加热时电芯过温
    56, // 电芯过温
    50, // 电芯过温恢复
    11000, // cycle
    400, // 充电电流（设置到充电IC）
    100, // 截止电流
    3840, // 满充电压
	320, // 充电上限温度限流值
    43, // 充电上限温度阈值
    39, // 充电上限温度恢复值
    53, // 充电过温
    48, // 充电过温恢复
    48, // 启动加热时电芯过温
    56, // 电芯过温
    50, // 电芯过温恢复
    2, // 充电低温
    4, // 充电低温恢复
    -8, // 电芯低温
    -6, // 电芯低温恢复
    3000, // 加热过程电芯低压
    3580, // 启动加热时电压低
    5, // 启动加热时电量低
    2000, // 电芯NG电压，不可恢复 （电压改到2.0）
    400, // 发热体过温 (TBD)//esaon 20240914
    200, // 启动加热时发热体过温(TBD)
    4450, // 电芯过压
    4350, // 电芯过压恢复
    -7000, // 加热放电过流
    4000, // 充电过流
    80, //  PCBA过温 (RT1-thermocouple cold NTC )
    60, // PCBA过温恢复
    1000, // 加热无电流
    360, // 充电超时，单位分钟 (6h)
    70, // USB端口过热
    55, // USB端口过热恢复
    1800, // 功率保护，单位0.01W（设置为0则禁止该功能）
    1, // 连续开机保护（1->使能， 0->禁止）
    1, // 加热空烧保护（1->使能， 0->禁止）
    1, // 断针保护（1->使能， 0->禁止）
    0, // 自清洁功能（1->使能， 0->禁止）
    1, // BLE使能（1->使能， 0->禁止）
    100, // 启动自清洁session值
    100, // 加热过程中 采集温度偏离目标温差
    2900, // 休眠时进入shiping mode电压
    0 // 原始的调试开关，加热的时候会打印log （1->使能， 0->禁止）
#endif 
};

const char cIniKeyTbl[INI_VAL_TBL_LEN + 2][5] =
{
    "V2.1" ,
    "070" ,
    "001" ,
    "002" ,
    "003" ,
    "004" ,
    "005" ,
    "006" ,
    "007" ,
    "008" ,
    "009" ,
    "010" ,
    "011" ,
    "012" ,
    "013" ,
    "014" ,
    "015" ,
    "016" ,
    "017" ,
    "018" ,
    "019" ,
    "020" ,
    "021" ,
    "022" ,
    "023" ,
    "024" ,
    "025" ,
    "026" ,
    "027" ,
    "028" ,
    "029" ,
    "030" ,
    "031" ,
    "032" ,
    "033" ,
    "034" ,
    "035" ,
    "036" ,
    "037" ,
    "038" ,
    "039" ,
    "040" ,
    "041" ,
    "042" ,
    "043" ,
    "044" ,
    "045" ,
    "046" ,
    "047" ,
    "048" ,
    "049" ,
    "050" ,
    "051" ,
    "052" ,
    "053" ,
    "054" ,
    "055" ,
    "056" ,
    "057" ,
    "058" ,
    "059" ,
    "060" ,
	"061" ,
	"062" ,
	"063" ,
	"064" ,
	"065" ,
	"066" ,
	"067" ,
	"068" ,
	"069" ,
	"070" 
};
//f 0
typedef struct ErrorCode_t {
    uint32_t magic;
    uint16_t len;
    uint16_t errCodeValTbl[ERRCODE_MAX_LEN];
    uint16_t crc;
} ErrorCode_t;

ErrorCode_t gt_errcode;

//uint16_t g_errCodeValTbl[ERRCODE_MAX_LEN]; // 上电开机时从eeprom处读出来，
// 后续如有增加错误码按顺序在最后追加， INI保存时保存版本和条数即可, 前面的位置固定不要变
const char cErrCodeKey[ERRCODE_MAX_LEN + 1][35]=
{
    "V1.2" ,
    //加热时触发的相关错误
    "FLT_DE_BAT_HOT_PRE_SES" , //              ,   //          = 101,   // 启动加热时电芯温度过高1 OK one shot error
    "FLT_DE_BAT_EMPTY" , //                    ,   //          = 102,   // 加热过程电压低1        OK, one shot error
    "FLT_DE_BAT_LOW" , //                      ,   //          = 103,   // 启动加热时电量低2      OK one shot error
    "FLT_DE_TC_SPIKE" , //                         ,   //      = 104,   // 温升过快2-------------one shot error
    "FLT_DE_THERMOCOUPLE_ERR" , //         ,   //      = 105,   // 加热温控异常 (TBD)根据发热体适配 &热偶开路/短路3  OK one shot error
    "FLT_DE_TC_ZONE_HOT" , //                      ,   //      = 106,   // 发热体过温8             OK one shot error
    "FLT_DE_TC_ZONE_HOT_PRE_SES" , //              ,   //      = 107,   // 启动加热时发热温度高＞200℃(TBD) ok   OK Wait Recover
    "FLT_DE_BAT_I_SENSE_DAMAGE" , //               ,   //      = 108,   // 加热无电流   OK one shot error
    "FLT_DE_HARDWARE_ERR" , //                     ,   //      = 109,   // 加热硬件异常 OK one shot error
    "FLT_DE_BOARD_TC_HIGH" , //                    ,   //      = 110,   // 电路板高温   OK Wait Recover
    "FLT_DE_POWER_OVERLOAT" , //                   ,   //      = 111,   // 功率过载     OK one shot error
    "FLT_DE_TARGET_TEMP_DIFF" , //                 ,   //      = 112,   // 测得温度 大于 目标温度50(ini设定)度one shot error

    //充电时触发的相关错误              
    "FLT_DE_BAT_DAMAGE" , //                       ,   //      = 201,   // 电芯损坏 <=0.9v Non Recover 实测
    "FLT_DE_CIC_OUTPUT_VOLTAGE" , //               ,   //      = 202,   // 充电电芯过压Leve0 ＞4.63V 不可恢复 Non recover
    "FLT_DE_CIC_CONFIG_ERROR" , //                 ,   //      = 203,   // 充电芯片iic通信错误 Reset recover
    "FLT_DE_BAT_CHARGE_CURRENT_OVER" , //          ,   //      = 204,   // 充电过流 Reset recover
    "FLT_DE_BAT_HOT_CHARGE" , //	               ,   //        = 205,   // 充电-电芯过温 Wait Recover
    "FLT_DE_BAT_COLD_CHARGE" , //                  ,   //      = 206,    // 充电-电芯低温 Wait Recover
    "FLT_DE_BAT_VOLTAGE_OVER" , //                 ,   //      = 207,    // 充电-电芯过压≥4.45V Wait Recover
    "FLT_DE_CHG_VBUS" , //                         ,   //      = 208,    // 充电-Vbus电压高 show wrong charge ui and stop charging
    "FLT_DE_CIC_CHARGE_TIMEOUT" , //               ,   //      = 209,    // 充电超时＞6H

    // 任何时候触发的错误               
    "FLT_DE_BAT_DISCHARGE_CURRENT_OVER" , //       ,   //      = 301,     // 放电过流
    "FLT_DE_BAT_HOT" , //    FLT_DE_BAT_HOT                      ,   //      = 302,     // 电芯过温
    "FLT_DE_BAT_COLD" , //                         ,   //      = 303,     // 电芯低温
    "FLT_DE_END_OF_LIFE" , //                      ,   //      = 304,     // 电芯EOL ≥ 10000
    "FLT_DE_CO_JUNC_HOT" , //                      ,   //      = 305,     // PCB过温≥80℃
    "FLT_DE_USB_HOT" , //                          ,   //      = 306,     // USB端口过热
};

const uint16_t cErrCodeKeyNum[ERRCODE_MAX_LEN]= // 位置固定不要变,如有增加，在最后追加
{
    //加热时触发的相关错误
    101,   // 启动加热时电芯温度过高1 OK one shot error
    102,   // 加热过程电压低1        OK, one shot error
    103,   // 启动加热时电量低2      OK one shot error
    104,   // 温升过快2-------------one shot error
    105,   // 加热温控异常 (TBD)根据发热体适配 &热偶开路/短路3  OK one shot error
    106,   // 发热体过温8             OK one shot error
    107,   // 启动加热时发热温度高＞200℃(TBD) ok   OK Wait Recover
    108,   // 加热无电流   OK one shot error
    109,   // 加热硬件异常 OK one shot error
    110,   // 电路板高温   OK Wait Recover
    111,   // 功率过载     OK one shot error
    112,   // 测得温度 大于 目标温度50(ini设定)度one shot error

    //充电时触发的相关错误              
    201,   // 电芯损坏 <=0.9v Non Recover 实测 12
    202,   // 充电电芯过压Leve0 ＞4.63V 不可恢复 Non recover 13
    203,   // 充电芯片iic通信错误 Reset recover
    204,   // 充电过流 Reset recover
    205,   // 充电-电芯过温 Wait Recover
    206,    // 充电-电芯低温 Wait Recover
    207,    // 充电-电芯过压≥4.45V Wait Recover
    208,    // 充电-Vbus电压高 show wrong charge ui and stop charging
    209,    // 充电超时＞6H

    // 任何时候触发的错误
    301,     // 放电过流
    302,     // 电芯过温
    303,     // 电芯低温
    304,     // 电芯EOL ≥ 10000
    305,     // PCB过温≥80℃
    306,     // USB端口过热
};

HeatParamInfo_t* get_heat_param_info_handle(void)
{
    return &g_tHeatParamInfo;
}

/**
  * @brief  获取镜像头部指针
  * @param  None
  * @return 返回镜像头部指针
  * @note   None
  */
ImageHeaderInfo_t* get_image_header_info_handle(void)
{
    return &g_tImageHeaderInfo;
}

/**
  * @brief  获取ini val tbl指针
  * @param  None
  * @return 返回ni val tbl指针
  * @note   None
  */
int16_t* get_ini_val_info_handle(void)
{
#ifdef INI_CONFIG_DEFAULT_EN
    return cIniValTbl;
#else
    return g_fdb_b_u.fdb_b1_t.iniValTbl;
#endif
}

/**
  * @brief  获取errCode val tbl指针
  * @param  None
  * @return 返回errCode val tbl指针
  * @note   None
  */
uint16_t* get_errCode_val_info_handle(void)
{
    return gt_errcode.errCodeValTbl;
}

SessionTimeSum_v2_t *get_session_time_sum_info_handle(void)
{
	return &(g_tSessionTimeSum.sessionTimeSum_v2);
}

uint8_t g_tImageMainRam[IMAGE_SEZE];
uint8_t g_tImageSecondRam[IMAGE_SEZE];

void gui_init(void)
{
    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
}

uint8_t* get_ram_main_gui(void)
{
    return &g_tImageMainRam[0];
}
uint8_t* get_ram_second_gui(void)
{
    return &g_tImageSecondRam[0];
}

uint8_t g_iap4kBuf[IAP_4K_BUF_LEN];
uint8_t* get_iap_4k_buf(void)
{
    return &g_iap4kBuf[0];
}

uint8_t g_iapCombinBuf[IAP_COMBINE_BUF_LEN];
uint8_t* get_iap_combine_buf(void)
{
    return &g_iapCombinBuf[0];
}

IniInfo_t* get_iniInfo_handle(void)
{
    return &g_iniInfo;
}

uint32_t get_shipping_mode(void)
{
    return g_fdb_b_u.fdb_b1_t.shippingMode;
}

void set_shipping_mode(uint32_t mode)
{
    uint32_t shippingMode = mode;

	if (mode == 1)				// 船运模式关闭蓝牙
	{
		SysSubStatus_u subSystemStatus = get_subSystem_status();
		if (SUBSTATUS_PARING_QRCODE == subSystemStatus || SUBSTATUS_PARING == subSystemStatus)
		{
			set_system_external_even(EXT_EVENT_PARING_CANCLE);
		}
		else if (SUBSTATUS_FIND_ME == subSystemStatus)
		{
			set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
		}
		else if (SUBSTATUS_LOADING == subSystemStatus)
		{
			set_system_external_even(EXT_EVENT_LOADING_STOP);		
		}
	}
	
    app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    g_fdb_b_u.fdb_b1_t.shippingMode = mode;
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
}

uint32_t get_rdp_on_mode(void)
{
    return g_fdb_b_u.fdb_b1_t.rdpon;
}

uint32_t g_rdpon_ram = 0;
void set_rdp_on_mode_no_save_flash(uint32_t mode)
{
    g_rdpon_ram = mode;
}

uint32_t get_rdp_on_mode_no_save_flash(void)
{
    return g_rdpon_ram;
}

uint8_t get_device_color(void)
{
	FDB_AREA_E_t tData;
	if (app_param_read(INDEX_E, &tData, sizeof(FDB_AREA_E_t)) == FALSE)
	{
		tData.color = 0xff;
		app_param_write(INDEX_E, &tData, sizeof(FDB_AREA_E_t));
	}
	return tData.color;
}

void set_device_color(uint8_t color)
{
	FDB_AREA_E_t tData;
	app_param_read(INDEX_E, &tData, sizeof(FDB_AREA_E_t));
	tData.color = color;
	app_param_write(INDEX_E, &tData, sizeof(FDB_AREA_E_t));
}

extern void cy_disable_debug(void);
void set_rdp_on_mode(uint32_t mode)
{
    app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    g_fdb_b_u.fdb_b1_t.rdpon = mode;
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    set_rdp_on_mode_no_save_flash(mode);
#ifdef DE_RDP_ON_EN
    cy_disable_debug();
#endif
}

MonitorDataInfo_t* get_monitor_data_info_handle(void)
{
    return &g_tMonitorDataInfo;
}

FDB_area_b1_u* get_fdb_b_info_handle(void)
{
    return &g_fdb_b_u; 
}
/**
  * @brief  初始化默认的加热参数
  * @param  None
  * @return None
  * @note   None
  */
 
void heat_param_info_init(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    
   // memcpy((uint8_t *)&(p_tHeatParamInfo->tTrInfo[0].tempeture), (uint8_t *)&(astDefTRTbl[0].tempeture), sizeof(TrInfo_t) * MAX_TR_TBL_GR);

   // memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestPower[0].time), (uint8_t *)&(astDefTestPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);

    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBasePower[0].time), (uint8_t *)&(astDefBase0POWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostPower[0].time), (uint8_t *)&(astDefBoostPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
   // memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanPower[0].time), (uint8_t *)&(astDefCleanPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);

   // memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestTemp[0].time), (uint8_t *)&(acstDefHeatTestTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBaseTemp[0].time), (uint8_t *)&(acstDefHeatBase0TempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostTemp[0].time), (uint8_t *)&(acstDefHeatBoostTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
    
    memcpy((uint8_t *)&(p_tHeatParamInfo->tPuffInfo.tStepPuffInfo[0].timePart), (uint8_t *)&(acstDefStepPuffTbl[0].timePart), sizeof(StepPuffInfo_t) * MAX_STEP_PUFF_POINT);
    
    p_tHeatParamInfo->tPuffInfo.maxPuff     = DEFAULT_MAX_PUFF;
    p_tHeatParamInfo->tPuffInfo.preHeatTime = DEFAULT_PREHEAT_TIME;
    p_tHeatParamInfo->tPuffInfo.maxHeatTime = DEFAULT_HEAT_TIME;

//    p_tHeatParamInfo->smokeBaseTotalTime    = DEF_SMOKE_TOTAL_TIME;
//    p_tHeatParamInfo->smokeBoostTotalTime   = DEF_SMOKE_TOTAL_TIME;
//    p_tHeatParamInfo->smokeSuctionSum       = DEF_SUCTION_SUM_CNT;

    p_tHeatParamInfo->validCheck = 0xAACC;

}

//------------------------------------------------------------------------------
#define  DEF_SN      "0123456789ABCDEF"
#define  DEF_INI_STR "PAA0000000000000"
void fdb_area_b_init(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    bool res;
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    sm_log(SM_LOG_ERR, "fdb_area_b_init() %x\r\n");
    res = app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    if (res != true) {
        sm_log(SM_LOG_ERR, "CY_FLASH_ERR_CODE:   %x\r\n", result);
    }
    if((g_fdb_b_u.fdb_b1_t.magic == 0xaa5555aa) || (g_fdb_b_u.fdb_b1_t.magic == 0xaa6666aa))
    {
        sm_log2(SM_LOG_ERR, "\r\n Area b read%d bytes ,fdb_b1_t.magic = 0x%x \r\n",sizeof(FDB_are_b1_Info_t),g_fdb_b_u.fdb_b1_t.magic);

        memcpy((uint8_t *)&(p_tHeatParamInfo->tTrInfo[0].tempeture), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tTrInfo[0].tempeture), sizeof(TrInfo_t) * MAX_TR_TBL_GR);

        memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestPower[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatTestPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
        memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestTemp[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatTestTemp[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);

 #ifdef FACTORY_PROD
            sm_log(SM_LOG_ERR, "--------------FACTORY_PROD load default clean profile ----------\r\n");
            memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanPower[0].time), (uint8_t *)&(astDefCleanPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanTemp[0].time), (uint8_t *)&(acstDefHeatCleanTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);

            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanPower[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanTemp[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanTemp[0].time), sizeof(HeatPower_t) * MAX_TEMP_CTRL_POINT);
#else
            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanPower[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanTemp[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanTemp[0].time), sizeof(HeatPower_t) * MAX_TEMP_CTRL_POINT);
#endif       

        // sm_log(SM_LOG_INFO, "Test temp profile:\r\n");
        // for(int i=0; i<MAX_TEMP_CTRL_POINT;i++)
        // {
        //     sm_log(SM_LOG_INFO, "%d    %d\r\n",p_tHeatParamInfo->tHeatTestTemp[i].time,p_tHeatParamInfo->tHeatTestTemp[i].tempeture);
        // }
        if(g_fdb_b_u.fdb_b1_t.adcCurrAdjK > 1.3f &&  g_fdb_b_u.fdb_b1_t.adcCurrAdjK < 0.7)// 防止旧版本升新版本 值错误问题
        {
            g_fdb_b_u.fdb_b1_t.adcCurrAdjK = 1;
            g_fdb_b_u.fdb_b1_t.adcCurrAdjB = 0;
            sm_log(SM_LOG_INFO, "CurrAdj ERR ! adcCurrAdjK write 1.0, adcCurrAdjB write 0\r\n");
            //写入校准值 
        }
        
#ifdef INI_CONFIG_DEFAULT_EN
    int j = 0;
    sm_log(SM_LOG_INFO, "ini info  default val\r\n");
    // 把默认val写到存储区
    memcpy((uint8_t *)(g_fdb_b_u.fdb_b1_t.iniValTbl), (uint8_t *)(cIniValTbl), sizeof(int16_t) * INI_VAL_TBL_LEN);
    g_iniInfo.valTotalLen = sizeof(cIniValTbl);
    j = 0;
    for (int i = 0; i < INI_VAL_TBL_LEN + 2; i++) {
        j += (strlen(cIniKeyTbl[i]) + 1);
    }
    g_iniInfo.keyTotalLen = j;
#else

#endif
        set_rdp_on_mode_no_save_flash(g_fdb_b_u.fdb_b1_t.rdpon);
    }else { // 第一次上电 载入默认值
        g_fdb_b_u.fdb_b1_t.magic = 0xaa5555aa;
        g_fdb_b_u.fdb_b1_t.rdpon = 0;
        set_rdp_on_mode_no_save_flash(g_fdb_b_u.fdb_b1_t.rdpon);
        g_fdb_b_u.fdb_b1_t.shippingMode = 0;
         
        memset(g_fdb_b_u.fdb_b1_t.snNumber,0, sizeof(g_fdb_b_u.fdb_b1_t.snNumber));// 清空产品序列号

        memcpy(g_fdb_b_u.fdb_b1_t.iniKeyString, (uint8_t*)DEF_INI_STR, sizeof(g_fdb_b_u.fdb_b1_t.iniKeyString));// ini 标识字符串
        memcpy((uint8_t *)(g_fdb_b_u.fdb_b1_t.iniValTbl), (uint8_t *)(cIniValTbl), sizeof(int16_t) * INI_VAL_TBL_LEN);//INI
        
        //ADJ 相关
        g_fdb_b_u.fdb_b1_t.tempAdjB  = DEFAULT_ADJ_TEMP_B;
        g_fdb_b_u.fdb_b1_t.tempAdjK  = DEFAULT_ADJ_TEMP_K;
        
        g_fdb_b_u.fdb_b1_t.powerAdjB = DEFAULT_ADJ_POWER_B;
        g_fdb_b_u.fdb_b1_t.powerAdjK = DEFAULT_ADJ_POWER_K;
        
        g_fdb_b_u.fdb_b1_t.opaAdjB   = DEFAULT_ADJ_OPA_B;
        g_fdb_b_u.fdb_b1_t.opaAdjK   = DEFAULT_ADJ_OPA_K;

        g_fdb_b_u.fdb_b1_t.adcCurrAdjB = DEFAULT_ADJ_Curr_B;
        g_fdb_b_u.fdb_b1_t.adcCurrAdjK = DEFAULT_ADJ_Curr_K;

        g_fdb_b_u.fdb_b1_t.adcOutVdjB  = DEFAULT_ADJ_OutV_B;
        g_fdb_b_u.fdb_b1_t.adcOutVdjK  = DEFAULT_ADJ_OutV_K;
        // 备用 TR表
        memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tTrInfo[0].tempeture), (uint8_t *)&(astDefTRTbl[0].tempeture), sizeof(TrInfo_t) * MAX_TR_TBL_GR);
        // 测试 功率表
        memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatTestPower[0].time), (uint8_t *)&(astDefTestPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
        // 测试温控曲线
        memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatTestTemp[0].time), (uint8_t *)&(acstDefHeatTestTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
        //
        
        memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanPower[0].time), (uint8_t *)&(astDefCleanPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
        memcpy((uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanTemp[0].time), (uint8_t *)&(acstDefHeatCleanTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
        
        //搬取到 加热参数结构体
        memcpy((uint8_t *)&(p_tHeatParamInfo->tTrInfo[0].tempeture), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tTrInfo[0].tempeture), sizeof(TrInfo_t) * MAX_TR_TBL_GR);

        memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestPower[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatTestPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
        memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestTemp[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatTestTemp[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);

        memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanPower[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
        memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanTemp[0].time), (uint8_t *)&(g_fdb_b_u.fdb_b1_t.tHeatCleanTemp[0].time), sizeof(HeatPower_t) * MAX_TEMP_CTRL_POINT);

        g_fdb_b_u.fdb_b1_t.len  = sizeof(FDB_are_b1_Info_t);

       
        res = app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
        sm_log2(SM_LOG_ERR, "--------------------------app_param_write(B)------------------------------\r\n");
        if (res != true) {
            sm_log2(SM_LOG_ERR, "CY_FLASH_ERR_CODE: %x\r\n", result);
        }else{
            sm_log2(SM_LOG_ERR, "area b1 write %d bytes  ok \r\n", sizeof(FDB_are_b1_Info_t));
            app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
            sm_log2(SM_LOG_ERR, "\r\n read again g_fdb_b_u.fdb_b1_t.magic = 0x%x \r\n",g_fdb_b_u.fdb_b1_t.magic);
        }
    }
    
}

//-------------------------------------------------------------------------------------
/**
  * @brief  初始化默认的errcode参数
  * @param  None
  * @return None
  * @note   None
  */
void errcode_val_tbl_init(void)
{
    bool res = app_param_read(INDEX_D_1, (uint8_t *)&gt_errcode, sizeof(gt_errcode));
    if (res == false) { // 未被保存过，则清空并写进flash中
         sm_log(SM_LOG_INFO, "errcode_val_tbl_init\r\n");
        memset((uint8_t *)&gt_errcode, 0, sizeof(gt_errcode));
        app_param_write(INDEX_D_1, (uint8_t *)&gt_errcode, sizeof(gt_errcode));
    }
}

void errcode_update_to_flash(void)
{
    app_param_write(INDEX_D_1, (uint8_t *)&gt_errcode, sizeof(gt_errcode));
}

void errcode_clr_to_flash(void)
{
    memset((uint8_t *)&gt_errcode, 0, sizeof(gt_errcode));
    app_param_write(INDEX_D_1, (uint8_t *)&gt_errcode, sizeof(gt_errcode));
}

void session_time_sum_init(void)
{
	bool res;
	if (app_param_read(INDEX_D_3, (uint8_t *)&g_tSessionTimeSum, sizeof(SessionTimeSum_v2_t)))		// 新版本读正确
	{
        sm_log(SM_LOG_INFO, "session_time_sum_init V2\r\n");
	}
	else if (app_param_read(INDEX_D_3, (uint8_t *)&g_tSessionTimeSum, sizeof(SessionTimeSum_v1_t)))	// 旧版本读正确
	{
		sm_log(SM_LOG_INFO, "session_time_sum_init V1\r\n");
		g_tSessionTimeSum.sessionTimeSum_v2.baseTimeSumClean = 0;
		g_tSessionTimeSum.sessionTimeSum_v2.boostTimeSumClean = 0;	
	}
	else
	{ // 未被保存过，则清空并写进flash中
        sm_log(SM_LOG_INFO, "session_time_sum_init\r\n");
        memset((uint8_t *)&g_tSessionTimeSum, 0, sizeof(SessionTimeSum_v2_t));
//		app_param_write(INDEX_D_3, (uint8_t *)&g_tSessionTimeSum, sizeof(g_tSessionTimeSum));
    }
	
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    p_tHeatParamInfo->smokeSuctionSum = g_tSessionTimeSum.sessionTimeSum_v2.suctionSum;
    p_tHeatParamInfo->smokeBoostTotalTime = g_tSessionTimeSum.sessionTimeSum_v2.boostTimeSum;
    p_tHeatParamInfo->smokeBaseTotalTime = g_tSessionTimeSum.sessionTimeSum_v2.baseTimeSum;
}

void session_time_sum_update_to_flash(void)
{
    app_param_write(INDEX_D_3, (uint8_t *)&g_tSessionTimeSum, sizeof(SessionTimeSum_v2_t));
}

/**
  * @brief  初始化eeprom参数
  * @param  None
  * @return None
  * @note   None
  */
void init_mcu_flash_param(void)
{
    static uint8_t mcu_flash_init_flag = 0;
    if (0 == mcu_flash_init_flag)						// 保证只上电时扫行一次，后续休眠后不需要执行
    {
    	mcu_flash_init_flag = 1;
    // ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
    // E2PDataType_t e2pTemp;

    // e2pTemp.addr = E2P_ADDR_OF_SHIPPING_MODE;
    // e2pTemp.pDate = (uint8_t*)&g_shippingMode; //
    // e2pTemp.size = E2P_SIZE_OF_SHIPPING_MODE;
    // e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));
    // if (g_shippingMode != 1 && g_shippingMode != 0) {
    //     g_shippingMode = 0;
    //     e2pTemp.addr = E2P_ADDR_OF_SHIPPING_MODE;
    //     e2pTemp.pDate = (uint8_t*)&g_shippingMode; //
    //     e2pTemp.size = E2P_SIZE_OF_SHIPPING_MODE;
    //     e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));
    // }

    // e2pTemp.addr = E2P_ADDR_OF_RDP_ON;
    // e2pTemp.pDate = (uint8_t*)&g_rdpOnMode; //
    // e2pTemp.size = E2P_SIZE_OF_RDP_ON;
    // e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));
    // if (g_rdpOnMode != 1 && g_rdpOnMode != 0) {
    //     g_rdpOnMode = 0;
    //     e2pTemp.addr = E2P_ADDR_OF_RDP_ON;
    //     e2pTemp.pDate = (uint8_t*)&g_rdpOnMode; //
    //     e2pTemp.size = E2P_SIZE_OF_RDP_ON;
    //     e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));
    // }
        fdb_area_b_init();
        errcode_val_tbl_init();
        sm_log(SM_LOG_INFO, "finish errcode_val_tbl_init\r\n");
        session_time_sum_init();

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	// add by vincent.he 20241105
        char devSN[32];
        memset(devSN, 0, sizeof(devSN));
        memcpy(devSN, g_fdb_b_u.fdb_b1_t.snNumber, 16);
        sm_log(SM_LOG_DEBUG, "get flash SN(%d): %s\n", strlen(devSN), devSN);

        if (strlen(devSN) == 0)
        {
            ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
            E2PDataType_t e2pTemp;

            memset(devSN, 0, sizeof(devSN));

            e2pTemp.addr = E2P_ADDR_OF_SN;
            e2pTemp.pDate = devSN;
            e2pTemp.size = E2P_SIZE_OF_SN;//E2P_SIZE_OF_SN
            e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));
            
            uint8_t devSN_len = 0;
            devSN_len = strlen(devSN);// = strlen(devSN);
            sm_log(SM_LOG_DEBUG, "get E2PROM SN(%d): %s\n", devSN_len, devSN);

            if (devSN_len == 16)//16bytes SN
            {
                sm_log(SM_LOG_DEBUG, "Copy E2PROM SN to Flash(%d): %s\n", devSN_len, devSN);

                app_param_read(INDEX_B, g_fdb_b_u.res, sizeof(FDB_are_b1_Info_t));
                memcpy(g_fdb_b_u.fdb_b1_t.snNumber,devSN,16);
                app_param_write(INDEX_B, g_fdb_b_u.res, sizeof(FDB_are_b1_Info_t));
            }
        }

	// add by Gison 20241026
		Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
		if (false == app_param_read(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t)))
		{
			memset((uint8_t *)pBleData, 0, sizeof(Ble_Flash_Data_t));	// 若读出数据无效，全清零
		}

    	life_cycle_info_init();
    	get_beep_mode_from_eeprom();
    	get_motor_strength_from_eeprom();
    	get_oled_brightness_from_eeprom();
    	get_lock_mode_from_eeprom();
		get_clean_prompt_from_eeprom();
		get_eos_prompt_from_eeprom();
		get_last_error_from_eeprom();
		init_errCode_info_handle();
    	get_time_stamp_from_flash();
    //		session_record_init();						// 该部分代码需要迁移至 system_param_info_init，因为SQPI在EEPROM后面初始化
    //		event_record_init();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    }
}
//蓝牙广播开关
#define BT_ADV_EN       0
static uint8_t g_flag_adv_en  = 0;
void bt_adv_set_en(uint8_t en)
{
#ifndef INI_CONFIG_DEFAULT_EN // 没有定义默认值，则需要保存  need debug
    ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
    E2PDataType_t e2pTemp;
    if(en){
        g_fdb_b_u.fdb_b1_t.iniValTbl[PFUN_BLE_EN] = 1;
    }
    else{
        g_fdb_b_u.fdb_b1_t.iniValTbl[PFUN_BLE_EN] = 0;
    }
    e2pTemp.addr = E2P_ADDR_OF_INI_VAL_TBL;
    e2pTemp.pDate = (uint8_t*)g_fdb_b_u.fdb_b1_t.iniValTbl; //
    e2pTemp.size = E2P_SIZE_OF_INI_VAL_TBL;
    e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));
#endif
}
uint8_t bt_get_adv_en(void)
{
    int16_t *pVal = get_ini_val_info_handle();
    return pVal[PFUN_BLE_EN];
}

void set_clean_status(bool status)
{
    if (status) {
        g_fdb_b_u.fdb_b1_t.iniValTbl[PFUN_AUTO_CLEAN] = 1;
    } else {
        g_fdb_b_u.fdb_b1_t.iniValTbl[PFUN_AUTO_CLEAN] = 0;
    }
    
}

bool get_clean_status(void)
{
    if (g_fdb_b_u.fdb_b1_t.iniValTbl[PFUN_AUTO_CLEAN] == 0) {
        return false;
    } else {
        return true;
    }
}

bool procotol_func_setup(uint8_t* pBuf, uint16_t opType)
{
	if (opType == 1) { // set
		memcpy(funcSetup, pBuf, 8);
	} else { // get
		memcpy(pBuf, funcSetup, 8);
	}

	return true;
}

uint8_t* get_func_setup(void)
{
	return (uint8_t *)funcSetup;
}

int16_t get_sessions_clean(void)
{
    return g_fdb_b_u.fdb_b1_t.iniValTbl[DB_SESSIONS_CLEAN] ;
}

/**
  * @brief  ui 数据安全验证
  * @param  dataLen：数据长度，addrLen：地址长度
  * @return true:数据合法，false:数据非法
  * @note   None
  */
bool ui_data_security_verify(uint32_t dataLen, uint32_t addrLen)
{
    if (dataLen > IMAGE_SEZE) {
        sm_log(SM_LOG_ERR, "ui_data_security_verify,dataLen:%u\r\n", dataLen);
        return false;
    }
    if (addrLen > QSPI_MAX_ADDR_LEN) {
        sm_log(SM_LOG_ERR, "ui_data_security_verify,addrLen:%u\r\n", addrLen);
        return false;
    }
    if (get_update_ui_timer_cnt() > 0) {
        amoled_display_clear();
        return false;
    }
    return true;
}

// 坐标起点等于则不合法(坐标从0开始)，宽高度等于则合法（宽高度从1开始）
bool ui_pos_securitu_verify(uint16_t imageHeightStart, uint16_t imageHeightEnd, uint16_t imageWitdthStart, uint16_t imageWitdthEnd) 
{
    if (imageHeightStart >= UI_VER_RES) { 
        sm_log(SM_LOG_ERR, "ui_data_security_verify,imageHeightStart:%u\r\n", imageHeightStart);
        return false;
    }
    if (imageHeightEnd > UI_VER_RES) { 
        sm_log(SM_LOG_ERR, "ui_data_security_verify,imageHeightEnd:%u\r\n", imageHeightEnd);
        return false;
    }
    if (imageWitdthStart >= UI_HOR_RES) { 
        sm_log(SM_LOG_ERR, "ui_data_security_verify,imageWitdthStart:%u\r\n", imageWitdthStart);
        return false;
    }
    if (imageWitdthEnd > UI_HOR_RES) { 
        sm_log(SM_LOG_ERR, "ui_data_security_verify,imageWitdthEnd:%u\r\n", imageWitdthEnd);
        return false;
    }
    return true;
}

/**
  * @brief  获取按键是否被锁定
  * @param  None
  * @return 按键锁定状态指针
  * @note   None
  */
KeyLockStatus_t *get_key_lock_status(void)
{
    return &g_keyLockStatus;
}

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
/**
  * @brief  get_life_cycle_handle
  * @param  None
  * @return 返回头部指针
  * @note   None
  */
LifeCycle_t* get_life_cycle_handle(void)
{
    return &g_tLifeCycle;
}

/**
  * @brief  life_cycle_info_init
  * @param  None
  * @return
  * @note   None
  */
void life_cycle_info_init(void)
{
#if 0
    ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
    E2PDataType_t e2pTemp;

    e2pTemp.addr = E2P_ADDR_OF_LIFE_CYCLE;
    e2pTemp.pDate = (uint8_t*)&g_tLifeCycle; //
    e2pTemp.size = sizeof(LifeCycle_t);
    e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));

    if (g_tLifeCycle.validCheck != 0xA55B)
	{
		lifecycle_clr_to_flash();
    }
#endif
    bool res = app_param_read(INDEX_D_4, (uint8_t *)&g_tLifeCycle, sizeof(g_tLifeCycle));
    if (res == false) { // 未被保存过，则清空并写进flash中
//        memset((uint8_t *)&g_tLifeCycle, 0, sizeof(g_tLifeCycle));
		lifecycle_clr_to_flash();
        app_param_write(INDEX_D_4, (uint8_t *)&g_tLifeCycle, sizeof(g_tLifeCycle));
    }
}

/**
  * @brief  lifecycle_update_to_flash
  * @param  None
  * @return
  * @note   None
  */
void lifecycle_update_to_flash(void)
{
    #if 0
    ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
    E2PDataType_t e2pTemp;

	e2pTemp.addr = E2P_ADDR_OF_LIFE_CYCLE;
	e2pTemp.pDate = (uint8_t*)&g_tLifeCycle; //
	e2pTemp.size = sizeof(LifeCycle_t);
	e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));
    #endif
    app_param_write(INDEX_D_4, (uint8_t *)&g_tLifeCycle, sizeof(g_tLifeCycle));
}
/**
  * @brief  lifecycle_clr_to_flash
  * @param  None
  * @return 返回头部指针
  * @note   None
  */
void lifecycle_clr_to_flash(void)
{
#if 0
    ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
    E2PDataType_t e2pTemp;

	memset((uint8_t*)&g_tLifeCycle, 0, sizeof(LifeCycle_t));
    g_tLifeCycle.validCheck = 0xA55B;
#endif
	memset((uint8_t*)&g_tLifeCycle, 0, sizeof(LifeCycle_t));
	g_tLifeCycle.batTempMax = 0x8000;	// 负极大值
	g_tLifeCycle.tempZ1Max = 0x8000;	// 负极大值
//	g_tLifeCycle.tempZ2Max = 0x8000;	// 负极大值
	g_tLifeCycle.batChargeCurrentMax = 0x8000;		// 负极大值

	g_tLifeCycle.batTempMin = 0x7FFF;	// 极大值
	g_tLifeCycle.tempZ1Min = 0x7FFF;	// 极大值
//	g_tLifeCycle.tempZ2Min = 0x7FFF;	// 极大值
	g_tLifeCycle.batDischargeCurrentMax = 0x7FFF;	// 负极大值	放电负值

	g_tLifeCycle.batVolMin = 0xFFFF;
#if 0
	e2pTemp.addr = E2P_ADDR_OF_LIFE_CYCLE;
	e2pTemp.pDate = (uint8_t*)&g_tLifeCycle; //
	e2pTemp.size = sizeof(LifeCycle_t);
    e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));	
#endif
    app_param_write(INDEX_D_4, (uint8_t *)&g_tLifeCycle, sizeof(g_tLifeCycle));
}




/**
  * @brief  读取EEPROM获取加热参数
  * @param  None
  * @return None
  * @note   None
  */
//static void heat_param_info_update_default(bool bBase, bool bBoost)
//{
//    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//	HEATER *heaterInfo = get_heat_manager_info_handle();
//
//	if (bBase)
//	{
//	    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBasePower[0].time), (uint8_t *)&(astDefBasePOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
//	    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBaseTemp[0].time), (uint8_t *)&(acstDefHeatBaseTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
//		heaterInfo->heatingProfile_base = 0;
//	}
//	if (bBoost)
//	{
//	    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostTemp[0].time), (uint8_t *)&(acstDefHeatBoostTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
//	    memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostPower[0].time), (uint8_t *)&(astDefBoostPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
//		heaterInfo->heatingProfile_boost = 1;
//	}
//}

void heatint_profile_init(void)						// 若EEPROM无对应加热曲线，则写入默认值
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t *	heatProfile = (heatProfile_t *)pBleData->heatProfile;

	uint8_t i;
	bool bChange = false;

	for (i = 0; i < MAX_NUM_HEATING_PROFILE; i++)						// 枚举10条曲线
	{
		if (heatProfile->magic != DEFAULT_HEATING_PROFILE_MAGIC_CONFIG)
		{
			heatProfile->magic = DEFAULT_HEATING_PROFILE_MAGIC_CONFIG;
			bChange = true;

			switch (i)
			{
				case 0:
					memcpy((uint8_t *)&(heatProfile->tHeatPower[0].time), (uint8_t *)&(astDefBase0POWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
					memcpy((uint8_t *)&(heatProfile->tHeatTemp[0].time), (uint8_t *)&(acstDefHeatBase0TempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
					break;
					
				case 1:
					memcpy((uint8_t *)&(heatProfile->tHeatPower[0].time), (uint8_t *)&(astDefBase1POWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
					memcpy((uint8_t *)&(heatProfile->tHeatTemp[0].time), (uint8_t *)&(acstDefHeatBase1TempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
					break;

				case 2:
					memcpy((uint8_t *)&(heatProfile->tHeatPower[0].time), (uint8_t *)&(astDefBase2POWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
					memcpy((uint8_t *)&(heatProfile->tHeatTemp[0].time), (uint8_t *)&(acstDefHeatBase2TempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
					break;
	
				case 3:
				default:
					memcpy((uint8_t *)&(heatProfile->tHeatPower[0].time), (uint8_t *)&(astDefBoostPOWERTbl[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
					memcpy((uint8_t *)&(heatProfile->tHeatTemp[0].time), (uint8_t *)&(acstDefHeatBoostTempTbl[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
					break;

			}
		}
		heatProfile++;
	}
	if (bChange)
	{
//		app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
		set_update_BleData_status(1);
	}
}


void heat_param_info_update(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t *	heatProfile;

	bool bChange = false;

// 先读取 Sel文件，确定使用的加热曲线
	heatProfileSel_t * heatProfileSel = (heatProfileSel_t *)pBleData->heatProfileSel;
// 若Sel无效，指向默认值
	if (heatProfileSel->magic != DEFAULT_HEATING_PROFILE_SEL_MAGIC_CONFIG)	// v1版本0xA55A，由于更新了Profile,更新Magic
	{
		heatProfileSel->magic = DEFAULT_HEATING_PROFILE_SEL_MAGIC_CONFIG;
		heatProfileSel->index_base_used = 0;
		heatProfileSel->index_boost_used = 3;	// 必须指定3位置
		bChange = true;
	}
	else
	{
		if (heatProfileSel->index_base_used >= MAX_NUM_HEATING_PROFILE)
		{
			heatProfileSel->index_base_used = 0;
			bChange = true;
		}
		if (heatProfileSel->index_boost_used != 3)		// 必须指定3，不接受其它指定
		{
			heatProfileSel->index_boost_used = 3;
			bChange = true;
		}
	}
// 通过Sel选择相应的加热曲线
	HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
	HEATER *heaterInfo = get_heat_manager_info_handle();
// 更新Base
	heatProfile = (heatProfile_t *)pBleData->heatProfile;
	heatProfile += (heatProfileSel->index_base_used);
	memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBasePower[0].time), (uint8_t *)&(heatProfile->tHeatPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
	memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBaseTemp[0].time), (uint8_t *)&(heatProfile->tHeatTemp[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
	heaterInfo->heatingProfile_base = heatProfileSel->index_base_used;
// 更新Boost
	heatProfile = (heatProfile_t *)pBleData->heatProfile;
	heatProfile += (heatProfileSel->index_boost_used);
	memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostPower[0].time), (uint8_t *)&(heatProfile->tHeatPower[0].time), sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
	memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostTemp[0].time), (uint8_t *)&(heatProfile->tHeatTemp[0].time), sizeof(HeatTemp_t) * MAX_TEMP_CTRL_POINT);
//	heaterInfo->heatingProfile_boost = heatProfileSel->index_boost_used;
	heaterInfo->heatingProfile_boost = 0x10;			// Session 记录时需要记录0x10而非0x04

	if (bChange)
	{
//		app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
		set_update_BleData_status(1);
	}
}

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

bool app_param_write(uint8_t index, uint8_t *pData, uint16_t len)
{
    ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    bool resWrite = false;
    int flashStatus = -1;
    uint16_t calCrc16Val = 0;
    sm_log(SM_LOG_ERR, "app_param_write index:%d\r\n", index);
    switch(index) {
        case INDEX_A:
            // 计算pData的CRC
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_A_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_B:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_B_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_C:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_C_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            t_temp.addr = PARTITION_C_BACKUP_START_ADDR;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_D_1:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_D_1_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            t_temp.addr = PARTITION_D_1_BACKUP_START_ADDR;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_D_2:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_D_2_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            t_temp.addr = PARTITION_D_2_BACKUP_START_ADDR;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_D_3:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_D_3_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            t_temp.addr = PARTITION_D_3_BACKUP_START_ADDR;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_D_4:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_D_4_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            t_temp.addr = PARTITION_D_4_BACKUP_START_ADDR;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_D_5:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_D_5_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            t_temp.addr = PARTITION_D_5_BACKUP_START_ADDR;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_E:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_E_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        case INDEX_F:
            calCrc16Val = CRC16_CCITT(pData, len - 2);
            pData[len - 2] = (calCrc16Val) & 0xff;
            pData[len - 1] = (calCrc16Val >> 8) & 0xff;
            t_temp.addr = PARTITION_F_START_ADDR;
            t_temp.pData = pData;
            t_temp.size = len;
            flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
            break;
        default:break;
    }
    if (flashStatus == 0) {
        resWrite = true;
    } else {
        resWrite = false;
    }
    return resWrite;
}

bool dual_backup_block_param_read(uint32_t addr, uint32_t backupAddr, uint8_t *pData, uint16 len)
{
    ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    t_temp.addr = addr;
    t_temp.pData = pData;
    t_temp.size = len;
    int flashStatus = -1;
    uint16_t calCrc16Val = 0;
    uint16_t saveCrc16_t = 0;
    flashStatus = mcuFlashDev->read((uint8_t*)&t_temp, t_temp.size);
    if (flashStatus != 0) {
        sm_log(SM_LOG_ERR, "main mcuFlashDev->read failed:%x\r\n", flashStatus);
        return false;
    }
    // 计算CRC，与存储的CRC对比是否相等
    calCrc16Val = CRC16_CCITT(pData, len - 2);
    saveCrc16_t = (pData[len - 1] << 8) + pData[len - 2];
    // 如果相等，则不需要读取备份区
    if (calCrc16Val == saveCrc16_t) {
        return true;
    }
    sm_log(SM_LOG_ERR, "main crc error, calCrc16Val:%x, saveCrc16_t:%x\r\n", calCrc16Val, saveCrc16_t);
    // 如果不相等则读出备份区
    t_temp.addr = backupAddr;
    t_temp.pData = pData;
    t_temp.size = len;
    flashStatus = mcuFlashDev->read((uint8_t*)&t_temp, t_temp.size);
    if (flashStatus != 0) {
        sm_log(SM_LOG_ERR, "backup mcuFlashDev->read failed:%x\r\n", flashStatus);
        return false;
    }
    // 计算备份区的CRC与备份区的CRC对比
    calCrc16Val = CRC16_CCITT(pData, len - 2);
    saveCrc16_t = (pData[len - 1] << 8) + pData[len - 2];
    // 如果备份区的CRC也不相等则返回错误
    if (calCrc16Val != saveCrc16_t) {
        sm_log(SM_LOG_ERR, "backup crc error, calCrc16Val:%x, saveCrc16_t:%x\r\n", calCrc16Val, saveCrc16_t);
        return false;
    }
    // 如果CRC与备份区的CRC相等则把备份区数据保存到主区
    t_temp.addr = addr;
    t_temp.pData = pData;
    t_temp.size = len;
    flashStatus = mcuFlashDev->write((uint8_t*)&t_temp, t_temp.size);
    if (flashStatus != 0) {
        sm_log(SM_LOG_ERR, "main mcuFlashDev->write failed:%x\r\n", flashStatus);
        return false;
    }
    return true;
}

bool single_block_param_read(uint32_t addr, uint8_t *pData, uint16 len)
{
    ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    int flashStatus = -1;
    uint16_t calCrc16Val = 0;
    uint16_t saveCrc16_t = 0;
    t_temp.addr = addr;
    t_temp.pData = pData;
    t_temp.size = len;
    flashStatus = mcuFlashDev->read((uint8_t*)&t_temp, t_temp.size);
    // 计算CRC，与存储的CRC对比是否相等
    if (flashStatus != 0) {
        return false;
    } else {
        calCrc16Val = CRC16_CCITT(pData, len - 2);
        saveCrc16_t = (pData[len - 1] << 8) + pData[len - 2];
        if (calCrc16Val == saveCrc16_t) {
            return true;
        } else {
            return false;
        }
    }
}

// 暂时不考虑flash读写不成功的情况，后续再增加，不然一下搞太复杂
// 读时， 读出来计算CRC与存储CRC比较，如果相等则直接返回，如果不等则读出备份区CRC与计算的CRC是否相等，如果相等，则把备份区的复制到主区，如果备份区也不等，则返回失败，
bool app_param_read(uint8_t index, uint8_t *pData, uint16 len)
{
    bool resRead = false;
    sm_log(SM_LOG_NOTICE, "app_param_read index:%d\r\n", index);
    switch(index) {
        case INDEX_A:
            resRead = single_block_param_read(PARTITION_A_START_ADDR, pData, len);
            break;
        case INDEX_B:
            resRead = single_block_param_read(PARTITION_B_START_ADDR, pData, len);
            break;
        case INDEX_C:
            resRead = dual_backup_block_param_read(PARTITION_C_START_ADDR, PARTITION_C_BACKUP_START_ADDR, pData, len);
            break;
        case INDEX_D_1:
            resRead = dual_backup_block_param_read(PARTITION_D_1_START_ADDR, PARTITION_D_1_BACKUP_START_ADDR, pData, len);
            break;
        case INDEX_D_2:
            resRead = dual_backup_block_param_read(PARTITION_D_2_START_ADDR, PARTITION_D_2_BACKUP_START_ADDR, pData, len);
            break;
        case INDEX_D_3:
            resRead = dual_backup_block_param_read(PARTITION_D_3_START_ADDR, PARTITION_D_3_BACKUP_START_ADDR, pData, len);
            break;
        case INDEX_D_4:
            resRead = dual_backup_block_param_read(PARTITION_D_4_START_ADDR, PARTITION_D_4_BACKUP_START_ADDR, pData, len);
            break;
        case INDEX_D_5:
            resRead = dual_backup_block_param_read(PARTITION_D_5_START_ADDR, PARTITION_D_5_BACKUP_START_ADDR, pData, len);
            break;
		case INDEX_E:
            resRead = single_block_param_read(PARTITION_E_START_ADDR, pData, len);
            break;
        case INDEX_F:
        resRead = single_block_param_read(PARTITION_F_START_ADDR, pData, len);
        break;
        default:break;
    }
    return resRead;
}

