#include "ui_base.h"

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT) 
#include "uimage_decode.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT) 

//************************************************************************************************** */
//extern uint8_t* get_ram_main_gui(void);
//extern uint8_t* get_ram_second_gui(void);
//************************************************************************************************** */
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
 * @brief   从片外flash获得图片数据
 * @param 	imageInfo, tmp data
 * @return	true/false
 * @note
 *****************************************************************************************/
bool get_image_from_flash(ImageInfo_t* imageInfo, uint8_t *data)
{
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    Qspiflash_t ptQspiflash;

    // 升级UI、APP 、BOOT、INI时，不响应按键, 升级APP和boot时也不响应按键
    if ((get_update_ui_timer_cnt() > 0)&&(app_bt_ota_upgrade_is_running() == false)) { // BUG 1926583 BUG1935946
        amoled_display_clear();
        return false;
    }

    ImageHeaderInfo_t* imageHeaderInfo = get_image_header_info_handle();

	ptQspiflash.addr = imageInfo->addr + imageHeaderInfo->offSetAddr;
	
//	if (ptQspiflash.addr > QSPI_MAX_ADDR_LEN) { // need to modify: check customized image addr
//		if ((ptQspiflash.addr < UIMAGE_START_ADDR) || (ptQspiflash.addr >= UIMAGE_END_ADDR)) {
//			sm_log(SM_LOG_ERR, "ui data verify, addrLen:%u\r\n", ptQspiflash.addr);
//			return false;
//		}
//	}

	ptQspiflash.len = imageInfo->witdh * imageInfo->height * UI_COLOR_DEPTH;
    if (ptQspiflash.len > IMAGE_SEZE) {
        sm_log(SM_LOG_ERR, "ui data verify, dataLen:%u\r\n", ptQspiflash.len);
        return false;
    }

	ptQspiflash.data = data;
	int ret = qspiflashDev->read((uint8_t*)&ptQspiflash, 0xff);

	return (ret==0) ? true : false;
}

/****************************************************************************************
 * @brief   从片外flash获得图片数据，绝对地址
 * @param 	imageInfo, tmp data
 * @return	true/false
 * @note
 *****************************************************************************************/
bool get_image_from_flash_absolute_addr(ImageInfo_t* imageInfo, uint8_t *data)
{
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    Qspiflash_t ptQspiflash;

    // 升级UI、APP 、BOOT、INI时，不响应按键, 升级APP和boot时也不响应按键
    if ((get_update_ui_timer_cnt() > 0)&&(app_bt_ota_upgrade_is_running() == false)) { // BUG 1926583 BUG1935946
        amoled_display_clear();
        return false;
    }

	ptQspiflash.addr = imageInfo->addr;
	ptQspiflash.len = imageInfo->witdh * imageInfo->height * UI_COLOR_DEPTH;
    if (ptQspiflash.len > IMAGE_SEZE) {
        sm_log(SM_LOG_ERR, "ui data verify, dataLen:%u\r\n", ptQspiflash.len);
        return false;
    }

	ptQspiflash.data = data;
	int ret = qspiflashDev->read((uint8_t*)&ptQspiflash, 0xff);

	return (ret==0) ? true : false;
}

/****************************************************************************************
 * @brief   将图片数据填充到主显存
 * @param 	imageInfo, mian buff; MODE_NONE,MODE_OR,MODE_AND
 * @return	true/false
 * @note
 *****************************************************************************************/
