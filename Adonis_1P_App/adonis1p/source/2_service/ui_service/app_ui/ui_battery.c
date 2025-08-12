/**
  ******************************************************************************
  * @file    ui_battery.c
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
#include "ui_battery.h"
#include "task_heatting_service.h"
#include "err_code.h"
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "driver_amoled.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "ui_base.h"

extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
/**
  * @brief  背光渐变
  * @param  
* @return None
* @note
*/
void amoled_backlight_fade(SysStatus_u tempSysStatus)
{
	uint32_t temp;
	temp = amoled_brigth_get();
	temp *= 255;
	temp /= 100;

	// 调整背光渐灭
	for (uint32_t i=temp; i>0; i--) {
		if (tempSysStatus != get_system_status()) {
			break;
		}
		driver_rm69600_write_command(0x51);
		driver_rm69600_write_data(i);
		sys_task_security_ms_delay(4, TASK_UI);
	}
}

//void amoled_backlight_fade2(SysStatus_u tempSysStatus)
//{
//	// 调整背光渐灭
//	for (uint8_t i =0xff; i > 0; i--) {
//		if (tempSysStatus != get_system_status()) {
//			break;
//		}
//		if (tempSysStatus == IDLE && batteryLevel < pIniVal[WAR_BAT_LOW_SOC]) {
//			if (UI_LEVEL != get_current_ui_detail_status() && UI_CHARGE_LEVEL != get_current_ui_detail_status() && UI_ERR_BAT_LOW != get_current_ui_detail_status() && UI_ERR_CHARGE_TIMEOUT != get_current_ui_detail_status()) {
//				break;
//			}
//		}
//		driver_rm69600_write_command(0x51);
//		driver_rm69600_write_data(i);
//		sys_task_security_ms_delay(4, TASK_UI);
//	}
//}

/**
  * @brief  恢复默认背光
  * @param  
* @return None
* @note
*/
void amoled_backlight_restore(void)
{
	uint32_t temp;
    // 清屏
    amoled_display_clear();

    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
	temp = amoled_brigth_get();
	temp *= 255;
	temp /= 100;
    driver_rm69600_write_data((uint8_t)temp);
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

#if 0
uint8_t get_batter_level(uint16_t batVal, SysStatus_u tempSysStatus)
{
    uint8_t level;
    float chargeUp =0.0;// 0.125;
    float batValTmp;
    float fLevel = 0.0;
    float val = (1.0 * batVal) / 1000;
    //sm_log(SM_LOG_DEBUG, "batVal:%d, %f\r\n", batVal, val);//因测试协议屏蔽
    if (CHARGING == tempSysStatus) {
        batValTmp = val - chargeUp;
    } else {
        batValTmp = val;
    }
    if (batValTmp < 3.585) {
        fLevel = 0;
    } else if (batValTmp > 4.385) {
        fLevel = 100;
    } else {
        fLevel = (-136.86 * (batValTmp * batValTmp)) + 1193.8 * batValTmp - 2506.8; // 电池充放电曲线公式
    }
    level = (uint8_t)fLevel;
//    sm_log(SM_LOG_DEBUG, "batVal:%d, %f, %f\r\n", level, fLevel, batValTmp);
    if (level <=100 && level>=0) {
//        level +=5;
//        level = level / 10;
//        level = level * 10; // 目前电量只设置十档
        return level;
    }
    return 0;
}
#endif

bool procotol_get_cell_capacity (uint16_t batVal,uint8_t* rValue)
{
    uint16_t tempValue = batVal;
    rValue[0] = LBYTE(tempValue);
    rValue[1] = HBYTE(tempValue);
    return true;
}

/**
  * @brief  取出电量百分比
  * @param  None
* @return None
* @note   None
*/
void amoled_load_battery_level_percent(uint8_t batteryLevel)
{
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    // 读取电量百分比图片
    // 根据电量等级读取图片并存放
    if (batteryLevel == 100) { // 满电量 直接调用11序号图片
        imageInedx = 11;
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);

		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
        imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].xpos;
        imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].xpos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
        
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
     } else if (batteryLevel < 10) { // 个位数电量，序号10为百分号，0~9 为数字
        // 读取数字
        imageInedx = batteryLevel;
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
        imageWitdthStart = 26;
        imageWitdthEnd = 26 + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
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
        // 读取 百分号 百分号的宽度与数字宽度不一样 数字17， 百分号21, 需临时保存
        imageInedx = 10;
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}


        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
        imageWitdthStart = 46;
        imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
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
    } else { // 两位数电量
        // 读取十位数字
        imageInedx = batteryLevel / 10;
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);

		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
        imageWitdthStart = 16;
        imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
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
        // 读取个位
        imageInedx = batteryLevel % 10;
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
        imageWitdthStart = 36;
        imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
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
        // 读取百分号
        imageInedx = 10;
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh * imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].ypos + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].height;
        imageWitdthStart = 56;
        imageWitdthEnd = imageWitdthStart + imageHeaderInfo->imageBatTablInfo.imageNumberPerTablInfo[imageInedx].witdh;
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
}

