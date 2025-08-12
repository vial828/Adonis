#include   "pd_src.h"
#include   "pd_com.h"
#include   "pd_cfg.h"
#include   "pd_global.h"
#include    "pd_pwr.h"
#include    "pd_timer.h"
#include   "pd_snk.h"
#include "public_typedef.h"


#ifdef DEBUG_BIST_TEST_DATA
void PD_Send_BIST_TEST_DATA(SPEC_REV_TYPE_e rev, uint8_t byte,SOP_TYPE_e msg_type);
void PD_BIST_TEST_act(SPEC_REV_TYPE_e rev, uint8_t byte,SOP_TYPE_e msg_type);
void PE_SRC_Send_Get_SINK_Cap_act(void);
void PE_SRC_Send_BIST_CARRY_MODE2_act(void);
#endif


extern  pd_status_t pd_status;
extern  pd_phy_t pd;
/* 1.Add 			   if(rdo_position==1)  tcpc_status.above5V=0;
                 else tcpc_status.above5V=1;
			           in process of judge request,preparing for sending BIST carry mode;
	 2.@2022.03.30    Add protocol error process in state of waiiting request-->send soft reset
	 3.Add process:   if protocol error happpens during power transitioning-->hard reset in send Accept or Reject			     
*/

typedef void fun_t(void);

#define   SEND_NEW       1  
#define   NOT_SEND_NEW   0
#define   VBUS_REG           0x27   

extern void                                       PD_Send_Sink_Cap_act(void);

static uint8_t                                    flag_src_send_PD_msg;
static uint8_t                                    flag_src_soft_reset_sysclk_start;
static uint8_t                                    flag_src_hard_reset_fsm_start_en;
static uint8_t                                    flag_src_hard_reset_sysclk_start;
static uint8_t                                    flag_src_pr_swap_sysclk_start;
static uint8_t                                    flag_src_vcon_swap_sysclk_start;

static pd_src_soft_reset_adjust_fsm_e             fsm_pd_src_soft_reset_fsm;
static pd_hard_reset_power_adjust_fsm_e           fsm_pd_src_hard_reset;
static pd_src_prswap_fsm_e                        fsm_pd_src_prswap;
static pd_src_vcon_swap_fsm_e                     fsm_pd_src_vcon_swap;


/*
The check fails if the meaaage ID  is not 000b under the following conditions:  
a.  The first message on each SOP* type after a Hard Reset was sent or received  
b.  The message is a Soft_Reset  
c.  The first message on the same SOP* type after receiving Soft_Reset 
d.  The first message on SOP after a successful Power Role Swap 
e.  The first message on SOP after a successful Fast Role Swap 
f.  The first message on each SOP* type upon initial entry to Attached state 
g.  The first message on each SOP�� and SOP�� after a Cable Reset 
*/

static void (*SRC_Process_Control_Msg[])()
	={
		 NULL,   // case    UNRECGNITED: 			      break;//do nothing
		 NULL,	// case    GOODCRC :				        break;//do noth                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ing 
		 PD_SRC_Send_Soft_Reset, 	//	 case    GOTOMIN:                                                             ;
		 PD_SRC_Send_Soft_Reset, //		 case    ACCEPT:                                                              ;
		 PD_SRC_Send_Soft_Reset,// 		 case    REJECT:                 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;   flag_src_send_PD_msg=1;               break;
		 NULL,  //	 case    PING:                    break;//do nothing 
		 PD_SRC_Send_Soft_Reset,//	 case    PS_RDY :                tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;   flag_src_send_PD_msg=1; break;// do nothing 
		 PD_SRC_Send_SOURCE_CAP,	//	 case    GET_SOURCE_CAP :        tcpc_status.PDstate_SRC=PE_SRC_Send_Capablities;    flag_src_send_PD_msg=1;    break;
		 PD_SRC_Send_SINK_CAP,//	 case    GET_SINK_CAP :          tcpc_status.PDstate_SRC=PE_SRC_Get_Sink_Cap;        flag_src_send_PD_msg=1;    break;
		 PD_SRC_Send_Not_Supported,//	 case    DR_SWAP :               tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;   flag_src_send_PD_msg=1;    break;//reject would also be a goog choice according to PD protocol
		 NULL,//	 case    PR_SWAP:                break;// process outside
		 NULL,//	 case    VCONN_SWAP :            tcpc_status.PDstate_SRC=PE_SRC_VCONN_SWAP;     break;    //2ѡ1, ACCEPT or Not supported
		 PD_SRC_Send_Soft_Reset,//	 case    WAIT :                  tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;   flag_src_send_PD_msg=1;       break;    
		 NULL,//	 case    SOFT_RESET :             break;    //process outside 
		 PD_SRC_Send_Not_Supported,//	 case    DATA_RESET :             tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;
		 PD_SRC_Send_Not_Supported,//	 case    DATA_RESET_COMPLETE:      tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;  break;    
		 NULL,//	 case    NOT_SUPPORTED :          break;    //do nothing
		 PD_SRC_Send_Not_Supported,//	 case    GET_SOURCE_CAP_EXTEND :     tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break; //sending src_cap_extened is also correct response; 
		 PD_SRC_Send_Not_Supported,//	 case    GET_STATUS_PD :          tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;//
		 PD_SRC_Send_Not_Supported,//	 case    FR_SWAP :                tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;
		 PD_SRC_Send_PPS_Status, //		 case    GET_PPS_STATUS :          tcpc_status.PDstate_SRC=PE_SRC_Send_PPS_Status;   flag_src_send_PD_msg=1;   break;//UUT must send PPS satus to tester;
		 PD_SRC_Send_Not_Supported,//		 case    GET_COUNTRY_CODES :      tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;// The check fails if the UUT does not send either Not_Supported or Country_Codes message. 
		 PD_SRC_Send_Not_Supported,//		 case    GET_SINK_CAP_EXTENED :     break; //Send  not support
		 PD_SRC_Send_Not_Supported//	 	 default:                         tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;
	 };
	
void   PD_SRC_Send_Soft_Reset(void)
{
	 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	
	 flag_src_send_PD_msg=1;
}

void   PD_SRC_Send_Not_Supported(void)
{
		 tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;    
		 flag_src_send_PD_msg=1;
}

void PD_SRC_Send_SOURCE_CAP(void)
{
	  tcpc_status.PDstate_SRC=PE_SRC_Send_Capablities;    
	  flag_src_send_PD_msg=1;  
}


void PD_SRC_Send_SINK_CAP(void)
{
	  tcpc_status.PDstate_SRC=PE_SRC_Get_Sink_Cap;    
	  flag_src_send_PD_msg=1;  
}

void PD_SRC_Send_PPS_Status(void)
{
	  tcpc_status.PDstate_SRC=PE_SRC_Send_PPS_Status;    
	  flag_src_send_PD_msg=1;  
}

static void PD_SRC_Send_PS_RDY(void)
{
	 flag_src_send_PD_msg =1; 	
	 tcpc_status.PDstate_SRC = PE_SRC_Send_Psrdy;
}
	
#if 0
void PD_Protocol_Reset(void)
{
	  uint8_t i;
	/*
	  tcpc_status.rx_stored_msgID=-1;
	 // tcpc_status.messageID=0;
	  tcpc_status.msg_id_tx[TYPE_SOP]=0;
	  tcpc_status.msg_id_tx[TYPE_SOPP]=0;
	  tcpc_status.msg_id_tx[TYPE_SOPPP]=0;
	  tcpc_status.msg_id_tx[TYPE_SOP_DBGG]=0;
	  tcpc_status.msg_id_tx[TYPE_SOP_DBGGG]=0;
	*/
	
	  for(i=0;i<5;i++)
	  {
			 tcpc_status.msg_id_tx[i]=0;
			 tcpc_status.msg_id_rx[i]=0;
			 tcpc_status.msg_id_rx_init[i]=0;
		}
}
#endif

extern void PD_Protocol_Reset(void);