bool fill_image_to_main_buff(ImageInfo_t* imageInfo, uint8_t *data, uint8_t *mainBuff, FillMode_e mode)
{
	int i = 0;
	int j = 0;
    int k = 0; // 扩展数据类型，防止大buff传入有数据溢出 Bug 1951315

    uint16_t widthStart = imageInfo->xpos;
    uint16_t widthEnd = imageInfo->xpos + imageInfo->witdh;
    uint16_t heightStart = imageInfo->ypos;
    uint16_t heightEnd = imageInfo->ypos + imageInfo->height;

    if (widthStart >= UI_HOR_RES) {
        sm_log(SM_LOG_ERR, "ui data verify, width start:%u\r\n", widthStart);
        return false;
    }
    if (widthEnd > UI_HOR_RES) {
        sm_log(SM_LOG_ERR, "ui data verify, width end:%u\r\n", widthEnd);
        return false;
    }
    if (heightStart >= UI_VER_RES) {
        sm_log(SM_LOG_ERR, "ui data verify, height start:%u\r\n", heightStart);
        return false;
    }
    if (heightEnd > UI_VER_RES) {
        sm_log(SM_LOG_ERR, "ui data verify, height end:%u\r\n", heightEnd);
        return false;
    }

    if (MODE_OR == mode) {
		for (i=heightStart; i<heightEnd; i++) {
			for (j=widthStart; j<widthEnd; j++) {
				mainBuff[(j + i * UI_HOR_RES) * 3 + 0] |= data[k++];
				mainBuff[(j + i * UI_HOR_RES) * 3 + 1] |= data[k++];
				mainBuff[(j + i * UI_HOR_RES) * 3 + 2] |= data[k++];
			}
		}
    } else if (MODE_AND == mode) {
		for (i=heightStart; i<heightEnd; i++) {
			for (j=widthStart; j<widthEnd; j++) {
				mainBuff[(j + i * UI_HOR_RES) * 3 + 0] &= data[k++];
				mainBuff[(j + i * UI_HOR_RES) * 3 + 1] &= data[k++];
				mainBuff[(j + i * UI_HOR_RES) * 3 + 2] &= data[k++];
			}
		}
    } else {
		for (i=heightStart; i<heightEnd; i++) {
			for (j=widthStart; j<widthEnd; j++) {
				mainBuff[(j + i * UI_HOR_RES) * 3 + 0] = data[k++];
				mainBuff[(j + i * UI_HOR_RES) * 3 + 1] = data[k++];
				mainBuff[(j + i * UI_HOR_RES) * 3 + 2] = data[k++];
			}
		}
    }

	return true;
}

/****************************************************************************************
 * @brief   将图片数据从外部flash填充到主显存
 * @param 	imageInfo, tmp data,  mian buff
 * @return	true/false
 * @note
 *****************************************************************************************/
