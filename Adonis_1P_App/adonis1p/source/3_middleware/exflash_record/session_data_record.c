/**
  ******************************************************************************
  * @file    session_data_record.c
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
  * 2024-08-21     V0.01      Gison@metextech.com           the first version
  * 2024-09-06     V0.02      Eason.cen@smooretech.com      fixed some bugs
  *
  ******************************************************************************
  */
#include <stdlib.h>
#include "platform_io.h"
#include "sm_log.h"
#include "session_data_record.h"
#include "system_status.h"
#include "data_base_info.h"
#include "system_status.h"
#include "cy_serial_flash_qspi.h"

static ptIoDev msTickDev;

static fs_record_u record_index_u, record_index_log;
Session_record_u session_u;
static uint32_t session_index_addr = SESSION_INDEX_ADDRESS; // session record index 
//static uint8_t flag_cycled_record_storage = 0; //数据已经被写满,需要循环存储，先进先出FIFO,总是擦除最前边的4K数据
static uint8_t flag_get_next = 0;
static uint8_t flag_log_next = 0;
static uint32_t tick_get_next = 0;

static SemaphoreHandle_t xSemaphore_session = NULL;

/*============= =========== ======== ===== ==================*/
void session_record_init()									// 主要定位读写位置
{
	if (NULL == xSemaphore_session)
	{
	    xSemaphore_session = xSemaphoreCreateMutex();
		xSemaphoreGive(xSemaphore_session); 
	}

    //检索 索引表 12*1024*1024 起始地址，Range 4K Byte 
    cy_rslt_t result = CY_RSLT_SUCCESS;
    session_index_addr = SESSION_INDEX_ADDRESS;
    fs_record_u  fs_read_u;
    Session_record_u full_check_u;
    msTickDev = io_dev_get_dev(DEV_MS_TICK);//SYS TICK 
    result = cy_serial_flash_qspi_read(session_index_addr,sizeof(fs_record_t),fs_read_u.res);
    //判断是否是第一次上电
    if(fs_read_u.index_t.magic != 0xAA5A)
    {
        session_index_addr = SESSION_INDEX_ADDRESS;
        record_index_u.index_t.magic     = 0xAA5A;
        record_index_u.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;
        record_index_u.index_t.writeAddr = SESSION_DATA_ADDRESS_STAR;
        session_index_total_sync();
        //擦除4K
        // sm_log(SM_LOG_INFO, "spi flash erase from:%X,len :%X ...\r\n",SESSION_DATA_ADDRESS_STAR, SESSION_INDEX_LEN);
        cy_serial_flash_qspi_erase(session_index_addr,  SESSION_INDEX_LEN);//改为只擦除 4K 
        // sm_log(SM_LOG_INFO, "spi flash erase done !\r\n");
        session_index_insert();
        return ;
    }
    //遍历整个index区（4K Byte）因为可能会有有效数据存在！ 其实这里不需要不断叠加到4KB 空间吧？ 一个16字节空间，不断更新就好了！后续再优化---vincent.he20241205
    while((session_index_addr < SESSION_INDEX_ADDRESS + SESSION_INDEX_LEN - sizeof(fs_record_t)) && fs_read_u.index_t.magic == 0xAA5A)
    {
        session_index_addr += sizeof(fs_record_t);//SESSION_DATA_ADDRESS_STAR 偏移++
        result = cy_serial_flash_qspi_read(session_index_addr, sizeof(fs_record_t), fs_read_u.res);//读取16个字节
        
    }
    //回退一条读取,遍历全部record
    if(session_index_addr > SESSION_INDEX_ADDRESS )
    {
       result =  cy_serial_flash_qspi_read(session_index_addr - sizeof(fs_record_t), sizeof(fs_record_t), record_index_u.res);//正式读取到 index共用体
        if(record_index_u.index_t.magic == 0xAA5A)
        {
           sm_log(SM_LOG_INFO, "session_index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
                   session_index_addr,\
                   record_index_u.index_t.magic,\
                   record_index_u.index_t.readAddr,\
                   record_index_u.index_t.writeAddr,\
                   record_index_u.index_t.total); 
        }
    }
}

// 擦除index记录，保存至Flash
int session_index_erase(void)			
{
    session_index_addr = SESSION_INDEX_ADDRESS;//索引需要记录到FLASH中，每次重启需要恢复！
    cy_serial_flash_qspi_erase(session_index_addr, SESSION_INDEX_LEN);//擦除 4 K index区
    session_index_insert();//插入第一条
	return 0;
}

