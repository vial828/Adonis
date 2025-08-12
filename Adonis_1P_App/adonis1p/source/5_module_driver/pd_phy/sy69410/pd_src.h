#ifndef PD_SRC
#define PD_SRC

#include "stdint.h"
#include "stddef.h"
#include "pd_com.h"




typedef enum
{ 
	PE_SRC_Default=0,
	PE_SRC_Startup,
	PE_SRC_Discovery,
	PE_SRC_Cable_Detective,
	PE_SRC_Wait_Identity,
	PE_SRC_Send_Capablities,
	PE_SRC_Wait_Request,
	PE_SRC_Negotiate_Capability,
	PE_SRC_Send_Accept,
	PE_SRC_Send_Reject,
	PE_SRC_Transition_Supply,
	PE_SRC_Ready,
	PE_SRC_Get_Sink_Cap,
	PE_SRC_Give_Source_Cap,
	PE_SRC_Hard_Reset,       
	PE_SRC_Soft_Reset,        
	PE_SRC_Soft_Reset_Receive, 
	PE_SRC_Hard_Reset_Received,
	PE_SRC_BIST_TEST_DATA,
	PE_SRC_BIST_Carried_Mode,  
	PE_SRC_VCONN_SWAP,
	PE_SRC_PRSWAP,
    PE_SRC_Send_NotSupported,
    PE_SRC_Send_Psrdy,
	PE_SRC_Disable,
	PE_SRC_Send_Vconn_Swap,
	PE_SRC_Send_Pr_Swap,
	PE_SRC_Send_PPS_Status,
	
#ifdef DEBUG_BIST_TEST_DATA
	 PE_SRC_BIST_SEND_AA_SOP,         
	 PE_SRC_BIST_SEND_55_SOP,            
	 PE_SRC_BIST_SEND_00FF_SOP,          
	 PE_SRC_BIST_AA_SOP_P,         
	 PE_SRC_BIST_55_SOP_PP,              
	 PE_SRC_BIST_FF_SOP_P_DEBUG,         
	 PE_SRC_BIST_00_SOP_PP_DEBUG,  
   PE_SRC_BIST_SEND_SORT_MSG,
	 PE_SRC_BIST_SEND_BIST_CARRY2,
#endif
}PD_src_state_e;


typedef enum
{
	FSM_ADJUST_TO_VSAFE5V=0,
	FSM_ADJUST_TO_VSAFE0V,
	FSM_WAIT_SRC_RECOVER,
	FSM_RESTORE_TO_VSAFE5V,
	FSM_RESTART_COMMUNICATION
}pd_hard_reset_power_adjust_fsm_e;


typedef enum
{
   FSM_PD_SRC_SR_SEND_SOFTRRESET=0,
	 FSM_PD_SRC_SR_WAIT_ACCEPT,
	 FSM_PD_SRC_SR_RESTART
}pd_src_soft_reset_adjust_fsm_e;


typedef enum
{
		FSM_PD_SRC_PRSWAP_EVALUAT_SWAP=0,
	  FSM_PD_SRC_PRSWAP_ACCEPT_SWAP,
	  FSM_PD_SRC_PRSWAP_TRANSITION_TO_OFF,
	  FSM_PD_SRC_PRSWAP_ASSERT_RD,
	  FSM_PD_SRC_PRSWAP_WAIT_SOURCE_ON,
	  FSM_PD_SRC_PRSWAP_SEND_SWAP,
	  FSM_PD_SRC_PRSWAP_WAIT_RESPONSE
}pd_src_prswap_fsm_e;



void  PD_SRC_Discover_Identity_Send(void);
void  Vendor_PD_SRC(void);



void  PD_SRC_Send_Hard_Reset(void);
void  PD_SRC_Send_Soft_Reset(void);
void  PE_SRC_Send_Capablities_act(void);


void  PD_SRC_Send_SOURCE_CAP(void);
void  PD_SRC_Send_SINK_CAP(void);
void  PD_SRC_Send_Not_Supported(void);
void  PD_SRC_Send_PPS_Status(void);



void  PE_SRC_Send_NotSupported_act(void);
void  PE_SRC_Ready_act(void);
void  PE_SRC_Wait_Request_act(void);
void  PE_SRC_Wait_Identity_act(void);
void  PE_SRC_Discovery_act(void);

