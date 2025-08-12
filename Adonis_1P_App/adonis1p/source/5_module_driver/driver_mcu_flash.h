#ifndef DRIVER_MCU_FLASH_H
#define DRIVER_MCU_FLASH_H

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


 typedef struct
 {
     uint32_t addr;// device address
     uint8_t *pData;//data point
     uint16 size;
 }McuFlashDataType_t;

extern int driver_mcu_flash_init(void);
extern int driver_mcu_flash_deinit(void);
extern int driver_mcu_flash_write(uint8_t *pBuf, uint16_t len);
extern int driver_mcu_flash_read(uint8_t *pBuf, uint16_t len);

#endif