void  Vendor_PD_SRC(void)
{
	if(pd.is_hard_reset_received()  )// Receive Hard_Reset
	{
		pd.enable_bist_data_mode(0);
		//BIST_TEST_DATA_Mode_Quit();	
		tcpc_status.hard_resrt_on=1;
	  flag_src_hard_reset_fsm_start_en=1;
		fsm_pd_src_hard_reset=FSM_ADJUST_TO_VSAFE5V;
		Stop_Sysclock();
		flag_src_pr_swap_sysclk_start=FALSE;
		Gtimer_Stop(GTIMER_SEL3);
		Gtimer_Stop(GTIMER_SEL4);
		tcpc_status.PDstate_SRC=PE_SRC_Hard_Reset_Received;
		DEBUG_INFO("\r\nRX-<HARD_RESET");
		
	}
	else switch(tcpc_status.PDstate_SRC)
  {
		case   PE_SRC_Default:          
																				 // PD_SRC_Init();  //reset power role and data role, EN PD receive,reset capscount,reset message ID,cable current capablity
																				 
		                                     #ifdef VCONN_EN
		                                         PD_SRC_Init();
																					if(tcpc_status.ra==0)
																					{
																						 tcpc_status.PDstate_SRC=PE_SRC_Startup;  
																					}
																					else if(tcpc_status.ra==1 && (pd_status.VCONN_ON==0))//VCONN ��û�д�
																					{
																						 pd.vcon_on();   //T_VCONStable=50ms
																						 start_Vconn_Timer(GTIMER_SEL3, T_FIRST_DISCOVER_IDENTITY, PD_SRC_VDMResponse_Timeout_act);//wait time max=45ms even though vconn_present=0  
																						 flag_src_send_PD_msg=1;
																						 tcpc_status.PDstate_SRC=PE_SRC_Cable_Detective;
																					}	
																					#else
																					   PD_SRC_Init();
																					   tcpc_status.PDstate_SRC=PE_SRC_Startup; //ֱ������
																					#endif
																				 break;
		case   PE_SRC_Cable_Detective: 
		 	                                   PE_SRC_Discover_CableID_act();   break;
    case   PE_SRC_Startup:              //after hard reset or prswap
			                                   tcpc_status.capscounter=0;
	                                       tcpc_status.pd_rev=REV_3;
	                                       PD_Protocol_Reset();
																				if(pd.is_vbus_ok()) //Vbus is present
																				{
																					 pd.clear_new_msg_received();
																						if(tcpc_status.CC1_PD ==1)
																							 pd.set_cc_role(NO_DRP_MODE,RP_1P5A,Open_CC,RP_CC);		//CC2VCON 
																						else
																							 pd.set_cc_role(NO_DRP_MODE,RP_1P5A,RP_CC,Open_CC);	
																					 //pd.set_cc_role(NO_DRP_MODE,RP_1P5A,RP_CC,RP_CC);
																					 tcpc_status.PDstate_SRC=PE_SRC_Send_Capablities;
																					 //Start_SourceCapablityTimer(GTIMER_SEL3, T_FIRST_SOURCE_CAP, PD_Advertise_SRC_Capablity);//seif defined : T_FIRST_SOURCE_CAP=150ms��time delay before sending source cap
																				}
																			//	else
																			//			DEBUG_INFO("\r\n VC not OK");
																				break;
		case    PE_SRC_Discovery:           PE_SRC_Discovery_act();          break;   
    case    PE_SRC_Wait_Identity:
		 									                  PE_SRC_Wait_Identity_act();      break;
	  case    PE_SRC_Send_Capablities:  
		 	                                  PE_SRC_Send_Capablities_act();   break;
	  case    PE_SRC_Wait_Request:  
		 	                                  PE_SRC_Wait_Request_act();       break;
	  case    PE_SRC_Negotiate_Capability:
			 	                                 if(PD_SRC_Process_Request())
																					   tcpc_status.PDstate_SRC=PE_SRC_Send_Accept;
																				 else
																					    tcpc_status.PDstate_SRC=PE_SRC_Send_Reject;
			 	                                                                 break;
	   case    PE_SRC_Send_Psrdy:           PE_SRC_Send_Psrdy_act();             break;         
	   case    PE_SRC_Send_Accept:          PE_SRC_Send_Accept_act();            break;
	   case    PE_SRC_Send_Reject:          PE_SRC_Send_Reject_act();            break;																				 
		 case    PE_SRC_Transition_Supply:    PE_SRC_Transition_Supply_act();		     break;
		 case    PE_SRC_Ready:                PE_SRC_Ready_act();                    break;
     case    PE_SRC_Hard_Reset:           SRC_Hard_Reset_fsm();                  break;
		 case    PE_SRC_Soft_Reset:           PD_SRC_Soft_Reset_fsm();               break; 
		 case    PE_SRC_Soft_Reset_Receive:   PE_SRC_Soft_Reset_Receive_act();       break;
		 case    PE_SRC_Hard_Reset_Received:  SRC_Hard_Reset_fsm();                  break;																				 
		 case    PE_SRC_PRSWAP:               SRC_PRSWAP_fsm();                      break;
     case    PE_SRC_VCONN_SWAP:           SRC_VCONN_Swap_fsm();                  break;
		 case    PE_SRC_Get_Sink_Cap:         PD_Send_Sink_Cap_act();                break;
		 case    PE_SRC_Send_NotSupported:    PE_SRC_Send_NotSupported_act();        break;
		 case    PE_SRC_Send_PPS_Status:      PE_SRC_Send_PPS_Status_Act();          break;
		 case    PE_SRC_BIST_TEST_DATA:                                              break; // donothing wait hard reset to quit this mode  
		 case    PE_SRC_BIST_Carried_Mode:    PE_SRC_BIST_Carried_Mode_act();        break;
	   case    PE_SRC_Disable:                                                     break;// do nothing 	
     
     #ifdef DEBUG_BIST_TEST_DATA																				 
		 case    PE_SRC_BIST_SEND_AA_SOP:     PD_BIST_TEST_act(REV_2,0XAA,TYPE_SOP); break;
		 case    PE_SRC_BIST_SEND_55_SOP:     PD_BIST_TEST_act(REV_3,0X55,TYPE_SOP); break;
		 case    PE_SRC_BIST_SEND_00FF_SOP:   PD_BIST_TEST_act(REV_3,0X00,TYPE_SOP);          break;
		 case    PE_SRC_BIST_AA_SOP_P:        PD_BIST_TEST_act(REV_3,0XAA,TYPE_SOPP);         break;
		 case    PE_SRC_BIST_55_SOP_PP:       PD_BIST_TEST_act(REV_3,0X55,TYPE_SOPPP);        break;
		 case    PE_SRC_BIST_FF_SOP_P_DEBUG:  PD_BIST_TEST_act(REV_3,0XFF,TYPE_SOP_DBGG);     break;
		 case    PE_SRC_BIST_00_SOP_PP_DEBUG:   PD_BIST_TEST_act(REV_3,0X00,TYPE_SOP_DBGGG);  break;
	   case    PE_SRC_BIST_SEND_SORT_MSG:    PE_SRC_Send_Get_SINK_Cap_act();               break;
		 case    PE_SRC_BIST_SEND_BIST_CARRY2:  PE_SRC_Send_BIST_CARRY_MODE2_act(); 
		 #endif
		 default  :                                                                    break;
		 
		 
    }
}

void PE_REQUEST_CARRYMODE2(void)
{
		uint8_t i;
		PD_MSG_t t_mes;
		t_mes.header.extended=0;
		t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
		t_mes.header.message_Type=BIST;
		t_mes.header.number_Data_Objects=1;
		t_mes.header.port_Data_Role=tcpc_status.data_role;
		t_mes.header.port_Power_Role=tcpc_status.power_role;
		t_mes.header.spec_Rev=tcpc_status.pd_rev;
			tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
		for(i=0;i<1;i++)
		{
			t_mes.data[i]=0x40000000;
		}
		t_mes.Sop_Type=TYPE_SOP;
		pd.send_msg(t_mes);
	
}

void PE_SRC_Send_BIST_CARRY_MODE2_act(void)
{

	   PE_SRC_Send_Complx_Msg_act(PE_REQUEST_CARRYMODE2,
	                              PE_SRC_Ready,NOT_SEND_NEW,//success path
	                              PE_SRC_Soft_Reset,SEND_NEW);//fail path
	
}
void PE_SRC_Send_Get_SINK_Cap_act()
{
		PE_SRC_Send_Control_Msg_act(  GET_SINK_CAP,//send control messageg type
	                                PE_SRC_Ready,NOT_SEND_NEW,//success path
	                                PE_SRC_Ready,NOT_SEND_NEW);//fail path
}

void PE_SRC_Discover_CableID_act() //send  discovery
{
	  
		if( flag_src_send_PD_msg==1 && pd_status.vconn_present==1) //seng message as soon as vcconn is present; no wait 
		{
			Stop_Vconn_Timer(GTIMER_SEL3);
			PD_SRC_Discover_Identity_Send();
			flag_src_send_PD_msg=0;
		}
		switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
				case TRANSMIT_FAIL:   tcpc_status.PDstate_SRC=PE_SRC_Startup;	
															break;   //
				case TRANSMIT_DISCARD:  
														  flag_src_send_PD_msg =1;     	                                                            
														  break;   //retransmit
				case TRANSMIT_SUCCESS:    
														  tcpc_status.PDstate_SRC=PE_SRC_Wait_Identity;
														  start_VDMResponse_Timer(GTIMER_SEL3, T_VDM_SENDER_RESPONSE , PD_SRC_VDMResponse_Timeout_act); //timer_out value=27ms
														  break;  
			 case TRANSMIT_ERROR:   flag_src_send_PD_msg =1;                                                               	   break;   //retransmit
			 default         :                                                                                                 break;   //
		}
}
/*
The NoResponseTimer is used by the Policy Engine in a Source to determine that its Port Partner is not responding 
after a Hard Reset.  When the NoResponseTimer times out, the Policy Engine Shall issue up to nHardResetCount 
additional Hard Resets before determining that the Port Partner is non-responsive to USB Power Delivery messaging. 

However it is not necessary in our code 
*/

void  PE_SRC_Discovery_act()
{
	   if(tcpc_status.PD_Connected==1)
		 {
					tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;
					flag_src_send_PD_msg=1;
		 }
		 else// previous no pd 
		 {
				 if(tcpc_status.capscounter<CAPSCOUNT_MAX)
				 {
						 tcpc_status.capscounter++;
						 //Start_SourceCapablityTimer(GTIMER_SEL3, T_TYPEC_SEND_SOURCE_CAP , PD_Advertise_SRC_Capablity);
					   tcpc_status.PDstate_SRC=PE_SRC_Send_Capablities;
				 }
				 else
				 {
						 Stop_SourceCapablityTimer(GTIMER_SEL3);
						 pd.enable_pd_receive(EN_HARD_RESET);	//disable pd message except hard reset
							if(tcpc_status.CC1_PD ==1)
								 pd.set_cc_role(NO_DRP_MODE,RP_1P5A,Open_CC,RP_CC);		//CC2VCON 
							else
								 pd.set_cc_role(NO_DRP_MODE,RP_1P5A,RP_CC,Open_CC);	
						 tcpc_status.PDstate_SRC=PE_SRC_Disable; 
				 }
		 }
}

void PE_SRC_Send_Capablities_act()
{
	 if(flag_src_send_PD_msg==1)
   {   
	   PD_Advertise_SRC_Capablity();
	   flag_src_send_PD_msg=0;
   }
   switch(pd.PD_Transmit_Status_Get())//Read would clear this status
   {
	   case TRANSMIT_FAIL:		 
			                       tcpc_status.PDstate_SRC=PE_SRC_Discovery;
							               break; 	             //delay then transmit
	   case TRANSMIT_DISCARD:	  flag_src_send_PD_msg =1;																	   
							                break;	 //retransmit
	   case TRANSMIT_SUCCESS:	  
														Stop_SourceCapablityTimer(GTIMER_SEL3);
														tcpc_status.capscounter=0;
														tcpc_status.hardresetcounter=0;
														Start_SenderResponseTimer(GTIMER_SEL3,T_SEND_RESPONSE,PD_SRC_Send_Hard_Reset);    
														tcpc_status.PDstate_SRC=PE_SRC_Wait_Request;
		                        tcpc_status.PD_Connected=1;  //about connection not power 
														break;
	   case TRANSMIT_ERROR: 	flag_src_send_PD_msg =1;								 break;   //retransmit
	   default		   :																								 break;  
   }
}


void  PE_SRC_Wait_Request_act()
{	
	   PD_MSG_t *message_rx;
     if(pd.is_new_msg_received())
		{
			 pd.clear_new_msg_received();
			 message_rx=pd.PD_Msg_Get();
		   Stop_SenderResponseTimer(GTIMER_SEL3);	
			if((message_rx->header.number_Data_Objects==1)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REQUEST))
			{
				tcpc_status.PDstate_SRC=PE_SRC_Negotiate_Capability;
			}
			else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
			{
				tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset_Receive;
				flag_src_send_PD_msg=1;
			}
			else //protocol error
			{
				 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;
				 flag_src_send_PD_msg=1;
			}
		}
}



