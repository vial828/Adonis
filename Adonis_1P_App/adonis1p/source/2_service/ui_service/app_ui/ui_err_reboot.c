/**
  ******************************************************************************
  * @file    ui_err_reboot.c
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
#include "ui_err_reboot.h"
#include "err_code.h"
#include "task_system_service.h"
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT) //bug2100523
#include "ota.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

void amoled_display_err_reboot_tip(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    SysStatus_u tempSysStatus = get_system_status();
    uint32_t msTick;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    uint32_t procTime = 0;
    sm_log(SM_LOG_DEBUG, "ui err reboot_tip!\r\n");
    uint8_t buf[3];
    keyDev->read( (uint8_t*)&buf, 3);

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

    // 添加 13数字
    // 13
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    
//    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].ypos;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].height;
    imageWitdthStart = 28;
    imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].witdh;
    
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
#if 0
    // 3
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    
    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].addr;
    ptQspiflash.data = g_tImageSecondRam;
    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].height * UI_COLOR_DEPTH;
    
    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
        return;
    }

    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
    k =0;
    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].ypos;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].height;
    imageWitdthStart = 46;
    imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].witdh;
    
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
#endif
    // 播放按键提示
    for (int cnt = 0; cnt < 5; cnt++) {
        keyDev->read( (uint8_t*)&buf, 3);
        if (buf[0] == 1 || buf[1] == 1) {
            // 播放倒计时数字,从13开始；用户自己操作 连续按下时 从9开始,硬件触发时间为11s(9.3~12.8,典型值为11)
            int numDownStart = 13 - 1;
        //    UiInfo_t *pUiInfo = get_error_high_pro_even();
        //    if (pUiInfo->uiStatus == UI_ERR_REBOOT_TIP) {
        //        numDownStart = 13 - 1;
        //    } 
            // 重新播放手势
            msTick = msTickDev->read( (uint8_t*)&msTick, 4);
            memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//            ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].addr;
//            ptQspiflash.data = g_tImageSecondRam;
//            ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].height * UI_COLOR_DEPTH;
//            
//            if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                return;
//            }
//            
//            qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
			if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13]),g_tImageSecondRam))
			{
				return;
			}

            
            k =0;
            imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].ypos;
            imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].height;
            imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].xpos;
            imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[13].witdh;
            
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
            // 清 13数字 防止重叠无法覆盖
            memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
            k =0;
            imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].ypos;
            imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].height;
            imageWitdthStart = 28;
            imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].witdh;

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
            int n = 0;
            int pressTime = 0;
            if (get_base_key_press_time() > get_boost_key_press_time()) { // BUG 1934401
                pressTime = get_base_key_press_time();
            } else {
                pressTime = get_boost_key_press_time();
            }
            if (pressTime != 0) {
                pressTime = (pressTime / 1000) + 1;
            }
            pressTime = pressTime + 4;
            for (n = numDownStart; n > pressTime; n--) {
                msTick = msTickDev->read( (uint8_t*)&msTick, 4);
                memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//                ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].addr;
//                ptQspiflash.data = g_tImageSecondRam;
//                ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].height * UI_COLOR_DEPTH;
//                
//                if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                    return;
//                }
//                
//                qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
				if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n]),g_tImageSecondRam))
				{
					return;
				}

            
                k =0;
                imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].ypos;
                imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].height;
                imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].xpos;
                imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].witdh;
                
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
                if (procTime < 1000) {
                    procTime = 1000 - procTime;
                    while (procTime > 10) {
                        keyDev->read( (uint8_t*)&buf, 3);
                        if (buf[0] != 1 && buf[1] != 1) {
                            cnt = 0;
                            n = -1; // 退出for循环
                            memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//                            ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].addr;
//                            ptQspiflash.data = g_tImageSecondRam;
//                            ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].height * UI_COLOR_DEPTH;
//                            
//                            if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                                return;
//                            }
//                            
//                            qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
							if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0]),g_tImageSecondRam))
							{
								return;
							}

                            
                            k =0;
                            imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].ypos;
                            imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].height;
                            imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].xpos;
                            imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[0].witdh;
                            
                            if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
                                return;
                            }

                            // 清除白色数字
                            for(int i = imageHeightStart;i < imageHeightEnd;i++) {
                                for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                                    g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = 0;
                                    g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = 0;
                                    g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = 0;
                                }
                            }

                            // 重新添加 13数字
                            // 1
                            memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
                            
//                            ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].addr;
//                            ptQspiflash.data = g_tImageSecondRam;
//                            ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].height * UI_COLOR_DEPTH;
//                            
//                            if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                              return;
//                            }
//                            
//                            qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
							if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0]),g_tImageSecondRam))
							{
								return;
							}

                            k =0;
                            imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].ypos;
                            imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].height;
                            imageWitdthStart = 28;
                            imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootSecTablInfo[0].witdh;
                            
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
                            #if 0
                            // 3
                            memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
                            
                            ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].addr;
                            ptQspiflash.data = g_tImageSecondRam;
                            ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].height * UI_COLOR_DEPTH;
                            
                            if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
                              return;
                            }
                            
                            qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
                            k =0;
                            imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].ypos;
                            imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].height;
                            imageWitdthStart = 46;
                            imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootNumberTablInfo[3].witdh;
                            
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
                            #endif
                            break;
                        }
                        procTime = procTime - 10;
                        sys_task_security_ms_delay(10, TASK_UI);
                    }
                } else { // 大于等于100时也需要释放MCU 1ms
                    sys_task_security_ms_delay(1, TASK_UI);
                }
            }
            if (n == pressTime) {
                amoled_display_clear();
				handle_time_stamp_trush_Long_Press_Reset();
                if (BleCongested_Read() == true) {
                    BleCongested_Write(false);
                    BleData_update_to_flash();
                }

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT) //bug2100523
                // Save event/session for bug2100523
                event_index_insert();
                session_index_insert();
                //如果建立了链接 先断开链接，然后再重启
                if(app_server_context.bt_conn_id)
                {
                    wiced_bt_gatt_disconnect(app_server_context.bt_conn_id);
                    vTaskDelay(50);
                } 
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
                driver_amoled_deinit();
                // 等待到13S ，因为充电IC 最大的复位时间为12.8S, 前面按键已按下9秒，如果按键在按下的前提下需再等待4S，如果已弹起，则直接复位
                msTick = msTickDev->read( (uint8_t*)&msTick, 4);
                keyDev->read( (uint8_t*)&buf, 3);
                while (buf[0] == 1 || buf[1] == 1) {
                    sys_task_security_ms_delay(100, TASK_UI);
                    keyDev->read( (uint8_t*)&buf, 3);
                    if (msTickDev->read( (uint8_t*)&msTick, 4) - msTick > 4000) {
                        break;
                    }
                }
                uint8_t cfg = CHG_SET_POWER_RST;
                ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
                chgDev->write((uint8_t *)&cfg, 1);
                return;
            }
        } else {
            for (int n = 0; n < REBOOT_CLICK_LEN; n++) {
                    keyDev->read( (uint8_t*)&buf, 3);
                    if (buf[0] == 1 || buf[1] == 1) { //重新提示
                        cnt = 0;
                        break;
                    }
                    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
                    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//                    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].addr;
//                    ptQspiflash.data = g_tImageSecondRam;
//                    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].height * UI_COLOR_DEPTH;
//                    
//                    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                        return;
//                    }
//                    
//                    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
					if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n]),g_tImageSecondRam))
					{
						return;
					}
                    
                    k =0;
                    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].ypos;
                    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].height;
                    imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].xpos;
                    imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[n].witdh;
                    
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
            }
        }

    }

    // 清屏
    
    amoled_display_clear();


}


extern int driver_amoled_deinit(void);

/**
  * @brief  用户复位， 
  * @param  None
  * @return None
  * @note   用err命名，提高优先级
  */
