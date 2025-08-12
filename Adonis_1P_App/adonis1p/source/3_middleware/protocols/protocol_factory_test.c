#include "protocol_factory_test.h"
#include "task_bt_service.h"
#include "data_base_info.h"
#include "platform_io.h"
#include "task_heatting_service.h"
#include "task_system_service.h"

#include "event_data_record.h"
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（1）状态控制PC->Device
__WEAK bool procotol_enter_sleep(void){return false;}// 进入休眠
__WEAK bool procotol_fuelGauge_enter_shutdown(void){//进入关机模式
	uint8_t cfgBuf[2] = {0};
	ptIoDev fgDev = io_dev_get_dev(DEV_FULE_GAUGE);

	cfgBuf[0] = FG_SET_SHUTDOWN;
	return (bool)fgDev->write((uint8_t *)&cfgBuf, 1);
}

__WEAK bool procotol_enter_transport_mode(void){//进入运输模式
	uint8_t cfgBuf[2] = {0};
	ptIoDev chgDev = io_dev_get_dev(DEV_CHARGE_IC);

	cfgBuf[0] = CHG_SET_SHIP_MODE;
	return (bool)chgDev->write((uint8_t *)&cfgBuf, 1);
}

__WEAK bool procotol_system_reset(void)
{
	flash_data_save_change(0);		// 带时间戳
    NVIC_SystemReset();//复位系统 FOR TEST
    return true;
}

//状态控制PC->Device
static uint16_t payload_sta_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);

    bool retFlag = false;
	if(tempLen != 0x0002)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    switch(pBuf->pData[1])
    {
        case 0x00://无效
        {
            retFlag = false;
            break;
        }
        case 0x01://进入休眠
        {
            retFlag = procotol_enter_sleep();
            break;
        }
        case 0x02://进入运输模式
        {   set_shipping_mode(1); // 0:正常模式，1：船运模式
#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
			uint8_t event_data[1];
			event_data[0] = DEVICE_PARAM_SHIPPING_MODE;
			event_record_generate(EVENT_CODE_PARAMETER_CHANGE, event_data, 1);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)
            retFlag = procotol_enter_sleep();
        //	retFlag  = procotol_fuelGauge_enter_shutdown(); // 休眠的时候再调用 防止正在显示时导致的显示撕裂
        //    retFlag += procotol_enter_transport_mode();// 休眠的时候再调用
            break;
        }
        case 0x5A://复位系统
        {
            retFlag = procotol_system_reset();
            break;
        }

    }

    
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（2）马达、蜂鸣器控制PC->Device
/*
    Payload [1]:
                0x00 无效
                0x01 马达
                0x02 蜂鸣器
    Payload [2]:
                0x00 OFF
                0x01 ON
*/

__WEAK bool procotol_motor_control(uint8_t ctr){return false;}// 马达
__WEAK bool procotol_buzzer_control(uint8_t ctr){return false;}//蜂鸣器
static uint16_t payload_motor_buzzer_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;
    
	if(tempLen != 0x0003)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    switch(pBuf->pData[1])
    {
        case 0x00://无效
        {
            retFlag = false;
            break;
        }
        case 0x01://马达
        {
            retFlag = procotol_motor_control(pBuf->pData[2]);
            break;
        }
        case 0x02://蜂鸣器
        {
            retFlag = procotol_buzzer_control(pBuf->pData[2]);
            break;
        }

    }
    
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（3）LED控制PC->Device
/*
Payload [1]:

            0x00 ALL OFF
            0x01 RED ON
            0x02 GREEN ON
            0x03 BLUE ON
            0x04 WHITE ON
*/
__WEAK bool procotol_led_control(uint8_t ctr){return false;}//LED

static uint16_t payload_led_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;
    
	if((tempLen != 0x0002)||(pBuf->pData[1] > 0x05))
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_led_control(pBuf->pData[1]);

    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（4）LCD控制PC->Device
/*
    Payload [1]:
                0x00 SCREEN OFF
                0x01 SCREEN RED
                0x02 SCREEN GREEN
                0x03 SCREEN BLUE
                0x04 SCREEN WHITE
*/

__WEAK bool procotol_lcd_control(uint8_t ctr){return false;}//LCD

static uint16_t payload_lcd_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;
    
	if((tempLen != 0x0002)||(pBuf->pData[1] > 0x05))
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_lcd_control(pBuf->pData[1]);

    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（5）加热电路控制PC->Device
