
#ifndef PROTOCOL_USB_H
#define PROTOCOL_USB_H
#include "protocol_base.h"

#include "protocol_factory_test.h"
#include "protocol_debug.h"

#define TEST_MODE       0x00
#define BASE_MODE       0x01
#define BOOST_MODE      0x02
#define CLEAN_MODE      0x03

#define BLE_0_BASE1     0x10
#define BLE_1_BASE2     0x11
#define BLE_2_BASE3     0x12
#define BLE_3_BOOST1    0x20
#define COM_UNLOCK_CODE			0x55aa11bb//解锁码

typedef enum{
	LOCK = 0,
	UNLOCK = 1,
}LockStatus_e;

typedef enum{
	STEP1 = 0,//解锁第1步
	STEP2 = 1,//解锁第2步
}LockStep_e;


typedef struct
{
	uint32_t 		unlockCode;//锁定码，解锁过程中使用
	LockStatus_e 	lockFlag;//锁定与解锁的状态
	LockStep_e		lockStep;//解锁步骤
}ProtocolUartUsb_t;

/************************************************/

/* Exported macros -----------------------------------------------------------*/
// ble profiles - DeviceInformation软件和硬件版本信息
// #define	FW_VERSION_MAJOR	1
// #define	FW_VERSION_MIN		0
// #define	SW_REVISION_PATCH	0x1300		// 大端格式
// #define	BOARD_VERSION		0x0500		// 大端格式
// #define BOOT_LOADER_VERSION	1           //

//APP && HARDWARE version
#define PROTOCOL_APP_VERSION            "AD1P_APP_4.1.1\n" // 固件版本与makefile中保持一致，用于PCTOOL版本查找，以及与签名的版本进行关联，如果签名的版本与此版本号不一致，则打印log

#define PROTOCOL_APP_SUB_VERSION_NULL ""
#define PROTOCOL_APP_SUB_VERSION_OTA "_OTA"
#define PROTOCOL_APP_SUB_VERSION_DEBUG "_debug"

#define PROTOCOL_APP_VERSION_DEBUG      PROTOCOL_APP_SUB_VERSION_NULL // "_OTA" 或"_debug"，用于增加版本后缀用, 没有后缀时用空字符串
#define PROTOCOL_HARDWARE_VERSION       "V5.0\n"

/*-------Control（0xF3，数据更新控制）-----------*/

//---Byte[0]更新区域:
#define UPDATA_AREA_APP         0x01//APP区域
#define UPDATA_AREA_BOOT        0x02//Boot区域
#define UPDATA_AREA_UI          0x03//UI素材区域
#define UPDATA_AREA_INI         0x04// ini写区域
#define UPDATA_AREA_READ_INI    0x05// ini读区域

//---Byte[1]控制命令:
#define UPDATA_CTR_START         0x01//启动更新
#define UPDATA_CTR_SET_ADDR      0x02//设置地址
#define UPDATA_CTR_READ_VERIfy   0x03//读取校验
#define UPDATA_CTR_RESULT        0x11//更新结果
#define UPDATA_CTR_TRANSFER_END  0x12//传输结束
#define UPDATA_CTR_STOP          0xE0//终止升级




#define UPDATA_BYTE01_APP_START     0x0101 //APP区域->启动更新
#define UPDATA_BYTE01_APP_FLAG      0x0111 //APP区域->数据传输完成后，更新结果标志，这时应该可以进行内部程序更新


#define UPDATA_BYTE01_BOOT_START    0x0201 //Boot区域->启动更新
#define UPDATA_BYTE01_BOOT_FLAG     0x0211 //Boot区域->数据传输完成后，更新结果标志，这时应该可以进行内部程序更新


#define UPDATA_BYTE01_UI_START    0x0301 //UI素材区域->启动更新
#define UPDATA_BYTE01_UI_FLAG     0x0311 //UI素材区域->数据传输完成后，更新结果标志，这时应该可以进行内部程序更新


