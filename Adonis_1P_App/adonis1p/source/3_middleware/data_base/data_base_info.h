/**
  ******************************************************************************
  * @file    data_base_info.h
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

#ifndef __DATA_BASE_INFO_H
#define __DATA_BASE_INFO_H
#include "stdint.h"
#include "platform_io.h"

/*************编译前需要检查的宏开关*****************/
#define DE_RDP_ON_EN   0 // 只有发工厂出货的版本才打开此宏，此宏加密后 SWD无法复原
#define DEF_PIN_BROKE_EN 1 // 使能断针保护( 给蓝牙app团队时，该宏需要关闭，需干烧测试， 防止版本过多，OTA文件比此宏关闭)
#define DEF_BLE_EN 1 // 使能蓝牙
//#define FACTORY_PROD 1 // 工厂生产，不能设置温控曲线，自清洁曲线， 校温曲线暂时可设置
//#define DEF_BLE_APP_GROUP_EN 1 // 给蓝牙app团队时，该宏需要打开
//#define DEF_DRY_CLEAN_EN 1 // 启用干烧清洁功能， 当不使用干烧清洁功能时，把此宏注释掉
/***************************************************/
/****************************************************/
//#define DEFAULT_HEATING_PROFILE_MAGIC_CONFIG 0xA55D // 更新默认加热曲线时，迭代此魔数
#define DEFAULT_HEATING_PROFILE_MAGIC_CONFIG 0xA55E // 更新默认加热曲线时，迭代此魔数
#define DEFAULT_HEATING_PROFILE_SEL_MAGIC_CONFIG 0xA55B // 更新默认加热曲线时，迭代此魔数

#define USB_REC_MAX_DATA_SIZE 250
//最大温控曲线控制点数
#define MAX_POWER_CTRL_POINT        (12)
#define MAX_TEMP_CTRL_POINT         (15)
#define MAX_TR_TBL_GR               (15)
#define MAX_STEP_PUFF_POINT         (1)
#define DEFAULT_MAX_PUFF            (12)
#define DEFAULT_PREHEAT_TIME        (5)
#define DEFAULT_HEAT_TIME           (300)

#define DEFAULT_ADJ_TEMP_B      (0)
#define DEFAULT_ADJ_TEMP_K      (1)

#define DEFAULT_ADJ_POWER_B     (0)// 功率校准b
#define DEFAULT_ADJ_POWER_K     (1)// 功率校准K

#define DEFAULT_ADJ_OPA_B       (0)// 功率校准b
#define DEFAULT_ADJ_OPA_K       (1)// 功率校准K

#define DEFAULT_ADJ_Curr_B       (0)// 电流校准值b
#define DEFAULT_ADJ_Curr_K       (1)// 电流校准值K

#define DEFAULT_ADJ_OutV_B       (0)// 发热体电压采样校准b
#define DEFAULT_ADJ_OutV_K       (1)// 发热体电压校准K
// 可配置功能开关 默认值
#define DEF_POWER_PRT_EN        (1) // 功率保护使能
#define DEF_HIGH_TEMP_PRT_EN    (1) // 过温保护使能
#define DEF_CYCLIC_WORK_PRT_EN  (1) // 连续循环开机保护使能
#define DEF_EMPTY_BURN_PRT_EN   (1) // 空烧保护使能
#define DEF_BROKEN_PIN_PRT_EN   (1) // 断针保护使能
#define DEF_SUCTION_CNT_EN      (1) // 抽吸支数 计数使能

#define DEF_SMOKE_TOTAL_TIME    (0)

#define  DEF_SUCTION_SUM_CNT    (0)
#define DEF_TIPS_CLEAN_TIME     (1000)//单位 秒

//断针保护相关变量
#define DEF_THRESHOLD_BASE_LTK       (18.7f)
#define DEF_THRESHOLD_BOOST_LTK      (18.6f)

#define DEF_THRESHOLD_BASE_TK       (39.2f)
#define DEF_THRESHOLD_BOOST_TK      (40.8f)

#define DEF_THRESHOLD_BASE_TB       (0)
#define DEF_THRESHOLD_BOOST_TB      (0)
#define DEF_THRESHOLD_ERR_TEMP      (20.0f)

/********************** image相关定义 **************************/
/***************  AD1P_UI_IMAGE_20250212  *******************/
#define HI_LEN                  (30)
#define HI_TO_PREHEAT_LEN       (25)
#define STANDARD_ICON_LEN       (21)
#define BOOST_ICON_LEN          (21)
#define PREHEAT_BAR_LEN         (3)
#define PREHEAT_TO_HEAT_LEN     (22)
#define HEAT_BAR_LEN            (3)
#define PREHEAT_TO_END_LEN      (20)
#define HEAT_TO_END_LEN         (30)
#define DISPOSE_LEN             (43)
#define BYE_LEN                 (24)
#define BALL_LEN                (36)

#define NUMBER_PER_LEN          (12)
#define BAR_BAT_GREEN_LEN       (2)
#define BAR_BAT_YELLOW_LEN      (2)
#define BAR_BAT_AMBER_LEN       (2)
#define BAR_BAT_RED_LEN         (2)
#define LIGHTNING_LEN           (1)
#define HINT_LEN                (5)
#define PLUG_LEN                (13)

#define WARNING_LEN             (1)
#define RESET_CHARGE_LEN        (15)
#define WRONG_CHARGE_LEN        (2)
#define COLD_HOT_LEN            (2)
#define WAITING_LEN             (25)
#define REBOOT_CONFIRM_LEN      (38)
#define REBOOT_CLICK_LEN        (14)
#define REBOOT_TIP_SEC_LEN      (2)
#define REBOOT_COUNT_LEN        (14)
#define OUT_SHIPPING_LEN        (38)
#define TIMEOUT_LEN             (9)
#define BATTERY_EOL_LEN         (3)
#define CRITICAL_QR_LEN         (3)

#define BLUETOOTH_LEN           (1)
#define FAILED_LEN              (1)
#define PAIRED_LEN              (24)
#define PAIRING_LEN             (1)
#define FIND_ME_LEN             (2)
#define LOADING_LEN             (1)
#define LOCK_LEN                (2)
#define DOWNLOAD_LEN            (2)

#define PROFILE_LEN        		(3)
#define HI_BYE_LEN              (2)
#define STANDARD_ICON_CUSTOM_LEN (21)

#if 0 // 移除自清洁的UI assets for SP
#define COUNT_DOWN_LEN          (12)
#define CLEANING_LEN          	(99)
#define CLEAN_HINT_LEN          (88)
#endif
/************************************************************/
#define UI_HOR_RES      90
#define UI_VER_RES      282
#define UI_COLOR_DEPTH  3   // 888
#define IMAGE_SEZE  (UI_HOR_RES * UI_VER_RES * UI_COLOR_DEPTH)
#define QSPI_MAX_ADDR_LEN (0x1000000)
#define IAP_COMBINE_BUF_LEN 512 // IAP 升级缓存包，凑满整包后写进FLASH中
#define IAP_4K_BUF_LEN 4096 // 4K 缓存包，凑满整包后写进FLASH中
#define ERRCODE_MAX_LEN 27

