/****************************************************************************************
*оƬ:     GDE230
*�ļ���:   ���Ȼ���������ģ��
*����:     Eason.cen
*ʱ��:     2024/04/07
*�汾��:    V0.2
*�޸ļ�¼:  fun_hot_coldģ��
****************************************************************************************/

#ifndef    _HEAT_HOT_COLD
#define    _HEAT_HOT_COLD    1
#include "stdio.h"

#include "platform_io.h"
#include "task_heatting_service.h"
#include "system_status.h"
#include "sm_log.h"
#include <stdio.h>
#include <string.h>
#include "timers.h"
#include "driver_heater.h"
#include "cyhal.h"
typedef struct
{
    uint32_t hot_time;
	  uint32_t cold_time;
}str_hot_cold;

typedef struct  dev_t
{
	float Heat_k_temp;    //�������ȵ�ż�¶�
    float bord_ntc_temp;
    float TypeC_ntc_temp;
} DEV_T;

float hal_get_hot_k(DEV_T dev_temp);
 
#endif
