/**
  ******************************************************************************
  * @file    event_data_record.c
  * @author  Gison@metextech.com
  * @date    2024/08/21
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
  * 2024-08-21     V0.01      Gison@metextech.com    the first version
  *
  ******************************************************************************
  */
#include <stdlib.h>
#include "platform_io.h"
#include "sm_log.h"
#include "event_data_record.h"
#include "system_status.h"
#include "data_base_info.h"
#include "cy_serial_flash_qspi.h"
#include "FreeRTOS.h"

static ptIoDev msTickDev;

static fs_enent_index_u enevt_index_u, event_index_log;
static Event_u          event_record_u;

static uint32_t index_addr = EVENT_INDEX_ADDRESS; // enevt record index 8K 
//static uint8_t flag_cycled_event_storage = 0; //数据已经被写满,需要循环存储，先进先出FIFO,总是擦除最前边的4K数据
static uint8_t flag_event_get_next = 0;
static uint8_t flag_event_log_next = 0;
static uint32_t tick_event_get_next = 0;
static volatile uint32_t recordCnt; //bug1992745
static SemaphoreHandle_t xSemaphore_event = NULL;
/*============= =========== ======== ===== ==================*/
void event_init_counter()						// 读取最后一次写入记录的Cnt，后续的Cnt在此基础上++；
{
    Event_u event_read_u;
	uint32_t address;

	address = enevt_index_u.index_t.writeAddr;
	address -= sizeof(Event_t);

	if (address < EVENT_INDEX_ADDRESS)
	{
		address = EVENT_DATA_ADDRESS_END - sizeof(Event_t);
	}

    cy_serial_flash_qspi_read(address,  sizeof(Event_t),    event_read_u.res);
    if(event_read_u.record_t.magic == EVENT_MAGIC_DATA)
    {
		recordCnt = event_read_u.record_t.rec_data.count;
    }
	else
	{
        recordCnt = 0;
    }	
}

void event_record_init()									// 主要定位读写位置
{
	if (NULL == xSemaphore_event)
	{
		xSemaphore_event = xSemaphoreCreateMutex();
		xSemaphoreGive(xSemaphore_event); 
	}

    //检索 索引表 12*1024*1024 起始地址，Range 4K Byte 
    cy_rslt_t result = CY_RSLT_SUCCESS;
    index_addr = EVENT_INDEX_ADDRESS;
    fs_enent_index_u  fs_read_u;
    Event_u full_check_u;

    recordCnt = 0;
    msTickDev = io_dev_get_dev(DEV_MS_TICK);//SYS TICK 
    result = cy_serial_flash_qspi_read(index_addr,sizeof(fs_event_index_t),fs_read_u.res);
    //判断是否是第一次上电
    if(fs_read_u.index_t.magic != 0xAA55)
    {
        index_addr = EVENT_INDEX_ADDRESS;
        enevt_index_u.index_t.magic     = 0xAA55;
        enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;
        enevt_index_u.index_t.writeAddr = EVENT_DATA_ADDRESS_STAR;
        event_index_total_sync();
        //擦除4K
        // sm_log(SM_LOG_INFO, "spi flash erase from:%X,len :%X ...\r\n",EVENT_DATA_ADDRESS_STAR, EVENT_INDEX_LEN);
        cy_serial_flash_qspi_erase(index_addr,  EVENT_INDEX_LEN);//改为只擦除 4K 
        // sm_log(SM_LOG_INFO, "spi flash erase done !\r\n");
        event_index_insert();
        return ;
    }

    //遍历整个index区（4K Byte）因为可能会有有效数据存在！ 其实这里不需要不断叠加到4KB 空间吧？ 一个16字节空间，不断更新就好了！后续再优化---vincent.he20241205
    while((index_addr < EVENT_INDEX_ADDRESS + EVENT_INDEX_LEN - sizeof(fs_event_index_t)) && fs_read_u.index_t.magic == 0xAA55)
    {
        index_addr += sizeof(fs_event_index_t);//EVENT_DATA_ADDRESS_STAR 偏移++
        result = cy_serial_flash_qspi_read(index_addr, sizeof(fs_event_index_t), fs_read_u.res);//读取16个字节
        
    }
    //回退一条读取,遍历全部record
    if(index_addr > EVENT_INDEX_ADDRESS )
    {
       result =  cy_serial_flash_qspi_read(index_addr - sizeof(fs_event_index_t), sizeof(fs_event_index_t), enevt_index_u.res);//正式读取到 index共用体
        if(enevt_index_u.index_t.magic == 0xAA55)
        {
           sm_log(SM_LOG_INFO, "index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
                   index_addr,\
                   enevt_index_u.index_t.magic,\
                   enevt_index_u.index_t.readAddr,\
                   enevt_index_u.index_t.writeAddr,\
                   enevt_index_u.index_t.total); 
        }
    }

	if (enevt_index_u.index_t.cnt)				// 新版本协议，若cnt不为零，则是新协议数据，使用该Cnt
	{
		recordCnt = enevt_index_u.index_t.cnt;
	}
	else
	{
		event_init_counter();					// 旧版本协议，使用Log的Cnt
	}
}

