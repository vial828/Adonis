
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

/*******************************************************************************
 * ******************************** 初始化流程 **********************************
1.读取SEALED状态
    wr 0x00 0x00 0x00 //发送control()命令
    rd 0x00       //读取control_status寄存器
    control_status寄存器-高字节bit5为1，则表示处于SEALED
2.检查是否处于UNSEAL，如果处于SEALED，发送解封命令   
    wr 0x00 0x00 0x80
    wr 0x00 0x00 0x80
3.发送配置更新子命令
    wr 0x00 0x13 0x00
4.等待进入配置更新模式
    检查Flags()寄存器-低字节bit4,bit4=1,则表示进入更新模式
    rd 0x06
5.启动数据块控制
    wr 0x61 0x00

6.写入数据块classid和偏移位置
    wr 0x3e id offset
7.写入块数据
    wr 0x40 data-32bytes
8.写入checksum
    wr 0x60 checksum
9.等待10ms
10.检查checksum
    a.写入数据块classid和偏移位置
        wr 0x3e id offset
    b.读取checksum
        rd 0x60
    c.判断checksum是否写入正确
11.重复6-10，写入全部数据

12.退出配置更新模式
    wr 0x00 0x42 0x00
13.等待退出
    检查Flags()寄存器-低字节bit4,bit4=0,则表示退出更新模式
    rd 0x06
14.重新封印SEALED
    wr 0x00 0x20 0x00

*********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "bq27426.h"
#include "bq27426_port.h"

/****************************************************
 * flash数据存储说明
 * 0-3 byte    0-1:OK校验字,    2:电芯id号,    3:0xff
 * 4-...       gmfs数据缓存   36*24=864
 * 
 * **************************************************/       

#define BQ27426_DATA_FLASH_OK       (BQ27426_GMFS_VERSION)      //flash数据OK校验值
#define BQ27426_DATA_FLASH_OK_ADDR  (BQ27426_FLASH_ADDR_START)  //flash数据OK标志地址
#define BQ27426_DATA_FLASH_ADDR     (BQ27426_FLASH_ADDR_START)  //flash存储地址

/* Private macro -------------------------------------------------------------*/

/* Private Consts -------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

typedef struct 
{
	uint8_t num;  //备用
    uint8_t classid;
    uint8_t offset;    //0-0  1-32  2-64
    uint8_t checksum;
    uint8_t blockdata[32];
}bqfs_block_data_t;    //4字节对齐


#define GMFS_CONFIG_MODE   (0)   //0-配置ChemID,GoldenImage   1-仅配置ChemID
#if (GMFS_CONFIG_MODE==0) // 0-配置ChemID,GoldenImage    
#define BQ27426_GMFS_BLOCK_NUM        (24)       //block  块数

#else                     // 1-仅配置ChemID
#define BQ27426_GMFS_BLOCK_NUM        (8)       //block  块数

#endif

#define BQ27426_GMFS_BLOCK_DATA_T_LEN (36)       //block data 结构体长度   4字节对齐
#define BQ27426_GMFS_BLOCK_SIZE       (BQ27426_GMFS_BLOCK_NUM*BQ27426_GMFS_BLOCK_DATA_T_LEN+4) //总字节数
/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static bq27426_bat_type_t  bq_bat_type;  //电芯类型

typedef struct FuelGauge_Bqfs_t {
    uint32_t magic;
    uint16_t len;
	
    uint16_t ver;
    uint8_t batType;
	uint8_t reserve;
    bqfs_block_data_t block_data_buf[BQ27426_GMFS_BLOCK_NUM];
	
    uint16_t crc;
} FuelGauge_Bqfs_t;

FuelGauge_Bqfs_t fg_bqfs = {
	.magic = 0,
	.len = 0,
	.ver = 0,
	.batType = 0,
	.reserve = 0,
	.block_data_buf = {
		//chemID
		{0xff, 0x53, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x53, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x54, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x54, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x55, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x6C, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x59, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x6D, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		//Golden gmfs
		{0xff, 0x02, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x24, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x31, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x40, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x44, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x50, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x50, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x50, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x51, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x52, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x59, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x6d, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x68, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x69, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x6B, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0xff, 0x70, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
	.crc = 0,
};

/* Exported functions --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/****************************************************************************************
 * @brief   Read a 16-bit command word from the bq27426
 * @param   subAddress  address
 * @return  16bit value
 * @note
 ****************************************************************************************/
static uint16_t bq27426_read_word(uint16_t subAddress)
{
	uint8_t data[2]={0,0};
	bq27426_read_bytes(subAddress, data, 2);
	return ((uint16_t)data[1] << 8) | data[0];
}

/****************************************************************************************
 * @brief   Read a 16-bit subcommand() from the BQ27426-G1A's control()
 * @param   function address
 * @return  16bit value
 * @note
 *****************************************************************************************/
static uint16_t bq27426_read_control_word(uint16_t function)
{
	uint8_t command[2] = {(function & 0x00FF), (function >> 8)};
	uint8_t data[2] = {0, 0};

	bq27426_write_bytes(BQ27426_COMMAND_CONTROL, command, 2);

	if (true == bq27426_read_bytes(BQ27426_COMMAND_CONTROL, data, 2))
	{
		return ((uint16_t)data[1] << 8) | data[0];
	}
	return false;
}

/****************************************************************************************
 * @brief   Execute a subcommand() from the BQ27426-G1A's control()
 * @param   function address
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_execute_control_word(uint16_t function)
{
	uint8_t command[2] = {(function & 0x00FF), (function >> 8)};

	if (true == bq27426_write_bytes(BQ27426_COMMAND_CONTROL, command, 2))
	{
		return true;
	}
	return false;
}

/*****************************************************************************
 ************************** Extended Data Commands ***************************
 *****************************************************************************/

//------------------------------------------------------------------------------
/****************************************************************************************
 * @brief   block data control enabel
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_blockdata_control_enable(void)
{
	uint8_t enableByte = 0x00;
	if(true == bq27426_write_bytes(BQ27426_EXTENDED_CONTROL, &enableByte, 1))
	{
		return true;
	}
	return false;
}

/****************************************************************************************
 * @brief   Read all 32 bytes of the loaded extended data and compute a checksum based on the values.
 * @param   none
 * @return  checksum 8-bit
 * @note
 *****************************************************************************************/
static uint8_t bq27426_computeBlockChecksum(void)
{
	uint8_t data[32];
	bq27426_read_bytes(BQ27426_EXTENDED_BLOCKDATA, data, 32);

	uint8_t csum = 0;
	for (int i=0; i<32; i++)
	{
		csum += data[i];
	}
	csum = 255 - csum;
	
	return csum;
}

