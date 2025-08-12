/**
  ******************************************************************************
  * @file    sm_log.c
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

#include "stdarg.h"
#include <stdio.h>
#include "platform_io.h"
#include "sm_log.h"
//#include "shell_cmd.h"
#include "system_status.h"
#include "protocol_usb.h"

#define LOG_BUF_SIZE 512 //250

SmLogLevel_t g_smLogLevel = SM_LOG_DEBUG;//SM_LOG_DEBUG;//SM_LOG_NOTICE;//SM_LOG_ERR

uint8_t smLogBuff[LOG_BUF_SIZE];
SemaphoreHandle_t xSemaphoreSmLog;

/**
  * @brief  设置日志等级
  * @param  logLevel: 日志等级
  * @return None
  * @note   None
  */
void set_log_level(SmLogLevel_t logLevel)
{
    g_smLogLevel = logLevel;
}

/**
  * @brief  获取当前日志等级
  * @param  None
  * @return g_smLogLevel: 返回当前日志等级
  * @note   None
  */
SmLogLevel_t get_log_level(void)
{
    return g_smLogLevel;
}

/**
  * @brief  创建日志打印互斥信号量
  * @param  fNone
  * @return None
  * @note   None
  */
void sm_log_semphore_mutex_create(void)
{
    xSemaphoreSmLog = xSemaphoreCreateMutex();
}

/**
  * @brief  日志打印函数
  * @param  format: 待打印的格式化字符串
  * @return None
  * @note   None
  */
void sm_log(SmLogLevel_t logLevel, const char *format, ...)
{

#if 1
    uint32_t length;
    //uint8_t tmpSmLogBuff[LOG_BUF_SIZE]; //放此处，打印浮点数，程序会跑飞，先暂时定义全局变量
    va_list args;
    // 大于当前等级的日志信息不打印
    if ((logLevel > g_smLogLevel) || (logLevel == SM_LOG_OFF)) {
        return;
    }
    if (get_system_status() == SLEEP) { // 休眠后 串口已去初始化，不能再发送数据
        return;
    }

    if (get_rdp_on_mode_no_save_flash() == 1) {
        return;
    }

    xSemaphoreTake(xSemaphoreSmLog,portMAX_DELAY);
    va_start(args, format);
    length = vsnprintf((char*)smLogBuff, LOG_BUF_SIZE, (char*)format, args);
    va_end(args);
    if (length >= LOG_BUF_SIZE) {
        xSemaphoreGive(xSemaphoreSmLog);
        return;
    }

#if (defined JLINK_SHELL_OUT)
    SEGGER_RTT_vprintf(0, format, &args);
#else
    ptIoDev usbDev = io_dev_get_dev(DEV_USB);
    usbDev->write( (unsigned char*)smLogBuff, length);
#endif
    xSemaphoreGive(xSemaphoreSmLog);
#endif
#if 0
    uint32_t length;
    uint8_t tmpSmLogBuff[LOG_BUF_SIZE]; //放此处，打印浮点数，程序会跑飞，先暂时定义全局变量
    va_list args;
    // 大于当前等级&&等于SM_LOG_OFF 的日志信息不打印
    if ((logLevel > g_smLogLevel) || (logLevel == SM_LOG_OFF)) {
        return;
    }
    if (get_system_status() == SLEEP) { // 休眠后 串口已去初始化，不能再发送数据
        return;
    }
    if (get_shipping_mode() == 1) { // 船运模式不打印日志
        return;
    }
    if (get_rdp_on_mode() == 1) {
        return;
    }
    tmpSmLogBuff[0] = 1;
    xSemaphoreTake(xSemaphoreSmLog,portMAX_DELAY);
    va_start(args, format);
    length = vsnprintf((char*)&tmpSmLogBuff[1], LOG_BUF_SIZE, (char*)format, args);
    va_end(args);
    if (length >= LOG_BUF_SIZE) {
        xSemaphoreGive(xSemaphoreSmLog);
        return;
    }
    ptIoDev usbDev = io_dev_get_dev(DEV_USB);
    int tempTxLen = frame_create_usb(smLogBuff,PROC_REQUEST_LOG,tmpSmLogBuff,length + 1);
    if(tempTxLen > 0)
    {
        usbDev->write(smLogBuff,(uint16_t)tempTxLen);
    }
    xSemaphoreGive(xSemaphoreSmLog);
#endif
}

void sm_log2(SmLogLevel_t logLevel, const char *format, ...)
{
    uint32_t length;
    va_list args;
    // 大于当前等级的日志信息不打印
    if (logLevel > g_smLogLevel) {
        return;
    }
    if (get_system_status() == SLEEP) { // 休眠后 串口已去初始化，不能再发送数据
        return;
    }

    if (get_rdp_on_mode_no_save_flash() == 1) {
        return;
    }

#if 1
    xSemaphoreTake(xSemaphoreSmLog,portMAX_DELAY);
    va_start(args, format);
    length = vsnprintf((char*)smLogBuff, LOG_BUF_SIZE, (char*)format, args);
    va_end(args);
    if (length >= LOG_BUF_SIZE) {
        xSemaphoreGive(xSemaphoreSmLog);
        return;
    }
#if (defined JLINK_SHELL_OUT)
    SEGGER_RTT_vprintf(0, format, &args);
#else
    ptIoDev usbDev = io_dev_get_dev(DEV_USB);
    usbDev->write( (unsigned char*)&smLogBuff, length);
#endif
#endif

    xSemaphoreGive(xSemaphoreSmLog);
}

extern const char cErrCodeKey[ERRCODE_MAX_LEN + 1][35];
extern const uint16_t cErrCodeKeyNum[ERRCODE_MAX_LEN];

bool procotol_errcode_control(void)
{
    uint16_t *pErrcodeHandle = get_errCode_val_info_handle();
    ptIoDev usbDev = io_dev_get_dev(DEV_USB);
    int j = 0;
    // 发送版本号
    memcpy(&smLogBuff[j],&cErrCodeKey[0], strlen(&cErrCodeKey[0]));
    j += strlen(&cErrCodeKey[0]);
    memcpy(&smLogBuff[j],"\r\n", strlen("\r\n"));
    j += strlen("\r\n");
    // 发送其他错误值
    for (int i = 0; i < ERRCODE_MAX_LEN; i++) {
        if (j > 200) {
            usbDev->write(smLogBuff,(uint16_t)j);
            sys_task_security_ms_delay(10, TASK_USB);
            j = 0;
        }
        memcpy(&smLogBuff[j],&cErrCodeKey[i + 1], strlen(&cErrCodeKey[i + 1]));
        j += strlen(&cErrCodeKey[i + 1]);
        uint8_t currentErrVal= 0;
        if (find_error_even(cErrCodeKeyNum[i]) != 0) {
            currentErrVal = 1;
        }
        int tmpLen = sprintf(&smLogBuff[j], "    %d, %d\r\n", pErrcodeHandle[i], currentErrVal);
        j += tmpLen;
        if (i == ERRCODE_MAX_LEN - 1) {
            usbDev->write(smLogBuff,(uint16_t)j);
            sys_task_security_ms_delay(10, TASK_USB);
            break;
        }
    }
    return true;
}

