/**
  ******************************************************************************
  * @file    ui_heat.c
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
  * 2024-10-11      V0.10      GLO_AD1P_UI_2024.10.11.1
  ******************************************************************************
  */

#include "platform_io.h"
#include "data_base_info.h"
#include "sm_log.h"
#include <stdlib.h>
#include "ui_heat.h"
#include "task_heatting_service.h"
#include "task_system_service.h"
#include "err_code.h"
#include "ui_base.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT) 
#include "app_bt_char_adapter.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

// Gets the custom profile flag from ble settings
#include "app_bt_char_adapter.h"
extern bool IsCustomHeatingProfile(void);
#define CUSTOM_BASE_PROFILE_FLAG  IsCustomHeatingProfile() // the profile == default is false;

static bool fill_heating_mode_image_to_main_buff(SysStatus_u sysStatus, uint8_t index)
{
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	if (HEATTING_BOOST == sysStatus) {
		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageBoostIconTablInfo[index];
	} else { // if (HEATTING_STANDARD == sysStatus)
		if (CUSTOM_BASE_PROFILE_FLAG == false) {// Gets the custom profile flag: true or false
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageStandardIconTablInfo[index];
		} else {
			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageStandardIconCustomTablInfo[index]; // white mark page
		}
	}

	return fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE);
}

void amoled_display_clear(void) // 清屏
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
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

/**
  * @brief  显示hi动画
  * @param  None
* @return None
* @note
*/

void amoled_display_heat_hi(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;
	uint16_t cnt = 0;

	clear_disp_buff();

	page = HI_LEN + 20; // 显示2s 共50帧
    for (int n = 0; n < page; n++) {
        if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
    		amoled_disp_clear();
            return;
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);
        
		if (n < 23) {
			index = n;
		} else if (n < 43){
			index = 23 - 1;
		} else {
			index = n - 20;
		}

		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHiTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}

		amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime < DISP_DLY) {
			sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
		} else { // 释放MCU 1ms
			sys_task_security_ms_delay(1, TASK_UI);
		}
    }

    // amoled_disp_clear();
}

/**
  * @brief  显示mode模式动画
  * @param  mode:0-base模式动画; 1-boost模式动画
* @return None
* @note
*/
amoled_display_hi_to_preheat(SysStatus_u sysStatus)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;
	uint16_t cnt = 0;

	clear_disp_buff();

	page = HI_TO_PREHEAT_LEN; // 显示1s 共25帧
    for (int n = 0; n < page; n++) {
        if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
            return;
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);

        index = n;
		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHiToPreheatTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			amoled_disp_clear();
			return;
		}

        if (n >= 18) { // 叠加加热模式图标
        	index = (page - n); // 倒播
        	if (false == fill_heating_mode_image_to_main_buff(sysStatus, index)) {
				amoled_disp_clear();
				return;
        	}
        }

        amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

        procTime = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
        if (procTime < DISP_DLY) {
            sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }
    }
}

