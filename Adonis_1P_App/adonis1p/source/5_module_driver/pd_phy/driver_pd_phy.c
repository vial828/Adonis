
#include "driver_pd_phy.h"
#include "semphr.h"

#include "aw35615_driver.h"
#include "sy69410_driver.h"

/**********************************************************************************/
typedef struct
{
	bool initFlag;
	PdPhyType_t type;
	SemaphoreHandle_t xSemaphore_pd;
}PdPhy_t;

static PdPhy_t gs_pdPhy;
//----------------------------------------------------------------------------
void pd_phy_mDelay(uint16_t ms)
{
	vTaskDelay(ms);
}

void pd_timer_refresh(void)
{
	if (gs_pdPhy.type == PD_SY69410) {
		pd_timer_add();
	} else { ;}
}

/****************************************************************************************
* @brief   pd phy init
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_pd_phy_init(void)
{
	int ret = 0;

	gs_pdPhy.xSemaphore_pd = xSemaphoreCreateMutex(); // Create  Mutex
	// 常规唤醒设备可不做PD型号判断
	if (true == gs_pdPhy.initFlag) {
		if (PD_AW35615 == gs_pdPhy.type) {
			ret = aw35615_init();
		} else {
			ret = sy69410_init();
		}

		if (RET_SUCCESS != ret) {
			gs_pdPhy.initFlag = false;
		}
	} else {
		/* check PD-phy chip id */
		ret = aw35615_init();
		if (RET_SUCCESS == ret) {
			gs_pdPhy.type = PD_AW35615;
		} else {
			ret = sy69410_init();
			if (RET_SUCCESS == ret) {
				gs_pdPhy.type = PD_SY69410;
			}
		}

		if (RET_SUCCESS != ret) {
			sm_log(SM_LOG_INFO, "No pd dev!\n");
		} else {
			gs_pdPhy.initFlag = true;
		}
	}

	return ret;
}

/****************************************************************************************
* @brief   pd phy deinit
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_pd_phy_deinit(void)
{
	int ret = 0;

	xSemaphoreTake(gs_pdPhy.xSemaphore_pd, portMAX_DELAY);

	if (gs_pdPhy.initFlag == true) {
		if (PD_SY69410 == gs_pdPhy.type ) {
			sy69410_deinit();
		} else { /* PD_AW35615 */
			aw35615_deinit();
		}
	}

	xSemaphoreGive(gs_pdPhy.xSemaphore_pd);
    vSemaphoreDelete(gs_pdPhy.xSemaphore_pd);

	return ret;
}

/****************************************************************************************
* @brief   pd phy write
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_pd_phy_write(uint8_t *pBuf, uint16_t len)
{
	int ret = 0;

    if(len==0) {
        return -1;
    }
    if(pBuf == NULL) {
        return -1;
    }
    /*********************************************************************************/
    if (gs_pdPhy.initFlag == false){
    	driver_pd_phy_init();
    	return -1;
    }
    /*********************************************************************************/
    xSemaphoreTake(gs_pdPhy.xSemaphore_pd,portMAX_DELAY);

    switch (pBuf[0]) {
	case PD_SET_PROC:
		if (gs_pdPhy.type == PD_SY69410) {
			sy69410_proc();
		} else { /* PD_AW35615 */
			aw35615_proc();
		}
		break;

	default:
		ret = RET_FAILURE;
		break;
    }

	xSemaphoreGive(gs_pdPhy.xSemaphore_pd);
    /*********************************************************************************/
	return ret;
}

/****************************************************************************************
* @brief   pd phy read
* @param
* @return  RET_SUCCESS:RET_FAILURE
* @note
****************************************************************************************
**/
int driver_pd_phy_read(uint8_t *pBuf, uint16_t len)
{
	int ret = 0;

    if(len==0) {
        return -1;
    }
    if(pBuf == NULL) {
        return -1;
    }
    /*********************************************************************************/
    if (gs_pdPhy.initFlag == false){
    	return -1;
    }
    /*********************************************************************************/
    xSemaphoreTake(gs_pdPhy.xSemaphore_pd,portMAX_DELAY);

    pBuf[0] = (uint8_t)gs_pdPhy.type; // 获取PD型号

	xSemaphoreGive(gs_pdPhy.xSemaphore_pd);
    /*********************************************************************************/
	return ret;
}


