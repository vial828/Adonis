/**
  ******************************************************************************
  * @file    shell_cmd.c
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
#include <string.h>
#include <stdlib.h>
#include "serial_line.h"
#include "shell_cmd.h"

#include "log.h"
#define LOG_MODULE "SHELL"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const ShellCmd_t *pastShellCommandHandlerTable = NULL;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Get next arg
  * @param  next_args: input arg
  * @retval return next valid arg
  * @note   none
  */
inline char* ShellGetNextArgs(char **next_args)
{
    char *args = *next_args;
    if(args != NULL)
    {
      if(*args == '\0')
      {
        args = NULL;
      }
      else
      {
        *next_args = strchr((args), ' ');
        if(*next_args != NULL)
        {
          **next_args = '\0';
          (*next_args)++;
        }
      }
    }
    else
    {
      *next_args = NULL;
    }
    return args;
 }

/**
  * @brief  Handle shell command
  * @param  args: input command
  * @param  arglen: command length
  * @retval none
  * @note   none
  */
void shell_command_handle(char *args,unsigned short arglen)
{
    char *next_args;
    char *name;

    SHELL_ARGS_INIT(args, next_args);
    /* Get and parse argument: module name */
    //SHELL_ARGS_NEXT(args, next_args);
    args = ShellGetNextArgs(&next_args);
    name = args;
    const ShellCmd_t *pstShellCmd = pastShellCommandHandlerTable;
    if ( name && pstShellCmd )
    {
        for ( unsigned char index = 0; index < 250; index++ )
        {
            if ( pstShellCmd->name == NULL )
            {
                // End of table
                break;
            }
            if ( !strcmp( pstShellCmd->name, name ) )
            {
                // find the math command
                if ( pstShellCmd->func )
                {
                    pstShellCmd->func( next_args, arglen - strlen( args ) );
                    break;
                }
            }
            pstShellCmd++;
        }
    }
    // New line for input
    SHELL_OUTPUT("\n>");
}

/**
  * @brief  Register a shell command handler table to handle shell commands
  * @param  pstShellCmd: shell command handler table
  * @retval none
  * @note   none
  */
void shell_register_handler(const ShellCmd_t *pstShellCmd)
{
    pastShellCommandHandlerTable = pstShellCmd;
}

/**
  * @brief  Unegister shell command handlers
  * @param  none
  * @retval none
  * @note   none
  */
void ShellUnregisterHandler(void)
{
    pastShellCommandHandlerTable = NULL;
}

