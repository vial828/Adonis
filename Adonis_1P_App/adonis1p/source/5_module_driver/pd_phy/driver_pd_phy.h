
#ifndef __DRIVER_PD_PHY_H
#define __DRIVER_PD_PHY_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "sm_log.h"


#define AW35615_7BIT_DEV_I2IC_ADDR	(0x22)
#define SY69410_7BIT_DEV_I2IC_ADDR	(0x4E)

typedef enum {
	PD_AW35615 = 0,
	PD_SY69410,
	PD_NONE
} PdPhyType_t;

typedef enum {
	PD_SET_INIT = 0,
	PD_SET_PROC,
	PD_SET_POS,
	PD_SET_DEBUG
} PdPhySet_t;

/**************************************************************/
extern void pd_phy_mDelay(uint16_t ms);
extern void pd_timer_refresh(void);

extern int driver_pd_phy_init(void);
extern int driver_pd_phy_deinit(void);

extern int driver_pd_phy_write(uint8_t *pBuf, uint16_t len);
extern int driver_pd_phy_read(uint8_t *pBuf, uint16_t len);

#endif
/**************************************************************/
