#ifndef DEV_I2C_BQ27426_H
#define DEV_I2C_BQ27426_H

#include "driver_i2c.h"
#include "bq27426.h"


typedef enum {
	FG_SET_INIT = 0,
	FG_SET_REFRESH_GMFS,
	FG_SET_SAVE_GMFS,
	FG_SET_TAPER_RATE,
	FG_SET_TAPER_VOLT,
	FG_SET_SHUTDOWN,
	FG_SET_DEBUG
} FuelGaugeSet_e;

/**************************************************************/
/**************************************************************/
extern int driver_fuel_gauge_init(void);
extern int driver_fuel_gauge_deinit(void);
extern int driver_fuel_gauge_write(uint8_t *pBuf, uint16_t len);
extern int driver_fuel_gauge_read(uint8_t *pBuf, uint16_t len);

#endif
