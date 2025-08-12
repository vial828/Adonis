#include "protocol_usb.h"
#include "data_base_info.h"
#include "platform_io.h"
#include "system_status.h"
#include "driver_mcu_eeprom.h"
#include "usr_ecc.h"
#include "driver_fuel_gauge.h"
#include "system_param.h"
#include "err_code.h"
#include "bq27426_port.h"
#include "system_status.h"
#include "system_interaction_logic.h"
#include "task_system_service.h"
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
#include "event_data_record.h"
#include "app_bt_char_adapter.h"
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

extern const char cIniKeyTbl[INI_VAL_TBL_LEN + 2][5];
extern const int16_t cIniValTbl[INI_VAL_TBL_LEN];

static uint32_t glb_ui_address_offset = 0;

extern uint8_t g_tImageSecondRam[IMAGE_SEZE];
//static char gs_ui_version_save_buf[32] = {0};
static ProtocolUartUsb_t gs_usbInfo=
{
    .unlockCode = COM_UNLOCK_CODE,//锁定码，解锁过程中使用
    .lockFlag = UNLOCK,//锁定与解锁的状态
    .lockStep = STEP1,//解锁步骤
};


static IapInfo_t gs_iapInfo = 
{
    .startFlag = false,
    .packet_total = 0,
    .frameIndex = 0,
    .offsetAddr = 0,
};

static const uint32_t app_uart_img_magic[] = {
    0xf395c277,
    0x7fefd260,
    0x0f505235,
    0x8079b62c,
};
    /* size of the boot_image_magic in mcuboot slot */
#define APP_UART_MAGIC_SZ   (sizeof app_uart_img_magic)

static  uint32_t app_uart_magic_off(void)
{
//    return (MCU_FLASH_SECOND_SLOT_SIZE - BOOT_UART_MAGIC_SZ);
    return (FLASH_AREA_IMG_1_SECONDARY_SIZE - APP_UART_MAGIC_SZ);
}


unsigned int flash_program_sector(unsigned char* dat, unsigned int addr)
{
    ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    t_temp.addr = addr;
    t_temp.pData = dat;
    t_temp.size = MCU_FLASH_ROW_SIZE;
    int flashStatus = mcuFlashDev->write( (uint8_t*)&t_temp, t_temp.size);
    if (flashStatus != 0) {
        sm_log(SM_LOG_ERR, "CY_FLASH_STATUS_CODE:   %x\r\n", flashStatus);
    }
    if (CY_FLASH_DRV_IPC_BUSY == flashStatus) {
        sys_task_security_ms_delay(100, TASK_USB); // 此处fae建议延迟100ms,如果还是不行，flash则异常了, 中止升级
        flashStatus = mcuFlashDev->write( (uint8_t*)&t_temp, t_temp.size); 
        if (flashStatus != 0) {
            sm_log(SM_LOG_ERR, "retry failed! CY_FLASH_STATUS_CODE:   %x\r\n", flashStatus);
            return 0;
        }
    }
//    sys_task_security_ms_delay(50, TASK_USB);
    return 1;
}

unsigned int flash_read_sector(unsigned char* dat, unsigned int addr)
{
    ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    t_temp.addr = addr;
    t_temp.pData = dat;
    t_temp.size = MCU_FLASH_ROW_SIZE;
    mcuFlashDev->read( (uint8_t*)&t_temp, t_temp.size);
//    sys_task_security_ms_delay(50, TASK_USB);
    return 1;
}

#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
int app_uart_write_magic(void)
{
    uint8_t magic[APP_UART_MAGIC_SZ];
    uint32_t off;
    uint32_t addr;
    uint8_t *pIapBuf = get_iap_combine_buf();

    memcpy(magic, app_uart_img_magic, APP_UART_MAGIC_SZ);

    off = app_uart_magic_off();
    
    addr = MCU_FLASH_SECOND_SLOT_ADDR + FLASH_AREA_IMG_1_SECONDARY_SIZE - MCU_FLASH_ROW_SIZE;
    flash_read_sector(pIapBuf, addr);
    memcpy(&pIapBuf[MCU_FLASH_ROW_SIZE - APP_UART_MAGIC_SZ ], magic, APP_UART_MAGIC_SZ);
    int flashStatus = flash_program_sector(pIapBuf, addr);
    if (flashStatus == 1) { // 0失败，1成功
        return 0;
    }
    return -1;
}
#else
/*wanggankun@2024-9-5, modify, 添加APP魔术写操作 */
int app_uart_write_app_magic(void)
{
    uint32_t addr;
    uint8_t version_ctrl[24];

    memset(version_ctrl, 0xff, sizeof(version_ctrl));
    version_ctrl[23] = 0x2;

    /*wanggankun@2024-9-5, modify, 这里判断，是否是外部FLASH。*/
    if (0)
    {//内部FLASH
    	uint8_t *pIapBuf = get_iap_combine_buf();
    	addr = MCU_FLASH_START_ADDR + FLASH_AREA_IMG_1_SECONDARY_START + FLASH_AREA_IMG_1_SECONDARY_SIZE - MCU_FLASH_ROW_SIZE;
		flash_read_sector(pIapBuf, addr);
		memcpy(&pIapBuf[MCU_FLASH_ROW_SIZE - APP_UART_MAGIC_SZ ], app_uart_img_magic, APP_UART_MAGIC_SZ);
        memcpy(&pIapBuf[MCU_FLASH_ROW_SIZE - APP_UART_MAGIC_SZ - sizeof(version_ctrl) ], version_ctrl, sizeof(version_ctrl));

		int flashStatus = flash_program_sector(pIapBuf, addr);
		if (flashStatus == 1)// 0失败，1成功
		{
			return 0;
		}
    }
    else
    {//外部FLASH
    	uint8_t *pIapBuf = get_iap_4k_buf();

    	memset(pIapBuf, 0, IAP_4K_BUF_LEN);

        /**
         * FLASH_AREA_IMG_1_SECONDARY_START :=0x1d60000
         * 
         * FLASH_AREA_IMG_1_SECONDARY_SIZE :=0x0c0000
         * #define EXTERN_FLASH_PAGE_SIZE (4096u)
        */
    	addr = FLASH_AREA_IMG_1_SECONDARY_START + FLASH_AREA_IMG_1_SECONDARY_SIZE - EXTERN_FLASH_PAGE_SIZE;
        //extern_flash_read_page(pIapBuf, addr);
	    ptIoDev qspiFlashDev = io_dev_get_dev(DEV_QSPIFLASH);
	    Qspiflash_t  t_temp;
        t_temp.addr = addr;
        t_temp.data = pIapBuf;
        t_temp.len = EXTERN_FLASH_PAGE_SIZE;
		qspiFlashDev->read( (uint8_t*)&t_temp, t_temp.len);

        memcpy(&pIapBuf[IAP_4K_BUF_LEN - APP_UART_MAGIC_SZ ], app_uart_img_magic, APP_UART_MAGIC_SZ);
        memcpy(&pIapBuf[IAP_4K_BUF_LEN - APP_UART_MAGIC_SZ - sizeof(version_ctrl) ], version_ctrl, sizeof(version_ctrl));

        int flashStatus = extern_flash_program_page(pIapBuf, addr);
            
        sm_log(SM_LOG_INFO, "%s flashStatus: %d\n", __FUNCTION__, flashStatus);
        if (flashStatus == 1)// 0失败，1成功
		{
			return 0;
		}
    }
    return -1;
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

unsigned int extern_flash_program_page(unsigned char* dat, unsigned int addr)
{
    ptIoDev qspiFlashDev = io_dev_get_dev(DEV_QSPIFLASH);
    Qspiflash_t  t_temp;
    t_temp.addr = addr;
    t_temp.data = dat;
    t_temp.len = EXTERN_FLASH_PAGE_SIZE; // 需要整4K写
    qspiFlashDev->write( (uint8_t*)&t_temp, t_temp.len);
//    vTaskDelay(10);
    return 1;
}

unsigned int extern_flash_read_page(unsigned char* dat, unsigned int addr)
{
    ptIoDev qspiFlashDev = io_dev_get_dev(DEV_QSPIFLASH);
    Qspiflash_t t_temp;
    t_temp.addr = addr;
    t_temp.data = dat;
    t_temp.len = EXTERN_FLASH_PAGE_SIZE; // 需要整4K写
    qspiFlashDev->read( (uint8_t*)&t_temp, 4096);
//    vTaskDelay(10);
    return 1;
}

/*---------------------------------------------------------------------------*/
//设置SN      

static uint16_t cmd_set_devSN(ProtocolBase_t *pBuf)
{
	uint16_t tbd = 0;
	char* devID = (char*)pBuf->pData;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    if(tempLen > 16)//如果长度超过存储的长度，则返回错误
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    memcpy(g_fdb_b_u.fdb_b1_t.snNumber,devID,tempLen);
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint8_t event_data[1];
	event_data[0] = DEVICE_PARAM_SERIAL_NUMBER;
	event_record_generate(EVENT_CODE_PARAMETER_CHANGE, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

	return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
    

}


/*---------------------------------------------------------------------------*/
//获得 ID 值        数据域：0 Byte

//#define DEVICE_ID				"Adonis 1P"	//设备ID
static uint16_t cmd_get_devSN(ProtocolBase_t *pBuf)
{
//    ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
	char* devID = (char*)pBuf->pData;
   //app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    memcpy(devID,g_fdb_b_u.fdb_b1_t.snNumber,16);

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = LBYTE(16);
    pBuf->dataLen.high = HBYTE(16);
	memcpy(pBuf->pData,(uint8_t*)devID,sizeof(devID));

	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}


/*---------------------------------------------------------------------------*/
//获取版本信息		数据域：10 Byte
/*
1P PCBA测试软件版本信息：
APP 版本：    AD1P_APP_POC1_2024.05.14.1.f
Bootload 版本：AD1P_BL_RDPO_POC1_2024.05.14.1
UI版本：          UI_2024.04.24.1
INI版本：      NA
硬件版本：     V2.2
电芯ID:       （AUCOPO / COSMX / UNKNOWN）
MCU类型：    0
电量计版本：   NA 

电芯ID定义：
小于0.84V -- AUCOPO
大于1.96V – COSMX
其他—UNKNOWN

MCU类型定义：
Infineon---0，  GD—1
*/

typedef uint16_t (*pVersionFun) (uint8* pBuf);

//返回长度
__WEAK uint16_t procotol_app_version(uint8* pBuf)
{
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	char *ver = get_app_version();
    memcpy(pBuf,(uint8*)ver,strlen(ver));
    return strlen(ver);
#else
    char ver[] = "AD1P_APP_SMP1_t_2024.10.29.1.f\n";
    char vers[] = "AD1P_APP_SMP1_t_2024.10.29.1.u\n";
    if (get_rdp_on_mode() == 1) {
        memcpy(pBuf,(uint8*)vers,strlen(vers));
        return strlen(vers);
    } else {
        memcpy(pBuf,(uint8*)ver,strlen(ver));
        return strlen(ver);
    }
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

__WEAK uint16_t procotol_boot_version(uint8* pBuf)
{
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    uint32_t addr;
    uint8_t *pIapBuf = get_iap_combine_buf();
    char ver[] = "NA\n";
#ifdef DEF_BLE_APP_GROUP_EN
    addr = MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - 3*MCU_FLASH_ROW_SIZE; // BUG 1925137
#else
    addr = MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - 1*MCU_FLASH_ROW_SIZE; // BUG 1925137
#endif
    flash_read_sector(pIapBuf, addr);
    sm_log(SM_LOG_INFO, "Boot Version: %s\n", pIapBuf);

    if (strlen(pIapBuf) == 0)
    {
        memcpy(pBuf, (uint8*)ver, strlen(ver));
        return strlen(ver);
    }
    else
    {
        memcpy(pBuf, (uint8*)pIapBuf, strlen(pIapBuf));
        return strlen(pIapBuf);
    }
#else
    char ver[] = "AD1P_BL_RDP0_POA1_2024.05.22.1\n";
    
    // 倒数第三行，前部分为boot版本号32字节
    uint32_t addr = 0x10000000u + (1024* 1024) - (1 * 512);
    uint8_t *pIapBuf = get_iap_combine_buf();
    flash_read_sector(pIapBuf, addr);
    if (strlen(pIapBuf) > 64 || (pIapBuf[0] != 'A' ||  pIapBuf[1] != 'D' ||  pIapBuf[2] != '1' ||  pIapBuf[3] != 'P')) { 
        memcpy(pBuf,(uint8*)ver,strlen(ver));
        return strlen(ver);
    } else {
        if (get_rdp_on_mode() == 1 && pIapBuf[11] == '0') {
            pIapBuf[11] = '2';
            flash_program_sector(pIapBuf, addr);
        } else if (get_rdp_on_mode() == 0 && pIapBuf[11] == '2') {
            pIapBuf[11] = '0';
        }
        memcpy(pBuf,pIapBuf,strlen(pIapBuf));
        return strlen(pIapBuf);
    }
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

}

__WEAK uint16_t procotol_ui_version(uint8* pBuf)
{
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    char ver[] = "NA\n";
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();
    if (imageHeaderInfo->ver[31] != 0) { 
        imageHeaderInfo->ver[31] = 0;
    }

    sm_log(SM_LOG_INFO, "UI Version: %s\n", imageHeaderInfo->ver);

    if (strlen(imageHeaderInfo->ver) == 0)
    {
        memcpy(pBuf, (uint8*)ver, strlen(ver));
        return strlen(ver);
    }
    else
    {
        memcpy((uint8*)pBuf,(uint8*)imageHeaderInfo->ver,strlen(imageHeaderInfo->ver));
        return strlen(imageHeaderInfo->ver);
    }
#else
		//char ver[] = "UI_2024.06.03.1\n";
		//memcpy(pBuf,(uint8*)ver,strlen(ver));
	
		ImageHeaderInfo_t* imageHeaderInfo;
		imageHeaderInfo = get_image_header_info_handle();
	//	  if (imageHeaderInfo->ver[31] != 0) { 
	//		  imageHeaderInfo->ver[31] = 0;
	//		  imageHeaderInfo->ver[30] = '\n';
	//	  }
		uint16_t i = 0;
		for (i = 0; i < 32; i++) {
			if (imageHeaderInfo->ver[i] == '\n') {
				break;
			}
		}
		
		uint16_t len;
		if (i == 32) {
			imageHeaderInfo->ver[31] = '\n';
			len = 32;
		} else {
			len = i + 1;
		}
	
		memcpy((uint8*)pBuf,(uint8*)imageHeaderInfo->ver,len);
		return len;

#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
}

__WEAK uint16_t procotol_ini_version(uint8* pBuf)
{
    char verEnter[] = "\n";
    uint8_t verLen = 0;
    uint8_t verBuffer[64];//版本号的长度应该不会超过这个数组长度

    memset(verBuffer, 0, sizeof(verBuffer)); // 
    
#ifdef INI_CONFIG_DEFAULT_EN
    memcpy(verBuffer, cIniKeyTbl[0],strlen(cIniKeyTbl[0]) + 1);
#else
//   e2pTemp.addr = E2P_ADDR_OF_INI_KEY_STR;
//    e2pTemp.pDate = verBuffer; //
//    e2pTemp.size = sizeof(verBuffer);
//    e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));
#endif

    verLen = strlen(&verBuffer[0]);

    // 拷贝版本字符串
    memcpy(pBuf, &verBuffer[0],verLen );
    memcpy(&pBuf[verLen],(uint8*)verEnter,strlen(verEnter));

    sm_log(SM_LOG_INFO, "Ini Version: %s", pBuf);

    return (verLen+strlen(verEnter));

    //char ver[] = "NA\n";
    //memcpy(pBuf,(uint8*)ver,strlen(ver));
    //return strlen(ver);
}

__WEAK uint16_t procotol_harware_version(uint8* pBuf)
{
    char ver[] = PROTOCOL_HARDWARE_VERSION;
    memcpy(pBuf,(uint8*)ver,strlen(ver));
    return strlen(ver);
}
/*
电芯ID定义：
小于0.84V -- AUCOPO
大于1.96V – COSMX
其他—UNKNOWN

*/
__WEAK uint16_t procotol_cellID_version(uint8* pBuf)
{
    const char var1[] = "AUCOPO\n";
    const char var2[] = "COSMX\n";
    const char var3[] = "UNKNOWN\n";
    char*pvar = var3;

    DetectorInfo_t heat_t;
    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);

    adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));

    if((heat_t.battTypeVoltage)<0.840f)//
    {
        pvar = var1;
    }
    else if((heat_t.battTypeVoltage) > 1.960f)
    {
        pvar = var2;
    }else{
        pvar = var3;
    }

    memcpy(pBuf,(uint8*)pvar,strlen(pvar));
    return strlen(pvar);
}

