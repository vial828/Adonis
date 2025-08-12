/**
  ******************************************************************************
  * @file    uimage_decode.c
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
  * 2024-11-18      V0.01      vincent.he@metextech.com     the first version
  * 
  ******************************************************************************
**/

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


#include "uimage_encode.h"
#include "uimage_decode.h"

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private Consts ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static bool decode_bytes(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    uint8_t* data = (uint8_t*)(*arg);
    size_t len = stream->bytes_left;
    return pb_read(stream, data, len);
    return true;
}


/* Exported functions --------------------------------------------------------*/
/*
 * Function Name: uimage_screen_update_nanopb_decode
 * 
 * Function Description:
 * @brief  
 *
 * @param  Intro_len, Greeting_len, Outro_len
 *
 * @return true: Successful 
 *         false:Failure
 */
bool uimage_screen_update_nanopb_decode(const uint8_t *pBuf, uint16_t len, 
                                        uint32_t* uImageAddr)
{
    sm_log(SM_LOG_DEBUG, "%s pBuf(%ld): \n", __FUNCTION__, len);

    if ((0 == len)||(256 < len))
    {
        return false;
    }

    #if 1
    for(uint16_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    /* This is the pBuf where we will store our message. */
    bool status;
    uint32_t address;

    /* Decode the protobuf
     * 
     */
    pb_istream_t stream = pb_istream_from_buffer(pBuf, len);

    char Intro[]="intro";
    char Greeting[]="greeting";
    char Outro[]="outro";
    char oprotocolVersion[10]={0};
    char okey[3][10]={0};
    char otype[3][10]={0};
    uint8_t oraw_data[3][4]={0};
    /* Allocate space for the decoded message. */
    
    Device odevice = Device_init_zero;
    /* Create a stream that reads from the pBuf. */

    odevice.protocolVersion.funcs.decode = decode_bytes;
    odevice.protocolVersion.arg = &oprotocolVersion;        
    odevice.frames[0].key.funcs.decode = decode_bytes;
    odevice.frames[0].key.arg = &okey[0];
    odevice.frames[0].value.type.funcs.decode = decode_bytes;
    odevice.frames[0].value.type.arg = &otype[0];
    // odevice.frames[0].value.rawData.funcs.decode = decode_bytes;
    // odevice.frames[0].value.rawData.arg = &oraw_data;

    odevice.frames[1].key.funcs.decode = decode_bytes;
    odevice.frames[1].key.arg = &okey[1];
    odevice.frames[1].value.type.funcs.decode = decode_bytes;
    odevice.frames[1].value.type.arg = &otype[1];
    // odevice.frames[1].value.rawData.funcs.decode = decode_bytes;
    // odevice.frames[1].value.rawData.arg = &oraw_data[1]; 

    odevice.frames[2].key.funcs.decode = decode_bytes;
    odevice.frames[2].key.arg = &okey[2];
    odevice.frames[2].value.type.funcs.decode = decode_bytes;
    odevice.frames[2].value.type.arg = &otype[2];
    // odevice.frames[2].value.rawData.funcs.decode = decode_bytes;
    // odevice.frames[2].value.rawData.arg = &oraw_data[2];              
    
    /* Now we are ready to decode the message. */
    status = pb_decode(&stream, Device_fields, &odevice);
    
    /* Check for errors... */
    if (!status)
    {
        sm_log(SM_LOG_DEBUG,"Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return status;
    }

    sm_log(SM_LOG_DEBUG,"Decoded data: \n");
    sm_log(SM_LOG_DEBUG,"protocolversion: %s\n", oprotocolVersion);
    sm_log(SM_LOG_DEBUG,"odevice.frames_count: %d\n", odevice.frames_count);

    if (0 == odevice.frames_count)//if no frames count, return false(nothing to do)
    {
        sm_log(SM_LOG_DEBUG, " 0 == odevice.frames_count!\n");
        return false;
    }

    for (uint8_t i=0; i<odevice.frames_count; i++)
    {
        sm_log(SM_LOG_DEBUG,"Key:   %s\n", okey[i]); 

        sm_log(SM_LOG_DEBUG,"Type:  %s\n", otype[i]);
        
        if (0 == memcmp(&Intro[0], &otype[i][0], strlen(Intro)))
        {   
            address = USER_UIMAGE_INTRO_START_ADDR;
            sm_log(SM_LOG_DEBUG,"uImageAddr:   0x%08X\n", address);

            Intro_unicode_chars_len = odevice.frames[i].value.unicodeCharacters_count;
            memcpy(&Intro_unicode_chars[0], &odevice.frames[i].value.unicodeCharacters[0], Intro_unicode_chars_len*4);

            // sm_log(SM_LOG_DEBUG,"Intro_unicode_chars: %d\n", Intro_unicode_chars_len);
            // for(uint8_t j=0; j < Intro_unicode_chars_len; j++)
            // {
            //     sm_log(SM_LOG_DEBUG,"%d ", Intro_unicode_chars[j]); 
            // }
        }
        else if (0 == memcmp(&Greeting[0], &otype[i][0], strlen(Greeting)))
        {   
            address = USER_UIMAGE_GREETING_START_ADDR;
            sm_log(SM_LOG_DEBUG,"uImageAddr:   0x%08X\n", address);

            Greeting_unicode_chars_len = odevice.frames[i].value.unicodeCharacters_count;
            memcpy(&Greeting_unicode_chars[0], &odevice.frames[i].value.unicodeCharacters[0], Greeting_unicode_chars_len*4);
        }
        else if (0 == memcmp(&Outro[0], &otype[i][0], strlen(Outro)))
        {   
            address = USER_UIMAGE_OUTRO_START_ADDR;
            sm_log(SM_LOG_DEBUG,"uImageAddr:   0x%08X\n", address);

            Outro_unicode_chars_len = odevice.frames[i].value.unicodeCharacters_count;
            memcpy(&Outro_unicode_chars[0], &odevice.frames[i].value.unicodeCharacters[0], Outro_unicode_chars_len*4);
        }

        if (uImageAddr != NULL)
        {
            *uImageAddr = address;
        }

        sm_log(SM_LOG_DEBUG,"UnicodeCharacters: ");
        for(uint8_t j=0; j < odevice.frames[i].value.unicodeCharacters_count; j++)
        {
            sm_log(SM_LOG_DEBUG,"%d ", odevice.frames[i].value.unicodeCharacters[j]); 
        }

        sm_log(SM_LOG_DEBUG,"\n");
        sm_log(SM_LOG_DEBUG,"length: %d\n", odevice.frames[i].value.length); 
    }

    return status;
}

