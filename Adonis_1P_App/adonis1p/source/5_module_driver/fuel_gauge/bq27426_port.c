#include "bq27426_port.h"

#define BQ27426_IIC_MAX_DATA_SIZE   64



#if __1P__

#include "bqfs_1p.h"  //gmfs文件

int8_t i2cReadReturn;
int8_t i2cWriteReturn;

/****************************************************************************************
 * @brief   看门狗喂狗
 * @param 	none
 * @return  none
 * @note
 *****************************************************************************************/
void bq27426_wdog_feed(void)
{
	int wdgTicks = 0;
	driver_wdg_write((uint8_t*)&wdgTicks, 4);
}

/****************************************************************************************
 * @brief   延时函数
 * @param 	none
 * @return  none
 * @note
 *****************************************************************************************/
void bq27426_mDelay(uint16_t ms)
{
	vTaskDelay(ms);
}

/****************************************************************************************
 * @brief   GPOUT Pin退出shutdow时发送脉冲
 * @param 	none
 * @return  none
 * @note
 *****************************************************************************************/
void bq27426_gpout_pin_exit_shutdown(void)
{
    // GPOUT脉冲,至少200us的高电平--------------------------
	cyhal_gpio_init(GPOUT_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);//模式-输出
	for (int i = 0; i<3; i++) {
		cyhal_gpio_write(GPOUT_PIN, 1);
		bq27426_mDelay(2);
		cyhal_gpio_write(GPOUT_PIN, 0);
		bq27426_mDelay(1);
	}
	cyhal_gpio_init(GPOUT_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLDOWN, 0); // 输入下拉

	bq27426_mDelay(10);
}

/****************************************************************************************
 * @brief   GPOUT Pin进入休眠时配置
 * @param 	none
 * @return  none
 * @note
 *****************************************************************************************/
void bq27426_gpout_pin_enter_sleep(void)
{
	cyhal_gpio_init(GPOUT_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLDOWN, 0); // 系统进入休眠时，gpout pin配置
}

/****************************************************************************************
 * @brief   根据电芯型号识别，返回对应gmfs数据指针
 * @param 	none
 * @return  none
 * @note
 *****************************************************************************************/
void bq27426_bat_type(bq27426_bat_type_t* bat_type)
{
	//TODO 电芯型号识别 - 根据不同型号，选择gmfs文件
	int8_t cnt = 0;
	uint8_t batTypeId = 0xFF;
	DetectorInfo_t det_t;
	bq27426_mDelay(20); // Wait for the adc conversion to stabilize
	do {
		bq27426_mDelay(20);
		driver_detector_read((uint8_t*)&det_t, sizeof(det_t));

		if (det_t.battTypeVoltage < 0.840f) { // < 0.84V -- AUCOPO
			batTypeId = 1;
		} else if(det_t.battTypeVoltage > 1.960f) { // > 1.96V –- COSMX
			batTypeId = 0;
		}else { // UNKNOW
			batTypeId = 2;
		}

		if (batTypeId == bat_type->id) {
			cnt++;
			if (cnt >= 3) {
				break; // The same bat type is checked
			}
		} else {
			cnt--;
			if (cnt <= -3) {
				cnt = 0;
				bat_type->id = batTypeId;
				BQ27426_LOG(BQ27426_LOG_LEVEL_INFO, "VMS:%0.2f\n", det_t.battTypeVoltage);
			}
		}
	} while(1);

	if (bat_type->id == 0) { // > 1.96V –- COSMX
		bat_type->gmfs = bqfs_1p_cosmx;
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO, "BAT-COSMX\n");
	} else if (bat_type->id == 1) { // < 0.84V -- AUCOPO
		bat_type->gmfs = bqfs_1p_aucopo; // AUCOPO;
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO, "BAT-AUCOPO\n");
	}else { // UNKNOW /* use last or default*/
		if (bat_type->gmfs == bqfs_1p_aucopo) {
			bat_type->id = 1;
			bat_type->gmfs = bqfs_1p_aucopo;
		} else { /* if : NULL or bqfs_1p_cosmx */
			bat_type->id = 0;
			bat_type->gmfs = bqfs_1p_cosmx;
		}
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO, "BAT-UNKNOW\n");
	}
}