/**
  * @brief  预热的动画
  * @param  None
* @return None
* @note   预热2秒, bar高度139
*/
void amoled_display_preheat(SysStatus_u sysStatus)
{
	SysStatus_u tempSysStatus = get_system_status();
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;

	clear_disp_buff();
    /* 预加载 加热模式图标 */
    index = (11 - 1);
	if (false == fill_heating_mode_image_to_main_buff(sysStatus, index)) {
		amoled_disp_clear();
		return;
	}
	/* preheat预处理 */
	HEATER* pt_heatManagerInfo = get_heat_manager_info_handle();
	imageInfo = &imageHeaderInfo->imageHeatTablInfo.imagePreheatBarTablInfo[1];
	int maxHeigh = imageInfo->height; // 获取bar高度 139 pixel
    int ellipseHeight = 3; // bar上半部分的椭圆圆弧高度
    int moveTargetRow = imageInfo->height; // 移动到目标行
    uint32_t preheatTime = pt_heatManagerInfo->PreHeatTime - 3000; // 预热bar显示时间2s
    uint32_t moveOneRowTime = preheatTime * 4 / imageInfo->height; // 扩展刷新间隔时间，否则刷不过来
    uint32_t msTickInterval = 0;
	msTickInterval = msTickDev->read((uint8_t*)&msTickInterval, 4);
	//page = 1; // 先准备刷新一次页面
    while (1) {
        if (UI_HEATTING < get_current_ui_detail_status()) {
            return;
        }

        tempSysStatus = get_system_status();
        msTick = msTickDev->read( (uint8_t*)&msTick, 4);

        // 移动掩码
        if (sysStatus != tempSysStatus) {
        	moveTargetRow += 10;
        	if (moveTargetRow > maxHeigh) {
            	moveTargetRow = maxHeigh;
        	}
        	page = 2; // 准备刷新进度条
        } else {
			if ((msTickDev->read((uint8_t*)&msTickInterval, 4) - msTickInterval) >= moveOneRowTime) { // 目标行+4
				msTickInterval = msTickDev->read((uint8_t*)&msTickInterval, 4);
				moveTargetRow -= 4;
				if (moveTargetRow < 4) {
					moveTargetRow = 4;
				}
				page = 1; // 准备刷新进度条
			} else {
				//continue;
			}
        }

        if (page != 0) {
			// 读取bar
			memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imagePreheatBarTablInfo[1];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				amoled_disp_clear();
				return;
			}
			// 读取掩码，掩码放最后,imageHeaderInfo->imageHeatTablInfo.imageConsumeBarTablLen -1处
			memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imagePreheatBarTablInfo[2];
			get_image_from_flash(imageInfo, g_tImageSecondRam); // 由flash读取图片到缓冲区

			int wlen = imageInfo->witdh;
			// 把椭圆圆弧放到目标行, 优先拷贝到尾部，避免重叠区误覆盖, 算法时间复杂度O(n)
			for (int i=0 ; i<(ellipseHeight*wlen*UI_COLOR_DEPTH); i++) {
				g_tImageSecondRam[moveTargetRow * wlen * UI_COLOR_DEPTH - 1 - i] =
					g_tImageSecondRam[ellipseHeight * wlen * UI_COLOR_DEPTH - 1 - i];
			}
			// 在新椭圆弧之前补充黑色O(n)
			memset(g_tImageSecondRam, 0, (moveTargetRow - ellipseHeight) * wlen * UI_COLOR_DEPTH);
			if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_AND)) {
				amoled_disp_clear();
				return;
			}
			// 读取外框
			memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imagePreheatBarTablInfo[0];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
				amoled_disp_clear();
				return;
			}

			amoled_disp_update((uint8_t*)&g_tImageMainRam);
			/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
			sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

			procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick; // 花费时间
			if (procTime < DISP_DLY) {
				sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
			} else { // 大于等于100时也需要释放MCU 1ms
				sys_task_security_ms_delay(1, TASK_UI);
			}
			if (page == 1) {
				if (moveTargetRow <= 4) {
					//sm_log(SM_LOG_DEBUG, "preheat time:%d\r\n", (msTickDev->read((uint8_t*)&tick, 4)-tick));
					return;
				}
			} else {
				if (moveTargetRow >= maxHeigh) {
					break;
				}
			}
			page = 0;
		}
    }
    // 预热结束，衔接拔烟
    page = PREHEAT_TO_END_LEN; // 显示0.8s,共20帧
    for (int n=0; n<page; n++) { //
        if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
            return;
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);

        clear_disp_buff();

		index = n;
		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imagePreheatToEndTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			amoled_disp_clear();
			return;
		}

        if (n < 6) { // 加载加热模式动画
        	index = n + 2; // 从mode第3页开始刷
        	if (false == fill_heating_mode_image_to_main_buff(sysStatus, index)) {
				amoled_disp_clear();
				return;
        	}
        }

		amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime < DISP_DLY) {
			sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
		} else { // 释放MCU 1ms
			sys_task_security_ms_delay(1, TASK_UI);
		}
    }
}

/**
  * @brief  显示consume过渡动画
  * @param  None
* @return None
* @note
*/
void amoled_display_preheat_to_consume(SysStatus_u sysStatus)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;

	clear_disp_buff();
    /* 预加载 加热模式图标 */
    index = (11 - 1);
	if (false == fill_heating_mode_image_to_main_buff(sysStatus, index)) {
		amoled_disp_clear();
		return;
	}

    page = PREHEAT_TO_HEAT_LEN; // 显示0.88s 共22帧
    for (int n=0; n<page; n++) {
        if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生,发生错误
            return;
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);

        imageInfo = &imageHeaderInfo->imageHeatTablInfo.imagePreheatToHeatTablInfo[n];
   		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}

        amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

        procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick; // 花费时间
        if (procTime < DISP_DLY) {
            sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }
    }
}