/****************************************************************************************
 * @brief   Seal the BQ27426-G1A
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_seal(void)
{
	// return bq27426_read_control_word(BQ27426_CONTROL_SEALED);
	// bq27426_read_control_word(BQ27426_CONTROL_SEALED);
	return true;
}

/****************************************************************************************
 * @brief   UNseal the BQ27426-G1A
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_unseal(void)
{
	// 退出cfg update mode 后，加锁，再次解锁需等4s

	int16_t tcout = 200;
	uint8_t command[2] = {0x00, 0x80};
	while ((tcout) && (bq27426_read_control_word(BQ27426_CONTROL_STATUS) & BQ27426_STATUS_SS))
	{
		bq27426_write_bytes(BQ27426_COMMAND_CONTROL, command, 2);
		bq27426_write_bytes(BQ27426_COMMAND_CONTROL, command, 2);
		bq27426_mDelay(10);
		bq27426_wdog_feed();
		tcout--;
	}
	if (tcout > 0)
	{
		return true;
	}
	return false;
}

/*****************************************************************************
 *************************** Control Sub-Commands ****************************
 *****************************************************************************/
// Enter configuration mode - set userControl if calling from an Arduino sketch
// and you want control over when to bq27426_exitConfig
/****************************************************************************************
 * @brief   enter config updata mode
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_enterConfig(void)
{
	if (false == bq27426_unseal())
	{
		return false;
	}

	if (bq27426_read_word(BQ27426_COMMAND_FLAGS) & BQ27426_FLAG_CFGUPMODE) // cfg update mode
	{
		return true;
	}
	else
	{
		int16_t timeout = BQ27426_I2C_TIMEOUT;
		do
		{
			bq27426_execute_control_word(BQ27426_CONTROL_SET_CFGUPDATE); // enter cfg
			bq27426_mDelay(1);
			bq27426_wdog_feed();
			timeout--;
		} while ((timeout > 0) && (!(bq27426_read_word(BQ27426_COMMAND_FLAGS) & BQ27426_FLAG_CFGUPMODE)));
		bq27426_mDelay(1);
		if (timeout > 0)
		{
			return true;
		}
	}
	return false;
}

// Exit configuration mode with the option to perform a resimulation
/****************************************************************************************
 * @brief   exit config updata mode
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_exitConfig(uint8_t resim)
{
	// There are two methods for exiting config mode:
	//    1. Execute the EXIT_CFGUPDATE command
	//    2. Execute the SOFT_RESET command
	// EXIT_CFGUPDATE exits config mode _without_ an OCV (open-circuit voltage)
	// measurement, and without resimulating to update unfiltered-SoC and SoC.
	// If a new OCV measurement or resimulation is desired, SOFT_RESET or
	// EXIT_RESIM should be used to exit config mode.
	
	if (bq27426_read_word(BQ27426_COMMAND_FLAGS) & BQ27426_FLAG_CFGUPMODE) // cfg update mode
	{
		int16_t timeout = BQ27426_I2C_TIMEOUT;
		uint16_t flag;
		uint16_t ctrl;
		do
		{
			bq27426_execute_control_word(BQ27426_CONTROL_SOFT_RESET); // soft reset
			flag = bq27426_read_word(BQ27426_COMMAND_FLAGS) & (BQ27426_FLAG_CFGUPMODE | BQ27426_FLAG_ITPOR); // flags() cfg  ITPOR
			ctrl = bq27426_read_control_word(BQ27426_CONTROL_STATUS)&BQ27426_STATUS_INITCOMP; // ctrl() INITCOMP
			bq27426_mDelay(1);
			bq27426_wdog_feed();
			timeout--;
		} while ((timeout > 0) && (flag || ctrl));
		bq27426_mDelay(10);
		if (timeout > 0)
		{
			bq27426_seal(); // 退出cfg update mode 后，加锁，再次解锁需等4s
			return true;
		}
		return false;
	}
	return true;
}



//-------------------------------------------------------------------------------------------
//gmfs 本地缓存操作api
//-------------------------------------------------------------------------------------------

/****************************************************************************************
 * @brief   计算32字节数据checksum
 * @param   blockdata   32字节数组地址
 * @return  checksum 8-bit
 * @note
 *****************************************************************************************/
static uint8_t blockdata_checksum_compute32_by_buf(uint8_t *blockdata)
{
	uint8_t csum=0;
    uint8_t i;
	for(i=0;i<32;i++)
	{
		csum += *(blockdata+i);
	}
	csum = 255-csum;
	return csum;
}

