/**
  ******************************************************************************
  * @file    ui_err_wait.c
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
#include "ui_err_wait.h"
#include "err_code.h"

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

void amoled_display_err_wait(uint8_t status)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    SysStatus_u tempSysStatus = get_system_status();
    uint32_t msTick;
    uint32_t msRoundDelayTick;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    uint32_t procTime = 0;
    sm_log(SM_LOG_DEBUG, "ui err wait!\r\n");

    // 添加 错误标志
    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd ;
    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);

//    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].witdh * imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].ypos;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].height;
    imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].xpos;
    imageWitdthEnd = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].xpos + imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].witdh;
    
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

    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    // 添加 冷 热, status 0:冷，1热, 2:general
    if (status < 2) {
//        ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].witdh * imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].height * UI_COLOR_DEPTH;
//        
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//        
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].ypos;
        imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].height;
        imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].xpos;
        imageWitdthEnd = imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].xpos + imageHeaderInfo->imageErrTablInfo.imageColdHotTablInfo[status].witdh;

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
    }


    // 播放漏斗
    for (int cnt = 0; cnt < 2; cnt++) {
        for (int n = 0; n < WAITING_LEN; n++) {
            msTick = msTickDev->read( (uint8_t*)&msTick, 4);
            memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//            ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].addr;
//            ptQspiflash.data = g_tImageSecondRam;
//            ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].height * UI_COLOR_DEPTH;
//            
//            if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                return;
//            }
//            
//            qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
			if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n]),g_tImageSecondRam))
			{
				return;
			}

        
            k =0;
            // 共用蓝牙打钩图，定位需要自己算 47 *47大小，
            imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].ypos;
            imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].height;
            imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].xpos;
            imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageWaitingTablInfo[n].witdh;
        
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
            procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
            if (procTime < 83) {
                sys_task_security_ms_delay(83 - procTime, TASK_UI);
            } else { // 大于等于100时也需要释放MCU 1ms
                sys_task_security_ms_delay(1, TASK_UI);
            }
            if (UI_ERR_REBOOT_TIP <= get_current_ui_detail_status()) {
                break;
            }
        }
        msRoundDelayTick = msTickDev->read( (uint8_t*)&msRoundDelayTick, 4);
        while (msTickDev->read( (uint8_t*)&msRoundDelayTick, 4) - msRoundDelayTick < 500) { // 错误码显示5秒， 一次循环2秒，中间等待0.5S 两次循环总共凑够5S
            sys_task_security_ms_delay(2, TASK_UI);
            if (UI_ERR_REBOOT_TIP <= get_current_ui_detail_status()) {
                break;
            }
        }
    }

    // 清屏
    amoled_display_clear();
}


