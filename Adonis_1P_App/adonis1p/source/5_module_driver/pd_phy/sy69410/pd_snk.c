#include   "pd_snk.h"
#include   "pd_cfg.h"
#include   "pd_com.h"
#include   "pd_timer.h"
#include   "pd_global.h"
#include   "pd_general.h"


extern pd_status_t  pd_status;
extern pd_phy_t  pd;
static  uint8_t                              flag_snk_send_PD_msg;
static  uint8_t                              local_request_num;
static  uint32_t                             local_request_current;
static  uint32_t                             local_request_voltage;
static  pd_snk_soft_reset_adjust_fsm_e       fsm_pd_snk_soft_reset;
uint16_t last_state;
#define SEND_NEW_MSG        1
#define NOT_SEND_NEW_MSG    0
typedef void fun_t(void);

static void (*SNK_Process_Control_Msg[])()
	={		 		                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  

		 NULL,		    // case    UNRECGNITED: 			    
		 NULL,			 // case    GOODCRC :				      
		 PD_SNK_Send_Soft_Reset,	 // case    GOTOMIN:           
		 PD_SNK_Send_Soft_Reset,	 // case    ACCEPT:                 
		 PD_SNK_Send_Soft_Reset,		 // case    REJECT:                 
		 NULL,	 // case    PING:                   
		 NULL, 	 // case    PS_RDY :                
		 PD_SNK_Send_Not_Supported, 		 // case    GET_SOURCE_CAP :       
		 PD_SNK_Send_SINK_CAP, // case    GET_SINK_CAP :           
		 PD_SNK_Send_Not_Supported,	 // case    DR_SWAP :              
		 PD_SNK_Send_Not_Supported,	 // case    PR_SWAP :              
		 PD_SNK_Send_Not_Supported,	 // case    VCONN_SWAP :             
		 PD_SNK_Send_Soft_Reset,		 // case    WAIT :                 
		 PD_SNK_Soft_Send_Accept_to_Soft_Reset,		 // case    SOFT_RESET :            
		 PD_SNK_Send_Not_Supported,		 // case    DATA_RESET :          
		 NULL, // case    DATA_RESET_COMPLETE:     break;    
		 NULL,	 // case    NOT_SUPPORTED :          break;    //do nothing
		 PD_SNK_Send_Not_Supported,		 // case    GET_SOURCE_CAP_EXTEND :    
		 PD_SNK_Send_Not_Supported,		 // case    GET_STATUS_PD :          tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;      break;
		 PD_SNK_Send_Not_Supported,	 // case    FR_SWAP :                tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;     break;
		 PD_SNK_Send_Not_Supported,	 // case    GET_PPS_STATUS :           tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;     break;
		 PD_SNK_Send_Not_Supported,		 // case    GET_COUNTRY_CODES :      tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;    break;
		 PD_SNK_Send_Not_Supported,		 // case    GET_SINK_CAP_EXTENED :   tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;    flag_snk_send_PD_msg=1;    break; //Send 
		 PD_SNK_Send_Not_Supported, 	// default:                          tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;    flag_snk_send_PD_msg=1;    break;
		 NULL,
		 NULL
	 };

void PD_Protocol_Reset(void)
{
	  uint8_t i;

	  for(i=0;i<5;i++)
	  {
			 tcpc_status.msg_id_tx[i]=0;
			 tcpc_status.msg_id_rx[i]=0;
			 tcpc_status.msg_id_rx_init[i]=0;
		}
}
	 
void PD_Advertise_SNK_Capablity(void)
{
	uint8_t i;
  PD_MSG_t t_mes;
	sink_cap_t* my_sink_capa=PD_Get_Sink_Capa();
	t_mes.header.extended=0;
	t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
	t_mes.header.message_Type=SINK_CAPABILITIES;
	t_mes.header.number_Data_Objects=REQUEST_TYPE_SUPPORTED_MAX;
	t_mes.header.port_Data_Role=tcpc_status.data_role;
	t_mes.header.port_Power_Role=tcpc_status.power_role;
	t_mes.header.spec_Rev=tcpc_status.pd_rev;
		tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
	for(i=0;i<SUPPLY_TYPE_SUPPORTED_MAX;i++)
	{
	  t_mes.data[i]=my_sink_capa[i].w;
	}
	t_mes.Sop_Type=TYPE_SOP;
	pd.send_msg(t_mes);
}

void PD_SNK_Send_Soft_Reset(void)
{
			flag_snk_send_PD_msg=1;																								
			tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
}

void  PD_SNK_Send_SOURCE_CAP(void)
{
	   tcpc_status.PDstate_SNK=PE_SNK_Give_Source_Cap;  
	   flag_snk_send_PD_msg=1; 
}


