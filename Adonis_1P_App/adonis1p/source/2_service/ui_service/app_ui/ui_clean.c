/**
  ******************************************************************************
  * @file    ui_clean.c
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
#include "system_status.h"
#include "ui_clean.h"
#include "ui_base.h"

#ifdef DEF_DRY_CLEAN_EN // 启用干烧清洁功能,不使用干烧清洁功能时，把此宏注释掉; 移除自清洁的UI assets for SP

static void amoled_display_number(uint16_t num, uint8_t mode, uint16_t dat)
{
    ImageInfo_t imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    uint16_t index = 0;

    switch (mode) {
    case 0: // 白色圆 + 黑字
		memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageCountDownTablInfo[10], sizeof(ImageInfo_t)); // 加载白色圆圈
		if (false == fill_flash_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}

		if (num > 9) { // 处理十位
			index = num / 10;
			memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageCountDownTablInfo[index], sizeof(ImageInfo_t));
			if (index != 1) {
				imageInfo.xpos = 28; // 直接调整坐标位置(注意：表头坐标是否被修改)
			} else {
				imageInfo.xpos = 25; // 修正数字1的坐标:左移3个像素
			}
			imageInfo.ypos = 225;
			if (false == fill_flash_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_AND)) {
				break;
			}
		}

		index = num % 10; // 处理个位
		memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageCountDownTablInfo[index], sizeof(ImageInfo_t));
		if (num >= 20) {
			imageInfo.xpos = 46;
		} else if (num >= 10) {
			imageInfo.xpos = 43; // 修正数字1的坐标:左移3个像素
		} else {
			if (index != 1) {
				imageInfo.xpos = 38;
			} else {
				imageInfo.xpos = 35; // 修正数字1的坐标:左移3个像素
			}
		}
		imageInfo.ypos = 225;
		if (false == fill_flash_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_AND)) {
			break;
		}
    	break;

    case 1: // 白字+单位s
		if (num > 9) { // 处理十位
			index = num / 10;
			memcpy(&imageInfo, &imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[index], sizeof(ImageInfo_t));
			imageInfo.xpos = 20;
			imageInfo.ypos = 225;
			if (false == fill_flash_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}
		}

		index = num % 10; // 处理个位
		memcpy(&imageInfo, &imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[index], sizeof(ImageInfo_t));
		if (num > 9) {
			imageInfo.xpos = 38; // 调整坐标位置(注意：表头坐标被修改)
		} else {
			imageInfo.xpos = 37;
		}
		imageInfo.ypos = 225;
		if (false == fill_flash_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}
		memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageCountDownTablInfo[11], sizeof(ImageInfo_t)); // 调用单位s
		if (false == fill_flash_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}
    	break;

    case 2: // 白色圆+黑字 渐变
    	memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageCountDownTablInfo[10], sizeof(ImageInfo_t));
		if (false == get_image_from_flash(&imageInfo, g_tImageSecondRam)) {
			break;
		}
		amoled_brightness_sw_cfg(&imageInfo, g_tImageSecondRam, (uint8_t)dat);
		if (false == fill_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}

		memcpy(&imageInfo, &imageHeaderInfo->imageOtherTablInfo.imageCountDownTablInfo[num%10], sizeof(ImageInfo_t));
		imageInfo.xpos = 37;
		if (false == get_image_from_flash(&imageInfo, g_tImageSecondRam)) {
			break;
		}
		amoled_brightness_sw_cfg(&imageInfo, g_tImageSecondRam, (uint8_t)dat);
		if (false == fill_image_to_main_buff(&imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_AND)) {
			break;
		}
    	break;

    default:
    	break;
    }
}
/**
  * @brief  显示清洁进行中
  * @param  None
* @return None
* @note  加热时间固定60s,帧率25帧/s
*/
/* 各组动画的帧数 */
#define PAGE_NUM_OF_STAR_START		(15)
#define PAGE_NUM_OF_CIRCLE_LOOP		(38) // 16-53
#define PAGE_NUM_OF_CIRCLE_TO_END	(15) // 54-68
#define PAGE_NUM_OF_STAR_TO_END		(31) // 69-99

/* 各组动画的索引地址 */
#define INDEX_OF_STAR_START		(1 - 1)
#define INDEX_OF_CIRCLE_LOOP	(16 - 1)
#define INDEX_OF_CIRCLE_TO_END	(54 - 1)
#define INDEX_OF_STAR_TO_END	(69 - 1)