#define INDEX_A 1 // 无备份 无CRC
#define INDEX_B 2 // 无备份 有CRC
#define INDEX_C 3 // 有备份 有CRC
#define INDEX_D_1 4 // 有备份 独立CRC
#define INDEX_D_2 5 // 有备份 独立CRC
#define INDEX_D_3 6 // 有备份 独立CRC
#define INDEX_D_4 7 // 有备份 独立CRC
#define INDEX_D_5 8 // 有备份 独立CRC
#define	INDEX_E 9 	// 无备份 有CRC
#define	INDEX_F 10 	// 无备份 有CRC
//#define MCU_FLASH_START_ADDR (0x10000000u)
//#define MCU_FLASH_TOTAL_SIZE (1024 * 1024u)
//#define MCU_FLASH_ROW_SIZE (512u)

#define PARTITION_A_SIZE (MCU_FLASH_ROW_SIZE * 3u)
#define PARTITION_B_SIZE (MCU_FLASH_ROW_SIZE * 2u)
#define PARTITION_C_SIZE (MCU_FLASH_ROW_SIZE * 13u)
#define PARTITION_C_BACKUP_SIZE PARTITION_C_SIZE
#define PARTITION_D_1_SIZE (MCU_FLASH_ROW_SIZE * 1u)
#define PARTITION_D_1_BACKUP_SIZE PARTITION_D_1_SIZE
#define PARTITION_D_2_SIZE (MCU_FLASH_ROW_SIZE * 2u)
#define PARTITION_D_2_BACKUP_SIZE PARTITION_D_2_SIZE
#define PARTITION_D_3_SIZE (MCU_FLASH_ROW_SIZE * 1u)
#define PARTITION_D_3_BACKUP_SIZE PARTITION_D_3_SIZE
#define PARTITION_D_4_SIZE (MCU_FLASH_ROW_SIZE * 1u)
#define PARTITION_D_4_BACKUP_SIZE PARTITION_D_4_SIZE
#define PARTITION_D_5_SIZE (MCU_FLASH_ROW_SIZE * 1u)
#define PARTITION_D_5_BACKUP_SIZE PARTITION_D_5_SIZE

#define PARTITION_E_SIZE (MCU_FLASH_ROW_SIZE * 1u)
#define PARTITION_F_SIZE (MCU_FLASH_ROW_SIZE * 2u)//1k Byte 

#define PARTITION_A_START_ADDR              (MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - PARTITION_A_SIZE)
#define PARTITION_B_START_ADDR              (PARTITION_A_START_ADDR-PARTITION_B_SIZE)
#define PARTITION_C_START_ADDR              (PARTITION_B_START_ADDR-PARTITION_C_SIZE)
#define PARTITION_C_BACKUP_START_ADDR       (PARTITION_C_START_ADDR-PARTITION_C_BACKUP_SIZE)

#define PARTITION_D_1_START_ADDR            (PARTITION_C_BACKUP_START_ADDR-PARTITION_D_1_SIZE)
#define PARTITION_D_1_BACKUP_START_ADDR     (PARTITION_D_1_START_ADDR-PARTITION_D_1_BACKUP_SIZE)

#define PARTITION_D_2_START_ADDR            (PARTITION_D_1_BACKUP_START_ADDR-PARTITION_D_2_SIZE)
#define PARTITION_D_2_BACKUP_START_ADDR     (PARTITION_D_2_START_ADDR-PARTITION_D_2_BACKUP_SIZE)

#define PARTITION_D_3_START_ADDR            (PARTITION_D_2_BACKUP_START_ADDR-PARTITION_D_3_SIZE)
#define PARTITION_D_3_BACKUP_START_ADDR     (PARTITION_D_3_START_ADDR-PARTITION_D_3_BACKUP_SIZE)

#define PARTITION_D_4_START_ADDR            (PARTITION_D_3_BACKUP_START_ADDR-PARTITION_D_4_SIZE)
#define PARTITION_D_4_BACKUP_START_ADDR     (PARTITION_D_4_START_ADDR-PARTITION_D_4_BACKUP_SIZE)

#define PARTITION_D_5_START_ADDR            (PARTITION_D_4_BACKUP_START_ADDR-PARTITION_D_5_SIZE)
#define PARTITION_D_5_BACKUP_START_ADDR     (PARTITION_D_5_START_ADDR-PARTITION_D_5_BACKUP_SIZE)

#define PARTITION_E_START_ADDR              (PARTITION_D_5_BACKUP_START_ADDR-PARTITION_E_SIZE)
#define PARTITION_F_START_ADDR              (PARTITION_E_START_ADDR - PARTITION_F_SIZE)
//wanggankun@2024-11-21, add, for UI/BIN UP
#define EXT_FLASH_UI1_START_ADDRESS 0x00000000
#define EXT_FLASH_UI1_SIZE 0xEAF000
#define EXT_FLASH_UI2_START_ADDRESS 0x00EAF000
#define EXT_FLASH_UI2_SIZE 0xEAF000
#define EXT_FLASH_UI_FLAG_START_ADDRESS 0x01D5E000
#define CY_OTA_IMAGE_USE_UI1            0
#define CY_OTA_IMAGE_USE_UI2            1
#define CY_OTA_IMAGE_USE_UNKNOW         2



/**
  * @brief Motor 模式枚举
  */
typedef enum {
  HAPTIC_1 = 0,
  HAPTIC_2,
  HAPTIC_3,				// 20250107 add For Eos
  HAPTIC_4,
  HAPTIC_5,
  HAPTIC_ERR,
  
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	HAPTIC_FIND_ME,
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

  HAPTIC_TATAL
} MOTOR_MODE_Enum;

/**
  * @brief Motor息结构体
  */
typedef struct MotorInfo_t
{
    uint8_t duty;
    uint16_t hz;

    uint16_t dutyOfRmsV;// RMS Voltage(V) 3.6V def
    uint16_t runIndv;
    uint16_t stopIndv;
    uint16_t runTimes;
    uint32_t motorTick;
    MOTOR_MODE_Enum HAPTICs;
} MotorInfo_t;
/**
  * @brief Motor息结构体
  */
typedef struct BeepInfo_t
{
    uint8_t duty;
    uint16_t hz;

    uint16_t runIndv;
    uint16_t stopIndv;
    uint16_t runTimes;
    uint32_t beepTick;
} BeepInfo_t;

//定义TR数据结构
typedef struct TrInfo_t
{
    uint16_t tempeture;         //TR表温度值
    uint16_t resistor;          //TR表电阻值
} TrInfo_t;

