/**
  ******************************************************************************
  * @file    shell_cmd_handler.c
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include "serial_line.h"
//#include "shell_cmd_handle.h"

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cyabs_rtos.h"

#include "system_status.h"
#include "task_heatting_service.h"
#include "sm_log.h"
#include "platform_io.h"
#include "data_base_info.h"
#include "task_system_service.h"
#include "ui_heat.h"
#include "err_code.h"
#include "ui_clean.h"
#include "ui_err_wait.h"
#include "ui_err_critical.h"
#include "ui_err_reboot.h"

/* Private define ------------------------------------------------------------*/
#define SHELLDATA_SIZE            256
#if 0
void start_heat_standard(void)
{
    SysStatus_u tempSysStatus;
    TaskHandle_t *temp_handle;
    tempSysStatus = get_system_status();
     if (IDLE == tempSysStatus) { // 空闲状态则加热
         set_system_status(HEATTING_STANDARD);
         temp_handle = get_task_heatting_handle();
         xTaskNotifyGive(*temp_handle);
     } else if (HEATTING_STANDARD == tempSysStatus || HEATTING_BOOST == tempSysStatus) { // 加热状态则停止加热
        sm_log(SM_LOG_DEBUG, "System is heatting cmd fail\r\n");
     } else { // 如果是充电状态并且足够一支烟电量，则先停止充电，再启动加热， 后续优化细化处理
        sm_log(SM_LOG_DEBUG, "System is charging cmd fail\r\n");
     }
}

void start_heat_boost(void)
{
    SysStatus_u tempSysStatus;
    TaskHandle_t *temp_handle;
    tempSysStatus = get_system_status();
     if (IDLE == tempSysStatus) { // 空闲状态则加热
         set_system_status(HEATTING_BOOST);
         temp_handle = get_task_heatting_handle();
         xTaskNotifyGive(*temp_handle);
     } else if (HEATTING_STANDARD == tempSysStatus || HEATTING_BOOST == tempSysStatus) { // 加热状态则停止加热
        sm_log(SM_LOG_DEBUG, "System is heatting cmd fail\r\n");
     } else { // 如果是充电状态并且足够一支烟电量，则先停止充电，再启动加热， 后续优化细化处理
        sm_log(SM_LOG_DEBUG, "System is charging cmd fail\r\n");
     }
}

void stop_heat(void)
{
    SysStatus_u tempSysStatus;
    TaskHandle_t *temp_handle;
    tempSysStatus = get_system_status();
     if (IDLE == tempSysStatus) { // 空闲状态则加热

     } else if (HEATTING_STANDARD == tempSysStatus || HEATTING_BOOST == tempSysStatus \
        || HEAT_MODE_TEST_VOLTAGE == tempSysStatus || HEAT_MODE_TEST_POWER == tempSysStatus || HEAT_MODE_TEST_TEMP == tempSysStatus) { // 加热状态则停止加热
        set_system_status(IDLE);
        temp_handle = get_task_heatting_handle();
        xTaskNotifyGive(*temp_handle);
     } else {
        sm_log(SM_LOG_DEBUG, "stop heat cmd fail\r\n");
     }
}

