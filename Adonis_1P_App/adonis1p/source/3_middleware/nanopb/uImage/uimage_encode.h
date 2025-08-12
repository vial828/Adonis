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


#ifndef __UIMAGE_ENCODE_H__
#define __UIMAGE_ENCODE_H__

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


/* Private define ------------------------------------------------------------*/


extern uint32_t Intro_unicode_chars[10];
extern uint32_t Greeting_unicode_chars[10];
extern uint32_t Outro_unicode_chars[10];

extern uint16_t  Intro_unicode_chars_len;
extern uint16_t  Greeting_unicode_chars_len;
extern uint16_t  Outro_unicode_chars_len;

/******************************************************************************
 *                                Constants
 ******************************************************************************/


/****************************************************************************
 *                              FUNCTION DECLARATIONS
 ***************************************************************************/

bool uimage_screen_update_nanopb_encode(const uint8_t *pBuf, uint16_t* len);










#endif      /*__UIMAGE_ENCODE_H__ */


/* [] END OF FILE */