//温控曲线控制点结构体
typedef struct HeatTemp_t
{
    uint16_t time;              //加热段时间
    uint16_t tempeture;         //加热段温度
} HeatTemp_t;

//功率表控制点结构体
typedef struct HeatPower_t
{
    uint16_t time;              //之前定义的是一维数组功率表， 后续拓展为时间对应功率，把powerL改为time
    uint16_t power;             //加热功率表
} HeatPower_t;

typedef struct StepPuffInfo_t
{ 
    uint16_t timePart; // 阶段抽吸时间
    uint16_t puffPart; // 阶段口数
} StepPuffInfo_t;

typedef struct PuffInfo_t
{
    uint8_t maxPuff;           //最大口数
    uint8_t preHeatTime;       //预热时间
    uint16_t maxHeatTime;       //最大加热时间
    StepPuffInfo_t tStepPuffInfo[MAX_STEP_PUFF_POINT]; // 补充数据
} PuffInfo_t;

// 可配置功能开关 默认值
#define DEF_POWER_PRT_EN        (1) // 功率保护使能
#define DEF_HIGH_TEMP_PRT_EN    (1) // 过温保护使能
#define DEF_CYCLIC_WORK_PRT_EN  (1) // 连续循环开机保护使能
#define DEF_EMPTY_BURN_PRT_EN   (1) // 空烧保护使能
#define DEF_BROKEN_PIN_PRT_EN   (1) // 断针保护使能
#define DEF_SUCTION_CNT_EN      (1) // 抽吸支数 计数使能

typedef struct HeatParamInfo_t
{
    uint16_t  validCheck;               // 数据区有效校验字节，0xaa：有效，其它值：无效
    TrInfo_t tTrInfo[MAX_TR_TBL_GR];

    HeatPower_t tHeatTestPower[MAX_POWER_CTRL_POINT]; // 产线测试用
    HeatTemp_t tHeatTestTemp[MAX_TEMP_CTRL_POINT];

    HeatPower_t tHeatBasePower[MAX_POWER_CTRL_POINT];
    HeatTemp_t tHeatBaseTemp[MAX_TEMP_CTRL_POINT];


    HeatPower_t tHeatBoostPower[MAX_POWER_CTRL_POINT];
    HeatTemp_t  tHeatBoostTemp[MAX_TEMP_CTRL_POINT];

    HeatPower_t tHeatCleanPower[MAX_POWER_CTRL_POINT];
    HeatTemp_t  tHeatCleanTemp[MAX_TEMP_CTRL_POINT];

    PuffInfo_t tPuffInfo;


    //各种功能开关
//    uint8_t powerPrtEnable;     // 功率保护使能
//    uint8_t highTempPrtEnable;  // 过温保护使能
//    uint8_t cyclicWorkPrtEnable;// 连续循环开机保护使能
//    uint8_t emptyBurnPrtEnable; // 空烧保护使能
//    uint8_t brokenPinPrtEnable; // 断针保护使能
//    uint8_t suctionCntEnable;   // 抽吸支数 计数使能
    uint32_t smokeSuctionSum;
    uint32_t smokeBaseTotalTime;
    uint32_t smokeBoostTotalTime;
} HeatParamInfo_t;

#pragma pack(1) // 单字节对齐, app需要同样的单字节对齐去定义结构体

/******************************************************************/
typedef struct ImageInfo_t {
    uint32_t addr;      // 存储地址
    uint16_t witdh;     // 像素宽度
    uint16_t height;    // 像素高度
    uint16_t xpos;      // 图片x定位点
    uint16_t ypos;      // 图片y定位点
} ImageInfo_t;

typedef struct ImageHeatTablInfo_t {
    uint16_t imageHiTablLen;
    ImageInfo_t imageHiTablInfo[HI_LEN];

    uint16_t imageHiToPreheatTablLen;
    ImageInfo_t imageHiToPreheatTablInfo[HI_TO_PREHEAT_LEN];

    uint16_t imageStandardIconTablLen;
    ImageInfo_t imageStandardIconTablInfo[STANDARD_ICON_LEN];

    uint16_t imageBoostIconTablLen;
    ImageInfo_t imageBoostIconTablInfo[BOOST_ICON_LEN];

    uint16_t imagePreheatBarTablLen;
    ImageInfo_t imagePreheatBarTablInfo[PREHEAT_BAR_LEN];
    
    uint16_t imagePreheatToHeatTablLen;
    ImageInfo_t imagePreheatToHeatTablInfo[PREHEAT_TO_HEAT_LEN];
    
    uint16_t imageHeatBarTablLen;
    ImageInfo_t imageHeatBarTablInfo[HEAT_BAR_LEN];

    uint16_t imagePreheatToEndTablLen;
    ImageInfo_t imagePreheatToEndTablInfo[PREHEAT_TO_END_LEN];

    uint16_t imageHeatToEndTablLen;
    ImageInfo_t imageHeatToEndTablInfo[HEAT_TO_END_LEN];

    uint16_t imageDisposeTablLen;
    ImageInfo_t imageDisposeTablInfo[DISPOSE_LEN];

    uint16_t imageByeTablLen;
    ImageInfo_t imageByeTablInfo[BYE_LEN];

    uint16_t imageBallTablLen;
    ImageInfo_t imageBallTablInfo[BALL_LEN];
} ImageHeatTablInfo_t;

typedef struct ImageBatTablInfo_t {
    uint16_t imageNumberPerTablLen;
    ImageInfo_t imageNumberPerTablInfo[NUMBER_PER_LEN];

    uint16_t imageBarBatGreenTablLen;
    ImageInfo_t imageBarBatGreenTablInfo[BAR_BAT_GREEN_LEN];

    uint16_t imageBarBatYellowTablLen;
    ImageInfo_t imageBarBatYellowTablInfo[BAR_BAT_YELLOW_LEN];

    uint16_t imageBarBatAmberTablLen;
    ImageInfo_t imageBarBatAmberTablInfo[BAR_BAT_AMBER_LEN];

    uint16_t imageBarBatRedTablLen;
    ImageInfo_t imageBarBatRedTablInfo[BAR_BAT_RED_LEN];

    uint16_t imageLightningTablLen;
    ImageInfo_t imageLightningTablInfo[LIGHTNING_LEN];

    uint16_t imageHintTablLen;
    ImageInfo_t imageHintTablInfo[HINT_LEN];

    uint16_t imagePlugTablLen;
    ImageInfo_t imagePlugTablInfo[PLUG_LEN];
} ImageBatTablInfo_t;