bool fill_flash_image_to_main_buff(ImageInfo_t* imageInfo, uint8_t* data, uint8_t* mainBuf, FillMode_e mode)
{
	// uint8_t data[IMAGE_SEZE];
    // 升级UI、APP 、BOOT、INI时，不响应按键, 升级APP和boot时也不响应按键
    if ((get_update_ui_timer_cnt() > 0)&&(app_bt_ota_upgrade_is_running() == false)) { // BUG 1926583 BUG1935946
        amoled_display_clear();
        return false;
    }

	if (false == get_image_from_flash(imageInfo, data)) {
		sm_log(SM_LOG_DEBUG, "ui data verify failed!\r\n");
		return false;
	}

	if (false == fill_image_to_main_buff(imageInfo, data, mainBuf, mode)) {
		sm_log(SM_LOG_DEBUG, "ui pos verify failed!\r\n");
		return false;
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
	ptIoDev amoledDev = io_dev_get_dev(DEV_AMOLED);
    AmoledInfo_t ptAmoledInfo;

    if(data == NULL) {
        return false;
    }

	ptAmoledInfo.area.x1 = 0;
	ptAmoledInfo.area.x2 = (UI_HOR_RES - 1);
	ptAmoledInfo.area.y1 = 0;
	ptAmoledInfo.area.y2 = (UI_VER_RES - 1);
	ptAmoledInfo.data = data;
	ptAmoledInfo.len = IMAGE_SEZE;
	int ret = amoledDev->write((uint8_t*)&ptAmoledInfo, sizeof(AmoledInfo_t));

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

/****************************************************************************************
 * @brief   display brightness 
 * @param   mode: 0-渐灭,1-渐亮; step:0-100; dly:间隔时间 ms;
 * @return  true/false
 * @note    
 *****************************************************************************************/
/* 256-step brightness table: gamma = 2.2 */
const uint8_t gamma_table[256] = {
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,
	  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,
	  2,  2,  3,  3,  3,  3,  3,  4,  4,  4,
	  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,
	  7,  7,  8,  8,  8,  9,  9,  9, 10, 10,
	 11, 11, 11, 12, 12, 13, 13, 13, 14, 14,
	 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
	 20, 20, 21, 22, 22, 23, 23, 24, 25, 25,
	 26, 26, 27, 28, 28, 29, 30, 30, 31, 32,
	 33, 33, 34, 35, 35, 36, 37, 38, 39, 39,
	 40, 41, 42, 43, 43, 44, 45, 46, 47, 48,
	 49, 49, 50, 51, 52, 53, 54, 55, 56, 57,
	 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
	 68, 69, 70, 71, 73, 74, 75, 76, 77, 78,
	 79, 81, 82, 83, 84, 85, 87, 88, 89, 90,
	 91, 93, 94, 95, 97, 98, 99,100,102,103,
	105,106,107,109,110,111,113,114,116,117,
	119,120,121,123,124,126,127,129,130,132,
	133,135,137,138,140,141,143,145,146,148,
	149,151,153,154,156,158,159,161,163,165,
	166,168,170,172,173,175,177,179,181,182,
	184,186,188,190,192,194,196,197,199,201,
	203,205,207,209,211,213,215,217,219,221,
	223,225,227,229,231,234,236,238,240,242,
	244,246,248,251,253,255,
};

extern void driver_rm69600_write_data(uint8_t data);
extern void driver_rm69600_write_command(uint8_t data);

bool amoled_brightness_set(bool mode, uint32_t step, uint32_t dly)
{
	uint32_t val = 0;
	uint32_t interval = 0;

	uint32_t temp = 0;
	temp = amoled_brigth_get(); //获取最大亮度:0~100%
	temp *= 255;
	temp /= 100;

	if (step > temp) {
		step = temp;
	} else if (step < 1) {
		step = 1;
	} 
	interval = temp / step;

	for (uint32_t i=1; i<=step; i++) {
		val = interval * i;
		//if (val > temp) {val = temp;}
		if (i >= step) {
			val = temp;
		}
		if (mode == 0) {
			val = temp - val;
		}

		driver_rm69600_write_command(0x51);
		driver_rm69600_write_data((uint8_t)val);

		sys_task_security_ms_delay(dly, TASK_UI);
		//sm_log(SM_LOG_DEBUG, "fade:%d, %d\r\n", val, gamma_table[(uint8_t)val]);
	}

	return true;
}

/****************************************************************************************
 * @brief   amoled_backlight_set
 * @param   onOff: 0-灭,1-亮;
 * @return  
 * @note    
 *****************************************************************************************/
void amoled_backlight_set(bool onOff)
{
	uint32_t temp = 0;
    
    if (onOff != 0) {
		temp = amoled_brigth_get();
		temp *= 255;
		temp /= 100;
    } else {
		temp = 0;
		amoled_disp_clear(); // 清屏
	}

	driver_rm69600_write_command(0x51);
    driver_rm69600_write_data((uint8_t)temp);
}

/****************************************************************************************
 * @brief   amoled brightness hardware configure
 * @param
 * @return
 * @note
 *****************************************************************************************/
bool amoled_brightness_hw_cfg(uint8_t val)
{
	driver_rm69600_write_command(0x51);
	driver_rm69600_write_data(val);

	return true;
}

/****************************************************************************************
 * @brief   amoled brightness software configure
 * @param
 * @return
 * @note	for rgb888
 *****************************************************************************************/
static uint8_t pixel_ch_gray_calc(uint8_t dat, uint8_t gray)
{
	uint16_t ret = 0;

	if ((dat == 0x00) || (gray == 0x00)) {
		return (0x00);
	}

	if (gray == 0xFF) {
		return (dat);
	}

	ret = (uint16_t)dat*(uint16_t)gray;

	return (uint8_t)(ret >> 8);
}

bool amoled_brightness_sw_cfg(ImageInfo_t* imageInfo, uint8_t *data, uint8_t gray)
{
    int size = imageInfo->witdh * imageInfo->height * UI_COLOR_DEPTH;

	for (int i=0; i<size; i++) {
		data[i] = pixel_ch_gray_calc(data[i], gray);
	}

	return true;
}
/****************************************************************************************
 * @brief   draw loading cycle (顺时针描点)
 * @param   mode
 * @return  true/false
 * @note
 *
 * |___|__R__|__G__|__B__|
 * | 红 | 255 |  0  |  0  |
 * | 橙 | 255 | 152 |  0  |
 * | 黄 | 255 | 255 |  0  |
 * | 绿 |  0  | 255 |  0  |
 * | 青 |  0  | 255 | 255 |
 * | 蓝 |  0  |  0  | 255 |
 * | 紫 | 150 |  0  | 255 |
 * | 黑 |  0  |  0  |  0  |
 * | 白 | 255 | 255 | 255 |
 *
 *****************************************************************************************/
bool fill_canvas_colour(uint8_t* data, uint16_t w, uint16_t h, ImageColour_u colour)
{
	if (data == NULL) {
		return false;
	}

	if ((w == 0) || (h == 0)) {
		return false;
	}

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int k = (j + i * h) * 3;
			data[k + 0] = colour.block.r;
			data[k + 1] = colour.block.g;
			data[k + 2] = colour.block.b;
		}
	}

	return true;
}

