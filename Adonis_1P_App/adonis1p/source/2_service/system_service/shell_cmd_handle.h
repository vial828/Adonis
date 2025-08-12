/**
  ******************************************************************************
  * @file    shell_cmd_handler.h
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
#ifndef __SHELL_CMD_HANDLE_H__
#define __SHELL_CMD_HANDLE_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "shell_cmd.h"
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/  
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

// shell输出
//#define CONF_SHELL_OUTPUT(...)          printf(__VA_ARGS__) //SEGGER_RTT_printf(0,__VA_ARGS__) //;
#define CONF_SHELL_OUTPUT(...)          SEGGER_RTT_printf(0,__VA_ARGS__) //;

//extern const ShellCmd_t staShellCommandTable[];

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_CMD_HANDLER_H__ */