/****************************************************************************************
 * @brief   读取blockdata数据块
 * @param   b_data   bqfs_block_data_t指针
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_read_block_data_t(bqfs_block_data_t *b_data)
{
	int16_t delay = 10;
	uint8_t data[2] = {0, 0};
	bq27426_wdog_feed();
	// 写classid  offset
	delay = 5;
	do {
		bq27426_mDelay(1);
		bq27426_write_bytes(BQ27426_EXTENDED_DATACLASS, &b_data->classid, 2);
		bq27426_mDelay(1);
		bq27426_read_bytes(BQ27426_EXTENDED_DATACLASS, data, 2);
		bq27426_wdog_feed();
	} while (((data[0] != b_data->classid) || (data[1] != b_data->offset)) && (delay--));
	if (delay <= 0)
	{
		return false;
	}

	// 读取blockdata  checksum
	uint8_t bdata[33];
	uint8_t csum = 0;
	uint8_t i = 0;
	delay = 5;
	do {
		bq27426_mDelay(1);
		bq27426_read_bytes(BQ27426_EXTENDED_BLOCKDATA, bdata, 32);
		bq27426_mDelay(1);
		bq27426_read_bytes(BQ27426_EXTENDED_CHECKSUM, &bdata[32], 1);
		// for (i = 0; i < 32; i++)
		// {
		// 	csum += bdata[i];
		// }
		// csum = 255 - csum;
		bq27426_wdog_feed();
		csum = blockdata_checksum_compute32_by_buf(bdata);
	} while ((csum != bdata[32]) && (delay--));
	if (delay <= 0)
	{
		return false;
	}

	// 数据正确 更新缓存
	for (i = 0; i < 32; i++)
	{
		b_data->blockdata[i] = bdata[i];
	}
	b_data->checksum = csum;

	return true;
}

/****************************************************************************************
 * @brief   读取指定地址，指定长度的blockdata数据块 
 * @param   id   class id
 * @param   offset_addr   块内offset addr
 * @param   databuf   读出数据保存指针
 * @param   len   数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_blockdata_read(uint8_t id,uint8_t offset_addr,uint8_t* databuf,uint8_t len)
{
	int8_t ret = true;

	if(false == bq27426_unseal()) //解锁
	{
		ret = false;
	}
	bq27426_mDelay(1);

	if (!bq27426_blockdata_control_enable()) // enable block data memory control
	{
		ret = false;
	}
    bq27426_mDelay(1);
    
	bqfs_block_data_t b_data;
	b_data.classid = id;
	b_data.offset = offset_addr/32;
	if(false ==bq27426_read_block_data_t(&b_data))
	{
		ret = false;
	}

	uint8_t addr = offset_addr%32;
	uint8_t i;
	for(i=0;i<len;i++)
	{
		*(databuf+i) = b_data.blockdata[addr+i];
	}
	bq27426_seal(); // 加锁SEALED
	return ret;
}


/****************************************************************************************
 * @brief   读取IC初始化配置的所有数据到缓存 
 * @param   none
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_readBlockData(void)
{
#if 0 // 测试代码-清零数组
	uint8_t j = 0,k = 0;
	for(j=0;j<BQ27426_GMFS_BLOCK_NUM;j++)
	{
		block_data_buf[j].checksum = 0;
		for(k=0;k<32;k++)
		{
			block_data_buf[j].blockdata[k] = 0;
		}
	}
#endif
	int8_t ret = true;
	
	if(false == bq27426_unseal()) //解锁
	{
		ret = false;
	}
	bq27426_mDelay(1);

	if (!bq27426_blockdata_control_enable()) // enable block data memory control
	{
		ret = false;
	}

	uint8_t i = 0;
	for (i = 0; i < BQ27426_GMFS_BLOCK_NUM; i++) // 读取blockdata
	{
		if (ret == false)
			break;
		if (false == bq27426_read_block_data_t(&fg_bqfs.block_data_buf[i]))
		{
			ret = false;
		}
	}

	bq27426_seal(); // 加锁SEALED
	return ret;
}

/****************************************************************************************
 * @brief   写入blockdata数据块
 * @param   b_data   bqfs_block_data_t指针
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_write_block_data_t(bqfs_block_data_t *b_data)
{
	int16_t delay = 5;
	uint8_t data[2] = {0, 0};
	uint8_t csum = 0;
	bq27426_wdog_feed();
	delay = 5;
	do {
		bq27426_mDelay(1);
		bq27426_write_bytes(BQ27426_EXTENDED_DATACLASS, &b_data->classid, 2); // 写classid  offset
		bq27426_mDelay(1);
		bq27426_read_bytes(BQ27426_EXTENDED_DATACLASS, data, 2); // 读classid  offset
		bq27426_wdog_feed();
	} while (((data[0] != b_data->classid) || (data[1] != b_data->offset)) && (delay--)); // 是否写成功
	if (delay < 0)
	{
		//BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"classid wr err!\r\n");
		return false;
	}

	delay = 5;
	do {
		bq27426_write_bytes(BQ27426_EXTENDED_BLOCKDATA, &b_data->blockdata[0], 32); // 写blockdata
		bq27426_mDelay(1);
		csum = bq27426_computeBlockChecksum();		   // 计算blockdata checksum
		bq27426_wdog_feed();
	} while ((b_data->checksum != csum) && (delay--)); // blockdata是否写成功
	if (delay < 0)
	{
		//BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"blockdata wr err!\r\n");
		return false;
	}

	delay = 5;
	do {
		bq27426_write_bytes(BQ27426_EXTENDED_CHECKSUM, &b_data->checksum, 1); // 写checksum
		bq27426_mDelay(10);													  // 延时10ms
		bq27426_write_bytes(BQ27426_EXTENDED_DATACLASS, &b_data->classid, 2); // 写classid  offset
		bq27426_mDelay(1);
		bq27426_read_bytes(BQ27426_EXTENDED_CHECKSUM, &csum, 1); // 读checksum
		bq27426_wdog_feed();
	} while ((b_data->checksum != csum) && (delay--)); // checksum是否写成功
	if (delay < 0)
	{
		//BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"checksum wr err!\r\n");
		return false;
	}
	return true;
}

/****************************************************************************************
 * @brief   写入指定地址，指定长度的blockdata数据块 
 * @param   id   class id
 * @param   offset_addr   块内offset addr
 * @param   databuf   写入数据指针
 * @param   len   数据长度
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_blockdata_write(uint8_t id,uint8_t offset_addr,uint8_t* databuf,uint8_t len)
{
	int8_t ret = true;

	if(false==bq27426_enterConfig())
	{
		ret = false;
	}
	// 使能数据块控制
	if (!bq27426_blockdata_control_enable()) // enable block data memory control
	{
		ret = false;
	}

	bqfs_block_data_t b_data;
	b_data.classid = id;
	b_data.offset = offset_addr/32;
	if(false ==bq27426_read_block_data_t(&b_data))
	{
		ret = false;
	}

	uint8_t addr = offset_addr%32;
	uint8_t i;
	for(i=0;i<len;i++)   //将数据写入buf
	{
		b_data.blockdata[addr+i] = *(databuf+i);
	}
	
	b_data.checksum = blockdata_checksum_compute32_by_buf(&b_data.blockdata[0]); //计算新checksum

	//写入新配置
    if(ret == true)
    {
        if(false == bq27426_write_block_data_t(&b_data))
        {
            ret = false;
        }

    }
	// 退出配置更新
	bq27426_exitConfig(true);
	return ret;
}

/****************************************************************************************
 * @brief   初始化IC参数  ---  将所有初始化数据写入IC
 * @param   none
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_writeBlockData(void)
{
	int8_t ret = true;

	// 进入配置更新模式
	if(false == bq27426_enterConfig())
	{
		//BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"enter config err!\r\n");
		ret = false;
	}
	// 使能数据块控制
	if (!bq27426_blockdata_control_enable()) // enable block data memory control
	{
		//BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"enable block err!\r\n");
		ret = false;
	}

	// 写入所有数据块
	uint8_t i;
	for (i = 0; i < BQ27426_GMFS_BLOCK_NUM; i++)
	{
		if (ret == false)
			break;
		if (false == bq27426_write_block_data_t(&fg_bqfs.block_data_buf[i]))
		{
			//BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"write block err!\r\n");
			ret = false;
		}
	}

	// 退出配置更新
	bq27426_exitConfig(true);

	return ret;
}


// uint16_t bq27426_blockdata_read_byte(){}

/****************************************************************************************
 * @brief   blockdata 读取指定地址字（2字节）
 * @param   id   class id
 * @param   offset_addr   块内offset addr
 * @return  16-bit数据
 * @note
 *****************************************************************************************/
