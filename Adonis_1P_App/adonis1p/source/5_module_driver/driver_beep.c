/*
 * driver_beep.c
 *
 *  Created on: 2024年5月9日
 *      Author: S1122385
 */
#include "driver_beep.h"
#include "sm_log.h"



#define ENABLE_DRIVER_BEEP

#ifdef ENABLE_DRIVER_BEEP

#include "cyhal.h"
#include "cybsp.h"
#define BEEP_PIN               P13_1
#define BEEP_PWM_FREQUENCY     4000

/**
  * @brief 驱动侧BEEP息结构体
  */

typedef struct DriverBeepInfo_t
{
    uint8_t duty;
    uint16_t hz;
        
    uint16_t runIndv;
    uint16_t stopIndv;
    uint16_t runTimes;
    uint32_t BeepTick;
} DriverBeepInfo_t;

/* PWM object */
cyhal_pwm_t beep_pwm_obj;

#endif

/**
  * @brief  初始化马达硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_beep_init(void)
{
#ifdef ENABLE_DRIVER_BEEP
    cyhal_gpio_free(P13_1);
    cyhal_gpio_free(P13_0);
    cy_rslt_t result = CY_RSLT_SUCCESS;
    result = cyhal_pwm_init_adv(&beep_pwm_obj, P13_0,BEEP_PIN,\
                                     CYHAL_PWM_RIGHT_ALIGN, true, 0u, false, NULL);// P13_1 pwm Beep en
    if(CY_RSLT_SUCCESS != result)
    {
         CY_ASSERT(false);
    }
    result = cyhal_pwm_stop(&beep_pwm_obj);
    if(CY_RSLT_SUCCESS != result)
    {
         CY_ASSERT(false);
    }
#if 0
    /* Start the PWM */
    result = cyhal_pwm_start(&beep_pwm_obj);
    if(CY_RSLT_SUCCESS != result)
    {
         CY_ASSERT(false);
    }
    result = cyhal_pwm_set_duty_cycle(&beep_pwm_obj, 100, BEEP_PWM_FREQUENCY);
     if(CY_RSLT_SUCCESS != result)
     {
         CY_ASSERT(false);
     }
#endif
#endif
    return 0;
}

/**
  * @brief  去初始化马达硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_beep_deinit(void)
{
#ifdef ENABLE_DRIVER_BEEP
    cyhal_pwm_free(&beep_pwm_obj);
    cyhal_gpio_init(P13_0, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_init(P13_1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
#endif
    return 0;
}

/**
  * @brief  控制马达
  * @param  pBuf:   要写入的数据
            len:    要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_beep_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_BEEP
    cy_rslt_t result = CY_RSLT_SUCCESS;
    DriverBeepInfo_t driverBeepInfo;

    // len合法判断
    if(len != sizeof(DriverBeepInfo_t))
    {
        return -1;
    }
    if(pBuf == NULL)
    {
        return -1;
    }
    memcpy(&driverBeepInfo.duty, pBuf, sizeof(DriverBeepInfo_t));
   // sm_log(SM_LOG_DEBUG, "Beep_write duty = %d frequency = %d \r\n", driverBeepInfo.duty,driverBeepInfo.hz);
    if (driverBeepInfo.duty == 100 || driverBeepInfo.hz == 0) {
        /* Stop the PWM */
        result = cyhal_pwm_stop(&beep_pwm_obj);
        if(CY_RSLT_SUCCESS != result)
        {
             CY_ASSERT(false);
        }
    } else {
        /* Start the PWM */
        result = cyhal_pwm_start(&beep_pwm_obj);
        if(CY_RSLT_SUCCESS != result)
        {
             CY_ASSERT(false);
        }
        
        result = cyhal_pwm_set_duty_cycle(&beep_pwm_obj, driverBeepInfo.duty, driverBeepInfo.hz);
         if(CY_RSLT_SUCCESS != result)
         {
             CY_ASSERT(false);
         }
    }
#endif
    return 0;
}

















