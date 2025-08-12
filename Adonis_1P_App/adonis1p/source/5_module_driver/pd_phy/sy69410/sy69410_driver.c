#include "sy69410_driver.h"
#include "pd_i2c.h"
#include "typec.h"
#include "pd_global.h"
#include "pd_cfg.h"
#include "pd_timer.h"
#include "pd_com.h"
#include "pd_i2c.h"

#include "driver_pd_phy.h"

//pd_status_t          pd_status;
static  uint8_t 	   tx_buf[40];
static	uint8_t      rx_buf[40];
static  PD_MSG_t     new_rx_message; 
static  PD_MSG_t     *pd_rx_info;

void Command_Write(COMMAND_TYPE_e command)
{
    Set1Reg(0x23, command);
}

void CC_Role_Config(CC_ROLE_CONFIG_t cc_role_config)
{
    uint8_t temp = cc_role_config.byte;
    Set1Reg(0x1A, temp); 
}

void CC_Role_Force(DRP_EN_e a, RP_VALUE_TYPE_e b, CC_TYPE_e c2,CC_TYPE_e c1)
{
    uint8_t temp = (a<<6)|(b<<4)|(c1<<0)|(c2<<2);
    Set1Reg(0x1A, temp); 
}

void Hard_Reset_Send(void)
{
   Set1Reg(0x50, 0x05); // start transmit HardReset, Retry 0 times
}

void BIST_Carrier_Mode_Send(void)
{
	 Set1Reg(0x50, 0x07); // Enter  BIST carrier mode for 45ms  
}


void BIST_TEST_DATA_Mode_Enter(void)
{
	uint8_t temp =Get1Reg(0x19);
	Set1Reg(0x19, temp|(0x02)); // Enter  BIST Test data mode
}
void BIST_TEST_DATA_Mode_Quit(void)
{
	uint8_t temp =Get1Reg(0x19);
	Set1Reg(0x19, temp&(~0x02)); // Quit BIST Test data mode
}

void  PD_Receive_Detect_Config(RECEIVE_DETECE_t temp)
{
    Set1Reg(0X2F, temp.byte);
}

void PD_Msg_Send(PD_MSG_t tmsg)
{
    uint8_t i,temp;
    uint8_t WRITE_BYTE_COUNT;
	  //before send message,clear the last message sending status
	//  PD_Transmit_Status_Get();it is necessary but not here
    WRITE_BYTE_COUNT = 2 + (tmsg.header.number_Data_Objects<< 2); //Header:2 Bytes+Data object( num_data_objects*4 )= RITE_BYTE_COUNT=2+4*n
    tx_buf[0] = WRITE_BYTE_COUNT; 	                         
	  tx_buf[1] = tmsg.header.w&(0xFF);
	  tx_buf[2] = (tmsg.header.w>>8)&(0xFF);     //Message header part
    temp = 3;
    for( i = 0; i < tmsg.header.number_Data_Objects; i++)
    {
		    tx_buf[temp] =     tmsg.data[i]&(0XFF);
        tx_buf[temp + 1] = (tmsg.data[i]>>8)&(0XFF);
        tx_buf[temp + 2] = (tmsg.data[i]>>16)&(0XFF);
        tx_buf[temp + 3] = (tmsg.data[i]>>24)&(0XFF);
        temp += 4;
    }                                         //Message Data part 
	 I2CWriteBytes(TCPC_Address, TX_BUF_ADDRESS, WRITE_BYTE_COUNT + 1, tx_buf); // write Tx_buffer
	
   if(tmsg.Sop_Type!=TYPE_SOP)
		  Set1Reg(0x50, 0x00|tmsg.Sop_Type); //No retry,start transmit 
	 else if(tmsg.header.spec_Rev == REV_3)
    	Set1Reg(0x50, 0x20|tmsg.Sop_Type); //retry 2 times,start transmit 
   else 
    	Set1Reg(0x50, 0x30|tmsg.Sop_Type); //retry 3 times,start transmit 
	 if(tmsg.header.number_Data_Objects==0)
	    DEBUG_INFO("\r\n\nTx->[%s]ID=%d,Rev:%d,P:%d,D:%d",control_msg_str[tmsg.header.message_Type],tmsg.header.messageID,tmsg.header.spec_Rev+1,tmsg.header.port_Power_Role,tmsg.header.port_Data_Role);
   else if(tmsg.header.extended==0)
	    DEBUG_INFO("\r\n\nTx->[%s]ID=%d,Rev:%d,P:%d,D:%d",data_msg_str[tmsg.header.message_Type],tmsg.header.messageID,tmsg.header.spec_Rev+1,tmsg.header.port_Power_Role,tmsg.header.port_Data_Role);
    else
      DEBUG_INFO("\r\n\nTx->[%s]ID=%d,Rev:%d,P:%d,D:%d",ext_data_msg_str[tmsg.header.message_Type],tmsg.header.messageID,tmsg.header.spec_Rev+1,tmsg.header.port_Power_Role,tmsg.header.port_Data_Role);
	
} 

