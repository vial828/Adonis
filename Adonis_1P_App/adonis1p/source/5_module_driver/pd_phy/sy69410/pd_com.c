#include "pd_com.h"
#include "pd_global.h"
#include "pd_general.h"
#include "pd_i2c.h"
#include "pd_timer.h"
#include "sy69410_driver.h"

//1.0 2022.0328 add "PD_Transmit_Status_Get()" before sending message;
	  
TCPC_STATUS_Type     tcpc_status;
pd_status_t          pd_status;
pd_timer_type        sy69410_pd_timer[7];

static  uint8_t      pd_rx_count;
static  tx_result_t  pd_tx_result;

void idle_delay(uint8_t time_limit)
{
	 uint8_t i;
	 for(i=0;i<200;i++);
}

bool IsVBusOK(void)
{
	if((pd_status.POWER_STATUS_REG&0x04)!=0)
		return TRUE;
	else
		return FALSE;
}

void Int_Fun(void)
{
	uint16_t ALERT_REG;
	uint8_t	 REG_1FH; 
	uint8_t CC_STATUS_REG;
	ALERT_REG = Get2Reg(0x10); //Alert address 10h-11h
	if(ALERT_REG!=0)
	{
	    if((ALERT_REG & ALERT_CC_STATUS) == ALERT_CC_STATUS)
	    {
	        CC_STATUS_REG = Get1Reg(0x1D); //CC STASUS address 1D	     
          				
	        pd_status.Looking4Connection = (CC_STATUS_REG & (0x20)) >> 5;
	        pd_status.ConnectResult = (CC_STATUS_REG & (0x10)) >> 4;
	        pd_status.CC2_State = (CC_STATUS_REG & (0x0C)) >> 2;
	        pd_status.CC1_State = (CC_STATUS_REG & (0x03));
			// DEBUG_INFO("\r\n %d,%d, %d, %d",pd_status.Looking4Connection,pd_status.ConnectResult,pd_status.CC2_State,pd_status.CC1_State );
				  
	    }

	    if((ALERT_REG & ALERT_POWER_STATUS) == ALERT_POWER_STATUS)
	    {
			pd_status.POWER_STATUS_REG = Get1Reg(0x1E); //Power STASUS address 1E
			pd_status.vconn_present=(pd_status.POWER_STATUS_REG>>1)&0x01;
	    }

	    if((ALERT_REG & ALERT_RX_HARD_RESET) == ALERT_RX_HARD_RESET)
	    {
	        pd_status.R_HARD_RESET = 1;
	    }

	    if((ALERT_REG & ALERT_RX_SOP) == ALERT_RX_SOP)
	    {
	        PD_Receive();
			// if((new_rx_message.header.messageID!=tcpc_status.rx_stored_msgID)||(new_rx_message.header.message_Type==0 && (new_rx_message.header.number_Data_Objects==0)))
			pd_rx_count++;
	    }

	    if((ALERT_REG & ALERT_TX_FAIL) == ALERT_TX_FAIL && (ALERT_REG & ALERT_TX_SUCCESS) == ALERT_TX_SUCCESS)
	    {
	        pd_tx_result.Transmit_Hard_Reset_Success = 1;
	    }
	    else if((ALERT_REG & ALERT_TX_FAIL) == ALERT_TX_FAIL)
	    {
			pd_tx_result.Transmit_Fail = 1;
			pd_tx_result.transmit_status=TRANSMIT_FAIL;
			DEBUG_INFO("\r\n TX_Fail!!!!");
	    }
	    else if((ALERT_REG & ALERT_TX_DISCARD) == ALERT_TX_DISCARD)
	    {
	        pd_tx_result.Transmit_Discard = 1;
			    pd_tx_result.transmit_status=TRANSMIT_DISCARD;
	    }
	    else if((ALERT_REG & ALERT_TX_SUCCESS) == ALERT_TX_SUCCESS)
	    {
	        pd_tx_result.Transmit_Success = 1;
		      pd_tx_result.transmit_status=TRANSMIT_SUCCESS;
	    }

	    if((ALERT_FAULT & ALERT_REG) == ALERT_FAULT)
	    {
	        REG_1FH = Get1Reg(0x1F);

	        if((REG_1FH & 0x01) == 1)//i2C error
	        {
	            Command_Write(RESET_TX_BUF);
	            pd_tx_result.Transmit_Discard = 1;
	            pd_tx_result.I2C_error = 1;
	        }
			if((REG_1FH & 0x02) == 0x02)//VCON_OC happened
			{
				Vcon_Off();
				//To Do:
			}
			if((REG_1FH & 0x80) == 0x80)//VCON_OV happened
			{
				Vcon_Off();
				Vcon_Discharge_Off();
				//To Do:
			}
			if((REG_1FH & 0x04) == 0x04)//CC_UV_Fault happeded
			{
				//To Do:
			}
	        Set1Reg(0x1F, REG_1FH);
	    }
		 if((ALERT_REG & ALERT_CC_STATUS) == ALERT_CC_STATUS)//added 20221021byGW
	    {
	        CC_STATUS_REG = Get1Reg(0x1D); //CC STASUS address 1D	     
          				
	        pd_status.Looking4Connection = (CC_STATUS_REG & (0x20)) >> 5;
	        pd_status.ConnectResult = (CC_STATUS_REG & (0x10)) >> 4;
	        pd_status.CC2_State = (CC_STATUS_REG & (0x0C)) >> 2;
	        pd_status.CC1_State = (CC_STATUS_REG & (0x03));
			// DEBUG_INFO("\r\n %d,%d, %d, %d",pd_status.Looking4Connection,pd_status.ConnectResult,pd_status.CC2_State,pd_status.CC1_State );
	    }
	    Set2Reg(0x10, ALERT_REG);//or  Set1Reg(0X10,0xFF); Set1Reg(0X11,0xFF);	
	 	  idle_delay(100);//it is necessary to ensure INT is cleared ;
	}
}

