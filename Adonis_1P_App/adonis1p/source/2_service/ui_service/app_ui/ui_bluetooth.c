/**
  ******************************************************************************
  * @file    ui_test.c
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

#include "ui_bluetooth.h"
#include "ui_base.h"

/* 蓝牙连接失败 */
void amoled_disp_ble_failed(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t index = 0;

    //sm_log(SM_LOG_DEBUG, "ui disp ble failed!\r\n");
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    clear_disp_buff();

	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageBluetoothTablInfo[0];
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		return;
	}

	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageFailedTablInfo[0];
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		return;
	}

	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

    do {
		if (UI_BLE_FAILED != get_current_ui_detail_status()) {
			break;
		}
//		if (get_key_press_status()) { // 中途按下按键，退出显示
//			break;
//		}

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime > 5500) { // 显示6s
			break;
		}
		sys_task_security_ms_delay(10, TASK_UI);
	} while (1);

    amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
}
/* 蓝牙已连接 */
void amoled_disp_ble_paired(void)
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
	uint16_t brightness = 0;

    //sm_log(SM_LOG_DEBUG, "ui ble paired!\r\n");
    clear_disp_buff();
	
    page = 38; // 显示约1.5s 共38帧
	for (int n = 0; n < page; n++) {
		if (UI_BLE_PAIRED != get_current_ui_detail_status()) {
			break;
		}

		msTick = msTickDev->read((uint8_t*)&msTick, 4); // 更新帧率计算时间
		
		if (n < PAIRED_LEN - 4) {
			if (n == 0) { /* 加载蓝牙图标 */
				index = 0;
				imageInfo = &imageHeaderInfo->imageBltTablInfo.imageBluetoothTablInfo[index];
				if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
					break;
				}
			}
			index = n;
			imageInfo = &imageHeaderInfo->imageBltTablInfo.imagePairedTablInfo[index];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}
		} else if (n >= (page - 4)) {
			imageInfo = &imageHeaderInfo->imageBltTablInfo.imageBluetoothTablInfo[0];
			if (false == get_image_from_flash(imageInfo, g_tImageSecondRam)) {
				amoled_disp_clear();
				return;
			}
			brightness = (page - n) * 51;
			amoled_brightness_sw_cfg(imageInfo, g_tImageSecondRam, (uint8_t)brightness);
			if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				amoled_disp_clear();
				return;
			}
			
			index = PAIRED_LEN - (page - n);
			imageInfo = &imageHeaderInfo->imageBltTablInfo.imagePairedTablInfo[index];
			//imageInfo->ypos = 46; // clean中调整y坐标为212(该操作修改了表头，注意坐标是否正确)
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
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
	amoled_disp_clear();
}

void amoled_disp_ble_download(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;

    //sm_log(SM_LOG_DEBUG, "ui blt download!\r\n");
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    clear_disp_buff();
#ifndef QR_RENDER_EN
	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageDownloadTablInfo[1]; // 加载Qr：index=0 黑底白码; index=1 白底黑码
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#else
	ImageInfo_t imageQrInfo;
	draw_qr_code(&imageQrInfo, g_tImageSecondRam, QR_DOWNLOAD);
	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageDownloadTablInfo[0]; // Qr 获取定位信息
	imageQrInfo.ypos = imageInfo->ypos; // 调整qr的y坐标
	if (false == fill_image_to_main_buff(&imageQrInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#endif

	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageBluetoothTablInfo[0]; // ble icon
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}

	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

    do {
		if (UI_BLE_DOWNLOAD != get_current_ui_detail_status()) { // 由系统切换UI，优先级可调低
			break;
		}

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime >= 300000) { // 超时300s退出（根据需要可调整）
			break;
		}
		sys_task_security_ms_delay(2, TASK_UI);
	} while (1);

    amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
}
/* 蓝牙连接中 */
void amoled_disp_ble_pairing(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

	ImageInfo_t *imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
	
	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;
    uint32_t disp_dly = DISP_DLY;
    uint16_t brightness = 0;
    uint32_t cnt = 0;

    //sm_log(SM_LOG_DEBUG, "ui blt pairing!\r\n");
    clear_disp_buff();

#ifndef QR_RENDER_EN
	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageDownloadTablInfo[1]; // 提前加载Qr：index=0 黑底白码; index=1 白底黑码
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#else
	ImageInfo_t imageQrInfo;
	draw_qr_code(&imageQrInfo, g_tImageSecondRam, QR_DOWNLOAD);
	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageDownloadTablInfo[0]; // 获取Qr定位信息
	imageQrInfo.ypos = imageInfo->ypos; // 调整qr的y坐标
	if (false == fill_image_to_main_buff(&imageQrInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#endif
	page = 12 * 2 + 1 + 25; // 显示3s 共51帧
    do {
        for (int n = 0; n < page; n++) {
        	if (UI_BLE_PAIRING != get_current_ui_detail_status()) {
        		amoled_brightness_set(0, 5, 100); // 5step * 100ms
        		amoled_disp_clear(); // 清屏
				return;
        	}

    		msTick = msTickDev->read((uint8_t*)&msTick, 4);

			if (n < 25) {
				imageInfo = &imageHeaderInfo->imageBltTablInfo.imagePairingTablInfo[0];
				if (false == get_image_from_flash(imageInfo, g_tImageSecondRam)) {
					amoled_disp_clear();
					return;
				}

				if (n < 12) { // 渐亮
					brightness = (n + 1) * 22;
				} else if (n > 12) { // 渐灭
					brightness = (25 - (n + 1)) * 22;
				} else { // 保持
					brightness = 255;
				}
				if (brightness > 255) {
					brightness = 255;
				}
				amoled_brightness_sw_cfg(imageInfo, g_tImageSecondRam, (uint8_t)brightness);
				if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
					amoled_disp_clear();
					return;
				}

				imageInfo = &imageHeaderInfo->imageBltTablInfo.imageBluetoothTablInfo[0]; // 加载 ble icon
				if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
					amoled_disp_clear();
					return;
				}

				amoled_disp_update((uint8_t*)&g_tImageMainRam);
				/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
				sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

				disp_dly = DISP_DLY * 2;
			} else {
				disp_dly = DISP_DLY;
			}

			procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
			if (procTime < disp_dly) {
				sys_task_security_ms_delay(disp_dly - procTime, TASK_UI);
			} else { // 释放MCU 1ms
				sys_task_security_ms_delay(1, TASK_UI);
			}
    	}

		cnt++;
		if (cnt > 40) { // 显示 120s
			break;
		}
    } while (1);

    amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
}