/**
  * @brief  显示加热过程动画
  * @param  None
* @return None
* @note   预热5秒后开始进入加热310 - 0.88秒, bar高度170， 启动消失时高度剩0， 因此310 - 0.88秒跑170高度 
*/
void amoled_display_heat_consume_all(SysStatus_u sysStatus)
{
	SysStatus_u tempSysStatus = get_system_status();
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;
	uint16_t cnt = 0;
	uint16_t disp_dly = DISP_DLY*2; // 0.08s

	/* preheat预处理 */
	imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHeatBarTablInfo[1];
	int maxHeigh = imageInfo->height; // 获取bar高度 170 pixel
    int ellipseHeight = 17; // bar上半部分的椭圆圆弧高度
    int moveTargetRow = ellipseHeight; // 移动到目标行
    int maxMoveRow = imageInfo->height + ellipseHeight; // 圆弧部分需要全部移动到170高度的外面，
    // 加热时间:standard 315-5s，boost 225-5s
    HEATER* pt_heatManagerInfo = get_heat_manager_info_handle();
    uint32_t heatTime = 0; // 获得总加热时间
    uint32_t moveOneRowTime = 0; // 移动一个像素的间隔周期
    uint32_t oneRowTimeCycle = 0; // 间隔周期内UI刷新次数
    uint32_t rowCntVal = 0;
    uint32_t moveRowCnt = 0;
    uint32_t msSmokeTick = 0;
    uint32_t msTickInterval = 0;
    uint32_t tick = 0;
	tick = msTickDev->read((uint8_t*)&tick, 4);

    heatTime = pt_heatManagerInfo->TotalHeatTime - pt_heatManagerInfo->PreHeatTime - 880; // 减去过渡到加热动画时间 0.88s;
    moveOneRowTime = heatTime / maxHeigh; //总共移动170行像素， 从17行起到170+17
    oneRowTimeCycle = moveOneRowTime / disp_dly;
    rowCntVal = (heatTime / disp_dly) - (maxHeigh * oneRowTimeCycle);
    rowCntVal = maxHeigh - rowCntVal; // (x+y=170)
	moveOneRowTime = oneRowTimeCycle * disp_dly;
    sm_log(SM_LOG_DEBUG, "heatTime:%d, oneRowTime:%d, cntx:%d\r\n", heatTime, moveOneRowTime, rowCntVal);

    msSmokeTick = msTickDev->read((uint8_t*)&msSmokeTick, 4);
    msTickInterval = msTickDev->read((uint8_t*)&msTickInterval, 4);
    page = 1; // 先准备刷新一次页面
    while ((msTickDev->read((uint8_t*)&msSmokeTick, 4) - msSmokeTick) < heatTime) {
        if (UI_HEATTING < get_current_ui_detail_status()) {
            return;
        }

        tempSysStatus = get_system_status();
        msTick = msTickDev->read((uint8_t*)&msTick, 4);

        // 移动掩码
		if (sysStatus != tempSysStatus) {
			moveTargetRow += 15;
			if (moveTargetRow >= maxMoveRow) {
				moveTargetRow = maxMoveRow;
			}
			page = 2; // 准备刷新进度条
			disp_dly = DISP_DLY; // 0.04s
		} else {
			if ((msTickDev->read((uint8_t*)&msTickInterval, 4) - msTickInterval) >= moveOneRowTime) { // 目标行+1
				msTickInterval = msTickDev->read((uint8_t*)&msTickInterval, 4);
				moveRowCnt++;
				if (moveRowCnt >= rowCntVal) {
					moveOneRowTime = (oneRowTimeCycle + 1) * disp_dly;
				}
				moveTargetRow++;
				page = 1; // 准备刷新进度条
				disp_dly = DISP_DLY*2; // 0.08s
			} else {
				//continue;
			}
		}
		/* 加载 加热进入条动画 */
		if (page != 0) {
			// 读取bar
			//memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHeatBarTablInfo[1];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				amoled_disp_clear();
				return;
			}
			// 读取掩码，掩码放最后,imageHeaderInfo->imageHeatTablInfo.imageConsumeBarTablLen -1处
			memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHeatBarTablInfo[2];
			get_image_from_flash(imageInfo, g_tImageSecondRam); // 由flash读取图片到缓冲区

			int wlen = imageInfo->witdh;
			// 把椭圆圆弧放到目标行, 优先拷贝到尾部，避免重叠区误覆盖, 算法时间复杂度O(n)
			for (int i=0; i<(ellipseHeight*wlen*UI_COLOR_DEPTH); i++) {
				g_tImageSecondRam[moveTargetRow * wlen * UI_COLOR_DEPTH - 1 - i] =
					g_tImageSecondRam[ellipseHeight * wlen * UI_COLOR_DEPTH - 1 - i];
			}
			// 在新椭圆弧之前补充黑色O(n)
			memset(g_tImageSecondRam, 0, (moveTargetRow - ellipseHeight) * wlen * UI_COLOR_DEPTH);
			if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_AND)) {
				amoled_disp_clear();
				return;
			}
			// 读取外框
			memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHeatBarTablInfo[0];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