__WEAK uint16_t procotol_mcu_version(uint8* pBuf)
{
    char ver[] = "0\n";
    pBuf[0] = 0x00;
    memcpy(&pBuf[1],(uint8*)ver,strlen(ver));
    return (strlen(ver)+1);
}

__WEAK uint16_t procotol_fuel_gauge_version(uint8_t* pBuf)
{
    char ver[] = "BQ27426 GMFS:01\n"; // BQ27426_GMFS_VERSION

    ver[13] = BQ27426_GMFS_VERSION % 100 / 10 + '0';
    ver[14] = BQ27426_GMFS_VERSION % 10 + '0';

    memcpy(pBuf,(uint8_t*)ver,strlen(ver));
    return strlen(ver);
}

__WEAK uint16_t procotol_chg_version(uint8_t* pBuf)
{
    char ver[] = "BQ25638\n";

    memcpy(pBuf,(uint8_t*)ver,strlen(ver));
    return strlen(ver);
}

__WEAK uint16_t procotol_pd_version(uint8_t* pBuf)
{
	char ver0[] = "SY69410\n";
	char ver1[] = "AW35615\n";

	ptIoDev pdDev = io_dev_get_dev(DEV_PD_PHY);
	uint8_t buf[1] = {0};
	pdDev->read((uint8_t *)&(buf), 1);

	if (buf[0] == PD_SY69410) {
		memcpy(pBuf,(uint8_t*)ver0,strlen(ver0));
		return strlen(ver0);
	}

	memcpy(pBuf,(uint8_t*)ver1,strlen(ver1));
    return strlen(ver1);
}

extern cy_en_smif_status_t qspi_read_flash_chip_id(uint8_t *id, uint16_t length);
__WEAK uint16_t procotol_flash_version(uint8_t* pBuf)
{
    char flashChip0[] = "GD25Q256D\n"; // MID = 0xC8
    char flashChip1[] = "BH25Q256FS\n"; // MID = 0x68

    uint8_t devId[3] = {0};
    qspi_read_flash_chip_id(devId, 3);
    if (devId[0] == 0xC8) {
    	memcpy(pBuf,(uint8_t*)flashChip0,sizeof(flashChip0));
    	return strlen(flashChip0);
    }

    memcpy(pBuf,(uint8_t*)flashChip1,sizeof(flashChip1));
    return strlen(flashChip1);
}

static const pVersionFun verFunItem[] = 
{
    procotol_app_version,
    procotol_boot_version,
    procotol_ui_version,
    procotol_ini_version,
    procotol_harware_version ,
    procotol_cellID_version,
    procotol_mcu_version,
    procotol_fuel_gauge_version,
	procotol_chg_version,
	procotol_pd_version,
	procotol_flash_version,
};


static uint16_t cmd_get_version(ProtocolBase_t *pBuf)
{
    
	uint16_t rLen = 0;
	uint16_t size = 0;
    uint8* p = pBuf->pData;

	for(uint16_t i = 0; i < eleof(verFunItem); i++)
	{
		if(verFunItem[i]!= NULL)
		{
			size = verFunItem[i](p);
            rLen+=size;
            p+=size;
		}
	}

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);

	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}

/*---------------------------------------------------------------------------*/
//启动加热		数据域：1 Byte

//__WEAK bool procotol_heat_start(uint8_t heatType){return false;}

static uint16_t cmd_heat_start(ProtocolBase_t *pBuf)
{
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
    
    if(procotol_heat_start(pBuf->pData[0])==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*---------------------------------------------------------------------------*/
//停止加热		数据域：0 Byte

__WEAK bool procotol_heat_stop(void){return false;}

static uint16_t cmd_heat_stop(ProtocolBase_t *pBuf)
{
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
    
    if(procotol_heat_stop()==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}



/*---------------------------------------------------------------------------*/
//解锁
static uint16_t cmd_unlock(ProtocolBase_t *pBuf){
	uint32_t UnLockCode;
	uint16_t rLen = 0;
    static uint8_t trngBuf[32] = {0}; // 32字节的随机数
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    
	if (pBuf->dataLen.low == 0) { // 获取随机数
        ptIoDev trngDev = io_dev_get_dev(DEV_TRNG);
        trngDev->read(trngBuf, 32);
		pBuf->cmd = PROC_RESPONE_DATA;
		rLen = 32;
        pBuf->dataLen.low = LBYTE(rLen);
        pBuf->dataLen.high = HBYTE(rLen);
        memcpy(pBuf->pData, trngBuf, 32);
	    gs_usbInfo.lockStep = STEP2;
    } else { // 验证签名解密
        if (gs_usbInfo.lockStep == STEP2 && pBuf->dataLen.low == 96) {
            sm_log(SM_LOG_INFO, "ecc_verify start!\r\n");
            if (ecc_verify(trngBuf, &pBuf->pData[0], &pBuf->pData[32])) {
                set_rdp_on_mode_no_save_flash(0);
               // gs_usbInfo.lockFlag = UNLOCK;
                pBuf->cmd = PROC_RESPONE_ACK;
                sm_log(SM_LOG_INFO, "ecc_verify succ!\r\n");

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				uint8_t event_data[1];
				event_data[0] = DEVICE_PARAM_EI_ENABLED;
				event_record_generate(EVENT_CODE_PARAMETER_CHANGE, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				
            }
            else {
                pBuf->cmd = PROC_RESPONE_NCK;
                sm_log(SM_LOG_ERR, "ecc_verify failed\r\n");

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
				uint8_t event_data[1];
				event_data[0] = DEVICE_PARAM_IS_ERROR_BLOCKED;
				event_record_generate(EVENT_CODE_PARAMETER_CHANGE, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

            }
        } else { // 防呆设计，没有获取随机数即发送解密数据，或者返回的签名数据<96字节，不处理
            if (gs_usbInfo.lockStep != STEP2) {
                sm_log(SM_LOG_ERR, "pls generate random nums first!\r\n");
            } else if (pBuf->dataLen.low != 96) {
                sm_log(SM_LOG_ERR, "the sign data len err!\r\n");
            }
            pBuf->cmd = PROC_RESPONE_NCK;
        }
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
        gs_usbInfo.lockStep = STEP1;
    }
    
	return cmd_base_reply(pBuf,pBuf->cmd);
}


//锁定
static uint16_t cmd_lock(ProtocolBase_t *pBuf)
{
    set_rdp_on_mode(1);
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
//	gs_usbInfo.lockFlag = LOCK;	
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*---------------------------------------------------------------------------*/
/*
Byte[0]更新区域:
    0x01 BASE
    0x02 BOOST
*/


//获取温度曲线信息
static uint16_t cmd_get_tempCurve(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    FDB_area_b1_u*   p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t rLen = 0;

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t * heatProfile = (heatProfile_t *)pBleData->heatProfile;;

    // if(pBuf->pData[0]>CLEAN_MODE)
    // {
    //     pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    //     return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    // }
    
    switch (pBuf->pData[0])
    {
        case TEST_MODE:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            
            memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatTestTemp[0].time), rLen);
            break;
        case BASE_MODE:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            memcpy(pBuf->pData,(uint8_t *)&(p_tHeatParamInfo->tHeatBaseTemp[0].time), rLen);
            break;
        case BOOST_MODE:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            memcpy(pBuf->pData,(uint8_t *)&(p_tHeatParamInfo->tHeatBoostTemp[0].time), rLen);
            break;
#if defined(DEF_DRY_CLEAN_EN)
        case CLEAN_MODE:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            
            memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatCleanTemp[0].time), rLen);
            break;
#endif  
        case BLE_0_BASE1:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            //指向 BASE1
            heatProfile += 0;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatTemp[0].time), rLen);
        break;  
        case BLE_1_BASE2:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            //指向 BASE2
            heatProfile += 1;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatTemp[0].time), rLen);
        break;
        case BLE_2_BASE3:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            //指向 BASE3
            heatProfile += 2;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatTemp[0].time), rLen);
        break;
        case BLE_3_BOOST1:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * 15u);
            //指向 BOOST4
            heatProfile += 3;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatTemp[0].time), rLen);
        break;
        default:
            break;
    }

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//设置温度曲线信息 
static uint16_t cmd_set_tempCurve(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//    ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
    FDB_area_b1_u*   p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low) - 1;

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t * heatProfile = (heatProfile_t *)pBleData->heatProfile;;
	heatProfileSel_t * heatProfileSel = (heatProfileSel_t *)pBleData->heatProfileSel;

    if( pBuf->pData[0]>CLEAN_MODE || tempLen > sizeof(p_tHeatParamInfo->tHeatBaseTemp))//这里还要加个长度的判断，防止溢出
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    
    switch (pBuf->pData[0])
    {
        case TEST_MODE:
            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestTemp[0].time), &pBuf->pData[1], tempLen);
            memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatTestTemp[0].time), &pBuf->pData[1], tempLen);
            app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
            break;
        case BASE_MODE:
//            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBaseTemp[0].time), &pBuf->pData[1], tempLen);
			heatProfile += (heatProfileSel->index_base_used);
			memcpy((uint8_t *)&(heatProfile->tHeatTemp[0].time), &pBuf->pData[1], tempLen);
//			app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
			set_update_BleData_status(1);
			heat_param_info_update();
            break;
        case BOOST_MODE:
//            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostTemp[0].time), &pBuf->pData[1], tempLen);
			heatProfile += (heatProfileSel->index_boost_used);
			memcpy((uint8_t *)&(heatProfile->tHeatTemp[0].time), &pBuf->pData[1], tempLen);
//			app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
			set_update_BleData_status(1);
			heat_param_info_update();
            break;
#if defined(DEF_DRY_CLEAN_EN)            
            case CLEAN_MODE: 
                memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanTemp[0].time), &pBuf->pData[1], tempLen);
                memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatCleanTemp[0].time), &pBuf->pData[1], tempLen);
                app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
            break;
#endif            
        default:
            break;
    }

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint8_t event_data[1];
	event_data[0] = PWM_COIL1_FREQ;
	event_record_generate(EVENT_CODE_PARAMETER_CHANGE, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	
	return cmd_base_reply(pBuf,pBuf->cmd);

}

