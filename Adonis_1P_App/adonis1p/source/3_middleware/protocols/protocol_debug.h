#ifndef PROTOCOL_DEBUG_H
#define PROTOCOL_DEBUG_H

#include "protocol_base.h"
#include "data_base_info.h"

#define PAYLOAD_DBG_PID 	0x01
#define PAYLOAD_DBG_MTO		0x02
#define PAYLOAD_DBG_BEEP	0x03
#define PAYLOAD_DBG_CHG 	0x04 // 设置充电EN
#define PAYLOAD_SET_LCD 	0x05 // 设置LCD亮度

#define PAYLOAD_SET_SOC		0x10
#define PAYLOAD_SET_VBAT	0x11
#define PAYLOAD_SET_IBAT    0x12
#define PAYLOAD_SET_TBAT    0x13
#define PAYLOAD_SET_TPCBA   0x14
#define PAYLOAD_SET_TUSB 	0x15
#define PAYLOAD_SET_THEAT   0x16
#define PAYLOAD_SET_VBUS    0x17
#define PAYLOAD_SET_IBUS    0x18
#define PAYLOAD_SET_SESSION 0x19
#define PAYLOAD_SET_TBD		0x1A

#define PAYLOAD_GET_CHG_REG	0x80 // 获取充电IC寄存器

//error code 模拟测试命令（0xF6）
uint16_t cmd_get_param(ProtocolBase_t *pBuf);
uint16_t cmd_set_param(ProtocolBase_t *pBuf);

MonitorDataInfo_t* get_debug_data_info_handle(void);
void clear_debug_data_info_handle(void);


#endif

