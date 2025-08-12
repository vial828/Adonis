/*******************************************************************************
*
 **** Copyright (C), 2020-2021, Shanghai awinic technology Co.,Ltd.
                                               all rights reserved. ************
 *******************************************************************************
 * File Name     : aw35615_driver.c
 * Author        : awinic
 * Date          : 2021-09-09
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
*
*******************************************************************************/
#include "core.h"
#include "Port.h"
#include "TypeC.h"

#include "aw35615_driver.h"

#include "driver_pd_phy.h"
#include "driver_wdg.h"
#include "driver_i2c.h"
#include "driver_tick.h"

/**********************************************************************************/
#define INT_PD_PIN      P5_2

static I2cDataType_t i2c = {
	.busN = I2C_BUS1,
	.devAddr = I2C_ADDR_AW35615,
	.wLen = 0,
	.rLen = 0
};

static cy_rslt_t pd_phy_write_bytes(uint8_t reg_addr, uint8_t *dat, uint8_t len)
{
	cy_rslt_t rslt = 0;
	uint8_t wBuf[16] = {0};

	if (!(len < sizeof(wBuf))) {
		return false;
	}

	wBuf[0] = reg_addr;
	for (uint8_t i = 0; i < len; i++) {
		wBuf[1 + i] = dat[i];
	}
	i2c.wDat = wBuf;
	i2c.wLen = len + 1;

	rslt = driver_i2c_write((uint8_t*) &i2c, 0);

	return rslt; // RET_SUCCESS (0); RET_FAILURE (-1)
}

static cy_rslt_t pd_phy_read_bytes(uint8_t reg_addr, uint8_t *dat, uint8_t len)
{
	cy_rslt_t rslt = 0;

	i2c.wDat = &reg_addr;
	i2c.wLen = 1;
	i2c.rDat = dat;
	i2c.rLen = len;

	rslt = driver_i2c_read((uint8_t*) &i2c, 0);

	return rslt; // RET_SUCCESS (0); RET_FAILURE (-1)
}

/**********************************************************************************/

#define AW35615_DRIVER_VERSION		"V1.1.0"

AW_BOOL G_HAVE_INT_READY;

static DevicePolicyPtr_t dpm;
static Port_t ports[NUM_PORTS];

void platform_delay_10us(AW_U32 delayCount)
{
	AW_U16 us = 0;

	if (delayCount > 100) {
		us = (AW_U16)(delayCount * 10 / 1000);
		pd_phy_mDelay(us);
	} else {
		pd_phy_mDelay(1u);
	}
}

extern uint32_t get_ms_tick(void);
AW_U32 get_system_time_ms(void)
{
	return (AW_U32)get_ms_tick();
}

AW_BOOL platform_get_device_irq_state(AW_U8 port)
{
	return (cyhal_gpio_read(INT_PD_PIN) == false) ? AW_TRUE : AW_FALSE;
}

AW_BOOL aw_exti_get_int_status(void)
{
	return G_HAVE_INT_READY;
}

void aw_exti_set_int_status(AW_BOOL intReady)
{
	G_HAVE_INT_READY = intReady;
}

void platform_set_pps_voltage(AW_U8 port, AW_U32 mv)
{
	//AW_LOG("Add setting voltage callback function");
	return;
}

void platform_set_pps_current(AW_U8 port, AW_U32 ma)
{
	//AW_LOG("Add setting current callback function");
	return;
}

AW_U16 platform_get_pps_voltage(AW_U8 port)
{
	return 0;
}

