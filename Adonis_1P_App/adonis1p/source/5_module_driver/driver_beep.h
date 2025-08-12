/*
 * driver_beep.h
 *
 *  Created on: 2024年5月9日
 *      Author: S1122385
 */

#ifndef __DRIVER_BEEP_H_
#define __DRIVER_BEEP_H_

#include "stdint.h"

int driver_beep_init(void);
int driver_beep_deinit(void);
int driver_beep_write(uint8_t *pBuf, uint16_t len);




#endif /* SOURCE_5_MODULE_DRIVER_DRIVER_BEEP_H_ */

