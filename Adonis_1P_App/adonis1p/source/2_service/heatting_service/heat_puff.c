#include "heat_puff.h"
#include "data_base_info.h"
#include "sm_log.h"
/*---------------------------计口数proc----------------------------------*/
//const HEAT_TEMP puff_ref_temp[15] = 
//{
//	//时间(1毫秒)，温度(0.01摄氏度)
//	{0,2500},
//	{7500,28000},
//	{11000,27500},
//	{21000,22500},
//	{61000,22000},
//};//无点

const HeatTemp_t puff_ref_temp[15] = 
{
	//时间(1毫秒)，温度(0.01摄氏度)
	{0,2500},
	{7500, 33500},
	{9000, 32500},
	{21000,23500},
	{60000,22000},
};//有点样机
/***************************************************************
* 函数名称: HalGetCurrCtrlTemperature
* 函数功能: 按照温控曲线计算下一时间的目标温度值
* 输入参数: HeatTime: 当前加热时间
* 返回结果: 当前加热时间对应温控曲线的温度
****************************************************************/
float heat_get_puff_ref_temp(unsigned int HeatTime)
{
	HeatTemp_t* pTempTbl;
	unsigned int i;
	float rts;
	float t1,t2,T1,T2;

	pTempTbl = (HeatTemp_t *)&puff_ref_temp[0];
	for(i = 0; i < 5 - 2; i++)
	{
		if(pTempTbl[i].time <= HeatTime
		&& pTempTbl[i + 1].time > HeatTime) break;
	}
	
	if((5 - 2) == i) i = 5 - 2;
	
	t1 = (float)pTempTbl[i].time;
	t2 = (float)pTempTbl[i + 1].time;
	T1 = (float)pTempTbl[i].tempeture;
	T2 = (float)pTempTbl[i + 1].tempeture;
	rts = (((T2 - T1) / (t2 - t1)) * (HeatTime - t1) + T1);
	if(rts < 0) rts = 0;
	return rts;
}

float           g_puff_k = 0;//计口数斜率
unsigned char   g_heat_puff;

void heat_puff_init(void)
{
    g_heat_puff = 15;
}
uint8_t heat_puff_get(void)
{
    return g_heat_puff;
}
//------------------------------------------------------
#define PUFF_WAVE_IDLE                  0
#define PUFF_WAIT_DEEP_FALLING_EDGE     1
#define PUFF_WAIT_RISING_EDGE           2
#define PUFF_JUDGE_MAX_COUNT            9
#define PUFF_JUDGE_START_WINDOW         30
#define PUFF_FILTER_WINDOW              80

float puff_sample_last = 0.0;
float puff_sample_buf[PUFF_JUDGE_MAX_COUNT] = {0.0};
uint32_t puff_sta       = PUFF_WAVE_IDLE;
static uint32_t puff_outtime;

void heat_puff_proc(uint8_t *puff_p,float temp_cur,unsigned int time)//20230711 cc 修改为斜率识别
{
    static float power_peek_max = 0.0,power_peek_min=0.0,old_puff_k = 0.0;
    static uint8_t dir_k=0,old_dir=0;
    if(time > 10000 || g_heat_puff <= 14)
    {
      //PRINTF("---%0.2f,%0.2f,%d---\r\n",ref,temp_cur,time);
        puff_sample_buf[2] = puff_sample_buf[1];
        puff_sample_buf[1] = puff_sample_buf[0];
        puff_sample_buf[0] = temp_cur;
        g_puff_k = (puff_sample_buf[0] - puff_sample_buf[2])/0.4;
        switch (puff_sta)
        {
            case PUFF_WAVE_IDLE://没发生抽吸 
                if(g_puff_k > 0.35f )
                {
                    puff_sta = PUFF_WAIT_DEEP_FALLING_EDGE;
                }
                puff_outtime = 0;
            break;
            
            case PUFF_WAIT_DEEP_FALLING_EDGE:
                if(g_puff_k > 0.42f)
                {
                    puff_sta = PUFF_WAIT_RISING_EDGE;
                    puff_outtime = 0;
                  //  sm_log(SM_LOG_INFO, "------------- puff_p RISING_EDGE----------------- \r\n");
                }else{
                    if(++puff_outtime > 1000/500)puff_sta = PUFF_WAVE_IDLE;//超时返回至IDLE
                }
            break;

            case PUFF_WAIT_RISING_EDGE://放开嘴 功率开始下降
                if(g_puff_k < -0.05)
                {
                    if(g_heat_puff > 0) g_heat_puff--;
                    puff_sta = PUFF_WAVE_IDLE;
                   // sm_log(SM_LOG_INFO, "------------- puff_p FALLING_EDGE----------------- \r\n");
                }
                else
                {
                    if(++puff_outtime > 5000/500)
                    {
                        if(time > 25000){//分时间段处理，后段长抽吸识别
                            if(g_heat_puff > 0)g_heat_puff--;
                        }
                        puff_sta = PUFF_WAVE_IDLE;//超时返回至IDLE
                    }
                }
            break;

            default:
            break;
        }
   }else{
       g_puff_k = 0;
       
       puff_sample_buf[2] = temp_cur;
       puff_sample_buf[1] = temp_cur;
       puff_sample_buf[0] = temp_cur;
       
       if(temp_cur < heat_get_puff_ref_temp(time )/ 100u - 8.0f && (time > 6000 ))
       {
         if(g_heat_puff >=15)g_heat_puff--;
       }
   }
}

float heat_get_puff_k(void)
{
    return g_puff_k ;
}

//end of hal_puff.c

void heat_puff_proc2(float power_cur,unsigned int time) //20MS 调用一次
{
    static int power_high_inv = 0;
     int dec_puff = 0;
    if(time > 5*1000)
    {
        if(power_cur > 3.0f)
        {
            power_high_inv ++;
        }
        else
        {
            if(power_high_inv >= 3.5*1000/20 && power_high_inv < 13*1000/20)
            {
                dec_puff = 1;
            }
            else if(power_high_inv >= 13*1000/20 && power_high_inv < 21*1000/20)
            {
               dec_puff = 2;
            }
            else if(power_high_inv >= 21*1000/20 && power_high_inv < 28*1000/20)
            {
               dec_puff = 3;
            }
            else if(power_high_inv >= 28*1000/20 && power_high_inv < 35*1000/20)
            {
               dec_puff = 4;
            }
            
            if(power_high_inv >= 35*1000/20)
            {
               dec_puff = 5;
            }

            if(dec_puff >=1)
            {
                // BspUartToPcSendString((uint8_t *)"\r\n Puff_high_inv");
                // BspUartToPcPrintInt(power_high_inv,1);
                if(g_heat_puff >= dec_puff)
                {
                    g_heat_puff -= dec_puff;
                }
                else
                {
                    g_heat_puff = 0;
                }
            }
            power_high_inv = 0;
        }
    }
}












