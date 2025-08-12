/**
  ******************************************************************************
  * @file    driver_motor.c
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
#include "driver_motor.h"
#include "sm_log.h"
#include "data_base_info.h"
#define ENABLE_DRIVER_MOTOR

#ifdef ENABLE_DRIVER_MOTOR

#include "cyhal.h"
#include "cybsp.h"
#define MOTOR_PIN               P5_0
#define MOTOR_PWM_FREQUENCY     4000

/**
  * @brief 驱动侧Motor息结构体
  */

typedef struct DriverMotorInfo_t
{
    uint8_t duty;
    uint16_t hz;
    uint16_t dutyOfRmsV;// RMS Voltage(V) 3.6V def    
    uint16_t runIndv;
    uint16_t stopIndv;
    uint16_t runTimes;
    uint32_t motorTick;
    MOTOR_MODE_Enum Paptics;
} DriverMotorInfo_t;

/* PWM object */
cyhal_pwm_t pwm_obj;

#endif

/**
  * @brief  初始化马达硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_motor_init(void)
{
#ifdef ENABLE_DRIVER_MOTOR

    cyhal_gpio_free(P5_0);
    cy_rslt_t result = CY_RSLT_SUCCESS;
    result = cyhal_pwm_init_adv(&pwm_obj, MOTOR_PIN, NC,\
                                     CYHAL_PWM_RIGHT_ALIGN, true, 0u, false, NULL);// P5_0 pwm motor en
    if(CY_RSLT_SUCCESS != result)
    {
         CY_ASSERT(false);
    }

    result = cyhal_pwm_stop(&pwm_obj);
    if(CY_RSLT_SUCCESS != result)
    {
         CY_ASSERT(false);
    }

#if 0
    /* Start the PWM */
    result = cyhal_pwm_start(&pwm_obj);
    if(CY_RSLT_SUCCESS != result)
    {
         CY_ASSERT(false);
    }
    result = cyhal_pwm_set_duty_cycle(&pwm_obj, 0, MOTOR_PWM_FREQUENCY);
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
int driver_motor_deinit(void)
{
#ifdef ENABLE_DRIVER_MOTOR
    cyhal_pwm_free(&pwm_obj);
    cyhal_gpio_init(P5_0, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
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
int driver_motor_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_MOTOR
    cy_rslt_t result = CY_RSLT_SUCCESS;
    DriverMotorInfo_t driverMotorInfo;

    // len合法判断
    if(len != sizeof(DriverMotorInfo_t))
    {
        return -1;
    }
    if(pBuf == NULL)
    {
        return -1;
    }
    memcpy(&driverMotorInfo.duty, pBuf, sizeof(DriverMotorInfo_t));
    //sm_log(SM_LOG_DEBUG, "motor_write duty = %d frequency = %d \r\n", driverMotorInfo.duty,driverMotorInfo.hz);
    if (driverMotorInfo.duty == 0 || driverMotorInfo.hz == 0) {
        /* Start the PWM */
        result = cyhal_pwm_stop(&pwm_obj);
        if(CY_RSLT_SUCCESS != result)
        {
             CY_ASSERT(false);
        }
        
    } else {
        /* Start the PWM */
        result = cyhal_pwm_start(&pwm_obj);
        if(CY_RSLT_SUCCESS != result)
        {
             CY_ASSERT(false);
        }
        
        result = cyhal_pwm_set_duty_cycle(&pwm_obj, driverMotorInfo.duty, driverMotorInfo.hz);
         if(CY_RSLT_SUCCESS != result)
         {
             CY_ASSERT(false);
         }
    }
#endif
    return 0;
}