/*---------------------------------------------------------------------------*/
//获取功率曲线 		数据域：0 Byte
static uint16_t cmd_get_powerTBL(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
    FDB_area_b1_u*   p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t rLen = 0;

    Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t * heatProfile = (heatProfile_t *)pBleData->heatProfile;

#if defined(DEF_DRY_CLEAN_EN)
    // if(pBuf->pData[0]>CLEAN_MODE )
    // {
    //     pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    //     return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    // }
#else
    // if(pBuf->pData[0]>BOOST_MODE )
    // {
    //     pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    //     return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    // }
#endif   
    switch (pBuf->pData[0])
    {
        case TEST_MODE:
            rLen = (uint16_t)(sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatTestPower[0].time), rLen);
            break;
        case BASE_MODE:
            rLen = (uint16_t)(sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy(pBuf->pData,(uint8_t *)&(p_tHeatParamInfo->tHeatBasePower[0].time), rLen);
            break;
        case BOOST_MODE:
            rLen = (uint16_t)(sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy(pBuf->pData,(uint8_t *)&(p_tHeatParamInfo->tHeatBoostPower[0].time), rLen);
            break;
#if defined(DEF_DRY_CLEAN_EN)
        case CLEAN_MODE:
            rLen = (uint16_t)(sizeof(HeatPower_t) * MAX_POWER_CTRL_POINT);
            memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatCleanPower[0].time), rLen);
            break; 
#endif
        case BLE_0_BASE1:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * MAX_POWER_CTRL_POINT);
            //指向 BASE1
            heatProfile += 0;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatPower[0]), rLen);
        break;  
        case BLE_1_BASE2:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * MAX_POWER_CTRL_POINT);
            //指向 BASE2
            heatProfile += 1;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatPower[0]), rLen);
        break;
        case BLE_2_BASE3:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * MAX_POWER_CTRL_POINT);
            //指向 BASE3
            heatProfile += 2;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatPower[0]), rLen);
        break;
        case BLE_3_BOOST1:
            rLen = (uint16_t)(sizeof(HeatTemp_t) * MAX_POWER_CTRL_POINT);
            //指向 BOOST1
            heatProfile += 3;
            memcpy(pBuf->pData,(uint8_t *)&(heatProfile->tHeatPower[0]), rLen);
        break;
        default:
            break;
    }

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}



//设置功率曲线 		数据域：0 Byte
static uint16_t cmd_set_powerTBL(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//    ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low) - 1;
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t * heatProfile = (heatProfile_t *)pBleData->heatProfile;;
	heatProfileSel_t * heatProfileSel = (heatProfileSel_t *)pBleData->heatProfileSel;
#if defined(DEF_DRY_CLEAN_EN)
    if( pBuf->pData[0]>CLEAN_MODE || tempLen > sizeof(p_tHeatParamInfo->tHeatBasePower) )//这里还要加个长度的判断，防止溢出
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
#else
    if( pBuf->pData[0]>BOOST_MODE || tempLen > sizeof(p_tHeatParamInfo->tHeatBasePower) )//这里还要加个长度的判断，防止溢出
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
#endif

    
    switch (pBuf->pData[0])
    {
        case TEST_MODE:
            memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatTestPower[0].time), &pBuf->pData[1], tempLen);
            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatTestPower[0].time), &pBuf->pData[1], tempLen);
            app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
            break;
		case BASE_MODE:
//			  memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBasePower[0].time), &pBuf->pData[1], tempLen);
			heatProfile += (heatProfileSel->index_base_used);
			memcpy((uint8_t *)&(heatProfile->tHeatPower[0].time), &pBuf->pData[1], tempLen);
//			app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
			set_update_BleData_status(1);
			heat_param_info_update();
			break;
		case BOOST_MODE:
//			  memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatBoostPower[0].time),&pBuf->pData[1], tempLen);
			heatProfile += (heatProfileSel->index_boost_used);
			memcpy((uint8_t *)&(heatProfile->tHeatPower[0].time), &pBuf->pData[1], tempLen);
//			app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
			set_update_BleData_status(1);
			heat_param_info_update();
			break;
 #if defined(DEF_DRY_CLEAN_EN)
        case CLEAN_MODE:
            memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.tHeatCleanPower[0].time), &pBuf->pData[1], tempLen);
            memcpy((uint8_t *)&(p_tHeatParamInfo->tHeatCleanPower[0].time), &pBuf->pData[1], tempLen);
            app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
            break; 
#endif           

        default:
            break;
    }
 


	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	uint8_t event_data[1];
	event_data[0] = PWM_COIL1_DUTY_CYCLE;
	event_record_generate(EVENT_CODE_PARAMETER_CHANGE, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	
	return cmd_base_reply(pBuf,pBuf->cmd);

}
/*---------------------------------------------------------------------------*/
// 获取 DCDC 输出电流 校准参数 返回 2 + 4 字节
static uint16_t cmd_get_curr_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t rLen = 6;
    memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.adcCurrAdjB), 2);
    memcpy(&pBuf->pData[2],(uint8_t *)&(p_fdb_b_info->fdb_b1_t.adcCurrAdjK),4);
    // cmd req
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//设置 DCDC 输出电流	数据域：2Byte + 4Byte
static uint16_t cmd_set_curr_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    //data copy
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.adcCurrAdjB), pBuf->pData, 2);
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.adcCurrAdjK), &pBuf->pData[2], 4);
    // 保存校准值
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    // cmd req
	pBuf->deviceID  = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd       = PROC_RESPONE_ACK;
    pBuf->dataLen.low   = 0;
    pBuf->dataLen.high  = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}

// 获取 功率 校准参数 返回 2 + 4 字节
static uint16_t cmd_get_opa_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t rLen = 6;
    memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.opaAdjB), 2);
    memcpy(&pBuf->pData[2],(uint8_t *)&(p_fdb_b_info->fdb_b1_t.opaAdjK),4);
    // cmd req
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}
//设置温度参数配置	数据域：2Byte + 4Byte
static uint16_t cmd_set_opa_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    //data copy
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.opaAdjB), pBuf->pData, 2);
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.opaAdjK), &pBuf->pData[2], 4);
    // 保存校准值
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    // cmd req
	pBuf->deviceID  = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd       = PROC_RESPONE_ACK;
    pBuf->dataLen.low   = 0;
    pBuf->dataLen.high  = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}

// 获取 功率 校准参数 返回 2 + 4 字节
static uint16_t cmd_get_power_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t rLen = 6;
    memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.powerAdjB), 2);
    memcpy(&pBuf->pData[2],(uint8_t *)&(p_fdb_b_info->fdb_b1_t.powerAdjK),4);
    // cmd req
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}
//设置 功率 校准参数	数据域：2Byte + 4Byte
static uint16_t cmd_set_power_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    // data copy
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.powerAdjB), pBuf->pData, 2);
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.powerAdjK), &pBuf->pData[2], 4);
    // 保存校准值
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    // cmd req
	pBuf->deviceID  = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd       = PROC_RESPONE_ACK;
    pBuf->dataLen.low   = 0;
    pBuf->dataLen.high  = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//获取温度参数配置	数据域：返回 2 + 4 字节
static uint16_t cmd_get_temp_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t rLen = 0;
    rLen = 6;
    memcpy(pBuf->pData,(uint8_t *)&(p_fdb_b_info->fdb_b1_t.tempAdjB), 2);
    memcpy(&pBuf->pData[2],(uint8_t *)&(p_fdb_b_info->fdb_b1_t.tempAdjK),4);
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//设置温度参数配置	数据域：2Byte + 4Byte
static uint16_t cmd_set_temp_adjust(ProtocolBase_t *pBuf)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
    // data copy
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.tempAdjB), pBuf->pData, 2);
    memcpy((uint8_t *)&(p_fdb_b_info->fdb_b1_t.tempAdjK), &pBuf->pData[2], 4);
    // 保存校准值
    app_param_write(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
    // cmd req
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}

/*---------------------------------------------------------------------------*/
//获取抽吸参数		数据域：0Byte or 2Byte
static uint16_t cmd_get_puff(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
	uint16_t rLen = 0;
    rLen = (uint16_t)sizeof(PuffInfo_t);
    memcpy(pBuf->pData,(uint8_t *)&(p_tHeatParamInfo->tPuffInfo.maxPuff), rLen);
    // cmd req
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}



//设置抽吸参数       数据域：(组数+2)*2Byte

static uint16_t cmd_set_puff(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//    ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
	uint16_t tempLen = 0;

	tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    memcpy((uint8_t *)&(p_tHeatParamInfo->tPuffInfo.maxPuff), pBuf->pData, tempLen);
//    paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
//    paramDev->read( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));

    
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);

}

/*---------------------------------------------------------------------------*/

//获取 TR 信息
static uint16_t cmd_get_TR(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
	uint16_t rLen = 0;
    rLen = (uint16_t)(sizeof(TrInfo_t) * MAX_TR_TBL_GR);
    memcpy(pBuf->pData,(uint8_t *)&(p_tHeatParamInfo->tTrInfo[0].tempeture), rLen);

    
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}


//设置 TR 信息
static uint16_t cmd_set_TR(ProtocolBase_t *pBuf)
{
    HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//    ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
	uint16_t tempLen = 0;

	tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    memcpy((uint8_t *)&(p_tHeatParamInfo->tTrInfo[0].tempeture), pBuf->pData, tempLen);
//    paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));
//    paramDev->read( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));

    
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*---------------------------------------------------------------------------*/
//数据更新控制        数据域：0~N Byte
/*
Byte[0]: 
        0x01 APP区域
        0x02 Boot区域
        0x03 UI素材区域
        0x04 ini区域

Byte[1]:
        0x01  启动更新
        0x02  设置地址
        0x03  读取校验
        0x11  更新结果
        0x12  传输结束
        0xE0  终止升级
        
Byte[2~N]参数:
        Byte[1]=0x01：Byte[2~5]总包数
                      Byte[6~N]文件信息
        Byte[1]=0x02：Byte[2~5]=起始地址
        Byte[1]=0x11：Byte[2] = 0x00失败，
        Byte[2] = 0x01成功
*/

// app 更新处理程序
bool updata_ctr_app(ProtocolBase_t *pBuf)
{
    gs_iapInfo.frameIndex = 0;
    gs_iapInfo.packet_total = COMB_4BYTE(pBuf->pData[5],pBuf->pData[4],pBuf->pData[3],pBuf->pData[2]);
    gs_iapInfo.startFlag = true;
    gs_iapInfo.offsetAddr = 0;

    return true;
}

// app 更新升级标志
bool updata_app_flag(ProtocolBase_t *pBuf)
{

    if(pBuf->pData[2] == 0x01)
    {
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    	//wanggankun@2024-9-5, modify, 添加外部FLASH写实现
        int flashStatus = app_uart_write_app_magic();//app_uart_write_magic();
#else
		int flashStatus = app_uart_write_magic();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
        if (flashStatus == 0) {
			
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//			set_time_stamp_to_flash();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			flash_data_save_change(0);		// 带时间戳
            sys_task_security_ms_delay(1000, TASK_USB);
            NVIC_SystemReset(); // 这一回复帧回不去了
        }
    }
    return false;
}

// ui 更新升级标志
bool updata_ui_flag(ProtocolBase_t *pBuf)
{
    if(pBuf->pData[2] == 0x01)
    {
        uint8_t buffer[10] = {0};
        Qspiflash_t ptQspiflash;
        ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);

        //wanggankun@2024-11-22, add, for image upgrade
        ptQspiflash.addr = EXT_FLASH_UI_FLAG_START_ADDRESS;
        ptQspiflash.data = &buffer[0];
        ptQspiflash.len = 10;
        qspiflashDev->read( (uint8_t*)&ptQspiflash, 10);

        sm_log(SM_LOG_DEBUG,"IMGAE FLAG: 0x%x\n", buffer[0]);

        if(buffer[0] == CY_OTA_IMAGE_USE_UI1 || buffer[0] == 0xFF )
        {
            sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI2_START_ADDRESS: 0x%x\n", EXT_FLASH_UI2_START_ADDRESS);
            memset(buffer, CY_OTA_IMAGE_USE_UI2, 10);
        }
        else if(buffer[0] == CY_OTA_IMAGE_USE_UI2)
        {
            sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI1_START_ADDRESS: 0x%x\n", EXT_FLASH_UI1_START_ADDRESS);
            memset(buffer, CY_OTA_IMAGE_USE_UI1, 10);
        }
        else
        {            
            memset(buffer, CY_OTA_IMAGE_USE_UI1, 10);
        }

        qspiflashDev->write( (uint8_t*)&ptQspiflash, 10);

//        app_uart_write_magic();
        //---------------------------末完待续
//        taskENTER_CRITICAL(); 
        /*写升级标志到FALSH*/
        
//        taskEXIT_CRITICAL();
        //---------------------------末完待续

        //复位系统
        
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//		set_time_stamp_to_flash();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		flash_data_save_change(0);		// 带时间戳
        sys_task_security_ms_delay(1000, TASK_USB);
        NVIC_SystemReset();
    }

    return true;//这一回复帧回不去了
}

// ini 更新升级标志
bool updata_ini_flag(ProtocolBase_t *pBuf)
{

    if(pBuf->pData[2] == 0x01)
    {
//        app_uart_write_magic();
        //---------------------------末完待续
//        taskENTER_CRITICAL(); 
        /*写升级标志到FALSH*/
        
//        taskEXIT_CRITICAL();
        //---------------------------末完待续

        //复位系统#
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//			set_time_stamp_to_flash();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//        NVIC_SystemReset();
    }

    return true;//这一回复帧回不去了
}