typedef struct ImageErrTablInfo_t {
    uint16_t imageWarningTablLen;
    ImageInfo_t imageWarningTablInfo[WARNING_LEN];

    uint16_t imageResetChargeTablLen;
    ImageInfo_t imageResetChargeTablInfo[RESET_CHARGE_LEN];

    uint16_t imageWrongChargeTablLen;
    ImageInfo_t imageWrongChargeTablInfo[WRONG_CHARGE_LEN];

    uint16_t imageColdHotTablLen;
    ImageInfo_t imageColdHotTablInfo[COLD_HOT_LEN];

    uint16_t imageWaitingTablLen;
    ImageInfo_t imageWaitingTablInfo[WAITING_LEN];

    uint16_t imageRebootConfirmTablLen;
    ImageInfo_t imageRebootConfirmTablInfo[REBOOT_CONFIRM_LEN];

    uint16_t imageRebootClickTablLen;
    ImageInfo_t imageRebootClickTablInfo[REBOOT_CLICK_LEN];

    uint16_t imageRebootSecTablLen;
    ImageInfo_t imageRebootSecTablInfo[REBOOT_TIP_SEC_LEN];

    uint16_t imageRebootCountTablLen;
    ImageInfo_t imageRebootCountTablInfo[REBOOT_COUNT_LEN];

    uint16_t imageOutShippingTablLen;
    ImageInfo_t imageOutShippingTablInfo[OUT_SHIPPING_LEN];

    uint16_t imageTimeoutTablLen;
    ImageInfo_t imageTimeoutTablInfo[TIMEOUT_LEN];

    uint16_t imageBatEolTablLen;
    ImageInfo_t imageBatEolTablInfo[BATTERY_EOL_LEN];

    uint16_t imageCriticalQrTablLen;
    ImageInfo_t imageCriticalQrTablInfo[CRITICAL_QR_LEN];
} ImageErrTablInfo_t;

typedef struct ImageBltTablInfo_t {
    uint16_t imageBluetoothTablLen;
    ImageInfo_t imageBluetoothTablInfo[BLUETOOTH_LEN];

    uint16_t imageFailedTablLen;
    ImageInfo_t imageFailedTablInfo[FAILED_LEN];

    uint16_t imagePairedTablLen;
    ImageInfo_t imagePairedTablInfo[PAIRED_LEN];

    uint16_t imagePairingTablLen;
    ImageInfo_t imagePairingTablInfo[PAIRING_LEN];

    uint16_t imageFindMeTablLen;
    ImageInfo_t imageFindMeTablInfo[FIND_ME_LEN];

    uint16_t imageLoadingTablLen;
    ImageInfo_t imageLoadingTablInfo[LOADING_LEN];

    uint16_t imageLockTablLen;
    ImageInfo_t imageLockTablInfo[LOCK_LEN];

    uint16_t imageDownloadTablLen;
    ImageInfo_t imageDownloadTablInfo[DOWNLOAD_LEN];
} ImageBltTablInfo_t;

typedef struct ImageOtherTablInfo_t {
    uint16_t imageProfileTablLen;
    ImageInfo_t imageProfileTablInfo[PROFILE_LEN];

    uint16_t imageHiByeTablLen;
    ImageInfo_t imageHiByeTablInfo[HI_BYE_LEN];

    uint16_t imageStandardIconCustomTablLen;
    ImageInfo_t imageStandardIconCustomTablInfo[STANDARD_ICON_CUSTOM_LEN];
	
#if 0 // 移除自清洁的UI assets for SP
    uint16_t imageCountDownTablLen;
    ImageInfo_t imageCountDownTablInfo[COUNT_DOWN_LEN];

    uint16_t imageCleaningTablLen;
    ImageInfo_t imageCleaningTablInfo[CLEANING_LEN];

    uint16_t imageCleanHintTablLen;
    ImageInfo_t imageCleanHintTablInfo[CLEAN_HINT_LEN];
#endif
} ImageOtherTablInfo_t;

// 镜像结构体
typedef struct ImageHeaderInfo_t
{
    char ver[32];
    ImageHeatTablInfo_t		imageHeatTablInfo;
    ImageBatTablInfo_t		imageBatTablInfo;
    ImageErrTablInfo_t		imageErrTablInfo;
    ImageBltTablInfo_t		imageBltTablInfo;
    ImageOtherTablInfo_t	imageOtherTablInfo;
	uint32_t				offSetAddr;				// UI素材可能存在于Bank0或Bank1，增加该偏移量用于Flash读取地址偏移，UI.bin不需要增加该字段
} ImageHeaderInfo_t;

/**************************************************************************/

typedef struct Qspiflash_t
{
    uint32_t addr;
    uint8_t *data;
    uint32_t len;
} Qspiflash_t;

typedef struct AmoledArea_t{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} AmoledArea_t;

typedef struct AmoledInfo_t{
    AmoledArea_t area;
    uint8_t *data;
    uint32_t len;
} AmoledInfo_t;

typedef struct WriteRgbVal_t{ // 这个顺序是显示屏565模式时的高低位顺序，g低位
    uint16_t g: 6;
    uint16_t r: 5;
    uint16_t b: 5;
} WriteRgbVal_t;
typedef union WRgbVal_u {
    WriteRgbVal_t tWriteRgbVal;
    uint8_t rgbBuf[2];
} WRgbVal_u;


typedef struct ReadRgbVal_t{
    uint16_t r: 5;
    uint16_t g: 6;
    uint16_t b: 5;
} ReadRgbVal_t;
typedef union RRgbVal_u {
    ReadRgbVal_t tReadRgbVal;
    uint8_t rgbBuf[2];
} RRgbVal_u;

// fuel gauge chip's battery state
typedef struct AppBatInfo_t {
    uint16_t voltage; // battery voltage (mV)
    int16_t  current; // charge or discharge current (mA)
    uint16_t temperature; // battery temperature (℃)

    uint16_t capacityFull; // full capacity (mAh)
    uint16_t capacityRemain; // remaining capacity (mAh)	 
    uint16_t power; // average power draw (mW)

    uint16_t soc; // state-of-capacity (%)
    uint16_t soh; // state-of-health (%)	
    uint32_t cycle; // charge-discharge cycle times
} AppBatInfo_t;

// 校准值 ,TR表 存储结构体
typedef struct FDB_are_b1_Info_t
{ 
    //ROW 1 
    uint32_t  magic;        // magic字段，存储标志
    uint16_t len;           // 结构体SIZE

    uint32_t rdpon;
    uint8_t  snNumber[16];
    uint32_t shippingMode;
    uint8_t  iniKeyString[16];
    int16    iniValTbl[128]; 
    //ROW 2 
    float   tempAdjK; //k
    int16_t tempAdjB; //b
 
    float   powerAdjK; //k
    int16_t powerAdjB; //b

    float   opaAdjK;   //k
    int16_t opaAdjB;   //b

    float   adcCurrAdjK; //k 输出电流采样校准值
    int16_t adcCurrAdjB; //b 输出电流采样校准值   

    float   adcOutVdjK; //k 输出电压采样校准值
    int16_t adcOutVdjB; //b 输出电压采样校准值   

    TrInfo_t tTrInfo[MAX_TR_TBL_GR];  

    HeatPower_t tHeatTestPower[MAX_POWER_CTRL_POINT]; // 产线测试用
    HeatTemp_t tHeatTestTemp[MAX_TEMP_CTRL_POINT];

    HeatPower_t tHeatCleanPower[MAX_POWER_CTRL_POINT];
    HeatTemp_t  tHeatCleanTemp[MAX_TEMP_CTRL_POINT];
    // HeatPower_t tHeatBasePower[MAX_POWER_CTRL_POINT];
    // HeatTemp_t tHeatBaseTemp[MAX_TEMP_CTRL_POINT];


    // HeatPower_t tHeatBoostPower[MAX_POWER_CTRL_POINT];
    // HeatTemp_t  tHeatBoostTemp[MAX_TEMP_CTRL_POINT];
    uint16_t crc16;
}FDB_are_b1_Info_t;

