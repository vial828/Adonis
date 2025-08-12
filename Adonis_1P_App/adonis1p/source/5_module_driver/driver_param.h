/**
  ******************************************************************************
  * @file    driver_param.h
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

#ifndef __DRIVER_PARAM_H
#define __DRIVER_PARAM_H

#include "stdint.h"

#define MCU_FLASH_START_ADDR (0x10000000u)
#define MCU_FLASH_TOTAL_SIZE (1024 * 1024u)
#define MCU_FLASH_ROW_SIZE (512u)
#define MCU_FLASH_IAP_PACKAGE_SIZE (256u)
#if 0
#define PARTITION_A_SIZE MCU_FLASH_ROW_SIZE
#define PARTITION_B_SIZE (MCU_FLASH_ROW_SIZE * 2u)
#define PARTITION_C_SIZE (MCU_FLASH_ROW_SIZE * 9u)
#define PARTITION_C_BACKUP_SIZE PARTITION_C_SIZE
#define PARTITION_D_SIZE (MCU_FLASH_ROW_SIZE * 6u)
#define PARTITION_D_BACKUP_SIZE PARTITION_D_1_SIZE

#define PARTITION_A_START_ADDR              (MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - MCU_FLASH_ROW_SIZE)
#define PARTITION_B_START_ADDR              (PARTITION_A_START_ADDR-PARTITION_B_SIZE)
#define PARTITION_C_START_ADDR              (PARTITION_B_START_ADDR-PARTITION_C_SIZE)
#define PARTITION_C_BACKUP_START_ADDR       (PARTITION_C_START_ADDR-PARTITION_C_BACKUP_SIZE)
#define PARTITION_D_START_ADDR              (PARTITION_C_BACKUP_START_ADDR-PARTITION_D_1_SIZE)
#define PARTITION_D_BACKUP_START_ADDR       (PARTITION_D_1_START_ADDR-PARTITION_D_1_BACKUP_SIZE)
#endif

#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#define MCU_FLASH_SECOND_SLOT_ADDR (0x1008c400u)
#define MCU_FLASH_SECOND_SLOT_SIZE (432 * 1024u)
#define MCU_FLASH_BOOT_LOADER_SLOT_SIZE (0x18000u)
#endif

#define EXTERN_FLASH_PAGE_SIZE (4096u)
#define EXTREN_FLASH_TOTAL_SIZE (256 * 4 * 1024 *32u) // 32MByte

int driver_param_init(void);
int driver_param_read(uint8_t *pBuf, uint16_t len);
int driver_param_write(uint8_t *pBuf, uint16_t len);

#endif 