void amoled_disp_ble_findme(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
	
	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;
	uint32_t disp_dly = 50; // 调整帧率为20帧/s
    uint16_t brightness = 0;
    uint32_t timeout = 0;

	timeout = msTickDev->read((uint8_t*)&timeout, 4);
    //sm_log(SM_LOG_DEBUG, "ui blt findme!\r\n");
    clear_disp_buff();

	page = 20; // 一个循环显示1s，共20帧 * 0.05s * n
    do {
        for (int n = 0; n < page; n++) {
        	if (UI_BLE_FIND_ME != get_current_ui_detail_status()) {
        		amoled_brightness_set(0, 5, 100); // 5step * 100ms
        		amoled_disp_clear();
				return;
        	}

    		msTick = msTickDev->read((uint8_t*)&msTick, 4);

			imageInfo = &imageHeaderInfo->imageBltTablInfo.imageFindMeTablInfo[1]; // 先加载frame
			if (false == get_image_from_flash(imageInfo, g_tImageSecondRam)) {
				amoled_disp_clear();
				return;
			}
			
			if (n < 8) { // 渐亮
				brightness = (n + 1) * 32;
			} else if (n >= (page-8)) { // 渐灭
				brightness = (page - (n + 1)) * 32;
			} else {
				brightness = 255;
			}
			
			if (brightness > 255) {
				brightness = 255;
			}
			amoled_brightness_sw_cfg(imageInfo, g_tImageSecondRam, (uint8_t)brightness);
			if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				amoled_disp_clear();
				return;
			}
			
			imageInfo = &imageHeaderInfo->imageBltTablInfo.imageFindMeTablInfo[0]; // 加载 findme
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
				amoled_disp_clear();
				return;
			}

			amoled_disp_update((uint8_t*)&g_tImageMainRam);
			/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
			sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成
			
			procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
			if (procTime < disp_dly) {
				sys_task_security_ms_delay(disp_dly - procTime, TASK_UI);
			} else { // 释放MCU 1ms
				sys_task_security_ms_delay(1, TASK_UI);
			}
		}
        // 上位机设置时间最大255s < 300s,由系统决定何时结束显示
        if ((msTickDev->read((uint8_t*)&timeout, 4) - timeout) > 300000) {
			amoled_brightness_set(0, 5, 100); // 5step * 100ms
			amoled_disp_clear(); // 清屏
			return; // 超时退出
		}
	} while (1);

	amoled_disp_clear();
}

void amoled_disp_ble_lock(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t index = 0;

    clear_disp_buff();

	index = 1;
	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageLockTablInfo[index];
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}

	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

	msTick = msTickDev->read((uint8_t*)&msTick, 4);
    do {
		if (UI_BLE_LOCK != get_current_ui_detail_status()) {
			//amoled_brightness_set(0, 5, 100); // 5step * 100ms
			//amoled_disp_clear();
			break;
		}

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime > 1500) {
			break;
		}
		sys_task_security_ms_delay(10, TASK_UI);
	} while (1);

    amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
}

