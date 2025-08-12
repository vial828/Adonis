/**
  ******************************************************************************
  * @file    event_data_record.h
  * @author  Gison@metextech.com
  * @date    2024/08/21
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
  * 2024-08-21     V0.01      Gison@metextech.com    the first version
  *
  ******************************************************************************
  */
#ifndef __EVENT_DATA_RECORD_H
#define __EVENT_DATA_RECORD_H

#include "stdint.h"
#include "cycfg_gatt_db.h"
/*========================数据结构和宏定义====================*/
typedef enum
{
    EVENT_CODE_WAKE_UP,
    EVENT_CODE_BUTTON_PRESS,
    EVENT_CODE_LED_BEHAVIOUR,
    EVENT_CODE_HAPTIC_BEHAVIOUR,
    EVENT_CODE_BATTERY_LEVEL,
    EVENT_CODE_SESSION_START,
    EVENT_CODE_SESSION_STOP,
    EVENT_CODE_CHARGE_START = 8,			// profile里空了7
    EVENT_CODE_CHARGE_STOP,
    EVENT_CODE_PARAMETER_CHANGE,
    EVENT_CODE_HARDWARE_RESET,
    EVENT_CODE_SLEEP,
    EVENT_CODE_ERROR,
    EVENT_CODE_IDLE,
    EVENT_CODE_INVALID = 0xFF
} EVENT_CODE_Enum;

typedef enum
{
    EVENT_CODE_KEY1 = 1,
    EVENT_CODE_KEY2,
} EVENT_CODE_KEY_Enum;

typedef enum
{
    EVENT_CODE_LCD_OFF = 0,
    EVENT_CODE_LCD_ON,
} EVENT_CODE_LCD_Enum;

typedef enum
{
	DEVICE_PARAM_IS_ERROR_BLOCKED = 1,
	DEVICE_PARAM_SHIPPING_MODE,
	DEVICE_PARAM_SERIAL_NUMBER,
	DEVICE_PARAM_EI_ENABLED,
	PWM_COIL1_FREQ,
	PWM_COIL1_DUTY_CYCLE,
//	DEVICE_PARAM_HAPTIC_LEVEL,
//	DEVICE_PARAM_LED_BRIGHTNESS_LEVEL,
//	DEVICE_PARAM_SERIAL_NUMBER,
//	DEVICE_PARAM_COV,
//	DEVICE_PARAM_WUP_TIME,
//	PWM_COIL1_FREQ,
//	PWM_COIL_2_FREQ,
//	PWM_COIL1_DUTY_CYCLE,
//	PWM_COIL2_DUTY_CYCLE,
//	PRE_SES_TEMP_LIMIT,
} EVENT_CODE_PARAM_CHANGE_Enum;

/*	与该结构体保持一致	在cycfg_gatt_db.h处定义
	typedef struct
	{
		uint32_t count;
		uint32_t timestamp;
		uint8_t event_code;
		uint8_t event_data[15];
	} debug_service_event_log_char_t;
*/
/*========================数据索引表结构体====================*/
#pragma pack (1)

typedef struct
{
    uint16_t    magic;       // magic字段，例如0xAA55为写有效
    uint16_t    total;       // record 总条数
    uint32_t    readAddr;   // 读取首地址
    uint32_t    writeAddr;   // 写入地址
    
//    uint8_t     reserved[3]; // 保留扩展使用，需注意保持整个结构占字为32Bytes-64Bytes等地址对齐
//    uint8_t     _xor;		 // 数据校验使用
	uint32_t cnt;
}fs_event_index_t; // 暂定16字节

// 读写共用体 数据结构 
typedef union
{
	fs_event_index_t  index_t;
	uint8_t      res[sizeof(fs_event_index_t)];
}fs_enent_index_u;

typedef struct
{
	uint32_t magic;				// magic字段，例如0xAA55为写有效，若读取数据后需要写0xAA00(必须从1写0)，
	debug_service_event_log_char_t rec_data;
    uint8_t  reserved[3];		// 保留扩展使用，需注意保持整个结构占字为32Bytes-64Bytes等地址对齐
    uint8_t  _xor;				// 数据校验使用，暂时末使用
}Event_t;//单条暂定 32字节 

typedef union
{
	Event_t   record_t;
	uint8_t   res[sizeof(Event_t)];
}Event_u;//读写共用体 数据结构

#pragma pack ()
/*========================数据相关宏定义====================*/
#define	EVENT_MAGIC_BLANK       (0xffffffff)	// 底层驱动有可能返回全0?
#define	EVENT_MAGIC_DATA        (0xFFAA55FF) // 代表数据正确写入，末上传至App

/** Event Log Max Number Calculate:
 * 
 * 1024KB - 4KB(INDEX) - 4KB(TEMP) = 1016KB
 * EVENT_LOG_MAX_NUM = (1016*1024)/32 == 32512 (0x7F00) 条
 * 
 * */
#define EVENT_INDEX_LEN              (4096)// index 查表区 大小为4K
#define	EVENT_LOG_MAX_NUM		     (32512)
#define	EVENT_INDEX_ADDRESS	         (0X01F00000)	// 
#define	EVENT_DATA_ADDRESS_STAR           (EVENT_INDEX_ADDRESS+EVENT_INDEX_LEN)	// 
#define	EVENT_DATA_ADDRESS_END	         (0X02000000)	// 总计 1MB ; 每个数据使用32Bytes，保留1个扇区用于擦除写入(实际保留数据会多于最大数据量)

/*===========================函数定义======================*/
void event_record_init(void); // 事件log 存储初始胡
int event_record_insert(debug_service_event_log_char_t *sRecord);   // 插入一条记录，保存至Flash，具体实现中count要内部存储并++
int event_record_get(debug_service_event_log_char_t *sRecord);      // 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
int event_record_get_next(debug_service_event_log_char_t *sRecord); // 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码
int event_record_total_get(void); // 返回数据记录总数
int event_record_erase_all(bool bCnt); // 清除所有event数据记录
int  event_index_insert(void);    // 插入一条index记录，保存至Flash
uint16_t event_index_total_sync(void);
void event_record_sync_proc(void);

// 设备产生的event log信息，补充TimeStamp等数据
int event_record_generate(EVENT_CODE_Enum code, uint8_t *optionalData, uint8_t len);


// 以下2函数跟上面的一致，区别是读取过的数据不做无效化处理
bool event_log_get(debug_service_event_log_char_t *sRecord);      // 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
bool event_log_get_next(debug_service_event_log_char_t *sRecord); // 从Flash读取下一条数据，上一条数据不做无效化处理；若有数据且数据正确，返回0，否则返回错误码


#endif