/**
  * @brief  取出充电超时图标
  * @param  None
* @return None
* @note   None
*/
void amoled_display_charge_overtime_icon(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t procTime = 0;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);

    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].witdh * imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].ypos;
    imageHeightEnd = imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].ypos + imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].height;
    imageWitdthStart = imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].xpos;
    imageWitdthEnd = imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].xpos + imageHeaderInfo->imageErrTablInfo.imageTimeoutTablInfo[8].witdh;
    
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
    ptAmoledInfo.area.x1 = 0;
    ptAmoledInfo.area.y1 = 0;
    ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
    ptAmoledInfo.area.y2 = UI_VER_RES - 1;
    ptAmoledInfo.data = g_tImageMainRam;
    ptAmoledInfo.len = IMAGE_SEZE;
    amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
    procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
    if (procTime < 95) { // 时间根据后续效果调整
        sys_task_security_ms_delay(95 - procTime, TASK_UI);
    } else { // 大于等于100时也需要释放MCU 1ms
        sys_task_security_ms_delay(1, TASK_UI);
    }
    #endif
}

/**
  * @brief  取出充电图标
  * @param  None
* @return None
* @note   None
*/
void amoled_load_battery_charge_icon(void)
{
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;
    // 存放闪电图标
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].ypos;
    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageLightningTablInfo[0].witdh;
    
    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
        return;
    }

    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
        }
    }
}

/**
  * @brief  取出烟支数量
  * @param  None
* @return None
* @note   None
*/
void amoled_load_battery_sessions(uint8_t cnt)
{
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;
    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;
    if (cnt == 0) {
        return;
    }
    // 烟支数量
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    #if 0
//    switch (cnt) {
//        case 1:
//            ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].addr;
//            break;
//        case 2:
//            ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[1].addr;
//            break;
//        case 3:
//            ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[2].addr;
//            break;
//        case 4:
//            ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[3].addr;
//            break;
//        case 5:
//            ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[4].addr;
//            break;
//        default:
//            ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].addr;
//            break;
//    }
////    #endif
////    if (cnt == 5) {
////        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].addr;
////    }
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);

	if (--cnt > 4)	cnt = 0;
	if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[cnt]),g_tImageSecondRam))
	{
		return;
	}
	
    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].ypos;
    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageHintTablInfo[0].witdh;
    
    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
        return;
    }

    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
        }
    }
}

/**
  * @brief  播放充电提示
  * @param  None
* @return None
* @note   None
*/
void amoled_load_battery_charge_tip(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    ImageHeaderInfo_t* imageHeaderInfo;
    AmoledInfo_t ptAmoledInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;
    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;
    uint32_t msTick;
    uint32_t procTime = 0;

    // 烟支数量
    for (int n = 0; n < 7; n++) {
        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].witdh * imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n]),g_tImageSecondRam))
		{
			return;
		}

        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].ypos + imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].height;
        imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].xpos;
        imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].xpos + imageHeaderInfo->imageBatTablInfo.imagePlugTablInfo[n].witdh;

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
        if (procTime < 95) { // 时间根据后续效果调整
            sys_task_security_ms_delay(95 - procTime, TASK_UI);
        } else { // 大于等于100时也需要释放MCU 1ms
            sys_task_security_ms_delay(1, TASK_UI);
        }
    }
}

/**
  * @brief  显示绿色外框
  * @param  None
* @return None
* @note   None
*/
void amoled_display_battery_green_level(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t procTime = 0;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);

    // 取出外框并显示
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].ypos;
    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[1].witdh;
    
    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
        return;
    }

    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
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
    if (procTime < 95) { // 时间根据后续效果调整
        sys_task_security_ms_delay(95 - procTime, TASK_UI);
    } else { // 大于等于100时也需要释放MCU 1ms
        sys_task_security_ms_delay(1, TASK_UI);
    }

}

/**
  * @brief  显示黄色外框
  * @param  None
* @return None
* @note   None
*/
void amoled_display_battery_yellow_level(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t procTime = 0;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);

    // 取出外框并显示
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].ypos;
    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[1].witdh;
    
    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
        return;
    }

    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
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
    if (procTime < 95) { // 时间根据后续效果调整
        sys_task_security_ms_delay(95 - procTime, TASK_UI);
    } else { // 大于等于100时也需要释放MCU 1ms
        sys_task_security_ms_delay(1, TASK_UI);
    }

}

/**
  * @brief  显示琥珀色外框
  * @param  None
* @return None
* @note   None
*/
void amoled_display_battery_amber_level(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t procTime = 0;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);

    // 取出外框并显示
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].ypos;
    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[1].witdh;
    
    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
        return;
    }

    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
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
    if (procTime < 95) { // 时间根据后续效果调整
        sys_task_security_ms_delay(95 - procTime, TASK_UI);
    } else { // 大于等于100时也需要释放MCU 1ms
        sys_task_security_ms_delay(1, TASK_UI);
    }

}

/**
  * @brief  显示红色外框
  * @param  None
* @return None
* @note   None
*/
void amoled_display_battery_red_level(void)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t procTime = 0;
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);

    // 取出外框并显示
    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].addr;
//    ptQspiflash.data = g_tImageSecondRam;
//    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].height * UI_COLOR_DEPTH;
//    
//    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//        return;
//    }
//
//    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
	if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1]),g_tImageSecondRam))
	{
		return;
	}

    k =0;
    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].ypos;
    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].height;
    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].xpos;
    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].witdh;
    
    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
        return;
    }

    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
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
    if (procTime < 95) { // 时间根据后续效果调整
        sys_task_security_ms_delay(95 - procTime, TASK_UI);
    } else { // 大于等于100时也需要释放MCU 1ms
        sys_task_security_ms_delay(1, TASK_UI);
    }
}