void amoled_disp_ble_unlock(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t index = 0;

    clear_disp_buff();

	index = 0;
	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageLockTablInfo[index];
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}

	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

	msTick = msTickDev->read((uint8_t*)&msTick, 4);
    do {
		if (UI_BLE_UNLOCK != get_current_ui_detail_status()) {
			//amoled_brightness_set(0, 5, 100); // 5step * 100ms
			//amoled_disp_clear();
			break;
		}

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime > 1500) {
			break;
		}
		sys_task_security_ms_delay(10, TASK_UI);
	} while (1);

    amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
}

static bool draw_loading_cycle(uint8_t* data, uint16_t w, uint16_t h, int angle, uint8_t mode)
{
	#define BRUSH_WIDTH (3) // 笔刷宽度 n>=1; n*n

	if (data == NULL) {
		return false;
	}

	if ((w == 0) || (h == 0)) {
		return false;
	}

	if (angle >= 360) {
		return false;
	}

	int i, j, n, m;
	int canvasSize = w * h * UI_COLOR_DEPTH;
    // 计算角度对应圆弧上的坐标（x,y）
    float r  = (float)(w / 2 - (BRUSH_WIDTH-1));
    float rx = (float)(w / 2);
    float ry = (float)(w / 2);
    float rad = (float)(angle) * 3.1415926f / 180.0f;

    uint16_t x = (uint16_t)(rx + r * sin(rad));// + 0.5f
    uint16_t y = (uint16_t)(ry - r * cos(rad));// + 0.5f
	//sm_log(SM_LOG_DEBUG, "angle:%d, %d, %d, %d\r\n", angle, x, y, mode);

	for (i=0; i<BRUSH_WIDTH; i++) {
		if ((angle >= 90) && (angle < 270)) {
			n = (y + i) * h;
		} else {
			n = (y - i) * h;
		}
		for (j=0; j<BRUSH_WIDTH; j++) {
		    if (angle < 180) {
		    	m = (x + j) + n;
			} else {
				m = (x - j) + n;
			}
		    m *= UI_COLOR_DEPTH;

		    if (mode%2 == 0) {
//		    	if (angle == 0) {
//		    		memset(&data[0], 0x00, canvasSize);
//		    	}
		    	memcpy(&data[m], &data[m + canvasSize], UI_COLOR_DEPTH);
		    } else {
		    	memset(&data[m], 0x00, UI_COLOR_DEPTH);
		    }
		}
	}

	return true;
}

void amoled_disp_loading(void)
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
    static uint16_t playCnt = 0;
    static int angle = 0;

    //sm_log(SM_LOG_DEBUG, "ui loading!\r\n");

    clear_disp_buff();

	imageInfo = &imageHeaderInfo->imageBltTablInfo.imageLoadingTablInfo[0];
	if (false == get_image_from_flash(imageInfo, g_tImageSecondRam)) { // 由flash读取图片到缓冲区
		sm_log(SM_LOG_DEBUG, "Get image err!\r\n");
	}
	int canvasSize = imageInfo->witdh * imageInfo->height * UI_COLOR_DEPTH;
	//memset(&g_tImageSecondRam[canvasSize], 0xFF, (canvasSize)); // debug 用于测试 20241202
	memmove(&g_tImageSecondRam[canvasSize], &g_tImageSecondRam[0], (canvasSize)); // 缓冲区备份图片
	
    do {
		if (UI_LOADING != get_current_ui_detail_status()) {
			index = 0;
			playCnt = 0;
			break;
		}

		switch (index) {
		case 0:
			memset(&g_tImageSecondRam[0], 0x00, canvasSize); // 清主画布
			procTime = 45;
			angle= 0;
			page = 1;
			index++;
			break;
			
		case 1:
			draw_loading_cycle(g_tImageSecondRam, imageInfo->witdh, imageInfo->height, angle, 0);
			
			if (angle < 96) { // 前12帧
				page = 8;
				procTime = 6;
			} else if (angle < 336) { // 中间6帧
				page = 40;
				procTime = 1;
			} else { // 后3帧
				page = 8;
				procTime = 6;
			}
			
			angle++;
			if (angle >= 360) {
				angle = 0;
				index++;
			}
			break;

		case 2:
			procTime = 45;
			cnt++;
			if (cnt >= 10) {
				cnt = 0;
				index++;
			}
			break;

		case 3:
			draw_loading_cycle(g_tImageSecondRam, imageInfo->witdh, imageInfo->height, angle, 1);
			if (angle < 96) { // 前12帧
				page = 8;
				procTime = 6;
			} else if (angle < 336) { // 中间6帧
				page = 40;
				procTime = 1;
			} else { // 后3帧
				page = 8;
				procTime = 6;
			}
			
			angle++;
			if (angle >= 360) {
				angle = 0;
				index++;
			}
			break;

		default:
			index = 0;
			procTime = 45;
			playCnt++;
			break;	
		}
   
    	if (angle % page == 0) {
    		fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE);
    		amoled_disp_update(g_tImageMainRam);
    	}