/****************************************************************************************
 * @brief   Render the qr code 
 * @param   获取页面信息，获取缓冲区数据，页面类型
 * @return  true/false
 * @note
 *
 *****************************************************************************************/
#ifdef QR_RENDER_EN
#include "qr_encode.h"
uint8_t bitdata[QR_MAX_BITDATA]; // 用于qr显示

static int qr_generator(QrPage_e page)
{
	char *input;

	switch (page) {
		case QR_EOL:
 			input = "https://qr.myglo.com/ErPknZCS"; // eol qr
			break;
		case QR_CRITICAL:
			input = "https://qr.myglo.com/baphvkVN"; // critical qr
			break;
		case QR_DOWNLOAD:
			input = "https://qr.myglo.com/fZpbMra8"; // download qr-1216
			//input = "https://qr.myglo.com/S28b5mvr"; // download qr-1218
			break;

		default:
			break;
	}

	sm_log(SM_LOG_DEBUG, "%s, string len:%d\r\n", input, strlen(input));

	if (input[strlen(input)-1] == '\n') {
		input[strlen(input)-1] = 0;
	}

	return qr_encode(QR_LEVEL_L, 0, (const char *)input, strlen(input), bitdata); // 8 L M | 14 L M | 20 L M
}

bool draw_qr_code(ImageInfo_t* imageQrInfo, uint8_t *data, QrPage_e page)
{
	int size = 0;
	int side = 0;
	int pixel_block = 0; //放大倍数
	int pixel_len = 0;

	side = qr_generator(page);
	if (side > (90/3)) { // limit: side*3 <= 90(OLED width)
		sm_log(SM_LOG_DEBUG, "QR size is out of OLED pixel size!\r\n");
		return false;
	}

	pixel_block = 3; // (UI_HOR_RES /side); //放大倍数固定设置为3
	pixel_len = pixel_block * side;
	sm_log(SM_LOG_DEBUG, "QR_LEVEL_L, side:%d, block:%d\r\n", side, pixel_block);

	size = (pixel_len * pixel_len * UI_COLOR_DEPTH);
	memset((uint8_t*)&data[0], 0x00, size);

	for (int i=0; i<side; i++) {
		for (int j=0; j<side; j++) {
			int a = j * side + i;
			if ((bitdata[a/8] & (1<<(7-a%8)))){
				for (int l=0; l<pixel_block; l++) {
					for (int n=0; n<pixel_block; n++) {
						data[((pixel_block*j+n) + (pixel_block*i+l) * pixel_len) * 3 + 0] = 0xFF;
						data[((pixel_block*j+n) + (pixel_block*i+l) * pixel_len) * 3 + 1] = 0xFF;
						data[((pixel_block*j+n) + (pixel_block*i+l) * pixel_len) * 3 + 2] = 0xFF;
					}
				}
			}
		}
	}

	imageQrInfo->witdh = pixel_len;
	imageQrInfo->height = pixel_len;
	imageQrInfo->xpos = (UI_HOR_RES - pixel_len) / 2;
	imageQrInfo->ypos = (UI_VER_RES - pixel_len) / 2;

	return true;
}
#endif
/*****************************************************************************************/