extern void driver_rm69600_write_data(uint8_t data);
extern void driver_rm69600_write_command(uint8_t data);

/**
  * @brief  显示绿色bar动图
  * @param  typeStatus: 0:显示电量，1：充电，2：充电超时错误
* @return None
* @note   None
*/
void amoled_display_battery_green_bar(uint8_t batteryLevel, uint8_t typeStatus)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;
    SysStatus_u tempSysStatus = get_system_status();

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t msTick1;
    uint32_t msTick2;
    uint32_t msTick3;
    uint32_t msTick4;
    uint32_t procTime = 0;
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    int maxHeigh = (batteryLevel * 97 / 100);
    int n = 0;
    while ( n <= maxHeigh) {
//        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
			if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0]),g_tImageSecondRam))
			{
				return;
			}

//        msTick1 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].height;
        imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].xpos;
        imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].witdh;

        if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
            return;
        }
        // 满图高度为97，对应100%电量， 电量*0.97 = 高度
        int blackHeigh = imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].height - n;
        int blackPix = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].witdh;
        int blackLen = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatGreenTablInfo[0].witdh * 3;
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, blackLen);
//        msTick2 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1;
        for(int i = imageHeightStart;i < imageHeightEnd;i++) {
            for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
            }
        }
        
        if (typeStatus == 2 && n == maxHeigh) { // 显示充电超时
            amoled_display_charge_overtime_icon();
        }
//        msTick3 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2;
        ptAmoledInfo.area.x1 = 0;
        ptAmoledInfo.area.y1 = 0;
        ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
//        msTick4 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2 - msTick3;
        // procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
//        sm_log(SM_LOG_DEBUG, "n:%d, %d, %d, %d\r\n", msTick1, msTick2, msTick3, msTick4);
        if (n == maxHeigh) {
            break;
        }
        if (n + 10 > maxHeigh) {
            n = maxHeigh;
        } else  {
            n += 10;
        }
//        if (procTime < 40) {
//            vTaskDelay(40 - procTime);
//        } else { // 大于等于100时也需要释放MCU 1ms
//            vTaskDelay(1);
//        }
        sys_task_security_ms_delay(1, TASK_UI);
    }
    while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 3000) {
        if (get_pctool_idle_cmd_status()) { // 收到上位机休眠指令 退出显示
            break;
        }
        if (get_update_ui_timer_cnt() > 0) {
            break;
        }

        sys_task_security_ms_delay(10, TASK_UI);
        if (tempSysStatus != get_system_status()) {
            break;
        }
        if (UI_LEVEL != get_current_ui_detail_status() && UI_CHARGE_LEVEL != get_current_ui_detail_status() && UI_ERR_CHARGE_TIMEOUT != get_current_ui_detail_status()) { // BUG 1911909
            amoled_display_clear();
            return;
        }
        if (typeStatus == 0) { //显示电量时，如果有更高优先级则中断当前状态
            if (UI_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 1) { // 显示充电时
            if (UI_CHARGE_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 2) { // 显示充电超时
            if (UI_ERR_CHARGE_TIMEOUT < get_current_ui_detail_status()) {
                break;
            }
        }
    }
#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    // 调整背光渐灭
    for (uint8_t i =0xff; i > 0; i--) {
        if (tempSysStatus != get_system_status()) {
            break;
        }
        driver_rm69600_write_command(0x51);
        driver_rm69600_write_data(i);
        sys_task_security_ms_delay(4, TASK_UI);
    }
    // 清屏
    
    amoled_display_clear();

    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);
#else
    // 调整背光渐灭
	amoled_backlight_fade(tempSysStatus);
    // 清屏
	amoled_backlight_restore();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

/**
  * @brief  显示黄色bar动图
  * @param  typeStatus: 0:显示电量，1：充电，2：充电超时错误
* @return None
* @note   None
*/
void amoled_display_battery_yellow_bar(uint8_t batteryLevel, uint8_t typeStatus)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;
    SysStatus_u tempSysStatus = get_system_status();

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t msTick1;
    uint32_t msTick2;
    uint32_t msTick3;
    uint32_t msTick4;
    uint32_t procTime = 0;
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    int maxHeigh = (batteryLevel * 97 / 100);
    int n = 0;
    while ( n <= maxHeigh) {
//        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0]),g_tImageSecondRam))
		{
			return;
		}

//        msTick1 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].height;
        imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].xpos;
        imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].witdh;

        if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
            return;
        }

        // 满图高度为97，对应100%电量， 电量*0.97 = 高度
        int blackHeigh = imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].height - n;
        int blackPix = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].witdh;
        int blackLen = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatYellowTablInfo[0].witdh * 3;
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, blackLen);
//        msTick2 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1;
        for(int i = imageHeightStart;i < imageHeightEnd;i++) {
            for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
            }
        }

        if (typeStatus == 2 && n == maxHeigh) { // 显示充电超时
            amoled_display_charge_overtime_icon();
        }

//        msTick3 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2;
        ptAmoledInfo.area.x1 = 0;
        ptAmoledInfo.area.y1 = 0;
        ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
