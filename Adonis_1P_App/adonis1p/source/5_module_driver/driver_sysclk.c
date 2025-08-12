/**
  ******************************************************************************
  * @file    driver_sysclk.c
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
#include "driver_sysclk.h"
#include "cybsp.h"

//#define ENABLE_DRIVER_SYSCLK
#ifdef ENABLE_DRIVER_SYSCLK

#endif


/**
  * @brief  初始化系统时钟
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_sysclk_init(void)
{
#ifdef ENABLE_DRIVER_SYSCLK
        cy_rslt_t cy_result;
    /* Initialize the board support package */
    cy_result = cybsp_init();
    if (CY_RSLT_SUCCESS != cy_result)
    {
        CY_ASSERT(0);
    }
    /* Enable global interrupts */
    __enable_irq();
#endif

    return 0;
}