// Show shell command help
#define HELP_CMDHELP  "Show this help info\r\n"
/**
  * @brief  显示目前支持的命令
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_help(char *args,unsigned short arglen)
{
    const ShellCmd_t *pstShellCmd = staShellCommandTable;
    sm_log(SM_LOG_DEBUG, "--------------------------------------------------------------------------\r\n");
    sm_log(SM_LOG_DEBUG, "--------------------------------------------------------------------------\r\n");
    for(unsigned char index = 0; index < 250; index++)
    {
        if( pstShellCmd->name == NULL )
        {
            // End of table
            break;
        }
    sm_log(SM_LOG_DEBUG, "%s: %s\n",pstShellCmd->name,pstShellCmd->help);
        pstShellCmd++;
    }
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMDREBOOT  "System reboot\r\n"
/**
  * @brief  系统复位
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_reboot(char *args,unsigned short arglen)
{
    sm_log(SM_LOG_DEBUG, "System reboot...\r\n");
    cy_rtos_delay_milliseconds(1000);
    NVIC_SystemReset();
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMD_START_HEAT_STANDARD  "Start heat standard mode\r\n"
/**
  * @brief  系统复位
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_start_heat_standard(char *args,unsigned short arglen)
{
    sm_log(SM_LOG_DEBUG, "Start heatting standard mode...\r\n");
    start_heat_standard();
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMD_START_HEAT_BOOST  "Start heat boost mode\r\n"
/**
  * @brief  系统复位
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_start_heat_boost(char *args,unsigned short arglen)
{
    sm_log(SM_LOG_DEBUG, "Start heatting boost mode...\r\n");
    start_heat_boost();
    return SHELL_CMD_OPS_OK;
}






//-----------------------------------------------------------------//
#define HELP_CMD_START_HEAT_TEST_VOLTAGE  "Start heat test dcdc out voltage mode\r\n"
/**
  * @brief  测试DCDC
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_start_heat_test_dcdc(char *args,unsigned short arglen)
{
    SysStatus_u tempSysStatus;
    TaskHandle_t *temp_handle;

    char *args1;
    char *args2;
    float  targetVoltage = 0.0f;
	unsigned int heatTotalTime=0;
    SHELL_ARGS_INIT(args, args1);
    char *param1 = ShellGetNextArgs(&args1);

    targetVoltage = (float)atof(param1);
    SHELL_ARGS_INIT(args1, args2);
    char *param2 = ShellGetNextArgs(&args2);

    heatTotalTime= (uint16_t)atoi(param2);

    
    sm_log(SM_LOG_DEBUG, "Start heatting test dcdc out voltage mode...\r\n");
    sm_log(SM_LOG_DEBUG, "Target voltage:%f HeatTotalTime:%d\r\n",targetVoltage,heatTotalTime);
    tempSysStatus = get_system_status();
     if (IDLE == tempSysStatus) { // 空闲状态则加热
         set_system_status(HEAT_MODE_TEST_VOLTAGE);

         temp_handle = get_task_heatting_handle();
         heat_test_voltage_start(targetVoltage,heatTotalTime);
         xTaskNotifyGive(*temp_handle);
         
     } else { // 加热状态则停止加热
        sm_log(SM_LOG_DEBUG, "System is heatting cmd fail\r\n");
     }
    return SHELL_CMD_OPS_OK;
}
#define HELP_CMD_START_HEAT_TEST_POWER  "Start heat test power mode\r\n"
/**
  * @brief  测试设置功率
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_start_heat_test_power(char *args,unsigned short arglen)
{
    SysStatus_u tempSysStatus;
    TaskHandle_t *temp_handle;
    char *args1;
    char *args2;
	float targetPower = 0.0f;

    unsigned int heatTotalTime = 0;
    SHELL_ARGS_INIT(args, args1);
    char *param1 = ShellGetNextArgs(&args1);

    targetPower = (float)atof(param1);
    SHELL_ARGS_INIT(args1, args2);
    char *param2 = ShellGetNextArgs(&args2);

    heatTotalTime= (uint16_t)atoi(param2);

    sm_log(SM_LOG_DEBUG, "Start heatting test power mode...\r\n");
    sm_log(SM_LOG_DEBUG, "Target power:%f HeatTotalTime:%d\r\n",targetPower,heatTotalTime);
    tempSysStatus = get_system_status();
    if (IDLE == tempSysStatus) { // 空闲状态则加热
        set_system_status(HEAT_MODE_TEST_POWER);
        temp_handle = get_task_heatting_handle();
        heat_test_power_start(targetPower,heatTotalTime);
        xTaskNotifyGive(*temp_handle);
    } else { // 加热状态则停止加热
       sm_log(SM_LOG_DEBUG, "System is heatting cmd fail\r\n");
    }
    return SHELL_CMD_OPS_OK;
}
#define HELP_CMD_START_HEAT_TEST_TEMP  "Start heat test temp ctrl mode\r\n"
/**
  * @brief  测试 温度控制
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_start_heat_test_temp(char *args,unsigned short arglen)
{
    SysStatus_u tempSysStatus;
    TaskHandle_t *temp_handle;
    char *args1;
    char *args2;
 
    float targetTemp = 0.0f;
    unsigned int heatTotalTime = 0;
    SHELL_ARGS_INIT(args, args1);
    char *param1 = ShellGetNextArgs(&args1);

    targetTemp = (float)atof(param1);
    SHELL_ARGS_INIT(args1, args2);
    char *param2 = ShellGetNextArgs(&args2);

    heatTotalTime = (uint16_t)atoi(param2);
    sm_log(SM_LOG_DEBUG, "Start heatting test temp ctrl mode...\r\n");
    sm_log(SM_LOG_DEBUG, "Target temp:%f HeatTotalTime:%d\r\n",targetTemp,heatTotalTime);
    tempSysStatus = get_system_status();
    if (IDLE == tempSysStatus) { // 空闲状态则加热
        set_system_status(HEAT_MODE_TEST_TEMP);
        temp_handle = get_task_heatting_handle();
        heat_test_temp_start(targetTemp,heatTotalTime);
        xTaskNotifyGive(*temp_handle); //任务同步
        
    } else { // 加热状态则停止加热
       sm_log(SM_LOG_DEBUG, "System is heatting cmd fail\r\n");
    }
    return SHELL_CMD_OPS_OK;
}
//-----------------------------------------------------------------//

#define HELP_CMD_STOP_HEAT  "Stop heat\r\n"
/**
  * @brief  系统复位
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_stop_heat(char *args,unsigned short arglen)
{
    sm_log(SM_LOG_DEBUG, "Stop heatting...\r\n");
    stop_heat();
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMD_MOTOR_CTRL  "Motor control\r\n"
/**
  * @brief  系统复位
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_motor_ctrl(char *args,unsigned short arglen)
{
	static uint8_t status = 0;
    char *next1_args;
    char *next2_args;
    char *next3_args;
    MotorInfo_t motorInfo;

    uint16_t runIndvMs;
    uint16_t stopIndvMs;
    uint16_t loopTimes;

    SHELL_ARGS_INIT(args, next1_args);
    char *param1 = ShellGetNextArgs(&next1_args);
    runIndvMs = (uint16_t)atoi(param1);

    SHELL_ARGS_INIT(next1_args, next2_args);
    char *param2 = ShellGetNextArgs(&next2_args);
    stopIndvMs = (uint16_t)atoi(param2);

    SHELL_ARGS_INIT(next2_args, next3_args);
    char *param3 = ShellGetNextArgs(&next3_args);
    loopTimes = (uint16_t)atoi(param3);

    
    
    if(runIndvMs < 10*1000 && stopIndvMs < 10*1000 && loopTimes <100)
    {
        motor_set(runIndvMs,stopIndvMs,loopTimes);
        sm_log(SM_LOG_DEBUG, "motor_set(%d,%d,%d)\r\n",runIndvMs,stopIndvMs,loopTimes);
    }else{
        sm_log(SM_LOG_DEBUG, "motor_set cmd fail\r\n");
    }
 
    return SHELL_CMD_OPS_OK;
}
#define HELP_CMD_BEEP_CTRL  "Beep control\r\n"
/**
  * @brief  系统复位
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_beep_ctrl(char *args,unsigned short arglen)
{
	static uint8_t status = 0;
    char *next0_args;
    char *next1_args;
    char *next2_args;
    char *next3_args;
    
   // BeepInfo_t beepInfo;
    uint16_t ring_hz;
    uint16_t runIndvMs;
    uint16_t stopIndvMs;
    uint16_t loopTimes;

    SHELL_ARGS_INIT(args, next0_args);
    char *param0 = ShellGetNextArgs(&next0_args);
    ring_hz = (uint16_t)atoi(param0);

    SHELL_ARGS_INIT(next0_args, next1_args);
    char *param1 = ShellGetNextArgs(&next1_args);
    runIndvMs = (uint16_t)atoi(param1);

    SHELL_ARGS_INIT(next1_args, next2_args);
    char *param2 = ShellGetNextArgs(&next2_args);
    stopIndvMs = (uint16_t)atoi(param2);

    SHELL_ARGS_INIT(next2_args, next3_args);
    char *param3 = ShellGetNextArgs(&next3_args);
    loopTimes = (uint16_t)atoi(param3);

    if(runIndvMs < 10*1000 && stopIndvMs < 10*1000 && loopTimes <100)
    {
        beep_test_set(ring_hz,runIndvMs,stopIndvMs,loopTimes);
        sm_log(SM_LOG_DEBUG, "beep_test_set(%d,%d,%d,%d)\r\n",ring_hz,runIndvMs,stopIndvMs,loopTimes);
    }else{
        sm_log(SM_LOG_DEBUG, "beep_test_set cmd fail\r\n");
    }
 
    return SHELL_CMD_OPS_OK;
}

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

#define HELP_CMD_GUI_HI  "gui display hi\r\n"

/**
  * @brief  显示内容, 定位点在顶部开始计算
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_display_hi(char *args,unsigned short arglen)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

    for (int n = 0; n < imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablLen; n++) {
//        int n = 28;
        memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
        ptQspiflash.addr = imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].addr;
        ptQspiflash.data = g_tImageSecondRam;
        ptQspiflash.len = imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].witdh * imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].height * UI_COLOR_DEPTH;
        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
        
        uint32_t k =0;
        uint16_t imageHeightStart = imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].ypos;
        uint16_t imageHeightEnd = imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].ypos + imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].height;
        uint16_t imageWitdthStart = imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].xpos;
        uint16_t imageWitdthEnd = imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].xpos + imageHeaderInfo->imageHeatTablInfo.imagePreheatHiTablInfo[n].witdh;

        for(int i = imageHeightStart;i < imageHeightEnd;i++) {
            for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = g_tImageSecondRam[k++];
            }
        }
        ptAmoledInfo.area.x1 = 0;
        ptAmoledInfo.area.y1 = 0;
        ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
        sm_log(SM_LOG_DEBUG, "n:%d, %d, %d, %d, %d, %d\r\n", n, imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd, ptQspiflash.len);
        vTaskDelay(100);
    }
//    cyhal_system_delay_ms(20000);
    return SHELL_CMD_OPS_OK;
}


#define HELP_CMD_GUI_HEAT_START  "heat_start_display\r\n"

/**
  * @brief  显示内容, 定位点在底部开始计算
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_display_heat_start(char *args,unsigned short arglen)
{
//#if 0
    set_system_status(HEATTING_BOOST);
    amoled_display_heat_hi();
    amoled_display_heat_start();
    amoled_display_heat_consume_all();
    amoled_display_heat_end(HEATTING_BOOST);
    amoled_display_heat_dispose();
    amoled_display_heat_byte();
//#endif
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMD_GUI_ERR  "err_display\r\n"

/**
  * @brief  显示内容, 显示错误码
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_display_err(char *args,unsigned short arglen)
{
    char *next0_args;
    SHELL_ARGS_INIT(args, next0_args);
    char *param0 = ShellGetNextArgs(&next0_args);
    EN_ERROR_CODE errVal = (uint16_t)atoi(param0);
    sm_log(SM_LOG_DEBUG, "set errVal %d\r\n", errVal);
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMD_GUI_CLEAN_TIP  "clean_tip_display\r\n"

/**
  * @brief  显示内容, 清洁提示
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_display_clean_tip(char *args,unsigned short arglen)
{
    sm_log(SM_LOG_DEBUG, "clean tip\r\n");
    amoled_display_clean_tip();
    return SHELL_CMD_OPS_OK;
}

#define HELP_CMD_GUI_CLEAN  "clean_display\r\n"
    
/**
  * @brief  显示内容, 清洁
  * @param  args:    参数
  * @param  arglen:  参数长度
  * @return None
  * @note   None
  */
