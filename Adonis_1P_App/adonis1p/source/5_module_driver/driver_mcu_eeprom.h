#ifndef DRIVER_MCU_EEPROM_H
#define DRIVER_MCU_EEPROM_H

#include "cy_em_eeprom.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "cycfg.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sm_log.h"
#include "public_typedef.h"
 

//线性校准结构体 调节关系式为Y=kX+b;
typedef struct AdjInfo_t
{
    float   adjK; //k
    int16_t adjB; //b
} AdjInfo_t;


 /* EEPROM Configuration details. All the sizes mentioned are in bytes.
  * For details on how to configure these values refer to cy_em_eeprom.h. The
  * library documentation is provided in Em EEPROM API Reference Manual. The user
  * access it from ModusToolbox IDE Quick Panel > Documentation> 
  * Cypress Em_EEPROM middleware API reference manual
  */
#define EEPROM_SIZE             (1024 * 16u)
#define EEPROM_OFFSET           (0u)
#define EEPROM_BASE				(0u)//(0X08000000UL + 1024UL * 60UL)//0X0800F000UL//

#if 1
 /* Logical Size of Emulated EEPROM in bytes. */
#define E2P_ADDR_OF_INIT_FLAG  	    (EEPROM_BASE+offsetof(ParamStore_t, initFlag))
#define E2P_SIZE_OF_INIT_FLAG		SIZEOF(ParamStore_t, initFlag)

#define E2P_ADDR_OF_CHECK_CRC  	    (EEPROM_BASE+offsetof(ParamStore_t, checkCRC))
#define E2P_SIZE_OF_CHECK_CRC		SIZEOF(ParamStore_t, checkCRC)

#define E2P_ADDR_OF_RDP_ON  	(EEPROM_BASE+offsetof(ParamStore_t, rdpOn))
#define E2P_SIZE_OF_RDP_ON		SIZEOF(ParamStore_t, rdpOn)

/*---------------------------------------------------------------------------------*/
#define E2P_ADDR_OF_SN  	        (EEPROM_BASE+offsetof(ParamStore_t, snNumber))
#define E2P_SIZE_OF_SN		        SIZEOF(ParamStore_t, snNumber)

/*-------------------------校准值--------------------------------------------*/
#define E2P_ADDR_OF_ADJ_AREA	    (EEPROM_BASE+offsetof(ParamStore_t, adjArea))
#define E2P_SIZE_OF_ADJ_AREA	    SIZEOF(ParamStore_t, adjArea)

#define E2P_ADDR_OF_SHIPPING_MODE  	(EEPROM_BASE+offsetof(ParamStore_t, shippingMode))
#define E2P_SIZE_OF_SHIPPING_MODE		SIZEOF(ParamStore_t, shippingMode)

#define E2P_ADDR_OF_INI_VAL_TBL  	(EEPROM_BASE+offsetof(ParamStore_t, iniValTbl))
#define E2P_SIZE_OF_INI_VAL_TBL		SIZEOF(ParamStore_t, iniValTbl)

#define E2P_ADDR_OF_INI_KEY_STR  	(EEPROM_BASE+offsetof(ParamStore_t, iniKeyString))
#define E2P_SIZE_OF_INI_KEY_STR		SIZEOF(ParamStore_t, iniKeyString)

#define E2P_ADDR_OF_ERRCODE_VAL_TBL  	(EEPROM_BASE+offsetof(ParamStore_t, errCodeVal))
#define E2P_SIZE_OF_ERRCODE_VAL_TBL		SIZEOF(ParamStore_t, errCodeVal)

#define E2P_ADDR_OF_ERRCODE_KEY_STR  	(EEPROM_BASE+offsetof(ParamStore_t, errCodeKeyString))
#define E2P_SIZE_OF_ERRCODE_KEY_STR		SIZEOF(ParamStore_t, errCodeKeyString)

#define E2P_ADDR_OF_FUEL_GAUGE  	(EEPROM_BASE+offsetof(ParamStore_t, fuelGauge))
#define E2P_SIZE_OF_FUEL_GAUGE		SIZEOF(ParamStore_t, fuelGauge)

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
// 以下部分为蓝牙相关增加的
// Gison 20241026	#define E2P_ADDR_OF_BE_DEV_NAME  	(EEPROM_BASE+offsetof(ParamStore_t, bt_dev_name))
// Gison 20241026	#define E2P_SIZE_OF_BE_DEV_NAME		SIZEOF(ParamStore_t, bt_dev_name)

// Gison 20241026	#define E2P_ADDR_OF_BE_FIND_ME  	(EEPROM_BASE+offsetof(ParamStore_t, bt_find_me))
// Gison 20241026	#define E2P_SIZE_OF_BE_FIND_ME		SIZEOF(ParamStore_t, bt_find_me)

// Gison 20241026	#define E2P_ADDR_OF_LED_BRIGHTNESS  	(EEPROM_BASE+offsetof(ParamStore_t, led_brightness))
// Gison 20241026	#define E2P_SIZE_OF_LED_BRIGHTNESS		SIZEOF(ParamStore_t, led_brightness)

// Gison 20241026	#define E2P_ADDR_OF_HAPTIC_STRENGTH  	(EEPROM_BASE+offsetof(ParamStore_t, haptic_strength))
// Gison 20241026	#define E2P_SIZE_OF_HAPTIC_STRENGTH		SIZEOF(ParamStore_t, haptic_strength)

// Gison 20241026	#define E2P_ADDR_OF_BUZ_LOUDNESS  		(EEPROM_BASE+offsetof(ParamStore_t, buz_loudness))
// Gison 20241026	#define E2P_SIZE_OF_BUZ_LOUDNESS		SIZEOF(ParamStore_t, buz_loudness)