// app 更新处理程序
bool updata_ctr_boot(ProtocolBase_t *pBuf)
{
    gs_iapInfo.frameIndex = 0;
    gs_iapInfo.packet_total = COMB_4BYTE(pBuf->pData[5],pBuf->pData[4],pBuf->pData[3],pBuf->pData[2]);
    gs_iapInfo.startFlag = true;
    gs_iapInfo.offsetAddr = 0;

    return true;
}

// ui 素材 更新处理程序
bool updata_ctr_ui(ProtocolBase_t *pBuf)
{
    gs_iapInfo.frameIndex = 0;
    gs_iapInfo.packet_total = COMB_4BYTE(pBuf->pData[5],pBuf->pData[4],pBuf->pData[3],pBuf->pData[2]);
    gs_iapInfo.startFlag = true;
    gs_iapInfo.offsetAddr = 0;

    //wanggankun@2024-11-22, add, for image upgrade
    ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
    Qspiflash_t ptQspiflash;
    uint8_t buffer = 0x88;
    ptQspiflash.addr = EXT_FLASH_UI_FLAG_START_ADDRESS;
    ptQspiflash.data = &buffer;
    ptQspiflash.len = 1;
    qspiflashDev->read( (uint8_t*)&ptQspiflash, 1);

    sm_log(SM_LOG_DEBUG,"IMGAE FLAG: 0x%x\n", buffer);

    if(buffer == CY_OTA_IMAGE_USE_UI1 || buffer == 0xFF )
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI2_START_ADDRESS: 0x%x\n", EXT_FLASH_UI2_START_ADDRESS);
        glb_ui_address_offset = EXT_FLASH_UI2_START_ADDRESS;
    }
    else if(buffer == CY_OTA_IMAGE_USE_UI2)
    {
    	sm_log(SM_LOG_DEBUG,"EXT_FLASH_UI1_START_ADDRESS: 0x%x\n", EXT_FLASH_UI1_START_ADDRESS);
        gs_iapInfo.offsetAddr =  EXT_FLASH_UI1_START_ADDRESS;
    }
    else
    {
        gs_iapInfo.offsetAddr =  EXT_FLASH_UI1_START_ADDRESS;
    }

    gs_iapInfo.offsetAddr =  glb_ui_address_offset; 

    return true;
}

// ini 读处理程序
bool updata_ctr_read_ini(void)
{
    uint32_t addrOffset = 1024 * 10; // 10K byte偏移
    // 读取KEY
    memset(g_tImageSecondRam, 0, IMAGE_SEZE); // 副显存可以共用

    int j = 0;

#ifdef INI_CONFIG_DEFAULT_EN
    for (int i = 0; i < INI_VAL_TBL_LEN + 2; i++) {
        strcpy(&g_tImageSecondRam[j + addrOffset], cIniKeyTbl[i]);
        j += (strlen(cIniKeyTbl[i]) + 1);
    }
#else
    e2pTemp.addr = E2P_ADDR_OF_INI_KEY_STR;
    e2pTemp.pDate = &g_tImageSecondRam[addrOffset]; //
    e2pTemp.size = E2P_SIZE_OF_INI_KEY_STR;
    e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));
#endif

    int16_t *pVal = get_ini_val_info_handle();
    j = 0;
    uint32_t addr = 0;
    // 拷贝版本字符串
    memcpy(&g_tImageSecondRam[addr], &g_tImageSecondRam[j + addrOffset], (strlen(&g_tImageSecondRam[j + addrOffset]) + 1));
    addr = addr + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
    j = j + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
    // 拷贝长度字符串
    memcpy(&g_tImageSecondRam[addr], &g_tImageSecondRam[j + addrOffset], (strlen(&g_tImageSecondRam[j + addrOffset]) + 1));
    addr = addr + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
    j = j + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
    for (int i = 0; i < INI_VAL_TBL_LEN; i++) { // 组合成键值对
        memcpy(&g_tImageSecondRam[addr], &g_tImageSecondRam[j + addrOffset], (strlen(&g_tImageSecondRam[j + addrOffset]) + 1)); // key
        addr = addr + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
        j = j + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
        memcpy(&g_tImageSecondRam[addr], (uint8_t *)&pVal[i], 2);
        addr += 2;
    }
    uint32_t length = addr;
    if (length % 256 == 0) {
        gs_iapInfo.packet_total = length / 256;
    } else {
        gs_iapInfo.packet_total = (length / 256) + 1;
    }
    gs_iapInfo.frameIndex = 0;
    gs_iapInfo.startFlag = true;
    gs_iapInfo.offsetAddr = 0;
    return true;
}


// ini 写 处理程序
bool updata_ctr_write_ini(ProtocolBase_t *pBuf)
{
    gs_iapInfo.frameIndex = 0;
    gs_iapInfo.packet_total = COMB_4BYTE(pBuf->pData[5],pBuf->pData[4],pBuf->pData[3],pBuf->pData[2]);
    gs_iapInfo.startFlag = true;
    gs_iapInfo.offsetAddr = 0;
    return true;
}

// boot 更新升级标志,boot 直接更新了
bool updata_boot_flag(ProtocolBase_t *pBuf)
{
    uint32_t rowNum;
	
#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
    uint8_t *pIapBuf = get_iap_combine_buf();
    uint32_t addr;
    if(pBuf->pData[2] == 0x01 && gs_iapInfo.frameIndex ==gs_iapInfo.packet_total && gs_iapInfo.frameIndex != 0)
    {
    
        // 升级boot不能断电，否则可能变砖
        // 把备份区boot拷贝到mcuboot区
        if ((gs_iapInfo.packet_total % 2) != 0) {
            rowNum = (gs_iapInfo.packet_total + 1) / 2;
        } else {
            rowNum = gs_iapInfo.packet_total / 2;
        }
        for(uint32_t i = 0; i < rowNum; i++) {
            addr = MCU_FLASH_SECOND_SLOT_ADDR + (i * MCU_FLASH_ROW_SIZE);
            flash_read_sector(pIapBuf, addr);
            sys_task_security_ms_delay(50, TASK_USB);
            addr = MCU_FLASH_START_ADDR + (i * MCU_FLASH_ROW_SIZE);
            flash_program_sector(pIapBuf, addr);
            sys_task_security_ms_delay(50, TASK_USB);
        }
#else

    if(pBuf->pData[2] == 0x01 && gs_iapInfo.frameIndex ==gs_iapInfo.packet_total && gs_iapInfo.frameIndex != 0)
    {
    
        // 升级boot不能断电，否则可能变砖
        // 把备份区boot拷贝到mcuboot区
        /*wanggankun@2024-9-5, modify, 这里判断，是否是外部FLASH。*/
        if (0)
        {//内部FLASH
            if ((gs_iapInfo.packet_total % 2) != 0) {
                rowNum = (gs_iapInfo.packet_total + 1) / 2;
            } else {
                rowNum = gs_iapInfo.packet_total / 2;
            }

        	uint8_t *pIapBuf = get_iap_combine_buf();
        	for(uint32_t i = 0; i < rowNum; i++)
        	{
				flash_read_sector(pIapBuf,  MCU_FLASH_START_ADDR + FLASH_AREA_BOOTLOADER_START + (i * MCU_FLASH_ROW_SIZE));
				sys_task_security_ms_delay(50, TASK_USB);
				flash_program_sector(pIapBuf, MCU_FLASH_START_ADDR + FLASH_AREA_BOOTLOADER_START + (i * MCU_FLASH_ROW_SIZE));
				sys_task_security_ms_delay(50, TASK_USB);
        	}
        }
        else
        {//外部FLASH
            if ((gs_iapInfo.packet_total % 16) != 0) {
                rowNum = (gs_iapInfo.packet_total/16 + 1);
            } else {
                rowNum = gs_iapInfo.packet_total / 16;
            }

        	uint8_t *pIapBuf = get_iap_4k_buf();
        	Qspiflash_t  t_temp;
        	ptIoDev qspiFlashDev = io_dev_get_dev(DEV_QSPIFLASH);

        	for(uint32_t i = 0; i < rowNum; i++)
        	{
				//extern_flash_read_page(pIapBuf, addr);
	            t_temp.addr = FLASH_AREA_IMG_1_SECONDARY_START + (i * EXTERN_FLASH_PAGE_SIZE);
	            t_temp.data = pIapBuf;
	            t_temp.len = EXTERN_FLASH_PAGE_SIZE;
				qspiFlashDev->read( (uint8_t*)&t_temp, t_temp.len);

				for (uint32_t j = 0; j < EXTERN_FLASH_PAGE_SIZE/MCU_FLASH_ROW_SIZE; j++)
				{
//					sm_log(SM_LOG_ERR, "begin write: j 0x%x, len 0x%x, addr 0x%x\r\n", j, EXTERN_FLASH_PAGE_SIZE/MCU_FLASH_ROW_SIZE, MCU_FLASH_START_ADDR + FLASH_AREA_BOOTLOADER_START + (i * EXTERN_FLASH_PAGE_SIZE) + j * MCU_FLASH_ROW_SIZE);
					sys_task_security_ms_delay(50, TASK_USB);
					flash_program_sector(&pIapBuf[j*MCU_FLASH_ROW_SIZE], MCU_FLASH_START_ADDR + FLASH_AREA_BOOTLOADER_START + (i * EXTERN_FLASH_PAGE_SIZE) + j * MCU_FLASH_ROW_SIZE);
					sys_task_security_ms_delay(50, TASK_USB);
				}
        	}
        }
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
        //复位系统

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
//		set_time_stamp_to_flash();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		flash_data_save_change(0);		// 带时间戳
        NVIC_SystemReset();
    }

    return true;//这一回复帧回不去了
}



volatile uint32_t g_updateUiBusyCnt = 0;
void set_update_ui_timer_cnt(uint32_t cnt)
{
    g_updateUiBusyCnt = cnt; // 5000 UI十秒超时,INI 6秒超时， 系统任务2ms运行一次
}

uint32_t get_update_ui_timer_cnt(void)
{
    if (g_updateUiBusyCnt > 0) {
        g_updateUiBusyCnt--;
    }
    return g_updateUiBusyCnt;
}
//更新控制命令入口 
static uint16_t cmd_updata_control(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
	uint16_t updataType = COMB_2BYTE(pBuf->pData[0],pBuf->pData[1]);//将BYTE[0]与BYTE[1]合并成为一个类型来使用
    bool retFlag;
    
    if(pBuf->pData[0]>UPDATA_AREA_READ_INI || pBuf->pData[1] > UPDATA_CTR_STOP)
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }

    if (get_rdp_on_mode() == 1) { // RDP ON 不能升级boot
        if (updataType == UPDATA_BYTE01_BOOT_START || updataType == UPDATA_BYTE01_BOOT_FLAG) { // 不可以升级boot
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
        }
    }

    switch(updataType)
    {
        case UPDATA_BYTE01_APP_START:
        {
            if (get_system_status() == HEATTING_STANDARD || get_system_status() == HEATTING_BOOST || get_system_status() == HEATTING_CLEAN) {
                set_system_status(IDLE);
            }
            retFlag = updata_ctr_app(pBuf);
            set_update_ui_timer_cnt(5000);
            break;
        }
        case UPDATA_BYTE01_BOOT_START:
        {
            retFlag = updata_ctr_boot(pBuf);
            set_update_ui_timer_cnt(5000);
            break;
        }
        case UPDATA_BYTE01_UI_START:
        {
            retFlag = updata_ctr_ui(pBuf);
            set_update_ui_timer_cnt(5000);
            break;
        }
        case UPDATA_BYTE01_INI_WRITE_START:
        {
#ifdef INI_CONFIG_DEFAULT_EN
            retFlag = false;
#else
            retFlag = updata_ctr_write_ini(pBuf);
#endif
            break;
        }

        case UPDATA_BYTE01_INI_READ_START:
        {
            
            set_update_ui_timer_cnt(3000);
            retFlag = updata_ctr_read_ini();
           // 读取配置区key-val 总长度计算总包数
            uint32 packNum = 0;
            memcpy(pBuf->pData,(uint8_t *)&gs_iapInfo.packet_total, 4);
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            pBuf->cmd = PROC_RESPONE_DATA;
            pBuf->dataLen.low = 0x04;
            pBuf->dataLen.high = 0;
            return cmd_base_reply(pBuf,pBuf->cmd);
        }
        
        case UPDATA_BYTE01_APP_FLAG:
        {
            retFlag = updata_app_flag(pBuf);
            break;
        }
        case UPDATA_BYTE01_BOOT_FLAG:
        {
            retFlag = updata_boot_flag(pBuf);
            break;
        }
        case UPDATA_BYTE01_UI_FLAG:
        {
            retFlag = updata_ui_flag(pBuf);
            break;
        }

        case UPDATA_BYTE01_INI_WRITE_FLAG:
        {
#ifdef INI_CONFIG_DEFAULT_EN
            retFlag = false;
#else
            retFlag = updata_ini_flag(pBuf);
#endif
            break;
        }
        case UPDATA_BYTE01_INI_READ_FLAG:
        {
            retFlag = updata_ini_flag(pBuf);
            break;
        }
        default:
            break;

    }
    
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);

}