// 插入一条session记录，其中会更新index区 ，保存至Flash
int session_index_insert(void)			
{
    if (session_index_addr > SESSION_INDEX_ADDRESS + SESSION_INDEX_LEN - sizeof(fs_record_t))// 空间不够，先擦除，再递归调用写
    {
        // sm_log(SM_LOG_INFO, "session index space full, erase 4K Byte fron addr 0X%X!\r\n",SESSION_DATA_ADDRESS_STAR);
        session_index_erase();//直接擦除index 4K
        return 0;
    }
    record_index_u.index_t.magic = 0xAA5A;//这里算AA5A作为有效数据
    cy_serial_flash_qspi_write(session_index_addr, sizeof(fs_record_t), record_index_u.res);//这里会保存在FLASH中
    // sm_log(SM_LOG_INFO, "session_index_insert write :addr %X, write: %d byte ok !\r\n",session_index_addr,sizeof(fs_record_t));
    session_index_addr += sizeof(fs_record_t); //为什么需要偏移这个地址呢？不是固定的吗？后续优化，commit by vincent.he
	return 0;
}
// 更新 session 条目数 ，这里的处理逻辑是通过读写地址进行管理的吗？
uint16_t session_index_total_sync(void)
{
    uint16_t actual_quantity = 0;

    if(record_index_u.index_t.readAddr == record_index_u.index_t.writeAddr)
    {
        record_index_u.index_t.total = 0;//默认就是0条！ 
    } 
    else if(record_index_u.index_t.readAddr < record_index_u.index_t.writeAddr)
    {
        record_index_u.index_t.total = (record_index_u.index_t.writeAddr - record_index_u.index_t.readAddr)/sizeof(Session_t);
    } 
    else if(record_index_u.index_t.readAddr > record_index_u.index_t.writeAddr)
    {
        record_index_u.index_t.total = ((SESSION_DATA_ADDRESS_END - record_index_u.index_t.readAddr) + \
            (record_index_u.index_t.writeAddr - (SESSION_DATA_ADDRESS_STAR)))/sizeof(Session_t);
    }

    /**
     *  Examples: 
     *  actual_quantity: 16128
        session_index_addr:1E80830,magic: AA5A readAddr: 1E85000 , writeAddr: 1E84000 ,total: 16128
        session_index_actual_total: 16128 
     */
    // sm_log(SM_LOG_INFO, "session_index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
    //         session_index_addr,\
    //         record_index_u.index_t.magic,\
    //         record_index_u.index_t.readAddr,\
    //         record_index_u.index_t.writeAddr,\
    //         record_index_u.index_t.total); 

    actual_quantity = record_index_u.index_t.total;

    // sm_log(SM_LOG_INFO, "actual_quantity: %ld\r\n", actual_quantity);
 
    if (SESSION_RECORD_MAX_NUM < record_index_u.index_t.total)
    {
        uint32_t offset =0;
        offset =  (record_index_u.index_t.total - SESSION_RECORD_MAX_NUM)*sizeof(Session_t);
        record_index_u.index_t.readAddr += offset;
        if (record_index_u.index_t.readAddr > SESSION_DATA_ADDRESS_END)
        {
            record_index_u.index_t.readAddr = SESSION_DATA_ADDRESS_STAR + (record_index_u.index_t.readAddr - SESSION_DATA_ADDRESS_END);
        }
        else if (record_index_u.index_t.readAddr == SESSION_DATA_ADDRESS_END)
        {
            record_index_u.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;
        }
        /**
         * repeat calculate total number*/
        if(record_index_u.index_t.readAddr < record_index_u.index_t.writeAddr)
        {
            record_index_u.index_t.total = (record_index_u.index_t.writeAddr - record_index_u.index_t.readAddr)/sizeof(Session_t);
        } 
        else if(record_index_u.index_t.readAddr > record_index_u.index_t.writeAddr)
        {
            record_index_u.index_t.total = ((SESSION_DATA_ADDRESS_END - record_index_u.index_t.readAddr) + \
                (record_index_u.index_t.writeAddr - (SESSION_DATA_ADDRESS_STAR)))/sizeof(Session_t);
        }
    }
    
    // sm_log(SM_LOG_INFO, "session_index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
    //         session_index_addr,\
    //         record_index_u.index_t.magic,\
    //         record_index_u.index_t.readAddr,\
    //         record_index_u.index_t.writeAddr,\
    //         record_index_u.index_t.total); 
    
    return actual_quantity;
}
static uint32_t session_test_cnt = 0;
/*============= =========== ======== ===== ==================*/
int session_record_insert(session_service_records_char_t *sRecord)			// 插入一条记录，保存至Flash
{
    xSemaphoreTake(xSemaphore_session, portMAX_DELAY); 

    // 先写数据区，再先写index区 ，
    session_u.record_t.magic = MAGIC_DATA_VALID;//0xffAA5AFF
    memcpy(&session_u.record_t.rec_data, sRecord, sizeof(session_service_records_char_t));
    memset(session_u.record_t.reserved, 0, sizeof(session_u.record_t.reserved));
	session_u.record_t._xor = 0x5a;
    //判断本次写入空间是否溢出
    if(record_index_u.index_t.writeAddr == (SESSION_DATA_ADDRESS_END))
    {
        record_index_u.index_t.writeAddr = SESSION_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
        // flag_cycled_record_storage = 1;
    }
    //判断数据是否被写满过(512K - 4K)
    if(record_index_u.index_t.writeAddr % 4096 == 0)
    {
        // if(flag_cycled_record_storage == 1)//数据是写满翻头，起始读取地址 上移至下一个4K地址
        {
            // if(record_index_u.index_t.readAddr >=record_index_u.index_t.writeAddr && \
            //    record_index_u.index_t.readAddr <= record_index_u.index_t.writeAddr + 4096) // 判断擦除是否影响 readAddr  为什么要有这个判断？readAddr是读地址！
               
            if ((record_index_u.index_t.readAddr > record_index_u.index_t.writeAddr)&&(record_index_u.index_t.readAddr < (record_index_u.index_t.writeAddr+4096)))
            {
                record_index_u.index_t.readAddr = record_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
                if (record_index_u.index_t.readAddr == (SESSION_DATA_ADDRESS_END))
                {
                    record_index_u.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
                }
            }
            else if ((record_index_u.index_t.total == (SESSION_RECORD_MAX_NUM))&&(record_index_u.index_t.readAddr == record_index_u.index_t.writeAddr))
            {
                record_index_u.index_t.readAddr = record_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
                if (record_index_u.index_t.readAddr == (SESSION_DATA_ADDRESS_END))
                {
                    record_index_u.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
                }
            }
        }

		if (flag_log_next)		// 如果Log正在传输，则需要更新读地址
		{								// 除写地址使用 enevt_index_u外，其余为Buf
			if ((record_index_log.index_t.readAddr > record_index_u.index_t.writeAddr)&&(record_index_log.index_t.readAddr < (record_index_u.index_t.writeAddr+4096)))
			{
				record_index_log.index_t.readAddr = record_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
				if (record_index_log.index_t.readAddr == (SESSION_DATA_ADDRESS_END))
				{
					record_index_log.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
				}
			}
			else if ((record_index_log.index_t.total == (SESSION_RECORD_MAX_NUM))&&(record_index_log.index_t.readAddr == record_index_u.index_t.writeAddr))
			{
				record_index_log.index_t.readAddr = record_index_u.index_t.writeAddr + 4096; //为啥直接偏移4K ? 因为翻转了，需要往前移位 4K
				if (record_index_log.index_t.readAddr == (SESSION_DATA_ADDRESS_END))
				{
					record_index_log.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;//这里的地址不应该直接偏移4K 啊，是循环滚动存储，index 偏移！
				}
			}
		}
	
        sm_log(SM_LOG_INFO, "session_index_actual_total: %ld \r\n", session_index_total_sync()); //currently must be keep! comment by vincent.he.
        sm_log(SM_LOG_INFO, "session record insert trig erase %d Byte fron addr 0X%X!------------\r\n", SESSION_INDEX_LEN, record_index_u.index_t.writeAddr);
        cy_serial_flash_qspi_erase(record_index_u.index_t.writeAddr, 4096);//擦除 4 K session record 将要写入的新扇区
    }
    cy_serial_flash_qspi_write(record_index_u.index_t.writeAddr, sizeof(Session_t), session_u.res);
    // index 参数更新,总条数更新
    record_index_u.index_t.writeAddr += sizeof(Session_t);//写 影响写地址，有时也会响读地址，比如写满翻头
     
    session_index_total_sync();

    //如果数据写满 翻头，需要处理读取地址 还未实现
    session_index_insert();

    // sm_log(SM_LOG_INFO, "session_index_addr:%X,magic: %X readAddr: %X , writeAddr: %X ,total: %d\r\n",\
    //         session_index_addr,\
    //         record_index_u.index_t.magic,\
    //         record_index_u.index_t.readAddr,\
    //         record_index_u.index_t.writeAddr,\
    //         record_index_u.index_t.total); 

    xSemaphoreGive(xSemaphore_session); 

    return 0;
}
// 此函数实现 在get next 被调用后，1秒后执行更新session index
void session_record_sync_proc(void)
{
    if(flag_get_next)
    {
        if(msTickDev->read( (uint8_t*)&tick_get_next, 4) - tick_get_next >= 1000)//1  秒
        {
            flag_get_next = 0;
            session_index_insert();
            tick_get_next = msTickDev->read( (uint8_t*)&tick_get_next, 4);
        }
    }
	if (flag_log_next)		// 1秒无数据传输，中断操作
	{
		if(msTickDev->read( (uint8_t*)&tick_get_next, 4) - tick_get_next >= 5000)//5  秒
		{
			flag_log_next = 0;
		}
	}
}
/*============= =========== ======== ===== ==================*/
int session_record_get(session_service_records_char_t *sRecord)					// 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
{
	xSemaphoreTake(xSemaphore_session, portMAX_DELAY); 
    int res = 0;
    cy_serial_flash_qspi_read(record_index_u.index_t.readAddr,sizeof(Session_t),session_u.res);
    if(session_u.record_t.magic == MAGIC_DATA_VALID)
    {
        memcpy(sRecord,&session_u.record_t.rec_data,sizeof(session_service_records_char_t));
        res = 0;
    }else{
        res = -1;
    }
	xSemaphoreGive(xSemaphore_session); 
	return res;
}
/*============= =========== ======== ===== ==================*/
__WEAK int session_record_get_next(session_service_records_char_t *sRecord)				// 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码
{
    int res = -1;
    /*这里依然做了偏移4K 的空间限制*/
     if(record_index_u.index_t.readAddr < SESSION_DATA_ADDRESS_STAR || \
        record_index_u.index_t.readAddr > (SESSION_DATA_ADDRESS_END - sizeof(Session_t)))
    {
        sm_log(SM_LOG_ERR, "session_record_get_next:addr %X,Address out-of-bounds!\r\n",record_index_u.index_t.readAddr,sizeof(Session_t));
        res = -1;
        return res;
    }
	xSemaphoreTake(xSemaphore_session, portMAX_DELAY); 
    //update total
    session_index_total_sync();//更新剩余多少条数！

    if(record_index_u.index_t.total > 0)//如果整体total 条数大于0， 那就get next
    {
        cy_serial_flash_qspi_read(record_index_u.index_t.readAddr,sizeof(Session_t),session_u.res);
        if(session_u.record_t.magic == MAGIC_DATA_VALID)//AA5A is valid
        {
            if(flag_get_next == 0)
            {
                flag_get_next = 1;
            }
            tick_get_next = msTickDev->read( (uint8_t*)&tick_get_next, 4);// 更新时基
            record_index_u.index_t.readAddr += sizeof(Session_t);
            if(record_index_u.index_t.readAddr >= SESSION_DATA_ADDRESS_END)
            {
                record_index_u.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;//这里仍然是偏移了4K!
            }
            session_index_total_sync();//更新剩余多少条数！
            memcpy(sRecord, &session_u.record_t.rec_data, sizeof(session_service_records_char_t));
            // sm_log(SM_LOG_INFO, "read session record:r_addr %X,w_addr %X, len %d byte ,total:%d!\r\n",\
            //             record_index_u.index_t.readAddr,record_index_u.index_t.writeAddr,sizeof(Session_t),record_index_u.index_t.total);
            res = 0;
        }
        else
        {//无效！
            tick_get_next = msTickDev->read( (uint8_t*)&tick_get_next, 4);// 更新时基
            record_index_u.index_t.readAddr += sizeof(Session_t);// 如果单条 数据错误，也需要跳过此条，避免读不到后边的数据
            session_index_total_sync();
            sm_log(SM_LOG_INFO, "read session record:addr %X, %d byte magic err !\r\n",record_index_u.index_t.readAddr,sizeof(Session_t));
            res = -1;
        }
    }
    else
    {
            sm_log(SM_LOG_INFO, "read session record:addr %X, %d byte, total = 0 err !\r\n",record_index_u.index_t.readAddr,sizeof(Session_t));
    }
	xSemaphoreGive(xSemaphore_session); 
	return res;
}
/*============= =========== ======== ===== ==================*/
int session_record_total_get(void)									// 返回数据记录总数
{
   session_index_total_sync();//更新剩余多少条数！
   return record_index_u.index_t.total;
}
/*============= =========== ======== ===== ==================*/
int session_record_erase_all(void)									// 清除所有Session数据记录
{
	xSemaphoreTake(xSemaphore_session, portMAX_DELAY); 
    //如果为了快速处理，不一定需要这样处理。只要把读写地址设置为相同即可，total = 0, 后面边写边擦除！comment by vincent.he, 20241205
    //擦除4K
    sm_log(SM_LOG_INFO, "spi flash sessions erase from:%X,len :%X ...\r\n", SESSION_INDEX_ADDRESS, SESSION_INDEX_LEN);
    cy_serial_flash_qspi_erase(SESSION_INDEX_ADDRESS,  SESSION_INDEX_LEN);//改为只擦除 4K ，初始化更快
    // cy_serial_flash_qspi_erase(SESSION_DATA_ADDRESS_END - 4096,4096);//擦除末尾4K
    sm_log(SM_LOG_INFO, "spi flash sessions erase done !\r\n");
//    flag_cycled_record_storage = 0;
    session_record_init();

	if (flag_log_next)
	{
		record_index_log = record_index_u;	// 更新Index，后续读的时候会终止
	}

	xSemaphoreGive(xSemaphore_session); 
	return 0;
}
/*============= =========== ======== ===== ==================*/
//uint16_t session_status_get(void)									// 高8字节返回模式，低8字节返回状态？？？
void	session_status_get(session_service_status_char_t * pData)
{
	HEATER *heaterInfo = get_heat_manager_info_handle();

	pData->eos_prompt_enable = get_eos_prompt();
	// pData->self_clean_prompt_enable = get_clean_prompt(); //bug2014438

	if ((heaterInfo->HeatMode) == HEATTING_CLEAN)
	{
		pData->session_mode = 3;
	}
	else if ((heaterInfo->HeatMode) == HEATTING_BOOST)
	{
		pData->session_mode = 2;
	}
	else 
	{
		pData->session_mode = 1;
	}

	switch (heaterInfo->HeatState)
	{
		case HEAT_FLOW_STATE_START:
		case HEAT_FLOW_STATE_PREHEAT:
		{
			pData->session_state = 1;
			break;
		}

		case HEAT_FLOW_STATE_HEAT_NORMAL:
		{
			pData->session_state = 2;
			break;
		}

		case HEAT_FLOW_STATE_HEAT_STAGE_LAST_EOS:
		{
            if (pData->eos_prompt_enable) //bug1979280
            {
			    pData->session_state = 3;
            }
            else
            {
			    pData->session_state = 2;
            }
			break;
		}
		
		case HEAT_FLOW_STATE_NONE:
		default:
		{
			pData->session_mode = 0;
			pData->session_state = 0;
			break;
		}	
	}
}
/*============= =========== ======== ===== ==================*/
uint16_t session_log_total_sync(void)
{
    uint16_t actual_quantity = 0;					// index_t.writeAddr 不能使用Bug值，有可能会更新

    if(record_index_log.index_t.readAddr == record_index_u.index_t.writeAddr)
    {
        record_index_log.index_t.total = 0;//默认就是0条！ 
    } 
    else if(record_index_log.index_t.readAddr < record_index_u.index_t.writeAddr)
    {
        record_index_log.index_t.total = (record_index_u.index_t.writeAddr - record_index_log.index_t.readAddr)/sizeof(Session_t);
    } 
    else if(record_index_log.index_t.readAddr > record_index_u.index_t.writeAddr)
    {
        record_index_log.index_t.total = ((SESSION_DATA_ADDRESS_END - record_index_log.index_t.readAddr) + \
            (record_index_u.index_t.writeAddr - (SESSION_DATA_ADDRESS_STAR)))/sizeof(Session_t);
    }

    actual_quantity = record_index_log.index_t.total;
 
    if (SESSION_RECORD_MAX_NUM < record_index_log.index_t.total)
    {
        uint32_t offset =0;
        offset =  (record_index_log.index_t.total - SESSION_RECORD_MAX_NUM)*sizeof(Session_t);
        record_index_log.index_t.readAddr += offset;
        if (record_index_log.index_t.readAddr > SESSION_DATA_ADDRESS_END)
        {
            record_index_log.index_t.readAddr = SESSION_DATA_ADDRESS_STAR + (record_index_log.index_t.readAddr - SESSION_DATA_ADDRESS_END);
        }
        else if (record_index_log.index_t.readAddr == SESSION_DATA_ADDRESS_END)
        {
            record_index_log.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;
        }

        if(record_index_log.index_t.readAddr < record_index_u.index_t.writeAddr)
        {
            record_index_log.index_t.total = (record_index_u.index_t.writeAddr - record_index_log.index_t.readAddr)/sizeof(Session_t);
        } 
        else if(record_index_log.index_t.readAddr > record_index_u.index_t.writeAddr)
        {
            record_index_log.index_t.total = ((SESSION_DATA_ADDRESS_END - record_index_log.index_t.readAddr) + \
                (record_index_u.index_t.writeAddr - (SESSION_DATA_ADDRESS_STAR)))/sizeof(Session_t);
        }
    }
    
    return actual_quantity;
}