static uint16_t bq27426_blockdata_read_word(uint8_t id,uint8_t offset_addr,uint16_t* word_data)
{
	uint8_t databuf[2]={0,0};
	if(false == bq27426_blockdata_read(id,offset_addr,databuf,2))
	{
		return false;
	}
	*word_data = ((uint16_t)(databuf[0]<<8)|(databuf[1]));
	return true;
}

/****************************************************************************************
 * @brief   blockdata 写入指定地址字（2字节）
 * @param   id   class id
 * @param   offset_addr   块内offset addr
 * @param   word_data   数据
 * @return  true/false
 * @note
 *****************************************************************************************/
static int8_t bq27426_blockdata_write_word(uint8_t id,uint8_t offset_addr,uint16_t word_data)
{
	int8_t timout = 5;
	uint8_t rdata[2] = {0,0};
	uint8_t wdata[2] = {(uint8_t)(word_data>>8),(uint8_t)word_data};
	if(false == bq27426_blockdata_read(id,offset_addr,rdata,2))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"read err![%d-%d]\n",id,offset_addr);
		return false;
	}

	while(timout && ((rdata[0]!=wdata[0])||(rdata[1]!=wdata[1])))
	{
		if(true == bq27426_blockdata_write(id, offset_addr,wdata,2))
		{
			if(true == bq27426_blockdata_read(id,offset_addr,rdata,2))
			{

			}
		}
		bq27426_mDelay(1);
		bq27426_wdog_feed();
		timout--;
	}
	if(timout>0)
	{
		uint16_t rd = ((uint16_t)(rdata[0])<<8)|rdata[1];
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"Write OK.[%d-%d]:0x%04x\n",id,offset_addr,rd);
		return true;
	}
	BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"write err![%d-%d]\n",id,offset_addr);
	return false;
}

//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
//block data 区间参数配置 api
//-------------------------------------------------------------------------------------------

/****************************************************************************************
 * @brief   配置taper rate
 * @param   taper_rate   0-2000  0.1hrata  
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_set_taper_rate(uint16_t taper_rate)
{
	if(false == bq27426_blockdata_write_word(82,21,taper_rate))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"TaperRate set fail!\r\n");
		return false;
	}
	// BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"TaperRate set:%d\r\n", taper_rate);
	return true;
}

/****************************************************************************************
 * @brief   配置taper voltage
 * @param   taper_voltage   0-5000  mV
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_set_taper_voltage(uint16_t taper_voltage)
{
	if(false == bq27426_blockdata_write_word(109,8,taper_voltage))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"TaperVoltage set fail!\r\n");
		return false;
	}
	//BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"TaperVoltage set:%d\r\n", taper_voltage);
	return true;
}

/****************************************************************************************
 * @brief   配置OpConfig
 * @param   enSleep   1-使能自动休眠 0-禁止
 * @param   enExNtc   1-使能外部NTC 0-内部NTC
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_set_opconfig(uint8_t enSleep,uint8_t enExNtc)
{
	uint16_t opconfig ; 

	if(false == bq27426_blockdata_read_word(64,0,&opconfig))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"opCofig set fail!\r\n");
		return false;
	}
    
	if (enSleep != 0) {
		opconfig |= BQ27426_OPCONFIG_SLEEP;
	} else {
		opconfig &= ~BQ27426_OPCONFIG_SLEEP;
	}

	if (enExNtc != 0) {
		opconfig &= ~BQ27426_OPCONFIG_TEMPS1;
		opconfig |= BQ27426_OPCONFIG_TEMPS0;
	} else {
		opconfig &= ~BQ27426_OPCONFIG_TEMPS1;
		opconfig &= ~BQ27426_OPCONFIG_TEMPS0;
	}
	
	if(false == bq27426_blockdata_write_word(64,0,opconfig))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"opCofig set fail!\r\n");
		return false;
	}
	return true;
}
/****************************************************************************************
 * @brief   获取design capacity 电芯设计容量
 * @param   *design_cap 数据指针
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_get_design_capacity(uint16_t *design_cap)
{
	if(false == bq27426_blockdata_read_word(82,6,design_cap))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 get Design Capacity fail!\r\n");
		return false;
	}
	return true;
}
/****************************************************************************************
 * @brief   获取Taper Rate
 * @param   *taper_rate 数据指针
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_get_taper_rate(uint16_t *taper_rate)
{
	if(false == bq27426_blockdata_read_word(82,21,taper_rate))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 get Taper Rate fail!\r\n");
		return false;
	}
	return true;
}

/****************************************************************************************
 * @brief   获取Taper Voltage
 * @param   *taper_volt 数据指针
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_get_taper_voltage(uint16_t *taper_volt)
{
	if(false == bq27426_blockdata_read_word(109,8,taper_volt))
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 get Taper Voltage fail!\r\n");
		return false;
	}
	return true;
}

/*****************************************************************************
 ********************** Battery Characteristics Functions ********************
 *****************************************************************************/

/****************************************************************************************
 * @brief   Read the device type - should be 0x0426
 * @param   none
 * @return  device type
 * @note
 *****************************************************************************************/
uint16_t bq27426_deviceType(void)
{
	return bq27426_read_control_word(BQ27426_CONTROL_DEVICE_TYPE);
}

/****************************************************************************************
 * @brief   Read the device fw version 
 * @param   none
 * @return  fw version 
 * @note
 *****************************************************************************************/
uint16_t bq27426_fwVersion(void)
{
	return bq27426_read_control_word(BQ27426_CONTROL_FW_VERSION);
}

/****************************************************************************************
 * @brief   Read voltage  -  BQ27426_COMMAND_VOLTAGE
 * @param   none
 * @return  voltage
 * @note
 *****************************************************************************************/
uint16_t bq27426_voltage(void)
{
	return bq27426_read_word(BQ27426_COMMAND_VOLTAGE);
}


/****************************************************************************************
 * @brief   Read current  
 * @param   type 类型
 * 				- AVG         BQ27426_COMMAND_AVG_CURRENT
 * 				- STBY        BQ27426_COMMAND_STDBY_CURRENT
 * 				- MAX_CURENT  BQ27426_COMMAND_MAX_CURRENT
 * @return  current
 * @note
 *****************************************************************************************/
int16_t bq27426_current(current_measure type)
{
	int16_t current = 0;
	switch (type)
	{
	case AVG:
		current = (int16_t) bq27426_read_word(BQ27426_COMMAND_AVG_CURRENT);
		break;
	case STBY:
		current = (int16_t) bq27426_read_word(BQ27426_COMMAND_STDBY_CURRENT);
		break;
	case MAX_CURENT:
		current = (int16_t) bq27426_read_word(BQ27426_COMMAND_MAX_CURRENT);
		break;
	}
	
	return current;
}