void handle_core_event(AW_U32 event, AW_U8 portId, void *usr_ctx, void *app_ctx)
{
	static int usb_state = 0;

	Port_t *port = aw35615_GetChip();

	if (!port) {
		AW_LOG("aw35615 - Error: port structure is NULL!\n");
		return;
	}

	switch (event) {
	case CC1_ORIENT:
	case CC2_ORIENT:
		AW_LOG("aw35615 :CC Changed=0x%x\n", event);

		if (event == CC1_ORIENT)
			AW_LOG("aw35615 change cc1\n");
		else
			AW_LOG("aw35615 change cc2\n");

		if (port->sourceOrSink == SINK) {
			usb_state = 1;
			AW_LOG("aw35615 start_usb_peripheral\n");
		} else if (port->sourceOrSink == SOURCE) {
			usb_state = 2;
			AW_LOG("aw35615 start_usb_host\n");
		}

		break;
	case CC_AUDIO_ORIENT:
		AW_LOG("aw35615 :CC_AUDIO_MODE\n");
		break;
	case CC_NO_ORIENT:
		AW_LOG("aw35615 CC_NO_ORIENT=0x%x\n", event);
		if (usb_state == 1) {
			usb_state = 0;
			AW_LOG("aw35615 - stop_usb_peripheral,event=0x%x,usb_state=%d\n",
					event, usb_state);
		} else if (usb_state == 2) {
			usb_state = 0;
			AW_LOG("aw35615 - stop_usb_host,event=0x%x,usb_state=%d\n",
					event, usb_state);
		}
		break;
	case CC_AUDIO_OPEN:
		AW_LOG("aw35615 CC_AUDIO_OPEN=0x%x\n", event);
		break;
	case PD_STATE_CHANGED:
		AW_LOG("aw35615 :PD_STATE_CHANGED=0x%x, PE_ST=%d\n",
				event, port->PolicyState);

		if (port->PolicyState == peSinkReady &&
				port->PolicyHasContract == AW_TRUE) {
			if (!port->pd_state) {
				port->pd_state = AW_TRUE;
			}
			AW_LOG("aw35615 req_obj=0x%x, sel_src_caps=0x%x\n",
					port->USBPDContract.FVRDO.ObjectPosition,
					port->SrcCapsReceived[port->USBPDContract.FVRDO.ObjectPosition - 1].object);
		} else if (port->PolicyState == peSourceReady&&
			port->PolicyHasContract == AW_TRUE) {
			if (!port->pd_state) {
				port->pd_state = AW_TRUE;
			}
		}
		break;
	case PD_NO_CONTRACT:
		AW_LOG("aw35615 :PD_NO_CONTRACT=0x%x, PE_ST=%d\n",
				event, port->PolicyState);
		break;
	case PD_NEW_CONTRACT:
		AW_LOG("aw35615 :PD_NEW_CONTRACT=0x%x, PE_ST=%d\n",
				event, port->PolicyState);
		break;
	case DATA_ROLE:
		AW_LOG("aw35615 :DATA_ROLE Conversion =0x%x\n", event);
		break;
	case POWER_ROLE:
		AW_LOG("aw35615 - POWER_ROLE Conversion event=0x%x", event);
		break;
	default:
		AW_LOG("aw35615 - default=0x%x", event);
		break;
	}
}

void aw35615_init_event_handler(void)
{
	/* max observer is 10 */
	register_observer(CC_ORIENT_ALL|PD_CONTRACT_ALL|POWER_ROLE|
			PD_STATE_CHANGED|DATA_ROLE|AUDIO_ACC|CUSTOM_SRC|EVENT_ALL,
			handle_core_event, NULL);
}

Port_t* aw35615_GetChip(void)
{
	return (Port_t *)&ports[0];
}

DevicePolicyPtr_t aw35615_GetDpm(void)
{
	return dpm;
}

AW_BOOL DeviceWrite(Port_t *port, AW_U8 regAddr, AW_U8 length, AW_U8* data)
{
    int ret = pd_phy_write_bytes(regAddr, data, length); // RET_SUCCESS (0); RET_FAILURE (-1)

    return (ret == RET_SUCCESS) ? AW_TRUE:AW_FALSE;
}

AW_BOOL DeviceRead(Port_t *port, AW_U8 regAddr, AW_U8 length, AW_U8* data)
{
	int ret = pd_phy_read_bytes(regAddr, data, length); // RET_SUCCESS (0); RET_FAILURE (-1)

    return (ret == RET_SUCCESS) ? AW_TRUE:AW_FALSE;
}

static AW_BOOL aw35615_check(Port_t *port)
{
	AW_BOOL ret = AW_FALSE;
	AW_U8 val = 0;

	for (uint8_t i=0; i<3; i++) {
		ret = DeviceRead(port, regDeviceID, 1, &val);/* Read chip id */
		if (ret == AW_FALSE) {
			pd_phy_mDelay(2);
		} else if ((val >> 4) == 0x9) {
			ret = AW_TRUE;
			sm_log(SM_LOG_INFO, "aw35615 id:0x%x\n", val);
			break;
		}
	}

	return ret;
}