#if 0 // 由系统决定何时结束显示
    	if (playCnt >= 24*5) { // 播放超时约5min退出
    		playCnt = 0;
    		index = 0;
    		//sm_log(SM_LOG_DEBUG, "Loading ui is over!\r\n");
    		break;
    	}
#endif
		sys_task_security_ms_delay(procTime, TASK_UI);
	} while (1);

	amoled_disp_clear();
}

/**
* @brief  显示加热曲线设置模式
* @param  None
* @return None
* @note   None
*/
// Gets the custom profile flag from ble settings
#include "app_bt_char_adapter.h"
extern bool IsCustomHeatingProfile(void);
extern bool IsCustomHeatingProfileBoost(void);
void amoled_display_set_profile(ProfileUI_e page)
{
	ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

	ImageInfo_t* imageInfo;
	ImageHeaderInfo_t* imageHeaderInfo;
	imageHeaderInfo = get_image_header_info_handle();
	ImageInfo_t imageDot;

	uint32_t msTick = 0;
	uint32_t procTime = 0;
	uint16_t index = 0;

	msTick = msTickDev->read((uint8_t*)&msTick, 4);
	clear_disp_buff();
	/** point to image profile page*/
	switch (page) {
	case PROFILE_UI_STANDARD:
		index = 1;
		imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageProfileTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			amoled_disp_clear();
			return;
		}
		// Gets the custom profile flag from ble settings, 用于抹除白色标记区域
		if (false == IsCustomHeatingProfile()) { // the profile==default is false;
			imageDot.height = 10;
			imageDot.witdh = imageDot.height;
			imageDot.xpos = 44;
			imageDot.ypos = 219;
			memset(g_tImageSecondRam, 0x00, (imageDot.height * imageDot.witdh * UI_COLOR_DEPTH));
			fill_image_to_main_buff(&imageDot, g_tImageSecondRam, g_tImageMainRam, MODE_NONE);
		}
		break;

	case PROFILE_UI_BOOST:
		index = 0;
		imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageProfileTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			amoled_disp_clear();
			return;
		}
		// Gets the custom profile flag from ble settings, 用于抹除白色标记区域
		if (false == IsCustomHeatingProfileBoost()) { // the profile==default is false;
			imageDot.height = 10;
			imageDot.witdh = imageDot.height;
			imageDot.xpos = 49;
			imageDot.ypos = 210;
			memset(g_tImageSecondRam, 0x00, (imageDot.height * imageDot.witdh * UI_COLOR_DEPTH));
			fill_image_to_main_buff(&imageDot, g_tImageSecondRam, g_tImageMainRam, MODE_NONE);
		}
		break;

	default:
		amoled_disp_clear();
		sm_log(SM_LOG_DEBUG, "no page!\r\n");
 		return; // break;
	}

#if 0
	if (PROFILE_UI_NONE != page) {
		if (PROFILE_UI_BOOST == page) {
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageBoostIconTablInfo[0]; // boost icon
		} else {
			imageInfo = &imageHeaderInfo->imageHeatTablInfo.imageStandardIconTablInfo[0]; // standard icon
		}
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			amoled_disp_clear();
			return;
		}
	}
#endif
	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

	do {
		if (PROFILE_UI_STANDARD == page) {
			if (UI_SET_PROFILE != get_current_ui_detail_status()) {
				break;
			}
		} else {
			if (UI_SET_PROFILE_BOOST != get_current_ui_detail_status()) {
				break;
			}
		}
		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime > 3500) { // 加上渐灭0.5s，共4s
			break;
		}
		sys_task_security_ms_delay(10, TASK_UI);
	} while (1);

	amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
}

void amoled_display_confirmation(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint16_t page = 0;

	clear_disp_buff();

	page = OUT_SHIPPING_LEN; // 显示1.52s, 共38帧
    for (int n = 0; n < page; n++) {
        if (UI_NONE == get_current_ui_detail_status()) {
            break;
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);

		imageInfo = &imageHeaderInfo->imageErrTablInfo.imageOutShippingTablInfo[n];
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

    amoled_disp_clear();
}
/*********************************************************************************/
