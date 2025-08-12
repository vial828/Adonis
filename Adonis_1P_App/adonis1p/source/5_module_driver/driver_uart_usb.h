/**
  ******************************************************************************
  * @file    driver_uart_usb.h
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

#ifndef __DRIVER_UART_USB_H
#define __DRIVER_UART_USB_H

#include "stdint.h"
#include "cyhal.h"
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "public_typedef.h"

#include "cycfg_pins.h"
#include "cyhal_uart.h"
//#include "driver_pin.h"

//#include "driver_buffer.h"
#include "public_typedef.h"


#define UART_USB_TIMEOUT		2u// ms
#define UART_USB_TIMEOUT_MAX	50u
#define UART_USB_LEN_MAX		512u//注:宏定义用于设置收发缓存的长度是2的幂,当缓冲区的数据长度是2的幂的时候，可以省去求余的运算，可以提高代码的执行速度。
#define UART_USB_RX_LEN_MAX		(1024u + 11)// 升级UI
#define UART_USB_LEN_MIN		3u//最小的BUFFER长度，随便定一个


typedef enum
{
    USB_RX                = BIT0,
    USB_LOG               = BIT1,

    USB_TYPE_NUM          = 2,
}UsbTypeOpte;

int driver_usb_init(void);
int driver_usb_deinit(void);
int driver_usb_read(uint8_t *pBuf, uint16_t len);
int driver_usb_write(uint8_t *pBuf, uint16_t len);
bool get_driver_status(void);

#endif