//        msTick4 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2 - msTick3;
        // procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
//        sm_log(SM_LOG_DEBUG, "n:%d, %d, %d, %d\r\n", msTick1, msTick2, msTick3, msTick4);
        if (n == maxHeigh) {
            break;
        }
        if (n + 5 > maxHeigh) {
            n = maxHeigh;
        } else  {
            n += 5;
        }
//        if (procTime < 40) {
//            vTaskDelay(40 - procTime);
//        } else { // 大于等于100时也需要释放MCU 1ms
//            vTaskDelay(1);
//        }
        sys_task_security_ms_delay(1, TASK_UI);
    }
    while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 3000) {
        if (get_pctool_idle_cmd_status()) { // 收到上位机休眠指令 退出显示
            break;
        }
        if (get_update_ui_timer_cnt() > 0) {
            break;
        }
        sys_task_security_ms_delay(10, TASK_UI);
        if (tempSysStatus != get_system_status()) {
            break;
        }
        if (UI_LEVEL != get_current_ui_detail_status() && UI_CHARGE_LEVEL != get_current_ui_detail_status() && UI_ERR_CHARGE_TIMEOUT != get_current_ui_detail_status()) { // BUG 1911909
            amoled_display_clear();
            return;
        }
        if (typeStatus == 0) { //显示电量时，如果有更高优先级则中断当前状态
            if (UI_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 1) { // 显示充电时
            if (UI_CHARGE_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 2) { // 显示充电超时
            if (UI_ERR_CHARGE_TIMEOUT < get_current_ui_detail_status()) {
                break;
            }
        }
    }
#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    // 调整背光渐灭
    for (uint8_t i =0xff; i > 0; i--) {
        if (tempSysStatus != get_system_status()) {
            break;
        }
        driver_rm69600_write_command(0x51);
        driver_rm69600_write_data(i);
        sys_task_security_ms_delay(4, TASK_UI);
    }
    // 清屏
    
    amoled_display_clear();

    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);
#else
	// 调整背光渐灭
	amoled_backlight_fade(tempSysStatus);
	// 清屏
	amoled_backlight_restore();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

/**
  * @brief  显示琥珀色bar动图
  * @param  typeStatus: 0:显示电量，1：充电，2：充电超时错误
* @return None
* @note   None
*/
void amoled_display_battery_amber_bar(uint8_t batteryLevel, uint8_t typeStatus)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;
    SysStatus_u tempSysStatus = get_system_status();

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t msTick1;
    uint32_t msTick2;
    uint32_t msTick3;
    uint32_t msTick4;
    uint32_t procTime = 0;
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    int maxHeigh = (batteryLevel * 97 / 100);
    int n = 0;
    while ( n <= maxHeigh) {
//        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0]),g_tImageSecondRam))
		{
			return;
		}

//        msTick1 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].height;
        imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].xpos;
        imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].witdh;

        if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
            return;
        }

        // 满图高度为97，对应100%电量， 电量*0.97 = 高度
        int blackHeigh = imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].height - n;
        int blackPix = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].witdh;
        int blackLen = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatAmberTablInfo[0].witdh * 3;
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, blackLen);
//        msTick2 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1;
        for(int i = imageHeightStart;i < imageHeightEnd;i++) {
            for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
            }
        }

        if (typeStatus == 2 && n == maxHeigh) { // 显示充电超时
            amoled_display_charge_overtime_icon();
        }

//        msTick3 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2;
        ptAmoledInfo.area.x1 = 0;
        ptAmoledInfo.area.y1 = 0;
        ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
//        msTick4 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2 - msTick3;
        // procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
//        sm_log(SM_LOG_DEBUG, "n:%d, %d, %d, %d\r\n", msTick1, msTick2, msTick3, msTick4);
        if (n == maxHeigh) {
            break;
        }
        if (n + 2 > maxHeigh) {
            n = maxHeigh;
        } else  {
            n += 2;
        }