bool PD_SRC_Process_Request()
{
	request_t	request_info;
	PD_MSG_t*	message_rx;
	source_cap_t* my_source_capa;
	uint32_t rdo_position;
	uint32_t is_request_valid=0;
  uint8_t  supply_type_supported_max;
	
	message_rx=pd.PD_Msg_Get();
	my_source_capa= PD_Get_Source_Capa();
	request_info.w=message_rx->data[0];
	rdo_position=request_info.fixed.object_position;
	my_source_capa+=rdo_position-1;
	
	if(tcpc_status.pd_rev==REV_3)
		 supply_type_supported_max=SUPPLY_TYPE_SUPPORTED_MAX;
	else
		 supply_type_supported_max=SUPPLY_TYPE_SUPPORTED_MAX-SUPPLY_PPS_SUPPORTED_NUM;
	
	if(message_rx->header.spec_Rev<=REV_2)
		 pd.set_pd_rev(REV_2);
	else
		pd.set_pd_rev(REV_3);	
	
	if(rdo_position<=supply_type_supported_max)
	{
			if(((my_source_capa->w)&(SUPPLY_TYPE_MASK))==FIXED_SUPPLY_TYPE)
			{
				tcpc_status.PPS_Mode=0;
				DEBUG_INFO("\r\nfixed request num:%d;cur:%dmA",rdo_position,request_info.fixed.operating_current_10mA*10);
				if(my_source_capa->fixed.current_max_10ma>=request_info.fixed.operating_current_10mA)
				{
					  is_request_valid=1;//send accept
			      tcpc_status.request_voltage=my_source_capa->fixed.voltage_50mv*50;
					  tcpc_status.request_current=my_source_capa->fixed.current_max_10ma*10;
				}
				else
				{
					  is_request_valid=0;
				}
			}
			else if((my_source_capa->w&(SUPPLY_TYPE_MASK))==APDO_SUPPLY_TYPE)
			{
				 
				 if((my_source_capa->pps.current_max_50ma>=request_info.pps.operating_current_50ma)&&(my_source_capa->pps.maximum_voltage_100mv*5>=(request_info.pps.voltage_20mv))
					&&(my_source_capa->pps.minimum_voltage_100mv*5<=(request_info.pps.voltage_20mv)))
				 {
					  is_request_valid=1;//send accept
					  tcpc_status.PPS_Mode=1;
					  time_s=0;
					  tcpc_status.request_voltage_history=tcpc_status.request_voltage;
					  tcpc_status.request_voltage=request_info.pps.voltage_20mv*20;
					  tcpc_status.request_current=request_info.pps.operating_current_50ma*50;
					 DEBUG_INFO("\r\npps request:%d;cur:%d mV %d mA",rdo_position,request_info.pps.voltage_20mv*20,request_info.pps.operating_current_50ma*50);
				 }
				 else
				 {
					    is_request_valid=0;
					    DEBUG_INFO("\r\n my src cap:%d mA;%d~%d mV",my_source_capa->pps.current_max_50ma*50,my_source_capa->pps.maximum_voltage_100mv*100,my_source_capa->pps.minimum_voltage_100mv*100);
					    DEBUG_INFO("\r\n unvalid pps request:%d;cur:%d mV %d mA",rdo_position,request_info.pps.voltage_20mv*20,request_info.pps.operating_current_50ma*50);
					 		
				 }
			}
			if(rdo_position==1)
				tcpc_status.above5V=0;
			else 
				tcpc_status.above5V=1;
	}
	else //send reject and then send hard reset
	{
		is_request_valid=0;
	  DEBUG_INFO("\r\nTx:reject");
	}
	
	flag_src_send_PD_msg=1;
	if(is_request_valid==1)
	{  
		 tcpc_status.PDstate_SRC = PE_SRC_Send_Accept;
		 return  TRUE;
	}
	else 
	{   
		  tcpc_status.PDstate_SRC = PE_SRC_Send_Reject;
		  return FALSE;
	}
}



void PE_SRC_Send_Accept_act(void)		
{
		PE_SRC_Send_Control_Msg_act(ACCEPT,//send control messageg type
	                                PE_SRC_Transition_Supply,NOT_SEND_NEW,//success path
	                                PE_SRC_Soft_Reset,SEND_NEW);//fail path
    if(pd.is_new_msg_received())
		{
				 pd.clear_new_msg_received();
				 //pd.PD_Msg_Get();
         PD_SRC_Send_Hard_Reset();//protocol error during power transitioning-->hard reset
		}
		
}


void PE_SRC_Transition_Supply_act(void)
{
	  if(tcpc_status.PPS_Mode==0)
	 {
		 delay_ms(20);//��ʱ��ѹ
		 pd_pwr.Output_Voltage_Set(tcpc_status.request_voltage);
		 pd_pwr.Output_Current_Set(tcpc_status.request_current+500);
		 Start_SourceCapablityTimer(GTIMER_SEL3, T_POWER_TRANSITION , PD_SRC_Send_PS_RDY);	//��ʱ300ms����PS-RDY 
	 }
	 else 
	 {   
			pd_pwr.Output_Voltage_Set(tcpc_status.request_voltage);//��ʱ��ѹ 
			pd_pwr.Output_Current_Set(tcpc_status.request_current);
			if(tcpc_status.request_voltage_history-tcpc_status.request_voltage>V_PPS_SMALL_STEP || tcpc_status.request_voltage-tcpc_status.request_voltage_history>V_PPS_SMALL_STEP )
						Start_SourceCapablityTimer(GTIMER_SEL3,T_PPSSRC_TRANS_LARGE,PD_SRC_Send_PS_RDY);	//��ʱ300ms����PS-RDY 
			else
						Start_SourceCapablityTimer(GTIMER_SEL3,T_PPSSRC_TRANS_SMALL,PD_SRC_Send_PS_RDY);	  //��ʱ25ms����PS-RDY 
	 }	
	 tcpc_status.PDstate_SRC = PE_SRC_Send_Psrdy;//ʵ�ʷ���ʱ���� Timer3��ʱ��������
}


void PE_SRC_Send_Psrdy_act()
{
	
	    if(flag_src_send_PD_msg==1)
			{	
				pd.send_ctrl_msg(PS_RDY,TYPE_SOP);
				flag_src_send_PD_msg=0;
			}
			switch(pd.PD_Transmit_Status_Get())//Read would clear this status
			{
				case TRANSMIT_FAIL: 	     PD_SRC_Send_Hard_Reset();
										               break;  // send hard reset
				case TRANSMIT_DISCARD:	   flag_src_send_PD_msg =1; 																	
											              break;	 //retransmit
				case TRANSMIT_SUCCESS:    
										 	            tcpc_status.PDstate_SRC=PE_SRC_Ready;
				                         						if(tcpc_status.CC1_PD ==1)
																							 pd.set_cc_role(NO_DRP_MODE,RP_1P5A,Open_CC,RP_CC);		//CC2VCON 
																						else
																							 pd.set_cc_role(NO_DRP_MODE,RP_1P5A,RP_CC,Open_CC);	
#ifdef DEBUG_BIST_TEST_DATA
				DEBUG_INFO("\r\n[1]:send soft reset");
				DEBUG_INFO("\r\n[2]:send hard reset");
				DEBUG_INFO("\r\n[3]:send pr_swap");
				DEBUG_INFO("\r\n[4]:send vcon swap");
				DEBUG_INFO("\r\n[5]:send pps status");
				DEBUG_INFO("\r\n[6]:BIST_AA_SOP");
				DEBUG_INFO("\r\n[7]:BIST_55_SOP");
				DEBUG_INFO("\r\n[8]:BIST_FF_SOP");
				DEBUG_INFO("\r\n[9]:BIST_SOP'_AA");
				DEBUG_INFO("\r\n[A]:BIST_SOP''_55");
				DEBUG_INFO("\r\n[B]:BIST_FF SOP'_Debug and SOP get sink cap");
				DEBUG_INFO("\r\n[C]:BIST_00_SOP''_Debug");
				DEBUG_INFO("\r\n[D]:BIST_SEND_BIST_CARRY2");
				DEBUG_INFO("\r\n  Please send test num:");
				
#endif				
											            break;	 
				case TRANSMIT_ERROR:	   flag_src_send_PD_msg =1; 																  break;   //retransmit
				default 		:																								 break;   //
			} 
}
									
void PE_SRC_Send_Reject_act(void)
{
		 if(flag_src_send_PD_msg==1)
	 {   
		 pd.send_ctrl_msg(REJECT,TYPE_SOP);
		 flag_src_send_PD_msg=0;
	 }
	 switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	 {
		case	TRANSMIT_FAIL:		 PD_SRC_Send_Hard_Reset();
														 break;  // send hard reset
		case TRANSMIT_DISCARD:	 flag_src_send_PD_msg =1;																   
														 break;   //retransmit
		case TRANSMIT_SUCCESS:	 
														 PD_SRC_Send_Hard_Reset();
														 break;   //retransmit
		case TRANSMIT_ERROR:		 flag_src_send_PD_msg =1;																 break;   //retransmit
		default		  : 																							                     break;	 //
	 }
	 if(pd.is_new_msg_received())
	{
			 pd.clear_new_msg_received();
			 PD_SRC_Send_Hard_Reset();//protocol error during power transitioning-->hard reset
	}
}


