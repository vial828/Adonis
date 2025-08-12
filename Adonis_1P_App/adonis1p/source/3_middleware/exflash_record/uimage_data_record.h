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
#ifndef __UIMAGE_DATA_RECORD_H
#define __UIMAGE_DATA_RECORD_H

#include "stdint.h"
#include "cycfg_gatt_db.h"
/*========================数据结构和宏定义====================*/

/*========================数据索引表结构体====================*/
#pragma pack (1)

#pragma pack ()
/*========================数据相关宏定义====================*/

/*===========================函数定义======================*/
bool uimage_record_payload_upgrade(uint32_t uimage_writeAddr, uint8_t* pBuf, size_t len);

bool uimage_record_payload_read(uint32_t uimage_readAddr, uint8_t* pBuf, size_t len);

int uimage_record_erase_sector(uint32_t address, uint16_t size);

int uimage_record_erase_all(void); // 清除所有uimage customization数据记录

#endif