//app 数据帧升级
static uint16_t cmd_tranfer_app_data(ProtocolBase_t *pBuf)
{
    bool retFlag = true;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint32_t tempIndex = COMB_4BYTE(pBuf->pData[3],pBuf->pData[2],pBuf->pData[1],pBuf->pData[0]);
    ptIoDev usbDev = io_dev_get_dev(DEV_USB);
    char ver[64] = {0};
    set_update_ui_timer_cnt(5000);

    if((gs_iapInfo.startFlag==false)||(tempIndex !=gs_iapInfo.frameIndex))
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    gs_iapInfo.frameIndex++;

    /*wanggankun@2024-9-5, modify, 这里判断，是否是外部FLASH。*/
    if (0)
    {/*内部FLASH*/
        uint8_t *pIapBuf = get_iap_combine_buf();
        
        memcpy(&pIapBuf[gs_iapInfo.offsetAddr % IAP_COMBINE_BUF_LEN], &pBuf->pData[4], tempLen - 4); // tempLen包含了tempIndex，因此需减4
        
        gs_iapInfo.offsetAddr += tempLen - 4; // tempLen包含了tempIndex，因此需减4

        // 需增加地址溢出判断， 防止错误覆盖其他地址
        if (gs_iapInfo.offsetAddr > FLASH_AREA_IMG_1_SECONDARY_SIZE)
        {
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
        }

        if(gs_iapInfo.offsetAddr % MCU_FLASH_ROW_SIZE == 0)
        {
            int flashStatus = flash_program_sector(pIapBuf, MCU_FLASH_START_ADDR + FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / MCU_FLASH_ROW_SIZE - 1) * MCU_FLASH_ROW_SIZE);
            if (flashStatus == 0) {
                retFlag = false;
            }
        }
        
        if(gs_iapInfo.frameIndex ==gs_iapInfo.packet_total) // 如果已经传完最后一包
        {
            if (gs_iapInfo.offsetAddr % MCU_FLASH_ROW_SIZE != 0) { // 如果最后一包凑不成整行，处理最后一行存储
                int flashStatus = flash_program_sector(pIapBuf, MCU_FLASH_START_ADDR + FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / MCU_FLASH_ROW_SIZE) * MCU_FLASH_ROW_SIZE); // 注意这里不能再减一，因为不是整除
                if (flashStatus == 0) {
                    retFlag = false;
                }
            }
            gs_iapInfo.frameIndex = 0;
            gs_iapInfo.packet_total = 0;
            gs_iapInfo.startFlag = false;
            gs_iapInfo.offsetAddr = 0;
            
        }

        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
        if(retFlag==true)
        {
            pBuf->cmd = PROC_RESPONE_ACK;
        }else{
            pBuf->cmd = PROC_RESPONE_NCK;
        }
        return cmd_base_reply(pBuf,pBuf->cmd);
    }
    else
    {/*外部FLASH*/
        uint8_t *pIapBuf = get_iap_4k_buf();
        

//        sm_log(SM_LOG_ERR, "offsetAddr %d, index %d, total %d!!!\r\n", gs_iapInfo.offsetAddr, gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
        memcpy(&pIapBuf[gs_iapInfo.offsetAddr % IAP_4K_BUF_LEN], &pBuf->pData[4], tempLen - 4); // tempLen包含了tempIndex，因此需减4
        
        if (get_rdp_on_mode()== 1 && gs_iapInfo.offsetAddr == 0) { // 第一个包 判断版本后，rdpon 后不能降级
            uint16_t revision =COMB_2BYTE(pIapBuf[23], pIapBuf[22]);
            #if 0
            if ((pIapBuf[20] < APP_VERSION_MAJOR || pIapBuf[21] < APP_VERSION_MINOR || revision < APP_VERSION_BUILD) ||
                (pIapBuf[20] == 5 && pIapBuf[21] == 1 && revision == 0)) { // 还没加限制降级功能前该版本号为5 1 0
                sm_log(SM_LOG_ERR, " Firmware can not downgrade!\r\n");
                sys_task_security_ms_delay(100, TASK_USB); //
                pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
                return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
            }
            #endif
            /**
             * refuse version5.1.0 to OTA upgrade
            */
            if ((pIapBuf[20] == 5) && (pIapBuf[21] == 1) && (revision == 0))
            {
                sm_log(SM_LOG_ERR, " Firmware can not downgrade!\r\n");
                sys_task_security_ms_delay(100, TASK_USB); //
                pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
                return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
            }
            /** version control */
            if (pIapBuf[20] > APP_VERSION_MAJOR)
            {
                //;
            }
            else if (pIapBuf[20] == APP_VERSION_MAJOR)
            {
                if (pIapBuf[21] > APP_VERSION_MINOR)/** miss "else" ,priority should MAJOR>MINOR>BUILD  */
                {
                    //;
                }
                else if (pIapBuf[21] == APP_VERSION_MINOR)/** miss "else" ,priority should MAJOR>MINOR>BUILD  */
                {
                    if (revision < APP_VERSION_BUILD)
                    {
                        sm_log(SM_LOG_ERR, " Firmware can not downgrade!\r\n");
                        sys_task_security_ms_delay(100, TASK_USB); //
                        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
                        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
                    }
                }
                else
                {
                    sm_log(SM_LOG_ERR, " Firmware can not downgrade!\r\n");
                    sys_task_security_ms_delay(100, TASK_USB); //
                    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
                    return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
                }
            }
            else
            {
                sm_log(SM_LOG_ERR, " Firmware can not downgrade!\r\n");
                sys_task_security_ms_delay(100, TASK_USB); //
                pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
                return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
            }
        }

        gs_iapInfo.offsetAddr += tempLen - 4; // tempLen包含了tempIndex，因此需减4
//        sm_log(SM_LOG_ERR, "offsetAddr %d, index %d, total %d!!!\r\n", gs_iapInfo.offsetAddr, gs_iapInfo.frameIndex, gs_iapInfo.packet_total);

        // 需增加地址溢出判断， 防止错误覆盖其他地址
        if (gs_iapInfo.offsetAddr > FLASH_AREA_IMG_1_SECONDARY_SIZE) 
        {
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
        }

        if(gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE == 0)
        {
//        	sm_log(SM_LOG_ERR, "program addr %d, index %d, total %d!!!\r\n", FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE - 1) * EXTERN_FLASH_PAGE_SIZE, gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
        	extern_flash_program_page(pIapBuf, FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE - 1) * EXTERN_FLASH_PAGE_SIZE);
        }

        if(gs_iapInfo.frameIndex ==gs_iapInfo.packet_total) // 如果已经传完最后一包
        {
            if (gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE != 0) { // 如果最后一包凑不成整行，处理最后一行存储
//            	sm_log(SM_LOG_ERR, "program addr %d, index %d, total %d!!!\r\n", FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE - 1) * EXTERN_FLASH_PAGE_SIZE, gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
            	extern_flash_program_page(pIapBuf, FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE) * EXTERN_FLASH_PAGE_SIZE); // 注意这里不能再减一，因为不是整除
            }
            gs_iapInfo.frameIndex = 0;
            gs_iapInfo.packet_total = 0;
            gs_iapInfo.startFlag = false;
            gs_iapInfo.offsetAddr = 0;            
        }
        
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
        if(retFlag==true)
        {
            pBuf->cmd = PROC_RESPONE_ACK;
        }
        else
        {
            pBuf->cmd = PROC_RESPONE_NCK;
        }

        return cmd_base_reply(pBuf,pBuf->cmd);
    }
}


//boot 数据帧升级
static uint16_t cmd_tranfer_boot_data(ProtocolBase_t *pBuf)
{
    bool retFlag = true;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint16_t tempIndex = COMB_4BYTE(pBuf->pData[3],pBuf->pData[2],pBuf->pData[1],pBuf->pData[0]);
    
    set_update_ui_timer_cnt(5000);

    if((gs_iapInfo.startFlag==false)||(tempIndex !=gs_iapInfo.frameIndex))
    {

        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    gs_iapInfo.frameIndex++;

    /*wanggankun@2024-9-5, modify, 这里判断，是否是外部FLASH。*/
	if (0)
	{/*内部FLASH*/
		uint8_t *pIapBuf = get_iap_combine_buf();

		memcpy(&pIapBuf[gs_iapInfo.offsetAddr % IAP_COMBINE_BUF_LEN], &pBuf->pData[4], tempLen - 4); // tempLen包含了tempIndex，因此需减4

		gs_iapInfo.offsetAddr += tempLen - 4; // tempLen包含了tempIndex，因此需减4

		// 需增加地址溢出判断， 防止错误覆盖其他地址
		if (gs_iapInfo.offsetAddr > FLASH_AREA_BOOTLOADER_SIZE) { // 大于mcu-boot空间，出错
			pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
			return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
		}
		if(gs_iapInfo.offsetAddr % MCU_FLASH_ROW_SIZE == 0)
		{
			/*写升级数据到备份FALSH*/
			//program 512byte bin here
			int flashStatus = flash_program_sector(pIapBuf, MCU_FLASH_START_ADDR + FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / MCU_FLASH_ROW_SIZE - 1) * MCU_FLASH_ROW_SIZE);
			if (flashStatus == 0) {
				retFlag = false;
			}
			// sm_log(SM_LOG_INFO, "addr %d, index %d, total %d!!!\r\n", (gs_iapInfo.offsetAddr / MCU_FLASH_ROW_SIZE - 1), gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
		}

		if(gs_iapInfo.frameIndex ==gs_iapInfo.packet_total) // 如果已经传完最后一包
		{
			if (gs_iapInfo.offsetAddr % MCU_FLASH_ROW_SIZE != 0) { // 如果最后一包凑不成整行，处理最后一行存储
				/*写升级数据到备份FALSH*/
				//program <512byte bin here
				int flashStatus = flash_program_sector(pIapBuf, MCU_FLASH_START_ADDR + FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / MCU_FLASH_ROW_SIZE) * MCU_FLASH_ROW_SIZE); // 注意这里不能再减一，因为不是整除
				if (flashStatus == 0) {
					retFlag = false;
				}
				//sm_log(SM_LOG_INFO, "last_addr %d, index %d, total %d!!!\r\n", (gs_iapInfo.offsetAddr / MCU_FLASH_ROW_SIZE), gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
			}
		}

		pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
		pBuf->dataLen.low = 0;
		pBuf->dataLen.high = 0;
		if(retFlag==true)
		{
			pBuf->cmd = PROC_RESPONE_ACK;
		}else{
			pBuf->cmd = PROC_RESPONE_NCK;
		}
		return cmd_base_reply(pBuf,pBuf->cmd);
	}
	else
	{/*外部FLASH*/
		uint8_t *pIapBuf = get_iap_4k_buf();

		memcpy(&pIapBuf[gs_iapInfo.offsetAddr % IAP_4K_BUF_LEN], &pBuf->pData[4], tempLen - 4); // tempLen包含了tempIndex，因此需减4

		gs_iapInfo.offsetAddr += tempLen - 4; // tempLen包含了tempIndex，因此需减4

		// 需增加地址溢出判断， 防止错误覆盖其他地址
		if (gs_iapInfo.offsetAddr > FLASH_AREA_BOOTLOADER_SIZE) { // 大于mcu-boot空间，出错
			pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
			return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
		}
		if(gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE == 0)
		{
			/*写升级数据到备份FALSH*/
			//program 4k byte bin here
			int flashStatus = extern_flash_program_page(pIapBuf, FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE - 1) * EXTERN_FLASH_PAGE_SIZE);
			if (flashStatus == 0) {
				retFlag = false;
			}
			// sm_log(SM_LOG_INFO, "addr %d, index %d, total %d!!!\r\n", (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE - 1), gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
		}

		if(gs_iapInfo.frameIndex ==gs_iapInfo.packet_total) // 如果已经传完最后一包
		{
			if (gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE != 0) { // 如果最后一包凑不成整行，处理最后一行存储
				/*写升级数据到备份FALSH*/
				//program < 4k byte bin here
				int flashStatus = extern_flash_program_page(pIapBuf, FLASH_AREA_IMG_1_SECONDARY_START + (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE) * EXTERN_FLASH_PAGE_SIZE); // 注意这里不能再减一，因为不是整除
				if (flashStatus == 0) {
					retFlag = false;
				}
				//sm_log(SM_LOG_INFO, "last_addr %d, index %d, total %d!!!\r\n", (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE), gs_iapInfo.frameIndex, gs_iapInfo.packet_total);
			}
		}

		pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
		pBuf->dataLen.low = 0;
		pBuf->dataLen.high = 0;
		if(retFlag==true)
		{
			pBuf->cmd = PROC_RESPONE_ACK;
		}else{
			pBuf->cmd = PROC_RESPONE_NCK;
		}
		return cmd_base_reply(pBuf,pBuf->cmd);
	}

}



