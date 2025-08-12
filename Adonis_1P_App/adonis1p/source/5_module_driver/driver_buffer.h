/**
  ******************************************************************************
  * @file    driver_buffer.h
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

#ifndef __DRIVER_BUFFER_H
#define __DRIVER_BUFFER_H

#include "stdint.h"

/**
  * @brief 环形缓冲区结构体
  */
typedef struct DriverRingBuffer_t
{
    uint8_t *fifo;
    uint16_t pw;
    uint16_t pr;
    uint16_t bufSize;
} DriverRingBuffer_t;

int driver_buffer_init(DriverRingBuffer_t *pBuffer, uint16_t size);
int driver_buffer_deinit(DriverRingBuffer_t *pBuffer, uint16_t size);
int driver_buffer_write(DriverRingBuffer_t *pBuffer, const uint8_t data);
int driver_buffer_write_bytes(DriverRingBuffer_t *pBuffer, const uint8_t *data_stream, uint8_t len);
int driver_buffer_read(DriverRingBuffer_t *pBuffer, uint8_t *data);
int driver_buffer_read_bytes(DriverRingBuffer_t *pBuffer, uint8_t *data_stream, uint8_t len);
int driver_buffer_clean(DriverRingBuffer_t *pBuffer);

#endif