typedef struct FDB_AREA_E_t
{ 
    uint32_t  magic;        // magic字段，存储标志
    uint16_t len;           // 结构体SIZE

    uint8_t color;
    uint8_t  reserve[503];	// 保留以供扩展，整个结构体长度512

    uint16_t crc16;
}FDB_AREA_E_t;

//温控曲线控制点结构体
typedef struct RefTemp_t
{
    float time;              //加热段时间 单位： mS
    float tempeture;         //加热段温度 单位： ℃
} RefTemp_t;
typedef struct FDB_pin_bk_chird_t
{ 
    // Solution 2 断针相关 归一化处理的 数值放大1000倍
    //BASE
    uint16_t    magicBase;          // magic字段，存储标志
    uint32_t    baseDryHeatSlope1;  // 空烧 0.479 - 1s 
    uint32_t    baseDryHeatSlope2;  // 空烧 2-5秒斜率
    uint32_t    baseThreshold_0_1_low;      // 判断阈值1
    uint32_t    baseThreshold_0_1_high;     // 判断阈值2
    uint32_t    baseThreshold_2_5_low;      // 判断阈值3
    uint32_t    baseThreshold_2_5_high;     // 判断阈值4 
    uint32_t    baseRev1;           // 保留4字节
    uint32_t    baseRev2;           // 保留4字节
    //BOOST
    uint16_t    magicBoost;         // magic字段，存储标志
    uint32_t    boostDryHeatSlope1; // 空烧 0.479 - 1s 斜率
    uint32_t    boostDryHeatSlope2; // 空烧 2-5秒斜率
    uint32_t    boostThreshold_0_1_low ;      // 判断阈值1
    uint32_t    boostThreshold_0_1_high;     // 判断阈值2
    uint32_t    boostThreshold_2_5_low ;      // 判断阈值3
    uint32_t    boostThreshold_2_5_high;     // 判断阈值4 
    uint32_t    boostRev1;          // 保留4字节
    uint32_t    boostRev2;          // 保留4字节
}FDB_pin_bk_chird_t;
typedef struct FDB_AREA_F_t
{ 
    uint32_t  magic;        // magic字段，存储标志
    uint16_t  len;           // 结构体SIZE

    int16     flag_base;  
    int16     base_info[4]; // [0]start TC temp ,[1]start PCB temp ,[2]start usb port temp,[3]start bat temp  Unit:0.01℃
    RefTemp_t base_ref[30]; // base 0 - 5s 空烧样本

    int16     flag_boost; 
    int16     boost_info[4]; // [0]start TC temp ,[1]start PCB temp ,[2]start usb port temp,[3]start bat temp  Unit:0.01℃ 
    RefTemp_t boost_ref[30]; // boost 0 - 5s 空烧样本
    FDB_pin_bk_chird_t pinBreakParam;
    uint8_t  reserve[448];	// 保留以供扩展，整个结构体长度1024
    uint16_t crc16;
}FDB_AREA_F_t;

typedef union
{
	FDB_AREA_F_t   fdb_f_t;
	uint8_t        res[sizeof(FDB_AREA_F_t)];
}FDB_AREA_F_u;//读写共用体 数据结构 512字节
extern FDB_AREA_F_u  g_fdb_f_u;


typedef union
{
	FDB_are_b1_Info_t   fdb_b1_t;
	uint8_t   res[sizeof(FDB_are_b1_Info_t)];
}FDB_area_b1_u;//读写共用体 数据结构

extern FDB_area_b1_u  g_fdb_b_u;

typedef enum {
  HEAT_FLOW_STATE_NONE = 0,
  HEAT_FLOW_STATE_START,
  HEAT_FLOW_STATE_PREHEAT,
  HEAT_FLOW_STATE_HEAT_NORMAL,
  HEAT_FLOW_STATE_HEAT_STAGE_LAST_EOS,
} HEAT_FLOW_STATE_Enum;
typedef struct heater_struct
{

    HEAT_FLOW_STATE_Enum HeatState; //
	unsigned char HeatFlag;         //
	unsigned char HeatStep;         //
    unsigned int HeatMode;			//加热模式 BASE BOOST
	unsigned int HeatTick;          // 加热时基 TICK
    unsigned int StartTime;         // 启动加热时的tick
	unsigned int PreHeatTime;       // 预热时间
	unsigned int TotalHeatTime;     // 总加热时间
	unsigned int HeatingTime;       // 当前加热时间

    float tempAdjK;          // 温度校准值 加热时目标温度为： CurrTargetTemp + adjTemp
    float tempAdjB;          // 温度校准值 加热时目标温度为： CurrTargetTemp + adjTemp

    float powerAdjK;          // 温度校准值 加热时目标温度为： CurrTargetTemp + adjTemp
    float powerAdjB;          // 温度校准值 加热时目标温度为： CurrTargetTemp + adjTemp

    float opaAdjK;          // 温度校准值 加热时目标温度为： CurrTargetTemp + adjTemp
    float opaAdjB;          // 温度校准值 加热时目标温度为： CurrTargetTemp + adjTemp

    float CurrAdjK;          // 发热体电流采样校准值K
    float CurrAdjB;          // 发热体电流采样校准值b 

    float voltageAdjK;       // 发热体电压采样校准值k 
    float voltageAdjB;       // 发热体电压采样校准值b                                                                   

    float CurrTargetTemp;   //
    float CurrDetectTemp;   //
    float detectPcbTemp;    // 热电偶冷端温度 也是 PCB温度
	float CurrResister;     // 发热体阻值
    float CurrPowerVal;     //
    float nmos_res; //链接发热体的板载NMOS 内阻
    float SetPower;
    float SetVotage;
    float DetectVotage;
    float DetectCurrent;//电流
    float Heat_Hot_K; //冷热机 系数 0.86 ~ 1.00
    float HeatIs1Voltage;
    uint8_t HeatPuff;
    float HeatTempK;        // 温度变化率K 
    int16_t opa_adc_val;    // 运放输出端 采集到的ADC 值
    float Heating_J;//加热耗能
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint32_t time_stamp;
	uint8_t time_stamp_trusted;
	int16_t battery_max_temp;	// 
	int16_t z1_max_temp;		// 需要增加温度监控
//	int16_t z2_max_temp;		// 需要增加温度监控
	uint8_t heatingProfile;			// 本次加热使用的曲线
	uint8_t heatingProfile_base;	// base 使用的Profile
	uint8_t heatingProfile_boost;	// Boost使用的Profile
	uint8_t bRunning;				// 正在运行
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
} HEATER;

