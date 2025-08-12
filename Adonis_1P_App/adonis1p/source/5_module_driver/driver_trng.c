/**
  ******************************************************************************
  * @file    driver_trng.c
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
#include "driver_trng.h"
#include "sm_log.h"
#define ENABLE_DRIVER_TRNG

#ifdef ENABLE_DRIVER_TRNG

#include "cyhal.h"
#include "cybsp.h"
/* Macro for the maximum value of the random number generated in bits */
#define ASCII_7BIT_MASK                 (0x7F)

#define TRNG_LENGTH                 (32u)
#define ASCII_VISIBLE_CHARACTER_START   (33u) // 大于此值为可见字符

cyhal_trng_t trng_obj;

/**
  * @brief 随机数结构体
  */

typedef struct DriverMotorInfo_t
{
    uint8_t duty;
    uint16_t hz;
    uint16_t dutyOfRmsV;// 
    uint16_t runIndv;
    uint16_t stopIndv;
    uint16_t runTimes;
    uint32_t trngTick;
 //   TRNG_MODE_Enum Paptics;
} DriverMotorInfo_t;

#endif

/*******************************************************************************
* Function Name: check_range
********************************************************************************
* Summary: This function check if the generated random number is in the 
*          range of alpha-numeric, special characters ASCII codes.  
*          If not, convert to that range
*
* Parameters:
*  uint8_t
*
* Return
*  uint8_t 
*
*******************************************************************************/
uint8_t check_range(uint8_t value)
{
    if (value < ASCII_VISIBLE_CHARACTER_START)
    {
         value += ASCII_VISIBLE_CHARACTER_START;
    }
     return value;
}

/**
  * @brief  初始化真随机数发生器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_trng_init(void)
{
#ifdef ENABLE_DRIVER_TRNG

    cy_rslt_t result = CY_RSLT_SUCCESS;
    /* Initialize the TRNG generator block*/
    result = cyhal_trng_init(&trng_obj);


#endif
    return 0;
}

/**
  * @brief  去初始化真随机数发生器
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_trng_deinit(void)
{
#ifdef ENABLE_DRIVER_TRNG
#endif
    return 0;
}

/**
  * @brief  保留
  * @param  pBuf:   要写入的数据
            len:    要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_trng_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_TRNG

#endif
    return 0;
}


/**
  * @brief  生成真随机数
  * @param  pBuf:   要读入的数据
            len:    要读入数据的长度
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_trng_read(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_TRNG
    if (pBuf == NULL) {
        return -1;
    }
    if (len != TRNG_LENGTH) { // 随机数长度固定为32
        return -1;
    }
    int8_t index;
    uint32_t random_val;
    uint8_t temp_value = 0;

    for (index = 0; index < len;)
    {
        /* Generate a random 32 bit number*/
        random_val = cyhal_trng_generate(&trng_obj);
    
        uint8_t bit_position  = 0;
    
        for(int8_t j=0;j<4;j++)
        {
            /* extract byte from the bit position offset 0, 8, 16, and 24. */
            temp_value=((random_val>>bit_position )& ASCII_7BIT_MASK);
            temp_value=check_range(temp_value);
            pBuf[index++] = temp_value;
            bit_position  = bit_position  + 8;
        }
     }
#endif
    return 0;
}