//ui 数据帧升级
static uint16_t cmd_tranfer_ui_data(ProtocolBase_t *pBuf)
{
    bool retFlag = true;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint16_t tempIndex = COMB_4BYTE(pBuf->pData[3],pBuf->pData[2],pBuf->pData[1],pBuf->pData[0]);
//    char invaildVer[] = "Invalid\n";
    set_update_ui_timer_cnt(5000);
    ImageHeaderInfo_t* imageHeaderInfo;
    imageHeaderInfo = get_image_header_info_handle();

    if((gs_iapInfo.startFlag==false)||(tempIndex !=gs_iapInfo.frameIndex))
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    gs_iapInfo.frameIndex++;

    // 需增加地址溢出判断， 防止错误覆盖其他地址
    if ((gs_iapInfo.offsetAddr + tempLen - 4) > EXTREN_FLASH_TOTAL_SIZE) {
        
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    while (UI_NONE != get_current_ui_detail_status()) { //等待显示结束
        set_update_ui_timer_cnt(5000);
        sys_task_security_ms_delay(10, TASK_USB);
    }
    uint8_t *pIapBuf = get_ram_second_gui(); // 升级图片时不要调用UI显示需要调用互斥

    memcpy(&pIapBuf[gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE], &pBuf->pData[4], tempLen - 4); // tempLen包含了tempIndex，因此需减4

    gs_iapInfo.offsetAddr += tempLen - 4; // tempLen包含了tempIndex，因此需减4

    if (gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE == 0) { // 整4K或者最后一包
//        if ((gs_iapInfo.offsetAddr-glb_ui_address_offset) == EXTERN_FLASH_PAGE_SIZE) { // 第一个包包含了版本号， 把版本号先保存下来，并写进一个无效的版本
//            memcpy(gs_ui_version_save_buf, pIapBuf, 32);
//            memcpy(pIapBuf, invaildVer, strlen(invaildVer));
//            memcpy(imageHeaderInfo->ver, invaildVer, strlen(invaildVer));
//        }

        extern_flash_program_page(pIapBuf, (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE - 1) * EXTERN_FLASH_PAGE_SIZE);
    }

    if(gs_iapInfo.frameIndex ==gs_iapInfo.packet_total) // 如果已经传完最后一包
    {
        if (gs_iapInfo.offsetAddr % EXTERN_FLASH_PAGE_SIZE != 0) { // 不足一包
            extern_flash_program_page(pIapBuf, (gs_iapInfo.offsetAddr / EXTERN_FLASH_PAGE_SIZE) * EXTERN_FLASH_PAGE_SIZE); // 注意这里不能再减一，因为不是整除
        }
        
        // 再把第一包读出来
 //       extern_flash_read_page(pIapBuf, 0);
        // 把版本放回去
//        memcpy(pIapBuf, gs_ui_version_save_buf, 32);
//        extern_flash_program_page(pIapBuf, 0);
        image_header_info_get(); // 更新header 数据 
        gs_iapInfo.frameIndex = 0;
        gs_iapInfo.packet_total = 0;
        gs_iapInfo.startFlag = false;
        gs_iapInfo.offsetAddr = 0;
    }

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }
    else
    {
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);

}

static uint16_t cmd_tranfer_read_ini_data(ProtocolBase_t *pBuf)
{
    uint32_t tempIndex = COMB_4BYTE(pBuf->pData[3],pBuf->pData[2],pBuf->pData[1],pBuf->pData[0]);
	uint16_t rLen = 0;
    IniInfo_t *iniInfo = get_iniInfo_handle();
    uint32_t totalLen = iniInfo->keyTotalLen + iniInfo->valTotalLen;
    
    set_update_ui_timer_cnt(3000);
    if(tempIndex >= gs_iapInfo.packet_total)
    {
    
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    uint32_t addr = tempIndex * 256; // 260
    if (totalLen - addr >= 256) {
        rLen = 256;
    } else {
        rLen = totalLen - addr;
    }
    memcpy(&pBuf->pData[4], &g_tImageSecondRam[addr], rLen);
    rLen += 4; // 把帧序号算进数据长度
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}

static uint16_t cmd_tranfer_write_ini_data(ProtocolBase_t *pBuf)
{
    bool retFlag = true;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint16_t tempIndex = COMB_4BYTE(pBuf->pData[3],pBuf->pData[2],pBuf->pData[1],pBuf->pData[0]);
    uint32_t addrOffset = 1024 * 10;
    int16_t *pVal = get_ini_val_info_handle();
    int j = 0;
    uint32_t addr = 0;
#ifdef INI_CONFIG_DEFAULT_EN
    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
#endif

    if((gs_iapInfo.startFlag==false)||(tempIndex !=gs_iapInfo.frameIndex))
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    gs_iapInfo.frameIndex++;
    set_update_ui_timer_cnt(3000);

    // 需增加地址溢出判断， 防止错误覆盖其他地址
//    if ((gs_iapInfo.offsetAddr + tempLen - 4) > EXTREN_FLASH_TOTAL_SIZE) {
//        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
//    }

//    uint8_t *pIapBuf = get_iap_combine_buf();

    memcpy(&g_tImageSecondRam[addrOffset  + gs_iapInfo.offsetAddr], &pBuf->pData[4], tempLen - 4); // tempLen包含了tempIndex，因此需减4

    gs_iapInfo.offsetAddr += tempLen - 4; // tempLen包含了tempIndex，因此需减4

    if(gs_iapInfo.frameIndex == gs_iapInfo.packet_total) // 如果已经传完最后一包, 保存eerom
    {
        j = 0;
        addr = 0;
        // 拆分ini存储
        // 拷贝版本字符串
        memcpy(&g_tImageSecondRam[addr], &g_tImageSecondRam[j + addrOffset], (strlen(&g_tImageSecondRam[j + addrOffset]) + 1));
        addr = addr + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
        j = j + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
        // 拷贝长度字符串
        memcpy(&g_tImageSecondRam[addr], &g_tImageSecondRam[j + addrOffset], (strlen(&g_tImageSecondRam[j + addrOffset]) + 1));
        addr = addr + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
        j = j + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);

        for (int i = 0; i < INI_VAL_TBL_LEN; i++) {
            memcpy(&g_tImageSecondRam[addr], &g_tImageSecondRam[j + addrOffset], (strlen(&g_tImageSecondRam[j + addrOffset]) + 1)); // key
            addr = addr + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
            j = j + (strlen(&g_tImageSecondRam[j + addrOffset]) + 1);
            memcpy((uint8_t *)&pVal[i],  &g_tImageSecondRam[j + addrOffset], 2);
            j += 2;
        }
        
#ifndef INI_CONFIG_DEFAULT_EN // 如果没有定义则需要保存到eeprom
        e2pTemp.addr = E2P_ADDR_OF_INI_KEY_STR;
        e2pTemp.pDate = g_tImageSecondRam; //
        e2pTemp.size = E2P_SIZE_OF_INI_KEY_STR;
        e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));

        e2pTemp.addr = E2P_ADDR_OF_INI_VAL_TBL;
        e2pTemp.pDate = (uint8_t *)pVal; //
        e2pTemp.size = E2P_SIZE_OF_INI_VAL_TBL;
        e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));
#endif
        gs_iapInfo.frameIndex = 0;
        gs_iapInfo.packet_total = 0;
        gs_iapInfo.startFlag = false;
        gs_iapInfo.offsetAddr = 0;
    }

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);

}

uint8_t g_send_pclog_type = 0;
void set_send_pc_log_type(uint8_t logType)
{
    g_send_pclog_type = logType;
}

uint8_t get_send_pc_log_type(void)
{
    return g_send_pclog_type;
}
/*---------------------------------------------------------------------------*/
//获取Log条目       数据域：0~N Byte
/*
0x00 -> 所有条目
0x01 -> Heat条目
0x02 -> Charge 条目
0x03 -> Idle 条目

*/

//获取Log条目
static uint16_t cmd_get_log_item(ProtocolBase_t *pBuf)
{
	ComtLogItem_t LogItem = {0};
	uint16_t table_len = 0;
	int i = 0;
    uint8_t state = pBuf->pData[0];//调试用 0
	char table[MAX_ITEM * MAX_LENGTH];

	const char* itemData[MAX_ITEM] = {
		"ticks,",
		"item,",
		"sp,",
		"pv,",
		"batv,",
		
		"bata,",
		"resoc,",
		"dciv,",
		"dcov,",
		"dcoa,",
		
		"dcow,",
		"htres,",
		"soc,",
		"batt,",
		"coldt,",
		
		"usbt,",
		"usbv,",
		"usba,",
		"power_J,",
		"chg_state,",
		
		"partab,",
		"ble"
	};

    const char* itemFuelData[MAX_ITEM] = {
        "ticks,",
        "item,",
        "batv,",
        "bati,",
        "batt,",
        
        "soc,",
        "truesoc,",
        "truerm,",
        "fltrm,",
        "truefcc,",
        
        "fltfcc,",
        "passchg,",
        "qmax,",
        "dod0,",
        "dodateoc,",
        
        "paredod,",
        "ctlsta,",
        "flags,",
        "na,",
        "na,",
        
        "na,",
        "na"
    };

    const char* itemEventData[EVENT_ITEM] = {
        "ticks,",
        "Count,",
        "Timestamp,",
        "EventCode,",
        "EventData"
    };

    const char* itemSessionData[SESSION_ITEM] = {
        "ticks,",
        "Count,",
        "Timestamp,",
        "Duration,",
        "ExitCode,",
        "Mode,",
        "HeatProf,",
        "Z1MaxTemp,",
        "BATMaxTemp,",
        "Trusted"
    };

    const char* itemLifeCycleData[LIFECYCLE_ITEM] = {
        "ticks,",
        "MinTemp,",
        "MaxTemp,",
        "MinVol,",
        "MaxVol,",
        "MaxChgCur,",
        "MaxDisChgCur,",
        "TolChgTime,",
        "FChgCnt,",
        "CMP_S,",
        "INCMP_S,",
        "MinZ1_T,",
        "MaxZ1_T"
    };

    uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint8_t item_length=MAX_ITEM;

    if ((pBuf->pData[1] == LOG_COMMON) || (pBuf->pData[1] == LOG_FUELGAGUE))
    {
        item_length = MAX_ITEM;
    }
    else if (pBuf->pData[1] == LOG_EVENT)
    {
        item_length = EVENT_ITEM;//sizeof(itemEventData);
    }
    else if (pBuf->pData[1] == LOG_SESSION)
    {
        item_length = SESSION_ITEM;//sizeof(itemSessionData);
    }
    else if (pBuf->pData[1] == LOG_LIFECYCLE)
    {
        item_length = LIFECYCLE_ITEM;//sizeof(itemLifeCycleData);
    } else {
        if (tempLen == 2) { // 没定义的返回NCK
            pBuf->cmd = PROC_RESPONE_NCK;
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            pBuf->dataLen.low = LBYTE(table_len);
            pBuf->dataLen.high = HBYTE(table_len);
            return cmd_base_reply(pBuf,pBuf->cmd);
        }
    }

	if(state == 0)
	{		
		//初始化条目数据和计算条目长度
		for(i = 0; i < item_length; i++)
		{   
            if (((tempLen == 2 && pBuf->pData[1] == LOG_COMMON)) || (tempLen == 1))//modified by vincent.he, 20250217
            {
                set_send_pc_log_type(LOG_COMMON);
    			strcpy(LogItem.data[i], itemData[i]);
            }
            else if (tempLen == 2 && pBuf->pData[1] == LOG_FUELGAGUE) 
            { 
                set_send_pc_log_type(LOG_FUELGAGUE);
    			strcpy(LogItem.data[i], itemFuelData[i]);
            } 
            else if (tempLen == 2 && pBuf->pData[1] == LOG_EVENT) 
            { 
                set_send_pc_log_type(LOG_EVENT);
    			strcpy(LogItem.data[i], itemEventData[i]);
            } 
            else if (tempLen == 2 && pBuf->pData[1] == LOG_SESSION) 
            { 
                set_send_pc_log_type(LOG_SESSION);
    			strcpy(LogItem.data[i], itemSessionData[i]);
            } 
            else if (tempLen == 2 && pBuf->pData[1] == LOG_LIFECYCLE) 
            { 
                set_send_pc_log_type(LOG_LIFECYCLE);
    			strcpy(LogItem.data[i], itemLifeCycleData[i]);
            } 
			LogItem.length[i] = strlen(LogItem.data[i]);
		}
		//拷进总数组
		for(i = 0; i < item_length; i++)
		{
			memcpy(table + table_len, LogItem.data[i], LogItem.length[i]);
			table_len += LogItem.length[i];   
		}

        memcpy(pBuf->pData,(uint8_t *)&table, table_len);
        pBuf->cmd = PROC_RESPONE_DATA;
	}
	else
	{
        pBuf->cmd = PROC_RESPONE_NCK;
	}

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = LBYTE(table_len);
    pBuf->dataLen.high = HBYTE(table_len);
	return cmd_base_reply(pBuf,pBuf->cmd);
}




/*---------------------------------------------------------------------------*/
//Log输出控制       数据域：0~N
/*
0x00 -> 关闭log输出
20~10000 -> log输出频率，单位ms
其它值无效
*/


//Log输出控制       数据域：(组数yte
__WEAK bool procotol_log_control(uint16_t optCode){return false;}//log

__WEAK bool procotol_errcode_control(void){return false;}//errcode