void  PE_SRC_BIST_Carried_Mode_act(void);
void  PE_SRC_Transition_Supply_act(void);
void  PE_SRC_Send_Reject_act(void);
void  PE_SRC_Soft_Reset_Receive_act(void);
void  PD_SRC_Init(void);


void  Pd_SRC_Advertise_PPS_Status(void);

void  PE_SRC_Send_Psrdy_act(void);
void  PE_SRC_Send_Accept_act(void);	

void  PE_SRC_Discover_CableID_act(void); 
bool  PD_SRC_Process_Request(void);

void  PE_SRC_Send_PPS_Status_Act(void);


void  PE_SRC_Wait_Response_Vcon_Swap_act(void);								
void  PE_SRC_Wait_Response_Vcon_Swap_act(void);


void   PD_Process_Protocol_Error(void);
void   PD_SRC_VDMResponse_Timeout_act(void);

void  PE_SRC_Send_Control_Msg_act(CONTROL_MESSAGE_TYPE_e message_type,PD_src_state_e success_path,uint8_t success_send_flag,PD_src_state_e fail_path,uint8_t fail_send_flag);
void  PE_SRC_Send_Complx_Msg_act(void(*action)(),PD_src_state_e success_path,uint8_t success_send_flag,PD_src_state_e fail_path,uint8_t fail_send_flag);

void   SRC_PRSWAP_fsm(void);
static fsm_result_e  _fsm_pd_sr_prswap_evaluate_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
static fsm_result_e  _fsm_pd_sr_prswap_accept_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
static fsm_result_e  _fsm_pd_sr_prswap_transtion_to_off(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
static fsm_result_e  _fsm_pd_sr_prswap_assert_rd(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
static fsm_result_e  _fsm_pd_sr_prswap_wait_source_on(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
static fsm_result_e  _fsm_pd_sr_prswap_send_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
static fsm_result_e  _fsm_pd_sr_prswap_wait_response(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);

void  PD_SRC_Soft_Reset_fsm(void);
static fsm_result_e  _fsm_pd_sr_send_soft_reset(pd_src_soft_reset_adjust_fsm_e * pd_src_soft_reset_fsm);
static fsm_result_e  _fsm_pd_sr_wait_accept(pd_src_soft_reset_adjust_fsm_e * pd_src_soft_reset_fsm);
static fsm_result_e  _fsm_pd_sr_restart(pd_src_soft_reset_adjust_fsm_e * pd_src_soft_reset_fsm);

void   SRC_Hard_Reset_fsm(void);

static fsm_result_e _fsm_pd_hardreset_power_adjust_to_Vsafe5V(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm );
static fsm_result_e _fsm_pd_hardreset_power_recover_to_Vsafe5V(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm);
static fsm_result_e _fsm_pd_hardreset_power_adjust_to_Vsafe0V(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm);
static fsm_result_e _fsm_pd_hardreset_power_wait_srcrecover(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm);
static fsm_result_e _fsm_pd_hardreset_restart_to_communication(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm);

void SRC_VCONN_Swap_fsm(void);

typedef enum
{
	  FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP=0,
	  FSM_PD_SRC_VCON_SWAP_ACCEPT_SWAP,
	  FSM_PD_SRC_VCON_SWAP_WAIT_FOR_VCONN,
	  FSM_PD_SRC_VCON_SWAP_TURN_ON_VCONN,
	  FSM_PD_SRC_VCON_SWAP_TURN_OFF_VCONN,
	  FSM_PD_SRC_VCON_SWAP_SEND_PS_RDY,
	  FSM_PD_SRC_VCON_SWAP_SEND_SWAP,
	  FSM_PD_SRC_VCON_SWAP_FORCE_VCONN, 
	  FSM_PD_SRC_VCON_SWAP_WAIT_RESPONSE,
}pd_src_vcon_swap_fsm_e;

static fsm_result_e _fsm_pd_src_vcon_swap_evaluat_swap(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_accept_swap(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_wait_for_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_turn_on_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_turn_off_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_send_ps_rdy(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_send_swap(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_force_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
static fsm_result_e _fsm_pd_src_vcon_swap_wait_response(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm);
#endif

