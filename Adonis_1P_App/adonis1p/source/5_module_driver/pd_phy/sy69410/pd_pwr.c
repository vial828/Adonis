#include "pd_pwr.h"

void OTG_On()
{
	 OTG_EN_Set(1);
}

void OTG_Off()
{
	 OTG_EN_Set(0);
}


/*
 void Sy6961_Init(void);
 uint16_t Read_ADC(uint8_t sel);
 uint16_t Read_Fault(void);
 void OTG_EN_Set(uint8_t a);
 void OTG_Voltage_Set(uint16_t   voltage_mv);
 void OTG_Current_Set(uint16_t   current_ma);
 void Input_Voltage_Set(uint16_t voltage_mv);
 void Input_Current_Set(uint16_t current_ma);
 void Charge_Current_Set(uint16_t current_ma);
 void Charge_Voltage_Set(uint16_t voltage_mv);
*/



