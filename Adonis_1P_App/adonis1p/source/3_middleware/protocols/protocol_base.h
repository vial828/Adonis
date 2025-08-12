#ifndef PROTOCOL_BASE_H
#define PROTOCOL_BASE_H




#include "cyhal.h"
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sm_log.h"

#include "public_typedef.h"
#include "platform_io.h"






#define PROC_UNLOCK_CODE		0x55aa11bb//解锁密钥初始化
#define PROC_AT_LIST_LEN		7//协议至少长度为7，去掉数据域的长度
#define PROC_ACK_LEN			7//协议至少长度为7，去掉数据域的长度 





/*设备号：高4位为发送设备，低4位为接收设备*/
typedef enum
{
	DEV_PC 		= 0x01,
	DEV_1P 		= 0x02,
	DEV_2P_PEN 	= 0x03,
	DEV_2P_CASE = 0x04,
}DeviceID_e;
#define DEV_TX_TO_RX(txDev,rxDev) ((uint8_t)(txDev<<4|rxDev))



//设备端回复
#define PROC_RESPONE_ACK				0x06//无内容执行成功		数据域：0 Byte
#define PROC_RESPONE_NCK				0x15//命令未执行成功		数据域：0 Byte
#define PROC_RESPONE_DATA				0x10//有内容回复			数据域：N Byte
#define PROC_RESPONE_REPLAY				0x05//请求重传数据 		数据域:4 Byte
#define PROC_REQUEST_DATA				0x07//请求数据包			数据域:0 Byte
#define PROC_REQUEST_LOG				0x13//Log数据			数据域:1+N Byte 使能后主动上传
#define PROC_REQUEST_ACLI				0x12//执行命令行返回		数据域:N Byte


//前导符(固定)
#define PROC_AHEAD						0xA5


//命令集  PC 端发送(通信回复时间<50mS)
#define PROC_CMD_UnLock					0x21//解锁 			数据域：4 Byte
#define PROC_CMD_Lock					0x22//锁定			数据域：0 Byte
#define PROC_CMD_SetSN					0xA2//写入SN		    SN码是一串字符串，典型长度为16字节
#define PROC_CMD_GetSN					0xA3//获得 ID 值		数据域：0 Byte 
#define PROC_CMD_GetVer					0xB2//获取版本信息		数据域：10 Byte
#define PROC_CMD_StartHeat				0x26//启动加热			数据域：0 Byte
#define PROC_CMD_StopHeat				0x27//停止加热			数据域：0 Byte
#define PROC_CMD_GetTempCurve			0xB3//获取温度曲线信息	数据域：0 Byte
#define PROC_CMD_SetTempCurve			0x33//设置温度曲线信息	数据域：(组数*4) Byte


#define PROC_CMD_Get_CurAdj             0xB9// 获取电流校准值  数据域：0Byte
#define PROC_CMD_Set_CurAdj             0x39// 设置电流校准值	数据域：6Byte

#define PROC_CMD_Get_OpaAdj             0xB8// 数据域：0Byte
#define PROC_CMD_Set_OpaAdj             0x38// 设置温度参数配置	数据域：6Byte

#define PROC_CMD_Get_PowerAdj           0xB7// 数据域：0Byte
#define PROC_CMD_Set_PowerAdj           0x37// 设置温度参数配置	数据域：6Byte


#define PROC_CMD_GetPowerCurve			0xB6//获取功率曲线 		数据域：0 Byte
#define PROC_CMD_SetPowerCurve			0x36//设置功率曲线 		数据域：0 Byte

#define PROC_CMD_GetAdj					0xB5//获取温度参数配置	数据域：0Byte
#define PROC_CMD_SetAdj					0x35//设置温度参数配置	数据域：2Byte

#define PROC_CMD_GetPuff				0xB4//获取抽吸参数		数据域：0Byte or 2Byte
#define PROC_CMD_SetPuff				0x34//设置抽吸参数		数据域：(组数+2)*2Byte
#define PROC_CMD_GetTR					0xB1//获取 TR 信息		数据域：0 Byte
#define PROC_CMD_SetTR					0x31//设置 TR 信息		数据域：(组数*4) Byte

