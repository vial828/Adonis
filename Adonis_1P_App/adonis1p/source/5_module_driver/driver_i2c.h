
#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H

#include "cyhal.h"
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


//#include "driver_pin.h"
#include "sm_log.h"

#include <math.h>  

#include "public_typedef.h"

#define I2C_WR_TIMEROUT     30u// ms
/**
  * @brief i2c bus 
  */
typedef enum
{
	I2C_BUS0		= 0,
	I2C_BUS1,
	I2C_BUS_NUM,
}I2cBus_e;



/**
  * @brief i2c slave device address
  */
typedef enum
{
	/*bus0*/
	I2C_ADDR_AW21009 			= 0x20,//AW21009
	I2C_ADDR_NU1680 			= 0x60,//NU1680 WIRELESS_CHARGING
	I2C_ADDR_HUSB238 			= 0x08,//HUSB238
	I2C_ADDR_MP2651 			= 0x5C,//MP2651
	I2C_ADDR_MP2770 			= 0x4B,//MP2770 -->Charge
	I2C_ADDR_BQ25638			= 0x6B,//BQ25638 --> Charge
	I2C_ADDR_BQ27426			= 0x55,//BQ27426 --> Fuel Gauge
	I2C_ADDR_AW35615			= 0x22,//AW35615 --> PD-PHY
	I2C_ADDR_SY69410			= 0x4E,//SY69410 --> PD-PHY
	I2C_ADDR_TPS55288			= 0X74,//TPS55288 -->DCDC
	/*bus1*/
	I2C_ADDR_HALL 				= 0x12,//MT8001
}I2cSlaveAddr_e;



typedef struct
{
	I2cBus_e busN;	// bus ID
	I2cSlaveAddr_e devAddr;// device address
	uint8_t *wDat;//write data point
	uint8_t wLen; // write length
	uint8_t *rDat;//read data point
	uint8_t rLen; // read length
}I2cDataType_t;






//FOR 1PC
#define CYBSP_I2C_BUS0_SCL P6_0
#define CYBSP_I2C_BUS0_SDA P6_1

//FOR 2PC
//#define CYBSP_I2C_BUS0_SCL P0_2
//#define CYBSP_I2C_BUS0_SDA P0_3


#define CYBSP_I2C_BUS1_SCL P6_4
#define CYBSP_I2C_BUS1_SDA P6_5



extern int driver_i2c_init(void);
extern int driver_i2c_deinit(void);
extern int driver_i2c_write(uint8_t *pBuf, uint16_t len);
extern int driver_i2c_read(uint8_t *pBuf, uint16_t len);


#endif



