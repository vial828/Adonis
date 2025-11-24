#include <stdint.h>
#include "data_base_info.h"

#ifndef HEAT_MONITOR_H

#define HEAT_MONITOR_H

void heat_monitor_init(void);
void heat_monitor(HEATER* heater);
float set_power( uint8_t val );
float get_power( void );
float set_shc_thres( float val );
float get_shc_thres( void );
float set_delta_t( uint8_t val );
uint8_t get_delta_t( void );
#endif // HEAT_MONITOR_H

