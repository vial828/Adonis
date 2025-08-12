/**
  ******************************************************************************
  * @file    serial_line.h
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SERIAL_LINE_H
#define __SERIAL_LINE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/  
typedef void (*SeriallineCallback)(char *pdata,unsigned short datalen);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/**
  * @brief  Put a char into serial line
  * @param  c: input char
  * @retval 0 - invalid char,otherwise ok
  * @note   none
  */
int serial_line_input_byte(unsigned char c);

/**
  * @brief  Put chars into serial line
  * @param  pdata: input chars
  * @param  datalen: input char count
  * @retval 0 - invalid char,otherwise ok
  * @note   none
  */
int SeriallineInputBuff(unsigned char *pdata,unsigned short datalen);

/**
  * @brief  Process serial line input data
  * @param  none
  * @retval 0 - invalid char,otherwise ok
  * @note   none
  */
void serial_line_process( void );

/**
  * @brief  Set the serial line input handler
  * @param  cb: serial line event handler
  * @retval none
  * @note   none
  */
void SeriallineSetHanlder(SeriallineCallback cb);

/**
  * @brief  Serial line initial
  * @param  cb: serial line event handler
  * @retval none
  * @note   none
  */
void serial_line_init(SeriallineCallback cb);

#ifdef __cplusplus
}
#endif

#endif /* __SERIALLINE_H */

/************************ (C) COPYRIGHT BangMart *****END OF FILE****/
