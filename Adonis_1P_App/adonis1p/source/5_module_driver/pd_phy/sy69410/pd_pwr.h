#ifndef PD_PWR
#define PD_PWR

#include "stdint.h"

typedef struct
{
    uint8_t(*Init)(void); 
    void(*OTG_On)(void);
    void(*OTG_Off)(void);
    void(*Output_Voltage_Set)(uint16_t   voltage_mv);
    void(*Output_Current_Set)(uint16_t   current_ma);
    void(*Input_Voltage_Set)(uint16_t voltage_mv);
    void(*Input_Current_Set)(uint16_t current_ma);
    void(*Charge_Current_Set)(uint16_t current_ma);
    void(*Charge_Voltage_Set)(uint16_t voltage_mv);
    uint16_t(*Read_ADC)(uint8_t sel);
}pd_pwr_t;

void OTG_On(void);
void OTG_Off(void);

#endif


