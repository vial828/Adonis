#include <stdint.h>
#include "data_base_info.h"

#ifndef HEAT_MONITOR_H

#define HEAT_MONITOR_H

void heat_monitor_init(void);
void heat_monitor(HEATER* heater);
extern float delta_t;

#endif // HEAT_MONITOR_H