uint8_t PD_Receive(void)
{
    uint8_t  i,num;
    uint16_t READABLE_COUNT;
    pd_rx_info= &new_rx_message;
    uint8_t extened_bit;
    I2CReadBytes(TCPC_Address, RX_BUF_ADDRESS, 2, rx_buf);
    if(rx_buf[0] > 1)
    {
	     READABLE_COUNT = rx_buf[0];
	     pd_rx_info->Sop_Type=(SOP_TYPE_e)rx_buf[1]; 
	     I2CReadBytes(TCPC_Address, RX_BUF_ADDRESS, READABLE_COUNT + 1, rx_buf); //��ȡBUffer�ڵ�ȫ������
      //Note:  
        //     rx_buf[0]:READABLE_BYTE_COUNT
        //     rx_buf[1]:RX_BUF_FRAME_TYPE
        //     rx_buf[2]:Header_L   rx_buf[3]:Header_H
        //     rx_buf[4-7]:Data1     rx_buf[8-11]:Data2      rx_buf[12]~rx_buf[15]: Date3   rx_buf[16]~rx_buf[19]: Date4
			pd_rx_info->header.w=(uint32_t)(rx_buf[2]|(rx_buf[3]<<8));
			num=pd_rx_info->header.number_Data_Objects;
			 extened_bit=pd_rx_info->header.extended;
			for(i=0;i<num;i++)
			{
				  pd_rx_info->data[i]=(rx_buf[i*4+7]<<24)|(rx_buf[i*4+6]<<16)|(rx_buf[i*4+5]<<8)|rx_buf[i*4+4];
			}
			
			if(num==0)
			{
			  DEBUG_INFO("\r\nRx<[%s]-ID:%d,Rev:%d,P:%d,D:%d,ctr:%d \r\n",control_msg_str[pd_rx_info->header.message_Type],pd_rx_info->header.messageID,pd_rx_info->header.spec_Rev+1,pd_rx_info->header.port_Power_Role,pd_rx_info->header.port_Data_Role,pd_rx_info->header.message_Type);
			}
			else if(extened_bit==0)
				DEBUG_INFO("\r\nRx<[%s]-ID:%d,Rev:%d,P:%d,D:%d,data:%d \r\n",data_msg_str[pd_rx_info->header.message_Type],pd_rx_info->header.messageID,pd_rx_info->header.spec_Rev+1,pd_rx_info->header.port_Power_Role,pd_rx_info->header.port_Data_Role,pd_rx_info->header.message_Type);
			else  
		   	DEBUG_INFO("\r\nRx<[%s]-ID:%d,Rev:%d,P:%d,D:%d,data:%d \r\n",ext_data_msg_str[pd_rx_info->header.message_Type],pd_rx_info->header.messageID,pd_rx_info->header.spec_Rev+1,pd_rx_info->header.port_Power_Role,pd_rx_info->header.port_Data_Role,pd_rx_info->header.message_Type);
			
			return TRUE;
    }
    return FALSE;
}

void PD_Goodcrc_Header_Init(CABLE_PLUG_TYPE_e cable_plug_type,DATA_ROLE_TYPE_e data_role_type,POWER_ROLE_TYPE_e pwr_role_type,SPEC_REV_TYPE_e spec_rev_type)
{
    uint8_t temp;
    temp=pwr_role_type|(spec_rev_type<<1)|(data_role_type<<3)|(cable_plug_type<<4);
    Set1Reg(0x2E,temp);
}

void PD_Receive_Enable(uint8_t temp)//auto send GoodCRC  enable
{
    uint8_t reg=Get1Reg(0X2F);
    reg=temp;
    Set1Reg(0x2F, reg);
}

void Hard_Reset_Disable(void)
{
    uint8_t temp;
    temp = Get1Reg(0x2F);
    temp = temp & (~0x20);
    Set1Reg(0x2F, temp);
}

void Hard_Reset_Enable(void)
{
    uint8_t temp;
    temp = Get1Reg(0x2F);
    temp = temp | (0x20);
    Set1Reg(0x2F, temp);
}

PD_MSG_t*  PD_Msg_Get(void)
{
    return pd_rx_info;
}

void CC_DRP_Period_Set(DRP_PERIOD_e period)
{
	  Set1Reg(0xA2, period);
}

void CC_DRP_Rp_Duty_Set(uint16_t duty)
{
	if(duty>1023)
		duty=1023;
	Set1Reg(0xA3, (uint8_t)(duty&(0xFF)));
	Set1Reg(0xA4, (uint8_t)(duty>>8));
}

void Vcon_Init()
{
	Vcon_Off();
	Vcon_Ocp_Set(Curr_300mA);
}

void Vcon_On(void)
{
	 Set1Reg(0x1C, 0X01);
	// pd_status.VCONN_ON=1;
}

void Vcon_Off(void)
{
	Set1Reg(0x1C, 0X00);
	// pd_status.VCONN_ON=0;
}

