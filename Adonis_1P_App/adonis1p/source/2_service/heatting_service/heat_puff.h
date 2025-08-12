#ifndef  __heat_puff_h__
#define  __heat_puff_h__

#include "stdint.h"
extern float g_puff_k;
extern unsigned char g_heat_puff;
float heat_get_puff_ref_temp(unsigned int HeatTime);
void heat_puff_proc(uint8_t *puff_p,float temp_cur,unsigned int time);

float heat_get_puff_k(void);

void heat_puff_proc2(float power_cur,unsigned int time); //20MS 调用一次

void heat_puff_init(void);
uint8_t heat_puff_get(void);
#endif
