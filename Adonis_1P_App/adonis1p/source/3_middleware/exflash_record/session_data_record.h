/**
  ******************************************************************************
  * @file    session_data_record.h
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
#ifndef __SESSION_DATA_RECORD_H
#define __SESSION_DATA_RECORD_H

#include "stdint.h"
#include "cycfg_gatt_db.h"
/*========================数据结构和宏定义====================*/
/*	与该结构体保持一致	在cycfg_gatt_db.h处定义
	typedef struct
	{
		uint32_t count;
		uint32_t time_stamp;
		uint16_t duration;
		uint16_t session_exit_code;
		uint8_t mode;
		uint8_t heatingProfile;		增加该字段
//		uint16_t z1_max_temp;
//		uint16_t z2_max_temp;
//		uint16_t battery_max_temp;
		int16_t z1_max_temp;
//		int16_t z2_max_temp;
		int16_t battery_max_temp;
		uint8_t trusted;
	} session_service_records_char_t;	//19 byte	session_records_t
*/
/*========================数据索引表结构体====================*/
#pragma pack (1)

typedef struct
{
    uint16_t    magic;       // magic字段，例如0xAA55为写有效
    uint16_t    total;       // record 总条数
    uint32_t    readAddr;  // 读取首地址
    uint32_t    writeAddr;  // 写入地址
    
    uint8_t     reserved[3]; // 保留扩展使用，需注意保持整个结构占字为32Bytes-64Bytes等地址对齐
    uint8_t     _xor;		 // 数据校验使用
}fs_record_t;//暂定16字节

typedef union{
	fs_record_t  index_t;
	uint8_t      res[sizeof(fs_record_t)];
}fs_record_u;//读写共用体 数据结构

typedef struct Session_record_t
{
	uint32_t magic;				// magic字段，例如0xAA55为写有效，若读取数据后需要写0xAA00(必须从1写0)，
	session_service_records_char_t rec_data;	//session_records_t rec_data;
    uint8_t  reserved[8];		// 保留扩展使用，需注意保持整个结构占字为32Bytes-64Bytes等地址对齐
    uint8_t  _xor;				// 数据校验使用，暂时末使用
}Session_t;//单条暂定 32字节 

typedef union{
	Session_t   record_t;
	uint8_t     res[sizeof(Session_t)];
}Session_record_u;//读写共用体 数据结构

#pragma pack ()

#define	MAGIC_BLANK				    (0xffffffff)	// 底层驱动有可能返回全0?
#define	MAGIC_DATA_VALID		    (0xffAA5AFF)	// 代表数据正确写入，末上传至App
#define	MAGIC_DATA_INVALID		    (0xffA005FF)	// 数据正确上传至App后写入，以区分数据是否末上传处理

/** Session Record Max Number Calculate:
 * 
 * 512KB - 4KB(INDEX) - 4KB(TEMP) = 504KB
 * SESSION_RECORD_MAX_NUM = (504*1024)/32 == 16128 (0x3F00) 条
 * 
 * */
#define SESSION_INDEX_LEN           (4096)// index 查表区 大小为4K
#define	SESSION_RECORD_MAX_NUM		  (16128)
#define	SESSION_INDEX_ADDRESS	      (0X01E80000)	// 
#define	SESSION_DATA_ADDRESS_STAR	  (SESSION_INDEX_ADDRESS+SESSION_INDEX_LEN)	// 
#define	SESSION_DATA_ADDRESS_END	  (0X01F00000)	// 每个数据使用32Bytes，保留1个扇区用于擦除写入(实际保留数据会多于最大数据量)

/*========================函数定义====================*/
void session_record_init();			// 主要定位读写位置
int session_record_insert(session_service_records_char_t *sRecord);					// 插入一条记录，保存至Flash；记录的ID需要函数内部更新
int session_record_get(session_service_records_char_t *sRecord);					// 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
int session_record_get_next(session_service_records_char_t *sRecord);				// 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码
int session_record_total_get(void);									// 返回数据记录总数
int session_record_erase_all(void);									// 清除所有Session数据记录
int session_index_insert(void);			// 插入一条index记录，保存至Flash
void session_record_sync_proc(void);
uint16_t session_index_total_sync(void);


/*====================================================================*/
//uint16_t session_status_get(void);									// 高8字节返回模式，低8字节返回状态？？？
void	session_status_get(session_service_status_char_t * pData);

bool session_log_get(session_service_records_char_t *sRecord);					// 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
bool session_log_get_next(session_service_records_char_t *sRecord);				// 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码

#endif
