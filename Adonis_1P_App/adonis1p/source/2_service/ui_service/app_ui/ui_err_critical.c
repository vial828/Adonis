/**
  ******************************************************************************
  * @file    ui_err_critical.c
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

#include "ui_err_critical.h"
#include "ui_base.h"

void amoled_display_err_critical(void)
{
	ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

	ImageInfo_t* imageInfo;
	ImageHeaderInfo_t* imageHeaderInfo;
	imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
	uint32_t procTime = 0;
	uint16_t index = 0;

	msTick = msTickDev->read((uint8_t*)&msTick, 4);

    clear_disp_buff();

	imageInfo = &imageHeaderInfo->imageErrTablInfo.imageWarningTablInfo[0];// 加载警告图标
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#ifndef QR_RENDER_EN
	imageInfo = &imageHeaderInfo->imageErrTablInfo.imageCriticalQrTablInfo[2]; //  // 加载Qr：index=1 黑底白码; index=2 白底黑码
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#else
	ImageInfo_t imageQrInfo;
	draw_qr_code(&imageQrInfo, g_tImageSecondRam, QR_CRITICAL);
	if (false == fill_image_to_main_buff(&imageQrInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#endif
	imageInfo = &imageHeaderInfo->imageErrTablInfo.imageCriticalQrTablInfo[0]; // 添加红白错误标志
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}

	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

    do {
    	sys_task_security_ms_delay(2, TASK_UI);
		//if (UI_ERR_CRITICAL != get_current_ui_detail_status()) { // 由系统切换UI，优先级可调低
		//	break;
		//}

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime > (10000)) { // 显示10s退出
			break;
		}
	} while (1);

    //amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
	//amoled_brightness_set(1, 1, 0); // 恢复亮度
}


