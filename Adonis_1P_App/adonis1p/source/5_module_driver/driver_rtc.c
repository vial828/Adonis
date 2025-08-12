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
  * 2024-07-30      V0.01      vincent.he@metextech.com    the first version
  *
  ******************************************************************************
  */

#include "stdio.h"
#include "string.h"
#include "driver_uart_usb.h"
#include "driver_rtc.h"
#include "driver_fuel_gauge.h"

#define ENABLE_DRIVER_RTC

#ifdef ENABLE_DRIVER_RTC
#include <time.h>
#include "cyhal.h"
#include "cybsp.h"
//#include "cy_retarget_io.h"
#endif


/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private Consts -------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
cyhal_rtc_t rtc_obj;

/* Private variables ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

void rtc_alarm_call_back_fun(void *callback_arg, cyhal_rtc_event_t event)
{
	//sm_log(SM_LOG_NOTICE, "1111!\r\n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	TaskHandle_t *temp_handle;
	temp_handle = get_task_system_handle();
	if (false == get_driver_status()) 
	{
		vTaskNotifyGiveFromISR(*temp_handle, &xHigherPriorityTaskWoken);
	}	
	bq27426_set_smooth_sync(TRUE);
}
/*******************************************************************************
* Function Name: rtc_set_new_timestamp
********************************************************************************
* Summary:
*  This functions takes the user input and sets the new date and time.
*
* Parameter:
*  uint32_t timeout_ms : Maximum allowed time (in milliseconds) for the
*  function
*
* Return :
*  void
*******************************************************************************/
//static int rtc_set_new_timestamp(void)
//{
//    struct tm *new_tm;
//	time_t rawtime;
//	rawtime = 1723075688;					// 2024 08 08 8:8:8
//
//	new_tm = localtime(&rawtime);
//
//	return cyhal_rtc_write(&rtc_obj, new_tm);
//}

/* Public function prototypes -----------------------------------------------*/
/****************************************************************************************
* @brief   rtc init
* @param
* @return  true /false
* @note
****************************************************************************************
**/
int driver_rtc_init(void)
{    
    cy_rslt_t rslt = CY_RSLT_SUCCESS;
    static uint8_t rtc_init_flag = 0;

#ifdef ENABLE_DRIVER_RTC
    if (0 == rtc_init_flag)
    {
        rtc_init_flag = 1;
        /* Initialize RTC */
        rslt = cyhal_rtc_init(&rtc_obj);

		cyhal_rtc_register_callback((&rtc_obj), rtc_alarm_call_back_fun, NULL);
		cyhal_rtc_enable_event((&rtc_obj), CYHAL_RTC_ALARM, 6, 1);
		
        if (CY_RSLT_SUCCESS != rslt)
        {
            sm_log(DRV_RTC_LOG_INFO, "%s Failure!\r\n", __FUNCTION__);
        }
        else
        {
//            rtc_set_new_timestamp();
            sm_log(DRV_RTC_LOG_INFO, "%s Successful!\r\n", __FUNCTION__);
        }
    }
#endif
    return rslt;
}

/****************************************************************************************
* @brief   rtc deinit
* @param
* @return  true /false
* @note
****************************************************************************************
**/
int driver_rtc_deinit(void)
{
#ifdef ENABLE_DRIVER_RTC
    // cyhal_rtc_free(&rtc_obj);
#endif
    return 0;
}

/****************************************************************************************
* @brief   rtc read timestamp
* @param
* @return  true /false
* @note
****************************************************************************************
**/
int driver_rtc_read(uint8_t *pBuf, uint16_t len)
{
	cy_rslt_t rslt = CY_RSLT_SUCCESS;

#ifdef ENABLE_DRIVER_RTC
    uint32_t r_timestamp;
	uint32_t *pData;
    struct tm date_time;

    if (NULL == pBuf || len != 4) 
    {
        return -1;
    }

    rslt = cyhal_rtc_read(&rtc_obj, &date_time);
    if (CY_RSLT_SUCCESS == rslt)
    {
		r_timestamp = mktime(&date_time);
		pData = (uint32_t *)pBuf;
		*pData = r_timestamp;
//		sm_log(DRV_RTC_LOG_INFO, "timestamp = %lu\r\n", r_timestamp);
    }
    else
    {
        sm_log(DRV_RTC_LOG_INFO, "%s Failure!\r\n", __FUNCTION__);
    }
#endif
    return rslt;
}

/****************************************************************************************
* @brief   rtc write timestamp
* @param
* @return  true /false
* @note
****************************************************************************************
**/
int driver_rtc_write(uint8_t *pBuf, uint16_t len)
{
	cy_rslt_t rslt = CY_RSLT_SUCCESS;

#ifdef ENABLE_DRIVER_RTC
    time_t rawtime;
    struct tm *new_tm;
	uint32_t *pData;

    if (NULL == pBuf || len != 4) 
    {
        return -1;
    }

	pData = (uint32_t *)pBuf;
	rawtime = *pData;
	new_tm = localtime(&rawtime);

	rslt = cyhal_rtc_write(&rtc_obj, new_tm);

    if (CY_RSLT_SUCCESS == rslt)
    {
        sm_log(DRV_RTC_LOG_INFO, "\rRTC time updated\r\n\n");
    }
    else
    {
        sm_log(DRV_RTC_LOG_INFO, "%s Failure!\r\n", __FUNCTION__);
    }
#endif
    return rslt;
}