void PE_SRC_Ready_act()
{
		PD_MSG_t * message_rx;
	  uint8_t msg_num_data;
	  uint8_t msg_extended;
	  uint8_t msg_type;
		if(pd.is_new_msg_received())
		{
				pd.clear_new_msg_received();
				message_rx=pd.PD_Msg_Get();
				msg_num_data=message_rx->header.number_Data_Objects;
				msg_extended=message_rx->header.extended;
				msg_type=message_rx->header.message_Type;

					if((msg_extended==0)&&(msg_num_data==1)&&(msg_type==REQUEST))
					{
						 tcpc_status.PDstate_SRC=PE_SRC_Negotiate_Capability;
					}
					else if((msg_num_data==0)&&(msg_type==SOFT_RESET)&&(msg_extended==0))
					{
						 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset_Receive;
						 flag_src_send_PD_msg=1;
					}
					else if((msg_num_data==0)&&(msg_type==PR_SWAP)&&(msg_extended==0))
					{
							tcpc_status.PDstate_SRC=PE_SRC_PRSWAP;
					}
					else if((msg_num_data==0)&&(msg_type==VCONN_SWAP)&&(msg_extended==0))
				  {
							 #ifdef VCONN_EN
									 tcpc_status.PDstate_SRC=PE_SRC_VCONN_SWAP;
							 #else
									 tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;    
									 flag_src_send_PD_msg=1;
							 #endif
				  }
					else
						PD_Process_Protocol_Error();
					

		}
		
		
		if(debug_src_test_num==1)//debug send soft reset
		{
				debug_src_test_num=0;
				tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	
				flag_src_send_PD_msg=1;
		}
		else if(debug_src_test_num==2)//debug send hard reset
		{
				debug_src_test_num=0;
				PD_SRC_Send_Hard_Reset();
		}
		else if(debug_src_test_num==3)//debug send prswap
		{
				debug_src_test_num=0;
			  tcpc_status.PDstate_SRC=PE_SRC_PRSWAP;	
				flag_src_send_PD_msg=1;
			  fsm_pd_src_prswap=FSM_PD_SRC_PRSWAP_SEND_SWAP;
		}
		else if(debug_src_test_num==4)//debug send vcon swap
		{
				debug_src_test_num=0;
			  tcpc_status.PDstate_SRC=PE_SRC_VCONN_SWAP;	
				flag_src_send_PD_msg=1;
			  fsm_pd_src_vcon_swap=FSM_PD_SRC_VCON_SWAP_SEND_SWAP;
		}
		else if(debug_src_test_num==5)//debug send pps status
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_Send_PPS_Status;
		}
#ifdef DEBUG_BIST_TEST_DATA
		else if(debug_src_test_num==6)//debug send AA
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_SEND_AA_SOP;
		}
		else if(debug_src_test_num==7)//debug send 55
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_SEND_55_SOP;
		}
		else if(debug_src_test_num==8)//debug send 00
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_SEND_00FF_SOP;
		}
		else if(debug_src_test_num==9)//debug send AA sop'
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_AA_SOP_P;
		}
		else if(debug_src_test_num==10)//debug send 55  sop''
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_55_SOP_PP;
		}
		else if(debug_src_test_num==11)//debug send FF sop_p_debug abd sop get_sink_cap
		{
				debug_src_test_num=13;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_FF_SOP_P_DEBUG;
		}
		else if(debug_src_test_num==12)//debug send 00 sop_debug"
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_00_SOP_PP_DEBUG;
		}	
		else if(debug_src_test_num==13)//debug send get sink cap
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_SEND_SORT_MSG;
		}	
		else if(debug_src_test_num==14)//debug send BIST Carrier mode2
		{
				debug_src_test_num=0;
				flag_src_send_PD_msg=1;
			  tcpc_status.PDstate_SRC=PE_SRC_BIST_SEND_BIST_CARRY2;
		}	

#endif
		
		if((tcpc_status.PPS_Mode==1)&&(time_s>T_PPS_TIME_OUT))// PPS_timeout
		{
				PD_SRC_Send_Hard_Reset();
				tcpc_status.PPS_Mode=0;
		}
}

/*  Action @PSRDY state ------------------------------------------------------ start */

void PD_Send_Sink_Cap_act()
{
		 PE_SRC_Send_Complx_Msg_act(PD_Advertise_SNK_Capablity,
	                              PE_SRC_Ready,NOT_SEND_NEW,//success path
	                              PE_SRC_Soft_Reset,SEND_NEW);//fail path
	
		/*	if(flag_src_send_PD_msg==1)
			{	
				PD_Advertise_SNK_Capablity();
				flag_src_send_PD_msg=0;
			}
			switch(pd.PD_Transmit_Status_Get())//Read would clear this status
			{
				case TRANSMIT_FAIL: 	   tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	
																 flag_src_send_PD_msg=1;
																 break;   
				case TRANSMIT_DISCARD:	   flag_src_send_PD_msg =1; 																	
																	break;	 //retransmit
				case TRANSMIT_SUCCESS:    
																	tcpc_status.PDstate_SRC=PE_SRC_Ready;
																	break;
				case TRANSMIT_ERROR:	   flag_src_send_PD_msg =1; 																  break;   //retransmit
				default 		:																							                          break;   //
			}
			*/
}




void PE_SRC_Send_NotSupported_act(void)
{
	
	PE_SRC_Send_Control_Msg_act(((tcpc_status.pd_rev==REV_3)?NOT_SUPPORTED:REJECT),//send control messageg type
	                                 PE_SRC_Ready,NOT_SEND_NEW,//success path
	                                 PE_SRC_Soft_Reset,SEND_NEW);//fail path
	
		/*		 if(flag_src_send_PD_msg==1)
				 {	
						if(tcpc_status.pd_rev==REV_3)
							pd.send_ctrl_msg(NOT_SUPPORTED);
						else 
						pd.send_ctrl_msg(REJECT);//Not supported is invalid in PD2.0
						flag_src_send_PD_msg=0;
				 }
				switch(pd.PD_Transmit_Status_Get())//Read would clear this status
				{
						case TRANSMIT_FAIL:   tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	 
																	flag_src_send_PD_msg=1;
																	break;  
						case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
																			 break;   //retransmit
						case TRANSMIT_SUCCESS:    
																			 tcpc_status.PDstate_SRC=PE_SRC_Ready;
																			 break;   
					 case TRANSMIT_ERROR:         flag_src_send_PD_msg =1;                                                               	  break;   //retransmit
					 default         :                                                                                                       break;   //
				}
		*/				
}


void PE_SRC_BIST_Carried_Mode_act(void)
{
	  PE_SRC_Send_Complx_Msg_act(pd.send_bist_carry_mode,
	                              PE_SRC_Ready,NOT_SEND_NEW,//success path
	                              PE_SRC_Ready,NOT_SEND_NEW);//fail path
		/*	if(flag_src_send_PD_msg==1)
	   {   
		   BIST_Carrier_Mode_Send();
		   flag_src_send_PD_msg=0;
	   }
	   switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	   {
		   case TRANSMIT_FAIL:	 
				                      tcpc_status.PDstate_SRC=PE_SRC_Ready;
									                       break;
		   case TRANSMIT_DISCARD:	  flag_src_send_PD_msg =1;																	   
									             break;	//retransmit
		   case TRANSMIT_SUCCESS:	  
									   tcpc_status.PDstate_SRC=PE_SRC_Ready;
									   break;
		   case TRANSMIT_ERROR: 	  flag_src_send_PD_msg =1;																	 break;   //retransmit
		   default		   :																								 break;   //
	   }
		 */
}


void PE_SRC_Send_PPS_Status_Act()
{
	   PE_SRC_Send_Complx_Msg_act(Pd_SRC_Advertise_PPS_Status,
	                              PE_SRC_Ready,NOT_SEND_NEW,//success path
	                             PE_SRC_Soft_Reset,SEND_NEW);//fail path

}


										
void PE_SRC_Soft_Reset_Receive_act(void)
{
	     PE_SRC_Send_Control_Msg_act(ACCEPT,//send control messageg type
	                                 PE_SRC_Send_Capablities,SEND_NEW,//success path
	                                PE_SRC_Soft_Reset,SEND_NEW);//fail path

}
											 



void PD_Advertise_SRC_Capablity(void)
{
		PD_MSG_t t_mes;
		uint8_t i;
	  uint8_t supply_type_supported_max;
	
		source_cap_t* my_source_capa=PD_Get_Source_Capa();
		t_mes.header.extended=0;
		t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
	  //tcpc_status.msg_id_tx[TYPE_SOP]=0;
	  if(tcpc_status.pd_rev==REV_3)
			 t_mes.header.number_Data_Objects=SUPPLY_TYPE_SUPPORTED_MAX;
		else
			 t_mes.header.number_Data_Objects=SUPPLY_TYPE_SUPPORTED_MAX-SUPPLY_PPS_SUPPORTED_NUM;
		
		supply_type_supported_max=t_mes.header.number_Data_Objects;
		t_mes.header.message_Type=SOURCE_CAPABILITIES;
		t_mes.header.number_Data_Objects=SUPPLY_TYPE_SUPPORTED_MAX;
		t_mes.header.port_Data_Role=tcpc_status.data_role;
		t_mes.header.port_Power_Role=tcpc_status.power_role;
		t_mes.header.spec_Rev=tcpc_status.pd_rev;
	//	tcpc_status.messageID=(tcpc_status.messageID+1)%8;
		tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
		for(i=0;i<supply_type_supported_max;i++)
		{
			t_mes.data[i]=my_source_capa[i].w;
		}
		t_mes.Sop_Type=TYPE_SOP;
		pd.send_msg(t_mes);
	//	DEBUG_INFO("\r\nTX:src_cap:");
}

void  Pd_SRC_Advertise_PPS_Status(void)
{
		pps_status_t pps_status;
	  extended_header_t extended_header;
	  PD_MSG_t t_mes;
	  uint16_t voltage_now;
	 	t_mes.header.extended=1;
		t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
		t_mes.header.message_Type=PPS_STATUS;
		t_mes.header.number_Data_Objects=2;
		t_mes.header.port_Data_Role=tcpc_status.data_role;
		t_mes.header.port_Power_Role=tcpc_status.power_role;
		t_mes.header.spec_Rev=tcpc_status.pd_rev;
	  
	  extended_header.bit.chunked=0;
	  extended_header.bit.chunk_number=0;
	  extended_header.bit.data_size=4;
	  extended_header.bit.request_chunk=0;
	  extended_header.bit.reserve=0;
	  voltage_now=pd_pwr.Read_ADC(VBUS_REG);
	  if(voltage_now*10<tcpc_status.request_current*9)
	     pps_status.bit.OMF=1;
		else
			 pps_status.bit.OMF=0;
		DEBUG_INFO("\r\nVbus:%d",voltage_now);
	  pps_status.bit.output_current=0xFF;
	  pps_status.bit.output_voltage=0xFFFF;
		pps_status.bit.PTF=0;
		pps_status.bit.reserve=0;
		pps_status.bit.reserve1=0;
	  
		t_mes.data[0]=extended_header.w|((pps_status.w&(0XFFFF))<<16);
		t_mes.data[1]=(pps_status.w>>16);
		t_mes.Sop_Type=TYPE_SOP;
		pd.send_msg(t_mes);
}