/****************************************************************************************
 * @brief   Read capacity
 * @param   type 类型
 * 				- REMAIN      BQ27426_COMMAND_REM_CAPACITY
 * 				- FULL        BQ27426_COMMAND_FULL_CAPACITY
 * 				- AVAIL       BQ27426_COMMAND_NOM_CAPACITY
 * 				- AVAIL_FULL  BQ27426_COMMAND_AVAIL_CAPACITY
 * 				- REMAIN_F    BQ27426_COMMAND_REM_CAP_FIL
 * 				- REMAIN_UF   BQ27426_COMMAND_REM_CAP_UNFL
 * 				- FULL_F      BQ27426_COMMAND_FULL_CAP_FIL
 * 				- FULL_UF     BQ27426_COMMAND_FULL_CAP_UNFL
 * @return  capacity
 * @note
 *****************************************************************************************/
uint16_t bq27426_capacity(capacity_measure type)
{
	uint16_t capacity = 0;
	switch (type)
	{
	case REMAIN:
		return bq27426_read_word(BQ27426_COMMAND_REM_CAPACITY);
		break;
	case FULL:
		return bq27426_read_word(BQ27426_COMMAND_FULL_CAPACITY);
		break;
	case AVAIL:
		capacity = bq27426_read_word(BQ27426_COMMAND_NOM_CAPACITY);
		break;
	case AVAIL_FULL:
		capacity = bq27426_read_word(BQ27426_COMMAND_AVAIL_CAPACITY);
		break;
	case REMAIN_F: 
		capacity = bq27426_read_word(BQ27426_COMMAND_REM_CAP_FIL);
		break;
	case REMAIN_UF:
		capacity = bq27426_read_word(BQ27426_COMMAND_REM_CAP_UNFL);
		break;
	case FULL_F:
		capacity = bq27426_read_word(BQ27426_COMMAND_FULL_CAP_FIL);
		break;
	case FULL_UF:
		capacity = bq27426_read_word(BQ27426_COMMAND_FULL_CAP_UNFL);
		break;
	default:
		break;
	}
	
	return capacity;
}


/****************************************************************************************
 * @brief   Read average power    
 * 			-BQ27426_COMMAND_AVG_POWER
 * @param   none
 * @return  power
 * @note
 *****************************************************************************************/
int16_t bq27426_power(void)
{
	return (int16_t) bq27426_read_word(BQ27426_COMMAND_AVG_POWER);
}

/****************************************************************************************
 * @brief   BAT SOC百分比重映射  
 * @param   soc   真实百分比
 * @return  映射后百分比
 * @note
 *****************************************************************************************/
#if 0 //
static uint16_t bat_soc_remappig(uint16_t soc)
{
    //0-10%    ->  0-5%
    //10-100%  ->  5-100%
	uint16_t p = 10;
	uint16_t re_soc;
	if(soc > 100) soc = 100;
	if (soc <= p)
	{
		re_soc = soc * 5 / p;
	}
	else
	{
		re_soc = (soc - p) * (100 - 5) / (100 - p) + 5;
	}
	return re_soc;
}
#endif

static uint16_t bat_soc_remappig(uint16_t soc)
{
    //0-10%    ->  0-5%
    //10-100%  ->  5-100%
    uint16_t    p = 10;
    uint16_t    re_soc;
    float       f_resoc;
    
    if(soc > 100) { soc = 100; }
    if (soc <= p)
    {
        f_resoc = (float)(soc) * 5.0f / p + 0.5f;
    }
    else
    {
        f_resoc = (float)(soc - p) * (100.0f - 5.0f) / (100.0f - p) + 5.0f + 0.5f;
    }   
    
    re_soc = (uint16_t)(f_resoc);
    
    if(re_soc > 100) { re_soc = 100; }
    
    return re_soc;
}

/****************************************************************************************
 * @brief   Read SOC
 * @param   type 类型
 * 				- FILTERED          BQ27426_COMMAND_SOC
 * 				- UNFILTERED        BQ27426_COMMAND_SOC_UNFL
 * @return  soc
 * @note
 *****************************************************************************************/
uint16_t bq27426_soc(soc_measure type)
{
	uint16_t socRet = 0;
	switch (type)
	{
	case FILTERED:
		socRet = bq27426_read_word(BQ27426_COMMAND_SOC);
		break;
	case UNFILTERED:
		socRet = bq27426_read_word(BQ27426_COMMAND_SOC_UNFL);
		break;
	}
	// socRet = bat_soc_remappig(socRet);
	return socRet;
}

/****************************************************************************************
 * @brief   Read SOH
 * @param   type 类型
 * 				- PERCENT          soh percent
 * 				- SOH_STAT         soh status
 * @return  soh/stat
 * @note
 *****************************************************************************************/
uint8_t bq27426_soh(soh_measure type)
{
	uint16_t sohRaw = bq27426_read_word(BQ27426_COMMAND_SOH);
	uint8_t sohStatus = sohRaw >> 8;
	uint8_t sohPercent = sohRaw & 0x00FF;
	
	if (type == PERCENT)	
		return sohPercent;
	else
		return sohStatus;
}

/****************************************************************************************
 * @brief   Read temperature
 * @param   type 类型
 * 				- BATTERY          温度
 * 				- INTERNAL_TEMP    内部温度
 * @return  temperature
 * @note
 *****************************************************************************************/
int16_t bq27426_temperature(temp_measure type)
{
	int16_t  temp = 0;
	int32_t  kelvin = 0;

	switch (type)
	{
	case BATTERY:
		kelvin |= bq27426_read_word(BQ27426_COMMAND_TEMP);
		break;
	case INTERNAL_TEMP:
		kelvin |= bq27426_read_word(BQ27426_COMMAND_INT_TEMP);
		break;
	}

	temp = (kelvin - 2721)/10;
	return temp;
}

/****************************************************************************************
 * @brief   Read passed charge
 * @param   none
 * @return  passed charge
 * @note
 *****************************************************************************************/
uint16_t bq27426_passed_charge(void)
{
	return bq27426_read_word(BQ27426_COMMAND_PASSED_CHARGE); //passed charge  能量
}

/****************************************************************************************
 * @brief   bq27426 enter shutdown mode
 * @param   none
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_shutdown(void)
{
	uint16_t stat = 0;
	uint8_t i = 0;
	
	bq27426_gpout_pin_enter_sleep(); // 关闭IC前先释放GPOUT
	do{
		bq27426_execute_control_word(BQ27426_CONTROL_SHUTDOWN_ENABLE);
		bq27426_mDelay(2);
		stat = bq27426_read_control_word(BQ27426_CONTROL_STATUS) & BQ27426_STATUS_SHUTDOWNEN;
		i++;
		if(i>5) {
			return false;
		}
		bq27426_wdog_feed();
	}while(0x8000 != stat);//等待shutdown en 标志置位
	bq27426_execute_control_word(BQ27426_CONTROL_SHUTDOWN);
	BQ27426_LOG(BQ27426_LOG_LEVEL_DEBUG,"bq27426 shutdown ok!\n");
	bq27426_mDelay(10);

	return true;
}

/****************************************************************************************
 * @brief   Read fuel gauge param
 * @param   fg_info   BatInfo_t*
 * @return  true / false
 * @note
 *****************************************************************************************/