//        if (procTime < 40) {
//            vTaskDelay(40 - procTime);
//        } else { // 大于等于100时也需要释放MCU 1ms
//            vTaskDelay(1);
//        }
            sys_task_security_ms_delay(1, TASK_UI);
    }
    while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 3000) {
        if (get_pctool_idle_cmd_status()) { // 收到上位机休眠指令 退出显示
            break;
        }
        if (get_update_ui_timer_cnt() > 0) {
            break;
        }
        sys_task_security_ms_delay(10, TASK_UI);
        if (tempSysStatus != get_system_status()) {
            break;
        }
        if (UI_LEVEL != get_current_ui_detail_status() && UI_CHARGE_LEVEL != get_current_ui_detail_status() && UI_ERR_CHARGE_TIMEOUT != get_current_ui_detail_status()) { // BUG 1911909
            amoled_display_clear();
            return;
        }
        if (typeStatus == 0) { //显示电量时，如果有更高优先级则中断当前状态
            if (UI_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 1) { // 显示充电时
            if (UI_CHARGE_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 2) { // 显示充电超时
            if (UI_ERR_CHARGE_TIMEOUT < get_current_ui_detail_status()) {
                break;
            }
        }
    }
#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    // 调整背光渐灭
    for (uint8_t i =0xff; i > 0; i--) {
        if (tempSysStatus != get_system_status()) {
            break;
        }
        driver_rm69600_write_command(0x51);
        driver_rm69600_write_data(i);
        sys_task_security_ms_delay(4, TASK_UI);
    }
    // 清屏
    
    amoled_display_clear();

    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);
#else
	// 调整背光渐灭
	amoled_backlight_fade(tempSysStatus);
//    // 清屏
	amoled_backlight_restore();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

/**
  * @brief  显示红色bar动图
  * @param  typeStatus: 0:显示电量，1：充电，2：充电超时错误
* @return None
* @note   None
*/
void amoled_display_battery_red_bar(uint8_t batteryLevel, uint8_t typeStatus)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    AmoledInfo_t ptAmoledInfo;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    Qspiflash_t ptQspiflash;
    SysStatus_u tempSysStatus = get_system_status();
    uint16_t *pIniVal = get_ini_val_info_handle();

    uint32_t k =0;
    uint16_t imageHeightStart;
    uint16_t imageHeightEnd;
    uint16_t imageWitdthStart;
    uint16_t imageWitdthEnd;
    uint8_t imageInedx = 0;

    uint32_t msTick;
    uint32_t msTick1;
    uint32_t msTick2;
    uint32_t msTick3;
    uint32_t msTick4;
    uint32_t procTime = 0;
    int blackHeigh;
    int blackPix;
    int blackLen;
    msTick = msTickDev->read((uint8_t*)&msTick, 4);
    uint8_t temHeight = batteryLevel;
    if (batteryLevel < pIniVal[WAR_BAT_LOW_SOC]) {
        temHeight = pIniVal[WAR_BAT_LOW_SOC];
    }
    int maxHeigh = (temHeight * 97 / 100);
    int n = 0;
    while ( n <= maxHeigh) {
//        msTick = msTickDev->read( (uint8_t*)&msTick, 4);
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//        ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].addr;
//        ptQspiflash.data = g_tImageSecondRam;
//        ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].height * UI_COLOR_DEPTH;
//
//        if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//            return;
//        }
//
//        qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
		if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0]),g_tImageSecondRam))
		{
			return;
		}

//        msTick1 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
        k =0;
        imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].ypos;
        imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].height;
        imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].xpos;
        imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh;

        if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
            return;
        }

        // 满图高度为97，对应100%电量， 电量*0.97 = 高度
        blackHeigh = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].height - n;
        blackPix = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh;
        blackLen = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh * 3;
        memset((uint8_t*)&g_tImageSecondRam[0], 0x00, blackLen);
//        msTick2 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1;
        for(int i = imageHeightStart;i < imageHeightEnd;i++) {
            for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
                g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
            }
        }

        if (typeStatus == 2 && n == maxHeigh) { // 显示充电超时
            amoled_display_charge_overtime_icon();
        }
        
//        msTick3 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2;
        ptAmoledInfo.area.x1 = 0;
        ptAmoledInfo.area.y1 = 0;
        ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
//        msTick4 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick - msTick1 - msTick2 - msTick3;
        // procTime =  msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
//        sm_log(SM_LOG_DEBUG, "n:%d, %d, %d, %d\r\n", msTick1, msTick2, msTick3, msTick4);
        if (n == maxHeigh) {
            break;
        }
        if (n + 1 > maxHeigh) {
            n = maxHeigh;
        } else  {
            n += 1;
        }
//        if (procTime < 40) {
//            vTaskDelay(40 - procTime);
//        } else { // 大于等于100时也需要释放MCU 1ms
//            vTaskDelay(1);
//        }

           sys_task_security_ms_delay(1, TASK_UI);
    }
    // 显示充电提示
    if (typeStatus == 0 && tempSysStatus == IDLE && batteryLevel < pIniVal[WAR_BAT_LOW_SOC]) {
        amoled_load_battery_charge_tip();
   //     msTick = msTickDev->read((uint8_t*)&msTick, 4);
    }

    uint32_t flashMsTick = msTickDev->read( (uint8_t*)&flashMsTick, 4);
    uint8_t onFlag = 1;
    uint8_t flashCnt = 0; // 亮次数
    uint32_t firstTime = 300; // 第一次灭快些
     while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 3000) {
        if (get_pctool_idle_cmd_status()) { // 收到上位机休眠指令 退出显示
            break;
        }
        if (get_update_ui_timer_cnt() > 0) {
            break;
        }
         sys_task_security_ms_delay(10, TASK_UI);
        if (tempSysStatus != get_system_status()) {
            break;
        }
        if (tempSysStatus == IDLE && batteryLevel < pIniVal[WAR_BAT_LOW_SOC]) { // 添加红色闪烁
            if (msTickDev->read( (uint8_t*)&flashMsTick, 4) - flashMsTick > 400- firstTime) {
                firstTime = 0;
                flashMsTick = msTickDev->read( (uint8_t*)&flashMsTick, 4);
                if (flashCnt >= 2) {
                    continue;
                }
                // 读取外框
                memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//                ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].addr;
//                ptQspiflash.data = &g_tImageSecondRam[0];
//                ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].height * UI_COLOR_DEPTH;
//                
//                if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                    return;
//                }
//                
//                qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
			   if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1]),g_tImageSecondRam))
			   {
				   return;
			   }

               uint16_t imageHeightStart1 = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].ypos;
               uint16_t imageHeightEnd1 = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].height;
               uint16_t imageWitdthStart1 = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].xpos;
               uint16_t imageWitdthEnd1 = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[1].witdh;
                
                if (false ==  ui_pos_securitu_verify(imageHeightStart1, imageHeightEnd1, imageWitdthStart1, imageWitdthEnd1)) {
                    return;
                }
                
                k =0;
                for(int i = imageHeightStart1;i < imageHeightEnd1;i++) {
                    for (int j =imageWitdthStart1; j < imageWitdthEnd1; j++) {
                        g_tImageMainRam[(j + i * UI_HOR_RES) * 3] = g_tImageSecondRam[k++];
                        g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] = g_tImageSecondRam[k++];
                        g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] = g_tImageSecondRam[k++];
                    }
                }

                if (onFlag == 1) { // 灭
                    onFlag = 0;
                } else { // 亮
                    onFlag = 1;
                    flashCnt++;
                    // 读取bar
                    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, IMAGE_SEZE);