#define INI_CONFIG_DEFAULT_EN   1
typedef enum IniValIndex_e {
#if 0 // 202405 list
    HAPTIC_VOLT                   = 0   ,    // 3600 (0~3600)mv, 震动马达RMS电压
    HAPTIC_PWM_FREQ               = 1   ,    // 20 (0~1200)kHz , 震动马达驱动频率
    WAR_CHG_VBUS                  = 2   ,    // 11000 (mv)     , 充电Vbus最大电压
    EOL_SESSION_MAX               = 3   ,    // 5 (1~5)        , EOL降额分段设置
    NEXT_EOL_STEP                 = 4   ,    // 1 (1~5)        , 下一个降额段
    STEP1_SESSION                 = 5   ,    // 0              , cycle
    STEP1_CHG_CURR                = 6   ,    // 2500           , 充电电流  （设置到充电IC）
    STEP1_GAUGE_TERM_CURR         = 7   ,    // 100            , 截止电流  （电量计）
    STEP1_CHG_VOLT                = 8   ,    // 4400           , 满充电压   （电量计）
    STEP2_SESSION                 = 9   ,    // 8200           , cycle
    STEP2_CHG_CURR                = 10  ,    // 2000           , 充电电流
    STEP2_GAUGE_TERM_CURR         = 11  ,    // 100            , 截止电流
    STEP2_CHG_VOLT                = 12  ,    // 4320           , 满充电压
    STEP3_SESSION                 = 13  ,    // 8000           , cycle
    STEP3_CHG_CURR                = 14  ,    // 2000           , 充电电流
    STEP3_GAUGE_TERM_CURR         = 15  ,    // 100            , 截止电流
    STEP3_CHG_VOLT                = 16  ,    // 4320           , 满充电压
    STEP4_SESSION                 = 17  ,    // 9000           , cycle
    STEP4_CHG_CURR                = 18  ,    // 2000           , 充电电流
    STEP4_GAUGE_TERM_CURR         = 19  ,    // 100            , 截止电流
    STEP4_CHG_VOLT                = 20  ,    // 4320           , 满充电压
    STEP5_SESSION                 = 21  ,    // 10000          , cycle
    STEP5_CHG_CURR                = 22  ,    // 400            , 充电电流
    STEP5_CHG_PRE_CURR            = 23  ,    // 200            , 预充电流
    STEP5_GAUGE_TERM_CURR         = 24  ,    // 100            , 截止电流
    STEP5_CHG_VOLT                = 25  ,    // 3840           , 满充电压
    FLT_BAT_HOT_PRE_SES           = 26  ,    // 51             , 启动加热时电芯过温
    FLT_BAT_HOT_CHARGING          = 27  ,    // 55             , 充电过温
    FLT_BAT_HOT_CHARGE_CLEAR      = 28  ,    // 50             , 充电过温恢复
    FLT_BAT_COLD_CHARGE           = 29  ,    // 2              , 充电低温
    FLT_BAT_COLD_CHARGE_CLEAR     = 30  ,    // 4              , 充电低温恢复
    FLT_BAT_HOT                   = 31  ,    // 58             , 电芯过温
    FLT_BAT_HOT_CLEAR             = 32  ,    // 52             , 电芯过温恢复
    FLT_BAT_COLD                  = 33  ,    // -8             , 电芯低温
    FLT_BAT_COLD_CLEAR            = 34  ,    // -6             , 电芯低温恢复
    WAR_BAT_EMPTY                 = 35  ,    // 3000           , 加热过程电芯低压
    WAR_BAT_LOW                   = 36  ,    // 3580 (3001~4000, 启动加热时电压低 (POC 先用电压）
    WAR_BAT_LOW_SOC               = 37  ,    // 6 (0~99)       , 启动加热时电量低
    WAR_BAT_VOLT_DAMAGE           = 38  ,    // 2000           , 电芯NG电压，不可恢复 （电压改到2.0）
    FLT_TC_ZONE_HOT               = 39  ,    // 500            , 发热体过温 (TBD)
    FLT_TC_ZONE_HOT_PRE_SES       = 40  ,    // 200            , 启动加热时发热体过温(TBD)
    FLT_BAT_VOLTAGE_OVER          = 41  ,    // 4450           , 电芯过压
    FLT_BAT_VOLTAGE_OVER_CLEAR    = 42  ,    // 4350           , 电芯过压恢复
    FLT_BAT_DISCHARGE_CURR_OVER   = 43  ,    // 7000           , 加热放电过流
    FLT_BAT_CHARGE_CURR_OVER      = 44  ,    // 4000           , 充电过流
    FLT_CO_JUNC_HOT               = 45  ,    // 80             , PCBA过温 (RT1-thermocouple cold NTC )
    FLT_CO_JUNC_HOT_CLEAR         = 46  ,    // 60             , PCBA过温恢复
    FLT_BAT_I_SENSE_DAMAGE        = 47  ,    // 1000           , 加热无电流
    FLT_CIC_CHARGE_TIMEOUT        = 48  ,    // 360            , 充电超时，单位分钟 (6h)
    FLT_USB_HOT_TEMP              = 49  ,    // 70             , USB端口过热
    FLT_USB_HOT_TEMP_CLEAR        = 50  ,    // 55             , USB端口过热恢复
    WAR_BAT_SES_EMPTY             = 51  ,    // 3000           , case加热低压
    WAR_BAT_DIS_EMPTY             = 52  ,    // 3150           , case给pen充电低压
    DB_FLT_TC_PWR_LIM             = 53  ,    // 1500           , 功率保护，单位0.01W（设置为0则禁止该功能）
    DB_FLT_TC_CONT                = 54  ,    // 1              , 连续开机保护（1->使能， 0->禁止）
    DB_FLT_TC_EMPTY               = 55  ,    // 1              , 加热空烧保护（1->使能， 0->禁止）
    DB_FLT_TC_BREAK               = 56  ,    // 1              , 断针保护（1->使能， 0->禁止）
    PFUN_AUTO_CLEAN               = 57  ,    // 0              , 自清洁功能（1->使能， 0->禁止）
    PFUN_BLE_EN                   = 58  ,    // 0              , BLE使能（1->使能， 0->禁止）
    DB_SESSIONS_CLEAN             = 59  ,    // 100            , 启动自清洁session值
    FLT_TARGET_TEMP_DIFF          = 60  ,    // 50            , 加热过程中 采集温度偏离目标温差
    SHIP_MODE_VOLT                = 61  ,    // 2900            , 休眠时进入shiping mode电压
    BASE_LOG_SW                   = 62  ,    //0                 ，原始的调试开关，加热的时候会打印log （1->使能， 0->禁止）
    INI_VAL_TBL_LEN               = 63  ,   // 条目数
#else // 20240803 list
    HAPTIC_VOLT                     =  0 , // 3600 (0~3600)mv, 震动马达RMS电压
    HAPTIC_PWM_FREQ                 =  1 , // 20 (0~1200)kHz , 震动马达驱动频率
    WAR_CHG_VBUS                    =  2 , // 11000 (mv)     , 充电Vbus最大电压
    EOL_SESSION_MAX                 =  3 , // 3, EOL降额分段设置
    STEP1_SESSION                   =  4 , // 0, cycle
    STEP1_CHG_CURR                  =  5 , // 2560, 充电电流（设置到充电IC）
    STEP1_GAUGE_TERM_CURR           =  6 , // 100, 截止电流（电量计）
    STEP1_CHG_VOLT                  =  7 , // 4400, 满充电压（电量计）
	STEP1_UPPER_TEMP_CHARGE_CURR    =  8 , // 800, 充电上限温度限流值
    STEP1_UPPER_TEMP_CHARGING       =  9 , // 45, 充电上限温度阈值
    STEP1_UPPER_TEMP_CHARGE_CLEAR   = 10 , // 41, 充电上限温度恢复值
    STEP1_FLT_BAT_HOT_CHARGING      = 11 , // 55, 充电过温
    STEP1_FLT_BAT_HOT_CHARGE_CLEAR  = 12 , // 50, 充电过温恢复
    STEP1_FLT_BAT_HOT_PRE_SES       = 13 , // 51, 启动加热时电芯过温
    STEP1_FLT_BAT_HOT               = 14 , // 58, 电芯过温
    STEP1_FLT_BAT_HOT_CLEAR         = 15 , // 52, 电芯过温恢复
    STEP2_SESSION                   = 16 , // 8200, cycle
    STEP2_CHG_CURR                  = 17 , // 2000, 充电电流（设置到充电IC）
    STEP2_GAUGE_TERM_CURR           = 18 , // 100, 截止电流
    STEP2_CHG_VOLT                  = 19 , // 4320, 满充电压
	STEP2_UPPER_TEMP_CHARGE_CURR    = 20 , // 800, 充电上限温度限流值
    STEP2_UPPER_TEMP_CHARGING       = 21 , // 43, 充电上限温度阈值
    STEP2_UPPER_TEMP_CHARGE_CLEAR   = 22 , // 39, 充电上限温度恢复值
    STEP2_FLT_BAT_HOT_CHARGING      = 23 , // 53, 充电过温
    STEP2_FLT_BAT_HOT_CHARGE_CLEAR  = 24 , // 48, 充电过温恢复
    STEP2_FLT_BAT_HOT_PRE_SES       = 25 , // 48, 启动加热时电芯过温
    STEP2_FLT_BAT_HOT               = 26 , // 56, 电芯过温
    STEP2_FLT_BAT_HOT_CLEAR         = 27 , // 50, 电芯过温恢复
    STEP3_SESSION                   = 28 , // 11000, cycle
    STEP3_CHG_CURR                  = 29 , // 400, 充电电流（设置到充电IC）
    STEP3_GAUGE_TERM_CURR           = 30 , // 100, 截止电流
    STEP3_CHG_VOLT                  = 31 , // 3840, 满充电压
	STEP3_UPPER_TEMP_CHARGE_CURR    = 32 , // 320, 充电上限温度限流值
    STEP3_UPPER_TEMP_CHARGING       = 33 , // 43, 充电上限温度阈值
    STEP3_UPPER_TEMP_CHARGE_CLEAR   = 34 , // 39, 充电上限温度恢复值
    STEP3_FLT_BAT_HOT_CHARGING      = 35 , // 53, 充电过温
    STEP3_FLT_BAT_HOT_CHARGE_CLEAR  = 36 , // 48, 充电过温恢复
    STEP3_FLT_BAT_HOT_PRE_SES       = 37 , // 48, 启动加热时电芯过温
    STEP3_FLT_BAT_HOT               = 38 , // 56, 电芯过温
    STEP3_FLT_BAT_HOT_CLEAR         = 39 , // 50, 电芯过温恢复
    FLT_BAT_COLD_CHARGE           = 40,    // 2, 充电低温
    FLT_BAT_COLD_CHARGE_CLEAR     = 41,    // 4, 充电低温恢复
    FLT_BAT_COLD                  = 42,    // -8, 电芯低温
    FLT_BAT_COLD_CLEAR            = 43,    // -6, 电芯低温恢复
    WAR_BAT_EMPTY                 = 44,    // 3000, 加热过程电芯低压
    WAR_BAT_LOW                   = 45,    // 3580, 启动加热时电压低
    WAR_BAT_LOW_SOC               = 46,    // 5 (0~99), 启动加热时电量低
    WAR_BAT_VOLT_DAMAGE           = 47,    // 2000, 电芯NG电压，不可恢复 （电压改到2.0）
    FLT_TC_ZONE_HOT               = 48,    // 500, 发热体过温 (TBD)
    FLT_TC_ZONE_HOT_PRE_SES       = 49,    // 200, 启动加热时发热体过温(TBD)
    FLT_BAT_VOLTAGE_OVER          = 50,    // 4450, 电芯过压
    FLT_BAT_VOLTAGE_OVER_CLEAR    = 51,  // 4350, 电芯过压恢复
    FLT_BAT_DISCHARGE_CURR_OVER   = 52,  // -7000, 加热放电过流
    FLT_BAT_CHARGE_CURR_OVER      = 53,  // 4000, 充电过流
    FLT_CO_JUNC_HOT               = 54,  // 80, PCBA过温 (RT1-thermocouple cold NTC )
    FLT_CO_JUNC_HOT_CLEAR         = 55,  // 60, PCBA过温恢复
    FLT_BAT_I_SENSE_DAMAGE        = 56,  // 1000, 加热无电流
    FLT_CIC_CHARGE_TIMEOUT        = 57,  // 360, 充电超时，单位分钟 (6h)
    FLT_USB_HOT_TEMP              = 58,  // 70, USB端口过热
    FLT_USB_HOT_TEMP_CLEAR        = 59,  // 55, USB端口过热恢复
    DB_FLT_TC_PWR_LIM             = 60,  // 1800, 功率保护，单位0.01W（设置为0则禁止该功能）
    DB_FLT_TC_CONT                = 61,  // 1, 连续开机保护（1->使能， 0->禁止）
    DB_FLT_TC_EMPTY               = 62,  // 1, 加热空烧保护（1->使能， 0->禁止）
    DB_FLT_TC_BREAK               = 63,  // 0, 断针保护（1->使能， 0->禁止）
    PFUN_AUTO_CLEAN               = 64,  // 0, 自清洁功能（1->使能， 0->禁止）
    PFUN_BLE_EN                   = 65,  // 0, BLE使能（1->使能， 0->禁止）
    DB_SESSIONS_CLEAN             = 66,  // 100, 启动自清洁session值
    FLT_TARGET_TEMP_DIFF          = 67,  // 50 , 加热过程中 采集温度偏离目标温差
    SHIP_MODE_VOLT                = 68,  // 2900, 休眠时进入shiping mode电压
    BASE_LOG_SW                   = 69,  //0，原始的调试开关，加热的时候会打印log （1->使能， 0->禁止）
    INI_VAL_TBL_LEN               = 70,  // 条目数
 #endif   
}IniValIndex_e;

