#ifndef  PD_GLOBAL_H
#define  PD_GLOBAL_H

#include <stdint.h>
#include "pd_pwr.h"
#include "pd_com.h"

#include "sm_log.h"

//#define SILERGY_UART_DEBUG

#ifdef SILERGY_UART_DEBUG
#define DEBUG_INFO(format, arg...)	sm_log(SM_LOG_DEBUG, "%s %d: " format, __func__, __LINE__, ##arg)
#else
#define DEBUG_INFO(format, arg...)
#endif

extern uint8_t  name;
extern pd_pwr_t pd_pwr;

extern uint8_t  debug_src_test_num;
extern uint8_t  debug_snk_test_num;
extern uint8_t  debug_request_next;
extern uint8_t  debug_request_add_voltage;


extern  TCPC_STATUS_Type   tcpc_status;

extern  uint16_t       Expected_Request_Voltage;
extern  uint16_t       time_s;

extern char control_msg_str[25][30];
extern char data_msg_str[16][25];

extern char passcable_cur_str[4][10];
extern char ext_data_msg_str[20][25];
extern char  sink_state_str[40][30];

#endif