#define UPDATA_BYTE01_INI_WRITE_START    0x0401 //INI区域->启动更新
#define UPDATA_BYTE01_INI_WRITE_FLAG     0x0412 //INI区域->数据传输完成后，更新结果标志，这时应该可以进行内部程序更新

#define UPDATA_BYTE01_INI_READ_START     0x0501 //INI读启动
#define UPDATA_BYTE01_INI_READ_FLAG      0x0512 //INI读结束




typedef struct
{
    bool            startFlag;//开启标志
    uint32_t        packet_total;//总包数
    uint32_t        frameIndex;
    uint32_t        offsetAddr;

}IapInfo_t;



/************************************************/


#define EVENT_ITEM                       5
#define SESSION_ITEM                     10
#define LIFECYCLE_ITEM                   13

#define MAX_ITEM                         22      //用了22个
#define MAX_LENGTH                       15      //default:12
typedef struct
{
	char data[MAX_ITEM][MAX_LENGTH];
	int length[MAX_ITEM];

}ComtLogItem_t;

typedef struct
{
	uint16_t time_L;       //32bit时间戳低16bit
	uint16_t time_H;       //32bit时间戳高16bit

}ComtLogTime_t;

typedef struct
{
	uint8_t item;          
	int16_t sp;         
	int16_t pv;       
	int16_t batv;        
	int16_t bata;        
	int16_t resoc;         
	int16_t dciv;
	int16_t dcov;
	int16_t dcoa;
	float dcow;
	float htres;
	int16_t soc;
	int16_t batt;
	int16_t coldt;
	int16_t usbt;
	int16_t usbv;
	int16_t usba;
	float power_J;
	uint8_t chg_state;
	uint8_t partab;
	uint8_t ble;
}ComtLogParam_t;

typedef struct
{
    uint8_t item;
    uint16_t batv;
    int16_t bati;
    int16_t batt;
    uint16_t soc;
    uint16_t truesoc;
    uint16_t truerm;
    uint16_t fltrm;
    uint16_t truefcc;
    uint16_t fltfcc;
    uint16_t passchg;
    uint16_t qmax;
    uint16_t dod0;
    uint16_t dodateoc;
    uint16_t paredod;
    uint16_t ctlsta;
    uint16_t flags;
}ComtFuelLogParam_t;

typedef struct
{
	char data[MAX_ITEM][MAX_LENGTH];
	int length[MAX_ITEM];

}ComtLogState_t;

typedef struct
{
	int16_t mode;   
	int16_t chg;  
	int16_t heat;  
	
}ComtLogStateParam_t;

typedef struct
{
	uint16_t frequency;
	uint64_t current_time;
	
}ComtLogControl_t;

/**
 * @brief This enumeration combines the types of
 *  0x00 -> 通用log
	0x01 -> fuelgague
	0x02 -> event
	0x03 -> session
	0x04 -> lifecycle
	0x05 -> discharge(2p)
 */
typedef enum
{
    LOG_COMMON = 0, 
    LOG_FUELGAGUE,   
    LOG_EVENT,     
    LOG_SESSION,
	LOG_LIFECYCLE,
}ComtLogType_e;

/************************************************/

int frame_parse_usb(uint8_t *pBuf,uint16_t rxLen);
uint16_t frame_create_usb(uint8_t *pTxBuf,uint8_t txCmd,uint8_t *pDataBuf,uint16_t txLen);
uint32_t get_update_ui_timer_cnt(void);
void set_update_ui_timer_cnt(uint32_t cnt);
uint8_t get_send_pc_log_type(void);
unsigned int extern_flash_program_page(unsigned char* dat, unsigned int addr);
unsigned int extern_flash_read_page(unsigned char *dat, unsigned int addr);

/*wanggankun@2024-9-5, modify, 添加APP魔术写操作 */
int app_uart_write_app_magic(void);

#endif