int8_t bq27426_read_param(BatInfo_t *fg_info)
{
#if 0
	 fg_info->ctrl_status = bq27426_read_control_word(BQ27426_CONTROL_STATUS);//control status()
	 fg_info->flags       = bq27426_read_word(BQ27426_COMMAND_FLAGS);//flags()

	 fg_info->voltage     = bq27426_voltage(); // Read battery voltage (mV)
	 fg_info->current     = bq27426_current(AVG); // Read average current (mA)
	 fg_info->power       = bq27426_power(); // Read average power draw (mW)
	 fg_info->temperature = bq27426_temperature(BATTERY);//(℃)
	 fg_info->pcbtemperature = bq27426_temperature(INTERNAL_TEMP);//(℃)

	 fg_info->capacityFull   = bq27426_capacity(FULL); // Read full capacity (mAh)
	 fg_info->capacityRemain = bq27426_capacity(REMAIN); // Read remaining capacity (mAh)

	 fg_info->passed_charge  = bq27426_read_word(BQ27426_COMMAND_PASSED_CHARGE);

	 fg_info->ture_rm      = bq27426_read_word(BQ27426_COMMAND_REM_CAP_UNFL); // RemainingCapacityUnfiltered()
	 fg_info->ture_fcc     = bq27426_read_word(BQ27426_COMMAND_FULL_CAP_UNFL);// FullChargeCapacityUnfiltered()
	 fg_info->filtered_rm  = bq27426_read_word(BQ27426_COMMAND_REM_CAP_FIL);  // RemainingCapacityFiltered()
	 fg_info->filtered_fcc = bq27426_read_word(BQ27426_COMMAND_FULL_CAP_FIL); // FullChargeCapacityFiltered()

	 fg_info->qmax          = bq27426_read_word(BQ27426_COMMAND_QMAX);
	 fg_info->dod0          = bq27426_read_word(BQ27426_COMMAND_DOD0);
	 fg_info->dodateoc      = bq27426_read_word(BQ27426_COMMAND_DODATEOC);
	 fg_info->paresent_dod  = bq27426_read_word(BQ27426_COMMAND_PRESENT_DOD);

	 fg_info->true_soc      = bq27426_soc(UNFILTERED);// bq27426_read_word(BQ27426_COMMAND_SOC_UNFL);// StateOfChargeUnfiltered()
	 fg_info->soc           = bq27426_soc(FILTERED);//bq27426_read_word(BQ27426_COMMAND_SOC);
	 fg_info->soh           = bq27426_soh(PERCENT); // Read state-of-health (%)
	 fg_info->bat_id        = bq_bat_type.id;
	 fg_info->cycle = 0; // charge-discharge cycle times
#endif

	static uint8_t sta=0;
	i2cReadReturn = false;
	switch (sta)
	{
	case 0:
		sta ++;
		fg_info->voltage = bq27426_voltage();	 // Read battery voltage (mV)
		fg_info->current = bq27426_current(AVG); // Read average current (mA)
		fg_info->temperature = bq27426_temperature(BATTERY);		  //(℃)
		fg_info->pcbtemperature = bq27426_temperature(INTERNAL_TEMP); //(℃)
		break;
	case 1:
		sta ++;
		fg_info->true_soc = bq27426_soc(UNFILTERED); // bq27426_read_word(BQ27426_COMMAND_SOC_UNFL);// StateOfChargeUnfiltered()
		fg_info->soc = bq27426_soc(FILTERED); // bq27426_read_word(BQ27426_COMMAND_SOC);
		fg_info->soh = bq27426_soh(PERCENT); // Read state-of-health (%)
		fg_info->remap_soc = bat_soc_remappig(fg_info->soc);
		break;
	case 2:
		sta ++;
		fg_info->bat_id = bq_bat_type.id;
		fg_info->cycle = 0; // charge-discharge cycle times
		fg_info->taper_volt = \
				((uint16_t)(fg_bqfs.block_data_buf[7].blockdata[8]<<8)|(fg_bqfs.block_data_buf[7].blockdata[9])); // 109,8
		//bq27426_get_taper_voltage(&fg_info->taper_volt);
		fg_info->ctrl_status = bq27426_read_control_word(BQ27426_CONTROL_STATUS); // control status()
		fg_info->flags = bq27426_read_word(BQ27426_COMMAND_FLAGS);				  // flags()
		break;
	case 3:
		sta ++;
		fg_info->power = bq27426_power();	 // Read average power draw (mW)
		fg_info->capacityFull = bq27426_capacity(FULL);		// Read full capacity (mAh)
		fg_info->capacityRemain = bq27426_capacity(REMAIN); // Read remaining capacity (mAh)
		fg_info->passed_charge = bq27426_read_word(BQ27426_COMMAND_PASSED_CHARGE);
		break;
	case 4:
		sta ++;
		fg_info->ture_rm = bq27426_read_word(BQ27426_COMMAND_REM_CAP_UNFL);	  // RemainingCapacityUnfiltered()
		fg_info->ture_fcc = bq27426_read_word(BQ27426_COMMAND_FULL_CAP_UNFL); // FullChargeCapacityUnfiltered()
		fg_info->filtered_rm = bq27426_read_word(BQ27426_COMMAND_REM_CAP_FIL);	 // RemainingCapacityFiltered()
		fg_info->filtered_fcc = bq27426_read_word(BQ27426_COMMAND_FULL_CAP_FIL); // FullChargeCapacityFiltered()
		break;
	case 5:
		sta = 0;
		fg_info->qmax = bq27426_read_word(BQ27426_COMMAND_QMAX);
		fg_info->dod0 = bq27426_read_word(BQ27426_COMMAND_DOD0);
		fg_info->dodateoc = bq27426_read_word(BQ27426_COMMAND_DODATEOC);
		fg_info->paresent_dod = bq27426_read_word(BQ27426_COMMAND_PRESENT_DOD);
		break;
	default:
		sta = 0;
		break;
	}
	
	return i2cReadReturn;
}
/****************************************************************************************
 * @brief   读取flash数据与本地缓存比较，不同则返回false
 * @param   none
 * @return  true-数据相同/false-数据不同
 * @note
 *****************************************************************************************/