//                    ptQspiflash.addr = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].addr;
//                    ptQspiflash.data = g_tImageSecondRam;
//                    ptQspiflash.len = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].height * UI_COLOR_DEPTH;
//            
//                    if (false == ui_data_security_verify(ptQspiflash.len, ptQspiflash.addr)) {
//                        return;
//                    }
//            
//                    qspiflashDev->read( (uint8_t*)&ptQspiflash, 0xff);
					if (!get_image_from_flash(&(imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0]),g_tImageSecondRam))
					{
						return;
					}

            //        msTick1 = msTickDev->read( (uint8_t*)&msTick, 4) - msTick;
                    k =0;
                    imageHeightStart = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].ypos;
                    imageHeightEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].ypos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].height;
                    imageWitdthStart = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].xpos;
                    imageWitdthEnd = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].xpos + imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh;
            
                    if (false ==  ui_pos_securitu_verify(imageHeightStart, imageHeightEnd, imageWitdthStart, imageWitdthEnd)) {
                        return;
                    }
            
                    // 满图高度为97，对应100%电量， 电量*0.97 = 高度
                    blackHeigh = imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].height - n;
                    blackPix = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh;
                    blackLen = blackHeigh * imageHeaderInfo->imageBatTablInfo.imageBarBatRedTablInfo[0].witdh * 3;
                    memset((uint8_t*)&g_tImageSecondRam[0], 0x00, blackLen);
                    for(int i = imageHeightStart;i < imageHeightEnd;i++) {
                        for (int j =imageWitdthStart; j < imageWitdthEnd; j++) {
                            g_tImageMainRam[(j + i * UI_HOR_RES) * 3] |= g_tImageSecondRam[k++];
                            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 1] |= g_tImageSecondRam[k++];
                            g_tImageMainRam[(j + i * UI_HOR_RES) * 3 + 2] |= g_tImageSecondRam[k++];
                        }
                    }
                }
                ptAmoledInfo.area.x1 = 0;
                ptAmoledInfo.area.y1 = 0;
                ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
                ptAmoledInfo.area.y2 = UI_VER_RES - 1;
                ptAmoledInfo.data = g_tImageMainRam;
                ptAmoledInfo.len = IMAGE_SEZE;
                amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
            }
        } 
        if (UI_LEVEL != get_current_ui_detail_status() && UI_CHARGE_LEVEL != get_current_ui_detail_status() && UI_ERR_BAT_LOW != get_current_ui_detail_status() && UI_ERR_CHARGE_TIMEOUT != get_current_ui_detail_status()) {
            amoled_display_clear();
            return;
        }
        if (typeStatus == 0 && batteryLevel < pIniVal[WAR_BAT_LOW_SOC]) { //显示低电量时，如果有更高优先级则中断当前状态
            if (UI_ERR_BAT_LOW < get_current_ui_detail_status()) {
                break;
            }
        }  else if (typeStatus == 0) { //显示电量时，如果有更高优先级则中断当前状态
            if (UI_CHARGE_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        }else if (typeStatus == 1) { // 显示充电时
            if (UI_CHARGE_LEVEL < get_current_ui_detail_status()) {
                break;
            }
        } else if (typeStatus == 2) { // 显示充电超时
            if (UI_ERR_CHARGE_TIMEOUT < get_current_ui_detail_status()) {
                break;
            }
        }
    }
     
    // 调整背光渐灭
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint32_t temp = amoled_brigth_get();
	temp *= 255;
	temp /= 100;
    for (uint8_t i = temp; i > 0; i--) {
#else
    for (uint8_t i =0xff; i > 0; i--) {
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
        if (tempSysStatus != get_system_status()) {
            break;
        }
        if (tempSysStatus == IDLE && batteryLevel < pIniVal[WAR_BAT_LOW_SOC]) {
            if (UI_LEVEL != get_current_ui_detail_status() && UI_CHARGE_LEVEL != get_current_ui_detail_status() && UI_ERR_BAT_LOW != get_current_ui_detail_status() && UI_ERR_CHARGE_TIMEOUT != get_current_ui_detail_status()) {
                break;
            }
        }
        driver_rm69600_write_command(0x51);
        driver_rm69600_write_data(i);
        sys_task_security_ms_delay(4, TASK_UI);
    }
#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    // 清屏
    
    amoled_display_clear();

    // 恢复背光亮度
    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);