static uint16_t cmd_log_control(ProtocolBase_t *pBuf)
{
    uint16_t optCode = COMB_2BYTE(pBuf->pData[1],pBuf->pData[0]);

    if(((optCode < 20u) || (optCode > 10000u))&&(optCode!=0))
    {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    if (get_rdp_on_mode_no_save_flash() == 1 && optCode > 0) {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    procotol_log_control(optCode);

    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
    return cmd_base_reply(pBuf,pBuf->cmd);
}

static uint16_t cmd_cli_control(ProtocolBase_t *pBuf)
{
    if (pBuf->pData[0] == 1) { // 返回错误码日志
        procotol_errcode_control();
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        pBuf->cmd = PROC_RESPONE_ACK;
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
        return cmd_base_reply(pBuf,pBuf->cmd);
    }  else if (pBuf->pData[0] == 2) { // 清空错误码计数,需要同时清空当前的错误码
        errcode_clr_to_flash();
        delete_all_error_even();
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
    } else {
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
}


//产品配置
__WEAK bool procotol_func_setup(uint8_t* pBuf, uint16_t opType){(void *)pBuf; return false;}

static uint16_t cmd_get_func(ProtocolBase_t *pBuf)
{
	procotol_func_setup(pBuf->pData, 0);
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = 8;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,pBuf->cmd);
}

static uint16_t cmd_set_func(ProtocolBase_t *pBuf)
{
	procotol_func_setup(pBuf->pData, 1);
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}

__WEAK bool procotol_sys_info_get(uint8_t* pBuf){(void *)pBuf; return false;}
static uint16_t cmd_get_sys_info(ProtocolBase_t *pBuf)
{
    procotol_sys_info_get(pBuf->pData);
    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = 8;
    pBuf->dataLen.high = 0;

    return cmd_base_reply(pBuf,pBuf->cmd);
}

__WEAK bool procotol_rdp_on_set(uint8_t* pBuf){(void *)pBuf; return false;}
static uint16_t cmd_set_rdp_on(ProtocolBase_t *pBuf)
{
    set_rdp_on_mode(1);
    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

    return cmd_base_reply(pBuf,pBuf->cmd);
}

static uint16_t cmd_set_get_log_level(ProtocolBase_t *pBuf)
{
    if (pBuf->pData[0] == 0xA5) { // 返回日志等级
        pBuf->cmd = PROC_RESPONE_DATA;
        pBuf->pData[0] = get_log_level();
        pBuf->dataLen.low = 1;
        pBuf->dataLen.high = 0;
    } else if (pBuf->pData[0] < SM_LOG_MAX) { // 设置日志等级
        pBuf->cmd = PROC_RESPONE_ACK;
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
        set_log_level(pBuf->pData[0]);
    } else {
        pBuf->cmd = PROC_RESPONE_NCK;
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
    }
    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);

    return cmd_base_reply(pBuf,pBuf->cmd);
}

static uint16_t cmd_get_clr_session(ProtocolBase_t *pBuf)
{
    if (pBuf->pData[0] == 0xA5) { // 清除烟支数
        clr_sessions_total();
        pBuf->cmd = PROC_RESPONE_ACK;
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
        // 清除总加热时间
    } else if (pBuf->pData[0] == 0) { // 查询烟支数
        uint16_t sessions = (uint16_t)get_sessions_total();
        pBuf->cmd = PROC_RESPONE_DATA;
        // 返回当前eol阶段
#ifdef INI_CONFIG_DEFAULT_EN
        if (sessions >= cIniValTbl[STEP3_SESSION]) {
            pBuf->pData[0] = 3;
        } else if (sessions >= cIniValTbl[STEP2_SESSION]) {
            pBuf->pData[0] = 2;
        } else {
            pBuf->pData[0] = 1;
        }
#else
if (sessions >= g_fdb_b_u.fdb_b1_t.iniValTbl[STEP3_SESSION]) {
    pBuf->pData[0] = 3;
} else if (sessions >= g_fdb_b_u.fdb_b1_t.iniValTbl[STEP2_SESSION]) {
    pBuf->pData[0] = 2;
} else {
    pBuf->pData[0] = 1;
}
#endif
        // 获取烟支数
        pBuf->pData[1] = LBYTE(sessions);
        pBuf->pData[2] = HBYTE(sessions);
        pBuf->dataLen.low = 3;
        pBuf->dataLen.high = 0;
    } else {
        pBuf->cmd = PROC_RESPONE_NCK;
        pBuf->dataLen.low = 0;
        pBuf->dataLen.high = 0;
    }
    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);

    return cmd_base_reply(pBuf,pBuf->cmd);
}

static uint16_t cmd_factory_reset(ProtocolBase_t *pBuf)
{
	
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	bt_adapter_factory_reset_all();
#else
    // 1. 恢复默认加热曲线， sessions
    heat_param_info_init();
    // 2. 清除错误码记录以及当前错误
    errcode_clr_to_flash();
    delete_all_error_even();
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    set_user_reset_status(true);
    pBuf->cmd = PROC_RESPONE_ACK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
    pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    return cmd_base_reply(pBuf,pBuf->cmd);
}
//获取断针保护阈值参数配置	数据域：返回 (2 + 4)  * 2字节
static uint16_t cmd_get_heate_pin_broke_threshold(ProtocolBase_t *pBuf)
{
	uint16_t rLen = 0;
    rLen = 12;
    float base_heater_bk_threshold_k = DEF_THRESHOLD_BASE_TK; //
    int16_t base_heater_bk_threshold_b = 0; //b
    float boost_heater_bk_threshold_k = DEF_THRESHOLD_BOOST_TK; //
    int16_t boost_heater_bk_threshold_b = 0; //b

    memcpy(&pBuf->pData[0],(uint8_t *)&(base_heater_bk_threshold_b),2);
    memcpy(&pBuf->pData[2],(uint8_t *)&(base_heater_bk_threshold_k),4);
    
    memcpy(&pBuf->pData[6],(uint8_t *)&(boost_heater_bk_threshold_b),2);
    memcpy(&pBuf->pData[8],(uint8_t *)&(boost_heater_bk_threshold_k),4);
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_DATA;
    pBuf->dataLen.low = LBYTE(rLen);
    pBuf->dataLen.high = HBYTE(rLen);
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//设置断针保护阈值参数配置	数据域：(2 + 4)  * 2
static uint16_t cmd_set_heate_pin_broke_threshold(ProtocolBase_t *pBuf)
{
 
    // cmd req
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->cmd = PROC_RESPONE_NCK;
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
	return cmd_base_reply(pBuf,pBuf->cmd);
}

/*---------------------------------------------------------------------------*/
static uint16_t cmd_set_color(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    if(tempLen != 1)
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }

	set_device_color(pBuf->pData[0]);

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
}

static uint16_t cmd_get_color(ProtocolBase_t *pBuf)
{
	pBuf->pData[0] = get_device_color();

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->dataLen.low = LBYTE(1);
	pBuf->dataLen.high = HBYTE(1);

	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}

/*---------------------------------------------------------------------------*/
static uint16_t cmd_set_rtc(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    if(tempLen != 4)
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
// 大小端格式转换
	u32_endian_swap((uint32_t *)(pBuf->pData));

	bt_adapter_time_set(pBuf->pData, 4);

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
}

static uint16_t cmd_get_rtc(ProtocolBase_t *pBuf)
{
	bt_adapter_time_get((session_service_time_char_write_read_t*)(pBuf->pData));
// 大小端格式转换
	u32_endian_swap((uint32_t *)(pBuf->pData));

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->dataLen.low = LBYTE(4);
	pBuf->dataLen.high = HBYTE(4);

	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}

static uint16_t cmd_get_heatingProfileSel(ProtocolBase_t *pBuf)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfileSel_t * heatProfileSel = (heatProfileSel_t *)pBleData->heatProfileSel;

	pBuf->pData[0] = heatProfileSel->index_base_used;
//	pBuf->pData[1] = heatProfileSel->index_boost_used;
	pBuf->pData[1] = 0x10;//heatProfileSel->index_boost_used;	固定为0x10

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->dataLen.low = LBYTE(2);
	pBuf->dataLen.high = HBYTE(2);

	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}

static uint16_t cmd_set_heatingProfileSel(ProtocolBase_t *pBuf)
{
	uint32_t res;
	res = bt_adapter_heating_prof_sel_index_set(pBuf->pData[0], pBuf->pData[1]);

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	if (WICED_BT_GATT_SUCCESS == res)
	{
		return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
	}
	else
	{
		return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}
}

extern uint8_t app_bt_adv_conn_state;
extern bool app_bt_gatt_is_connected(void);
/*---------------------------------------------------------------------------*/
static uint16_t cmd_get_BleStatus(ProtocolBase_t *pBuf)
{
	uint8_t sta;
	
    sm_log(SM_LOG_DEBUG, "app_bt_adv_conn_state:   %x\r\n", app_bt_adv_conn_state);

	pBuf->pData[0] = app_bt_adv_conn_state;

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->dataLen.low = LBYTE(1);
	pBuf->dataLen.high = HBYTE(1);

	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}


extern wiced_bt_device_address_t cy_bt_remote_address;
static uint16_t cmd_get_BlePairList(ProtocolBase_t *pBuf)
{
    uint8_t isExisted = 0;
    for (int i = 0; i < 6; i++)
    {
        if (cy_bt_remote_address[i] != 0) 
        {
            isExisted = 1;
            break;
        }
    }
    
    sm_log(SM_LOG_DEBUG, "cy_bt_remote_address: %02X:%02X:%02X:%02X:%02X:%02X, isExisted_flag: %d\n", 
                                                cy_bt_remote_address[0],
                                                cy_bt_remote_address[1],
                                                cy_bt_remote_address[2],
                                                cy_bt_remote_address[3],
                                                cy_bt_remote_address[4],
                                                cy_bt_remote_address[5],
                                                isExisted);
    /* 
        remote address is existed.*/
	if (isExisted)
	{
		memcpy(&pBuf->pData[0], cy_bt_remote_address, 6);
		pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
		pBuf->dataLen.low = LBYTE(6);
		pBuf->dataLen.high = HBYTE(6);

		return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
	}
	else
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}	
}

extern bool app_bt_gatt_is_connected(void);
/*---------------------------------------------------------------------------*/

static uint16_t cmd_set_pair(ProtocolBase_t *pBuf)
{
	uint8_t cmd = pBuf->pData[0];

	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    if(tempLen != 1 || cmd > 1)
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }

	int cur_sysSta = get_system_status();
	int cur_subSysSta = get_subSystem_status();

    if ((HEATTING_TEST == cur_sysSta)||(HEATTING_STANDARD == cur_sysSta)||
		(HEATTING_BOOST == cur_sysSta)||(HEATTING_CLEAN == cur_sysSta)||
		(HEAT_MODE_TEST_VOLTAGE == cur_sysSta)||(HEAT_MODE_TEST_POWER == cur_sysSta)||(HEAT_MODE_TEST_TEMP == cur_sysSta))
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);		// 加热状态不响应
    }

	if (cmd)
	{
		if ((cur_subSysSta != SUBSTATUS_IDLE)) 
		{
			return cmd_base_reply(pBuf,PROC_RESPONE_NCK);		// 非Idle状态不响应
		}

        if (app_bt_gatt_is_connected())//bug1974582
        {
            /**
             * if connected, dispaly pair success*/
            set_system_external_even(EXT_EVENT_PARI_OK);
			return cmd_base_reply(pBuf,PROC_RESPONE_NCK);		// 非Idle状态不响应
        }
		set_system_external_even(EXT_EVENT_PARING_QRCODE);		
	}
	else
	{
		if ((cur_subSysSta != SUBSTATUS_PARING_QRCODE) && (cur_subSysSta != SUBSTATUS_PARING))
		{
			return cmd_base_reply(pBuf,PROC_RESPONE_NCK);		// 非配对模式，不响应
		}
		set_system_external_even(EXT_EVENT_PARING_CANCLE);		
	}

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
}

static uint16_t cmd_set_dev_lock(ProtocolBase_t *pBuf)
{
	uint8_t cmd = pBuf->pData[0];

	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    if(tempLen != 1 || cmd > 1)
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }

	int cur_sysSta = get_system_status();

    if ((HEATTING_TEST == cur_sysSta)||(HEATTING_STANDARD == cur_sysSta)||
		(HEATTING_BOOST == cur_sysSta)||(HEATTING_CLEAN == cur_sysSta)||
		(HEAT_MODE_TEST_VOLTAGE == cur_sysSta)||(HEAT_MODE_TEST_POWER == cur_sysSta)||(HEAT_MODE_TEST_TEMP == cur_sysSta))
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);		// 加热状态不响应
    }

	if (cmd)
	{
		set_system_external_even(EXT_EVENT_LOCK);
	}
	else
	{
		set_system_external_even(EXT_EVENT_UNLOCK);
	}

	set_lock_mode(cmd);
	set_update_lock_status(1);

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
}

extern cy_rslt_t app_bt_delete_bond_info(void);

static uint16_t cmd_clr_ble_data(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);

    // 清除蓝牙协议栈邦定数据&&RAM key数据
    if ((tempLen != 0)||app_bt_delete_bond_info())
    {
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }

// 清除蓝牙邦定数据部分
//	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
//	memset((uint8_t *)pBleData->ble_bonding_data, 0, sizeof(pBleData->ble_bonding_data));	// 若读出数据无效，全清零
//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
//	set_update_BleData_status(0);

	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;

	return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
}