typedef struct IniInfo_t{
    uint32_t keyTotalLen;
    uint32_t valTotalLen;
}IniInfo_t;

/*调试参数*/
typedef enum
{
    DEBUG_SOC       = BIT0,
    DEBUG_VBAT      = BIT1,
    DEBUG_IBAT      = BIT2,
    DEBUG_TBAT      = BIT3,
    DEBUG_TPCBA     = BIT4,
    DEBUG_TUSB      = BIT5,
    DEBUG_THEAT     = BIT6,
    DEBUG_VBUS      = BIT7,
    DEBUG_IBUS      = BIT8,
    DEBUG_SESSION   = BIT9,
    DEBUG_TBD       = BIT10
}DebugParamFlag_e;

// 需要监控的数据
typedef struct MonitorDataInfo_t {
    DetectorInfo_t det;
    BatInfo_t bat;
    ChgInfo_t chg;
    HEATER heaterInfo;

	uint8_t state;
	uint8_t partab;

    uint16_t session;
    uint16_t dbgBit;
}MonitorDataInfo_t;

typedef enum KeyStatus_e {
    KEY_UNLOCK = 0,
    KEY_LOCK,
}KeyStatus_e;
typedef struct KeyLockStatus_t {
    uint8_t baseKey;
    uint8_t boostKey;
    uint8_t usb;
}KeyLockStatus_t;