#else
	// 清屏
	// 恢复背光亮度
	amoled_backlight_restore();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

/************* global: V10 user flow **************/
#define SOC_LEVEL_4		(60) // 60 ~ 100% : green
#define SOC_LEVEL_3		(28) // 28 ~ 59% : yellow
#define SOC_LEVEL_2		(10) // 10 ~ 27% : amber
#define SOC_LEVEL_1		(5)  // 5 ~ 9% : red
#define SOC_LEVEL_0		(0)  // 0 ~ 4% : red

#define SOC_SESSIONS_N	(SOC_LEVEL_3)
#define SOC_SESSIONS_5	(23) // 23~27%
#define SOC_SESSIONS_4	(19) // 19~22%
#define SOC_SESSIONS_3	(15) // 15~18%
#define SOC_SESSIONS_2	(10) // 10~14%
#define SOC_SESSIONS_1	(5)  // 05~09%

uint8_t get_sessions_left(uint8_t soc)
{
	uint8_t val = 0;

	if (soc >= SOC_SESSIONS_5) {
		val = 5;
	} else if (soc >= SOC_SESSIONS_4) {
		val = 4;
	} else if (soc >= SOC_SESSIONS_3) {
		val = 3;
	} else if (soc >= SOC_SESSIONS_2) {
		val = 2;
	} else if (soc >= SOC_SESSIONS_1) {
		val = 1;
	} else {
		val = 0;
	}

	return val;
}

/**
  * @brief  显示充电电量
  * @param  None
* @return None
* @note   4秒电量显示
*/
void amoled_display_battery_charge_level(void)
{
    SysStatus_u tempSysStatus = get_system_status();

    // 获取电池电量
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
    
    uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;
    // 读取电量百分比图片
    amoled_load_battery_level_percent(batteryLevel);
	amoled_load_battery_charge_icon();

    // 根据电量显示不同颜色外框和bar
    if (batteryLevel >= SOC_LEVEL_4) { // 显示绿色
        // 绿色外框
        amoled_display_battery_green_level();
        // 绿色bar播放
        amoled_display_battery_green_bar(batteryLevel, 1);
    } else if (batteryLevel >= SOC_LEVEL_3) { // 显示黄色
        // 黄色外框
        amoled_display_battery_yellow_level();
        // 黄色ar播放
        amoled_display_battery_yellow_bar(batteryLevel, 1);
    } else if (batteryLevel >= SOC_LEVEL_2) { // 显示琥珀色,充电情况不需要显示烟支数
        // 琥珀色外框
        amoled_display_battery_amber_level();
        // 琥珀色bar播放
        amoled_display_battery_amber_bar(batteryLevel, 1);
    } else { // 显示红色,充电情况不需要显示烟支数
        // 红色外框
        amoled_display_battery_red_level();
        // 红色bar播放
        amoled_display_battery_red_bar(batteryLevel, 1);
    }
}

/**
  * @brief  显示电量
  * @param  typeStatus: 0:显示电量，1：充电，2：充电超时错误
* @return None
* @note   4秒电量显示
*/
void amoled_display_battery_level(uint8_t typeStatus)
{
    SysStatus_u tempSysStatus = get_system_status();

    // 获取电池电量
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
    
    uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;
    // 读取电量百分比图片
    amoled_load_battery_level_percent(batteryLevel);

    // 根据电量显示不同颜色外框和bar
    if (batteryLevel >= SOC_LEVEL_4) { // 显示绿色
        // 绿色外框
        amoled_display_battery_green_level();
        // 绿色bar播放
        amoled_display_battery_green_bar(batteryLevel, typeStatus);
    } else if (batteryLevel >= SOC_LEVEL_3) { // 显示黄色
        // 黄色外框
        amoled_display_battery_yellow_level();
        // 黄色ar播放
        amoled_display_battery_yellow_bar(batteryLevel, typeStatus);
    } else if (batteryLevel >= SOC_LEVEL_2) { // 显示琥珀色
        // 琥珀色外框
        amoled_display_battery_amber_level();
        // 显示烟支数量
        if (typeStatus == 0) {
            amoled_load_battery_sessions(get_sessions_left(batteryLevel));
        }
        // 琥珀色bar播放
        amoled_display_battery_amber_bar(batteryLevel, typeStatus);
    } else if (batteryLevel >= SOC_LEVEL_1) { // 显示红色
        // 琥珀色外框
        amoled_display_battery_red_level();
            // 显示烟支数量
        if (typeStatus == 0) {
            amoled_load_battery_sessions(get_sessions_left(batteryLevel));
        }
        // 红色bar播放
        amoled_display_battery_red_bar(batteryLevel, typeStatus);
    } else { // 显示红色,不足1支烟
        // 红色外框
        amoled_display_battery_red_level();
        // 红色bar播放
        amoled_display_battery_red_bar(batteryLevel, typeStatus);
    }
}