// 插入一条index记录，保存至Flash
static int enevt_index_erase(void)			
{
    index_addr = EVENT_INDEX_ADDRESS;//索引需要记录到FLASH中，每次重启需要恢复！
    cy_serial_flash_qspi_erase(index_addr, EVENT_INDEX_LEN);//擦除 4 K index区
    event_index_insert();//插入第一条
	return 0;
}

// 插入一条enevt记录，其中会更新index区 ，保存至Flash
int event_index_insert(void)			
{
    if (index_addr > EVENT_INDEX_ADDRESS + EVENT_INDEX_LEN - sizeof(fs_event_index_t))// 空间不够，先擦除，再递归调用写
    {
        // sm_log(SM_LOG_INFO, "event log index space full, erase 4K Byte fron addr 0X%X!\r\n",EVENT_DATA_ADDRESS_STAR);
        enevt_index_erase();//直接擦除index 4K
        return 0;
    }
    enevt_index_u.index_t.magic = 0xAA55;//这里算AA55作为有效数据
    enevt_index_u.index_t.cnt = recordCnt;
    cy_serial_flash_qspi_write(index_addr, sizeof(fs_event_index_t), enevt_index_u.res);//这里会保存在FLASH中
    // sm_log(SM_LOG_INFO, "event_index_insert write :addr %X, write: %d byte ok !\r\n",index_addr,sizeof(fs_event_index_t));
    index_addr += sizeof(fs_event_index_t); //为什么需要偏移这个地址呢？不是固定的吗？后续优化，commit by vincent.he
	return 0;
}