#ifdef DEBUG_BIST_TEST_DATA

void PD_Send_BIST_TEST_DATA(SPEC_REV_TYPE_e rev, uint8_t byte,SOP_TYPE_e msg_type)
{
	
	  PD_MSG_t t_mes;
	  uint16_t i;
	 	t_mes.header.extended=0;
		t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
		t_mes.header.message_Type=BIST;
		t_mes.header.number_Data_Objects=7;
		t_mes.header.port_Data_Role=tcpc_status.data_role;
		t_mes.header.port_Power_Role=tcpc_status.power_role;
		tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
		t_mes.header.spec_Rev=rev;
	  t_mes.data[0]=(1<<31);
    for(i=1;i<7;i++)
			t_mes.data[i]=(byte)|(byte<<8)|(byte<<16)|(byte<<24);

		t_mes.Sop_Type=msg_type;
		pd.send_msg(t_mes);
}
void PD_BIST_TEST_act(SPEC_REV_TYPE_e rev, uint8_t byte,SOP_TYPE_e msg_type)
{
			if(flag_src_send_PD_msg==1)
		{	
			PD_Send_BIST_TEST_DATA(rev,byte,msg_type);
			flag_src_send_PD_msg=0;
		}
		switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
			case TRANSMIT_FAIL: 	   tcpc_status.PDstate_SRC=PE_SRC_Ready;	
															 flag_src_send_PD_msg=0;
											         break; //delay then transmit
			case TRANSMIT_DISCARD:	 flag_src_send_PD_msg =1; 																	
									             break;	   //retransmit
			case TRANSMIT_SUCCESS:    
										    	     tcpc_status.PDstate_SRC=PE_SRC_Ready;  
			                         flag_src_send_PD_msg=0;
										           break;	
			case TRANSMIT_ERROR:	   flag_src_send_PD_msg =1; 						  break;   //retransmit
			default 		:																							      break;   //
		}
}

#endif






void PD_Process_Protocol_Error()
{
	 PD_MSG_t* message_rx=pd.PD_Msg_Get();
	
	 uint8_t msg_data_num=message_rx->header.number_Data_Objects;
	 uint8_t msg_extended=message_rx->header.extended;
	 uint8_t msg_type=message_rx->header.message_Type;
	
	if(msg_data_num==0 && msg_extended==0)
	{
		 SRC_Process_Control_Msg[msg_type]();
		/*switch(message_rx->header.message_Type)
			{
				 case    UNRECGNITED: 			      break;//do nothing
				 case    GOODCRC :				        break;//do noth                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              ing 
				 case    GOTOMIN:                                                             ;
				 case    ACCEPT:                                                              ;
				 case    REJECT:                 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;   flag_src_send_PD_msg=1;               break;
				 case    PING:                    break;//do nothing 
				 case    PS_RDY :                tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;   flag_src_send_PD_msg=1; break;// do nothing 
				 case    GET_SOURCE_CAP :        tcpc_status.PDstate_SRC=PE_SRC_Send_Capablities;    flag_src_send_PD_msg=1;    break;
				 case    GET_SINK_CAP :          tcpc_status.PDstate_SRC=PE_SRC_Get_Sink_Cap;        flag_src_send_PD_msg=1;    break;
				 case    DR_SWAP :               tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;   flag_src_send_PD_msg=1;    break;//reject would also be a goog choice according to PD protocol
				 case    PR_SWAP:                break;// process outside
				 case    VCONN_SWAP :            tcpc_status.PDstate_SRC=PE_SRC_VCONN_SWAP;     break;    //2ѡ1, ACCEPT or Not supported
				 case    WAIT :                  tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;   flag_src_send_PD_msg=1;       break;    
				 case    SOFT_RESET :             break;    //process outside 
				 case    DATA_RESET :             tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;
				 case    DATA_RESET_COMPLETE:      tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;  break;    
				 case    NOT_SUPPORTED :          break;    //do nothing
				 case    GET_SOURCE_CAP_EXTEND :     tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break; //sending src_cap_extened is also correct response; 
				 case    GET_STATUS_PD :          tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;//
				 case    FR_SWAP :                tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;
				 case    GET_PPS_STATUS :          tcpc_status.PDstate_SRC=PE_SRC_Send_PPS_Status;   flag_src_send_PD_msg=1;   break;//UUT must send PPS satus to tester;
				 case    GET_COUNTRY_CODES :      tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;// The check fails if the UUT does not send either Not_Supported or Country_Codes message. 
				 case    GET_SINK_CAP_EXTENED :     break; //Send  not support
			 	 default:                         tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;  flag_src_send_PD_msg=1;   break;
			}
			*/
	}
	else if(msg_extended==0 && (msg_data_num!=0))
	{
			switch(msg_type)
			{
					case	   SOURCE_CAPABILITIES:		        break;//do nothing 
					case	   REQUEST:            		        break;//process outside 
					case	   BIST:                  		
															if( message_rx->data[0]>>28==BIST_CARRIER_MODE && (tcpc_status.above5V==0) )
																tcpc_status.PDstate_SRC=PE_SRC_BIST_Carried_Mode;
															else if(message_rx->data[0]>>28==BIST_TEST_DATA)
															{ 
																pd.enable_bist_data_mode(1);//   BIST_TEST_DATA_Mode_Enter();
																tcpc_status.PDstate_SRC=PE_SRC_BIST_TEST_DATA;
															}
															else 
															{
																tcpc_status.PDstate_SRC=PE_SRC_Send_NotSupported;
																flag_src_send_PD_msg=1;
															}
															break;
					case	   SINK_CAPABILITIES:  		        break;  //do nothing
					case	   BATTERY_STATUS:     		        break;  //do nothing 
					case	   ALERT:              		PD_SRC_Send_Not_Supported();       
																					break;  
					case	   GET_COUNTRY_INFO:   	  PD_SRC_Send_Not_Supported();         break;
					case	   ENTER_USB:          		PD_SRC_Send_Not_Supported();         break;
					case	   VENDER_DEFINED:     		PD_SRC_Send_Not_Supported();         break;// do nothing shoould send not supported
					default:                                break;
			}
	}
	else 
	{
			switch(msg_type)
			{
			case		  SOURCE_CAPABILITIES_EXTENDED:                                                                               break;
			case	    STATUS:                                                                                                     break;
			case	    GET_BATTERY_CAP:           PD_SRC_Send_Not_Supported();        break;
			case	    GET_BATTERY_STATUS :       PD_SRC_Send_Not_Supported();        break;
			case	    BATTERY_CAPABILITIES :                                                                                      break;
			case	    GET_MANUFACTURER_INFO :    PD_SRC_Send_Not_Supported();        break;
			case	    MANUFACTURER_INFO :        PD_SRC_Send_Not_Supported();        break;
			case	    SECURITY_REQUES:           PD_SRC_Send_Not_Supported();        break;
			case	    SECURITY_RESPONSE :        PD_SRC_Send_Not_Supported();        break;
			case	    FIRMWARE_UPDATE_REQUEST :                                                                                   break;
			case	    FIRMWARE_UPDATE_RESPONSE:                                                                                   break;
			case	    PPS_STATUS :                                                                                                break;
			case	    COUNTRY_INFO :                                                                                              break;
			case	    COUNTRY_CODES :                 break;
			case	    SINK_CAPABILITIES_EXTENDED :    break;
			case		  31:                      
                                          if(((message_rx->data[0])&(0X00008000))!=0)	//chunked message
																					   Start_Chunking_Notsupported_Timer(GTIMER_SEL4,T_CHUNKING_NOTSUPPORTED,PD_SRC_Send_Not_Supported);   
                                          else //unchunked message    																					 
				                                    Start_Chunking_Notsupported_Timer(GTIMER_SEL4,T_UNCHUNKING_NOTSUPPORTED,PD_SRC_Send_Not_Supported);  
																					      break;
			default :                                 break;
			}
	}
	
}



/* Cable Communication VDM Start */

void PD_SRC_VDMResponse_Timeout_act()
{
	tcpc_status.PDstate_SRC=PE_SRC_Startup;  
}

void PE_SRC_Wait_Identity_act()//to 
{
	   PD_MSG_t *message_rx;
	   vdm_header_t vdm_header;
		 uint8_t max_voltage;
	   uint8_t max_current;
	  
    
	   cable_t* cable=PD_Get_Cable_Capa();
	   id_header_vdo_t id_header;

	
    if(pd.is_new_msg_received())
		{
				pd.clear_new_msg_received();
				message_rx=pd.PD_Msg_Get();
				if((message_rx->header.number_Data_Objects>4)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==VENDER_DEFINED))
				{ 
					 vdm_header.w=message_rx->data[0];
					 id_header.w=message_rx->data[1];
					 if( (vdm_header.bit.command==DISCOVER_IDENTITY)  &&(vdm_header.bit.command_type==ACK)&&(id_header.bit.product_type==PASSIVE_CABLE || (id_header.bit.product_type==ACTIVE_CABLE)) )
					 {
								 Stop_SenderResponseTimer(GTIMER_SEL3);	
								 cable->w=message_rx->data[4];
						     max_voltage=cable->passive_cable_t.maximum_vbus;
						     max_current=cable->passive_cable_t.current_capablity;
						     DEBUG_INFO("\r\nreceive passive cable capa,maxV:%d,maxI:%s",20+max_voltage*10,passcable_cur_str[max_current]);//0,4 Reserve,1:3A,2:5A
								 tcpc_status.PDstate_SRC=PE_SRC_Startup;
					 }
				}
				else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
				{
					
					Stop_SenderResponseTimer(GTIMER_SEL3);
					tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset_Receive;
					flag_src_send_PD_msg=1;
				}	
		}
}

void 	PD_SRC_Discover_Identity_Send()
{
	 PD_MSG_t t_mes;
	 t_mes.Sop_Type=TYPE_SOPP;//communicate with cable
	 t_mes.header.extended=0;
	 t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOPP];
	 t_mes.header.message_Type=VENDER_DEFINED;
	 t_mes.header.number_Data_Objects=1;
	 t_mes.header.port_Data_Role=0;
	 t_mes.header.port_Power_Role=0;
	 t_mes.header.spec_Rev=tcpc_status.pd_rev;

	 t_mes.data[0]=0xFF008001;//Discover Identity,request; or  FF00A001
	 pd.send_msg(t_mes);
}

/* Cable Communication VDM finished */

