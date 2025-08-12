#include "protocol_debug.h"
#include "protocol_factory_test.h"
#include "driver_amoled.h"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
__WEAK bool procotol_chg_set_en(uint8_t en){return false;}//CHG DBG
__WEAK bool procotol_chg_get_en(void){return false;}//CHG DBG

static uint16_t payload_chg_ctrl_en(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;
    if (tempLen == 0x0001 && pBuf->cmd == PROC_CMD_GetParam) { // 查询
       pBuf->cmd = PROC_RESPONE_DATA;
       pBuf->pData[0] = 4;
       pBuf->pData[1] = 0;
       if (procotol_chg_get_en() == true) {
            pBuf->pData[1] = 1;
       }
       pBuf->dataLen.low = 2;
       pBuf->dataLen.high = 0;
       return cmd_base_reply(pBuf,pBuf->cmd);
    }
 	if((tempLen != 0x0002)) {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_chg_set_en(pBuf->pData[1]);

    if(retFlag==true) {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//__WEAK bool procotol_lcd_set(uint16_t val){return false;}//LCD
bool procotol_lcd_set(uint8_t val) // LCD 背光测试
{
#if 0
	if (val > 255) {val = 255;}
	if (val < 0) {val = 0;}
#endif

	driver_rm69600_write_command(0x51);
	driver_rm69600_write_data(val);

	return true;
}

static uint16_t payload_lcd_set(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;

 	if((tempLen != 0x0002)) {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_lcd_set(pBuf->pData[1]);

    if(retFlag==true) {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//__WEAK bool procotol_param_set(uint8_t optType, int16_t optCode) {return false;}

MonitorDataInfo_t g_debugDataInfo;

MonitorDataInfo_t* get_debug_data_info_handle(void)
{
    return &g_debugDataInfo;
}

void clear_debug_data_info_handle(void) // enter sleep callback
{
	g_debugDataInfo.dbgBit = 0;
}

bool procotol_param_set(uint8_t optType, int16_t val)
{
	if (optType == 0x10) {
		if (val > 100) {val = 100;} // MAX:100%
		if (val < 0) {val = 0;} // MIN:0%
		g_debugDataInfo.bat.remap_soc = val;
		g_debugDataInfo.dbgBit |= DEBUG_SOC;
	} else if (optType == 0x11) {
		if (val > 5000) {val = 5000;} // MAX:5V
		if (val < 0) {val = 0;} // MAX:0V
		g_debugDataInfo.chg.bat_volt = (uint16_t)val;
		g_debugDataInfo.bat.voltage = (uint16_t)val;
		g_debugDataInfo.dbgBit |= DEBUG_VBAT;
	} else if (optType == 0x12) {
		if (val >  10000) {val =  10000;} // MAX: 10A
		if (val < -10000) {val = -10000;} // MAX:-10A
		g_debugDataInfo.chg.bat_curr = val;
		g_debugDataInfo.bat.current = val;
		g_debugDataInfo.dbgBit |= DEBUG_IBAT;
	} else if (optType == 0x13) {
		if (val > 1000) {val = 1000;} // MAX: 1000℃
		if (val < -50) {val = -50;} // MIN: -50℃
		g_debugDataInfo.bat.temperature = val;
		g_debugDataInfo.dbgBit |= DEBUG_TBAT;
	} else if (optType == 0x14) {
		if (val > 1000) {val = 1000;} // MAX: 1000℃
		if (val < -50) {val = -50;} // MIN: -50℃
		g_debugDataInfo.det.heat_K_cood_temp = (float)val;
		g_debugDataInfo.dbgBit |= DEBUG_TPCBA;
	} else if (optType == 0x15) {
		if (val > 1000) {val = 1000;} // MAX: 1000℃
		if (val < -50) {val = -50;} // MIN: -50℃
		g_debugDataInfo.det.usb_port_temp = (float)val;
		g_debugDataInfo.dbgBit |= DEBUG_TUSB;
	} else if (optType == 0x16) {
		if (val > 1000) {val = 1000;} // MAX: 1000℃
		if (val < -50) {val = -50;} // MIN: -50℃
		g_debugDataInfo.det.heat_K_temp = (float)val;
		g_debugDataInfo.dbgBit |= DEBUG_THEAT;
	} else if (optType == 0x17) {
		if (val > 20000) {val = 20000;} // MAX:20V
		if (val < 0) {val = 0;} // MIN: 0V
		g_debugDataInfo.chg.bus_volt = (uint16_t)val;
		g_debugDataInfo.dbgBit |= DEBUG_VBUS;
	} else if (optType == 0x18) {
		if (val > 5000) {val = 5000;} // MAX:5A
		if (val < 0) {val = 0;} // MIN: 0A
		g_debugDataInfo.chg.bus_curr = val;
		g_debugDataInfo.dbgBit |= DEBUG_IBUS; // no use
	} else if (optType == 0x19) {
		if (val > 30000) {val = 30000;} // MAX:30000
		if (val < 0) {val = 0;} // MIN: 0
		g_debugDataInfo.session = (uint16_t)val;
		g_debugDataInfo.dbgBit |= DEBUG_SESSION;
	} else if (optType == 0x1A) { // DEBUG_TBD
		g_debugDataInfo.dbgBit = 0x0000; // 清除bit位
	} else {
		g_debugDataInfo.dbgBit = 0x0000; // 清除bit位
	}

	// motor_set2(PAPTIC_1); // debug
	return true;
}

static uint16_t payload_param_set(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint8_t  cmd = pBuf->pData[0];
    uint16_t val = COMB_2BYTE(pBuf->pData[2],pBuf->pData[1]);
	bool retFlag = false;
	
	if((tempLen != 0x0003))
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

	retFlag = procotol_param_set(cmd, (int16_t)val);

	if(retFlag==true)
	{
		pBuf->cmd = PROC_RESPONE_ACK;
	}else{
		pBuf->cmd = PROC_RESPONE_NCK;
	}
	return cmd_base_reply(pBuf,pBuf->cmd);
}


__WEAK bool procotol_get_chg_reg(uint8_t cmd) {return false;}
static uint16_t payload_get_chg_reg(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint8_t  cmd = pBuf->pData[0];
	bool retFlag = false;
	
	if((tempLen != 0x0001))
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

	retFlag = procotol_get_chg_reg(cmd); // 正常响应，额外打印到上位机信息

	if(retFlag==true)
	{
		pBuf->cmd = PROC_RESPONE_ACK;
	}else{
		pBuf->cmd = PROC_RESPONE_NCK;
	}
	return cmd_base_reply(pBuf,pBuf->cmd);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static const payloadFun_t payloadList[] =
{
	{PAYLOAD_DBG_CHG,		payload_chg_ctrl_en}, // 充电设置
	{PAYLOAD_SET_LCD,		payload_lcd_set}, // LCD亮度调节

	{PAYLOAD_SET_SOC,		payload_param_set},
	{PAYLOAD_SET_VBAT,		payload_param_set},
	{PAYLOAD_SET_IBAT,		payload_param_set},
	{PAYLOAD_SET_TBAT,		payload_param_set},
	{PAYLOAD_SET_TPCBA,		payload_param_set},
	{PAYLOAD_SET_TUSB,		payload_param_set},
	{PAYLOAD_SET_THEAT,		payload_param_set},
	{PAYLOAD_SET_VBUS,		payload_param_set},
	{PAYLOAD_SET_IBUS,		payload_param_set},
	{PAYLOAD_SET_SESSION,	payload_param_set},
	{PAYLOAD_SET_TBD,		payload_param_set},

	{PAYLOAD_GET_CHG_REG,	payload_get_chg_reg}, // 获取充电IC寄存器
};

uint16_t cmd_get_param(ProtocolBase_t *pBuf)
{
    uint8_t Payload0 = pBuf->pData[0];
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P, DEV_PC);

    for(uint16_t i = 0; i < eleof(payloadList); i++)
	{
		if(Payload0 == payloadList[i].payload && payloadList[i].func != NULL)
		{
			return payloadList[i].func(pBuf);
		}
	}
	
	pBuf->cmd = PROC_RESPONE_NCK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,pBuf->cmd);
}

uint16_t cmd_set_param(ProtocolBase_t *pBuf)
{
    uint8_t Payload0 = pBuf->pData[0];
 	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P, DEV_PC);

    for(uint16_t i = 0; i < eleof(payloadList); i++)
	{
		if(Payload0 == payloadList[i].payload && payloadList[i].func != NULL)
		{
			return payloadList[i].func(pBuf);
		}
	}
	pBuf->cmd = PROC_RESPONE_NCK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,pBuf->cmd);
}