/*============= =========== ======== ===== ==================*/
bool session_log_get(session_service_records_char_t *sRecord) // 从Flash读取第一条末上传的记录；若有数据且数据正确，返回0，否则返回错误码
{
	xSemaphoreTake(xSemaphore_session, portMAX_DELAY); 
	record_index_log = record_index_u;
	xSemaphoreGive(xSemaphore_session); 
		
	flag_log_next = 1;
    bool res = session_log_get_next(sRecord);
	if (res)
	{
        res =  true;
//		tick_get_next = msTickDev->read( (uint8_t*)&tick_get_next, 4);// 更新时基
    }
	else
	{
		flag_log_next = 0;
        res = false;
    }

	return res;
}
/*============= =========== ======== ===== ==================*/
bool session_log_get_next(session_service_records_char_t *sRecord)			// 从Flash读取下一条数据，上一条数据需要无效化处理；若有数据且数据正确，返回0，否则返回错误码
{
	Session_record_u session_u;
	bool res = false;

	if(record_index_log.index_t.readAddr < SESSION_DATA_ADDRESS_STAR || record_index_log.index_t.readAddr > (SESSION_DATA_ADDRESS_END - sizeof(Session_t)))
    {
		flag_log_next = 0;
        return false;
    }
	
	if (flag_log_next == 0)
	{
        return false;
    }

	xSemaphoreTake(xSemaphore_session, portMAX_DELAY); 
    session_log_total_sync();//更新剩余多少条数！

    if(record_index_log.index_t.total > 0)//如果整体total 条数大于0， 那就get next
    {
        cy_serial_flash_qspi_read(record_index_log.index_t.readAddr,sizeof(Session_t),session_u.res);
        if(session_u.record_t.magic == MAGIC_DATA_VALID)//AA5A is valid
        {
			flag_log_next = 1;
            tick_get_next = msTickDev->read( (uint8_t*)&tick_get_next, 4);// 更新时基
            record_index_log.index_t.readAddr += sizeof(Session_t);

//            sm_log(SM_LOG_INFO, "record_index_log.index_t.readAddr: 0x%08X\r\n", record_index_log.index_t.readAddr);

            if(record_index_log.index_t.readAddr >= SESSION_DATA_ADDRESS_END)
            {
                record_index_log.index_t.readAddr = SESSION_DATA_ADDRESS_STAR;//这里仍然是偏移了4K!
            }
            session_log_total_sync();//更新剩余多少条数！
            memcpy(sRecord, &session_u.record_t.rec_data, sizeof(session_service_records_char_t));
            res = true;
			update_idl_delay_time();	// 延时休眠
        }
        else
        {
			flag_log_next = 0;
            res = false;
        }
    }
    else
    {
		flag_log_next = 0;
        res = false;
    }
	xSemaphoreGive(xSemaphore_session); 
	return res;
}
/*============= =========== ======== ===== ==================*/

