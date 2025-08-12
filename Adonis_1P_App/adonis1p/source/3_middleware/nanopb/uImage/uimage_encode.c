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

#include "uimage_decode.h"
#include "uimage_encode.h"

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

#define BUFFER_SIZE 256

/* Private Consts ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

uint32_t Intro_unicode_chars[10]    = {0}; //{72, 101, 108, 108, 111}; //"Hello"
uint32_t Greeting_unicode_chars[10] = {0}; //{82, 111, 98}; //"Rob"
uint32_t Outro_unicode_chars[10]    = {0}; //{71, 111, 111, 100, 98, 121,101,33}; //"Goodbye" 

uint16_t  Intro_unicode_chars_len    =0;
uint16_t  Greeting_unicode_chars_len =0;
uint16_t  Outro_unicode_chars_len    =0;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static bool encode_bytes(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    return  pb_encode_tag_for_field(stream, field) && pb_encode_string(stream, *arg, strlen(*arg));
}


/* Exported functions --------------------------------------------------------*/
/*
 * Function Name: uimage_screen_update_nanopb_encode
 * 
 * Function Description:
 * @brief  
 *
 * @param  Intro_len, Greeting_len, Outro_len
 *
 * @return true: Successful 
 *         false:Failure
 */
bool uimage_screen_update_nanopb_encode(const uint8_t *pBuf, uint16_t* len)
{
    /* This is the buffer where we will store our message. */
    bool status;

    /* Encode our message */
    {
        /* Allocate space on the stack to store the message data.
         *
         * Nanopb generates simple struct definitions for all the messages.
         * - check out the contents of simple.pb.h!
         * It is a good idea to always initialize your structures
         * so that you do not have garbage data from RAM in there.
         */
        uint8_t en_buffer[BUFFER_SIZE];
        char protocolVersion[] = "1.0";
        char key0[] = "frame0";
        char key1[] = "frame1";
        char key2[] = "frame2";

        char string_type0[] = "Intro";
        char string_type1[] = "Greeting";
        char string_type2[] = "Outro";
        // uint8_t raw_data[4]={0xff,0xff,0xff,0xff};
        
        Device device = Device_init_zero;

        device.protocolVersion.funcs.encode = encode_bytes;
        device.protocolVersion.arg = &protocolVersion;

        device.frames_count = 3;

        device.frames[0].key.funcs.encode = encode_bytes;
        device.frames[0].key.arg = &key0;
        device.frames[0].has_value = true;
        device.frames[0].value.type.funcs.encode = encode_bytes;
        device.frames[0].value.type.arg = &string_type0;
        device.frames[0].value.unicodeCharacters_count = Intro_unicode_chars_len;
        memcpy(device.frames[0].value.unicodeCharacters, Intro_unicode_chars, Intro_unicode_chars_len*4);
        // device.frames[0].value.length = 4;
        // device.frames[0].value.rawData.funcs.encode = encode_bytes;
        // device.frames[0].value.rawData.arg = &raw_data;
        
        device.frames[1].key.funcs.encode = encode_bytes;
        device.frames[1].key.arg = &key1;
        device.frames[1].has_value = true;
        device.frames[1].value.type.funcs.encode = encode_bytes;
        device.frames[1].value.type.arg = &string_type1;
        device.frames[1].value.unicodeCharacters_count = Greeting_unicode_chars_len;
        memcpy(device.frames[1].value.unicodeCharacters, Greeting_unicode_chars, Greeting_unicode_chars_len*4);
        // device.frames[1].value.length = 4;
        // device.frames[1].value.rawData.funcs.encode = encode_bytes;
        // device.frames[1].value.rawData.arg = &raw_data;

        device.frames[2].key.funcs.encode = encode_bytes;
        device.frames[2].key.arg = &key2;
        device.frames[2].has_value = true;
        device.frames[2].value.type.funcs.encode = encode_bytes;
        device.frames[2].value.type.arg = &string_type2;
        device.frames[2].value.unicodeCharacters_count = Outro_unicode_chars_len;
        memcpy(device.frames[2].value.unicodeCharacters, Outro_unicode_chars, Outro_unicode_chars_len*4);
        // device.frames[2].value.length = 4;
        // device.frames[2].value.rawData.funcs.encode = encode_bytes;
        // device.frames[2].value.rawData.arg = &raw_data;

        /* Create a stream that will write to our buffer. */
        pb_ostream_t stream = pb_ostream_from_buffer(en_buffer, sizeof(en_buffer));
        
        /* Fill in the lucky number */
        /* Now we are ready to encode the message! */
        status = pb_encode(&stream, Device_fields, &device);

        *len = stream.bytes_written;
        
        /* Then just check for any errors.. */
        if (!status)
        {
            sm_log(SM_LOG_DEBUG,"Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return 1;
        }
        sm_log(SM_LOG_DEBUG,"Encoded data: ");
        for (size_t i = 0; i < stream.bytes_written; i++) {
            sm_log(SM_LOG_DEBUG,"%02x ", en_buffer[i]);
        }
        sm_log(SM_LOG_DEBUG,"\r\n");
		memcpy(pBuf, en_buffer, *len);
    }

    return status;
}