void PD_SRC_Send_Hard_Reset()
{
	pd.send_hard_reset();
  DEBUG_INFO("\r\nTx->HARD_RESET");
	tcpc_status.hardresetcounter++;
	tcpc_status.PDstate_SRC=PE_SRC_Hard_Reset;
	flag_src_hard_reset_fsm_start_en=1;
}

void  PE_SRC_Send_Control_Msg_act(CONTROL_MESSAGE_TYPE_e message_type,PD_src_state_e success_path,uint8_t success_send_flag,PD_src_state_e fail_path,uint8_t fail_send_flag)
{
	 	if(flag_src_send_PD_msg==1)
		{	
			pd.send_ctrl_msg(message_type,TYPE_SOP);
			flag_src_send_PD_msg=0;
		}
		switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
			case TRANSMIT_FAIL: 	   tcpc_status.PDstate_SRC=fail_path;	
															 flag_src_send_PD_msg=fail_send_flag;
											         break; //delay then transmit
			case TRANSMIT_DISCARD:	 flag_src_send_PD_msg =1; 																	
									             break;	   //retransmit
			case TRANSMIT_SUCCESS:    
										    	     tcpc_status.PDstate_SRC=success_path;  
			                         flag_src_send_PD_msg=success_send_flag;
										           break;	
			case TRANSMIT_ERROR:	   flag_src_send_PD_msg =1; 						  break;   //retransmit
			default 		:																							      break;   //
		}
}

void  PE_SRC_Send_Complx_Msg_act(fun_t* action,PD_src_state_e success_path,uint8_t success_send_flag,PD_src_state_e fail_path,uint8_t fail_send_flag)
{
	 	if(flag_src_send_PD_msg==1)
		{	
			action();
			flag_src_send_PD_msg=0;
		}
		switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
			case TRANSMIT_FAIL: 	   tcpc_status.PDstate_SRC=fail_path;	
															 flag_src_send_PD_msg=fail_send_flag;
											         break; //delay then transmit
			case TRANSMIT_DISCARD:	 flag_src_send_PD_msg =1; 																	
									             break;	   //retransmit
			case TRANSMIT_SUCCESS:    
										    	     tcpc_status.PDstate_SRC=success_path;  
			                         flag_src_send_PD_msg=success_send_flag;
										           break;	
			case TRANSMIT_ERROR:	   flag_src_send_PD_msg =1; 						  break;   //retransmit
			default 		:																							      break;   //
		}
}



//********************************************************************************************************************************
//
//   FSM_PD_SRC_SR_SEND_SOFTRRESET=0,			
//	 FSM_PD_SRC_SR_WAIT_ACCEPT,
//	 FSM_PD_SRC_SR_RESTART
//   type_name: pd_src_softreset_adjust_fsm_e;
//********************************************************************************************************************************

static fsm_result_e (*_fsm_pd_src_soft_reset_handle[])(pd_src_soft_reset_adjust_fsm_e *)=
{
	  _fsm_pd_sr_send_soft_reset,
    _fsm_pd_sr_wait_accept,
    _fsm_pd_sr_restart
};

void PD_SRC_Soft_Reset_fsm()
{
	fsm_result_e result;
	do
	{
	   result=_fsm_pd_src_soft_reset_handle[fsm_pd_src_soft_reset_fsm](&fsm_pd_src_soft_reset_fsm); 
	}while(result==FSM_SWITCH_IMM);
}



static fsm_result_e  _fsm_pd_sr_send_soft_reset(pd_src_soft_reset_adjust_fsm_e * pd_src_soft_reset_fsm)
{
	 if(flag_src_send_PD_msg==1)
	{	
	    //PD_Protocol_Reset(); //Clear message ID
			tcpc_status.msg_id_tx[TYPE_SOP]=0;
	    tcpc_status.msg_id_rx[TYPE_SOP]=0;
		  tcpc_status.msg_id_rx_init[TYPE_SOP]=0;
		  pd.send_ctrl_msg(SOFT_RESET,TYPE_SOP);
		  flag_src_send_PD_msg=0;
  }
    else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	{
	    case TRANSMIT_FAIL:         
                                 PD_SRC_Send_Hard_Reset();				
		                             return FSM_WAITING;//�漰״̬�л�
	    case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
							  break;   //retransmit
		  case TRANSMIT_SUCCESS:    
																	*pd_src_soft_reset_fsm=FSM_PD_SRC_SR_WAIT_ACCEPT; 
																	Start_SenderResponseTimer(GTIMER_SEL3,25,PD_SRC_Send_Hard_Reset);	
																	return FSM_SWITCH_IMM;
		   case TRANSMIT_ERROR:       flag_src_send_PD_msg =1;                                                               	  break;   //retransmit
		   default  :                                                                                                        break;   //
	}
	return FSM_WAITING;
}


static fsm_result_e _fsm_pd_sr_wait_accept(pd_src_soft_reset_adjust_fsm_e* pd_src_soft_reset_fsm)
{
	  PD_MSG_t* message_rx;
    if(pd.is_new_msg_received())
		{
				pd.clear_new_msg_received();
				message_rx=pd.PD_Msg_Get();
				if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
				{
					 Stop_SenderResponseTimer(GTIMER_SEL3);	
					 *pd_src_soft_reset_fsm=FSM_PD_SRC_SR_RESTART;
					 return FSM_SWITCH_IMM;
				}
		}
    return FSM_WAITING;
}


static fsm_result_e _fsm_pd_sr_restart(pd_src_soft_reset_adjust_fsm_e* pd_src_soft_reset_fsm)
{
	 if(flag_src_soft_reset_sysclk_start==FALSE)
   {
   		Start_Sysclock();
		  flag_src_soft_reset_sysclk_start=TRUE;
   }
   else
   {
	    if(Get_system_tick()>10)
	   	{
	   	   //reset protocal layer
	   	    Stop_Sysclock();
	    	  *pd_src_soft_reset_fsm=FSM_PD_SRC_SR_SEND_SOFTRRESET;
		      tcpc_status.PDstate_SRC=PE_SRC_Send_Capablities;
				  flag_src_send_PD_msg=1;//added @20201021
				  DEBUG_INFO("\r\n*Enter PE_SRC_Send_Capablities");
	        return FSM_WAITING;
	   	}
  }
   return FSM_WAITING;
}

/* *******************************************************************************************************************************

typedef enum
{
	FSM_ADJUST_TO_VSAFE5V=0,
	FSM_ADJUST_TO_VSAFE0V,
	FSM_WAIT_SRC_RECOVER,
	FSM_RESTORE_TO_VSAFE5V,
	FSM_RESTART_COMMUNICATION
}pd_hard_reset_power_adjust_fsm_e;

********************************************************************************************************************************/

static fsm_result_e (*_fsm_pd_hard_reset_power_adjust_handle[])(pd_hard_reset_power_adjust_fsm_e *)=
{
	  _fsm_pd_hardreset_power_adjust_to_Vsafe5V,
    _fsm_pd_hardreset_power_adjust_to_Vsafe0V,
    _fsm_pd_hardreset_power_wait_srcrecover,
    _fsm_pd_hardreset_power_recover_to_Vsafe5V,
    _fsm_pd_hardreset_restart_to_communication
};


void SRC_Hard_Reset_fsm()
{
	fsm_result_e result;
	do
	{
	   result=_fsm_pd_hard_reset_power_adjust_handle[fsm_pd_src_hard_reset](&fsm_pd_src_hard_reset); 
	}while (result==FSM_SWITCH_IMM);
}


static fsm_result_e _fsm_pd_hardreset_power_adjust_to_Vsafe5V(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm )
{
  
   if(flag_src_hard_reset_fsm_start_en==0)
	     return FSM_WAITING;
   if(flag_src_hard_reset_sysclk_start==FALSE)
   {
   		Start_Sysclock();
		  flag_src_hard_reset_sysclk_start=TRUE;
   }
   else
   	{
	   if(Get_system_tick()>T_PS_HARDRESET) //delay 25ms then Set voltage to 5V
	   	{
			  pd_pwr.Output_Voltage_Set(5100);
				pd_pwr.Output_Current_Set(3000);
				pd.vcon_off();// remove Rp in VCONN
	   	  Stop_Sysclock();
				flag_src_hard_reset_sysclk_start=FALSE;
		    flag_src_hard_reset_fsm_start_en=0;
	   	  *pd_hard_reset_power_adjust_fsm=FSM_ADJUST_TO_VSAFE0V;
			   DEBUG_INFO("\r\n*Adjust_to_Vsafe5V");
	      return FSM_SWITCH_IMM;
	   	}
   	}
   return FSM_WAITING; 
}

static fsm_result_e _fsm_pd_hardreset_power_adjust_to_Vsafe0V(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm)
{
  //Set to 0V, start discharge 
   if(flag_src_hard_reset_sysclk_start==FALSE)
   {
   		Start_Sysclock();
		 
		  flag_src_hard_reset_sysclk_start=TRUE;
		 	 DEBUG_INFO("\r\n*Enter adjust_to_Vsafe0V");
   }
   else
   	{
	   if(Get_system_tick()>10)//delay 10ms
	   	{
	   	   pd_pwr.OTG_Off();				//�����ŵ�
	   	   Stop_Sysclock();
				 flag_src_hard_reset_sysclk_start=FALSE;
	   	   *pd_hard_reset_power_adjust_fsm=FSM_WAIT_SRC_RECOVER;
	       return FSM_SWITCH_IMM;
	   	}
   	}
  return FSM_WAITING;
	
}

static fsm_result_e _fsm_pd_hardreset_power_wait_srcrecover(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm)
{
	  if(flag_src_hard_reset_sysclk_start==FALSE)
	   {
			  Start_Sysclock();
			  flag_src_hard_reset_sysclk_start=TRUE;
			  DEBUG_INFO("\r\n*Enterwait_srcrecover");
	   }
	   else
		{  
			// DEBUG_INFO("\r\n %d",Get_system_tick());
		   if(Get_system_tick()>T_SRC_RECOVER_SPR)//T_SRC_RECOVER=660ms
			{
			  Stop_Sysclock();
				flag_src_hard_reset_sysclk_start=FALSE;
			  *pd_hard_reset_power_adjust_fsm=FSM_RESTORE_TO_VSAFE5V;
			  return FSM_SWITCH_IMM;
			}
		}
	   return FSM_WAITING;
}

