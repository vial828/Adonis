#ifndef  PD_TIMER_H
#define  PD_TIMER_H

#include <stdint.h>
#include <stdbool.h>


typedef void(*TIMEOUT_HANDLE)(void);

typedef struct 
{
	uint8_t  counter_enable;
	uint32_t counter_ms;
	uint32_t period;
	TIMEOUT_HANDLE timeout_action;
	
}pd_timer_type;

typedef enum
{
	Timer_CC_DEBOUNCE=0,
	Timer_PD_DEBOUNCE,
	Timer_Chunking_Notsupported,
	Timer_SENDER_RESPONSE,
	Timer_PSTransition,
  Timer_SinkWaitCap,
	Timer_Hardreset
}pd_timer_id;



typedef void(*GTIMER_INT_HANDLE)(void);

/**/
typedef enum
{
	GTIMER_SEL0=0,
	GTIMER_SEL1,
	GTIMER_SEL2,
	GTIMER_SEL3,
	GTIMER_SEL4
}timer_selection_e;


void   TIM2_Int_Init(uint16_t arr, uint16_t psc);
void   TIM3_Int_Init(uint16_t arr, uint16_t psc);
void   TIM4_Int_Init(uint16_t arr, uint16_t psc);

//��ʱ��3�жϷ������

void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);

/*
void Systick_Init(void);
void Start_Sysclock(void);
uint16_t Get_system_tick(void);     //return ms
void Stop_Sysclock(void);
*/


void Start_CCdebounce_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_CCdebounce_Timer(pd_timer_id timer_selection);

void Start_PDdebounce_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_PDdebounce_Timer(pd_timer_id timer_selection);




void Start_SenderResponseTimer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_SenderResponseTimer(pd_timer_id timer_selection);

void Start_Chunking_Notsupported_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_Chunking_Notsupported_Timer(pd_timer_id timer_selection);


void Start_PSTransitionTimer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_PSTransitionTimer(pd_timer_id timer_selection);


void Start_SinkWaitCapTimer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_SinkWaitCapTimer(pd_timer_id timer_selection);


void Start_Hard_Reset_Timer(pd_timer_id timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);
void Stop_Hard_Reset_Timer(pd_timer_id timer_selection);

//void Gtimer_Init(timer_selection_e timer_selection,uint32_t timer_interval_ms,GTIMER_INT_HANDLE int_handle);

void Gtimer_Stop(pd_timer_id ID);
void pd_timer_intial(void);
void pd_timer_add(void);
void check_pd_timer_out(void);
void stop_all_timer(void);

#endif
