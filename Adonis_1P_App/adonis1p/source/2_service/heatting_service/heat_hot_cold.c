 
#include "math.h"

#include "heat_hot_cold.h"

double coefficient1,coefficient2;
//static double g_start_thermocouple_temp=25.0;

 
 
typedef struct ht_info
{
	uint32_t Start_Temp;    // 开启加热时的温度 单位℃
	uint32_t HotCood_K;     // 冷热机比例 0.83 - 1.0
} HOTCOOD_INFO;
// const HOTCOOD_INFO astDef_HC_Tbl[30] = 
// {
// 	{4500 ,1000},//100%
// 	{5500 ,955},//95%
// 	{7500 ,925},//90%
// 	{10000,895},//85
// 	{12500,865},//80
// 	{15000,855},//78
// 	{40000,850},
// 	{0,0},
// 	{0,0},
// 	{0,0},
// 	{0,0},
// 	{0,0},
// 	{0,0},
// 	{0,0},
// 	{0,0}
// };
const HOTCOOD_INFO astDef_HC_Tbl[30] = 
{
	{4500 ,1000},//100%
	{5500 ,1000},//95%
	{7500 ,1000},//90%
	{10000,990},//85
	{12500,980},//80
	{15000,950},//78
	{40000,950},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0}
};

float hal_get_hot_k(DEV_T dev_temp)
{
    HOTCOOD_INFO* pHC_Tbl;
    unsigned int i;
    float t1,t2,T1,T2;
    float hot_k = 1.000f;
    float start_temp = 0.0f;
    dev_temp.bord_ntc_temp*=100;
    dev_temp.Heat_k_temp*=100;

    if(dev_temp.bord_ntc_temp > 15000.0f){
        dev_temp.bord_ntc_temp = 15000.0f;
    }

    if(dev_temp.Heat_k_temp > 15000.0f)
    {
        dev_temp.Heat_k_temp = 15000.0f;
    }
    if(dev_temp.Heat_k_temp <4500.0f)
    {
        return 1.000f;
    }
    start_temp =  dev_temp.Heat_k_temp;//RANGE 0.0f - 150.0f
    
    pHC_Tbl = (HOTCOOD_INFO*)&astDef_HC_Tbl[0];
	for(i = 0; i < 15 - 2; i++)
	{
		if(pHC_Tbl[i].Start_Temp <= start_temp
		&& pHC_Tbl[i + 1].Start_Temp > start_temp) break;
	}
	
	if((15 - 2) == i) i = 15 - 2;
	
    t1 = (float)pHC_Tbl[i].Start_Temp;
	t2 = (float)pHC_Tbl[i + 1].Start_Temp;
	T1 = (float)pHC_Tbl[i].HotCood_K;
	T2 = (float)pHC_Tbl[i + 1].HotCood_K;
	
	hot_k = (((T2 - T1) / (t2 - t1)) * (start_temp - t1) + T1);
	if(hot_k <  780) hot_k = 780;
    if(hot_k >  1000) hot_k = 1000;
    return hot_k/1000.0f;
}
/*
 
*/









