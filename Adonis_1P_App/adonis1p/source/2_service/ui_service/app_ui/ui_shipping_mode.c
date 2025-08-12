/**
  ******************************************************************************
  * @file    ui_shipping_mode.c
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

#include "platform_io.h"
#include "data_base_info.h"
#include "sm_log.h"
#include <stdlib.h>
#include "system_status.h"
#include "ui_shipping_mode.h"
#include "err_code.h"

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

void amoled_display_exit_shipping_mode(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    SysStatus_u tempSysStatus = get_system_status();
    uint32_t msTick = 0;
    uint32_t msSendTick;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    uint32_t procTime = 0;
    
    sm_log(SM_LOG_DEBUG, "ui shipping mode!\r\n");

    // 电量计初始化配置完成后，SOC更新约需要4s，等待电量计就绪
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    while (msTickDev->read((uint8_t*)&msTick, 4) - msTick < 200) {
            sys_task_security_ms_delay(1, TASK_UI);
    }

    for (int n = 0; n < imageHeaderInfo->imageErrTablInfo.imageOutShippingTablLen; n++) {
        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n].height * UI_COLOR_DEPTH;
//        
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//        
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n]),g_tImageSecondRam))
		{
			return;
		}


        uint32_t k =0;
        // 共用蓝牙打钩图，定位需要自己算 47 *47大小，
        uint16_t imageHeightStart = 116;
        uint16_t imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n].height;
        uint16_t imageWitdthStart = 22;
        uint16_t imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n].witdh;

        if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
            return;
        }

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
        // 发送满屏小于25ms这是示波器实测，理论计算24.364ms
        // 等待发送完成
        msSendTick = msTickDev->read( (uint8_t*)&msSendTick, 4);
        while (msTickDev->read( (uint8_t*)&msSendTick, 4) - msSendTick < 25) {
         sys_task_security_ms_delay(2, TASK_UI);
        }
//        sm_log(SM_LOG_DEBUG, "n:%d, %d, %d, %d, %d, %d\r\n", n, imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd, ptQspiflash.len);
        procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
        if (procTime < 42) {
            sys_task_security_ms_delay(42 - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }
    }
    
    // 调整背光渐灭
    #if 0 // 原图已有渐暗
    for (uint8_t i =0xff; i > 0; i--) {
        if (tempSysStatus != get_system_status()) {
            break;
        }
        driver_rm69600_write_command(0x51);
        driver_rm69600_write_data(i);
        sys_task_security_ms_delay(4, TASK_UI);
    }
    #endif
    // 清屏
    amoled_display_clear();
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    // 等待电量计就绪
    while (msTickDev->read((uint8_t*)&msTick, 4) - msTick < 200) {
    	sys_task_security_ms_delay(1, TASK_UI);
    }
    #if 0
    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);
    #endif
}