PDTRANS_STATUS_e  PD_Transmit_Status_Get(void)
{
	PDTRANS_STATUS_e temp=pd_tx_result.transmit_status;
	if(temp!=0)
		pd_tx_result.transmit_status=TRANSMIT_NONE;
	return temp;
}


void start_drp(void)
{
	Command_Write(LOOK_4_CONNECTION);
}

void enable_bist_data_mode(uint8_t en)
{	
	if(en==0)
		BIST_TEST_DATA_Mode_Quit();
	else
		BIST_TEST_DATA_Mode_Enter();
}


void PD_Send_Control_Message(CONTROL_MESSAGE_TYPE_e type,SOP_TYPE_e sop_type )
{
	PD_MSG_t t_mes;
	t_mes.header.extended=0;
	
	t_mes.header.message_Type=type;
	t_mes.header.number_Data_Objects=0;
	t_mes.header.port_Data_Role=tcpc_status.data_role;
	t_mes.header.port_Power_Role=tcpc_status.power_role;
	t_mes.header.spec_Rev=tcpc_status.pd_rev;
	t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
	tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
	t_mes.Sop_Type=TYPE_SOP;
	PD_Msg_Send(t_mes);
}

//extended header
/*
extender_L extender_H  byte0=type byte1=data=0

*/
void PD_Send_Extended_Control_Message(uint8_t type)
{
	PD_MSG_t t_mes;
	PD_MSG_EXTENDED_HEADER_t ex_header;
	t_mes.header.extended=1;
	t_mes.header.message_Type=EXTENDED_CONTROL;
	t_mes.header.number_Data_Objects=1;
	t_mes.header.port_Data_Role=tcpc_status.data_role;
	t_mes.header.port_Power_Role=tcpc_status.power_role;
	t_mes.header.spec_Rev=tcpc_status.pd_rev;
	t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];	
	ex_header.chunked=1;
	ex_header.chunk_number=0;
	ex_header.data_size=2;
	ex_header.request_chunk=0;
	ex_header.reserved=0;
	t_mes.data[0]=ex_header.w|(type<<16);
	tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
	t_mes.Sop_Type=TYPE_SOP;
	PD_Msg_Send(t_mes);
}

/*
void PD_Send_EPR_Mode(epr_action_e action)
{
		PD_MSG_t t_mes;
	  erp_mode_t epr_data;
	  t_mes.header.extended=0;
		t_mes.header.message_Type=EPR_MODE;
	  t_mes.header.number_Data_Objects=1;
	  t_mes.header.port_Data_Role=tcpc_status.data_role;
	  t_mes.header.port_Power_Role=tcpc_status.power_role;
  	t_mes.header.spec_Rev=tcpc_status.pd_rev;
	  t_mes.header.messageID=tcpc_status.msg_id_tx[TYPE_SOP];
    tcpc_status.msg_id_tx[TYPE_SOP]=((tcpc_status.msg_id_tx[TYPE_SOP])+1)%8;
    t_mes.Sop_Type=TYPE_SOP;
	  epr_data.bit.action=action;
	  epr_data.bit.data=0;
	  epr_data.bit.reserve=0;
	  t_mes.data[0]=epr_data.w;
    PD_Msg_Send(t_mes);
}
*/