int cmd_display_clean(char *args,unsigned short arglen)
{
    sm_log(SM_LOG_DEBUG, "cleaing\r\n");
//     amoled_display_err_wait(0);

//     amoled_display_err_wait(1);
//     amoled_display_err_critical();
     amoled_display_err_reboot_tip();
    amoled_display_err_reboot_countDown();

//    amoled_display_cleaning();
    return SHELL_CMD_OPS_OK;
}

const ShellCmd_t staShellCommandTable[] =
{
 {"help",                       cmd_help,                 HELP_CMDHELP},
 {"reboot",                     cmd_reboot,               HELP_CMDREBOOT},
 {"heat",                       cmd_start_heat_standard,  HELP_CMD_START_HEAT_STANDARD},
 {"boost",                      cmd_start_heat_boost,     HELP_CMD_START_HEAT_BOOST},

 {"heat_test_v",                cmd_start_heat_test_dcdc,  HELP_CMD_START_HEAT_TEST_VOLTAGE},
 {"heat_test_p",                cmd_start_heat_test_power,  HELP_CMD_START_HEAT_TEST_POWER},
 {"heat_test_t",                cmd_start_heat_test_temp,  HELP_CMD_START_HEAT_TEST_TEMP},

 {"unheat",                     cmd_stop_heat,            HELP_CMD_STOP_HEAT},
 {"motor",                      cmd_motor_ctrl,           HELP_CMD_MOTOR_CTRL},
 {"beep",                       cmd_beep_ctrl,            HELP_CMD_BEEP_CTRL},
  {"hi",                         cmd_display_hi,           HELP_CMD_GUI_HI},
 {"heatstart",                  cmd_display_heat_start,   HELP_CMD_GUI_HEAT_START},
 {"err",                        cmd_display_err,          HELP_CMD_GUI_ERR},
 {"cleantip",                   cmd_display_clean_tip,    HELP_CMD_GUI_CLEAN_TIP},
 {"clean",                      cmd_display_clean,        HELP_CMD_GUI_CLEAN_TIP},

 #if 0
 {"log",                         CmdLog,                  HELP_CMDLOG},
 {"inv",                         CmdInventoryInfo,        HELP_CMDINVINFO},
 {"print",                       CmdPrint,                HELP_CMDPRINT},
 {"uart",                        CmdUartTest,             HELP_CMDUARTTEST},
 {"flash",                       CmdFlashTest,            HELP_CMDFLASHTEST},
 {"ef",                          CmdExtFlashTest,         HELP_CMDEXTFLASHTEST},
 {"sys",                         CmdSysConfig,            HELP_CMDSYSCONFIG},
 {"ifconfig",                    CmdIfConfig,             HELP_CMDIFCONFIG},
 {"fw",                          CmdFirmware,             HELP_CMDFIRMWARE},
 #endif
 {NULL,                          NULL,                    NULL}
};
#endif
