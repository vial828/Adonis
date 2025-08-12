#ifndef __UI_BASE_H__
#define __UI_BASE_H__

#include "platform_io.h"
#include "data_base_info.h"
#include "system_status.h"
#include "task_system_service.h"
#include "task_ui_service.h"
#include "sm_log.h"

//************************************************************************************************** */
#define FPS (25) // 帧率
#define DISP_DLY (1000 / FPS) // 每帧间隔时间

//#define QR_RENDER_EN	(1) // 20250219取消MCU生成Qr，SP阶段使用白底黑码Qr

//************************************************************************************************** */
extern uint8_t g_tImageMainRam[IMAGE_SEZE];
extern uint8_t g_tImageSecondRam[IMAGE_SEZE];

typedef union {
	uint32_t rgb;
	struct {
		uint32_t b:8;
		uint32_t g:8;
		uint32_t r:8;
		uint32_t dummy:8;
	} block;
} ImageColour_u;

typedef enum {
	MODE_NONE = 0,
	MODE_OR,
	MODE_AND,
} FillMode_e;

//************************************************************************************************** */
void clear_disp_buff(void);
bool get_image_from_flash(ImageInfo_t* imageInfo, uint8_t *data);
bool get_image_from_flash_absolute_addr(ImageInfo_t* imageInfo, uint8_t *data);
bool fill_image_to_main_buff(ImageInfo_t* imageInfo, uint8_t *data, uint8_t *mainBuff, FillMode_e mode);
bool fill_flash_image_to_main_buff(ImageInfo_t* imageInfo, uint8_t* data, uint8_t* mainBuf, FillMode_e mode);
bool amoled_disp_update(uint8_t* data);
bool amoled_disp_clear(void);
bool amoled_brightness_set(bool mode, uint32_t step, uint32_t dly);
void amoled_backlight_set(bool onOff);
bool amoled_brightness_hw_cfg(uint8_t val);
bool amoled_brightness_sw_cfg(ImageInfo_t* imageInfo, uint8_t *data, uint8_t gray);
bool fill_canvas_colour(uint8_t* data, uint16_t w, uint16_t h, ImageColour_u colour);

#ifdef QR_RENDER_EN
typedef enum {
	QR_EOL = 0,
	QR_CRITICAL,
	QR_DOWNLOAD,
} QrPage_e;

bool draw_qr_code(ImageInfo_t* imageQrInfo, uint8_t *data, QrPage_e page);
#endif

#endif
/*****************************************************************************************/