static fsm_result_e _fsm_pd_hardreset_power_recover_to_Vsafe5V(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm)
{
	#ifdef VCONN_EN
	 if(tcpc_status.ra==1)
	     pd.vcon_on();  //based on USB TYPEC 2.0,VCONN may be applied prior to the application of VBUS;but no later than 2 ms
	     pd.enable_pd_receive(EN_SOP|EN_SOP1|EN_HARD_RESET);
  #else
	 	   pd.enable_pd_receive(EN_SOP|EN_HARD_RESET);
	#endif
	  pd_pwr.OTG_On();
	 *pd_hard_reset_power_adjust_fsm=FSM_RESTART_COMMUNICATION;
	  DEBUG_INFO("\r\n*Enter recover_to_Vsafe5V");
	 return FSM_SWITCH_IMM;
}



static fsm_result_e _fsm_pd_hardreset_restart_to_communication(pd_hard_reset_power_adjust_fsm_e *pd_hard_reset_power_adjust_fsm)
{
    //Setup Rp
    *pd_hard_reset_power_adjust_fsm=FSM_ADJUST_TO_VSAFE5V;
   // tcpc_status.PDstate_SRC=PE_SRC_Default;  // after hard reset process commpleed, restart vconn 
	  tcpc_status.PDstate_SRC=PE_SRC_Startup;   //  changed @20221126
	  DEBUG_INFO("\r\n enter PE_SRC_Startup");
	  return FSM_WAITING;
}


void PD_SRC_Init(void)
{
	
	tcpc_status.capscounter=0;
  tcpc_status.PD_Connected=0;
	
	pd.set_power_role(SRC_ROLE);//hard reset or por would reset the port data role to default:SRC/DFT
	pd.set_data_role(DFP);
	pd.set_pd_rev(REV_3);
	
	//protocol reset
  PD_Protocol_Reset();
	
	tcpc_status.PPS_Mode=0;
	tcpc_status.cc_vbus_update_en=1;
	tcpc_status.hard_resrt_on=0;
	
	#ifdef VCONN_EN
	pd.enable_pd_receive(EN_SOP|EN_SOP1|EN_HARD_RESET);
  #else
	pd.enable_pd_receive(EN_SOP|EN_HARD_RESET);
	#endif
	
	Stop_Sysclock();
	fsm_pd_src_soft_reset_fsm=FSM_PD_SRC_SR_SEND_SOFTRRESET;
	fsm_pd_src_prswap=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
	flag_src_pr_swap_sysclk_start=FALSE;	
}




static fsm_result_e (*_fsm_pd_src_prswap_handle[])(pd_src_prswap_fsm_e*)=
{
	_fsm_pd_sr_prswap_evaluate_swap,
	_fsm_pd_sr_prswap_accept_swap,
	_fsm_pd_sr_prswap_transtion_to_off,
	_fsm_pd_sr_prswap_assert_rd,
	_fsm_pd_sr_prswap_wait_source_on,
	_fsm_pd_sr_prswap_send_swap,
	_fsm_pd_sr_prswap_wait_response
};

void   SRC_PRSWAP_fsm(void)
{
	 fsm_result_e result;
	 do
	 {
		 result=_fsm_pd_src_prswap_handle[fsm_pd_src_prswap](&fsm_pd_src_prswap); 
	 }while (result==FSM_SWITCH_IMM);	
}


/*******************************************************************************************************************************************************

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


	static fsm_result_e  _fsm_pd_sr_prswap_evaluate_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	static fsm_result_e  _fsm_pd_sr_prswap_accept_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	static fsm_result_e  _fsm_pd_sr_prswap_transtion_to_off(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	static fsm_result_e  _fsm_pd_sr_prswap_assert_rd(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	static fsm_result_e  _fsm_pd_sr_prswap_wait_source_on(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	static fsm_result_e  _fsm_pd_sr_prswap_send_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	static fsm_result_e  _fsm_pd_sr_prswap_wait_response(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm);
	
***********************************************************************************************************************************************************	
*/


static fsm_result_e  _fsm_pd_sr_prswap_evaluate_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{
	  *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_ACCEPT_SWAP;
	  flag_src_send_PD_msg=1;
	  return FSM_WAITING;
}


static fsm_result_e  _fsm_pd_sr_prswap_accept_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{
	 if(flag_src_send_PD_msg==1)
	{	
		pd.send_ctrl_msg(ACCEPT,TYPE_SOP);
		flag_src_send_PD_msg=0;
  }
  else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	{
	    case TRANSMIT_FAIL:         
                                 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	 //�˴�Э�鲢û����ȷ˵��
		                             flag_src_send_PD_msg=1;	
                                 	*pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;//RESET state
		                             return FSM_WAITING;//�漰״̬�л�
	    case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
							                   break;   //retransmit
		  case TRANSMIT_SUCCESS:    
																*pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_TRANSITION_TO_OFF; 
																return FSM_SWITCH_IMM;
		   case TRANSMIT_ERROR:     flag_src_send_PD_msg =1;                        break;   //retransmit
		   default  :                                                               break;   //
	}
	return FSM_WAITING;
}


static fsm_result_e  _fsm_pd_sr_prswap_transtion_to_off(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{
	  PD_MSG_t *message_rx;
	   //otg off,set Rd
	  if(flag_src_pr_swap_sysclk_start==FALSE)
		 {
				Start_Sysclock();
				flag_src_pr_swap_sysclk_start=TRUE;
		 }
    else
   	{
			 if(Get_system_tick()>T_SRC_TRANSITION)//delay 30ms
				{
					 Stop_Sysclock();
					 flag_src_pr_swap_sysclk_start=FALSE;
					 pd_pwr.OTG_Off();
					 tcpc_status.cc_vbus_update_en=0;
					 pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);	
					 tcpc_status.power_role=SNK_ROLE;
					 *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_ASSERT_RD;
					 flag_src_send_PD_msg=1;
					 return FSM_SWITCH_IMM;
				}
   	}
		if(pd.is_new_msg_received())
		{
				pd.clear_new_msg_received();
				message_rx=pd.PD_Msg_Get();
				if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))//error recovery
				{
						Stop_Sysclock();
						flag_src_pr_swap_sysclk_start=FALSE;
						tcpc_status.error_recovery=1;
						*pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
						flag_src_send_PD_msg=1;
						return FSM_WAITING;
				}
		}
    return FSM_WAITING;
}


static fsm_result_e  _fsm_pd_sr_prswap_assert_rd(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{
	   //send psrdy
	 if(flag_src_send_PD_msg==1)
	{	
		pd.send_ctrl_msg(PS_RDY,TYPE_SOP);
		flag_src_send_PD_msg=0;
  }
  else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	{
	    case TRANSMIT_FAIL:         
                                 tcpc_status.error_recovery=1;  //Э��Ҫ��
		                             return FSM_WAITING;//�漰״̬�л�
	    case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
							                   break;   //retransmit
		  case TRANSMIT_SUCCESS:    
																*pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_WAIT_SOURCE_ON; 
																return FSM_SWITCH_IMM;
		  case TRANSMIT_ERROR:     flag_src_send_PD_msg =1;                  break;   //retransmit
		  default  :                                                         break;   //
	}
	return FSM_WAITING;
}


static fsm_result_e  _fsm_pd_sr_prswap_wait_source_on(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{  
   	  PD_MSG_t *message_rx;
	    if(flag_src_pr_swap_sysclk_start==FALSE)
		  {
				Start_Sysclock();
				flag_src_pr_swap_sysclk_start=TRUE;
		  }
			else
			{
				 if(Get_system_tick()>T_NEW_SRC)//time out 
					{
						 Stop_Sysclock();
						 flag_src_pr_swap_sysclk_start=FALSE;
						 tcpc_status.error_recovery=1;
						 *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
						 flag_src_send_PD_msg=1;
						 return FSM_WAITING;
					}
			}
			if(pd.is_new_msg_received())
			{
					pd.clear_new_msg_received();
					message_rx=pd.PD_Msg_Get();
					if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==PS_RDY))
					{
						 *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
						 Stop_Sysclock();
						 flag_src_pr_swap_sysclk_start=FALSE;
						 tcpc_status.flag_src_to_snk=1;
						 return FSM_SWITCH_IMM;			 
					}
					else //receive other message ->protocol error
					{
						  Stop_Sysclock();
						  flag_src_pr_swap_sysclk_start=FALSE;
						  tcpc_status.error_recovery=1;
						  *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
						  flag_src_send_PD_msg=1;
						  return FSM_WAITING;
					}
			}
    return FSM_WAITING;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
}

static fsm_result_e  _fsm_pd_sr_prswap_send_swap(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{
		  if(flag_src_send_PD_msg==1)
			{	
				pd.send_ctrl_msg(PR_SWAP,TYPE_SOP);
				flag_src_send_PD_msg=0;
			}
			else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
			{
					case TRANSMIT_FAIL:         
																	   tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	 //�˴�Э�鲢û����ȷ˵��
		                                 flag_src_send_PD_msg=1;	
					                           break;
					case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
																		 break;   //retransmit
					case TRANSMIT_SUCCESS:    
																		*pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_WAIT_RESPONSE; 
																		return FSM_SWITCH_IMM;
					 case TRANSMIT_ERROR:     flag_src_send_PD_msg =1;                  break;   //retransmit
					 default  :                                                         break;  
			}
			return FSM_WAITING;
}


static fsm_result_e  _fsm_pd_sr_prswap_wait_response(pd_src_prswap_fsm_e * pd_src_pr_swap_fsm)
{
		  PD_MSG_t *message_rx;
	    if(flag_src_pr_swap_sysclk_start==FALSE)
		  {
				Start_Sysclock();
				flag_src_pr_swap_sysclk_start=TRUE;
				
		  }
			else
			{
				 if(Get_system_tick()>T_SEND_RESPONSE)//27ms time out
					{
						 Stop_Sysclock();
						 flag_src_pr_swap_sysclk_start=FALSE;
						 tcpc_status.PDstate_SRC=PE_SRC_Ready;//according to PD protocol Page 611, it is not regared as an error
						 *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
						 return FSM_WAITING;
					}
			}
			if(pd.is_new_msg_received())
			{
					pd.clear_new_msg_received();
					message_rx=pd.PD_Msg_Get();
					if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
					{
						 *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_TRANSITION_TO_OFF;
						 Stop_Sysclock();
						 flag_src_pr_swap_sysclk_start=FALSE;
						 return FSM_SWITCH_IMM;			 
					}
					else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REJECT ||(message_rx->header.message_Type==WAIT) ))
					{
						 tcpc_status.PDstate_SRC=PE_SRC_Ready;//according to PD protocol Page 611, it is not regared as an error
						 Stop_Sysclock();
						 flag_src_pr_swap_sysclk_start=FALSE;
						 *pd_src_pr_swap_fsm=FSM_PD_SRC_PRSWAP_EVALUAT_SWAP;
						 return FSM_WAITING;			 
					}	
			}
    return FSM_WAITING;                                       
}