// 更新 Event 条目数 
uint16_t event_index_total_sync(void)
{
    uint16_t actual_quantity = 0;

    if(enevt_index_u.index_t.readAddr == enevt_index_u.index_t.writeAddr)
    {
        enevt_index_u.index_t.total = 0;//默认就是0条！ 
    } 
    else if(enevt_index_u.index_t.readAddr < enevt_index_u.index_t.writeAddr)
    {
        enevt_index_u.index_t.total = (enevt_index_u.index_t.writeAddr - enevt_index_u.index_t.readAddr)/sizeof(Event_t);
    } 
    else if(enevt_index_u.index_t.readAddr > enevt_index_u.index_t.writeAddr)
    {
        enevt_index_u.index_t.total = ((EVENT_DATA_ADDRESS_END - enevt_index_u.index_t.readAddr) + \
            (enevt_index_u.index_t.writeAddr - (EVENT_DATA_ADDRESS_STAR)))/sizeof(Event_t);
    }

    /**
     *  Examples: 
     *  actual_quantity: 16128
        index_addr:1E80830,magic: AA55 readAddr: 1E85000 , writeAddr: 1E84000 ,total: 16128
        event_index_actual_total: 16128 
     */
    // sm_log(SM_LOG_INFO, "index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
    //         index_addr,\
    //         enevt_index_u.index_t.magic,\
    //         enevt_index_u.index_t.readAddr,\
    //         enevt_index_u.index_t.writeAddr,\
    //         enevt_index_u.index_t.total); 

    actual_quantity = enevt_index_u.index_t.total;

    // sm_log(SM_LOG_INFO, "actual_quantity: %ld\r\n", actual_quantity);
 
    if (EVENT_LOG_MAX_NUM < enevt_index_u.index_t.total)
    {
        uint32_t offset =0;
        offset =  (enevt_index_u.index_t.total - EVENT_LOG_MAX_NUM)*sizeof(Event_t);
        enevt_index_u.index_t.readAddr += offset;
        if (enevt_index_u.index_t.readAddr > EVENT_DATA_ADDRESS_END)
        {
            enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR + (enevt_index_u.index_t.readAddr - EVENT_DATA_ADDRESS_END);
        }
        else if (enevt_index_u.index_t.readAddr == EVENT_DATA_ADDRESS_END)
        {
            enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;
        }
        /**
         * repeat calculate total number*/
        if(enevt_index_u.index_t.readAddr < enevt_index_u.index_t.writeAddr)
        {
            enevt_index_u.index_t.total = (enevt_index_u.index_t.writeAddr - enevt_index_u.index_t.readAddr)/sizeof(Event_t);
        } 
        else if(enevt_index_u.index_t.readAddr > enevt_index_u.index_t.writeAddr)
        {
            enevt_index_u.index_t.total = ((EVENT_DATA_ADDRESS_END - enevt_index_u.index_t.readAddr) + \
                (enevt_index_u.index_t.writeAddr - (EVENT_DATA_ADDRESS_STAR)))/sizeof(Event_t);
        }
    }
    
    // sm_log(SM_LOG_INFO, "index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
    //         index_addr,\
    //         enevt_index_u.index_t.magic,\
    //         enevt_index_u.index_t.readAddr,\
    //         enevt_index_u.index_t.writeAddr,\
    //         enevt_index_u.index_t.total); 
    
    return actual_quantity;
}