//				sm_log(SM_LOG_DEBUG, "image info:%d, %d, %d, %d\r\n",
//						imageInfo->height,imageInfo->witdh,imageInfo->xpos,imageInfo->ypos);
				amoled_disp_clear();
				return;
			}
		}
        /* 加载 加热模式动画 */
		if ((page >= 2) || (moveTargetRow >= maxMoveRow - 1)) { // 接近结束不闪动火苗动画
			if (index > 10) {
				index--;
			}
		} else {
			if (cnt < 11) {
				index = 10 + cnt;
			} else if (cnt < 21) {
				index = 30 - cnt;
			}
			cnt++;
			if (cnt >= 21) {
				cnt = 0;
			}
		}
		if (false == fill_heating_mode_image_to_main_buff(sysStatus, index)) {
			amoled_disp_clear();
			return;
		}

    	page = 0;
        amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

        procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick; // 花费时间
        if (procTime < disp_dly) {
            sys_task_security_ms_delay(disp_dly - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }

        if (moveTargetRow >= maxMoveRow) {
        	sm_log(SM_LOG_DEBUG, "heat time0:%d\r\n", (msTickDev->read((uint8_t*)&tick, 4) - tick));
        	//sm_log(SM_LOG_DEBUG, "ui heat bar end:%d!\r\n", moveTargetRow);
            return;
        }
    }
    sm_log(SM_LOG_DEBUG, "heat time1:%d\r\n", (msTickDev->read((uint8_t*)&tick, 4) - tick));
}

/**
  * @brief  显示加热结束
  * @param  None
* @return None
* @note
*/
void amoled_display_heat_end(SysStatus_u sysStatus)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;

    clear_disp_buff();

    page = HEAT_TO_END_LEN; // 显示1.2s,共30帧
    for (int n=0; n<page; n++) { //
        if (UI_HEATTING < get_current_ui_detail_status()) { // 加热过程发生,发生错误
            return;
        }

        msTick = msTickDev->read( (uint8_t*)&msTick, 4);

        clear_disp_buff();

		index = n;
		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageHeatToEndTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			amoled_disp_clear();
			return;
		}
		// 加载加热模式动画
		if (n < 15) {
	        if (n < 5) {
	        	index = 10;
	        } else if (n >= 5) {
	        	index = n - 5;
	        }
	    	if (false == fill_heating_mode_image_to_main_buff(sysStatus, index)) {
				amoled_disp_clear();
				return;
    		}
		}

		amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime < DISP_DLY) {
			sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
		} else { // 释放MCU 1ms
			sys_task_security_ms_delay(1, TASK_UI);
		}
    }
}

