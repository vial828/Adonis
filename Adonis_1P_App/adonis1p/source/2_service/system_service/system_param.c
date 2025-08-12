/**
  ******************************************************************************
  * @file    system_param.c
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "system_param.h"
#include "data_base_info.h"
#include "platform_io.h"
#include "sm_log.h"
#include "protocol_usb.h"
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "session_data_record.h"
#include "event_data_record.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

void printf_tr_info(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    sm_log(SM_LOG_INFO, "Tr info:\r\n");
    for (int i = 0; i < MAX_TR_TBL_GR; i++) {
        sm_log(SM_LOG_INFO, "%d  %d\r\n", p_tHeatParamInfo->tTrInfo[i].tempeture, p_tHeatParamInfo->tTrInfo[i].resistor);
    }
    sm_log(SM_LOG_INFO, "-------------------------\r\n");
}

void printf_temp_curve_info(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    sm_log(SM_LOG_INFO, "Base Temp curve info:\r\n");
    for (int i = 0; i < MAX_TEMP_CTRL_POINT; i++) {
        sm_log(SM_LOG_INFO, "%d  %d\r\n", p_tHeatParamInfo->tHeatBaseTemp[i].time, p_tHeatParamInfo->tHeatBaseTemp[i].tempeture);
    }
    sm_log(SM_LOG_INFO, "Boost Temp curve info:\r\n");
    for (int i = 0; i < MAX_TEMP_CTRL_POINT; i++) {
        sm_log(SM_LOG_INFO, "%d  %d\r\n", p_tHeatParamInfo->tHeatBoostTemp[i].time, p_tHeatParamInfo->tHeatBoostTemp[i].tempeture);
    }
   sm_log(SM_LOG_INFO,"-------------------------\r\n");
}

void printf_power_info(void)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    sm_log(SM_LOG_INFO, "Heat power info:\r\n");
    for (int i = 0; i < MAX_POWER_CTRL_POINT; i++) {
        sm_log(SM_LOG_INFO, "%d  %d\r\n", p_tHeatParamInfo->tHeatBasePower[i].time, p_tHeatParamInfo->tHeatBasePower[i].power);
    }
    sm_log(SM_LOG_INFO, "Heat power info:\r\n");
    for (int i = 0; i < MAX_POWER_CTRL_POINT; i++) {
        sm_log(SM_LOG_INFO, "%d  %d\r\n", p_tHeatParamInfo->tHeatBoostPower[i].time, p_tHeatParamInfo->tHeatBoostPower[i].power);
    }
   sm_log(SM_LOG_INFO,"-------------------------\r\n");
}

/**
  * @brief  从QSPIFLASH获取镜像头信息
  * @param  None
  * @return 返回指针
  * @note   None
  */
void image_header_info_get(void)
{
    ImageHeaderInfo_t* imageHeaderInfo;
    Qspiflash_t ptQspiflash;
    uint8_t buffer = 0x88;
    imageHeaderInfo = get_image_header_info_handle();
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    
    //wanggankun@2024-11-22, add, for image upgrade
    ptQspiflash.addr = EXT_FLASH_UI_FLAG_START_ADDRESS;
    ptQspiflash.data = &buffer;
    ptQspiflash.len = 1;
    qspiflashDev->read( (uint8_t*)&ptQspiflash, 1);

    sm_log(SM_LOG_DEBUG,"IMGAE FLAG: 0x%x\n", buffer);

    if(buffer == CY_OTA_IMAGE_USE_UI1 || buffer == 0xFF )
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI1_START_ADDRESS: 0x%x\n", EXT_FLASH_UI1_START_ADDRESS);
        ptQspiflash.addr = EXT_FLASH_UI1_START_ADDRESS; 
		imageHeaderInfo->offSetAddr = EXT_FLASH_UI1_START_ADDRESS;
    }
    else if(buffer == CY_OTA_IMAGE_USE_UI2)
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI2_START_ADDRESS: 0x%x\n", EXT_FLASH_UI2_START_ADDRESS);
        ptQspiflash.addr = EXT_FLASH_UI2_START_ADDRESS; 
		imageHeaderInfo->offSetAddr = EXT_FLASH_UI2_START_ADDRESS;
    }
    else
    {
        ptQspiflash.addr = EXT_FLASH_UI1_START_ADDRESS;
		imageHeaderInfo->offSetAddr = EXT_FLASH_UI1_START_ADDRESS;
    }

    ptQspiflash.data = (uint8_t*)imageHeaderInfo;
    ptQspiflash.len = sizeof(ImageHeaderInfo_t) - 4;					// 注意offsetAddr不要给覆盖了
    qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(Qspiflash_t));

//    uint8_t *p = (uint8_t*)imageHeaderInfo;
//    for (int i = 0; i < sizeof(ImageHeaderInfo_t); i++) {
//            sm_log(SM_LOG_INFO, "%x", p[i]);
//    }
}