void  PD_SNK_Send_SINK_CAP(void)
{
	   tcpc_status.PDstate_SNK=PE_SNK_Give_Sink_Cap;  
	   flag_snk_send_PD_msg=1; 
}

void  PD_SNK_Send_Not_Supported(void)
{
	   flag_snk_send_PD_msg=1;	
	   tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  
}


void  PD_SNK_Soft_Send_Accept_to_Soft_Reset(void)// SOFTRESET
{
	  tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
	  flag_snk_send_PD_msg=1;	
}

/*******************************************************************************************************************************

                                                        SNK Processs

********************************************************************************************************************************/  
void  Vendor_PD_SNK(void)
{
	 PD_MSG_t* message_rx;
		if(last_state!=tcpc_status.PDstate_SNK)
			{	 
				  DEBUG_INFO("\r\n %s->%s",sink_state_str[last_state],sink_state_str[tcpc_status.PDstate_SNK]); 
				 last_state=tcpc_status.PDstate_SNK;
			}
	if(pd.is_hard_reset_received())// Receive Hard_Reset
	{
		pd.enable_bist_data_mode(0);
		
		Gtimer_Stop(Timer_Chunking_Notsupported);
		Gtimer_Stop(Timer_SENDER_RESPONSE);
		Gtimer_Stop(Timer_PSTransition);
		Gtimer_Stop(Timer_SinkWaitCap);
		Gtimer_Stop(Timer_Hardreset);
		/*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
		tcpc_status.hard_resrt_on=1;
		tcpc_status.hardresetcounter=0;
		tcpc_status.PDstate_SNK=PE_SNK_Hard_Reset_Wait;    //receive hard reset-> wait src_recovery
	}
	else switch(tcpc_status.PDstate_SNK)
    { 

			
		  case    PE_SNK_Default:                
				                                       PD_SNK_Init(); //Reset Protocol Layer
			                                         tcpc_status.PDstate_SNK=PE_SNK_Startup;//new start 
				                                       break;
   		case    PE_SNK_Startup:                  //start after hard reset or prswap     
																							 PD_Protocol_Reset();
			                                         // pd.set_pd_rev(REV_3);
			                                         pd.set_pd_rev(REV_3);
																							 tcpc_status.PPS_Mode=0;
			                                         tcpc_status.EPR_Mode=0;
			                                         tcpc_status.Explict_Contract=0;
										                        	#ifdef REQUEST_MAX_20V 
		                                        	Expected_Request_Voltage=20000;//inportant
			                                        #else
			                                        Expected_Request_Voltage=9000;//inportant
			                                        #endif
			                                         
	                                             tcpc_status.PDstate_SNK=PE_SNK_Discovery;
	                                             break;
    	 case 	 PE_SNK_Discovery:  
																							 if(pd.is_vbus_ok()) //Vbus is present
																							 {
																									tcpc_status.PDstate_SNK=PE_SNK_Wait_for_Capablities;
																									Start_SinkWaitCapTimer(Timer_SinkWaitCap, T_TYPEC_SINKWAIT_CAP, PD_SNK_Send_Hard_Reset); //source_cap has to be received within 465ms
																								 /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
																							 }
																							 break;
		   case   PE_SNK_Wait_for_Capablities:  
																							if(pd.is_new_msg_received())
																							{
																									pd.clear_new_msg_received();
																									tcpc_status.PD_Connected=1;
																									message_rx=pd.PD_Msg_Get();
																									if((message_rx->header.number_Data_Objects>0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOURCE_CAPABILITIES))
																									{
																										 Stop_SinkWaitCapTimer(Timer_SinkWaitCap);	
																										 /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
																										 tcpc_status.EPR_Mode=0;
																										 if(message_rx->header.spec_Rev==0x02)
																										 {
																											 pd.set_pd_rev(REV_3);
																										 }
																										 else
																										 {
																											 pd.set_pd_rev(REV_2);
																										 }
																										tcpc_status.PDstate_SNK=PE_SNK_Evaluate_Capablity; 
																										DEBUG_INFO("\r\nRx:SRC_CAP");
																									}
																									else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
																									{
																										Stop_SinkWaitCapTimer(Timer_SinkWaitCap);	
																										flag_snk_send_PD_msg=1;																								
																										tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
																									}			
																							}
																							break;
      case   PE_SNK_Evaluate_Capablity:     
																							tcpc_status.hardresetcounter=0;//receive source cap
																							PE_SNK_Evaluate_Capablity_act(); //to get request num and current
																							tcpc_status.PDstate_SNK=PE_SEN_Send_Request;
			                                         //DEBUG_INFO("\r\n\nTx:REQUEST");
																							flag_snk_send_PD_msg=1;
																							break;
		 case   PE_SEN_Send_Request:              PE_SNK_Select_Capability_act();  break;
		 case   PE_SNK_Wait_Accept:               PE_SNK_Wait_Accept_act();        break;
		 case   PE_SNK_Transition_Sink:           PE_SNK_Transition_Sink_act();    break;
		 case   PE_SNK_Ready:                     PE_SNK_Ready_act();              break;
		 case   PE_SNK_Give_Sink_Cap:             PE_SNK_Give_Sink_Cap_act();      break;
		 case   PE_SNK_Hard_Reset:              
		 	                              
		 	                                     PD_SNK_Send_Hard_Reset();
		 case   PE_SNK_Hard_Reset_Wait:       
			                                      //DEBUG_INFO("\r\n PE_SNK_Hard_Reset_Wait");
		 	                                       Start_Hard_Reset_Timer(Timer_Hardreset, T_SNK_WAIT_SRC_RECOVERY, PD_SNK_Jump_to_Default);  //wait time=660ms
		                                        //during hard reset,SPP* message would be disabled by IC
		                                        /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
		 										                                              break;
		 case   PE_SNK_Soft_Reset:              PE_SNK_Soft_Reset_fsm();  break;
		 case   PE_SNK_Transition_to_Default:  
		 	                                      //Reset Local HW
		                                         pd.set_data_role(UFP);
		                                         pd.set_power_role(SNK_ROLE);
		                                         pd.set_pd_rev(REV_3);
											                       tcpc_status.PDstate_SNK=PE_SNK_Startup; 
		                                         tcpc_status.hard_resrt_on=0;
		                                         pd.enable_pd_receive(EN_SOP|EN_HARD_RESET); //after receive hard reset,EN_RECEIVE should be rewritten
		 	                                      break;
		 case PE_SNK_Hard_Reset_Received:       tcpc_status.PDstate_SNK=PE_SNK_Transition_to_Default;      break;//
		 case PE_SNK_Soft_Reset_Receive:        PE_SNK_Soft_Reset_Receive_act();       break; 
		 case PE_SNK_Send_NotSupported:         PE_SNK_Send_NotSupported_act();        break;
		 case PE_SNK_BIST_TEST_DATA:                                                    break;
		 case PE_SNK_BIST_Carried_Mode:         PE_SNK_BIST_Carried_Mode_act();        break;
		 case PE_SNK_Send_GetSRCCAP:            PE_SNK_Send_GetSRCCAP_act();           break;
		 case PE_SNK_Disable:                                                          break;
 		 default:             DEBUG_INFO("\r\n error state default quit ");            break;
    }
}

void  PD_SNK_Init()
{
		tcpc_status.PD_Connected = 0;
		tcpc_status.hard_resrt_on=0;
	  tcpc_status.hardresetcounter=0;
		pd.set_power_role(SNK_ROLE);//hard reset or por would reset the port data role to default:SRC/DFT
		pd.set_data_role(UFP);
		pd.set_pd_rev(REV_3);	   
    PD_Protocol_Reset();
	  tcpc_status.cc_vbus_update_en=1;
		tcpc_status.pd_rev=REV_3;
	  pd.PD_Transmit_Status_Get(); 
	  pd.enable_pd_receive(EN_SOP|EN_HARD_RESET);   
	
		Gtimer_Stop(Timer_Chunking_Notsupported);
		Gtimer_Stop(Timer_SENDER_RESPONSE);
		Gtimer_Stop(Timer_PSTransition);
		Gtimer_Stop(Timer_SinkWaitCap);
		Gtimer_Stop(Timer_Hardreset); 
	
		#ifdef REQUEST_MAX_20V 
		Expected_Request_Voltage=20000;//inportant
		#else
		Expected_Request_Voltage=9000;//inportant
		#endif
		local_request_num=0;
	//  Stop_Sysclock();
    DEBUG_INFO("\r\n SNK Init"); 
}

void PD_SNK_Jump_to_Default()
{	
	 Stop_Hard_Reset_Timer(Timer_Hardreset);
	 /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
	 pd.vcon_off();
	 DEBUG_INFO("\r\n fun:PD_SNK_Jump_to_Default");
	 tcpc_status.PDstate_SNK=PE_SNK_Transition_to_Default;
}

void PE_SNK_Evaluate_Capablity_act()
{
	PD_MSG_t*      message_rx;
	source_cap_t   rx_source_cap_data;
	source_cap_t*  rx_source_cap=&rx_source_cap_data;
	sink_cap_t*    my_sink_cap =PD_Get_Sink_Capa();
	uint16_t  request_line[7];
	uint16_t  request_voltage[7];
	uint16_t  request_current[7];
	uint32_t  request_power[7];
	uint32_t  max_power=0;
	uint8_t i;
	message_rx=pd.PD_Msg_Get();
 if(tcpc_status.pd_rev!=message_rx->header.spec_Rev)
 {
	 if(message_rx->header.spec_Rev==REV_3)
	 { 
		 tcpc_status.pd_rev=REV_3;
	 }
	 else
	 {
		 tcpc_status.pd_rev=REV_2;
	 }
	  PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
 }

			
	for(i=0;i<message_rx->header.number_Data_Objects;i++)
		{   
				rx_source_cap->w=message_rx->data[i];
			  
				if( (((message_rx->data[i])&SUPPLY_TYPE_MASK)==FIXED_SUPPLY_TYPE) &&(rx_source_cap->fixed.voltage_50mv*50<=Expected_Request_Voltage) )
				{
					request_line[i]=1;
					if(rx_source_cap->fixed.current_max_10ma*rx_source_cap->fixed.voltage_50mv<(100*1000000/500))//src提供功率<100W
					{
						     request_current[i]=rx_source_cap->fixed.current_max_10ma;//请求最大电流
					}
					else if(100*1000000/500/rx_source_cap->fixed.voltage_50mv>300)// Src提供功率>100W 并且>3A
					{
								request_current[i]=300;//请求3A
					}
					else
							request_current[i]=100*1000000/500/rx_source_cap->fixed.voltage_50mv;  
					
				  request_voltage[i]=rx_source_cap->fixed.voltage_50mv;
					request_power[i]=request_current[i]*rx_source_cap->fixed.voltage_50mv;
					DEBUG_INFO("\r\nfixed:%d mV,%d mA ",rx_source_cap->fixed.voltage_50mv*50,rx_source_cap->fixed.current_max_10ma*10);
			 }
			else if((((message_rx->data[i])&SUPPLY_TYPE_MASK)==FIXED_SUPPLY_TYPE))
			{
				 request_line[i]=0;
				 DEBUG_INFO("\r\nfixed:%d mV,%d mA ",rx_source_cap->fixed.voltage_50mv*50,rx_source_cap->fixed.current_max_10ma*10);
				 
			}
			else 
			{
				  request_line[i]=0;
					DEBUG_INFO("\r\nPPS:min %d mV,max %d mV,%d mA ",rx_source_cap->pps.minimum_voltage_100mv*100,rx_source_cap->pps.maximum_voltage_100mv*100,rx_source_cap->pps.current_max_50ma*50);
			}

	}
		for(i=0;i<message_rx->header.number_Data_Objects;i++)
		{
				if(request_line[i]==1)
				{
					if(max_power<=request_power[i])
					{
						max_power=request_power[i];
				  	local_request_num=i;
						local_request_voltage=request_voltage[i];
					  local_request_current=request_current[i];
					}
				}
		} 	
}

void PE_SNK_Select_Capability_act()
{
	  if(flag_snk_send_PD_msg==1)
	   {   
		   PD_Send_Request(local_request_num,local_request_current);
			 if(local_request_num==0)
				 tcpc_status.above5V=0;
			 else
				 tcpc_status.above5V=1;
		  flag_snk_send_PD_msg=0;
	   }
	  else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	   {
		   case TRANSMIT_FAIL:	tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
			                      flag_snk_send_PD_msg=1;
			                      DEBUG_INFO("\r\n Tx  Request fail");
								 break;   
		   case TRANSMIT_DISCARD:	  flag_snk_send_PD_msg =1;																	   
								 break;   //retransmit
		   case TRANSMIT_SUCCESS:	  
								 tcpc_status.PDstate_SNK=PE_SNK_Wait_Accept;
			           //DEBUG_INFO("\r\n wait accept");
								 Start_SenderResponseTimer(Timer_SENDER_RESPONSE,T_SEND_RESPONSE, PD_SNK_Send_Soft_Reset);  
			         /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
			 
								 break;  
		   case TRANSMIT_ERROR: 	  flag_snk_send_PD_msg =1;																	 break;   //retransmit
		   default		   :																								                    break;   //
	   }
}


void PD_Send_Request(uint8_t number,uint16_t current)
{
	request_t request_msg;
	PD_MSG_t  t_mes;
	if(tcpc_status.PPS_Mode==1)
	{
		  request_msg.pps.capablity_mismatch=0;
		  request_msg.pps.erp_mode_capable=0;
		  request_msg.pps.giveback_flag=0;
		  request_msg.pps.no_usb_suspend=0;
		  request_msg.pps.object_position=number+1;
		  request_msg.pps.operating_current_50ma=40;
		  request_msg.pps.reserved=0;
		  request_msg.pps.reserved1=0;
		  request_msg.pps.unchunked_extended_message_support=0;
		  request_msg.pps.usb_communications_capable=0;
		  request_msg.pps.voltage_20mv=local_request_voltage*5;
		  time_s=0;
	}
	else
	{
			request_msg.fixed.max_min_current_10ma=current;
			request_msg.fixed.operating_current_10mA=current;
			request_msg.fixed.reserved=0;
			request_msg.fixed.erp_mode_capable=1;
			request_msg.fixed.unchunked_extended_message_support=0;
			request_msg.fixed.no_usb_suspend=0;              
			request_msg.fixed.usb_communications_capable=0;  
			request_msg.fixed.capablity_mismatch=0;         
			request_msg.fixed.giveback_flag=0;             
			request_msg.fixed.object_position=number+1;
	}

		t_mes.header.extended=0;
		t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
		t_mes.header.message_Type=REQUEST;
		t_mes.header.number_Data_Objects=1;
		t_mes.header.port_Data_Role=tcpc_status.data_role;
		t_mes.header.port_Power_Role=SNK_ROLE;
		t_mes.header.spec_Rev=tcpc_status.pd_rev;
		tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
		t_mes.data[0]=request_msg.w;
		t_mes.Sop_Type=TYPE_SOP;
		DEBUG_INFO("\r\n request %d:Current:%d ",number+1,current);
		pd.send_msg(t_mes);		
}

void PE_SNK_Wait_Accept_act()
{
  	PD_MSG_t*  message_rx;
	
    if(pd.is_new_msg_received())
		{
			pd.clear_new_msg_received();
			message_rx=pd.PD_Msg_Get();
			if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
			{
				// /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
				Stop_SenderResponseTimer(Timer_SENDER_RESPONSE);	
				tcpc_status.PDstate_SNK=PE_SNK_Transition_Sink;
//				pd_pwr.Input_Current_Set(0);//receive accept
				//DEBUG_INFO("\r\nRX:accept");
				Start_PSTransitionTimer(Timer_PSTransition, T_PSTransition, PD_SNK_Send_Hard_Reset);//wait soft reset ,else send hard reset
			}
			else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
			{
				Stop_SinkWaitCapTimer(Timer_SinkWaitCap);	
				flag_snk_send_PD_msg=1;
				tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
			}
			else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REJECT ||message_rx->header.message_Type==WAIT )&&(tcpc_status.Explict_Contract==1))
				{ //reveive reject or wait in explict_contract 
					Stop_SinkWaitCapTimer(Timer_SinkWaitCap);	
					tcpc_status.PDstate_SNK=PE_SNK_Ready;
				}
			else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REJECT ||message_rx->header.message_Type==WAIT )&&(tcpc_status.Explict_Contract==0))
				{ //reveive reject or wait in unexplict_contract 
					Stop_SinkWaitCapTimer(Timer_SinkWaitCap);	
					tcpc_status.PDstate_SNK=PE_SNK_Wait_for_Capablities;
					Start_SinkWaitCapTimer(Timer_SinkWaitCap, T_TYPEC_SINKWAIT_CAP, PD_SNK_Send_Hard_Reset); //source_cap has to be received within 465ms
				}
		}
}

void PE_SNK_Transition_Sink_act()
{
		PD_MSG_t*  message_rx;
		if(pd.is_new_msg_received())
		{
				pd.clear_new_msg_received();
				message_rx=pd.PD_Msg_Get();
				if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==PS_RDY))
				{
					//DEBUG_INFO("\r\nRx:PS_RDY");
					
					Stop_PSTransitionTimer(Timer_PSTransition);	
					DEBUG_INFO("\r\nRx:PS_RDY");
					tcpc_status.PDstate_SNK=PE_SNK_Ready;
					if(tcpc_status.PPS_Mode==1)
					  {
//							pd_pwr.Charge_Current_Set(4000);
//					  pd_pwr.Input_Current_Set(local_request_current*50);
						 }
					else 
					   {
	//						 pd_pwr.Input_Current_Set(local_request_current*10);
	//						 pd_pwr.Charge_Current_Set(4000);
						 }
				}
				else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
				{
					Stop_PSTransitionTimer(Timer_PSTransition);	
					flag_snk_send_PD_msg=1;
					tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
					DEBUG_INFO("\r\n:");
				}
				else
				{
					 Stop_PSTransitionTimer(Timer_PSTransition);	
					 PD_SNK_Send_Hard_Reset();
				}
		}
}

void PD_SNK_Send_Hard_Reset()
{
	  if(tcpc_status.hardresetcounter<=N_HARDRESETCPOUNT)//max hard_reset count=2
		{
				 pd.send_hard_reset();
			   DEBUG_INFO("\r\nTX->HARD_RESET");
				 tcpc_status.hard_resrt_on=1;
				 tcpc_status.hardresetcounter++;
				 tcpc_status.PDstate_SNK=PE_SNK_Hard_Reset_Wait;	
    }
		else
		{
			   DEBUG_INFO("\r\n SNK Disable");
			   //Disable Receive
			   pd.enable_pd_receive(EN_HARD_RESET);
			   tcpc_status.PDstate_SNK=PE_SNK_Disable;	
		}
		    
}


void PE_SNK_Send_NotSupported_act()
{
	 if(flag_snk_send_PD_msg==1)
	 {	
		 if(tcpc_status.pd_rev==REV_3)
		    pd.send_ctrl_msg(NOT_SUPPORTED,TYPE_SOP);
		 else
			  pd.send_ctrl_msg(REJECT,TYPE_SOP);
		flag_snk_send_PD_msg=0;
		Stop_Chunking_Notsupported_Timer(Timer_Chunking_Notsupported);/*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
	 }
    switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
				case TRANSMIT_FAIL:   tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
															flag_snk_send_PD_msg=1;
									break;   
				case TRANSMIT_DISCARD:     flag_snk_send_PD_msg =1;     	                                                            
									break;   //retransmit
		  	case TRANSMIT_SUCCESS:    
									tcpc_status.PDstate_SNK=PE_SNK_Ready;
									break;  
			  case TRANSMIT_ERROR:       flag_snk_send_PD_msg =1;                 break;   //retransmit
			  default         :                                                   break;   //
		}
}


void PE_SNK_Ready_act()
{       
	   PD_MSG_t * message_rx;
	   uint8_t   msg_num_data;
	   uint8_t   msg_extended;
	   uint8_t   msg_type;
		if(pd.is_new_msg_received())
		{
				pd.clear_new_msg_received();
				message_rx=pd.PD_Msg_Get();
		    msg_num_data=message_rx->header.number_Data_Objects;
	      msg_extended=message_rx->header.extended;
	      msg_type=message_rx->header.message_Type;
			 
				if((msg_num_data>0)&&(msg_extended==0)&&(msg_type==SOURCE_CAPABILITIES))
				{
					 tcpc_status.PDstate_SNK=PE_SNK_Evaluate_Capablity;//retry Request
				}
				else if((msg_num_data==0)&&(msg_extended==0)&&(msg_type==SOFT_RESET))
				{
					 tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
					 flag_snk_send_PD_msg=1; 
				}
				else
					PD_SNK_Process_Protocol_Error();
		}

}


void PD_SNK_Process_Protocol_Error()
{
     PD_MSG_t* message_rx=pd.PD_Msg_Get();
	   uint8_t msg_data_num=message_rx->header.number_Data_Objects;
	   uint8_t msg_extended=message_rx->header.extended;
	   uint8_t msg_type=message_rx->header.message_Type;
	if(msg_data_num==0 && msg_extended==0)
	{
		  SNK_Process_Control_Msg[msg_type]();	
	}

	else if(msg_extended==0 && msg_data_num!=0)
	{
			switch(msg_type)
			{
				case	   SOURCE_CAPABILITIES:		        break;//do nothing 
				case	   REQUEST:            		        break;//process outside 
				case	   BIST:                  		
														if( message_rx->data[0]>>28==BIST_CARRIER_MODE && (tcpc_status.above5V==0) )
														{
															 flag_snk_send_PD_msg=1;  
															tcpc_status.PDstate_SNK=PE_SNK_BIST_Carried_Mode;
														}
														else if(message_rx->data[0]>>28==BIST_TEST_DATA)
														 { 
															 pd.enable_bist_data_mode(1);
															 tcpc_status.PDstate_SNK=PE_SNK_BIST_TEST_DATA;
														 }
														else 
														{
															tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported; flag_snk_send_PD_msg=1;  
														}
															break;
				case	   SINK_CAPABILITIES:  		            break;  //do nothing
				case	   BATTERY_STATUS:     		            break;  //do nothing 
				case	   ALERT:              		PD_SNK_Send_Not_Supported();        break;  
				case	   GET_COUNTRY_INFO:   	  PD_SNK_Send_Not_Supported();         break;
				case	   ENTER_USB:          		PD_SNK_Send_Not_Supported();      break;
				case	   VENDER_DEFINED:     	  PD_SNK_Send_Not_Supported();      break;// do nothing 
		    default:                                    break;
			}
	}
	else 
	{
			switch(msg_type)
			{
			case		  SOURCE_CAPABILITIES_EXTENDED:                                                        break;
			case	    STATUS:                                                                              break;
			case	    GET_BATTERY_CAP:          PD_SNK_Send_Not_Supported();        break;
			case	    GET_BATTERY_STATUS :       PD_SNK_Send_Not_Supported();      break;
			case	    BATTERY_CAPABILITIES :                                                                                  break;
			case	    GET_MANUFACTURER_INFO :   PD_SNK_Send_Not_Supported();     break;
			case	    MANUFACTURER_INFO :        PD_SNK_Send_Not_Supported();     break;
			case	    SECURITY_REQUES:          PD_SNK_Send_Not_Supported();    break;
			case	    SECURITY_RESPONSE :       PD_SNK_Send_Not_Supported();      break;
			case	    FIRMWARE_UPDATE_REQUEST :       break;
			case	    FIRMWARE_UPDATE_RESPONSE:       break;
			case	    PPS_STATUS :                    break;
			case	    COUNTRY_INFO :                  break;
			case	    COUNTRY_CODES :                 break;
			
			case	    SINK_CAPABILITIES_EXTENDED :    break;
			case      EXTENDED_CONTROL:               break;
			case	    EPR_SOURCE_CAP:                 break;
			case      EPR_SINK_CAP:                   break;
			case		  31:                         if(((message_rx->data[0])&(0X00008000))!=0)	//chunked message
																					    Start_Chunking_Notsupported_Timer(Timer_Chunking_Notsupported,T_CHUNKING_NOTSUPPORTED,PD_SNK_Send_Not_Supported);
                                            else //unchunked message    																					 
				                                      Start_Chunking_Notsupported_Timer(Timer_Chunking_Notsupported,T_UNCHUNKING_NOTSUPPORTED,PD_SNK_Send_Not_Supported); 
																					       break;

			default :                                 break;
			}
	}

}


void PE_SNK_Soft_Reset_Receive_act()
{
		  PE_SNK_Send_Control_Msg_act(ACCEPT,//to send msg
	                                PE_SNK_Wait_for_Capablities,NOT_SEND_NEW_MSG, //success path
	                                PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path
	

}

void PE_SNK_Give_Sink_Cap_act()
{
	PE_SNK_Send_Complx_Msg_act(PD_Advertise_SNK_Capablity,PE_SNK_Ready,NOT_SEND_NEW_MSG,PE_SNK_Soft_Reset,SEND_NEW_MSG);

}


void PE_SNK_Get_Source_Cap_act()
{
		  PE_SNK_Send_Control_Msg_act(NOT_SUPPORTED,//to send msg
	                              PE_SNK_Ready,NOT_SEND_NEW_MSG, //success path
	                              PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path
	
}

void PE_SNK_BIST_Carried_Mode_act(void)
{
      PE_SNK_Send_Complx_Msg_act( pd.send_bist_carry_mode, 
	                                PE_SNK_Ready,NOT_SEND_NEW_MSG,
	                                PE_SNK_Ready,NOT_SEND_NEW_MSG);

}


void PE_SNK_Send_GetSRCCAP_act(void)
{
	
	  PE_SNK_Send_Control_Msg_act(GET_SOURCE_CAP,//to send msg
	                              PE_SNK_Ready,NOT_SEND_NEW_MSG, //success path
	                              PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path

}

void PE_SNK_Send_Debug_act()
{			
	 
}

void  PE_SNK_Send_Control_Msg_act(CONTROL_MESSAGE_TYPE_e message_type,PD_snk_state_e success_path,uint8_t success_send_flag,PD_snk_state_e fail_path,uint8_t fail_send_flag)
{
	 	if(flag_snk_send_PD_msg==1)
		{	
			pd.send_ctrl_msg(message_type,TYPE_SOP);
			flag_snk_send_PD_msg=0;
		}
		switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
			case TRANSMIT_FAIL: 	   tcpc_status.PDstate_SNK=fail_path;	
															 flag_snk_send_PD_msg=fail_send_flag;
											         break; //delay then transmit
			case TRANSMIT_DISCARD:	 flag_snk_send_PD_msg =1; 																	
									             break;	   //retransmit
			case TRANSMIT_SUCCESS:    
										    	     tcpc_status.PDstate_SNK=success_path;  
			                         flag_snk_send_PD_msg=success_send_flag;
										           break;	
			case TRANSMIT_ERROR:	   flag_snk_send_PD_msg =1; 						  break;   //retransmit
			default 		:																							      break;   //
		}
}

void  PE_SNK_Send_Complx_Msg_act(fun_t* action,PD_snk_state_e success_path,uint8_t success_send_flag,PD_snk_state_e fail_path,uint8_t fail_send_flag)
{
	 	if(flag_snk_send_PD_msg==1)
		{	
			action();
			flag_snk_send_PD_msg=0;
		}
		switch(pd.PD_Transmit_Status_Get())//Read would clear this status
		{
			case TRANSMIT_FAIL: 	   tcpc_status.PDstate_SNK=fail_path;	
															 flag_snk_send_PD_msg=fail_send_flag;
											         break; //delay then transmit
			case TRANSMIT_DISCARD:	 flag_snk_send_PD_msg =1; 																	
									             break;	   //retransmit
			case TRANSMIT_SUCCESS:    
										    	     tcpc_status.PDstate_SNK=success_path;  
			                         flag_snk_send_PD_msg=success_send_flag;
										           break;	
			case TRANSMIT_ERROR:	   flag_snk_send_PD_msg =1; 						  break;   //retransmit
			default 		:																							      break;   //
		}
}


/* *****************************************************Soft Reset Process******************************************************
typedef enum
{
   FSM_PD_SNK_SR_SEND_SOFTRRESET=0,
	 FSM_PD_SNK_SR_WAIT_ACCEPT,
	 FSM_PD_SNK_SR_RESTART
}pd_snk_soft_reset_adjust_fsm_e;

********************************************************************************************************************************/

static fsm_result_e (*_fsm_pd_snk_soft_reset_handle[])(pd_snk_soft_reset_adjust_fsm_e*)=
{
	_fsm_pd_snk_sr_send_soft_reset,
	_fsm_pd_snk_sr_wait_accept,
	_fsm_pd_snk_sr_restart
};

void PE_SNK_Soft_Reset_fsm()
{
	fsm_result_e result;
	do
	{
	   result=_fsm_pd_snk_soft_reset_handle[fsm_pd_snk_soft_reset](&fsm_pd_snk_soft_reset); 
	}while (result==FSM_SWITCH_IMM);
}

static fsm_result_e _fsm_pd_snk_sr_send_soft_reset(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm)
{
	 if(flag_snk_send_PD_msg==1)
	{			
	  tcpc_status.msg_id_tx[TYPE_SOP]=0;
	  tcpc_status.msg_id_rx[TYPE_SOP]=0;
		tcpc_status.msg_id_rx_init[TYPE_SOP]=0;
		pd.send_ctrl_msg(SOFT_RESET,TYPE_SOP);
		DEBUG_INFO("\r\nTx:soft rst");
		flag_snk_send_PD_msg=0;
    }
    else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
	{
	    case TRANSMIT_FAIL:          tcpc_status.PDstate_SNK=PE_SNK_Hard_Reset;
	                                	DEBUG_INFO("\r\n[S] change to PE_SNK_Hard_Reset ");
		                               return FSM_WAITING;//change to FSM_WAITING    @20221021
	    case TRANSMIT_DISCARD:       flag_snk_send_PD_msg =1;     	                                                            
							                     break;   //retransmit
		  case TRANSMIT_SUCCESS:    
															    *pd_snk_softreset_fsm=FSM_PD_SNK_SR_WAIT_ACCEPT;
															    Start_SenderResponseTimer(Timer_SENDER_RESPONSE,25,PD_SNK_Send_Hard_Reset);			// /*Timer_CC_DEBOUNCE=0,Timer_PD_DEBOUNCE,Timer_Chunking_Notsupported,Timer_SENDER_RESPONSE,Timer_PSTransition,Timer_SinkWaitCap,Timer_Hardreset*/
															    return FSM_SWITCH_IMM;
		case TRANSMIT_ERROR:          flag_snk_send_PD_msg =1;                                                               	  break;   //retransmit
		default  :                                                                                         
			break;   //
	}
	return FSM_WAITING;
}


static fsm_result_e _fsm_pd_snk_sr_wait_accept(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm)
{
	  PD_MSG_t*  message_rx;
    if(pd.is_new_msg_received())
	{
		pd.clear_new_msg_received();
		message_rx=pd.PD_Msg_Get();
		if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
		{
			 Stop_SenderResponseTimer(Timer_SENDER_RESPONSE);	
			 *pd_snk_softreset_fsm=FSM_PD_SNK_SR_RESTART;
			 DEBUG_INFO("\r\n[S]receive_accept");
			 return FSM_SWITCH_IMM;
		}
	}
    return FSM_WAITING;
}


static fsm_result_e _fsm_pd_snk_sr_restart(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm)
{
   	  //reset protocal layer
   	  *pd_snk_softreset_fsm=FSM_PD_SNK_SR_SEND_SOFTRRESET;
	    tcpc_status.PDstate_SNK=PE_SNK_Discovery;
	     DEBUG_INFO("\r\n[S]back to SNK_Discovery");
      return FSM_WAITING;
}