/*
    Payload [1]:
                0x00 关闭输出，
                0x01 设置输出电压并启动，
                0x02 设置输出功率并启动，
                0x03 设置目标温度并启动
    Payload [2~3]:电压值（mv） 或 功率值（mW）或 温度值（℃）< 16位无符号整数>
*/

//__WEAK bool procotol_heat_opt(uint8_t optType,uint16_t optCode){return true;}//heat

static uint16_t payload_heat_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint16_t optCode =  COMB_2BYTE(pBuf->pData[3],pBuf->pData[2]);
    
    bool retFlag = false;
    
	if((tempLen != 0x0004)||(pBuf->pData[1]>0x05))
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_heat_opt(pBuf->pData[1],optCode);
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（6）外部flash读写测试PC->Device
__WEAK bool procotol_ext_flash_test(void){return false;}//heat

static uint16_t payload_ext_flash_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;
	if(tempLen != 0x0001)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_ext_flash_test();
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（0x07）充电SOC限制
__WEAK bool payload_chg_limit(uint8_t en, uint8_t soc, uint16_t volt){return false;}

static uint16_t payload_chg_limit_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
	uint16_t volt = COMB_2BYTE(pBuf->pData[3],pBuf->pData[4]); // 该数据格式为大端模式
    bool retFlag = false;
	if(tempLen != 0x0005)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = payload_chg_limit(pBuf->pData[1], pBuf->pData[2], volt);
	
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（7）蓝牙控制（TBD）
/*
    Payload [1]:
                0x00 关闭蓝牙
                0x01 打开蓝牙
*/

__WEAK bool procotol_ble_opt(uint8_t opt){return false;}

static uint16_t payload_ble_ctr(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    bool retFlag = false;
	if ((tempLen != 0x0002)||((pBuf->pData[1] != 0x00)&&(pBuf->pData[1] != 0x01)&&(pBuf->pData[1] != 0xfd)&&(pBuf->pData[1] != 0xff)))
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}
    
#ifndef INI_CONFIG_DEFAULT_EN // 没有定义默认值，则需要保存 -- modify by zshe 20240919
    retFlag = false;
#else
    retFlag = procotol_ble_opt(pBuf->pData[1]);
#endif
    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//（8）获取静态数据PC->Devic
/*
    数据长度：0x14 0x00
    Payload[0~1]:   发热体温度（℃） < 16位整数>
    Payload [2~3]:  PCBA温度（℃） < 16位整数>
    Payload [4~5]:  Type C温度（℃） < 16位整数>
    Payload [6~7]:  电芯温度（℃） < 16位整数>
    Payload [8~9]:  电芯电压（mv） < 16位无符号整数>
    Payload [10~11]:    电芯电量（%） < 16位无符号整数>
    Payload [12~13]:    发热体阻值（mΩ）< 16位无符号整数>
    Payload [14~19]:    蓝牙MAC地址
*/

__WEAK bool procotol_get_cell_capacity (uint16_t batVal,uint8_t* rValue){return false;}
__WEAK bool procotol_get_ble_mac(uint8_t* rValue){return false;}

static uint16_t payload_get_static_data(ProtocolBase_t *pBuf)
{
//    DetectorInfo_t heat_t;
//    BatInfo_t batteryInfo;
//    ptIoDev adcDev = io_dev_get_dev(DEV_ADC);
//    ptIoDev fulgaugeDev = io_dev_get_dev(DEV_FULE_GAUGE);
    uint16_t tempValue = 0;
    MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();

	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
	if(tempLen != 0x0001)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}
