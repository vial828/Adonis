/**
  ******************************************************************************
  * @file    serial_line.c
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
/* Includes ------------------------------------------------------------------*/
#include "ring_buf.h"
#include "serial_line.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifndef RX_RINGBUFF_SIZE
#define RX_RINGBUFF_SIZE 128
#endif /* SERIAL_LINE_CONF_BUFSIZE */

#if (RX_RINGBUFF_SIZE & (RX_RINGBUFF_SIZE - 1)) != 0
#error RX_RINGBUFF_SIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#endif

#ifndef IGNORE_CHAR
#define IGNORE_CHAR(c) (c == 0x0d)
#endif

#ifndef SERIAL_LINE_END
#define SERIAL_LINE_END 0x0a
#endif

#ifndef SERIAL_LINE_ENTER
#define SERIAL_LINE_ENTER 0x0d
#endif


#ifndef BS
#define BS 0x08
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct ringbuf stRxRingBuff;
static unsigned char ucRxRingBuffData[RX_RINGBUFF_SIZE];
static SeriallineCallback cbRxSeriallineHandler = 0;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#include "sm_log.h"

/**
  * @brief  Put a char into serial line
  * @param  c: input char
  * @retval 0 - invalid char,otherwise ok
  * @note   none
  */
int serial_line_input_byte(unsigned char c)
{
  static unsigned char overflow = 0; /* Buffer overflow: ignore until END */

//  if(IGNORE_CHAR(c)) windows 回车键为0d， 需要靠0d来判断输入结束
//  {
//    return 0;
//  }

  if(!overflow)
  {
    /* Add character */
    if(ringbuf_put(&stRxRingBuff, c) == 0)
    {
      /* Buffer overflow: ignore the rest of the line */
      overflow = 1;
    }
  }
  else
  {
    /* Buffer overflowed:
     * Only (try to) add terminator characters, otherwise skip */
    if(c == SERIAL_LINE_END && ringbuf_put(&stRxRingBuff, c) != 0)
    {
      overflow = 0;
    }
  }

  /* send msg to serial line process */
  // SeriallineProcess();

  return 1;
}


/**
  * @brief  Put chars into serial line
  * @param  pdata: input chars
  * @param  datalen: input char count
  * @retval 0 - invalid char,otherwise ok
  * @note   none
  */
int SeriallineInputBuff(unsigned char *pdata,unsigned short datalen)
{
    for(unsigned short index = 0; index < datalen; index++ )
    {
        serial_line_input_byte(pdata[index]);

    }
    return 1;
}


/**
  * @brief  Process serial line input data
  * @param  none
  * @retval 0 - invalid char,otherwise ok
  * @note   none
  */
void serial_line_process( void )
{
    static char buf[RX_RINGBUFF_SIZE];
    static int ptr = 0;
    int c;

    while ( 1 )
    {
        /* Fill application buffer until newline or empty */
        c = ringbuf_get( &stRxRingBuff );
        if ( c == -1 )
        {
            /* Buffer empty, wait for poll */
            return;
        }
        else
        {
            if( c == BS )
            {
                // Backspace
                if( ptr > 0 )
                {
                    ptr--;
                }
                return;
            }

            if ( (c != SERIAL_LINE_END) && (c != SERIAL_LINE_ENTER) ) // 换行符
            {
                if ( ptr < RX_RINGBUFF_SIZE - 1 )
                {
                    buf[ptr++] = (unsigned char) c;
                }
                else
                {
                    /* Ignore character (wait for EOL) */
                }
            }
            else
            {
                /* Terminate */
                buf[ptr++] = (unsigned char) '\0'; // 接收到换行符代码字符串结束，不保存换行符，并追加结束符
                /* Handle the serialline event */
                if( cbRxSeriallineHandler )
                {
                    cbRxSeriallineHandler(buf,ptr);
                }

                // Reset ptr
                ptr = 0;
            }
        }
    }
}

/**
  * @brief  Set the serial line input handler
  * @param  cb: serial line event handler
  * @retval none
  * @note   none
  */
void SeriallineSetHanlder(SeriallineCallback cb)
{
    cbRxSeriallineHandler = cb;
}

/**
  * @brief  Serial line initial
  * @param  cb: serial line event handler
  * @retval none
  * @note   none
  */
void serial_line_init(SeriallineCallback cb)
{
    SeriallineSetHanlder(cb);
    ringbuf_init(&stRxRingBuff, ucRxRingBuffData, sizeof(ucRxRingBuffData));
}
