#include "pd_global.h"
// #include "pd_pwr.h"
// #include "SY6961.h"

uint8_t name;
/*
pd_pwr_t pd_pwr={
	 .Init=Sy6961_Init,
	 .OTG_On=OTG_On,
   .OTG_Off=OTG_Off,
   .Output_Voltage_Set=OTG_Voltage_Set,
	 .Output_Current_Set=OTG_Current_Set,
	 .Input_Voltage_Set=Input_Voltage_Set,
	 .Input_Current_Set=Input_Current_Set,
	 .Charge_Current_Set=Charge_Current_Set,
	 .Charge_Voltage_Set=Charge_Voltage_Set,
	 .Read_ADC=Read_ADC
};

*/

uint8_t  flag_sysclk_start;
uint16_t Expected_Request_Voltage;

uint8_t  debug_snk_test_num;
uint8_t  debug_request_next;
uint8_t  debug_request_add_voltage;
uint8_t  debug_src_test_num;

uint16_t time_s;

 char control_msg_str[25][30]={
     "UNRECGNITED",
     "GOODCRC",
     "GOTOMIN",
     "ACCEPT",
     "REJECT",
     "PING",
     "PS_RDY",
     "GET_SOURCE_CAP",
     "GET_SINK_CAP",
     "DR_SWAP",
     "PR_SWAP",
     "VCONN_SWAP",
     "WAIT",
     "SOFT_RESET",
     "DATA_RESET",
     "DATA_RESET_COMPLETE",
     "NOT_SUPPORTED",
     "GET_SOURCE_CAP_EXTEND",
     "GET_STATUS_PD",
     "FR_SWAP",
    "GET_PPS_STATUS",
    "GET_COUNTRY_CODES",
    "GET_SINK_CAP_EXTENED",
    "GET_SOURCE_INFO",
    "GET_REVISION"
};
 
 char ext_data_msg_str[20][25]={
	  "RESERVED",
    "SOURCE_CAP_EXTENDED",
    "STATUS",
    "GET_BATTERY_CAP",
    "GET_BATTERY_STATUS",
    "BATTERY_CAPABILITIES",
    "GET_MANUFACTURER_INFO",
    "MANUFACTURER_INFO",
    "SECURITY_REQUES",
    "SECURITY_RESPONSE",
    "FIRMWARE_UPDATE_REQUEST",
    "FIRMWARE_UPDATE_RESPONSE",
    "PPS_STATUS",
    "COUNTRY_INFO",
    "COUNTRY_CODES",
    "SINK_CAP_EXTENDED",
	  "EXTENDED_CONTROL",
	  "EPR_SOURCE_CAPABILITIES",
	  "EPR_SINK_CAPABILITIES"
};
 

char passcable_cur_str[4][10]={
  "Reserve",
	"3A",
	"5A",
	"Reserve"
};
 

char data_msg_str[16][25]={
    "ERSERVED",
    "SOURCE_CAPABILITIES",
    "REQUEST",
    "BIST",
    "SINK_CAPABILITIES",
    "BATTERY_STATUS",
	  "ALERT",
    "GET_COUNTRY_INFO",
    "ENTER_USB",//8
	  "EPR_REQUEST",
	  "EPR_MODE",
		"SOURCE_INFO",
		"REVSION",
		"ERSERVED",
	  "ERSERVED",
    "VENDER_DEFINED" //15
};


char sink_state_str[40][30]={
  "PE_SNK_Default",
	"PE_SNK_Startup",
	"PE_SNK_Discovery",
	"PE_SNK_Hard_Reset_Received",
	"PE_SNK_Wait_for_Capablities",
	"PE_SNK_Evaluate_Capablity",
	"PE_SEN_Send_Request",
  "PE_SNK_Wait_Accept",
	"PE_SNK_Transition_Sink",
	"PE_SNK_Ready",
	"PE_SNK_Give_Sink_Cap",
	"PE_SNK_Hard_Reset",
	"PE_SNK_Soft_Reset",
	"PE_SNK_Transition_to_Default",
	"PE_SNK_Soft_Reset_Receive",
	"PE_SNK_Send_NotSupported",
	"PE_SNK_BIST_TEST_DATA",
	"PE_SNK_BIST_Carried_Mode",
	"PE_SNK_Give_Source_Cap",
	"PE_SNK_VCONN_SWAP",
	"PE_SNK_Hard_Reset_Wait",
	"PE_SNK_Send_GetSRCCAP",
	"PE_SNK_PR_SWAP",
	"PE_SNK_Disable",
	"PE_SNK_Send_Debug",
	
	"PE_SNK_Send_EPR_Enter",   
	"PE_SNK_wait_EPR_Enter_Result",
	"PE_SNK_wait_EPR_SOURCE_CAP",
	"PE_SNK_Send_EPR_Alive",
	"PE_SNK_Send_EPR_Request",
};
//send_flag_t    src_next_send_msg;