#pragma pack()

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#pragma pack(1)
typedef struct LifeCycle_t {
    uint32_t magic;               // 数据区有效校验字节，0xA55A：有效，其它值：无效
    uint16_t len;
	int16_t batTempMin;
	int16_t batTempMax;
	uint16_t batVolMin;
	uint16_t batVolMax;
	int16_t batChargeCurrentMax;
	int16_t batDischargeCurrentMax;
	uint32_t batChargeTimeTotal;
	uint32_t batFullChargeCnt;
	uint32_t sessionCompleteCnt;
	uint32_t sessionIncompleteCnt;
	int16_t tempZ1Min;
	int16_t tempZ1Max;
    uint16_t crc;
//	int16_t tempZ2Min;
//	int16_t tempZ2Max;
}LifeCycle_t;

#define MAX_BEEP_CTRL_POINT         (30)
#define DEF_BEEP_VOL                (50)
//buzzer t
typedef struct beepFrq_t
{
    uint16_t time;          //beep time /100MS
    uint16_t frequency;     //beep frequency /hz
} beepFrq_t;


//heatProfileSel t
typedef struct heatProfileSel_t
{
    uint32_t magic;
    uint8_t index_base_used;
	uint8_t index_boost_used;
} heatProfileSel_t;

typedef struct heatProfile_t
{
    uint32_t magic;
    HeatPower_t tHeatPower[MAX_POWER_CTRL_POINT];
    HeatTemp_t tHeatTemp[MAX_TEMP_CTRL_POINT];
} heatProfile_t;



#pragma pack()


LifeCycle_t* get_life_cycle_handle(void);
void life_cycle_info_init(void);
void lifecycle_update_to_flash(void);
void lifecycle_clr_to_flash(void);
void heatint_profile_init(void);
void heat_param_info_update(void);

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
typedef struct SessionTimeSum_v1_t {
    uint32_t magic;
    uint16_t len;
    uint32_t suctionSum;
    uint32_t baseTimeSum;
    uint32_t boostTimeSum;
    uint16_t crc;
} SessionTimeSum_v1_t;

typedef struct SessionTimeSum_v2_t {
    uint32_t magic;
    uint16_t len;
    uint32_t suctionSum;
    uint32_t baseTimeSum;
    uint32_t boostTimeSum;
	uint32_t baseTimeSumClean;			// 增加自清洁提醒的烟支时间
    uint32_t boostTimeSumClean;			// 增加自清洁提醒的烟支时间
    uint16_t crc;
} SessionTimeSum_v2_t;

typedef union
{
	SessionTimeSum_v1_t   sessionTimeSum_v1;
	SessionTimeSum_v2_t   sessionTimeSum_v2;
}SessionTimeSum_u;

uint8_t* get_usb_data_handle(void);
HeatParamInfo_t* get_heat_param_info_handle(void);
void heat_param_info_init(void);
ImageHeaderInfo_t* get_image_header_info_handle(void);
void gui_init(void);
uint8_t* get_ram_main_gui(void);
uint8_t* get_ram_second_gui(void);
uint8_t* get_iap_combine_buf(void);
int16_t* get_ini_val_info_handle(void);
uint16_t* get_errCode_val_info_handle(void);
void ini_val_tbl_init(void);
IniInfo_t* get_iniInfo_handle(void);
MonitorDataInfo_t* get_monitor_data_info_handle(void);
FDB_area_b1_u* get_fdb_b_info_handle(void);
uint32_t get_shipping_mode(void);
void set_shipping_mode(uint32_t mode);
void set_rdp_on_mode_no_save_eeprom(uint32_t mode);
uint32_t get_rdp_on_mode_no_save_flash(void);

uint32_t get_rdp_on_mode(void);
void set_rdp_on_mode(uint32_t mode);

void init_mcu_flash_param(void);

void bt_adv_set_en(uint8_t en);
uint8_t bt_get_adv_en(void);

uint8_t* get_func_setup(void); // use for func setup
int16_t get_sessions_clean(void);
bool get_clean_status(void);
bool ui_data_security_verify(uint32_t dataLen, uint32_t addrLen);
KeyLockStatus_t *get_key_lock_status(void);
bool ui_pos_securitu_verify(uint16_t imageHeightStart, uint16_t imageHeightEnd, uint16_t imageWitdthStart, uint16_t imageWitdthEnd);
void errcode_update_to_flash(void);
void errcode_clr_to_flash(void);

bool app_param_read(uint8_t index, uint8_t *pData, uint16 len);
bool app_param_write(uint8_t index, uint8_t *pData, uint16_t len);
void session_time_sum_update_to_flash(void);
SessionTimeSum_v2_t *get_session_time_sum_info_handle(void);

uint8_t get_device_color(void);
void set_device_color(uint8_t color);

#endif

