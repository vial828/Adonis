/**
  ******************************************************************************
  * @file    platform_io.h
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

#ifndef __PLATFORM_IO_H
#define __PLATFORM_IO_H
#include "stdint.h"
#include <string.h>
#include "stdio.h"

#include "driver_amoled.h"
#include "driver_key.h"
#include "driver_charge_ic.h"
#include "driver_heater.h"
#include "driver_detector.h"
#include "driver_uart_usb.h"
#include "driver_motor.h"
#include "driver_beep.h"
#include "driver_param.h"
#include "driver_wdg.h"
#include "driver_idle.h"
#include "driver_tick.h"
#include "driver_sysclk.h"
#include "driver_qspiflash.h"
#include "driver_rtc.h"
#include "driver_i2c.h"
#include "driver_fuel_gauge.h"
#include "driver_pd_phy.h"
#include "driver_mcu_eeprom.h"
#include "driver_mcu_flash.h"
#include "driver_trng.h"


typedef enum 
{
    DEV_USB         = (0),
    DEV_MS_TICK     		,
    DEV_100US_TICK  		,
    DEV_10US_TICK   		,
	DEV_I2C         		,
    DEV_KEY         		,
    DEV_ADC         		,
    DEV_MCU_EEPROM         	,
	DEV_MCU_FLASH         	, // 调整到电量计前面根据是否处在船运模式判断是否初始化电量计：电量计在MCU重启时需要重新初始化很长时间，在船运模式下，影响到按键动作，如果是船运模式，将初始化放到退出船运模式之后
    DEV_CHARGE_IC   		,
	DEV_PD_PHY		   		,
	DEV_FULE_GAUGE   		,
    DEV_HEATER      		,
    DEV_MOTOR       		,
	DEV_BEEP				,
    DEV_PARAM       		,
    DEV_QSPIFLASH   		,
    DEV_AMOLED      		,
    DEV_TRNG      		,
    DEV_RTC         		,

    DEV_WDG         		,
    DEV_IDLE        		,
    DEV_LOG         		,

    DEV_MAX         , // 重要：最大值，根据实际外设个数，设定此值
} IoDevType_u;

typedef struct IoDev_t {
    IoDevType_u udevType;
    int(*init)(void);
    int(*deinit)(void);
    int(*write)( uint8_t *buf, uint16_t len);
    int(*read)( uint8_t *buf, uint16_t len);
}IoDev_t, *ptIoDev;


ptIoDev io_dev_get_dev(IoDevType_u udevType);
void device_init(void);
void device_deinit(void);



#endif

