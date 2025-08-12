/**
  ******************************************************************************
  * @file    ui_err.c
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
#include "ui_err.h"
#include "err_code.h"

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];
#if 0
/**
  * @brief  显示错误信息
  * @param  esysErrUiStaus 错误码值
* @return None
* @note  
*/
void amoled_display_err(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev fuleGaugeDev = io_dev_get_dev(DEV_FULE_GAUGE);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    uint32_t msTick;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    EN_ERROR_CODE esysErrUiStaus =  get_system_ui_err_status();
    uint16_t errCodeVal = (uint16_t)esysErrUiStaus;

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd ;
    uint8_t imageInedx = 0;
    int numWitdh;
    int charSpace = 3; // 字符之间间隔3个像素
    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    // 读取百位数字
    errCodeVal = 401;
    imageInedx = errCodeVal / 100;
    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
    ptQspiflash.data = g_tImageSecondRam;
    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + 10;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].xpos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = g_tImageSecondRam[k++];
        }
    }
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    // 读取十位
    imageInedx = errCodeVal / 10 % 10;
    numWitdh = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh; 
    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
    ptQspiflash.data = g_tImageSecondRam;
    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + 10;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].xpos + numWitdh + charSpace;
    imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = g_tImageSecondRam[k++];
        }
    }
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    // 读取个位
    imageInedx = errCodeVal % 10;
    numWitdh =2 * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh; // 十位和个位的数字宽度
    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
    ptQspiflash.data = g_tImageSecondRam;
    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + 10;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].xpos + numWitdh + charSpace + charSpace;
    imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = g_tImageSecondRam[k++];
        }
    }
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);

    // 读取错误图标
    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].addr;
    ptQspiflash.data = g_tImageSecondRam;
    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].witdh * imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].height * UI_COLOR_DEPTH;
    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
    k =0;
    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].ypos;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].height;
    imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].xpos;
    imageWitdthEnd = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].xpos + imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].witdh;

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
    while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 4000) {
        sys_task_security_ms_delay(10, TASK_UI);
        if (esysErrUiStaus !=  get_system_ui_err_status()) {
            break;
        }
    }
    // 清屏
    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    ptAmoledInfo.area.x1 = 0;
    ptAmoledInfo.area.y1 = 0;
    ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
    ptAmoledInfo.area.y2 = UI_VER_RES - 1;
    ptAmoledInfo.data = g_tImageMainRam;
    ptAmoledInfo.len = IMAGE_SEZE;
    amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
    sys_task_security_ms_delay(20, TASK_UI);
}
#endif
/**
  * @brief  显示 0:usb过热, 1:usb过压, 2:bat过压复位
  * @param  esysErrUiStaus 错误码值
* @return None
* @note
*/
void amoled_display_chg_err(uint8_t status)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);

    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    uint32_t msTick;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    uint32_t procTime = 0;

//    EN_ERROR_CODE esysErrUiStaus =  get_system_ui_err_status();
//    uint16_t errCodeVal = (uint16_t)esysErrUiStaus;

    uint32_t k = 0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd ;
    uint8_t  imageInedx = 0;

    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);

    // 读取警告图标
//    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].witdh * imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0].height * UI_COLOR_DEPTH;
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0]),g_tImageSecondRam))
	{
		return;
	}

    k = 0;
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
    // 0:usb过热, 1:usb过压, 2:bat过压复位
    if (status < 2) {
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);

//        ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].witdh * imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].height * UI_COLOR_DEPTH;
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status]),g_tImageSecondRam))
		{
			return;
		}

        k = 0;
        imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].ypos;
        imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].height;
        imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].xpos;
        imageWitdthEnd = imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].xpos + imageHeaderInfo->imageErrTablInfo.imageWrongChargeTablInfo[status].witdh;

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
        ptAmoledInfo.area.x2 = UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
        while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 5000) {
            sys_task_security_ms_delay(10, TASK_UI);
       //     if (esysErrUiStaus !=  get_system_ui_err_status()) {
       //         break;
      //      }
            if (status == 1) { // 0:usb过热, 1:usb过压
                if (UI_ERR_USB_OV < get_current_ui_detail_status()) {
                    break;
                }
            } else if (status == 0) {
                if (UI_ERR_USB_HOT < get_current_ui_detail_status()) {
                    break;
                }
            }
        }
    } else {
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
        // 播放device轮廓
        imageInedx = 14; // 调用最后一页
//        ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].witdh * imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}

        k = 0;
        imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].height;
        imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].xpos;
        imageWitdthEnd = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].xpos + imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[imageInedx].witdh;

        if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
         //   sm_log(SM_LOG_NOTICE, "imageResetChargeTablInfo!!! %d\r\n", imageInedx);
            return;
        }

        for(int i = imageHeightStart;i < imageHeightEnd;i++) {
            for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = g_tImageSecondRam[k++];
            }
        }
        // 播放plug
        for (int cnt = 0; cnt < 2; cnt++) {
            for (int n = 0; n < imageHeaderInfo->imageErrTablInfo.imageResetChargeTablLen - 1; n++) {
                msTick = msTickDev->read( (uint8_t*)&msTick, 4);
                memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//                ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].addr;
//                ptQspiflash.data = g_tImageSecondRam;
//                ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].height * UI_COLOR_DEPTH;
//                qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
				if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n]),g_tImageSecondRam))
				{
					return;
				}

                k = 0;
                imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].ypos;
                imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].height;
                imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].xpos;
                imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageResetChargeTablInfo[n].witdh;
                
                if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
                 //   sm_log(SM_LOG_NOTICE, "imageResetChargeTablInfo!!!n:%d, cnt:%d\r\n", n, cnt);
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
                if (procTime < 190) {
                    sys_task_security_ms_delay(190 - procTime, TASK_UI);
                } else { // 大于等于100时也需要释放MCU 1ms
                    sys_task_security_ms_delay(1, TASK_UI);
                }
                if (UI_ERR_BAT_OV < get_current_ui_detail_status()) {
                    break;
                }
            }
            if (UI_ERR_BAT_OV < get_current_ui_detail_status()) {
                break;
            }
        }
    }
    // 清屏
    
    amoled_display_clear();

}