int aw35615_init(void)
{
	Port_t *port = aw35615_GetChip();

	DPM_Init(&dpm);

	port[0].dpm = dpm;
	port[0].PortID = 0;
	port[0].I2cAddr = AW35615SlaveAddr;
	if (aw35615_check(port) != AW_TRUE) {
		return (-1);
	}

	core_initialize(port);
	DPM_AddPort(dpm, &ports[0]);
	aw_exti_set_int_status(AW_FALSE);
	/* init pd int trig */
	cyhal_gpio_init(INT_PD_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULL_NONE, false);

	return 0; // RET_SUCCESS (0) : RET_FAILURE (-1)
}


int aw35615_deinit(void)
{
	AW_U8 val = 0;
	Port_t *port = aw35615_GetChip();

	val = 0x01;
	DeviceWrite(port, regReset, 1, &val);
	pd_phy_mDelay(2);
	val = 0x00;
	DeviceWrite(port, regPower, 1, &val);

	return 0;
}


void aw35615_request_val(AW_U8 pos)
{
	Port_t *port = aw35615_GetChip();
	if (pos < 1) {pos = 1;}
	if (pos > 2) {pos = 2;}

	port->USBPDTxFlag = AW_TRUE;
	port->PDTransmitHeader.word = port->PolicyTxHeader.word;
	port->PDTransmitHeader.MessageType = DMTRequest;

	port->PDTransmitObjects[0].object = 0;
	port->PDTransmitObjects[0].FVRDO.ObjectPosition = pos; //select Supply_Type
	port->PDTransmitObjects[0].FVRDO.GiveBack =
			port->PortConfig.SinkGotoMinCompatible;
	port->PDTransmitObjects[0].FVRDO.NoUSBSuspend =
			port->PortConfig.SinkUSBSuspendOperation;
	port->PDTransmitObjects[0].FVRDO.USBCommCapable =
			port->PortConfig.SinkUSBCommCapable;
	port->PDTransmitObjects[0].FVRDO.OpCurrent = 200;
	port->PDTransmitObjects[0].FVRDO.MinMaxCurrent = 200;

	aw_exti_set_int_status(AW_TRUE);

	AW_LOG("PD request\n");
}

void aw35615_wdg_feed(void)
{
	int wdgTicks = 0;
	driver_wdg_write((uint8_t*)&wdgTicks, 4);
}

void aw35615_proc(void)
{
	AW_U32 timeout = 0;
	Port_t *port = aw35615_GetChip();

    static uint32_t pdIntTrigTime = 0;
    static AW_BOOL pdTrigFlag = AW_FALSE;

    static uint32_t pdRecheckTime = 0;
    static AW_BOOL pdRecheckFlag = AW_FALSE;

	if (platform_get_device_irq_state(port[0].PortID) == AW_TRUE) { // PD-phy int trig (AW_TRUE = low level)
		if (pdTrigFlag != AW_TRUE) {
			pdTrigFlag = AW_TRUE;
			pdIntTrigTime = get_system_time_ms();
			aw35615_wdg_feed(); // ÿ�δ�����ιһ�ι�
			//sm_log(SM_LOG_DEBUG, "pd int trig!\n");
		}
		aw_exti_set_int_status(AW_TRUE);
		if (get_system_time_ms() > (pdIntTrigTime + 10000)) { // trig keep time > 10s
			pdIntTrigTime = get_system_time_ms();
			aw_exti_set_int_status(AW_FALSE);
			aw35615_init(); // reinit chip
			//sm_log(SM_LOG_ERR, "pd trig err!\n");
		}
	} else {
		if (pdTrigFlag != AW_FALSE) {
			pdTrigFlag = AW_FALSE;
		}
	}
	/***********************************************************/
	if (aw_exti_get_int_status()) {
		aw_exti_set_int_status(AW_FALSE);
		pdRecheckFlag = AW_FALSE;
		// Run the state machine
		core_state_machine(port);
		if (platform_get_device_irq_state(port[0].PortID)) {
			aw_exti_set_int_status(AW_TRUE);
		} else {
			timeout = core_get_next_timeout(port);
			if (timeout > 0) {
				if (timeout == 1) {
					aw_exti_set_int_status(AW_TRUE);
				} else {
					//pdRecheckFlag = AW_TRUE;
					pdRecheckTime = get_system_time_ms();
				}
			}
		}
	}
	/***********************************************************/
	if (pdRecheckFlag == AW_TRUE) {
		if (get_system_time_ms() > (pdRecheckTime + timeout)) {
			aw_exti_set_int_status(AW_TRUE);
			sm_log(SM_LOG_ERR, "pd recheck:%d!\n", timeout);
		}
	} else {;}
}

