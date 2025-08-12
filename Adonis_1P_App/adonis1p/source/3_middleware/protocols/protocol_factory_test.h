#ifndef PROTOCOL_FACTORY_TEST_H
#define PROTOCOL_FACTORY_TEST_H



#include "protocol_base.h"



#define PAYLOAD_STA_CTR             0x01// 状态控制：[1]0x00 无效、0x01 进入休眠、0x02 进入运输模式、0x5A 复位系统
#define PAYLOAD_MTO_BEE             0x02// 马达、蜂鸣器控制 :[1]0x00 无效、0x01 马达、0x02 蜂鸣器;[2]0x00 OFF、0x01 ON
#define PAYLOAD_LED_CTR             0x03// LED控制:0x00 ALL OFF、0x01 RED ON、0x02 GREEN ON、0x03 BLUE ON、0x04 WHITE ON
#define PAYLOAD_LCD_CTR             0x04// LCD控制：0x00 SCREEN OFF、0x01 SCREEN RED、0x02 SCREEN GREEN、0x03 SCREEN BLUE、0x04 SCREEN WHITE
#define PAYLOAD_HEAT_CTR            0x05// 加热电路控制:
                                            /*   Payload [1]:
                                                    0x00 关闭输出，
                                                    0x01 设置输出电压并启动，
                                                    0x02 设置输出功率并启动，
                                                    0x03 设置目标温度并启动
                                                Payload [2~3]:电压值（mv） 或 功率值（mW）或 温度值（℃）< 16位无符号整数>
                                            */
#define PAYLOAD_EXT_FALSH           0x06// 外部flash读写测试
#define PAYLOAD_CHG_LIMIT       	0x07// 充电SOC限制、充电电压限制 预留

#define PAYLOAD_BLE_CTR             0x0E// 蓝牙控制:0x00 关闭蓝牙、0x01 打开蓝牙




#define PAYLOAD_STATIC_DATA         0x20// 获取静态数据:
                                            /*
                                                Payload[0~1]:   发热体温度（℃） < 16位整数>
                                                Payload [2~3]:  PCBA温度（℃） < 16位整数>
                                                Payload [4~5]:  Type C温度（℃） < 16位整数>
                                                Payload [6~7]:  电芯温度（℃） < 16位整数>
                                                Payload [8~9]:  电芯电压（mv） < 16位无符号整数>
                                                Payload [10~11]:    电芯电量（%） < 16位无符号整数>
                                                Payload [12~13]:    发热体阻值（mΩ）< 16位无符号整数>
                                                Payload [14~19]:    蓝牙MAC地址
                                                Payload [20~21]:电芯电流（mA） < 16位有符号整数>
                                            */
#define PAYLOAD_GET_STA             0x22// 获取状态:
                                            /*
                                                Payload[0]:
                                                        0x00 非充电状态，
                                                        0x01 充电状态
														0x12 充电限制使能,（SOC限制）非充电状态
														0x13 充电限制使能,（VOLT限制）非充电状态
														0x10 充电限制使能，非充电状态
														0x11 充电限制使能,充电状态
                                                Payload [1]:
                                                        0x00 key2 and key1 up
                                                        0x01 key2 up, key1 down
                                                        0x02 key2 down, key1 up
                                                        0x03 key2 down, key1 down
                                                Payload [2]:
                                                        0x00霍尔开关off
                                                        0x01霍尔开关 on
                                                Payload [3]:
                                                        Error code:
                                                        0x00 正常
                                                        Bit0: 0->正常，1->低压
                                                        Bit1: 0->正常，1->发热体过温
                                                        Bit2: 0->正常，1->硬件故障
                                            */


#define PAYLOAD_DCDC_ADJUST_S1       0x50// DC/DC校准Step1:0x00 关闭加热、0x01 设置1.2V加热、0x02 设置3.5V 加热
#define PAYLOAD_DCDC_ADJUST_S2       0x51// DC/DC校准Step2:Payload [1~2]:T40~T45 间电压值（mV）< 16位无符号整数>
                                            /*
                                                    Step3: 延时＞500ms，等待稳定，PC再次读取电压并判断是否在设置范围。
                                                    注：Step1~Step3, 需进行两次循环，分别为设置1.2V 及 3.5V电压下。
                                            */

#define PAYLOAD_OPA_ADJUST              0x57// DC/DC校准Step3:Payload [1~2]:T1运放输出电压值（mV）< 16位无符号整数>




typedef struct{
	uint8_t payload;//命令
	uint16_t (*func)(ProtocolBase_t* pData);//处理函数 
}payloadFun_t;



















//3.3.5测试命令（0x05）
extern uint16_t cmd_factory_test(ProtocolBase_t *pBuf);






#endif