void amoled_display_err_reboot_countDown(void)
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
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    uint32_t procTime = 0;
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    uint8_t buf[3];

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

    // 添加 按键提示图
    uint32_t index = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablLen - 1; // 调取最后一页手势
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index]),g_tImageSecondRam))
	{
		return;
	}

    
    k =0;
    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].ypos;
    imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].height;
    imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].xpos;
    imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootClickTablInfo[index].witdh;
    
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


    // 播放倒计时数字,从13开始；用户自己操作 连续按下时 从9开始,硬件触发时间为11s
    int numDownStart = 9;
//    UiInfo_t *pUiInfo = get_error_high_pro_even();
//    if (pUiInfo->uiStatus == UI_ERR_REBOOT_TIP) {
//        numDownStart = 13 - 1;
//    } 
    int n = 0;
    for ( n = numDownStart; n > 5; n--) {
        if (!sys_get_reset_flag()) { // reset释放触发，跳过后续页面显示
        	break;
        }

        if (UI_ERR_CRITICAL <= get_current_ui_detail_status()) {
            break;
        }
        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].height * UI_COLOR_DEPTH;
//        
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//        
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n]),g_tImageSecondRam))
		{
			return;
		}

    
        k =0;
        imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].ypos;
        imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].height;
        imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].xpos;
        imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootCountTablInfo[n].witdh;

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
        if (procTime < 1000) {
            sys_task_security_ms_delay(1000 - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }
    }

    // 清屏

    amoled_display_clear();

    if (n != 5) {
        sys_task_security_ms_delay(20, TASK_UI);
    }
    if (n == 5) {
		handle_time_stamp_trush_Long_Press_Reset();
        if (BleCongested_Read() == true) {
            BleCongested_Write(false);
            BleData_update_to_flash();
        }
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT) //bug2100523
        // Save event/session for bug2100523
        event_index_insert();
        session_index_insert();
        //如果建立了链接 先断开链接，然后再重启
        if(app_server_context.bt_conn_id)
        {
            wiced_bt_gatt_disconnect(app_server_context.bt_conn_id);
            vTaskDelay(50);
        } 
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
        driver_amoled_deinit();
        // 等待到13S ，因为充电IC 最大的复位时间为12.8S, 前面按键已按下9秒，如果按键在按下的前提下需再等待4S，如果已弹起，则直接复位
        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        keyDev->read( (uint8_t*)&buf, 3);
        while (buf[0] == 1 || buf[1] == 1) {
            sys_task_security_ms_delay(100, TASK_UI);
            keyDev->read( (uint8_t*)&buf, 3);
            if (msTickDev->read( (uint8_t*)&msTick, 4) - msTick > 4000) {
                break;
            }
        }
    	uint8_t cfg = CHG_SET_POWER_RST;
    	ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);
        chgDev->write((uint8_t *)&cfg, 1);
    }

}

/**
  * @brief  系统复位或上电开机播放画面 
  * @param  None
  * @return None
  * @note   
  */
void amoled_display_boot_tip(void)
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
    uint32_t msSendTick;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    uint32_t procTime = 0;
    sm_log(SM_LOG_DEBUG, "ui boot_tip!\r\n");

 
    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd ;
    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    //
    for (int n = 0; n < REBOOT_CONFIRM_LEN; n++) { //
        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].witdh * imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].height * UI_COLOR_DEPTH;
//        
////        sm_log(SM_LOG_ERR, "ui boot_tip! n:%d, addr:%x\r\n", n, ptQspiflash.addr);
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
////            sm_log(SM_LOG_ERR, "ui boot_tip! n:%d\r\n", n);
//            return;
//        }
//        
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n]),g_tImageSecondRam))
		{
			return;
		}

    
        k =0;
        imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].ypos;
        imageHeightEnd = imageHeightStart + imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].height;
        imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].xpos;
        imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageErrTablInfo.imageRebootConfirmTablInfo[n].witdh;
        // 
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
        if (procTime < 41) {
            sys_task_security_ms_delay(41 - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }
    }
    #if 0
    // 调整背光渐灭
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
    #if 0
    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);
    #endif
}



