#ifndef  __heat_safety_h__
#define  __heat_safety_h__

#include "task_heatting_service.h"
#include "data_base_info.h"


extern float g_power_i;


float heat_get_temp_k(void);
void HalSafetyInit(float startTemp,uint8_t heat_mode);
uint8_t safety_check_before_heating(void);
uint8_t HalSafetyProc(HEATER *Heat_t);
void Hal_clear_Safety_flag(void);
void protect_proc_break(void);
float HalGetCurrProtectCtrlPower(unsigned int HeatTime);
float HalGetProtectCtrlPowerI(unsigned int HeatTime);
void set_ref_tbl(uint8_t mode,int16 *info_p,RefTemp_t *refTemp_p ,float ref_k_0_1s,float ref_k_2_5s);
bool get_ref_tbl(uint8_t mode,int16 *info_p,RefTemp_t *refTemp_p );
bool clear_ref_tbl(void);
uint8_t get_ref_heat_sta(void);
void set_ref_heat_sta(uint8_t sta);

#endif



