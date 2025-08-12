/**
  ******************************************************************************
  * @file    platform_io.c
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

#include "platform_io.h"


int null_init(void){return 0;}
int null_deinit(void){return 0;}
int null_write(uint8_t *pBuf, uint16_t len){return -1;}
int null_read(uint8_t *pBuf, uint16_t len){return -1;}


static IoDev_t gs_tIoDevs[DEV_MAX] = 
{
	{DEV_USB,         driver_usb_init, 			driver_usb_deinit, 			driver_usb_write, 			driver_usb_read},

	{DEV_MS_TICK,     driver_mstick_init, 		driver_mstick_deinit, 		null_write, 				driver_mstick_read}, 
	{DEV_100US_TICK,  driver_100ustick_init, 	driver_100ustick_deinit, 	null_write, 				driver_100ustick_read},
	{DEV_10US_TICK,   driver_10ustick_init, 	driver_10ustick_deinit, 	null_write, 				driver_10ustick_read},
	{DEV_I2C,         driver_i2c_init, 			driver_i2c_deinit, 			driver_i2c_write, 			driver_i2c_read},

	{DEV_KEY,         driver_key_init, 			driver_key_deinit, 			null_write, 				driver_key_read}, 
	{DEV_ADC,         driver_detector_init, 	driver_detector_deinit, 	driver_detector_write, 		driver_detector_read},
	{DEV_MCU_EEPROM,  driver_mcu_eeprom_init, 	driver_mcu_eeprom_deinit, 	driver_mcu_eeprom_write, 	driver_mcu_eeprom_read},
	{DEV_MCU_FLASH,   driver_mcu_flash_init, 	driver_mcu_flash_deinit, 	driver_mcu_flash_write, 	driver_mcu_flash_read},

	{DEV_CHARGE_IC,   driver_charge_ic_init, 	driver_charge_ic_deinit, 	driver_charge_ic_write, 	driver_charge_ic_read},
	{DEV_PD_PHY,   	  driver_pd_phy_init, 		driver_pd_phy_deinit, 		driver_pd_phy_write, 		driver_pd_phy_read},
	{DEV_FULE_GAUGE,  driver_fuel_gauge_init, 	driver_fuel_gauge_deinit, 	driver_fuel_gauge_write, 	driver_fuel_gauge_read},
	
	{DEV_HEATER,      driver_heater_init, 		driver_heater_deinit, 		driver_heater_write, 		null_read},
	{DEV_MOTOR,       driver_motor_init, 		driver_motor_deinit, 		driver_motor_write, 		null_read},
	{DEV_BEEP,        driver_beep_init, 		driver_beep_deinit, 		driver_beep_write, 		    null_read},
	{DEV_PARAM,       driver_param_init, 		null_deinit,				driver_param_write, 		driver_param_read},
	{DEV_QSPIFLASH,   driver_qspiflash_init, 	driver_qspiflash_deinit, 	driver_qspiflash_write, 	driver_qspiflash_read},
	{DEV_AMOLED,      driver_amoled_init, 		driver_amoled_deinit, 		driver_amoled_write, 		driver_amoled_read}, 
	{DEV_TRNG,        driver_trng_init, 		driver_trng_deinit, 		driver_trng_write, 		    driver_trng_read}, 
	{DEV_RTC,         driver_rtc_init, 	driver_rtc_deinit, 	driver_rtc_write, 	driver_rtc_read}, 

	{DEV_WDG,         driver_wdg_init, 			driver_wdg_deinit, 			driver_wdg_write, 			null_read}, 
	{DEV_IDLE,        null_init, 				null_deinit, 				driver_idle_write, 			driver_idle_read}, 
	{DEV_LOG,         null_init, 				null_deinit, 				null_write, 				null_read},
	
	//exmple:{DEV_xxxx,         null_init, 				null_deinit, 				null_write, 				null_read},
};

typedef int(*deinitFun)(void);

static const deinitFun deinitList[] =
{
	driver_charge_ic_deinit,
	driver_fuel_gauge_deinit,
	driver_pd_phy_deinit,

	driver_heater_deinit,
	driver_motor_deinit,
	driver_beep_deinit,
	driver_amoled_deinit,
	driver_key_deinit,

	driver_mcu_eeprom_deinit,
	driver_mcu_flash_deinit,
	driver_qspiflash_deinit,
	driver_i2c_deinit,
	driver_usb_deinit,

	driver_mstick_deinit,

	driver_wdg_deinit,
};


ptIoDev g_ptDev[DEV_MAX];


/**
  * @brief  根据设备号获取操作设备的结构体地址
  * @param  IoDevType_u:    设备号
  * @return None
  * @note   None
  */
ptIoDev io_dev_get_dev(IoDevType_u udevType)
{
    return &gs_tIoDevs[udevType];
}


/**
  * @brief  设备初始化
  * @param  None
  * @return None
  * @note   设备开机或休眠唤醒时调用
  */
void device_init(void)
{
    uint8_t i = 0;
    for (i = 0; i < DEV_MAX; i++) 
	{
#if 0 // 电量计初始化由主任务 监管函数处理
	    if (i == DEV_FULE_GAUGE && 1 == get_shipping_mode()) { // 船运模式不进行初始化电量计，电量计太耗时
            continue;
        } else {
            gs_tIoDevs[i].init();
        }
#else
	    gs_tIoDevs[i].init();
#endif
        if (i == DEV_MCU_FLASH) {
            init_mcu_flash_param();
        }
    }
}

/**
  * @brief  设备去初始化
  * @param  None
  * @return None
  * @note   设备进入休眠时调用
  */
void device_deinit(void)
{
	for (uint16_t i = 0; i < eleof(deinitList); i++)
	{
		if(deinitList[i] != NULL)
		{
			deinitList[i]();
		}
	}
}


