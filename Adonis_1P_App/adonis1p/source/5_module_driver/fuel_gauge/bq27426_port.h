#ifndef ___BQ27426_PORT_H___
#define ___BQ27426_PORT_H___

#include <stdint.h>

#define  __1P__        (1)
#define  __2P_CASE__   (0)
#define  __2P_PEN__    (0)

//---------------------------------------------------------------------
#if __1P__
#include "driver_i2c.h"
#include "driver_detector.h"
#include "driver_mcu_eeprom.h"
#include "driver_wdg.h"

#define BQ27426_GMFS_VERSION       		(0x0004) //GMFS版本，修改需同步上位机读取版本信息内容

#define GPOUT_PIN                   	P9_3  
#define BQ27426_IIC_ADDR            	I2C_ADDR_BQ27426 //设备IIC地址
#define BQ27426_FLASH_ADDR_START    	5 // INDEX_D_2 //数据存储本地flash初地址 这里需要注意， 因为不能向上包含头文件，INDEX_D_2值改变时， BQ27426_FLASH_ADDR_START同步改变
#define BQ27426_PRINTF(fmt, ...)		sm_log(SM_LOG_DEBUG, fmt, ##__VA_ARGS__)
#endif
//---------------------------------------------------------------------

//---------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------
typedef enum{
    BQ27426_LOG_LEVEL_OFF,    
    BQ27426_LOG_LEVEL_ERR,         
    BQ27426_LOG_LEVEL_WARNING,     
    BQ27426_LOG_LEVEL_INFO,      
    BQ27426_LOG_LEVEL_DEBUG,    
}bq27426_log_level_t;

#define BQ27426_LOG_LEVEL   BQ27426_LOG_LEVEL_DEBUG  //打印等级

#if (BQ27426_LOG_LEVEL < BQ27426_LOG_LEVEL_DEBUG)
#define BQ27426_LOG_LINE()
#else
#define BQ27426_LOG_LINE() BQ27426_PRINTF("[%s:%d]", __func__, __LINE__) // 打印函数名和行号

#endif

#if 0
#define BQ27426_LOG(level, fmt, ...)    \
    {                                   \
        if (level <= BQ27426_LOG_LEVEL) \
        {                               \
                 if (level ==BQ27426_LOG_LEVEL_ERR)        {BQ27426_PRINTF("[ERR]");    BQ27426_LOG_LINE();BQ27426_PRINTF(fmt,##__VA_ARGS__);}\
            else if (level ==BQ27426_LOG_LEVEL_WARNING)    {BQ27426_PRINTF("[WARNING]");BQ27426_LOG_LINE();BQ27426_PRINTF(fmt,##__VA_ARGS__);}\
            else if (level ==BQ27426_LOG_LEVEL_INFO)       {BQ27426_PRINTF("[INFO]");   BQ27426_LOG_LINE();BQ27426_PRINTF(fmt,##__VA_ARGS__);}\
            else if (level ==BQ27426_LOG_LEVEL_DEBUG)      {BQ27426_PRINTF("[DEBUG]");  BQ27426_LOG_LINE();BQ27426_PRINTF(fmt,##__VA_ARGS__);}\
        }                               \
    }

#define BQ27426_LOG_ERR(fmt, ...)       BQ27426_LOG(BQ27426_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__) 
#define BQ27426_LOG_WARNING(fmt, ...)   BQ27426_LOG(BQ27426_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__) 
#define BQ27426_LOG_INFO(fmt, ...)      BQ27426_LOG(BQ27426_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__) 
#define BQ27426_LOG_DEBUG(fmt, ...)     BQ27426_LOG(BQ27426_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__) 
#else
#define BQ27426_LOG(level, fmt, ...) 	BQ27426_PRINTF(fmt,##__VA_ARGS__)
#endif

//------------------------------------------------------------------------------------------------------------------------------------

typedef struct 
{
    uint8_t id;           //电芯型号
    const uint8_t *gmfs;  //电芯gmfs数据指针
}bq27426_bat_type_t;      //电芯类型

extern int8_t i2cReadReturn;
extern int8_t i2cWriteReturn;

void bq27426_wdog_feed(void);
void bq27426_mDelay(uint16_t ms);
void bq27426_gpout_pin_exit_shutdown(void);
void bq27426_gpout_pin_enter_sleep(void);

void bq27426_bat_type(bq27426_bat_type_t* bat_type);

bool bq27426_flash_read(uint32_t addr,uint8_t* databuf,uint16_t len);
bool bq27426_flash_erase(uint32_t addr,uint8_t* databuf,uint16_t len);
bool bq27426_flash_write(uint32_t addr,uint8_t* databuf,uint16_t len);

int8_t bq27426_read_bytes(uint8_t cmdAddress, uint8_t * rBuffer, uint8_t rLength);
int8_t bq27426_write_bytes(uint8_t cmdAddress, uint8_t * wBuffer, uint8_t wLength);

#endif