// Gison 20241026	#define E2P_ADDR_OF_LOCK_MODE	  		(EEPROM_BASE+offsetof(ParamStore_t, lock_mode))
// Gison 20241026	#define E2P_SIZE_OF_LOCK_MODE			SIZEOF(ParamStore_t, lock_mode)

#define E2P_ADDR_OF_LIFE_CYCLE	  		(EEPROM_BASE+offsetof(ParamStore_t, life_cycle))
#define E2P_SIZE_OF_LIFE_CYCLE			SIZEOF(ParamStore_t, life_cycle)

#define E2P_ADDR_OF_TIME_STAMP	  		(EEPROM_BASE+offsetof(ParamStore_t, time_stamp))
#define E2P_SIZE_OF_TIME_STAMP			SIZEOF(ParamStore_t, time_stamp)

// 加热参数存储区，目前存储10条曲线
// Gison 20241026	#define E2P_ADDR_OF_HEAT_PROFILE  		(EEPROM_BASE+offsetof(ParamStore_t, heatProfile))
// Gison 20241026	#define E2P_SIZE_OF_HEAT_PROFILE		SIZEOF(ParamStore_t, heatProfile)
// HeatingProfile 选择
// Gison 20241026	#define E2P_ADDR_OF_HEAT_PROFILE_SEL  	(EEPROM_BASE+offsetof(ParamStore_t, heatProfileSel))
// Gison 20241026	#define E2P_SIZE_OF_HEAT_PROFILE_SEL	SIZEOF(ParamStore_t, heatProfileSel)

// 加热参数部分，全部移至EEPROM
#define E2P_ADDR_OF_HEAT_PARAM  	(EEPROM_BASE+offsetof(ParamStore_t, heatParam))
#define E2P_SIZE_OF_HEAT_PARAM		SIZEOF(ParamStore_t, heatParam)

// Gison 20241026	#define E2P_ADDR_OF_BONDING_DATA  	(EEPROM_BASE+offsetof(ParamStore_t, ble_bonding_data))
// Gison 20241026	#define E2P_SIZE_OF_BONDING_DATA	SIZEOF(ParamStore_t, ble_bonding_data)

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

/*---------------------------------------------------------------------------------*/
#define E2P_ADDR_OF_CRC_END		 	(EEPROM_BASE+offsetof(ParamStore_t, endCRC))
#define E2P_SIZE_OF_CRC				(E2P_ADDR_OF_CRC_END-E2P_ADDR_OF_INIT_FLAG)//校验的长度
#endif //EEPROM


 typedef struct{
 
    uint32_t initFlag;        //上电初始化的标志
    uint32_t checkCRC;        //校验
    uint32_t rdpOn;           // 加密标志
    uint32_t snNumber[4];    //SN码是一串字符串，典型长度为16字节 


    uint8_t   adjArea[512];//512字节 存放校准信息

    uint32_t shippingMode;    // 船运标志，1：船运模式，0船运模式
//    uint32_t batEolMode;     // eol标志，1：电芯无效 ，0电芯有效
    // 存放eol标志
    // 预留错误码统计表
//    uint32_t uid[20];              //UID
//    uint32_t trList[40+1];         //TR表信息 20*2 
//    uint32_t tempCurve[40*3+1];    //TC表信息 30*2 双管3条温控曲线 
//    uint32_t powerCurve[40*3+1];   //TC表信息 30*2 双管3条温控曲线 
//    uint32_t puff[5*3+1];          //口数信息
//    uint32_t tempAdjust[2+1];      //调节参数
    int16_t  iniValTbl[128];
    char     iniKeyString[4096]; // 前面两条只有KEY,没有VAL第一个是版本号，第二个是条目数 字符串形式表示
// 增加错误码配置预留100, 最小值0， 最大值65535
    uint16_t errCodeVal[100]; //
    char     errCodeKeyString[1024];
    uint8_t fuelGauge[1024];
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
// 蓝牙使用的设备名称
// Gison 20241026		uint8_t bt_dev_name[32];
// Gison 20241026		uint8_t	bt_find_me[128];
// Gison 20241026		uint8_t led_brightness[32];
// Gison 20241026		uint8_t haptic_strength[32];
// Gison 20241026		uint8_t buz_loudness[32];
// Gison 20241026		uint8_t lock_mode[32];
	uint8_t life_cycle[128];
	uint8_t time_stamp[32];		// 20240923增加
// Gison 20241026		uint8_t heatProfileSel[32];
	uint8_t reserve[56];		// 以上参数占用7560+32，加120-32占位，保持512字节
//	uint8_t heatProfile[256];
	uint8_t reserve2[1024];		// uint8_t reserve2[1024];		// 扩展用；且保持加热参数1K字节对齐
	uint8_t heatParam[1024];	// 加热参数部分

// Gison 20241026	uint8_t ble_bonding_data[4096];	// 蓝牙邦定信息
// Gison 20241026	uint8_t heatProfile[2048];	// 需要存储10条Profile，移至末端，增加至2K
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    uint32_t endCRC;           //校验结束位置 
 
 }ParamStore_t;
 
 typedef struct
 {
     uint32_t addr;// device address
     uint8_t *pDate;//data point
     uint16 size;
 }E2PDataType_t;

 extern int driver_mcu_eeprom_init(void);
 extern int driver_mcu_eeprom_deinit(void);
 extern int driver_mcu_eeprom_write(uint8_t *pBuf, uint16_t len);
 extern int driver_mcu_eeprom_read(uint8_t *pBuf, uint16_t len);

#endif