static int8_t bq27426_flash_data_update_check(void)
{
	uint16_t k = 0;
	FuelGauge_Bqfs_t fg_bqfsTmp;

	if (false == bq27426_flash_read(BQ27426_DATA_FLASH_ADDR, (uint8_t *)&fg_bqfsTmp, sizeof(fg_bqfsTmp))) {
		return false; //读数据失败
	}

	for (uint16_t i=0; i<BQ27426_GMFS_BLOCK_NUM; i++) {
		if (fg_bqfs.block_data_buf[i].checksum != fg_bqfsTmp.block_data_buf[i].checksum) { // 数据不同
			k++;
			BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"%d:0x%02x,0x%02x\r\n", i,fg_bqfs.block_data_buf[i].checksum,fg_bqfsTmp.block_data_buf[i].checksum);
			break;
		}
		for(uint16_t j=0; j<32; j++) { // The blockdata buffer lenth is 32.
			if (fg_bqfs.block_data_buf[i].blockdata[j] != fg_bqfsTmp.block_data_buf[i].blockdata[j]) { // 数据不同
				k++;
				break;
			}
		}
	}

	if (k > 0) {
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"fg_bqfs data is not same!\r\n");
		return false;
	}

	return true;
}

/****************************************************************************************
 * @brief   保存blockdata缓存到本地flash 
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_save_blockdata_to_flash(void)
{
	fg_bqfs.ver = (uint16_t)BQ27426_DATA_FLASH_OK;
	fg_bqfs.batType = bq_bat_type.id;
	fg_bqfs.reserve = 0xff;

	if (false == bq27426_flash_write(BQ27426_DATA_FLASH_ADDR, (uint8_t*)&fg_bqfs, sizeof(fg_bqfs))) // 如果写失败
	{
		fg_bqfs.ver = 0x0000;
		fg_bqfs.batType = 0x00;
		fg_bqfs.reserve = 0x00;
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"flash data write err0!\n");
		return false;
	}
	
	if(false == bq27426_flash_data_update_check()) //数据不相同
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"flash data write err1!\n");
		return false;
	}

	return true;
}

/****************************************************************************************
 * @brief   读取本地flash的数据到blockdata缓存
 * @param   none
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_refresh_blockdata_from_flash(void)
{
	//TODO 从flash中读取block_data_buf[16]
	if (false == bq27426_flash_read(BQ27426_DATA_FLASH_ADDR, (uint8_t *)&fg_bqfs, sizeof(fg_bqfs))) // 如果读失败
	{
		return false;
	}

	return true;
}


/****************************************************************************************
 * @brief   读取gmfs文件数据到blockdata缓存
 * @param   gmfs_image   gmfs数据初始地址
 * @return  true /false
 * @note
 *****************************************************************************************/
static int8_t bq27426_refresh_data_from_gmfs_image(const unsigned char *gmfs_image)
{
    uint16_t index = 34;
    uint8_t i = 0;
    uint8_t j = 0;
//TODO  
    for (i = 0; i < BQ27426_GMFS_BLOCK_NUM; i++)
    {
		fg_bqfs.block_data_buf[i].num = 0xFF;
		fg_bqfs.block_data_buf[i].classid = *(gmfs_image + index + 4);
		fg_bqfs.block_data_buf[i].offset = *(gmfs_image + index + 5);
        for (j = 0; j < 32; j++)
        {
            fg_bqfs.block_data_buf[i].blockdata[j] = *(gmfs_image + index + 10 + j);
        }
        fg_bqfs.block_data_buf[i].checksum = *(gmfs_image + index + 46);
        index += 62; // 6+36+5+4+6+5
    }
    return true;
}
/****************************************************************************************
 * @brief   flash数据是否有数据
 * @param   none
 * @return  true/false
 * @note    确认flash的gmfs数据，正确返回true,否则返回false
 *****************************************************************************************/
static int8_t bq27426_flash_data_check(void)
{
	//for(uint8_t i=0;i<3;i++) /* flash数据读取已做校验，取消多次读取判断 */
	{
		if(true == bq27426_flash_read(BQ27426_DATA_FLASH_ADDR, (uint8_t *)&fg_bqfs, sizeof(fg_bqfs)))
		{
			if ((fg_bqfs.ver == (uint16_t)BQ27426_DATA_FLASH_OK) &&
				(fg_bqfs.batType == bq_bat_type.id))
			{
				//BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"Flash gmfs check ok!\n");
				return true;
			}
		}
	}
	BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"Flash gmfs check ng!\n");
	return false;
}
/****************************************************************************************
 * @brief   BQ27426初始化
 * @param   none
 * @return  true/false
 * @note    flash有数据，则用flash数据初始化；
 * 			flash无数据，则用gmfs文件初始化，然后保存数据到flash
 *****************************************************************************************/
static int8_t bq27426_gmfs_update(void)
{
	int8_t flag = 0;

	if(true == bq27426_flash_data_check()) //flash有gmfs数据
	{
		if(true ==bq27426_refresh_blockdata_from_flash()) //从flash中加载数据 成功
		{
			flag = 1;
		}
	}
	if(flag == 0)
	{
		if(true == bq27426_refresh_data_from_gmfs_image(bq_bat_type.gmfs)) //从gmfs数组中获取数据
		{
			flag = 2;
		}
	}
	if(flag == 0) //gmfs数据错误 初始化失败
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 init data err.\n");
		return false;
	}
	else if(flag==1)
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 init data from mcu flash.\n");
	}
	else if(flag==2)
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 init data from gmfs file.\n");
	}

	if (false == bq27426_writeBlockData()) // 写入cfg数据
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 init fail!\n");
		return false;
	}
	bq27426_execute_control_word(BQ27426_CONTROL_SOFT_RESET); // add：soft reset

	if(flag == 2) //从gmfs文件更新后，保存数据到flash
	{
		if(false == bq27426_save_blockdata_to_flash()) {
			BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"gmfs data save fail!\n"); //保存失败
			return false;
		} else {
			BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"gmfs data save ok!\n");
		}
	}

	BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 init ok:%d\n", fg_bqfs.ver);
	return true;
}

static int8_t bq27426_init_flag; //初始化失败标志
/****************************************************************************************
 * @brief   fuel gauge 初始化成功、失败状态
 * @param   none
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_get_init_status(void)
{
	return bq27426_init_flag;
}

bool smoothSyncFlg; // set smooth sync cmd
/****************************************************************************************
 * @brief   fuel gauge smooth sync cmd
 * @param   none
 * @return  true/false
 * @note
 *****************************************************************************************/
void bq27426_set_smooth_sync(bool flg)
{
	smoothSyncFlg = flg;
}

