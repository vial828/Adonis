#include "typec.h"
#include "pd_snk.h"
#include "pd_global.h"
#include "pd_timer.h"
#include "driver_pd_phy.h"
#include "public_typedef.h"

#define SRC_ONLY  0x02
#define SNK_ONLY  0x01
#define DRP_ONLY  0x03

extern pd_status_t pd_status;
extern pd_phy_t pd;

void TypeC_scan(void)
{
	if(!Action_Check_Key())
	{
		if(tcpc_status.error_recovery==1)
		{
			pd.set_cc_role(NO_DRP_MODE,RP_3A,Open_CC,Open_CC);
			tcpc_status.error_recovery=0;
			pd_phy_mDelay(T_ERROR_ROCOVERY);        //delay 25ms
			tcpc_status.Now_TypeC_State= Unattached_SNK;
			pd.set_cc_role(DRP_MODE,RP_3A,RD_CC,RD_CC);
		}

		switch(tcpc_status.Now_TypeC_State)
		{
			case Unattached_SNK:
				Is_Unattached_SNK();  break;
			case Attachwait_SNK:
				Action_Attachwait_SNK(); break;
			case Attached_SNK  :
				Action_Attached_SNK(); break;
		}
	}
}

bool Action_Check_Key(void)
{
	  return FALSE;
}

void Choose_PDcommunication_CC(void)
{
	 if(tcpc_status.CC1_PD == 1)
			pd.set_cc_plug(1);  //choose CC1 
	 else if(tcpc_status.CC2_PD == 1)
			pd.set_cc_plug(2);  //choose CC2
}

void Updata_CC_Status()
{
	if(tcpc_status.CC1_PD == 1)
		tcpc_status.CC_State = pd_status.CC1_State;
	else if(tcpc_status.CC2_PD == 1)
		tcpc_status.CC_State = pd_status.CC2_State;
	else
		DEBUG_INFO("update_CC error");
}

void Action_Attachwait_SNK()
{
	Updata_CC_Status();
	if(tcpc_status.CC_State != SNK_OPEN)//detected Rp attach
	{
		Start_CCdebounce_Timer(Timer_CC_DEBOUNCE,T_CCDEBOUNCE,Check_Vbus_to_Attached_SNK);//CC debounce 100ms
	}
	else
	{
		Stop_CCdebounce_Timer(Timer_CC_DEBOUNCE);
		tcpc_status.Now_TypeC_State = Unattached_SNK;
		pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);
		DEBUG_INFO("\r\nUnat_SNK");
	}
}

uint8_t Is_Unattached_SNK(void)
{
	if(pd_status.CC1_State!=SNK_OPEN && pd_status.CC2_State==SNK_OPEN)
	{
		tcpc_status.CC1_PD =1;
		tcpc_status.CC2_PD =0;
		tcpc_status.Now_TypeC_State = Attachwait_SNK;
		//   pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);		//added in 20221021
		DEBUG_INFO("\r\n AWaitSK");
		return 0;
	}
	else if(pd_status.CC1_State==SNK_OPEN && pd_status.CC2_State!=SNK_OPEN)
	{
		tcpc_status.CC1_PD =0;
		tcpc_status.CC2_PD =1;
		tcpc_status.Now_TypeC_State = Attachwait_SNK;
		//   pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);
		DEBUG_INFO("\r\n AWaitSK");
		return 0;
	}
	return 1;
}

void Check_Vbus_to_Attached_SNK()
{
	Stop_CCdebounce_Timer(Timer_CC_DEBOUNCE);
    if(pd.is_vbus_ok()) //Vbus_present
    {
		Jump_to_Attached_SNK();
		tcpc_status.Now_TypeC_State = Attached_SNK;
		DEBUG_INFO("\r\n Attaed_SNK");
    }
	else
	{
		DEBUG_INFO("checkingVBus");
		Start_CCdebounce_Timer(Timer_CC_DEBOUNCE,10,Check_Vbus_to_Attached_SNK);//check vbus every 10ms
	}
}

void Jump_to_Attached_SNK()
{
    Choose_PDcommunication_CC();
	//  pd_pwr.Input_Voltage_Set(4500);//Set VDPM/IDPM CHARGE_EN
	//  pd_pwr.Input_Current_Set(500); //5V 500mA
	tcpc_status.PDstate_SNK=0;     //
}

void Action_Attached_SNK()
{
    if(tcpc_status.cc_vbus_update_en==1)	
	Updata_CC_Status();
	if((tcpc_status.CC_State == SNK_OPEN) || ((!pd.is_vbus_ok()) && tcpc_status.hard_resrt_on==0 &&(tcpc_status.cc_vbus_update_en) )) //Vbus is not present or CC Open
	{
		Start_PDdebounce_Timer(Timer_PD_DEBOUNCE,T_PDDEBOUNCE,Jump_to_Unattached_SNK);
	}
	else
	{
		Stop_PDdebounce_Timer(Timer_PD_DEBOUNCE);
	}
	Vendor_PD_SNK();
}

void Jump_to_Unattached_SNK()
{
	Stop_All_PD_Timer();

	pd.vcon_off();
	tcpc_status.Now_TypeC_State = Unattached_SNK;
	pd.enable_pd_receive(EN_HARD_RESET);	//disable receive
	
	tcpc_status.CC1_PD =0;
	tcpc_status.CC2_PD =0;
	DEBUG_INFO("\r\nBack to Unattached_SNK");
}


void Typec_Init(void)
{
	tcpc_status.Now_TypeC_State= Unattached_SNK;
	pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);
}

void Stop_All_PD_Timer(void)
{
   stop_all_timer();
}
