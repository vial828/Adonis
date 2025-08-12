#include "pd_timer.h"
#include "pd_global.h"

#ifndef NULL
    #define NULL ((void *)0)
#endif

extern pd_timer_type sy69410_pd_timer[7];

/*
typedef enum
{
	Timer_CC_DEBOUNCE=0,
	Timer_PD_DEBOUNCE,
	Timer_Chunking_Notsupported,
	Timer_SENDER_RESPONSE,
	Timer_PSTransition��
	Timer_SinkWaitCap,
	Timer_Hardreset
}pd_timer_id;
*/

void Gtimer_Stop(pd_timer_id ID)
{
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void Start_CCdebounce_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
	pd_timer_id ID=Timer_CC_DEBOUNCE;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}
}

void Stop_CCdebounce_Timer(pd_timer_id timer_selection)
{
	 pd_timer_id ID=Timer_CC_DEBOUNCE;
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void Start_PDdebounce_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
		pd_timer_id ID=Timer_PD_DEBOUNCE;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}
	
}

void Stop_PDdebounce_Timer(pd_timer_id timer_selection)
{
	 pd_timer_id ID=Timer_PD_DEBOUNCE;
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void Start_Chunking_Notsupported_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
	pd_timer_id ID=Timer_Chunking_Notsupported;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}
}

void Stop_Chunking_Notsupported_Timer(pd_timer_id timer_selection)
{
	 pd_timer_id ID=Timer_Chunking_Notsupported;
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}
/*
	Timer_CC_DEBOUNCE=0,
	Timer_PD_DEBOUNCE,
	Timer_Chunking_Notsupported,
	Timer_SENDER_RESPONSE,
	Timer_PSTransition��
	Timer_SinkWaitCap,
	Timer_Hardreset
*/
void Start_SenderResponseTimer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
	pd_timer_id ID=Timer_SENDER_RESPONSE;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}
}
void Stop_SenderResponseTimer(pd_timer_id timer_selection)
{
		 pd_timer_id ID=Timer_SENDER_RESPONSE;
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void Start_PSTransitionTimer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
	pd_timer_id ID=Timer_PSTransition;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}	
}

void Stop_PSTransitionTimer(pd_timer_id timer_selection)
{
		 pd_timer_id ID=Timer_PSTransition;
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void Start_SinkWaitCapTimer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
	pd_timer_id ID=Timer_SinkWaitCap;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}
}

void Stop_SinkWaitCapTimer(pd_timer_id timer_selection)
{
	 pd_timer_id ID=Timer_SinkWaitCap;
   if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void Start_Hard_Reset_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle)
{
	pd_timer_id ID=Timer_Hardreset;
	if(sy69410_pd_timer[ID].counter_enable==0)
	{
		sy69410_pd_timer[ID].counter_ms=0;
		sy69410_pd_timer[ID].period=timer_interval_ms;
		sy69410_pd_timer[ID].timeout_action=int_handle;
		sy69410_pd_timer[ID].counter_enable=1;
	}
}

void Stop_Hard_Reset_Timer(pd_timer_id timer_selection)
{
	pd_timer_id ID=Timer_Hardreset;
	if(sy69410_pd_timer[ID].counter_enable==1)
	 {
		 sy69410_pd_timer[ID].counter_enable=0;
		 sy69410_pd_timer[ID].counter_ms=0;
	 }
}

void pd_timer_intial(void)
{
	for(uint8_t i=0;i<7;i++)
	{
		sy69410_pd_timer[i].counter_enable=0;
		sy69410_pd_timer[i].counter_ms=0;
	}
}

void pd_timer_add(void)
{
	for(uint8_t i=0; i<7; i++) {
		if(sy69410_pd_timer[i].counter_enable==1) {
			if(sy69410_pd_timer[i].counter_ms<sy69410_pd_timer[i].period) {
				sy69410_pd_timer[i].counter_ms++;
			} else {
			//do nothing
			}
		} else {
			// sy69410_pd_timer[i].counter_ms=0;
		}
	}
}

void check_pd_timer_out(void)
{
	for(uint8_t i=0;i<7;i++) {
		if(sy69410_pd_timer[i].counter_enable==1) {
			if(sy69410_pd_timer[i].counter_ms>=sy69410_pd_timer[i].period) {
				sy69410_pd_timer[i].counter_enable=0;
				sy69410_pd_timer[i].counter_ms=0;
				DEBUG_INFO("!!!TIME_OUT %d\r\n",i);
				if(sy69410_pd_timer[i].timeout_action!=NULL)
				sy69410_pd_timer[i].timeout_action();
			}
		}
	}
}


void stop_all_timer(void)
{
	for(uint8_t i=0;i<7;i++)
	{
		sy69410_pd_timer[i].counter_enable=0;
		sy69410_pd_timer[i].counter_ms=0;
		sy69410_pd_timer[i].timeout_action=0;
	}
}

		