static uint16_t cmd_get_rdpon_status(ProtocolBase_t *pBuf)
{
    uint32_t rdpon = get_rdp_on_mode();
	pBuf->pData[0] = (uint8_t)rdpon;
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pBuf->dataLen.low = LBYTE(1);
	pBuf->dataLen.high = HBYTE(1);
	return cmd_base_reply(pBuf,PROC_RESPONE_DATA);
}
static uint16_t cmd_get_set_dry_heat_data(ProtocolBase_t *pBuf)
{
    uint16_t rLen = 0;
    uint8_t cmd = pBuf->pData[0];
    uint8_t mode = pBuf->pData[1];
    int16 info_p[4] = {0};
    if(cmd == 0)//0 获取 1 ：设置
    {
        if(mode == 0)//base mode 
        {
            get_ref_tbl(HEATTING_STANDARD,info_p,&pBuf->pData[0]);
            rLen = 8*30;
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            pBuf->cmd = PROC_RESPONE_DATA;
            pBuf->dataLen.low = LBYTE(rLen);
            pBuf->dataLen.high = HBYTE(rLen);
            return cmd_base_reply(pBuf,pBuf->cmd);
        }else if(mode == 1){// boost mode
            get_ref_tbl(HEATTING_BOOST,info_p,&pBuf->pData[0]);
            rLen = 8*30;
            pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            pBuf->cmd = PROC_RESPONE_DATA;
            pBuf->dataLen.low = LBYTE(rLen);
            pBuf->dataLen.high = HBYTE(rLen);
            return cmd_base_reply(pBuf,pBuf->cmd);
        }else{
            return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
        }
    }else{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
}
// 获取/设置 断针阈值数据
static uint16_t cmd_get_set_sp_pin_broke_threshold(ProtocolBase_t *pBuf)
{
    FDB_AREA_F_u  fdb_f_u;
    uint16_t rLen = sizeof(FDB_pin_bk_chird_t);
    uint8_t chirdCmd  = pBuf->pData[0];
    if(chirdCmd == 0)//0 获取 1 ：设置
    {
        app_param_read(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
        memcpy(&pBuf->pData[0],&fdb_f_u.fdb_f_t.pinBreakParam.magicBase,sizeof(FDB_pin_bk_chird_t));
        pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        pBuf->cmd = PROC_RESPONE_DATA;
        pBuf->dataLen.low = LBYTE(rLen);
        pBuf->dataLen.high = HBYTE(rLen);
        return cmd_base_reply(pBuf,pBuf->cmd);
    }else{
        // app_param_read(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));
        // memcpy(&fdb_f_u.fdb_f_t.pinBreakParam.magicBase,&pBuf->pData[0],sizeof(FDB_pin_bk_chird_t));
        // app_param_write(INDEX_F,fdb_f_u.res,sizeof(FDB_AREA_F_u));

        // pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        // pBuf->dataLen.low = 0;
        // pBuf->dataLen.high = 0;
        // return cmd_base_reply(pBuf,PROC_RESPONE_ACK);
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
    }
    return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
}
/*---------------------------------------------------------------------------*/
static const CmdProtocol_t cmdListUsb[]=
{
	{PROC_CMD_SetSN,				cmd_set_devSN},             //获得 ID 值		数据域：0 Byte
	{PROC_CMD_GetSN,				cmd_get_devSN},             //获得 ID 值		数据域：0 Byte
	{PROC_CMD_GetVer,				cmd_get_version},           //获取版本信息		数据域：10 Byte
	{PROC_CMD_StartHeat,			cmd_heat_start},            //启动加热		数据域：1 Byte
	{PROC_CMD_StopHeat,				cmd_heat_stop},             //停止加热		数据域：0 Byte
	{PROC_CMD_FactoryTest,			cmd_factory_test},          //测试命令		数据域：N Byte
	
	{PROC_CMD_UnLock,			    cmd_unlock},                //解锁 			数据域：4 Byte
	{PROC_CMD_Lock,			        cmd_lock},                  //锁定			数据域：0 Byte
	{PROC_CMD_GetTempCurve,			cmd_get_tempCurve},         //获取温度曲线信息	数据域：0 Byte
	{PROC_CMD_SetTempCurve,			cmd_set_tempCurve},         //设置温度曲线信息	数据域：(组数*4) Byte

	{PROC_CMD_GetPowerCurve,		cmd_get_powerTBL},          //获取功率曲线 		数据域：0 Byte
	{PROC_CMD_SetPowerCurve,		cmd_set_powerTBL},          //设置功率曲线 		数据域：0 Byte

	{PROC_CMD_Get_CurAdj,		    cmd_get_curr_adjust},          // 获取电流校准值   数据域：0Byte
	{PROC_CMD_Set_CurAdj,		    cmd_set_curr_adjust},          // 设置电流校准值	数据域：6Byte

	{PROC_CMD_Get_OpaAdj,           cmd_get_opa_adjust},        //获取 运放校准参数 配置	数据域：0Byte
	{PROC_CMD_Set_OpaAdj,           cmd_set_opa_adjust},        //设置 运放校准参数 配置	数据域：2 + 4Byte

    {PROC_CMD_Get_PowerAdj,         cmd_get_power_adjust},      //获取 功率校准参数 配置	数据域：0Byte
	{PROC_CMD_Set_PowerAdj,         cmd_set_power_adjust},      //设置 功率校准参数 配置	数据域：2 + 4Byte

	{PROC_CMD_GetAdj,			    cmd_get_temp_adjust},            //获取 温度校准参数配置	数据域：0Byte
	{PROC_CMD_SetAdj,			    cmd_set_temp_adjust},            //设置 温度校准参数配置	数据域：2 + 4Byte

	{PROC_CMD_GetPuff,			    cmd_get_puff},              //获取抽吸参数		数据域：0Byte or 2Byte
	{PROC_CMD_SetPuff,			    cmd_set_puff},              //设置抽吸参数		数据域：(组数+2)*2Byte
	{PROC_CMD_GetTR,			    cmd_get_TR},                //获取 TR 信息		数据域：0 Byte
	{PROC_CMD_SetTR,			    cmd_set_TR},                //设置 TR 信息		数据域：(组数*4) Byte

	{PROC_CMD_LogItem,			    cmd_get_log_item},          //获取Log条目 		数据域：0 Byte
	{PROC_CMD_LogControl,			cmd_log_control},           //Log输出控制		数据域：1 Byte 
	
	{PROC_CMD_GetFunc,			    cmd_get_func},          	//获取配置 		数据域：8 Byte
	{PROC_CMD_SetFunc,				cmd_set_func},          	//设置配置			数据域：8 Byte
	
	{PROC_CMD_GetParam,				cmd_get_param},          	//设置调试参数		数据域：n Byte
	{PROC_CMD_SetParam,				cmd_set_param},          	//设置调试参数		数据域：n Byte

	{PROC_CMD_GetSysInfo,			cmd_get_sys_info},          //获取系统信息			数据域：(4+1+1+2) Byte
//	{PROC_CMD_Rdpon,			    cmd_set_rdp_on},          	//rdp on
	
	{PROC_CMD_UpdateControl,		cmd_updata_control},          //数据更新控制		数据域：0~N Byte
	{PROC_CMD_CLI_set,		        cmd_cli_control},          //cli控制		数据域：1 Byte
	{PROC_CMD_AppData,			    cmd_tranfer_app_data},          //传输App数据			数据域：N Byte
	{PROC_CMD_BootData,			    cmd_tranfer_boot_data},          //传输Boot数据			数据域：N Byte
	{PROC_CMD_UI_Data,			    cmd_tranfer_ui_data},          //传输ui数据			数据域：N Byte
	
	{PROC_CMD_UI_Data,			    NULL},          //传输UI数据		 	数据域：N Byte
	{PROC_CMD_WRITE_INI,			cmd_tranfer_write_ini_data},          //写ini数据			数据域：N Byte
	{PROC_CMD_READ_INI,			    cmd_tranfer_read_ini_data},          //读ini数据			数据域：N Byte
	{PROC_CMD_SetGetLogLevel,       cmd_set_get_log_level},             //设置/获取日志级别			数据域：1 Byte
	{PROC_CMD_GetClrSession,        cmd_get_clr_session},             //获取/清除session			数据域：1 Byte
	{PROC_CMD_FactoryReset,         cmd_factory_reset},             //恢复出厂设置		数据域：0 Byte
    {PROC_CMD_SetHeatePinBThreshold, cmd_set_heate_pin_broke_threshold},             // 设置断针保护阈值
	{PROC_CMD_GetHeatePinBThreshold, cmd_get_heate_pin_broke_threshold},             // 获取断针保护阈值

	{PROC_CMD_SetColor,				cmd_set_color},
	{PROC_CMD_GetColor,				cmd_get_color},
	{PROC_CMD_SetBlePairMode,		cmd_set_pair},
	{PROC_CMD_SetDevLock,			cmd_set_dev_lock},
	{PROC_CMD_ClrBleData,			cmd_clr_ble_data},
	{PROC_CMD_SetRTC,				cmd_set_rtc},
	{PROC_CMD_GetRTC,				cmd_get_rtc},
	
	{PROC_CMD_GetRdpOnStatus,		cmd_get_rdpon_status},

	{PROC_CMD_GetBleStatus,			cmd_get_BleStatus},
	{PROC_CMD_GetBlePairList,		cmd_get_BlePairList},
	{PROC_CMD_GetHeatingProfileSel,	cmd_get_heatingProfileSel},
	{PROC_CMD_SetHeatingProfileSel,	cmd_set_heatingProfileSel},
	
    {PROC_CMD_SetGetDryHeatRefParam,    cmd_get_set_dry_heat_data},
    {PROC_CMD_GetSPVerPinBrokeParam,    cmd_get_set_sp_pin_broke_threshold}
};

int filter_cmd_rdp_on(ProtocolBase_t *pFrame)
{
    int res = 0;
    switch (pFrame->cmd) {
        case PROC_CMD_GetVer:
            res = 1;
            break;
        case PROC_CMD_UnLock:
            res = 1;
            break;
        case PROC_CMD_FactoryTest:
            if ( pFrame->pData[0] == PAYLOAD_STA_CTR && (pFrame->pData[1] == 2)) { // 可以进入船运
                res = 1;
            }
            break;
        case PROC_CMD_UpdateControl:
            uint16_t updataType = COMB_2BYTE(pFrame->pData[0],pFrame->pData[1]);
            if (updataType == UPDATA_BYTE01_APP_START || updataType == UPDATA_BYTE01_APP_FLAG) { // 可以升级app
                res = 1;
            }
            break;
        case PROC_CMD_AppData:
            res = 1;
            break;
        case PROC_CMD_BootData:
            res = 0;
            break;
        case PROC_CMD_UI_Data:
            res = 0;
            break;
        case PROC_CMD_GetSN:
            res = 1;
            break;
        case PROC_CMD_LogControl:
            res = 1;
            break;
        case PROC_CMD_GetRdpOnStatus:
            res = 1;
            break;
        default:
            res = 0;
            break;
    }
    return res;
}

int filter_cmd_factory_prod(ProtocolBase_t *pFrame)
{
    int res = 1;
    switch (pFrame->cmd) {
        case PROC_CMD_SetTempCurve:
            if (pFrame->pData[0] != TEST_MODE) {
                res = 0;
            }
            break;
        case PROC_CMD_SetPowerCurve:
            if (pFrame->pData[0] != TEST_MODE) {
                res = 0;
            }
            break;
        default:
            res = 1;
            break;
    }
    return res;
}

//procotol parse 
int frame_parse_usb(uint8_t *pBuf,uint16_t rxLen)
{

	ProtocolBase_t *pFrame = (ProtocolBase_t*)pBuf;
	uint16_t tempLen = 0;
	uint16_t tempSum = 0;

	if(rxLen < PROC_AT_LIST_LEN)
	{
		return RET_FAILURE;
	}

	/*check ahead*/
	if(pFrame->ahead != PROC_AHEAD)
	{
		return RET_FAILURE;
	}

	/*check device ID*/
	if(pFrame->deviceID != DEV_TX_TO_RX(DEV_PC,DEV_1P))
	{
		return RET_FAILURE;
	}

	/*check length*/
	tempLen = COMB_2BYTE(pFrame->dataLen.high,pFrame->dataLen.low);
	if(rxLen < (tempLen+PROC_AT_LIST_LEN))
	{
		return RET_FAILURE;
	}


	/*check checksum*/
	tempSum = ((uint16_t)(pFrame->pData[tempLen+1]<<8)|(uint16_t)pFrame->pData[tempLen]);
	if(calc_checksum((uint8_t*)pBuf,(tempLen+PROC_AT_LIST_LEN-2))!=tempSum)
	{
		return RET_FAILURE;
	}
//    #if 0
	if (get_rdp_on_mode_no_save_flash() == 1) { //
	    if (filter_cmd_rdp_on(pFrame) != 1) {
            pFrame->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
            return (int)cmd_base_reply(pFrame,PROC_RESPONE_NCK);
        }
    }
#ifdef FACTORY_PROD
    if (filter_cmd_factory_prod(pFrame) != 1) {
        pFrame->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
        return (int)cmd_base_reply(pFrame,PROC_RESPONE_NCK);
    }
#endif
//    #endif
	for(uint16_t i = 0; i < eleof(cmdListUsb); i++)
	{
		if(pFrame->cmd == cmdListUsb[i].dataCmd && cmdListUsb[i].func != NULL)
		{
			return (int)cmdListUsb[i].func(pFrame);
		}
	}
   
	return RET_FAILURE;
}




//procotol create 
uint16_t frame_create_usb(uint8_t *pTxBuf,uint8_t txCmd,uint8_t *pDataBuf,uint16_t txLen)
{
	ProtocolBase_t *pFrame = (ProtocolBase_t*)pTxBuf;
	uint16_t checkSum = 0;

	pFrame->ahead = PROC_AHEAD;
	pFrame->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);
	pFrame->cmd = 	txCmd;
	pFrame->dataLen.low = LBYTE(txLen);
	pFrame->dataLen.high = HBYTE(txLen);
	memcpy(pFrame->pData,pDataBuf,txLen);
	checkSum = calc_checksum((uint8_t*)pTxBuf,(txLen+PROC_AT_LIST_LEN-2));
	pFrame->pData[txLen] = LBYTE(checkSum);
	pFrame->pData[txLen+1] = HBYTE(checkSum);

	return (txLen+PROC_AT_LIST_LEN);
}









