#include "stdio.h"
#include "string.h"
#include "driver_heater.h"
#include "driver_i2c.h"
#include "sm_log.h"
 
#define TPS55288_ADDR	 0x74
//--------------------------------DC-DC相关-----------------------------------------------
 /**
  * @brief  DCDC写寄存器
  * @param  reg_addr 寄存器地址，*dat数据指针，len 数据长度
  * @return 返回状态
  * @note   None
  */
cy_rslt_t hal_tps55288_write_reg(uint8_t reg_addr,uint8_t *dat,uint8_t len)
{
	cy_rslt_t result=0;
	uint8_t write_buf[8];
	I2cDataType_t w_t = {0};
	w_t.busN = I2C_BUS0;
 	w_t.devAddr = I2C_ADDR_TPS55288;
	write_buf[0] = reg_addr;
	write_buf[1] = dat[0];
	w_t.wDat = write_buf;
	w_t.wLen = len + 1;
//	taskENTER_CRITICAL() ;
	result = driver_i2c_write((uint8_t*)&w_t,0);
//	taskEXIT_CRITICAL();
	return result;
}
/**
 * @brief  DCDC读写寄存器
 * @param  reg_addr 寄存器地址，*out_dat数据输出指针，len 数据长度
 * @return 返回状态
 * @note   None
 */
cy_rslt_t hal_tps55288_read_reg(uint8_t reg_addr,uint8_t *out_dat,uint8_t len)
{
	cy_rslt_t result;
	 I2cDataType_t r_t = {0};
	r_t.busN = I2C_BUS0;
 	r_t.devAddr = I2C_ADDR_TPS55288;
	r_t.wDat = &reg_addr;
	r_t.wLen =   1;
	r_t.rDat = out_dat;
	r_t.rLen = len;
//	taskENTER_CRITICAL() ;
    result = driver_i2c_read((uint8_t*)&r_t,0);
//    taskEXIT_CRITICAL();
	return result;
}
/**
 * @brief  DCDC 使能
 * @param  en :1 使能 ，0 失能
 * @return 返回状态
 * @note   None
 */
cy_rslt_t hal_tps55288_set_enable(uint8_t en)
{
	cy_rslt_t result;
	uint8_t read_reg;
	uint8_t wr_buf[8]={0};
	result = hal_tps55288_read_reg(0x06,wr_buf,1);
	read_reg = wr_buf[0];
	if(en == 0){
		read_reg &= 0x7f;
		wr_buf[0] = read_reg;
		result = hal_tps55288_write_reg(0x06,wr_buf,1);
	}else{
		read_reg |= 0x80;
		wr_buf[0] = read_reg;
		result = hal_tps55288_write_reg(0x06,wr_buf,1);
	}
	return result;
}
/**
 * @brief  DCDC 设置输出电压
 * @param  set_v 设置的输出电压：单位 W ；Range 0 - 4.8V
 * @return 返回状态
 * @note   None
 */
cy_rslt_t hal_tps55288_set_out_v(float set_v)
{
	cy_rslt_t result;
	uint8_t ref_lsb,ref_msb;
	uint16_t num = 0;
	uint8_t wr_buf[8]={0};

	/*限制VOUT电压范围：0.8V-4.8V*/
	if(set_v < 0.8f) set_v = 0.0f;
	else if(set_v > 4.8f) set_v = 4.8f;

	num = (uint16_t)((0.2256f * set_v - 0.045f) / 0.001129f);	//(num*0.001129+0.045)/0.1128=value
	ref_lsb = (uint8_t)(num % 0x0100);
	ref_msb = (uint8_t)(num >> 8);

	wr_buf[0] = ref_lsb;
	result = hal_tps55288_write_reg(0x00,wr_buf,1);
	wr_buf[0] = ref_msb;
	result = hal_tps55288_write_reg(0x01,wr_buf,1);

	return result;
}


cy_rslt_t hal_tps55288_init(void)
{
	cy_rslt_t result;
	uint8_t wr_buf[8]={0};
	// result = cyhal_gpio_init(CYBSP_DCDCEN_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
	// cyhal_gpio_write(CYBSP_DCDCEN_PIN, 1);

	//i2c_initialize(I2C_BUS0);//P60--SCL;P61---SDA
	// IOUT_LIMIT
	wr_buf[0] = 0XE4;//0x7f; //20240618 FAE 建议修改此寄存器值
	result = hal_tps55288_write_reg(0x02,wr_buf,1);	// disabled current limit,63.5mV/5mR=12.7A
	//OCP,SR
	wr_buf[0] = 0x31;
	result = hal_tps55288_write_reg(0x03,wr_buf,1);	// delay 12ms,2.5mV/uS
	//FB,INTFB
	wr_buf[0] = 0x00;
	result = hal_tps55288_write_reg(0x04,wr_buf,1);	//internal FB,ratio=0.2256
	//MASK,CDC
	wr_buf[0] = 0xA0;
	result = hal_tps55288_write_reg(0x05,wr_buf,1);	//SC,dis OCP,OVP,intermal CDC,0.1V
	//MODE
	wr_buf[0] = 0x21;
	result = hal_tps55288_write_reg(0x06,wr_buf,1);	//dis output,dis double frequency,enable hiccup,dis vout when shutdown, vcc,74h,PFM,set by register

	return result;
}


/**
  * @brief  heatting 相关IO初始化
  * @param  pvParam:    任务入参
  * @return None
  * @note   None
  */
void heating_io_init(void)
 {
	cyhal_gpio_init(CYBSP_DCDCEN_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_init(CYBSP_HEAT_NMOS_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
	
    cyhal_gpio_write(CYBSP_DCDCEN_PIN, 0); // 为1 则 DISABLE
    cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN,1);
 }

/**
  * @brief  初始化发热体控制IO
  * @param  None
  * @return None
  * @note   None
  */
int driver_heater_init(void)
{//唤醒被调用
 
// 初始化 DCDC 、设置输出0V DCDCEN 引脚失能
	 //driver_i2c_init();
    // sm_log(SM_LOG_INFO, "heating_io_dcdc_init...\r\n");
    // heating_io_init();
    // hal_tps55288_init();
    // hal_tps55288_set_enable(0);
    // sm_log(SM_LOG_INFO, "heating init ok\r\n");
 //hal_tps55288_set_out_v(0.0f);
    
 
    return 0;
}

/**
  * @brief  去初始化发热体控制IO
  * @param  None
  * @return None
  * @note   None
  */
int driver_heater_deinit(void)
{//休眠前 被调用

	cyhal_gpio_init(CYBSP_DCDCEN_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_init(CYBSP_HEAT_NMOS_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
	
	cyhal_gpio_write(CYBSP_DCDCEN_PIN, 1);   // 为1 则 DISABLE
    cyhal_gpio_write(CYBSP_HEAT_NMOS_PIN,0); // 加热NMOS 关闭
    return 0;
}

/**
  * @brief  控制发热体发热
  * @param  buf:要写入的数据
            len:要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   控功率或者阻值
  */
int driver_heater_write(uint8_t *pBuf, uint8_t len)
{
 
    return 0;
}


