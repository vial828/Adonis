/**
  ******************************************************************************
  * @file    sm_log.h
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

#ifndef __SM_LOG_H
#define __SM_LOG_H

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


//#define ENABLE_PCTOOL_SHELL
//#define JLINK_SHELL_OUT

typedef enum SmLogLevel_t {
    SM_LOG_OFF = 0,     /**< Do not print log messages */
    SM_LOG_ERR,         /**< Print log message if run-time level is <= SM_LOG_ERR       */
    SM_LOG_WARNING,     /**< Print log message if run-time level is <= SM_LOG_WARNING   */
    SM_LOG_NOTICE,      /**< Print log message if run-time level is <= SM_LOG_NOTICE    */
    SM_LOG_INFO,        /**< Print log message if run-time level is <= SM_LOG_INFO      */
    SM_LOG_DEBUG,       /**< Print log message if run-time level is <= SM_LOG_DEBUG     */
    SM_LOG_MAX
} SmLogLevel_t;

void sm_log(SmLogLevel_t logLevel, const char *format, ...);
void sm_log2(SmLogLevel_t logLevel, const char *format, ...);
void sm_log_semphore_mutex_create(void);
void set_log_level(SmLogLevel_t logLevel);
SmLogLevel_t get_log_level(void);

#endif