#define SESSION_CNT_5 99
#define SESSION_CNT_4 80
#define SESSION_CNT_3 60
#define SESSION_CNT_2 40
#define SESSION_CNT_1 20
#define SESSION_CNT_0 0
/**
  * @brief  eol显示电量
  * @param  None
* @return None
* @note   4秒电量显示
*/
void amoled_display_battery_eol_level(void)
{
    SysStatus_u tempSysStatus = get_system_status();

    // 获取电池电量
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
    uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;
    uint8_t sessionCnt = 0;
    if (batteryLevel > SESSION_CNT_5) {
        sessionCnt = 5;
    } else if (batteryLevel > SESSION_CNT_4) {
        sessionCnt = 4;
    } else if (batteryLevel > SESSION_CNT_3) {
        sessionCnt = 3;
    } else if (batteryLevel > SESSION_CNT_2) {
        sessionCnt = 2;
    } else if (batteryLevel > SESSION_CNT_1) {
        sessionCnt = 1;
    } else {
        sessionCnt = 0;
    }
//    sessionCnt = 0; // 暂不显示session数
//    sm_log(SM_LOG_ERR, "amoled_display_battery_eol_level:%d\r\n", sessionCnt);
    // 读取电量百分比图片
    amoled_load_battery_level_percent(batteryLevel);

    // 根据电量显示不同颜色外框和bar
    if (batteryLevel >= 60) { // 显示绿色
        // 绿色外框
        amoled_display_battery_green_level();
        amoled_load_battery_sessions(sessionCnt);
        // 绿色bar播放
        amoled_display_battery_green_bar(batteryLevel, 0);
    
    } else if (batteryLevel >= 30) { // 显示黄色
        // 黄色外框
        amoled_display_battery_yellow_level();
        amoled_load_battery_sessions(sessionCnt);
        // 黄色ar播放
        amoled_display_battery_yellow_bar(batteryLevel, 0);

    } else if (batteryLevel >= 10) { // 显示琥珀色,烟支数 >25:5, >20:4, >15:3, >10:2, >5:1， 充电情况不需要显示烟支数
        // 琥珀色外框
        amoled_display_battery_amber_level();
        // 显示烟支数量
        amoled_load_battery_sessions(sessionCnt);
        // 琥珀色bar播放
        amoled_display_battery_amber_bar(batteryLevel, 0);
    } else if (batteryLevel >= 5) { // 显示琥珀色,烟支数 >25:5, >20:4, >15:3, >10:2, >5:1， 充电情况不需要显示烟支数
        // 琥珀色外框
        amoled_display_battery_red_level();
        // 显示烟支数量
        amoled_load_battery_sessions(sessionCnt);
        // 红色bar播放
        amoled_display_battery_red_bar(batteryLevel, 0);
    } else { // 显示红色0~4 不足1支烟
        // 红色外框
        amoled_display_battery_red_level();
        // 红色bar播放
        amoled_display_battery_red_bar(batteryLevel, 0);
    }
}


/**
  * @brief  显示低电量
  * @param  None
* @return None
* @note   4秒低电量显示
*/
void amoled_display_battery_low_level(void)
{
    SysStatus_u tempSysStatus = get_system_status();

    // 获取电池电量
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
   // uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.soc;
//   uint8_t batteryLevel = 0; // 加热过程中电压突然降到3V，电量计电量没瞬间降
   
   uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;


    uint16_t *pIniVal = get_ini_val_info_handle();

    if (batteryLevel >= pIniVal [WAR_BAT_LOW_SOC] && pIniVal [WAR_BAT_LOW_SOC] > 0) {
        batteryLevel = pIniVal [WAR_BAT_LOW_SOC] - 1;
    }
    // 读取电量百分比图片
    amoled_load_battery_level_percent(batteryLevel);
    amoled_display_battery_red_level();
    amoled_display_battery_red_bar(batteryLevel, 0);
}

/**
  * @brief  显示eol
  * @param  None
* @return None
* @note   
*/
void amoled_display_battery_eol(SysStatus_u sysStatus)
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
	imageInfo = &imageHeaderInfo->imageErrTablInfo.imageBatEolTablInfo[2]; // 加载Qr：index=1 黑底白码; index=2 白底黑码
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#else
	ImageInfo_t imageQrInfo;
	draw_qr_code(&imageQrInfo, g_tImageSecondRam, QR_EOL);
	if (false == fill_image_to_main_buff(&imageQrInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}
#endif
	imageInfo = &imageHeaderInfo->imageErrTablInfo.imageBatEolTablInfo[0]; // 添加电池标志
	if (false == fill_flash_image_to_main_buff(imageInfo, g_tImageSecondRam, g_tImageMainRam, MODE_NONE)) {
		amoled_disp_clear();
		return;
	}

	amoled_disp_update((uint8_t*)&g_tImageMainRam);
	/* 发送满屏小于25ms这是示波器实测，理论计算24.364ms */
	sys_task_security_ms_delay(25, TASK_UI); // 等待发送完成

    do {
    	sys_task_security_ms_delay(5, TASK_UI);

		if (CHARGING != get_system_status() && IDLE != get_system_status()) {
			break;
		}
		if (UI_EOL < get_current_ui_detail_status()) {
			break;
		}

		procTime = msTickDev->read((uint8_t*)&msTick, 4) - msTick;
		if (procTime > (10000)) { // 显示10s退出
			break;
		}
	} while (1);

    //amoled_brightness_set(0, 5, 100); // 5step * 100ms
	amoled_disp_clear();
	//amoled_brightness_set(1, 1, 0); // 恢复亮度
}

