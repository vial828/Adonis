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

#include "ui_loading.h"
#include "driver_amoled.h"

#include "bootutil/bootutil_log.h"

#include "stdbool.h"
#include "stdio.h"

#define UI_HOR_RES      90
#define UI_VER_RES      282
#define UI_COLOR_DEPTH  3   // 888
#define BRUSH_WIDTH (3) // 笔刷宽度 n>=1; n*n
#define IMAGE_SEZE  (UI_HOR_RES * UI_VER_RES * UI_COLOR_DEPTH)

#define LOAD_SIZE 11532 // 需要与实际数组大小一致，实际数组更改时，更改此宏
#define LOAD_W 62
#define LOAD_H 62
#define LOAD_XPOS 14
#define LOAD_YPOS 110
uint8_t g_tImageMainRam[IMAGE_SEZE];
uint8_t g_tImageSecondRam[IMAGE_SEZE];

// 图片像素：62*62（W*H）
// 图片定位：14*110（原点：左上角，以偶数点定位图为准）
extern const unsigned char gImage_loading[LOAD_SIZE];

/****************************************************************************************
 * @brief   清缓存
 * @param 	none
 * @return
 * @note
 *****************************************************************************************/
void clear_disp_buff(void)
{
    memset(g_tImageMainRam, 0x00, IMAGE_SEZE);
    memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);
}

/****************************************************************************************
 * @brief   将图片数据填充到主显存
 * @param 	imageInfo, mian buff; MODE_NONE,MODE_OR,MODE_AND
 * @return	true/false
 * @note
 *****************************************************************************************/
bool fill_image_to_main_buff(uint8_t *data, uint8_t *mainBuff)
{
	int i = 0;
	int j = 0;
    int k = 0; // 扩展数据类型，防止大buff传入有数据溢出 Bug 1951315

    uint16_t widthStart = LOAD_XPOS;
    uint16_t widthEnd = LOAD_XPOS + LOAD_W;
    uint16_t heightStart = LOAD_YPOS;
    uint16_t heightEnd = LOAD_YPOS + LOAD_H;
    
    if (widthStart >= UI_HOR_RES) {
//        sm_log(SM_LOG_ERR, "ui data verify, width start:%u\r\n", widthStart);
        return false;
    }
    if (widthEnd > UI_HOR_RES) {
//        sm_log(SM_LOG_ERR, "ui data verify, width end:%u\r\n", widthEnd);
        return false;
    }
    if (heightStart >= UI_VER_RES) {
//        sm_log(SM_LOG_ERR, "ui data verify, height start:%u\r\n", heightStart);
        return false;
    }
    if (heightEnd > UI_VER_RES) {
//        sm_log(SM_LOG_ERR, "ui data verify, height end:%u\r\n", heightEnd);
        return false;
    }
    
	for (i=heightStart; i<heightEnd; i++) {
		for (j=widthStart; j<widthEnd; j++) {
			mainBuff[(j + i * UI_HOR_RES) * 3 + 0] = data[k++];
			mainBuff[(j + i * UI_HOR_RES) * 3 + 1] = data[k++];
			mainBuff[(j + i * UI_HOR_RES) * 3 + 2] = data[k++];
		}
	}
	return true;
}


/****************************************************************************************
 * @brief   将显存数据刷新到屏幕
 * @param 	main ram
 * @return	true/false
 * @note
 *****************************************************************************************/
bool amoled_disp_update(uint8_t* data)
{
    DrvAmoledInfo_t ptAmoledInfo;

    if(data == NULL) {
        return false;
    }

	ptAmoledInfo.area.x1 = 0;
	ptAmoledInfo.area.x2 = (UI_HOR_RES - 1);
	ptAmoledInfo.area.y1 = 0;
	ptAmoledInfo.area.y2 = (UI_VER_RES - 1);
	ptAmoledInfo.data = data;
	ptAmoledInfo.len = IMAGE_SEZE;
    int ret = driver_amoled_write((uint8_t*)&ptAmoledInfo, sizeof(DrvAmoledInfo_t));
	return (ret==0) ? true : false;
}

/****************************************************************************************
 * @brief   清屏
 * @param 	none
 * @return	true/false
 * @note
 *****************************************************************************************/
bool amoled_disp_clear(void)
{
	memset(g_tImageMainRam, 0x00, IMAGE_SEZE);
    //memset(g_tImageSecondRam, 0x00, IMAGE_SEZE);

    return amoled_disp_update(g_tImageMainRam);
}

