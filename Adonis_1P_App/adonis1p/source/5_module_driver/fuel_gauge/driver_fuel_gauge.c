
/**
  ******************************************************************************
  * @file    driver_fuel_gauge.c
  * @author  vincent.he@metextech.com
  * @date    2024/06/28
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
  * 2024/06/28      V0.01      vincent.he@metextech.com     modified API
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "driver_fuel_gauge.h"
#include "bq27426.h"

/* Private define ------------------------------------------------------------*/
#define GPOUT_PIN      P9_3 // V03 POC

/* Private macro -------------------------------------------------------------*/

/* Private Consts -------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
typedef struct 
{
	bool initFlag;

	SemaphoreHandle_t xSemaphore_fuleGauge;
}BQ27426_t;

/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static BQ27426_t gs_FuleGauge;

/* Exported functions --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/****************************************************************************************
* @brief   fuel gauge init
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_fuel_gauge_init(void)
{
	int8_t ret = 0;

	gs_FuleGauge.xSemaphore_fuleGauge = xSemaphoreCreateMutex();// Create  Mutex

	ret = bq27426_init(0);
	if (true == ret) {
		gs_FuleGauge.initFlag = true;
	}

	return ((ret == true) ? RET_SUCCESS:RET_FAILURE);
}

/****************************************************************************************
* @brief   fuel gauge deinit
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_fuel_gauge_deinit(void)
{
	int8_t ret = 0;

	xSemaphoreTake(gs_FuleGauge.xSemaphore_fuleGauge,portMAX_DELAY);
	
    ret = bq27426_deinit();

	xSemaphoreGive(gs_FuleGauge.xSemaphore_fuleGauge);
    vSemaphoreDelete(gs_FuleGauge.xSemaphore_fuleGauge);
	
	return ((ret == true) ? RET_SUCCESS:RET_FAILURE);
}

/****************************************************************************************
* @brief   fuel gauge write
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_fuel_gauge_write(uint8_t *pBuf, uint16_t len)
{
	int8_t ret = 0;
	uint16_t val = 0;

    if(len==0) {
        return -1;
    }
    if(pBuf == NULL) {
        return -1;
    }
    /*********************************************************************************/
    xSemaphoreTake(gs_FuleGauge.xSemaphore_fuleGauge,portMAX_DELAY);

    switch (pBuf[0]) {
	case FG_SET_INIT:
		ret = bq27426_init(1);
		if (true == ret) {
			gs_FuleGauge.initFlag = true;
		}
		break;

    case FG_SET_SAVE_GMFS:
		ret = bq27426_save_gmfs();
		break;

    case FG_SET_TAPER_RATE:
    	val = ((uint16_t)pBuf[1] << 8) | (uint16_t)pBuf[2];
		ret = bq27426_set_taper_rate(val);
		break;

    case FG_SET_TAPER_VOLT: // setup battery volt
		val = ((uint16_t)pBuf[1] << 8) | (uint16_t)pBuf[2];
		if (val > 4400u) {val = 4400u;} // max:4400mV
		ret = bq27426_set_taper_voltage(val);
		break;

    case FG_SET_SHUTDOWN:
    	ret = bq27426_shutdown();
		break;
	
	case FG_SET_DEBUG:
		bq27426_print_gmfs();
		ret = true;
		break;

	default:
		ret = false;
		break;
    }
	xSemaphoreGive(gs_FuleGauge.xSemaphore_fuleGauge);
    /*********************************************************************************/
	return ((ret == true) ? RET_SUCCESS:RET_FAILURE);
}

/****************************************************************************************
* @brief   fuel gauge read
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_fuel_gauge_read(uint8_t *pBuf, uint16_t len)
{
	int8_t ret;

    if(len==0) {
        return -1;
    }
    if(pBuf == NULL) {
        return -1;
    }
    /*********************************************************************************/
    xSemaphoreTake(gs_FuleGauge.xSemaphore_fuleGauge,portMAX_DELAY);

    pBuf[0] = (uint8_t)bq27426_get_init_status(); // gs_FuleGauge.initFlag;
    ret = bq27426_read_param((BatInfo_t *)pBuf);

	xSemaphoreGive(gs_FuleGauge.xSemaphore_fuleGauge);

	return ((ret == true) ? RET_SUCCESS:RET_FAILURE);
}