#define PROC_CMD_LogItem				0xF1//获取Log条目 		数据域：0 Byte
#define PROC_CMD_LogControl				0xF2//Log输出控制		数据域：1 Byte
#define PROC_CMD_UpdateControl			0xF3//数据更新控制		数据域：0~N Byte

#define PROC_CMD_CLI_set				0xF4//命令行指令		数据域：(组数*4) Byte

#define PROC_CMD_GetFunc				0xBB//获取产品配置		数据域：(组数*0) Byte
#define PROC_CMD_SetFunc				0xF5//设置产品配置		数据域：(组数*8) Byte

#define PROC_CMD_GetParam				0xBA//设置调试参数		数据域：(组数*0) Byte
#define PROC_CMD_SetParam				0xF6//设置调试参数		数据域：(组数*n) Byte

#define PROC_CMD_GetSysInfo				0xBC//获取系统信息		数据域：(4+1+1+2) Byte
//#define PROC_CMD_Rdpon				    0xBD//rdp on

#define PROC_CMD_GetClrSession		    0xC0// 获取/清除session

#define PROC_CMD_SetGetLogLevel		    0xC3// 设置/获取日志级别

#define PROC_CMD_SetHeatePinBThreshold	0xC5// 设置断针保护阈值
#define PROC_CMD_GetHeatePinBThreshold  0xC6// 获取断针保护阈值

#define PROC_CMD_GetRdpOnStatus			0xC7//获取RDP ON 状态，

#define PROC_CMD_FactoryReset			0xF0//恢复出厂设置		数据域：0 Byte

#define PROC_CMD_AppData				0x71//传输App数据			数据域：N Byte
#define PROC_CMD_BootData				0x72//传输Boot数据			数据域：N Byte
#define PROC_CMD_UI_Data				0x73//传输UI数据		 	数据域：N Byte
#define PROC_CMD_WRITE_INI				0x74//写ini数据			数据域：N Byte
#define PROC_CMD_READ_INI				0x75//读ini数据			数据域：N Byte

#define PROC_CMD_Heart 					0x81//2P的Pen Case间传输心跳包 		数据域：7Byte

#define PROC_CMD_FactoryTest 			0x05//测试命令				数据域：N Byte

#define PROC_CMD_SetColor				0xA6//写入Color属性
#define PROC_CMD_GetColor				0xA7//获得 Color属性 
#define PROC_CMD_SetBlePairMode			0xA8//设置/取消蓝牙配对模式
#define PROC_CMD_SetDevLock				0xA9//设置童锁锁定/解锁 
#define PROC_CMD_ClrBleData				0xAA//清除蓝牙邦定数据
#define	PROC_CMD_SetRTC					0xAB
#define	PROC_CMD_GetRTC					0xAC

#define	PROC_CMD_GetBleStatus			0xC8
#define	PROC_CMD_GetBlePairList			0xC9
#define	PROC_CMD_GetHeatingProfileSel	0xCA

#define PROC_CMD_SetGetDryHeatRefParam  0XCB
#define PROC_CMD_GetSPVerPinBrokeParam  0XCC
#define	PROC_CMD_SetHeatingProfileSel	0xD0
//case pen protoclo
typedef struct
{
	uint8_t ahead;	
	uint8_t deviceID;	
	uint8_t cmd;	
	struct dataLen
	{
		uint8_t low; 
		uint8_t high;
        
	}dataLen;
	uint8_t pData[1];
}ProtocolBase_t;



typedef struct{
	uint8_t dataCmd;//命令
	uint16_t (*func)(ProtocolBase_t* pData);//处理函数 
}CmdProtocol_t;










extern uint16_t calc_checksum(uint8_t *pBuf,uint16_t len);
extern uint16_t cmd_base_reply(ProtocolBase_t *pBuf,uint8_t replyaCMD);


extern uint32_t ComGetCodeUnLockValue(uint32_t RanCode);

extern uint16_t CRC16_CCITT(const unsigned char* data, uint32_t size);





#endif