static bool draw_loading_cycle(uint8_t* data, uint16_t w, uint16_t h, int angle, uint8_t mode)
{

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
//              if (angle == 0) {
//                  memset(&data[0], 0x00, canvasSize);
//              }
                memcpy(&data[m], &data[m + canvasSize], UI_COLOR_DEPTH);
            } else {
                memset(&data[m], 0x00, UI_COLOR_DEPTH);
            }
        }
    }

    return true;
}
void run_cycle_angle(uint16_t angle, uint8_t offset_angle, uint8_t mdoe)
{
    uint16_t temp_angle = angle;
    for (int i = 0; i < offset_angle; i++) {
        temp_angle++;
        if (temp_angle > 360) {
            break;
        }
        draw_loading_cycle(g_tImageSecondRam, LOAD_W, LOAD_H, temp_angle, mdoe);
    }
}
void amoled_disp_loading(void)
{
//    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);

//    static uint32_t msTick = 0;
    static uint32_t procTime = 0;
    static uint16_t page = 0;
    static uint16_t index = 0;
    static uint16_t cnt = 0;
//    static uint16_t playCnt = 0;
    static int angle = 0;
    static uint8_t startFlag = 1;
    static uint32_t ticks = 0;
    int canvasSize = LOAD_SIZE;
    #if 0 // test timer
    for (int i = 0; i < 10; i++) {
        ticks = get_ms_tick();
        while (get_ms_tick() -ticks < 1000);
        BOOT_LOG_INF("amoled_disp_loading %d!", i);
    }
    #endif
    if (startFlag == 1) {
        startFlag = 0;
        procTime = 0;
        page = 0;
        index = 0;
        cnt = 0;
        angle = 0;
        ticks = get_ms_tick();
        clear_disp_buff();
        memcpy(g_tImageSecondRam, gImage_loading, LOAD_SIZE);
        canvasSize = LOAD_SIZE;
        memcpy(&g_tImageSecondRam[canvasSize], &g_tImageSecondRam[0], (canvasSize)); // 缓冲区备份图片
    }
    
    if (get_ms_tick() -ticks < procTime) {
        return;
    }
    switch (index) {
        case 0:
            memset(&g_tImageSecondRam[0], 0x00, canvasSize); // 清主画布
            procTime = 0;
            angle= 0;
            page = 1;
            index++;
            break;
            
        case 1:
//            draw_loading_cycle(g_tImageSecondRam, LOAD_W, LOAD_H, angle, 0);
            
            if (angle < 96) { // 前12帧, 6*8 48ms播放一帧，每8°放一帧，   2*4
                page = 8;
                procTime = 6;
                run_cycle_angle(angle, 4, 0);
                angle += 4;
            } else if (angle < 320) { // 中间6帧， 40ms播放一帧，每40°放一帧
                page = 40;
                procTime = 1;
                if (angle < 120) {
                    run_cycle_angle(angle, 120 - angle, 0);
                    angle = 120;
                } else {
                    run_cycle_angle(angle, 20, 0);
                    angle += 20;
                }
            } else { // 后3帧
                page = 8;
                procTime = 6;
                if (angle < 336) {
                    run_cycle_angle(angle, 336 - angle, 0);
                    angle = 336;
                } else {
                    run_cycle_angle(angle, 4, 0);
                angle += 4;
                }
            }
            
            //angle++;
            if (angle >= 360) {
                angle = 0;
                index++;
            }
            procTime = 0;
            break;
    
        case 2:
            procTime = 45;
            cnt++;
            if (cnt >= 18) { // 18 *24
                cnt = 0;
                index++;
            }
            procTime = 0;
            break;
    
        case 3:
          //  draw_loading_cycle(g_tImageSecondRam, LOAD_W, LOAD_H, angle, 1);
            if (angle < 96) { // 前12帧
                page = 8;
                procTime = 6;
                run_cycle_angle(angle, 4, 1);
                angle += 4;
            } else if (angle < 320) { // 中间6帧
                page = 40;
                procTime = 1;
                if (angle < 120) {
                    run_cycle_angle(angle, 120 - angle, 1);
                    angle = 120;
                } else {
                    run_cycle_angle(angle, 20, 1);
                    angle += 20;
                }
            } else { // 后3帧
                page = 8;
                procTime = 6;
                if (angle < 336) {
                    run_cycle_angle(angle, 336 - angle, 1);
                    angle = 336;
                } else {
                    run_cycle_angle(angle, 4, 1);
                    angle += 4;
                }
            }
            
           // angle++;
            if (angle >= 360) {
                angle = 0;
                index++;
            }
            procTime = 0;
            break;
    
        default:
            index = 0;
            //procTime = 45;
            procTime = 0;
        //    playCnt++;
            break;  
    }
    
    if (angle % page == 0) {
        fill_image_to_main_buff(g_tImageSecondRam, g_tImageMainRam);
        amoled_disp_update(g_tImageMainRam);
    }
    ticks = get_ms_tick();
}
/*********************************************************************************/