// 此函数实现 在get next 被调用后，1秒后执行更新event index
void event_record_sync_proc(void)
{
    if(flag_event_get_next)
    {
        if(msTickDev->read( (uint8_t*)&tick_event_get_next, 4) - tick_event_get_next >= 1000)//1  秒
        {
            flag_event_get_next = 0;
            event_index_insert();
            tick_event_get_next = msTickDev->read( (uint8_t*)&tick_event_get_next, 4);
        }
    }

	if (flag_event_log_next)		// 1秒无数据传输，中断操作
	{
		if(msTickDev->read( (uint8_t*)&tick_event_get_next, 4) - tick_event_get_next >= 5000)//5  秒
		{
			flag_event_log_next = 0;
		}
	}
}
uint32_t test_cnt = 0;
/*================================================================================================================*/
int event_record_insert(debug_service_event_log_char_t *sRecord)   // 插入一条记录，保存至Flash，具体实现中count要内部存储并++
{
	xSemaphoreTake(xSemaphore_event, portMAX_DELAY); 

    recordCnt++;
	sRecord->count = recordCnt;

    // 先写数据区，再先写index区 ，
    event_record_u.record_t.magic = EVENT_MAGIC_DATA;//0xffAA55FF
    memcpy(&event_record_u.record_t.rec_data, sRecord, sizeof(debug_service_event_log_char_t));
    memset(event_record_u.record_t.reserved, 0, sizeof(event_record_u.record_t.reserved));
	event_record_u.record_t._xor = 0x5a;
    //判断本次写入空间是否溢出
    if(enevt_index_u.index_t.writeAddr == (EVENT_DATA_ADDRESS_END))
    {
        enevt_index_u.index_t.writeAddr = EVENT_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
        // flag_cycled_record_storage = 1;
    }
    //判断数据是否被写满过(512K - 4K)
    if(enevt_index_u.index_t.writeAddr % 4096 == 0)
    {
        // if(flag_cycled_record_storage == 1)//数据是写满翻头，起始读取地址 上移至下一个4K地址
        {
            // if(enevt_index_u.index_t.readAddr >=enevt_index_u.index_t.writeAddr && \
            //    enevt_index_u.index_t.readAddr <= enevt_index_u.index_t.writeAddr + 4096) // 判断擦除是否影响 readAddr  为什么要有这个判断？readAddr是读地址！
               
            if ((enevt_index_u.index_t.readAddr > enevt_index_u.index_t.writeAddr)&&(enevt_index_u.index_t.readAddr < (enevt_index_u.index_t.writeAddr+4096)))
            {
                enevt_index_u.index_t.readAddr = enevt_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
                if (enevt_index_u.index_t.readAddr == (EVENT_DATA_ADDRESS_END))
                {
                    enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
                }
            }
            else if ((enevt_index_u.index_t.total == (EVENT_LOG_MAX_NUM))&&(enevt_index_u.index_t.readAddr == enevt_index_u.index_t.writeAddr))
            {
                enevt_index_u.index_t.readAddr = enevt_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
                if (enevt_index_u.index_t.readAddr == (EVENT_DATA_ADDRESS_END))
                {
                    enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
                }
            }
        }

		if (flag_event_log_next)		// 如果Log正在传输，则需要更新读地址
		{								// 除写地址使用 enevt_index_u外，其余为Buf
			if ((event_index_log.index_t.readAddr > enevt_index_u.index_t.writeAddr)&&(event_index_log.index_t.readAddr < (enevt_index_u.index_t.writeAddr+4096)))
			{
				event_index_log.index_t.readAddr = enevt_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
				if (event_index_log.index_t.readAddr == (EVENT_DATA_ADDRESS_END))
				{
					event_index_log.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
				}
			}
			else if ((event_index_log.index_t.total == (EVENT_LOG_MAX_NUM))&&(event_index_log.index_t.readAddr == enevt_index_u.index_t.writeAddr))
			{
				event_index_log.index_t.readAddr = enevt_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
				if (event_index_log.index_t.readAddr == (EVENT_DATA_ADDRESS_END))
				{
					event_index_log.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
				}
			}
		}
		
        sm_log(SM_LOG_INFO, "event_index_actual_total: %ld \r\n", event_index_total_sync()); //currently must be keep! comment by vincent.he.
        sm_log(SM_LOG_INFO, "event log record insert trig erase %d Byte fron addr 0X%X!------------\r\n", EVENT_INDEX_LEN, enevt_index_u.index_t.writeAddr);
        cy_serial_flash_qspi_erase(enevt_index_u.index_t.writeAddr, 4096);//擦除 4 K event log record 将要写入的新扇区
    }
    cy_serial_flash_qspi_write(enevt_index_u.index_t.writeAddr, sizeof(Event_t), event_record_u.res);
    // index 参数更新,总条数更新
    enevt_index_u.index_t.writeAddr += sizeof(Event_t);//写 影响写地址，有时也会响读地址，比如写满翻头
     
    event_index_total_sync();

    //如果数据写满 翻头，需要处理读取地址 还未实现
    event_index_insert();

    // sm_log(SM_LOG_INFO, "index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
    //         index_addr,\
    //         enevt_index_u.index_t.magic,\
    //         enevt_index_u.index_t.readAddr,\
    //         enevt_index_u.index_t.writeAddr,\
    //         enevt_index_u.index_t.total); 


	xSemaphoreGive(xSemaphore_event); 
	
    return 0;  
}