//    adcDev->read((uint8_t*)&heat_t,sizeof(heat_t));
//    fulgaugeDev->read((uint8_t*)&batteryInfo,sizeof(batteryInfo));// 船运模式下电量计未初始化
    tempValue = (uint16_t)((int16_t)((int32_t)p_tMontorDataInfo->det.heat_K_temp));//发热体温度（℃） < 16位整数> BUG 2031258
    pBuf->pData[0] = LBYTE(tempValue);
    pBuf->pData[1] = HBYTE(tempValue);

    tempValue = (uint16_t)((int16_t)((int32_t)p_tMontorDataInfo->det.heat_K_cood_temp)); // (uint16_t)p_tMontorDataInfo->det.heat_K_cood_temp;//PCBA温度（℃） < 16位整数>BUG 2031258
    pBuf->pData[2] = LBYTE(tempValue);
    pBuf->pData[3] = HBYTE(tempValue);

    tempValue = (uint16_t)((int16_t)((int32_t)p_tMontorDataInfo->det.usb_port_temp)); //(uint16_t)p_tMontorDataInfo->det.usb_port_temp;//Type C温度（℃） < 16位整数>BUG 2031258
    pBuf->pData[4] = LBYTE(tempValue);
    pBuf->pData[5] = HBYTE(tempValue);

    tempValue = (uint16_t)p_tMontorDataInfo->bat.temperature;//电芯温度（℃） < 16位整数>
    pBuf->pData[6] = LBYTE(tempValue);
    pBuf->pData[7] = HBYTE(tempValue);

    tempValue = (uint16_t)p_tMontorDataInfo->bat.voltage;//电芯电压（mv） < 16位无符号整数>
    pBuf->pData[8] = LBYTE(tempValue);
    pBuf->pData[9] = HBYTE(tempValue);

    procotol_get_cell_capacity(p_tMontorDataInfo->bat.remap_soc,&pBuf->pData[10]); //电芯电量（%） < 16位无符号整数>

    tempValue = (uint16_t)p_tMontorDataInfo->det.heat_R;//发热体阻值（mΩ）< 16位无符号整数>
    pBuf->pData[12] = LBYTE(tempValue);
    pBuf->pData[13] = HBYTE(tempValue);
    
    procotol_get_ble_mac(&pBuf->pData[14]);

    tempValue = (uint16_t)p_tMontorDataInfo->bat.current;//电芯电流（mA） <16位有符号整数>
    pBuf->pData[20] = LBYTE(tempValue);
    pBuf->pData[21] = HBYTE(tempValue);

    tempLen = 0x0016;//响应的数据长度
    pBuf->dataLen.low = LBYTE(tempLen);
    pBuf->dataLen.high = HBYTE(tempLen);
    pBuf->cmd = PROC_RESPONE_DATA;        

	return cmd_base_reply(pBuf,pBuf->cmd);
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（9）获取状态PC->Device
/*
   Payload[0]:
           0x00 非充电状态，
           0x01 充电状态     
   Payload [1]:
           0x00 key2 and key1 up
           0x01 key2 up, key1 down
           0x02 key2 down, key1 up
           0x03 key2 down, key1 down
   Payload [2]:
           0x00霍尔开关off
           0x01霍尔开关 on
   Payload [3]:
           Error code:
           0x00 正常
           Bit0: 0->正常，1->低压
           Bit1: 0->正常，1->发热体过温
           Bit2: 0->正常，1->硬件故障
*/

__WEAK bool procotol_get_charge_sta(uint8_t* rValue){return false;}

__WEAK bool procotol_get_key_sta(uint8_t* rValue)
{
    ptIoDev keyDev = io_dev_get_dev(DEV_KEY);
    uint8_t buf[3];//KEY1、KEY2、usbIN
    keyDev->read( (uint8_t*)&buf, 3);

    if(buf[0]==1 && buf[1]==0 )
    {
        rValue[0] =0x01;
    }
    else if(buf[0]==0 && buf[1]==1 )
    {
        rValue[0] =0x02;
    }
    else if(buf[0]==1 && buf[1]==1 )
    {
        rValue[0] =0x03;
    }else{
        rValue[0] =0x00;
    }     
    return true;
}

__WEAK bool procotol_get_hall_sta(uint8_t* rValue){rValue[0] =0x00;return false;}
__WEAK bool procotol_get_heat_sta(uint8_t* rValue)
{ 
    return false;
}

