/**
  ******************************************************************************
  * @file    driver_tick.c
  * @author  xuhua.huang@metextech.com
  * @date    2024/03/013
  * @version V0.01
  * @brief   Brief description.
  *
  *   Detailed description starts here.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 SMOORE TECHNOLOGY CO.,LTD.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  * Change Logs:
  * Date            Version    Author                       Notes
  * 2024-03-013     V0.01      xuhua.huang@metextech.com    the first version
  *
  ******************************************************************************
  */

#include "stdio.h"
#include "string.h"
#include "driver_tick.h"

#define ENABLE_DRIVER_TICK

#ifdef ENABLE_DRIVER_TICK

#include "cyhal.h"
#include "cybsp.h"
//#include "cy_retarget_io.h"

#endif
/* clock value in Hz  */
#define MS_TIMER_CLOCK_HZ          (1000000)
/* period value */
#define MS_TIMER_PERIOD           (999)

static volatile uint32_t ms_tick = 0;
static volatile uint32_t us100_tick = 0;
static volatile uint32_t us10_tick = 0;

cyhal_timer_t ms_timer;

void ms_timer_init(void);
static void isr_timer_ms(void *callback_arg, cyhal_timer_event_t event);

/**
  * @brief  初始化ms定时器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_mstick_init(void)
{
    ms_timer_init();
    return 0;
}

/**
  * @brief  初始化100us定时器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_100ustick_init(void)
{
    return 0;
}

/**
  * @brief  初始化10us定时器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */

int driver_10ustick_init(void)
{
    return 0;
}

/**
  * @brief  去初始化ms定时器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_mstick_deinit(void)
{
#ifdef ENABLE_DRIVER_TICK
    cyhal_timer_free(&ms_timer);
#endif
    return 0;
}

/**
  * @brief  去初始化100us定时器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_100ustick_deinit(void)
{
#ifdef ENABLE_DRIVER_TICK
#endif
    return 0;
}

/**
  * @brief  去初始化10us定时器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_10ustick_deinit(void)
{
#ifdef ENABLE_DRIVER_TICK
#endif
    return 0;
}

uint32_t get_ms_tick(void)
{
    return ms_tick;
}

uint32_t get_us100_tick(void)
{
    return us100_tick;
}

uint32_t get_us10_tick(void)
{
    return us10_tick;
}


/**
  * @brief  读取ms计时器数值
  * @param  pBbuf:  要读入的数据
            len:    要读入数据的长度
  * @return 返回ms计时器值
  * @note
  */
int driver_mstick_read(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_TICK
    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
//    if(len==0)  return -1;
//    if(buf == NULL)     return -1;
    return (int)get_ms_tick();
#else
    return 0;
#endif
}

/**
  * @brief  读取100us计时器数值
  * @param  pBbuf:  要读入的数据
            len:    要读入数据的长度
  * @return 返回100us计时器值
  * @note
  */
int driver_100ustick_read(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_TICK
    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
//    if(len==0)  return -1;
//    if(buf == NULL)     return -1;
	return (int)get_us100_tick();
#else
    return 0;
#endif
}

/**
  * @brief  读取10uss计时器数值
  * @param  pBbuf:  要读入的数据
            len:    要读入数据的长度
  * @return 返回10us计时器值
  * @note
  */
int driver_10ustick_read(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_TICK
    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
//    if(len==0)  return -1;
//    if(buf == NULL)     return -1;
	return (int)get_us10_tick();
#endif
    return 0;
}


void ms_timer_init(void)
{
   cy_rslt_t result;
#if 0
   /* Initialize the User LED */
   result = cyhal_gpio_init(P1_5, CYHAL_GPIO_DIR_OUTPUT,
                            CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
   
   /* GPIO init failed. Stop program execution */
   if (result != CY_RSLT_SUCCESS)
   {
       CY_ASSERT(0);
   }
#endif

   ms_tick = 0; //休眠唤醒时清零

   const cyhal_timer_cfg_t ms_timer_cfg =
   {
       .compare_value = 0,                 /* Timer compare value, not used */
       .period = MS_TIMER_PERIOD,   /* Defines the timer period */
       .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
       .is_compare = false,                /* Don't use compare mode */
       .is_continuous = true,              /* Run timer indefinitely */
       .value = 0                          /* Initial value of counter */
   };

   /* Initialize the timer object. Does not use input pin ('pin' is NC) and
    * does not use a pre-configured clock source ('clk' is NULL). */
   result = cyhal_timer_init(&ms_timer, NC, NULL);

   /* timer init failed. Stop program execution */
   if (result != CY_RSLT_SUCCESS)
   {
       CY_ASSERT(0);
   }

   /* Configure timer period and operation mode such as count direction,
      duration */
   cyhal_timer_configure(&ms_timer, &ms_timer_cfg);

   /* Set the frequency of timer's clock source */
   cyhal_timer_set_frequency(&ms_timer, MS_TIMER_CLOCK_HZ);

   /* Assign the ISR to execute on timer interrupt */
   cyhal_timer_register_callback(&ms_timer, isr_timer_ms, NULL);

   /* Set the event on which timer interrupt occurs and enable it */
   cyhal_timer_enable_event(&ms_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT,
                             7, true);

   /* Start the timer with the configured settings */
   cyhal_timer_start(&ms_timer);
//   UsbLogPrintf("ms_timer_init complete!\r\n");

//   sm_log(SM_LOG_INFO, "%s complete!\r\n", __FUNCTION__);
}

/**
  * @brief  ms定时器中断函数
  * @param  callback_arg:  
            event:    
  * @return 
  * @note
  */
static void isr_timer_ms(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;
    ms_tick++;
}

