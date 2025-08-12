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

#include "platform_io.h"
#include "data_base_info.h"
#include "sm_log.h"
#include <stdlib.h>
#include "system_status.h"
#include "ui_heat.h"
#include "task_ui_service.h"


extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];
extern int driver_amoled_reinit(void);

/**
  * @brief  上位机测试显示屏
  * @param  0x00 SCREEN OFF, 0x01 SCREEN RED, 0x02 SCREEN GREEN, 0x03 SCREEN BLUE, 0x04 SCREEN WHITE
* @return None
* @note
*/
bool procotol_lcd_control(uint8_t ctr)
{
    ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    AmoledInfo_t ptAmoledInfo;
    Qspiflash_t ptQspiflash;
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    int i = 0;
    switch (ctr) {

        case 0:
            memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
            break;
    
        case 1:
            memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
            while (i< IMAGE_SEZE) {
             g_tImageMainRam[i++] = 0xFF;
             g_tImageMainRam[i++] = 0x00;
             g_tImageMainRam[i++] = 0x00;
            }
            break;
            
        case 2:
            memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
            while (i< IMAGE_SEZE) {
             g_tImageMainRam[i++] = 0x00;
             g_tImageMainRam[i++] = 0xFF;
             g_tImageMainRam[i++] = 0x00;
            }
            break;
            
        case 3:
            memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
            while (i< IMAGE_SEZE) {
             g_tImageMainRam[i++] = 0x00;
             g_tImageMainRam[i++] = 0x00;
             g_tImageMainRam[i++] = 0xFF;
            }
            break;
            
        case 4:
            memset((uint8_t*)&g_tImageMainRam[0], 0x00, IMAGE_SEZE);
            while (i< IMAGE_SEZE) {
             g_tImageMainRam[i++] = 0xFF;
             g_tImageMainRam[i++] = 0xFF;
             g_tImageMainRam[i++] = 0xFF;
            }
            break;
        
        default:
            break;
    }

    if (ctr >= 0 && ctr <= 4) {
        driver_amoled_reinit();
        ptAmoledInfo.area.x1 = 0;
        ptAmoledInfo.area.y1 = 0;
        ptAmoledInfo.area.x2 =UI_HOR_RES - 1;
        ptAmoledInfo.area.y2 = UI_VER_RES - 1;
        ptAmoledInfo.data = g_tImageMainRam;
        ptAmoledInfo.len = IMAGE_SEZE;
        amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));
        sys_task_security_ms_delay(20, TASK_USB);
        return true;
    } else {
       return false;
    }
}

bool procotol_ext_flash_test(void)
{
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    uint32_t msTick = 0;
    uint32_t testMask = 0xAA55AA55;
    uint8_t *pIapBuf = get_iap_4k_buf();
    sm_log(SM_LOG_ERR, "Procotol_ext_flash_test start!\r\n");
    // 不在升级状态
    if (get_update_ui_timer_cnt() > 0) {
        return false;
    }
    while (UI_NONE != get_current_ui_detail_status()) {
        sys_task_security_ms_delay(10, TASK_USB);
    }
    
//    memset(g_tImageSecondRam, 0, 4096);
    memset(pIapBuf, 0xFF, 4096);

    uint32_t verTmp = 0;
   // uint32_t version = 0; // 备份版本 变量定义
    int res = 0;
    //1 读原数据保存（将外部FLASH原数据读出来）
    Qspiflash_t ptQspiflash;
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);

    #if 0 // 不再判断原来有什么数据， 只做写、 读、对比即可
    ptQspiflash.addr = FLASH_AREA_IMG_1_SECONDARY_START; // 使用备份区地址
    ptQspiflash.data = (uint8_t*)g_tImageSecondRam;
    ptQspiflash.len = 4096;
    res = qspiflashDev->read( (uint8_t*)&ptQspiflash, 4);
    if (res != 0) {
        sm_log(SM_LOG_ERR, "1. Read origin data failed!\r\n");
        return false;
    }
    memcpy((uint8_t *)&version, g_tImageSecondRam, 4);
    memcpy((uint8_t *)&version, g_tImageSecondRam, 4);
    if (version == 0 || version == 0xFFFFFFFF) {
        sm_log(SM_LOG_ERR, "1.1 Read origin data failed,the ver err!\r\n");
        return false;
    }else if (version == testMask) { // 之前没还原，已损坏
        sm_log(SM_LOG_ERR, "1.2 Read origin data failed, the ver was test data!\r\n");
        return false;
    }
    #endif
    // 1. 验证测试数据
    // 把版本字节替换为AA55AA55
    memcpy(pIapBuf, (uint8_t *)&testMask, 4);
    // 1.1 写测试数据
    ptQspiflash.addr = FLASH_AREA_IMG_1_SECONDARY_START;
    ptQspiflash.data = pIapBuf;
    ptQspiflash.len = 4096;
    res = qspiflashDev->write( (uint8_t*)&ptQspiflash, 4);
    if (res != 0) {
        sm_log(SM_LOG_ERR, "1. Write test data failed !\r\n");
        return false;
    }
    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 20) {

    }

    // 2 把测试数据读出
    memset(pIapBuf, 0xFF, 4096);
    ptQspiflash.addr = FLASH_AREA_IMG_1_SECONDARY_START;
    ptQspiflash.data = pIapBuf;
    ptQspiflash.len = 4096;
    res = qspiflashDev->read( (uint8_t*)&ptQspiflash, 4);
    if (res != 0) {
        sm_log(SM_LOG_ERR, "2 Read test data failed !\r\n");
        return false;
    }
    // 3验证版本是否变为AA55AA55
    memcpy((uint8_t *)&verTmp, pIapBuf, 4);
    if (verTmp != testMask) {
        sm_log(SM_LOG_ERR, "3 Verify test data failed !\r\n");
        return false;
    }
    #if 0
    // 3 还原数据
    // 3.1 把原数据写回去 
    ptQspiflash.addr = FLASH_AREA_IMG_1_SECONDARY_START;
    ptQspiflash.data = g_tImageSecondRam;
    ptQspiflash.len = 4096;
    res = qspiflashDev->write( (uint8_t*)&ptQspiflash, 4);
    if (res != 0) {
        sm_log(SM_LOG_ERR, "3. Write back origin data failed !\r\n");
        return false;
    }

    msTick = msTickDev->read( (uint8_t*)&msTick, 4);
    while (msTickDev->read( (uint8_t*)&msTick, 4) - msTick < 20) {

    }

    // 3.2 再读出来验证版本
    memset(g_tImageMainRam, 0, 4096);
    ptQspiflash.addr = FLASH_AREA_IMG_1_SECONDARY_START;
    ptQspiflash.data = g_tImageMainRam;
    ptQspiflash.len = 4096;
    res = qspiflashDev->read( (uint8_t*)&ptQspiflash, 4);
    if (res != 0) {
        sm_log(SM_LOG_ERR, "3.1 Read back origin data failed !\r\n");
        return false;
    }
    memcpy((uint8_t *)&verTmp, g_tImageMainRam, 4);
    if (verTmp != version) {
        sm_log(SM_LOG_ERR, "3.2 Verify origin data failed !\r\n");
        return false;
    }
    #endif
    return true;
}