static uint16_t payload_get_status(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
	if(tempLen != 0x0001)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    procotol_get_charge_sta(&pBuf->pData[0]);
    procotol_get_key_sta(&pBuf->pData[1]);
    procotol_get_hall_sta(&pBuf->pData[2]);
    procotol_get_heat_sta(&pBuf->pData[3]);

    tempLen = 0x0004;//响应的数据长度
    pBuf->dataLen.low = LBYTE(tempLen);
    pBuf->dataLen.high = HBYTE(tempLen);
    pBuf->cmd = PROC_RESPONE_DATA;        

	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（10）DC/DC校准
/*
    Step1:
        Payload [1~2]: 设置输出电压
                
    Step2:
        Payload [1~2]: T40~T45 间电压值（mV）< 16位无符号整数>
        Payload [3~4]: 电流值
        
    Step3: 延时＞500ms，等待稳定，PC再次读取电压并判断是否在设置范围。
    注：Step1~Step3, 需进行两次循环，分别为设置1.2V 及 3.5V电压下。
*/

//__WEAK bool procotol_DCDC_opt(uint16_t optVol){return false;}
//Step1:设置输出PC->Device
static uint16_t payload_dcdc_step1(ProtocolBase_t *pBuf)
{
    bool retFlag = false;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
	uint16_t optVol  = COMB_2BYTE(pBuf->pData[2],pBuf->pData[1]);
    
	if(tempLen != 0x0003)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_DCDC_opt(optVol);

    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}

//__WEAK bool procotol_DCDC_adjust(uint16_t adjustVol, uint16_t adjustCur){return false;}
//Step2: 延时＞500ms，等待稳定，然后PC读取仪器测到的电压给到MCU
static uint16_t payload_dcdc_step2(ProtocolBase_t *pBuf)
{
    bool retFlag = false;
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint16_t adjustVol =  COMB_2BYTE(pBuf->pData[2],pBuf->pData[1]);
    uint16_t adjustCur =  COMB_2BYTE(pBuf->pData[4],pBuf->pData[3]);

	if(tempLen != 0x0005)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}
    
    retFlag = procotol_DCDC_adjust(adjustVol, adjustCur);

    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//（11）运放校准PC->Device
/*
    Payload [1~2]:T1运放输出电压值（mV）< 16位无符号整数>
*/

__WEAK bool procotol_OPA_adjust_vol(uint16_t adjustVol){return false;}

static uint16_t payload_OPA_adjust(ProtocolBase_t *pBuf)
{
	uint16_t tempLen = COMB_2BYTE(pBuf->dataLen.high,pBuf->dataLen.low);
    uint16_t adjustVol =  COMB_2BYTE(pBuf->pData[2],pBuf->pData[1]);
    bool retFlag = false;
	if(tempLen != 0x0003)
	{
        return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
	}

    retFlag = procotol_OPA_adjust_vol(adjustVol);

    if(retFlag==true)
    {
        pBuf->cmd = PROC_RESPONE_ACK;
    }else{
        pBuf->cmd = PROC_RESPONE_NCK;
    }
	return cmd_base_reply(pBuf,pBuf->cmd);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static const payloadFun_t payloadList[]=
{
	{PAYLOAD_STA_CTR       	,		payload_sta_ctr},           //（1）状态控制PC->Device
	{PAYLOAD_MTO_BEE       	,		payload_motor_buzzer_ctr},  //（2）马达、蜂鸣器控制PC->Device
	{PAYLOAD_LED_CTR       	,		payload_led_ctr},           //（3）LED控制PC->Device
	{PAYLOAD_LCD_CTR       	,		payload_lcd_ctr},           //（4）LCD控制PC->Device
	{PAYLOAD_HEAT_CTR      	,		payload_heat_ctr},          //（5）加热电路控制PC->Device
	{PAYLOAD_EXT_FALSH     	,		payload_ext_flash_ctr},     //（6）外部flash读写测试PC->Device
	{PAYLOAD_CHG_LIMIT		,		payload_chg_limit_ctr},  	//（0x07）充电SOC和VOLT限制
	{PAYLOAD_BLE_CTR       	,		payload_ble_ctr},           //（0x0E）蓝牙控制（TBD）
	
	{PAYLOAD_STATIC_DATA   	,		payload_get_static_data},   //（8）获取静态数据PC->Device
	{PAYLOAD_GET_STA       	,		payload_get_status},        //（9）获取状态PC->Device
	{PAYLOAD_DCDC_ADJUST_S1	,		payload_dcdc_step1},        //（10）DC/DC校准 Step1:设置输出PC->Device
	{PAYLOAD_DCDC_ADJUST_S2	,		payload_dcdc_step2},        //（10）DC/DC校准 Step2: 延时＞500ms，等待稳定，然后PC读取仪器测到的电压给到MCU
	{PAYLOAD_OPA_ADJUST	    ,		payload_OPA_adjust},        //（11）运放校准PC->Device
};

//3.3.5测试命令（0x05）
uint16_t cmd_factory_test(ProtocolBase_t *pBuf)
{
    uint8_t Payload0 = pBuf->pData[0];
	pBuf->deviceID = DEV_TX_TO_RX(DEV_1P,DEV_PC);

    for(uint16_t i = 0; i < eleof(payloadList); i++)
	{
		if(Payload0 == payloadList[i].payload && payloadList[i].func != NULL)
		{
			return payloadList[i].func(pBuf);
		}
	}
    pBuf->dataLen.low = 0;
    pBuf->dataLen.high = 0;
	return cmd_base_reply(pBuf,PROC_RESPONE_NCK);
}