extern bool app_param_read(uint8_t index, uint8_t *pData, uint16_t len);
extern bool app_param_write(uint8_t index, uint8_t *pData, uint16_t len);
/****************************************************************************************
 * @brief   flash读字节
 * @param 	cmdAddress 	 	32bit数据地址
 * @param	rBuffer 		数据缓存指针地址
 * @param	rLength 		数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
bool bq27426_flash_read(uint32_t addr,uint8_t* databuf,uint16_t len)
{
#if 0
    //本地数据存储，读成功，返回true;读失败，返回false
    E2PDataType_t e2pTemp;

    e2pTemp.addr = addr;
    e2pTemp.pDate = (uint8_t*)databuf;
    e2pTemp.size = len; // sizeof(fuelGauge);
    if (RET_SUCCESS == driver_mcu_eeprom_read((uint8_t*)&e2pTemp, sizeof(e2pTemp))) {
    	//BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"eeprom->read gmfs ok!\n");
    	return true;
    }

    return false;
#else
    return app_param_read((uint8_t)addr, databuf, len);
#endif
}
/****************************************************************************************
 * @brief   flash擦除
 * @param 	cmdAddress 	 	32bit数据地址
 * @param	rLength 		数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
bool bq27426_flash_erase(uint32_t addr,uint8_t* databuf,uint16_t len)
{
    //flash写之前先擦除
	memset((uint8_t *)&databuf, 0, len);
	app_param_write((uint8_t)addr, (uint8_t *)databuf, len);

    return true;
}
/****************************************************************************************
 * @brief   flash写字节
 * @param 	cmdAddress 	 	32bit数据地址
 * @param	rBuffer 		数据缓存指针地址
 * @param	rLength 		数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
bool bq27426_flash_write(uint32_t addr,uint8_t* databuf,uint16_t len)
{
#if 0
    //本地数据存储，写成功，返回true;写失败，返回false
    E2PDataType_t e2pTemp;

    e2pTemp.addr = addr;
    e2pTemp.pDate = (uint8_t*)databuf;
    e2pTemp.size = len; // sizeof(fuelGauge);
    if (RET_SUCCESS == driver_mcu_eeprom_write((uint8_t*)&e2pTemp, sizeof(e2pTemp))) {
    	//BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"eeprom->write gmfs ok!\n");
    	return true;
    }

    return false;
#else
    return app_param_write((uint8_t)addr, databuf, len);
#endif
}

/****************************************************************************************
 * @brief   IIC读字节
 * @param 	cmdAddress 	 	8bit数据地址
 * @param	rBuffer 		数据缓存指针地址
 * @param	rLength 		数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_read_bytes(uint8_t cmdAddress, uint8_t * rBuffer, uint8_t rLength)
{ 
    if(rLength>BQ27426_IIC_MAX_DATA_SIZE) 
	{
		i2cReadReturn = false;
		return false;
    }
	
    I2cDataType_t rData ;

	rData.busN = I2C_BUS1;
	rData.devAddr = BQ27426_IIC_ADDR;
	rData.wDat = &cmdAddress;
	rData.wLen = 1;
	rData.rDat = rBuffer;
	rData.rLen = rLength;

    if(RET_SUCCESS == driver_i2c_read((uint8_t*)&rData,0))
    {
        i2cReadReturn = true;
	    return true;
    }
	
	i2cReadReturn = false;
	return false;
}
/****************************************************************************************
 * @brief   IIC写字节
 * @param 	cmdAddress 	 	8bit数据地址
 * @param	rBuffer 		数据缓存指针地址
 * @param	rLength 		数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_write_bytes(uint8_t cmdAddress, uint8_t * wBuffer, uint8_t wLength)
{
	if(wLength>BQ27426_IIC_MAX_DATA_SIZE) 
	{
		i2cWriteReturn = false;
		return false;
	}

    I2cDataType_t wData ;
	uint8_t pBuf[BQ27426_IIC_MAX_DATA_SIZE];

	pBuf[0] = cmdAddress;
	for (int i=0; i<wLength; i++) {
		pBuf[i+1] = wBuffer[i];
	}	

	wData.busN = I2C_BUS1;
	wData.devAddr = BQ27426_IIC_ADDR;
	wData.wDat = pBuf;
	wData.wLen = wLength+1;

    if(RET_SUCCESS == driver_i2c_write((uint8_t*)&wData,0))
    {
        i2cWriteReturn = true;
	    return true;
    }
	
	i2cWriteReturn = false;
	return false;
}
#endif