bool PD_Receive_NewMsg_Check(void)
{
	if(pd_rx_count>0)
		return TRUE;
	else
	  return FALSE; 
}

// 锟接口诧拷
bool PD_Receive_Hard_Reset_Check(void)
{ 
   bool temp; 
	 temp=((pd_status.R_HARD_RESET==1)?TRUE:FALSE);
	 if(pd_status.R_HARD_RESET==1)  pd_status.R_HARD_RESET=0;
	 return temp;
}


//锟接口诧拷
void PD_Receive_NewMsg_Flag_Clear(void) // decrease 1
{
	  if(pd_rx_count>0)
	    pd_rx_count--;
	 //  return TRUE;
}

// DATA_ROLE_TYPE_e POWER_ROLE_TYPE_e SPEC_REV_TYPE_e
void  Set_Power_Role(POWER_ROLE_TYPE_e temp)
{
	if(tcpc_status.power_role!=temp)
	{
		 tcpc_status.power_role=temp;
		 PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
	}
}

void  Set_Data_Role(DATA_ROLE_TYPE_e temp)
{
	 if(tcpc_status.data_role!=temp)
	{
		 tcpc_status.data_role=temp;
		 PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
	}
}

void  Set_PD_Rev(SPEC_REV_TYPE_e temp)
{
	if(tcpc_status.pd_rev!=temp)
	{
		 tcpc_status.pd_rev=temp;
		 PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
	}
}

unsigned char Is_looking_4_connection(void)
{
	if(pd_status.Looking4Connection==1)
		return 1;
	else
		return 0;
}

void vcon_on(void)
{
	Vcon_On();
	pd_status.VCONN_ON=1;
}

void vcon_off(void)
{
	Vcon_Off();
	pd_status.VCONN_ON=0;
}

pd_phy_t pd={
	.start_drp=start_drp,
	.set_cc_plug=CC_Plug_Set,
	.set_cc_role=CC_Role_Force,
	.set_drp_period=CC_DRP_Period_Set,//Used set Dual_role_togggle period
	.set_drp_rp_duty=CC_DRP_Rp_Duty_Set,   //used to ser Rp duty cycle during one DRP period :Rp_duty=(duty+1)/1024
	.set_power_role=Set_Power_Role,
	.set_data_role=Set_Data_Role,
	.set_pd_rev=Set_PD_Rev,
	.enable_pd_receive=PD_Receive_Enable,
	.PD_Msg_Get=PD_Msg_Get,
	.enable_bist_data_mode=enable_bist_data_mode,
	.send_hard_reset=Hard_Reset_Send,
	.send_msg=PD_Msg_Send,
	.send_ctrl_msg=PD_Send_Control_Message,
	.send_bist_carry_mode=BIST_Carrier_Mode_Send,
	.is_new_msg_received=PD_Receive_NewMsg_Check,
	.is_vbus_ok=IsVBusOK,
	.clear_new_msg_received=PD_Receive_NewMsg_Flag_Clear,
	.is_hard_reset_received=PD_Receive_Hard_Reset_Check,
	.PD_Transmit_Status_Get=PD_Transmit_Status_Get,
	.vcon_init=Vcon_Init,
	.vcon_on=vcon_on,//turn on VCON_BKFT
	.vcon_off=vcon_off//Disable VCON_BKFT
};

void BA41_Init(void)
{
	VENDER_PWR_t temp;
//	Vender_Reset();
//	pd_phy_mDelay(2);
	Vender_PDIC_Shutdown_Quit();
	pd_phy_mDelay(2);

	temp.LPR_EN=0;
	temp.BG_EN=1;
	temp.OSC_EN=1;
	temp.VCON_DISCHARGEEN=0;
	temp.VBUS_DETECTEN=1;

	Vender_Power_Config(temp);
	Vender_Auto_Idle_Enable(PERIOD_102_4ms);
	Command_Write(ENABLE_VBUS_DETECT);
	
	Vender_Auto_Idle_Diable();
	Int_Fun();
	
	pd.enable_pd_receive(EN_HARD_RESET);
	
	tcpc_status.power_role=(POWER_ROLE_TYPE_e)SRC_ROLE;
	tcpc_status.data_role =(DATA_ROLE_TYPE_e)DFP;
	tcpc_status.pd_rev=REV_3;

	PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, REV_3);
}

