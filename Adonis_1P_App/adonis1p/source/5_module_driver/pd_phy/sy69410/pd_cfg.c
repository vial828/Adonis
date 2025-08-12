#include  "pd_cfg.h"


sink_cap_t    sink_output_capa[REQUEST_TYPE_SUPPORTED_MAX];
cable_t       cable_capa;


void PD_Power_Sink_Capability_Init(void)
{
	 /*
	   Note 1:In the case that the Sink needs more than vSafe5V (e.g., 12V) to provide full functionality, then the Higher Capability bit Shall be set.
          2.	
	*/
	//init 5V3A
	sink_output_capa[0].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_0|HIGHER_CAPABILITY_1|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
					DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
	sink_output_capa[0].fixed.operational_current_10ma=300;
	sink_output_capa[0].fixed.voltage_50mv=100;
	
	//init 9V3A
	sink_output_capa[1].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_1|HIGHER_CAPABILITY_1|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
						DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
	sink_output_capa[1].fixed.operational_current_10ma=300;
	sink_output_capa[1].fixed.voltage_50mv=180;
	
	 #ifdef REQUEST_MAX_20V
	
	  sink_output_capa[2].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_1|HIGHER_CAPABILITY_1|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
	                        DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
    sink_output_capa[2].fixed.operational_current_10ma=300;//
	  sink_output_capa[2].fixed.voltage_50mv=280;//12000/50=1200/5=280
		
		//20V  3A

	  sink_output_capa[3].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_1|HIGHER_CAPABILITY_1|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
	                        DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
    sink_output_capa[3].fixed.operational_current_10ma=300;
	  sink_output_capa[3].fixed.voltage_50mv=400;//20000/50=400
		#endif
}

 sink_cap_t*  PD_Get_Sink_Capa(void)
{
    return sink_output_capa;
}

 cable_t*  PD_Get_Cable_Capa(void)
{
    return &cable_capa;
}

uint16_t PD_Get_Cable_Current_Limit(void)
{
	  uint16_t current_limit;
	 
	  switch(cable_capa.passive_cable_t.current_capablity)
		{
		case	 1: {current_limit=3000; break;}
		case	 2: {current_limit=5000; break;}
		default:   
		case	0:  {current_limit=1500; break;}
	  }
	  return current_limit;
}