int event_record_get(debug_service_event_log_char_t *sRecord) // 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
{
	xSemaphoreTake(xSemaphore_event, portMAX_DELAY); 
    int res = 0;
    Event_u event_read_u;
    cy_serial_flash_qspi_read(enevt_index_u.index_t.readAddr,sizeof(Event_t),event_read_u.res);
    if(event_read_u.record_t.magic == EVENT_MAGIC_DATA)
    {
        memcpy(sRecord,&event_read_u.record_t.rec_data,sizeof(debug_service_event_log_char_t));
        res =  0;
    }else{
        res = -1;
    }
	xSemaphoreGive(xSemaphore_event); 
	return res;
}

int event_record_get_next(debug_service_event_log_char_t *sRecord) // 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码
{

    int res = -1;
    /*这里依然做了偏移4K 的空间限制*/
     if(enevt_index_u.index_t.readAddr < EVENT_DATA_ADDRESS_STAR || \
        enevt_index_u.index_t.readAddr > (EVENT_DATA_ADDRESS_END - sizeof(Event_t)))
    {
        sm_log(SM_LOG_ERR, "event_record_get_next:addr %X,Address out-of-bounds!\r\n",enevt_index_u.index_t.readAddr,sizeof(Event_t));
        res = -1;
        return res;
    }

	xSemaphoreTake(xSemaphore_event, portMAX_DELAY); 
    //update total
    event_index_total_sync();//更新剩余多少条数！

    if(enevt_index_u.index_t.total > 0)//如果整体total 条数大于0， 那就get next
    {
        cy_serial_flash_qspi_read(enevt_index_u.index_t.readAddr,sizeof(Event_t),event_record_u.res);
        if(event_record_u.record_t.magic == EVENT_MAGIC_DATA)//AA55 is valid
        {
            if(flag_event_get_next == 0)
            {
                flag_event_get_next = 1;
            }
            tick_event_get_next = msTickDev->read( (uint8_t*)&tick_event_get_next, 4);// 更新时基
            enevt_index_u.index_t.readAddr += sizeof(Event_t);
            if(enevt_index_u.index_t.readAddr >= EVENT_DATA_ADDRESS_END)
            {
                enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;//这里仍然是偏移了4K!
            }
            event_index_total_sync();//更新剩余多少条数！
            memcpy(sRecord, &event_record_u.record_t.rec_data, sizeof(debug_service_event_log_char_t));
            // sm_log(SM_LOG_INFO, "read event log record:r_addr %X,w_addr %X, len %d byte ,total:%d!\r\n",\
            //             enevt_index_u.index_t.readAddr,enevt_index_u.index_t.writeAddr,sizeof(Event_t),enevt_index_u.index_t.total);
            res = 0;
        }
        else
        {//无效！
            tick_event_get_next = msTickDev->read( (uint8_t*)&tick_event_get_next, 4);// 更新时基
            enevt_index_u.index_t.readAddr += sizeof(Event_t);// 如果单条 数据错误，也需要跳过此条，避免读不到后边的数据
            event_index_total_sync();
            sm_log(SM_LOG_INFO, "read event log record:addr %X, %d byte magic err !\r\n",enevt_index_u.index_t.readAddr,sizeof(Event_t));
            res = -1;
        }
    }
    else
    {
            sm_log(SM_LOG_INFO, "read event log record:addr %X, %d byte, total = 0 err !\r\n",enevt_index_u.index_t.readAddr,sizeof(Event_t));
    }
	xSemaphoreGive(xSemaphore_event); 
	return res;
}

int event_record_total_get(void) // 返回数据记录总数
{
    event_index_total_sync();//更新剩余多少条数！

    return enevt_index_u.index_t.total;
}

