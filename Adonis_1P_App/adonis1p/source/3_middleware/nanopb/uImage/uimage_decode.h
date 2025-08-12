/**
  ******************************************************************************
  * @file    app_bt_char.h
  * @author  vincent.he@metextech.com 
  * @date    2024/08/12
  * @version V0.01
  * @brief   Brief description.
  *
  * Description: This file consists of the bt char functions that will help
  *              debugging and developing the applications easier with much
  *              more meaningful information.
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
  * 2024-08-12      V0.01      vincent.he@metextech.com     the first version
  * 
  ******************************************************************************
**/


#ifndef __UIMAGE_DECODE_H__
#define __UIMAGE_DECODE_H__

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "uimage.pb.h"
#include "stdlib.h"
#include "sm_log.h"

/* Public macro ------------------------------------------------------------*/
//Screen Upgrade 384KB
#define UIMAGE_START_ADDR                (0x1E20000)
#define USER_UIMAGE_INTRO_START_ADDR     (0x1E20000)
#define USER_UIMAGE_GREETING_START_ADDR  (0x1E40000)
#define USER_UIMAGE_OUTRO_START_ADDR     (0x1E60000)
#define UIMAGE_END_ADDR                  (0x1E80000)

/* Private define ------------------------------------------------------------*/


/******************************************************************************
 *                                Constants
 ******************************************************************************/


/****************************************************************************
 *                              FUNCTION DECLARATIONS
 ***************************************************************************/

bool uimage_screen_update_nanopb_decode(const uint8_t *pBuf, uint16_t len,
                                        uint32_t* uImageAddr);










#endif      /*__UIMAGE_DECODE_H__ */


/* [] END OF FILE */