int get_app_vsrsion_start_pos(char *ver, uint8_t verLen)
{
    int j = 0;
    for (j = 0; j < verLen; j++) {
        if (ver[j] == '.') {
            break;
        }
    }
    int firstDotPos = j; // 找到第一个点号位置
    int posDownLineNum = 0;
    for (j = firstDotPos; j > 0; j--) {
        if (ver[j] == '_') {
            break;
        }
    }
    posDownLineNum = j; // 找到点号前第一个下划线位置
    for (j = posDownLineNum; j < firstDotPos; j++) {
        if (ver[j] >= '0' && ver[j] <= '9') {
            break;
        }
    }
    int verStartPos = j; // 找到有效版本位置
    return verStartPos;
}
char g_appVsersionBuf[64] = {0};
/**
  * @brief  初始化固件版本号
  * @param  None
  * @return None
  * @note   将makefile中的固件版本转换为字符串
  */
void init_app_version(void)
{
    int i = 0;
    char tmpAppVersionBuf[64] = {0};
    char tmpCodeDefAppVerionBuf[64] = {0};
    int tmpLen = sprintf(&g_appVsersionBuf[0], "%d", APP_VERSION_MAJOR);
    i += tmpLen;
    g_appVsersionBuf[i] = '.';
    i++;
    tmpLen = sprintf(&g_appVsersionBuf[i], "%d", APP_VERSION_MINOR);
    i += tmpLen;
    g_appVsersionBuf[i] = '.';
    i++;
    tmpLen = sprintf(&g_appVsersionBuf[i], "%d", APP_VERSION_BUILD);
    i += tmpLen;
    int j = 0;
    for (j = 0; j < i; j++) {
        tmpAppVersionBuf[j] = g_appVsersionBuf[j];
    }
    tmpAppVersionBuf[i] = '\n'; // 得到主版本，将主版本与宏定义的版本进行对比，如果不同则打印提示信息
    int verLen = strlen(PROTOCOL_APP_VERSION);
    memcpy(tmpCodeDefAppVerionBuf, PROTOCOL_APP_VERSION, verLen);
    int verStartPos = get_app_vsrsion_start_pos(tmpCodeDefAppVerionBuf, verLen);

    if (strcmp(&tmpCodeDefAppVerionBuf[verStartPos], tmpAppVersionBuf) != 0) {
        sm_log(SM_LOG_ERR,"build app ver was not match macro app ver build:%s, macro: %s !\r\n",tmpAppVersionBuf,  &tmpCodeDefAppVerionBuf[verStartPos]);
    }
    
    ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    uint8_t *pIapBuf = get_iap_combine_buf();
    t_temp.addr = MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - (2 * MCU_FLASH_ROW_SIZE); // sign header 保存位置
    t_temp.pData = pIapBuf;
    t_temp.size = MCU_FLASH_ROW_SIZE;
    mcuFlashDev->read( (uint8_t*)&t_temp, t_temp.size);
    
    uint16_t revision =COMB_2BYTE(pIapBuf[23], pIapBuf[22]);
    if ((pIapBuf[20] != APP_VERSION_MAJOR || pIapBuf[21] != APP_VERSION_MINOR || revision != APP_VERSION_BUILD)) {
        sm_log(SM_LOG_ERR,"sign app ver not match build app ver! sig: %d.%d.%d, build %s\r\n", pIapBuf[20], pIapBuf[21], revision,tmpAppVersionBuf);
    }
    tmpLen = sprintf(&g_appVsersionBuf[i], "%s", PROTOCOL_APP_VERSION_DEBUG); // 调试版本字段
    i += tmpLen;

    g_appVsersionBuf[i] = '\n';
    i++;
    g_appVsersionBuf[i] = 0;
}

/**
  * @brief  获取固件版本号
  * @param  None
  * @return 返回指针
  * @note   None
  */
char* get_app_version(void)
{
    return g_appVsersionBuf;
}

/**
  * @brief  初始化参数
  * @param  None
  * @return 返回指针
  * @note   None
  */
void system_param_info_init(void)
{
    init_app_version();
//    ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//    paramDev->read( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
    if (p_tHeatParamInfo->validCheck == 0xAACC) {
//        sm_log(SM_LOG_INFO,"FLASH had param info!\r\n");
//        printf_tr_info();
//        printf_temp_curve_info();
//        printf_power_info();
    } else {
        // 存储区数据无效，初始化参数
//        sm_log(SM_LOG_INFO,"heat_param_info_init !\r\n");
        heat_param_info_init();

//        sm_log(SM_LOG_INFO,"Before write!\r\n");
//        printf_tr_info();
//        printf_temp_curve_info();
//        printf_power_info();
//        paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
//        paramDev->read( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
 //       sm_log(SM_LOG_INFO,"After read!\r\n");
//        printf_tr_info();
//        printf_temp_curve_info();
//        printf_power_info();
    }
    image_header_info_get();
    gui_init();
//    init_mcu_flash_param();

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	session_record_init();						// 该部分代码需要注意，必须等QSPI初始化完成后执行
	event_record_init();
	heatint_profile_init();						// 读取Heating_Profile，若空，则补充默认值
	heat_param_info_update();					// 读取EEPROM加热参数，更新加热文件
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