int event_record_erase_all(bool bCnt) // 清除所有event数据记录
{
	xSemaphoreTake(xSemaphore_event, portMAX_DELAY); 

    //如果为了快速处理，不一定需要这样处理。只要把读写地址设置为相同即可，total = 0, 后面边写边擦除！comment by vincent.he, 20241205
    //擦除4K
    sm_log(SM_LOG_INFO, "spi flash event logs erase from:%X,len :%X ...\r\n", EVENT_INDEX_ADDRESS, EVENT_INDEX_LEN);
    cy_serial_flash_qspi_erase(EVENT_INDEX_ADDRESS,  EVENT_INDEX_LEN);//改为只擦除 4K ，初始化更快
    // cy_serial_flash_qspi_erase(EVENT_DATA_ADDRESS_END - 4096,4096);//擦除末尾4K
    sm_log(SM_LOG_INFO, "spi flash event erase done !\r\n");

	if (bCnt)
	{
		recordCnt = 0;
	}

	index_addr = EVENT_INDEX_ADDRESS;		// 必须初始化地址
	enevt_index_u.index_t.magic = 0xAA55;//这里算AA55作为有效数据
	enevt_index_u.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;
	enevt_index_u.index_t.writeAddr = EVENT_DATA_ADDRESS_STAR;
	enevt_index_u.index_t.total = 0;

	event_index_insert();	// 更新Cnt后回写

//    flag_cycled_event_storage = 0;
    event_record_init();

	if (flag_event_log_next)
	{
		event_index_log = enevt_index_u;	// 更新Index，后续读的时候会终止
	}

	xSemaphoreGive(xSemaphore_event); 
	return 0;

}

/*============= =========== ======== ===== ==================*/
int event_record_generate(EVENT_CODE_Enum code, uint8_t *optionalData, uint8_t len)
{
	uint32_t ts;
	debug_service_event_log_char_t record;
	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->read((uint8_t *)&ts, 4);

	memset(&(record.event_data), 0, 15);
	record.timestamp = ts;
	record.event_code = code;
	memcpy(&(record.event_data), optionalData, len);

#if 0
	sm_log(SM_LOG_INFO, "event code =  %d\r\n", code);
	if (len)
	{
		sm_log(SM_LOG_INFO, "event data =  ");
		while (len--)
		{
			sm_log(SM_LOG_INFO, "%02x ", *optionalData);
			optionalData++;
		}
		sm_log(SM_LOG_INFO, "\r\n", code);
	}
#endif

	return event_record_insert(&record);
}
/*============= =========== ======== ===== ==================*/