void amoled_display_cleaning(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageInfo_t* imageInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

	uint32_t msTick = 0;
    uint32_t procTime = 0;
    uint32_t cleanTime = 0;
    uint16_t page = 0;
    uint16_t index = 0;
    int n = 0;
    int m = 0;
	uint16_t disp_dly = DISP_DLY*2; // 80ms

    uint16_t allFrameNum = 0; // 显示倒计时的帧数
    uint16_t cntDown = 0;
    static uint16_t cntTime = 0;
    uint16_t adjustFrameNum = 0; // 调节帧;
    uint16_t loopFrameNum = 0;
    uint16_t circleLoop = 0;
    uint16_t tmp = 0;
    uint16_t brightness = 0;
    uint32_t tick = msTickDev->read((uint8_t*)&tick, 4);
	clear_disp_buff();

#if 0
    cleanTime = 60*1000;  // defualt:60s (TBD)
#else
	HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
	HeatTemp_t *P_tHeatTemp = &p_tHeatParamInfo->tHeatCleanTemp[0];
	cleanTime = P_tHeatTemp[14].time*10; // defualt:60s
	//sm_log(SM_LOG_DEBUG, "ui disp clean:%dms\r\n", cleanTime);
#endif
	tmp = PAGE_NUM_OF_STAR_START; // 60s内的非周期动画时间
	allFrameNum = cleanTime / disp_dly; // 计算60s内的帧数
    if (allFrameNum > (tmp + PAGE_NUM_OF_CIRCLE_LOOP)) {
    	circleLoop = (allFrameNum - tmp) / (PAGE_NUM_OF_CIRCLE_LOOP); // 减掉非周期动画时间，计算中间动画的播放次数
    } else {
    	circleLoop = 1; // 至少播放一个周期
    }
    cleanTime /= 1000; // 转为s单位
    loopFrameNum = (circleLoop * PAGE_NUM_OF_CIRCLE_LOOP);
   	adjustFrameNum = (allFrameNum - (tmp + loopFrameNum));// 计算调节帧
	sm_log(SM_LOG_DEBUG, "ui clean frame:%d, circleLoop:%d, adjustFrameNum:%d\r\n", allFrameNum, circleLoop, adjustFrameNum);
	page = allFrameNum; // 60s
	for (n=0; n<page; n++) {
        if (UI_CLEAN_PRO < get_current_ui_detail_status() || HEATTING_CLEAN != get_system_status()) { // 发生错误 或主动退出
			if (n < (allFrameNum - 10)) { // 临近结束不执行强制退出，准备播放结束动画
				amoled_brightness_set(0, 5, 100);
				amoled_disp_clear();
				sys_task_security_ms_delay(500, TASK_UI); // 主动退出清洁，延时1s退出，防止无过度UI，导致马达震动冲突
				return;
        	}
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);
        /* 处理star和转圈页面 */
		if (n < (PAGE_NUM_OF_STAR_START + adjustFrameNum)) {
			if (n < PAGE_NUM_OF_STAR_START) {
				index = n; // star 01-15
			} else {
				index = INDEX_OF_CIRCLE_LOOP - 1; // 调节帧
			}
		} else {
        	m = n -  (PAGE_NUM_OF_STAR_START + adjustFrameNum);
			if (m < loopFrameNum) { // 周期旋转
				index = INDEX_OF_CIRCLE_LOOP + (m % PAGE_NUM_OF_CIRCLE_LOOP);
			}
		}
		//sm_log(SM_LOG_DEBUG, "ui index:%d\r\n", index); // debug
		imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleaningTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}
		/* 处理倒计时页面 */
		if (0 == (n%12) && (cleanTime < 100)) {
			cntTime = (n * disp_dly);
			cntDown = (cntTime / 1000) % cleanTime;
			if (cntDown > cleanTime) {cntDown = cleanTime;}
			cntDown = cleanTime - cntDown;
			if (cntDown == cleanTime) { // 显示白字+单位s
				amoled_display_number(cntDown, 1, 0);
			} else { // 显示白圆+黑字
				if (cntDown == (cleanTime - 1)) {
					memset((g_tImageMainRam + (90*213*3)), 0x00, (90*46*3)); // 清除残影
				}
				amoled_display_number(cntDown, 0, 0);
			}
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
	sm_log(SM_LOG_DEBUG, "ui clean:%d\r\n", (msTickDev->read((uint8_t*)&tick, 4) - tick));
	clear_disp_buff();
	page = (PAGE_NUM_OF_STAR_START - 3) + PAGE_NUM_OF_CIRCLE_TO_END + PAGE_NUM_OF_STAR_TO_END;
	for (n=0; n<page; n++) {
        msTick = msTickDev->read((uint8_t*)&msTick, 4);
		if (n < (PAGE_NUM_OF_CIRCLE_TO_END + PAGE_NUM_OF_STAR_TO_END)) {
			index = INDEX_OF_CIRCLE_TO_END + n;
		} else {
			index = ((PAGE_NUM_OF_STAR_START - 3) - 1) - (n - (PAGE_NUM_OF_CIRCLE_TO_END + PAGE_NUM_OF_STAR_TO_END));
		}

		imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleaningTablInfo[index];
		if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
			break;
		}

		if (n <= 5) { // 前5帧 倒计时数字0渐灭
			brightness = n * 51;
			if (brightness > 255) {brightness = 255;}
			amoled_display_number(0, 2, (255 - brightness)); // 数字0渐灭
		} else { // 准备开始播放打钩动画(页数24)
			if (n >= PAGE_NUM_OF_CIRCLE_TO_END) {
				m = n - PAGE_NUM_OF_CIRCLE_TO_END;
				if (m < 20) {
					index = m;
				} else if (m < (page - PAGE_NUM_OF_CIRCLE_TO_END - 4)) {
					index = 20 - 1;
				} else {
					index = 20 + (n - (page - 4));
				}
				//sm_log(SM_LOG_DEBUG, "ui index:%d\r\n", index); // debug
				imageInfo = &imageHeaderInfo->imageBltTablInfo.imagePairedTablInfo[index];
				uint16_t ypos = imageInfo->ypos; // 保存页面默认的定位坐标
				imageInfo->ypos = 212; // 调整y坐标为212(该操作修改了表头，注意另外一处调用坐标是否正确)
				if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
					break;
				}
				imageInfo->ypos = ypos; // 恢复保存页面此前的定位坐标
			}
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
    sm_log(SM_LOG_DEBUG, "ui clean:%d\r\n", (msTickDev->read((uint8_t*)&tick, 4) - tick));
	amoled_disp_clear();
	sys_task_security_ms_delay(10, TASK_UI); // 防止无过度UI，导致马达震动冲突
}

/**
* @brief  显示清洁提示
* @param None
* @return keyState - 0:no key pressed; 1:boost key pressed; 2:base key pressed;
* @note
*/
/* 辅助页偏移地址remap */
#define PAGE_BOOST_FRAME	(81 + 0)
#define PAGE_BOOST_BUTTON	(81 + 1)
#define PAGE_BASE_FRAME		(81 + 2)
#define PAGE_BASE_BUTTON	(81 + 3)
#define PAGE_STAR			(81 + 4)
#define PAGE_EXECUTE		(81 + 5)
#define PAGE_CANCEL			(81 + 6)

//uint8_t CleanHintFlag = 0;
static uint8_t CleanHintBoostKeyPressed = 0;
static uint8_t CleanHintBaseKeyPressed = 0;

static void clr_key_state(void)
{
	CleanHintBoostKeyPressed = 0;
	CleanHintBaseKeyPressed = 0;
}

static void set_key_state(void)
{
	CleanHintBoostKeyPressed = 1;
	CleanHintBaseKeyPressed = 1;
}

// 按键状态接口函数，需返回加热接收后，清洁提示过程中的按键单击状态
static uint8_t get_key_state(void) // 仅用于调试
{
// 更新按键状态
	if (CleanHintBaseKeyPressed == 0 && get_base_key_press_time() > 20)
	{
		CleanHintBaseKeyPressed = 1;
	}
	if (CleanHintBoostKeyPressed == 0 && get_boost_key_press_time() > 20)
	{
		CleanHintBoostKeyPressed = 1;
	}
// 重键时，释放后清零
	if (CleanHintBaseKeyPressed && CleanHintBoostKeyPressed)
	{
		if (0 == get_base_key_press_time() && 0 == get_boost_key_press_time())
		{
			clr_key_state();
		}
		return 0;
	}

	if (CleanHintBoostKeyPressed)		{return 1;}
	else if (CleanHintBaseKeyPressed)	{return 2;}
	else								{return 0;}
}

uint8_t amoled_display_clean_hint(void)
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
    uint16_t nStep[5] = {0};
    uint16_t brightness = 0;
    uint16_t brightnessMidNum = 0;
	uint8_t keyState = 0;
	uint8_t keyFadeloop = 0;
	//uint32_t tick = msTickDev->read((uint8_t*)&tick, 4);

	clear_disp_buff();
	set_key_state();
	keyFadeloop = 32; // 32帧 1.28s
	brightnessMidNum = keyFadeloop / 2; // 按键渐变效果 计数的中间值
	nStep[0] = 36;
	nStep[1] = nStep[0] + (keyFadeloop*3);
	nStep[2] = nStep[1] + 14;
	nStep[3] = nStep[2] + 45;
	nStep[4] = nStep[3] + (keyFadeloop*2);
	page = nStep[4]; // 总帧数，总时间约10s
    for (int n = 0; n < page; n++) {
        if (UI_NONE == get_current_ui_detail_status()) {
            break;
        }
        if (UI_HEATTING < get_current_ui_detail_status()) { // || HEATTING_CLEAN == get_system_status())
            break;
        }
        if (sys_get_reset_flag()) { // reset触发，跳过后续页面显示
        	break;
        }

        msTick = msTickDev->read((uint8_t*)&msTick, 4);
		/* 获取按键操作 - 0:no key pressed; 1:boost key pressed; 2:base key pressed; */
		keyState = get_key_state(); // 按键状态接口函数
		// 播完1帧动画，执行按键状态检查
		if (0 != keyState) {
			clr_key_state(); // 此处不能清除标志位，UI主任务要用keyState
			break; // return keyState;
		}

        if (n < nStep[0]) { // 前36帧，刷起始过渡页面
			index = n;
			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[index];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}
        } else if (n < nStep[1]) { // 按键呼吸闪烁
        	index = n - nStep[0];
        	cnt = index % keyFadeloop;
        	if (0) { // 先加载其他辅助页
    			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_STAR];
    			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
    				break;
    			}
    			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_EXECUTE];
    			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
    				break;
    			}
        	}
        	imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_BOOST_BUTTON];
			if (false == get_image_from_flash(imageInfo, g_tImageSecondRam)) {
				break;
			}
			if (cnt <= brightnessMidNum) {
				brightness = (brightnessMidNum - cnt) * ((255 - 78) / brightnessMidNum) + 78;
			} else {
				brightness = (cnt - brightnessMidNum) * ((255 - 78) / brightnessMidNum) + 78;
			}
			//if (brightness > 255) {brightness = 255;}
			amoled_brightness_sw_cfg(imageInfo, g_tImageSecondRam, (uint8_t)brightness);
			if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}
			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_BOOST_FRAME];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
				break;
			}
        } else if (n < nStep[3]) { // 固定显示
        	if (n < nStep[2]) {
        		index = (36 - 1); // 第36页 保持14帧
        	} else {
        		index = 36 + (n - nStep[2]);
        	}
        	//sm_log(SM_LOG_DEBUG, "ui index:%d\r\n", index); // debug
			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[index];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}
        } else { // if (n < nStep[4])按键呼吸闪烁
        	index = n - nStep[3];
        	cnt = index % keyFadeloop;
			if (0) { // 先加载其他辅助页?
				imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_STAR];
				if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
					break;
				}
				imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_CANCEL];
				if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
					break;
				}
			}
			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_BASE_BUTTON];
			if (false == get_image_from_flash(imageInfo, g_tImageSecondRam)) {
				break;
			}
			if (cnt <= brightnessMidNum) {
				brightness = (brightnessMidNum - cnt) * ((255 - 78) / brightnessMidNum) + 78;
			} else {
				brightness = (cnt - brightnessMidNum) * ((255 - 78) / brightnessMidNum) + 78;
			}
			//if (brightness > 255) {brightness = 255;}
			amoled_brightness_sw_cfg(imageInfo, g_tImageSecondRam, (uint8_t)brightness);
			if (false == fill_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
				break;
			}
			imageInfo = &imageHeaderInfo->imageOtherTablInfo.imageCleanHintTablInfo[PAGE_BASE_FRAME];
			if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_OR)) {
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
	amoled_brightness_set(0, 5, 100);
	amoled_disp_clear();
	amoled_brightness_set(1, 0, 0); // 恢复亮度，以免影响后续UI播放
    //sm_log(SM_LOG_DEBUG, "ui hint:%d\r\n", (msTickDev->read((uint8_t*)&tick, 4) - tick));
    return keyState;
}

#endif // 启用干烧清洁功能,不使用干烧清洁功能时，把此宏注释掉