/**
  * @brief  显示拔烟动画，循环2次
  * @param  None
* @return None
* @note   None
*/
void amoled_display_heat_dispose(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;

    if (sys_get_reset_flag()) { // reset触发，跳过后续页面显示
    	amoled_disp_clear();
		return;
    }

    clear_disp_buff();

    for (int i=0; i<2; i++) { // 播放两次 1.68s * 2
    	page = 42; // 显示1.68s 共36帧
        for (int n=0; n<page; n++) {
            if (sys_get_reset_flag()) { // reset触发，跳过后续页面显示
            	amoled_disp_clear();
        		return;
            }
            if (UI_NONE == get_current_ui_detail_status()) {
            	amoled_disp_clear();
        		return;
            }
            if (UI_HEATTING < get_current_ui_detail_status()) {
            	amoled_disp_clear();
        		return;
            }

            msTick = msTickDev->read( (uint8_t*)&msTick, 4);
            
            memset(g_tImageMainRam, 0x00, IMAGE_SEZE);

            if (n < 6) {
            	index = 29; // 显示第30页圆圈 保持6帧
            } else if (n < 11) {
            	index = n - 6;
            } else if (n < 18) {
            	index = (5 - 1); // 显示第5页 保持7帧
            } else {
            	index = 5 + (n - 18);
            }
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageDisposeTablInfo[index];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}

    		amoled_disp_update((uint8_t*)&g_tImageMainRam);
    		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
    		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

    		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
    		if (procTime < DISP_DLY) {
    			sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
    		} else { // 释放MCU 1ms
    			sys_task_security_ms_delay(1, TASK_UI);
    		}
        }
    }
}

void amoled_display_dispose_to_bye(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;

    clear_disp_buff();
    page = 14; // 显示0.56s 共14帧(过度动画，衔接bye页面的小球轨迹)
    for (int n=0; n<page; n++) {
        if (sys_get_reset_flag()) { // reset触发，跳过后续页面显示
        	amoled_disp_clear();
    		return;
        }
        if (UI_NONE == get_current_ui_detail_status()) {
        	amoled_disp_clear();
    		return;
        }
        if (UI_HEATTING < get_current_ui_detail_status()) {
        	amoled_disp_clear();
    		return;
        }

        msTick = msTickDev->read( (uint8_t*)&msTick, 4);

        memset(g_tImageMainRam, 0x00, IMAGE_SEZE);

        index = 29 + n;
 		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageDisposeTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}

		amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime < DISP_DLY) {
			sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
		} else { // 释放MCU 1ms
			sys_task_security_ms_delay(1, TASK_UI);
		}
    }

    if (sys_get_reset_flag() || // reset触发，跳过后续页面显示
    	UI_NONE == get_current_ui_detail_status()) {
    	amoled_disp_clear();
		return;
    }
}

/**
  * @brief  显示byte视频
  * @param  None
* @return None
* @note   1.44秒视频36帧
*/
void amoled_display_bye(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;

    if (sys_get_reset_flag()) { // reset触发，跳过后续页面显示
    	amoled_disp_clear();
    	return;
    }

    clear_disp_buff();

    page = 36; // 显示1.44s 共36帧
    for (int n = 0; n < page; n++) {
        memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
        if (UI_NONE == get_current_ui_detail_status()) {
            break;
        }

        if (UI_HEATTING < get_current_ui_detail_status()) {
            break;
        }

        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        /* 加载bye动画 */
		if (n < 22) {
			index = n;
		} else if (n < page - 2) {
			index = 21; // 第21页保持12帧
		} else {
			index = n - 12; // 减去停留的12帧
		}

		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageByeTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}
		/* 加载小球轨迹 */
		index = n;
		imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageBallTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
			break;
		}

		amoled_disp_update((uint8_t*)&g_tImageMainRam);
		/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
		sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime < DISP_DLY) {
			sys_task_security_ms_delay(DISP_DLY - procTime, TASK_UI);
		} else { // 释放MCU 1ms
			sys_task_security_ms_delay(1, TASK_UI);
		}
    }

    amoled_disp_clear(); // 清屏
    //sys_task_security_ms_delay(20, TASK_UI);
}

