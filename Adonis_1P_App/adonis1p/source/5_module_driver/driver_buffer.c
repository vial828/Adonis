/**
  ******************************************************************************
  * @file    driver_buffer.c
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "driver_buffer.h"

/**
  * @brief  初始化一个指定的环形缓冲区
  * @param  pBuffer:    指向目标缓冲区
            size:       表示缓冲区分配的内存大小，单位是字节
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_buffer_init(DriverRingBuffer_t *pBuffer, uint16_t size)
{
    if(pBuffer == NULL || size == 0)
    {
        return -1;
    }

    if(pBuffer->fifo == NULL)
    {
        pBuffer->fifo = (uint8_t*)malloc(size);
        if(pBuffer->fifo == NULL)    
        {
//            printf("Malloc %d bytes failed.\r\n", size);
            return -1;
        }
    }

    pBuffer->pw = pBuffer->pr = 0;
    pBuffer->bufSize = size;
    return 0;
}

/**
  * @brief  释放一个指定的环形缓冲区
  * @param  pBuffer:    指向目标缓冲区
            size:       表示缓冲区分配的内存大小，单位是字节
  * @return 0：成功
  * @note   None
  */
int driver_buffer_deinit(DriverRingBuffer_t *pBuffer, uint16_t size)
{
    if(pBuffer == NULL || size == 0)
    {
        return -1;
    }

    if(pBuffer->fifo != NULL)
    {
        free(pBuffer->fifo);
    }

    return 0;
}

/**
  * @brief  往指定的环形缓冲区写入一个字节的数据
  * @param  pBuffer:    指向目标缓冲区
            data:       写入的数据
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_buffer_write(DriverRingBuffer_t *pBuffer, const uint8_t data)
{
    if(pBuffer == NULL || pBuffer->fifo==NULL)
    {
        return -1;
    }

    int i = (pBuffer->pw + 1) % pBuffer->bufSize;
    if(i != pBuffer->pr)
    {
        pBuffer->fifo[pBuffer->pw] = data;
        pBuffer->pw = i;
        return 0;
    }

    return -1;
}

/**
  * @brief  往指定的环形缓冲区写入多个字节的数据
  * @param  pBuffer:        指向目标缓冲区
            data_stream:    写入的数据流
            len:            写入的数据个数，单位是字节
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_buffer_write_bytes(DriverRingBuffer_t *pBuffer, const uint8_t *data_stream, uint8_t len)
{
    int i = 0;
    if(pBuffer == NULL || pBuffer->fifo==NULL)
    {
        return -1;
    }

    if(data_stream == NULL)
    {
        return -1;
    }

    if(len == 0)
    {
        return -1;
    }

    for(i=0; i<len; i++)
    {
        if(driver_buffer_write(pBuffer, data_stream[i]) != 0)
        {
            break;
        }
    }

    return i;
}

/**
  * @brief  从指定的环形缓冲区读出一个字节的数据
  * @param  pBuffer:    指向目标缓冲区
            data:       读出的数据
  * @return 0:成功，-1：失败
  * @note   None
  */
int driver_buffer_read(DriverRingBuffer_t *pBuffer, uint8_t *data)
{
    if(pBuffer == NULL || pBuffer->fifo==NULL)
    {
        return -1;
    }

    if(data == NULL)
    {
        return -1;
    }

    if(pBuffer->pr == pBuffer->pw)
    {
        return -1;
    }

    *data = pBuffer->fifo[pBuffer->pr];
    pBuffer->pr = (pBuffer->pr + 1) % pBuffer->bufSize;
    return 0;
}

/**
  * @brief  往指定的环形缓冲区读出多个字节的数据
  * @param  pBuffer:        指向目标缓冲区
            data_stream:    读出的数据流
            len:            读出的数据个数，单位是字节
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_buffer_read_bytes(DriverRingBuffer_t *pBuffer, uint8_t *data_stream, uint8_t len)
{
    int i = 0;
    if(pBuffer == NULL || pBuffer->fifo==NULL)
    {
        return -1;
    }

    if(data_stream == NULL)
    {
        return -1;
    }

    if(len == 0)
    {
        return -1;
    }

    for(i=0; i<len; i++)
    {
        if(driver_buffer_read(pBuffer, &data_stream[i]) != 0)
        {
            break;
        }
    }

    return i;
}

/**
  * @brief  清除指定的环形缓冲区
  * @param  pBuffer:        指向目标缓冲区
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_buffer_clean(DriverRingBuffer_t *pBuffer)
{
    if(pBuffer == NULL || pBuffer->fifo==NULL)
    {
        return -1;
    }

    memset(pBuffer->fifo, 0, pBuffer->bufSize);
    pBuffer->pw = pBuffer->pr = 0;
    return 0;
}
