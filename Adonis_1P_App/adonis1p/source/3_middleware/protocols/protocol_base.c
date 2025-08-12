
#include "protocol_base.h"



uint16_t calc_checksum(uint8_t *pBuf,uint16_t len)
{
	uint16_t sum = 0;

	for(uint16_t i = 0; i < len; i++)
	{
		sum += (uint16_t)pBuf[i];
	}

	return sum;	
}


//回复ACK或是NCK
uint16_t cmd_base_reply(ProtocolBase_t *pBuf,uint8_t replyaCMD)
{
	uint16_t crc;
	uint16_t rlen = 0;
	pBuf->cmd = replyaCMD;
    switch(replyaCMD)
    {
        case PROC_RESPONE_ACK:
        case PROC_RESPONE_NCK:
        case PROC_REQUEST_DATA:
        {
            rlen = 0 ;
            pBuf->dataLen.low = LBYTE(rlen);
            pBuf->dataLen.high = HBYTE(rlen);
            break;
        }
        case PROC_RESPONE_DATA:
        {
            rlen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);        
            break;
        }
        case PROC_RESPONE_REPLAY:
        {
            rlen = 4;//协议上是4个字节，内容暂末定义
            pBuf->dataLen.low = LBYTE(rlen);
            pBuf->dataLen.high = HBYTE(rlen);
            break;
        }    
        default:
            break;
    }


	crc = calc_checksum((uint8_t*)pBuf,(rlen+PROC_ACK_LEN-2));
	pBuf->pData[rlen] = crc & 0x00ff;
	pBuf->pData[rlen+1] = (crc >> 8) & 0x00ff;
	return (rlen+PROC_ACK_LEN);
}




/***************************************************************
* 函数名称: CRC16_formula
* 函数功能: 计算CRC16结果
* 输入参数: crcIn:CRC输入，byte:数据
* 返回结果: CRC结果
****************************************************************/
static unsigned short CRC16_formula(unsigned short crcIn, unsigned char byte)
{
	uint32_t crc = crcIn;
	uint32_t in = byte|0x100;

	do{
		crc <<= 1;
		in <<= 1;

		if(in&0x100)
		{
			++crc;
		}

		if(crc&0x10000)
		{
			crc ^= 0x1021;
		}
	} while(!(in&0x10000));

	return (crc&0xffffu);
}

/*******************************************************************************
*Function: CRC16_CCITT
*Description: Standard CRC CCITT calculation
*Input:  data->Input data stream 
*        size->Data stream size 
*Output: None
*Return: Crc data feedback value
*******************************************************************************/
uint16_t CRC16_CCITT(const unsigned char* data, uint32_t size)
{
	uint32_t crc = 0;
	const unsigned char* dataEnd = data+size;

	while(data<dataEnd)
	{
		crc = CRC16_formula(crc,*data++);
	}
	crc = CRC16_formula(crc,0);
	crc = CRC16_formula(crc,0);

	return (crc&0xffffu);
}

/***************************************************************
* 函数名称: ComGetCodeUnLockValue
* 函数功能: 获取解锁数据
* 输入参数: RanCode，随机码
* 返回结果: 生成的解锁码
****************************************************************/
uint32_t ComGetCodeUnLockValue(uint32_t RanCode)
{
	uint16_t Data[ 4 ];

	RanCode = RanCode + 0x4D4F5441;
	Data[0] = RanCode;
	Data[1] = RanCode>>16;

	Data[2] = CRC16_CCITT((unsigned char*)Data,4);
	Data[3] = CRC16_CCITT((unsigned char*)&Data[1],4);

	return (Data[3]<<16 | Data[2]);
}