/* 播放客制化hi、name和bye的页面，mode控制显示的退出机制; 播放时间需大于渐变帧数的两倍; clear设置非0，播放结束会清屏; */
void amoled_display_customize(DispMode_e mode, CustomizeUI_e page, uint32_t time, uint8_t clear)
{
	ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
	ImageInfo_t imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    //SysStatus_u tempSysStatus = get_system_status();

	uint32_t msTick = 0;
	uint32_t procTime = 0;
	uint32_t index = 0;
	uint32_t disp_dly = 55; // 如果整屏运算灰度，最大帧率的刷新间隔最好为55ms,18帧每秒
	uint32_t disp_dlyTmp = 0;
	uint16_t brightness = 0;
	uint16_t brightness_step = 0;
	uint16_t fps = 7; // 渐亮或渐灭的帧数
	bool getDataFromFlashAddrType = 0; // 0:相对地址; 1:绝对地址
	//uint32_t tick = msTickDev->read((uint8_t*)&tick, 4);
	msTick = msTickDev->read((uint8_t*)&msTick, 4);

	clear_disp_buff();

	imageInfo.witdh = 90;
	imageInfo.height = 282;
	imageInfo.xpos = 0;
	imageInfo.ypos = 0;
	
	index = time / disp_dly;
	if (index > (fps * 2)) { // 渐亮渐灭分别需要n帧 * 55ms
		disp_dlyTmp = time - index * disp_dly;
	} else {
		index = fps * 2;
		disp_dlyTmp = 0;
	}
	brightness_step = (255 / fps) + 1; // 最大亮度255
	/** point to image ext-flash addr to dispaly user photo */
	switch (page) {
	case CUSTOMIZE_UI_HI: // defalt；1.0s
		imageInfo.addr = MyName_uImage_Intro_Addr_Get(); //0x1E20000+35;
		getDataFromFlashAddrType = 1; // 绝对地址
		break;

	case CUSTOMIZE_UI_NAME: // defalt；1.0s
		imageInfo.addr = MyName_uImage_Greeting_Addr_Get(); //0x1E20000+35;
		getDataFromFlashAddrType = 1; // 绝对地址
		break;

	case CUSTOMIZE_UI_BYE: // defalt；1.44s
		imageInfo.addr = MyName_uImage_Outro_Addr_Get(); //0x1E20000+35;
		getDataFromFlashAddrType = 1; // 绝对地址
		break;

	case CUSTOMIZE_UI_HI_DEFAULT: // defalt；1.0s
		memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageHiByeTablInfo[0], sizeof(ImageInfo_t));
		break;

	case CUSTOMIZE_UI_BYE_DEFAULT: // defalt；1.44s
		memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageHiByeTablInfo[1], sizeof(ImageInfo_t));
		break;

	default:
		sm_log(SM_LOG_DEBUG, "Err page!\r\n");
 		return; // break;
	}

	procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
	if (procTime >= disp_dlyTmp) {
		disp_dlyTmp = 0;
	} else {
		disp_dlyTmp -= procTime;
	}
	//sm_log(SM_LOG_DEBUG, "ui index:%d, dlyTmp:%d\r\n", index, disp_dlyTmp);
	for (int n=0; n<index; n++) {
		if (DISP_HEAT == mode) { // 加热显示控制
			if (UI_HEATTING < get_current_ui_detail_status()) { // 预热过程发生页面切换
				break;
			}
		} else { // DISP_OTA OTA显示控制
			if (UI_CUSTOMIZE != get_current_ui_detail_status()) {
				break;
			}
		}
		msTick = msTickDev->read((uint8_t*)&msTick, 4);

		if ((n < fps) || (n >= (index - fps))) {
			if (getDataFromFlashAddrType == 0) { // 相对地址读取数据
				if (false == get_image_from_flash(&imageInfo, g_tImageSecondRam)) {
					amoled_disp_clear();
					return;
				}
			} else { // 绝对地址读取数据
				if (false == get_image_from_flash_absolute_addr(&imageInfo, g_tImageSecondRam)) {
					amoled_disp_clear();
					return;
				}
			}
			if (n < fps) {
				brightness = (n + 1) * brightness_step;
			} else {
				brightness = (index - n - 1) * brightness_step;
			}
			if (brightness > 255) {
				brightness = 255;
			}
			amoled_brightness_sw_cfg(&imageInfo, g_tImageSecondRam, (uint8_t)brightness);
			if (false == fill_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				amoled_disp_clear();
				return;
			}

			amoled_disp_update((uint8_t*)&g_tImageMainRam);
			/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
			sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成
		} 
		
		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime < disp_dly) {
			sys_task_security_ms_delay(disp_dly - procTime, TASK_UI);
			if (n == fps) { // 渐变结束后 修正一下时间
				sys_task_security_ms_delay(disp_dlyTmp, TASK_UI);
			}
		} else { // 释放MCU 1ms
			sys_task_security_ms_delay(1, TASK_UI);
		}
	}

	if (clear != 0) {
		amoled_disp_clear(); // 单独调用需清屏
	}
	//sm_log(SM_LOG_DEBUG, "ui customize:%dms\r\n", (msTickDev->read((uint8_t*)&tick, 4) - tick));
}


