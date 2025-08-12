/**
  ******************************************************************************
  * @file    shell_cmd.h
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
#ifndef __SHELL_CMD_H
#define __SHELL_CMD_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "sm_log.h"
#include "SEGGER_RTT.h"

/* Exported types ------------------------------------------------------------*/
 /* Command handling function type */
 typedef int (ShellCommandFunc)(char *args,unsigned short arglen);

 typedef struct
 {
   const char *name;
   ShellCommandFunc *func;
   const char *help;
 }ShellCmd_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#ifndef NULL
#define NULL 0
#endif

// Shell command operation code
#define SHELL_CMD_OPS_OK            0
#define SHELL_CMD_OPS_ERR           1

#if (defined JLINK_SHELL_OUT)
#define SHELL_OUTPUT(...)           SEGGER_RTT_printf(0,__VA_ARGS__)
#elif (defined ENABLE_PCTOOL_SHELL)
#define SHELL_OUTPUT(...)           sm_log(SM_LOG_DEBUG, __VA_ARGS__)
#else
#define SHELL_OUTPUT(...)
#endif

/* Private macro -------------------------------------------------------------*/
/* Helper macros to parse arguments */
#define SHELL_ARGS_INIT(args, next_args) (next_args) = (args);

#define SHELL_ARGS_NEXT(args, next_args) do {   \
    (args) = (next_args);                       \
    if((args) != NULL) {                        \
      if(*(args) == '\0') {                     \
        (args) = NULL;                          \
      } else {                                  \
        (next_args) = strchr((args), ' ');      \
        if((next_args) != NULL) {               \
          *(next_args) = '\0';                  \
          (next_args)++;                        \
        }                                       \
      }                                         \
    } else {                                    \
      (next_args) = NULL;                       \
    }                                           \
  } while(0)

#define ASSERT_PARAM(ARG,ERR_MSG)  do{ \
    if(ARG == NULL ){ SHELL_OUTPUT(ERR_MSG); return SHELL_CMD_OPS_ERR; } \
    }while(0)

/* Exported functions --------------------------------------------------------*/
 /**
   * @brief  Get next arg
   * @param  next_args: input arg
   * @retval return next valid arg
   * @note   none
   */
char* ShellGetNextArgs(char **next_args);

 /**
   * @brief  Handle shell command
   * @param  args: input command
   * @param  arglen: command length
   * @retval none
   * @note   none
   */
 void shell_command_handle(char *args,unsigned short arglen);

 /**
   * @brief  Register a shell command handler table to handle shell commands
   * @param  pstShellCmd: shell command handler table
   * @retval none
   * @note   none
   */
 void shell_register_handler(const ShellCmd_t *pstShellCmd);

 /**
   * @brief  Unegister shell command handlers
   * @param  none
   * @retval none
   * @note   none
   */
 void ShellUnregisterHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __SHELLCMD_H */

/************************ (C) COPYRIGHT BangMart *****END OF FILE****/