/****************************************************************************************
 * @brief   fuel gauge init
 * @param   mode 模式选择
 * 				0-正常模式
 * 				1-强制初始化
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_init(uint8_t mode) 
{
	uint16_t deviceID;
	uint16_t flags;

	//检查IC是否需要初始化
 	for (uint8_t i = 0; i < 5; i++) {
		//bq27426_gpout_pin_exit_shutdown(); bq27426_mDelay(10); // GPOUT输出激活脉冲(已提前进行)
 		if (true != bq27426_init_flag) {
 			bq27426_bat_type(&bq_bat_type); // 获取电芯类型
 		}
		deviceID = bq27426_deviceType(); // Read deviceType from BQ27426
		if (deviceID == BQ27426_DEVICE_ID) {
			flags = bq27426_read_word(BQ27426_COMMAND_FLAGS);
			BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 id:0x0%02x,flags:0x%x\n", deviceID, flags);
			flags &= BQ27426_FLAG_ITPOR;
			if((mode==0)&&(flags==0)&&(true == bq27426_flash_data_check())) //ITPOR==0 已经初始化， 且flash数据OK 返回   // if(flags==0) //ITPOR==0 已经初始化，返回
			{
				if (smoothSyncFlg == TRUE) {
					smoothSyncFlg = FALSE;
					bq27426_unseal();
					bq27426_execute_control_word(BQ27426_CONTROL_SMOOTH_SYNC);
					BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"smooth sync!\n");
				}
				BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"already init!\n", deviceID);
				//bq27426_execute_control_word(BQ27426_CONTROL_SOFT_RESET); // add：soft reset
				//bq27426_set_opconfig(0,1);
				bq27426_init_flag = true;
				return true;
			}
			else //ITPOR==1  开始初始化
			{
				BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"start init!\n", deviceID);
				if(true == bq27426_gmfs_update()) //初始化
				{
					bq27426_mDelay(10);
					bq27426_init_flag = true;
					return true;
				}
				else
				{
					mode = 1;
					bq27426_wdog_feed();
					bq27426_mDelay(200);
					bq27426_wdog_feed();
				}
			}
			break;
		}
		bq27426_gpout_pin_exit_shutdown(); // GPOUT输出激活脉冲
	}
	BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"init err!\n");
	bq27426_init_flag = false; //初始化失败
	
	return false;
}

/****************************************************************************************
 * @brief   fuel gauge deinit
 * @param   none
 * @return  true/false
 * @note
 *****************************************************************************************/
int8_t bq27426_deinit(void) 
{
	//休眠之前，SEALED 加锁
	//bq27426_read_control_word(BQ27426_CONTROL_SEALED);
	//TODO GPOUT引脚休眠配置
	bq27426_gpout_pin_enter_sleep();

	return true;
}

/****************************************************************************************
 * @brief   将芯片内部配置参数保存到本地
 * @param   none
 * @return  true/false
 * @note    系统达到特定条件后调用保存电量计数据
 *****************************************************************************************/
int8_t bq27426_save_gmfs(void)
{
	//从IIC设备读取数据
	//检测本地是否有数据，无数据直接写入本地
	//本地有数据，对比数据是否有差异，有差异则更新本地数据
	//TODO 
	if(false == bq27426_get_init_status()) //初始化失败，不保存电量计数据
	{
		return true;
	}
	if(false == bq27426_readBlockData())  //从ic读取数据
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 read err.\n");
		return false;
	}
	
	if ((fg_bqfs.ver == (uint16_t)BQ27426_DATA_FLASH_OK) &&
		(fg_bqfs.batType == bq_bat_type.id))
	{
		if (true == bq27426_flash_data_update_check())//数据相同，不需要保存
		{
			BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 flash data is same as buf.\n");
			return true;
		}
	}
	if(false == bq27426_save_blockdata_to_flash())//保存到flash
	{
		BQ27426_LOG(BQ27426_LOG_LEVEL_ERR,"bq27426 save err.\n");
		return false;
	}
	BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 save ok.\n");
	return true;  //保存成功
}

/****************************************************************************************
 * @brief   本地保存数据清除
 * @param   none
 * @return  true/false
 * @note    
 *****************************************************************************************/
int8_t bq27426_flash_data_clear(void)
{
//	if(true == bq27426_flash_erase(BQ27426_FLASH_ADDR_START, sizeof(fg_bqfs)))
//	{
//		return true;
//	}
	return false;
}


/****************************************************************************************
 * @brief   某一时段消耗电量统计
 * @param   none
 * @return  电量值   正数-放电 负数-充电
 * @note    
 * 启动统计()  -  bq27426_passed_mah(0)；
 * 结束统计  -  bq27426_passed_mah(1)  并返回统计电量；
 * 需要成对调用
 *****************************************************************************************/
int16_t bq27426_passed_mah(uint8_t type)
{
	static uint16_t passed_mah;
	
	if(type==0) //初值
	{
		passed_mah = bq27426_passed_charge();
		return 0;
	}
	else
	{
		uint16_t passed_mah_end = (int16_t)bq27426_passed_charge();

		if(passed_mah<=passed_mah_end)
		{
			if(passed_mah_end - passed_mah >30000)  //end-65535  sta-0    0->65535 下降
			{
				return (0-((65535-passed_mah_end)+passed_mah));
			}
			else     //上升   0->100
			{
				return (int16_t)(passed_mah_end-passed_mah);
			}
		}
		else
		{
			if(passed_mah - passed_mah_end >30000)  //sta-65535  end-0   65535->0 上升
			{
				return (int16_t)((65535-passed_mah)+passed_mah_end);
			}
			else //下降  100->0
			{
				return (int16_t)(0-(passed_mah-passed_mah_end));
			}
		}
	}
}

/****************************************************************************************
 * @brief   打印缓存和flash内存信息
 * @param   none
 * @return
 * @note
 *****************************************************************************************/
void bq27426_print_gmfs(void)
{
	uint16_t i=0;
	uint16_t j=0;

	BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 gmfs ver:%d\r\n", fg_bqfs.ver);
	BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 gmfs bat:%d\r\n", fg_bqfs.batType);
	BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"bq27426 gmfs flash:\r\n");
	bq27426_mDelay(5);

	for( i=0; i<BQ27426_GMFS_BLOCK_NUM; i++) {
		for(j=0; j<BQ27426_GMFS_BLOCK_DATA_T_LEN; j++) {
			BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"0x%02x, ", *(((uint8_t *)&fg_bqfs.block_data_buf[i])+j));
			bq27426_mDelay(2);
		}
		BQ27426_LOG(BQ27426_LOG_LEVEL_INFO,"\r\n");
	}
}