// 更新 Event 条目数 
uint16_t event_log_index_total_sync(void)
{
    uint16_t actual_quantity = 0;				// enevt_index_u.index_t.writeAddr 不能使用Bug值，有可能会更新

    if(event_index_log.index_t.readAddr == enevt_index_u.index_t.writeAddr)
    {
        event_index_log.index_t.total = 0;//默认就是0条！ 
    } 
    else if(event_index_log.index_t.readAddr < enevt_index_u.index_t.writeAddr)
    {
        event_index_log.index_t.total = (enevt_index_u.index_t.writeAddr - event_index_log.index_t.readAddr)/sizeof(Event_t);
    } 
    else if(event_index_log.index_t.readAddr > enevt_index_u.index_t.writeAddr)
    {
        event_index_log.index_t.total = ((EVENT_DATA_ADDRESS_END - event_index_log.index_t.readAddr) + \
            (enevt_index_u.index_t.writeAddr - (EVENT_DATA_ADDRESS_STAR)))/sizeof(Event_t);
    }

    actual_quantity = event_index_log.index_t.total;

    if (EVENT_LOG_MAX_NUM < event_index_log.index_t.total)
    {
        uint32_t offset =0;
        offset =  (event_index_log.index_t.total - EVENT_LOG_MAX_NUM)*sizeof(Event_t);
        event_index_log.index_t.readAddr += offset;
        if (event_index_log.index_t.readAddr > EVENT_DATA_ADDRESS_END)
        {
            event_index_log.index_t.readAddr = EVENT_DATA_ADDRESS_STAR + (event_index_log.index_t.readAddr - EVENT_DATA_ADDRESS_END);
        }
        else if (event_index_log.index_t.readAddr == EVENT_DATA_ADDRESS_END)
        {
            event_index_log.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;
        }
        /**
         * repeat calculate total number*/
        if(event_index_log.index_t.readAddr < enevt_index_u.index_t.writeAddr)
        {
            event_index_log.index_t.total = (enevt_index_u.index_t.writeAddr - event_index_log.index_t.readAddr)/sizeof(Event_t);
        } 
        else if(event_index_log.index_t.readAddr > enevt_index_u.index_t.writeAddr)
        {
            event_index_log.index_t.total = ((EVENT_DATA_ADDRESS_END - event_index_log.index_t.readAddr) + \
                (enevt_index_u.index_t.writeAddr - (EVENT_DATA_ADDRESS_STAR)))/sizeof(Event_t);
        }
    }
	
    return actual_quantity;
}
/*============= =========== ======== ===== ==================*/
bool event_log_get(debug_service_event_log_char_t *sRecord) // 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
{
    // sm_log(SM_LOG_INFO, "%s\r\n", __FUNCTION__);
	xSemaphoreTake(xSemaphore_event, portMAX_DELAY); 
	event_index_log = enevt_index_u;
	xSemaphoreGive(xSemaphore_event); 
	
	flag_event_log_next = 1;
    bool res = event_log_get_next(sRecord);
	if (res)
	{
        res =  true;
//		tick_event_get_next = msTickDev->read( (uint8_t*)&tick_event_get_next, 4);// 更新时基
    }
	else
	{
		flag_event_log_next = 0;
        res = false;
    }

	return res;
}
/*============= =========== ======== ===== ==================*/
bool event_log_get_next(debug_service_event_log_char_t *sRecord) // 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码
{
    // sm_log(SM_LOG_INFO, "%s\r\n", __FUNCTION__);
    Event_u event_read_u;
    bool res = false;
    /*这里依然做了偏移4K 的空间限制*/
	if(event_index_log.index_t.readAddr < EVENT_DATA_ADDRESS_STAR || event_index_log.index_t.readAddr > (EVENT_DATA_ADDRESS_END - sizeof(Event_t)))
    {
		flag_event_log_next = 0;
        return false;
    }

	if (flag_event_log_next == 0)
	{
        return false;
    }

	xSemaphoreTake(xSemaphore_event, portMAX_DELAY); 

    event_log_index_total_sync();//更新剩余多少条数！
    

    if(event_index_log.index_t.total > 0)//如果整体total 条数大于0， 那就get next
    {
        cy_serial_flash_qspi_read(event_index_log.index_t.readAddr,sizeof(Event_t),event_read_u.res);
        if(event_read_u.record_t.magic == EVENT_MAGIC_DATA)//AA55 is valid
        {
			flag_event_log_next = 1;
            tick_event_get_next = msTickDev->read( (uint8_t*)&tick_event_get_next, 4);// 更新时基
            event_index_log.index_t.readAddr += sizeof(Event_t);
            if(event_index_log.index_t.readAddr >= EVENT_DATA_ADDRESS_END)
            {
                event_index_log.index_t.readAddr = EVENT_DATA_ADDRESS_STAR;//这里仍然是偏移了4K!
            }
            event_log_index_total_sync();//更新剩余多少条数！
            memcpy(sRecord, &event_read_u.record_t.rec_data, sizeof(debug_service_event_log_char_t));
            res = true;
			update_idl_delay_time();	// 延时休眠
        }
        else
        {
			flag_event_log_next = 0;
            res = false;
        }
    }  
	else
	{
		flag_event_log_next = 0;
        res = false;
    }
	xSemaphoreGive(xSemaphore_event); 
	return res;
}

/*============= =========== ======== ===== ==================*/