void Vcon_Discharge_On(void)
{
	uint8_t temp;
	temp=Get1Reg(0x90);
	Set1Reg(0x90,temp|0x20);
}

void Vcon_Discharge_Off(void)
{
	uint8_t temp;
	temp=Get1Reg(0x90);
	Set1Reg(0x90,temp&(~0x20));
}

void Vcon_Ocp_Set(VCON_OCP_e value)
{
	Set1Reg(0x93, value<<5);
}

void Vender_Power_Config(VENDER_PWR_t pwr_config)
{
	Set1Reg(0X90,pwr_config.byte);
}


VENDER_PWR_t  Vender_Power_Get(void)
{
	VENDER_PWR_t pwr_config;
	uint8_t temp=Get1Reg(0X90);
	pwr_config.byte=temp;
	return pwr_config;
}

void Vender_Reset(void)
{
	Set1Reg(0xA0, 0X01);
	//To Do:delay 2ms
}

void Vender_Wakeup_Enable(void)
{
	Set1Reg(0x9F, 0X80);
}

void Vender_Wakeup_Disable(void)
{
	Set1Reg(0x9F, 0X00);
}

void Vender_Auto_Idle_Enable(uint8_t time_out_value)
{
	uint8_t temp;
	temp=Get1Reg(0x9B);
	Set1Reg(0x9B,temp|0X08|(time_out_value&0X07));
}

void Vender_Auto_Idle_Diable(void)
{
	uint8_t temp;
	temp=Get1Reg(0x9B);
	Set1Reg(0x9B,temp&(~0X08));
}

void Vender_PDIC_Shutdown_Quit(void)
{
	uint8_t temp;
	temp=Get1Reg(0x9B);
	Set1Reg(0x9B,temp|(0X20));
}

void Vender_PDIC_Shutdown_Enter(void)
{
	uint8_t temp;
	temp=Get1Reg(0x9B);
	Set1Reg(0x9B,temp&(~0X20));
}


void Vcon_Oc_Detect_Disable(void)
{
	uint8_t temp;
	temp=Get1Reg(0x1B);
	Set1Reg(0x1B,temp|0X01);
}

void Vcon_Oc_Detect_Enable(void)
{
	uint8_t temp;
	temp=Get1Reg(0x1B);
	Set1Reg(0x1B,temp&(~0X01));
}
	
void Vcon_Ov_Detect_Disable(void)
{
	uint8_t temp;
	temp=Get1Reg(0x1B);
	Set1Reg(0x1B,temp|0X80);
}

void Vcon_Ov_Detect_Enable(void)
{
	uint8_t temp;
	temp=Get1Reg(0x1B);
	Set1Reg(0x1B,temp&(~0X80));
}

void CC_Plug_Set(uint8_t cc)
{

	uint8_t temp;
	temp=Get1Reg(0x19);
	if(cc==1)
	  temp=temp&(~01);  //Choose CC1 to PD communication
	else if(cc==2)
		temp=temp|(01);
	Set1Reg(0x19,temp); //Choose CC2 to PD communication	
}

uint8_t CC_status_Get(void)
{
    uint8_t temp=Get2Reg(0x1D);
    return temp;
}
uint8_t Power_status_Get(void)
{
    uint8_t temp=Get2Reg(0x1E);
    return temp; 
}

uint8_t  Fault_status_Get(void)
{
    uint8_t temp=Get2Reg(0x1F);
    return temp; 
}

uint16_t Alert_status_Get(void)
{
    uint16_t temp=Get2Reg(0x10);
    return temp;
}

void Alert_Clear(uint16_t temp)
{
    Set2Reg(0x10,temp);
}

#define DEBUG_PR_SWAP     3 
#define DEBUG_VCONN_SWAP  4

extern pd_phy_t pd;

#define SY_REG_DEV_ID (0x04)
static bool sy69410_check(void)
{
	bool ret = false;
	uint16_t val = 0;

	for (uint8_t i=0; i<3; i++) {
		val = Get2Reg(SY_REG_DEV_ID);
		if (val == 0x3C02) {
			ret = true;
			sm_log(SM_LOG_INFO, "sy69410 id:0x%x\n", val);
			break;
		}
		//Vender_Reset();
		pd_phy_mDelay(2);
	}

	return ret;
}

int sy69410_init(void)
{
	int ret = 0;

	if (sy69410_check() != true) {
		return (-1);
	}

	PD_Power_Sink_Capability_Init();
    tcpc_status.drp_state=0x01;
    BA41_Init();
    pd.vcon_off();
    Typec_Init();
    pd_phy_mDelay(1);
    pd_timer_intial();

    return ret; // RET_SUCCESS (0) : RET_FAILURE (-1)
}

int sy69410_deinit(void)
{
	int ret = 0;

	Vender_Reset();
    pd_phy_mDelay(2);
    Vender_PDIC_Shutdown_Enter();

    return ret;
}

void sy69410_proc(void)
{
	check_pd_timer_out();
	Int_Fun();                                                                                                                                                                                                                                      // st\r\n");
    TypeC_scan();
}