//Note:
/* 
1.After a successful Power Role Swap the Port Partners Shall reset their respective Protocol Layers (equivalent to a Soft 
  Reset): resetting their MessageIDCounter, RetryCounter and Protocol Layer state machines before attempting to 
  establish an Explicit Contract.  At this point the Source Shall also reset its CapsCounter. 

2.The DFP (Host), UFP (Device) roles and VCONN Source Shall remain unchanged during the Power Role Swap process. 

*/



/* *******************************************************************************************************************************  


                                                 VCON_SWAP   Process

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

*///***********************************************************************************************************************************


static fsm_result_e (*_fsm_pd_src_vcon_swap_handle[])(pd_src_vcon_swap_fsm_e*)=
{
	_fsm_pd_src_vcon_swap_evaluat_swap,
	_fsm_pd_src_vcon_swap_accept_swap,
	_fsm_pd_src_vcon_swap_wait_for_vconn,
	_fsm_pd_src_vcon_swap_turn_on_vconn,
	_fsm_pd_src_vcon_swap_turn_off_vconn,
	_fsm_pd_src_vcon_swap_send_ps_rdy,
	_fsm_pd_src_vcon_swap_send_swap,
	_fsm_pd_src_vcon_swap_force_vconn,
	_fsm_pd_src_vcon_swap_wait_response
};

void SRC_VCONN_Swap_fsm(void)
{
	 fsm_result_e result;
	 do
	 {
	   result=_fsm_pd_src_vcon_swap_handle[fsm_pd_src_vcon_swap](&fsm_pd_src_vcon_swap); 
	 }while (result==FSM_SWITCH_IMM);	
}

static fsm_result_e _fsm_pd_src_vcon_swap_evaluat_swap(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)
{
	     *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_ACCEPT_SWAP;
	     flag_src_send_PD_msg=1;
	     return FSM_WAITING; 
}

static fsm_result_e _fsm_pd_src_vcon_swap_accept_swap(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//����ACCEPT 
{
	 if(flag_src_send_PD_msg==1)
	{	
		pd.send_ctrl_msg(ACCEPT,TYPE_SOP);
		flag_src_send_PD_msg=0;
  }
  else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	{
	    case TRANSMIT_FAIL:         
                                 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	 //�˴�Э�鲢û����ȷ˵��
		                             flag_src_send_PD_msg=1;	
                                 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;		
		                             return FSM_WAITING;//�漰״̬�л�
	    case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
							                   break;   //retransmit
		  case TRANSMIT_SUCCESS:   
                                if(pd_status.VCONN_ON==1)	//VCONN_SRC would be off after snk vcon on	
																{
																	 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_WAIT_FOR_VCONN; 
																}
																else
																{ 
																	 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_TURN_ON_VCONN;
																	 pd.vcon_on();
																}
																return FSM_SWITCH_IMM;
		   case TRANSMIT_ERROR:     flag_src_send_PD_msg =1;                        break;   //retransmit
		   default  :                                                               break;   //
	}
	return FSM_WAITING;
}

static fsm_result_e _fsm_pd_src_vcon_swap_wait_for_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//�ȴ��Է���VCONN������PSRDY
{
	    PD_MSG_t *message_rx;
	    if(flag_src_vcon_swap_sysclk_start==FALSE)
		  {
				Start_Sysclock();
				flag_src_vcon_swap_sysclk_start=TRUE;
		  }
			else
			{
				 if(Get_system_tick()>T_VCONN_SOURCE_TIMEOUT)//time out then send hard reset
					{
						 Stop_Sysclock();
						 flag_src_vcon_swap_sysclk_start=FALSE;
						 PD_SRC_Send_Hard_Reset();
						 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;
						 return FSM_WAITING;
					}
			}
			if(pd.is_new_msg_received())
			{
					pd.clear_new_msg_received();
					message_rx=pd.PD_Msg_Get();
					if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==PS_RDY))
					{
						 Stop_Sysclock();
						 flag_src_vcon_swap_sysclk_start=FALSE;
						 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_TURN_OFF_VCONN;
						 return FSM_SWITCH_IMM;			 
					}
					else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))//�Զ��壬�յ�Softreset ������hard reset ����
					{
						 Stop_Sysclock();
						 flag_src_vcon_swap_sysclk_start=FALSE;
						 PD_SRC_Send_Hard_Reset();
						 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;
						 return FSM_WAITING;
					}
			}
    return FSM_WAITING;   
}

static fsm_result_e _fsm_pd_src_vcon_swap_turn_on_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//��VCONN
{
	   if(flag_src_vcon_swap_sysclk_start==FALSE)
		  {
				Start_Sysclock();
				flag_src_vcon_swap_sysclk_start=TRUE;
		  }
			else
			{
				 if(Get_system_tick()>20)//10ms��VCONN��Ȼû�д� then send hard reset
					{
						 Stop_Sysclock();
						 flag_src_vcon_swap_sysclk_start=FALSE;
						 PD_SRC_Send_Hard_Reset();
						 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;
						 return FSM_WAITING;
					}
			}
			if(pd_status.vconn_present==1)
			{ 
				flag_src_send_PD_msg=1;
				* pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_SEND_PS_RDY;
				Stop_Sysclock();
				flag_src_vcon_swap_sysclk_start=FALSE;
			}
			return FSM_WAITING;
}

static fsm_result_e _fsm_pd_src_vcon_swap_turn_off_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//�ر�VCONN 
{
	   pd.vcon_off();
	   *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;
	   tcpc_status.PDstate_SRC=PE_SRC_Ready;
	   return FSM_WAITING;
}

static fsm_result_e _fsm_pd_src_vcon_swap_send_ps_rdy(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//����PSRDY
{
		 if(flag_src_send_PD_msg==1)
		{	
			pd.send_ctrl_msg(PS_RDY,TYPE_SOP);
			flag_src_send_PD_msg=0;
		}
		else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
				case TRANSMIT_FAIL:         
																	 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	 //�˴�Э�鲢û����ȷ˵��
																	 flag_src_send_PD_msg=1;	
																	 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;		
																	 return FSM_WAITING;//�漰״̬�л�
				case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
																	 break;   //retransmit
				case TRANSMIT_SUCCESS:   
                                  tcpc_status.PDstate_SRC=PE_SRC_Ready;
																	*pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;
																	return  FSM_WAITING;
				 case TRANSMIT_ERROR:     flag_src_send_PD_msg =1;                        break;   //retransmit
				 default  :                                                               break;   //
		}
		return FSM_WAITING;
}


static fsm_result_e _fsm_pd_src_vcon_swap_send_swap(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//����VCONN SWAP 
{
			 if(flag_src_send_PD_msg==1)
			{	
				pd.send_ctrl_msg(VCONN_SWAP,TYPE_SOP);
				flag_src_send_PD_msg=0;
			}
			else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
			{
					case TRANSMIT_FAIL:         
																		 tcpc_status.PDstate_SRC=PE_SRC_Soft_Reset;	 //�˴�Э�鲢û����ȷ˵��
																		 flag_src_send_PD_msg=1;	
																		 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;		
																		 return FSM_WAITING;//�漰״̬�л�
					case TRANSMIT_DISCARD:     flag_src_send_PD_msg =1;     	                                                            
																		 break;   //retransmit
					case TRANSMIT_SUCCESS:   
																		*pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_WAIT_RESPONSE; 
																		return FSM_SWITCH_IMM;
					 case TRANSMIT_ERROR:     flag_src_send_PD_msg =1;                        break;   //retransmit
					 default  :                                                               break;   //
			}
			return FSM_WAITING;
}

static fsm_result_e _fsm_pd_src_vcon_swap_force_vconn(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)
{	   
	     pd.vcon_on();
	     *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;		
	     tcpc_status.PDstate_SRC=PE_SRC_Ready;//�Է������ҷ���VCONN_SWAP�ظ�not_supported,�ҷ�ǿ�ƴ�VCONN
	     return FSM_WAITING; 
}


static fsm_result_e _fsm_pd_src_vcon_swap_wait_response(pd_src_vcon_swap_fsm_e* pd_src_vcon_swap_fsm)//����ACCEPT 
{
	    PD_MSG_t *message_rx;
	    if(flag_src_vcon_swap_sysclk_start==FALSE)
		  {
				Start_Sysclock();
				flag_src_vcon_swap_sysclk_start=TRUE;
		  }
			else
			{
				 if(Get_system_tick()>T_SEND_RESPONSE)//time out 27ms
					{
						 Stop_Sysclock();
						 flag_src_vcon_swap_sysclk_start=FALSE;
					   tcpc_status.PDstate_SRC=PE_SRC_Ready;	 //
						 //flag_src_send_PD_msg=1;	
						 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;
						 return FSM_WAITING;
					}
			}
			if(pd.is_new_msg_received())
			{
					pd.clear_new_msg_received();
					message_rx=pd.PD_Msg_Get();
					if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
					{
						  Stop_Sysclock();
						  flag_src_vcon_swap_sysclk_start=FALSE;
						  if(pd_status.VCONN_ON==1)	//VCONN_SRC would be off after snk vcon on	
							{
									*pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_WAIT_FOR_VCONN; 
							}
							else
							{ 
								 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_TURN_ON_VCONN;
								 pd.vcon_on();
							}
						 return FSM_SWITCH_IMM;			 
					}
					else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REJECT))
					{
						  flag_src_vcon_swap_sysclk_start=FALSE;
						  tcpc_status.PDstate_SRC=PE_SRC_Ready;
              *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;		
						  return FSM_WAITING;			 
					}
					else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==NOT_SUPPORTED))
					{
						  flag_src_vcon_swap_sysclk_start=FALSE;
						  if(pd_status.VCONN_ON==1)	//VCONN_SRC still on
							{
									 tcpc_status.PDstate_SRC=PE_SRC_Ready;
                   *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_EVALUAT_SWAP;		
							}
							else
							{ 
								 *pd_src_vcon_swap_fsm=FSM_PD_SRC_VCON_SWAP_FORCE_VCONN;
								 //to do 
							}
						 return FSM_SWITCH_IMM;			 
					}
					
			}
    return FSM_WAITING;     
}


