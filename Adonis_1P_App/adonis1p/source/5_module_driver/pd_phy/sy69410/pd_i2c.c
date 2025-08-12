/* * * * * * * * * * * *+ * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* @Purpose:  I2C Communication driver(By IO)
* @Author:  
* @Version:  1.0
* @Date:  Create 
* 
* 
* Copyright (C) BlueWhale Tech.   
* All rights reserved.  
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Include Files */
#include "pd_i2c.h"
#include "driver_pd_phy.h"

/**********************************************************************************/
#include "driver_i2c.h"

static I2cDataType_t i2c = {
	.busN = I2C_BUS1,
	.devAddr = I2C_ADDR_SY69410,
	.wLen = 0,
	.rLen = 0
};

static cy_rslt_t pd_phy_write_bytes(uint8_t reg_addr, uint8_t *dat, uint8_t len)
{
	cy_rslt_t rslt = 0;
	uint8_t wBuf[16] = {0};

	if (!(len < sizeof(wBuf)) )
	{
		return false;
	}

	wBuf[0] = reg_addr;
	for (uint8_t i = 0; i < len; i++)
	{
		wBuf[1 + i] = dat[i];
	}
	i2c.wDat = wBuf;
	i2c.wLen = len + 1;

	rslt = driver_i2c_write((uint8_t*) &i2c, 0);

	return rslt; // RET_SUCCESS (0); RET_FAILURE (-1)
}

static cy_rslt_t pd_phy_read_bytes(uint8_t reg_addr, uint8_t *dat, uint8_t len)
{
	cy_rslt_t rslt = 0;

	i2c.wDat = &reg_addr;
	i2c.wLen = 1;
	i2c.rDat = dat;
	i2c.rLen = len;

	rslt = driver_i2c_read((uint8_t*) &i2c, 0);

	return rslt; // RET_SUCCESS (0); RET_FAILURE (-1)
}
/**********************************************************************************/

uint8_t I2CWriteBytes(uint8_t SlaveAddress,uint8_t MemoryAdress,uint8_t wrNumber,uint8_t* wrPointer)
{
	int ret = pd_phy_write_bytes(MemoryAdress, wrPointer, wrNumber);
	return (ret == RET_SUCCESS) ? 1 : 0;
}

uint8_t I2CReadBytes(uint8_t SlaveAddress,uint8_t MemoryAdress,uint8_t rdNumber,uint8_t* rdPointer)
{
	int ret = pd_phy_read_bytes(MemoryAdress, rdPointer, rdNumber);
	return (ret == RET_SUCCESS) ? 1 : 0;
}

void Set1Reg(uint8_t memory_addr, uint8_t value) {
	uint8_t tmp = value;
	int ret = pd_phy_write_bytes(memory_addr, &tmp, 1);
}

uint8_t Get1Reg(uint8_t memory_addr) {
	uint8_t tmp = 0;
	int ret = pd_phy_read_bytes(memory_addr, &tmp, 1);

	return tmp;
}

void Set2Reg(uint8_t memory_addr, uint16_t value)
{
	uint16_t tmp = value;
	int ret = pd_phy_write_bytes(memory_addr, (uint8_t*) &tmp, 2);
}

uint16_t Get2Reg(uint8_t memory_addr)
{
	uint16_t tmp = 0;
	int ret = pd_phy_read_bytes(memory_addr, (uint8_t*) &tmp, 2);

	return (uint16_t)tmp;
}

