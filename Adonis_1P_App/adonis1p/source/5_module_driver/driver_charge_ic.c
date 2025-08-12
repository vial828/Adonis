/**
  ******************************************************************************
  * @file    chg.c
  * @author  xuhua.huang@metextech.com
  * @date    2024/03/013
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
  * 2024-03-013     V0.01      xuhua.huang@metextech.com    the first version
  *
  ******************************************************************************
  */

#include "driver_charge_ic.h"
#include "data_base_info.h"

typedef struct {
	uint8_t userConfig;
	SemaphoreHandle_t xSemaphore;
}BQ25638_t;

static BQ25638_t gs_chg;

/**********************************************************************************/
#if 1
static void chg_mDelay(uint16_t ms)
{
	vTaskDelay(ms);
}
#endif
/**********************************************************************************/
static I2cDataType_t i2c = {
	.busN = I2C_BUS0,
	.devAddr = I2C_ADDR_BQ25638,
	.wLen = 0,
	.rLen = 0
};

cy_rslt_t chg_write_bytes(uint8_t reg_addr, uint8_t *dat, uint8_t len)
{
	cy_rslt_t rslt = 0;
	uint8_t wBuf[8] = {0};

	if(!(len < sizeof(wBuf))) {
		return RET_FAILURE;
	}

	wBuf[0] = reg_addr;
	for (uint8_t i = 0; i < len; i++) {
		wBuf[1 + i] = dat[i];
	}

	i2c.wDat = wBuf;
	i2c.wLen = len + 1;

	rslt = (cy_rslt_t)driver_i2c_write((uint8_t*) &i2c, 0);

	return rslt;
}

cy_rslt_t chg_read_bytes(uint8_t reg_addr, uint8_t *dat, uint8_t len)
{
	cy_rslt_t rslt = 0;

	i2c.wDat = &reg_addr;
	i2c.wLen = 1;
	i2c.rDat = dat;
	i2c.rLen = len;

	rslt = (cy_rslt_t)driver_i2c_read((uint8_t*) &i2c, 0);

	return rslt;
}
/****************************************************************************************
* @brief   update and read 16bit reg appointed bits
* @param   reg_addr.bit_mask.bit_val
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note
****************************************************************************************
**/
int chip_reg_update_word(uint8_t reg_addr, uint16_t bit_mask, uint16_t bit_val)
{
	cy_rslt_t rslt = 0;
    uint16_t tmp = 0;

    rslt = chg_read_bytes(reg_addr, (uint8_t *)&tmp, 2);
    tmp = (tmp & ~bit_mask);
    tmp |= bit_val;
    rslt = chg_write_bytes(reg_addr, (uint8_t *)&tmp, 2);

	return (int)rslt;
}

int chip_reg_read_word(uint8_t reg_addr, uint16_t bit_mask, uint16_t *dat)
{
	cy_rslt_t rslt = 0;
	uint16_t tmp = 0;

    rslt = chg_read_bytes(reg_addr, (uint8_t*) &tmp, 2);

	*dat = (uint16_t)(tmp & bit_mask);

	return (int)rslt;
}

/****************************************************************************************
* @brief   update and read 8bit reg appointed bits
* @param   reg_addr.bit_mask.bit_val
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note
****************************************************************************************
**/
int chip_reg_update_byte(uint8_t reg_addr, uint8_t bit_mask, uint8_t bit_val)
{
	cy_rslt_t rslt = 0;
    uint8_t tmp = 0;

    rslt = chg_read_bytes(reg_addr, &tmp, 1);
    tmp = (tmp & ~bit_mask);
    tmp |= bit_val;
    rslt = chg_write_bytes(reg_addr, &tmp, 1);

	return (int)rslt;
}

int chip_reg_read_byte(uint8_t reg_addr, uint8_t bit_mask, uint8_t *dat)
{
	cy_rslt_t rslt = 0;
    uint8_t tmp = 0;

    rslt = chg_read_bytes(reg_addr, &tmp, 1);
    *dat = (uint8_t)(tmp & bit_mask);

	return (int)rslt;
}

/****************************************************************************************
* @brief   Bat charge current limit setting
* @param   Bat charge current limit(range:80mA-5040mA); default:2000mA
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    unit:mA
****************************************************************************************
**/
int chg_set_bat_curr_limit(uint16_t curr)
{
	int ret = 0;
    uint16_t temp = 0;

    if (curr > 5040U) {
        curr = 5040U;
    }
    if (curr < 80U) {
        curr = 80U;
    }
    temp = (curr / 80U) << CHG_CURR_LIMIT_SHIFT;
	ret = chip_reg_update_word(CHG_REG02, CHG_CURR_LIMIT_MASK, temp);
	return ret;
}

/****************************************************************************************
* @brief   bat charge volt limit setting
* @param   bat charge volt limit(range:3500mV-4800mV); default:4200mV
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    unit:mV
****************************************************************************************
**/
int chg_set_bat_volt_limit(uint16_t volt)
{
	int ret = 0;
    uint16_t temp = 0;

    if (volt >= 4800U) {
        volt = 4500U;
    }
    if (volt <= 3500U) {
        volt = 3500U;
    }
    temp = (volt / 10U) << CHG_VOLT_LIMIT_SHIFT;
	ret = chip_reg_update_word(CHG_REG04, CHG_VOLT_LIMIT_MASK, temp);
	return ret;
}

/****************************************************************************************
* @brief   Input current limit setting
* @param   Input current limit(range:100mA-3200mA); default:3200mA
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note	   unit:mA
****************************************************************************************
**/
int chg_set_input_curr_limit(uint16_t curr)
{
	int ret = 0;
    uint16_t temp = 0;

    if (curr >= 3200U) {
    	curr = 3200U;
    }
    if (curr <= 100U) {
    	curr = 100U;
    }
    temp = (curr / 20U) << INPUT_CURR_LIMIT_SHIFT;
	ret = chip_reg_update_word(CHG_REG06, INPUT_CURR_LIMIT_MASK, temp);
	return ret;
}

/****************************************************************************************
* @brief   Input volt limit setting
* @param   Input voltage limit(range:3800mV-16800mV); default:4400mV
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    unit:mV
****************************************************************************************
**/
int chg_set_input_volt_limit(uint16_t vindpm)
{
	int ret = 0;
    uint16_t temp = 0;

    if (vindpm >= 16800U) {
        vindpm = 16800U;
    }
    if (vindpm <= 3800U) {
        vindpm = 3800U;
    }
    temp = (vindpm / 40U) << INPUT_VOLT_LIMIT_SHIFT;
	ret = chip_reg_update_word(CHG_REG08, INPUT_VOLT_LIMIT_MASK, temp);
	return ret;
}

/****************************************************************************************
* @brief   Minimal system volt Limit setting
* @param   Minimal system voltage limit(range:2560mV-3840mV);default:3520mV
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    unit:mV
****************************************************************************************
**/
int chg_set_min_sys_volt_limit(uint16_t volt)
{
	int ret = 0;
    uint16_t temp = 0;

    if (volt >= 3840U) {
    	volt = 3840U;
    }
    if (volt <= 2560U) {
    	volt = 2560U;
    }
    temp = (volt / 80U) << MIN_SYS_VOLT_SHIFT;
	ret = chip_reg_update_word(CHG_REG0E, MIN_SYS_VOLT_MASK, temp);
	return ret;
}

/****************************************************************************************
* @brief   Set the pre-charge current
* @param   pre-charge current(range:40mA-1000mA);default:200mA
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    unit:mA
****************************************************************************************
**/
int chg_set_pre_charge_curr(uint16_t curr)
{
	int ret = 0;
    uint16_t temp = 0;

    if (curr >= 1000U) {
    	curr = 1000U;
    }
    if (curr <= 40U) {
    	curr = 40U;
    }
    temp = (curr / 20U) << CHG_IPRE_SHIFT;
	ret = chip_reg_update_word(CHG_REG10, CHG_IPRE_MASK, temp);
	return ret;
}
/****************************************************************************************
* @brief   Set the termination current
* @param   Iterm_current(range:30mA-1000mA);default: 200mA
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    unit:mA
****************************************************************************************
**/
int chg_set_term_charge_curr(uint16_t curr)
{
	int ret = 0;
    uint16_t temp = 0;

    if (curr >= 1000U) {
        curr = 1000U;
    }
    if (curr <= 30U) {
    	curr = 30U;
    }
    temp = (curr / 10U) << CHG_ITERM_SHIFT;
	ret = chip_reg_update_word(CHG_REG12, CHG_ITERM_MASK, temp);
	return ret;
}

/****************************************************************************************
* @brief   wdg reset
* @param   None
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note
****************************************************************************************
**/
int chg_wdg_reset(void)
{
	int ret = chip_reg_update_byte(CHG_REG16, WDG_RESET_MASK, WDG_RESET);
	return ret;
}
/****************************************************************************************
* @brief   wdg timer config
* @param   0--WDG_DISABLE
           1--WDG_40S
           2--WDG_80S
           3--WDG_160S
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note
****************************************************************************************
**/
int chg_wdg_timer_cfg(uint8_t time)
{
	int ret = 0;

	ret  = chip_reg_update_byte(CHG_REG16, WDG_RESET_MASK, WDG_RESET);
	ret += chip_reg_update_byte(CHG_REG16, WDG_TIMER_MASK, time);

	return ret;
}
/****************************************************************************************
* @brief   reg reset
* @param   None
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note
****************************************************************************************
**/
int chg_reset_all_reg(void)
{
	int ret = chip_reg_update_byte(CHG_REG17, RESET_REG_MASK, REG_RESET);
	return ret;
}

/****************************************************************************************
* @brief   The control logic of the BATFET to force the device enter different modes
* @param
		DEV_ENTER_IDLE_MODE // (default)
		DEV_ENTER_SHUTDOWN_MODE
		DEV_ENTER_ULTRA_LOW_POWER_MODE
		DEV_ENTER_SYS_POWER_RST
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    default:Idle
****************************************************************************************
**/
int chg_batfet_ctrl(uint8_t mode)
{
	int ret = chip_reg_update_byte(CHG_REG18, BATFET_CTRL_MASK, mode);
	return ret;
}

/****************************************************************************************
* @brief   Enable External ILIM pin input current regulation
* @param   sw:1--enable  0--disable
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    default:enable
****************************************************************************************
**/
int chg_external_Ilim_pin_ctrl(uint8_t en)
{
	int ret = chip_reg_update_byte(CHG_REG19, EXT_ILIM_PIN_CTRL_MASK, (en != 0 ? EXT_ILIM_PIN_ENABLE : EXT_ILIM_PIN_DISABLE));
	return ret;
}
/****************************************************************************************
* @brief   Input Current Optimization (ICO) Algorithm Control
* @param   sw:1--enable and start  0--disable
* @return  RET_SUCCESS(0);RET_FAILURE(-1);
* @note    default:enable but closed
****************************************************************************************
**/
int chg_ico_start(uint8_t en)
{
	int ret = 0;

	if (en != 0) {
		ret  = chip_reg_update_byte(CHG_REG19, ICO_CTRL_MASK, ICO_ENABLE);
		ret += chip_reg_update_byte(CHG_REG19, FORCE_ICO_MASK, FORCE_ICO_START);
	} else {
		ret = chip_reg_update_byte(CHG_REG19, ICO_CTRL_MASK, ICO_DISABLE);
	}
	return ret;
}

/***************************************************************************************/
int chg_control(uint8_t en)
{
	return (int)chip_reg_update_byte(CHG_REG16, CHG_CFG_MASK, (en != CHG_DIS ? CHG_CFG_ENABLE : CHG_CFG_DISABLE));
}

int chg_HIZ_mode(uint8_t en)
{
	return (int)chip_reg_update_byte(CHG_REG16, HIZ_MODE_MASK, (en != CHG_DIS ? HIZ_MODE_ENABLE : HIZ_MODE_DISABLE));
}
/***************************************************************************************/
int16_t binary_to_origin(int16_t dat, int16_t mask, int16_t bit, int16_t shift)
{
	int16_t temp = 0;

	dat >>= shift;
	if ((dat & mask) != 0) {
		temp = (~dat) & bit;
		temp = -(temp + 1);
	} else {
		temp = (int16_t)dat;
	}

    return temp;
}

int chg_dat_get(ChgInfo_t *chg)
{
    int ret = 0;
    uint16_t dat = 0;
	int16_t calc_tmp = 0;

#if 0
	ret += chip_reg_read_byte(CHG_REG14, 0xFF, &chg->reg14_ctrlTimer);
	ret += chip_reg_read_byte(CHG_REG15, 0xFF, &chg->reg15_ctrl0); // Charger_Status
	ret += chip_reg_read_byte(CHG_REG16, 0xFF, &chg->reg16_ctrl1);
	ret += chip_reg_read_byte(CHG_REG17, 0xFF, &chg->reg17_ctrl2);
	ret += chip_reg_read_byte(CHG_REG18, 0xFF, &chg->reg18_ctrl3);
	ret += chip_reg_read_byte(CHG_REG19, 0xFF, &chg->reg19_ctrl4);
	ret += chip_reg_read_byte(CHG_REG1A, 0xFF, &chg->reg1A_ctrl5);

	ret += chip_reg_read_byte(CHG_REG16, 0xFF, &chg->reg16_ctrl1);
	ret += chip_reg_read_byte(CHG_REG20, 0xFF, &chg->reg20_state); // Charger_Status
	ret += chip_reg_read_byte(CHG_REG21, 0xFF, &chg->reg21_state); // Charger_Status
	ret += chip_reg_read_byte(CHG_REG22, 0xFF, &chg->reg22_fault);
	ret += chip_reg_read_byte(CHG_REG23, 0xFF, &chg->reg23_flag);
	ret += chip_reg_read_byte(CHG_REG24, 0xFF, &chg->reg24_flag);
	ret += chip_reg_read_byte(CHG_REG25, 0xFF, &chg->reg25_fault);
	ret += chip_reg_read_byte(CHG_REG2B, 0xFF, &chg->reg2B_adcCfg);

	if (chg->reg2B_adcCfg & 0x80) { // ADC EN PS:非充电状态 电芯电压小于3.0V时，ADC不工作, 进入HIZ模式，ADC 不工作
        ret += chip_reg_read_word(CHG_REG2D, IBUS_ADC_VALUE_MASK, &dat);
		calc_tmp = binary_to_origin((int16_t)dat, 0x4000, 0x7FFF, IBUS_ADC_VALUE_SHIFT);
		chg->bus_curr = (int16_t)(calc_tmp * 25 / 10); // mA

        ret += chip_reg_read_word(CHG_REG2F, IBAT_ADC_VALUE_MASK, &dat);
		calc_tmp = binary_to_origin((int16_t)dat, 0x1000, 0x1FFF, IBAT_ADC_VALUE_SHIFT);
		chg->bat_curr = (int16_t)(calc_tmp * 5); // mA
        
        ret += chip_reg_read_word(CHG_REG31, VBUS_ADC_VALUE_MASK, &dat);
        calc_tmp = (int16_t)(dat >> VBUS_ADC_VALUE_SHIFT);
		chg->bus_volt = (uint16_t)((int32_t)calc_tmp * 5); // mV
		if (chg->bus_volt < 10)
			chg->bus_volt = 0; // mV

        ret += chip_reg_read_word(CHG_REG35, VBAT_ADC_VALUE_MASK, &dat);
        calc_tmp = (uint16_t)(dat >> VBAT_ADC_VALUE_SHIFT);
		chg->bat_volt = (uint16_t)((int32_t)calc_tmp * 125 / 100); // mV
	} else {
		if ((chg->reg20_state & 0x80) || ((chg->reg16_ctrl1 & HIZ_MODE_MASK) == HIZ_MODE_ENABLE)) { // Vbus input flag
	    	ret += chip_reg_update_byte(CHG_REG2C, 0xFF, 0x0F);
	    	ret += chip_reg_update_byte(CHG_REG2B, 0xFF, 0x81); // Reactivate the ADC conversion
		}
	}
#endif

	static uint8_t sta = 0;

	switch (sta) {
	case 0:
		sta++;
		ret += chip_reg_read_byte(CHG_REG16, 0xFF, &chg->reg16_ctrl1);
		ret += chip_reg_read_byte(CHG_REG20, 0xFF, &chg->reg20_state); // Charger_Status
		ret += chip_reg_read_byte(CHG_REG21, 0xFF, &chg->reg21_state); // Charger_Status
		ret += chip_reg_read_byte(CHG_REG22, 0xFF, &chg->reg22_fault);
		ret += chip_reg_read_byte(CHG_REG23, 0xFF, &chg->reg23_flag);
		ret += chip_reg_read_byte(CHG_REG24, 0xFF, &chg->reg24_flag);
		ret += chip_reg_read_byte(CHG_REG25, 0xFF, &chg->reg25_fault);
		break;
	case 1:
		sta = 0;
		/* check ADC EN, PS:非充电状态 电芯电压小于3.0V，或进入HIZ模式，ADC需重新触发 */
		ret += chip_reg_read_byte(CHG_REG2B, 0xFF, &chg->reg2B_adcCfg);
		if (!(chg->reg2B_adcCfg & ADC_EN_MASK)) {
			if ((chg->reg20_state & PG_STAT_MASK) || // Vbus input, PG_STAT_VBUS_ABOVE_PG_TH
				(chg->reg16_ctrl1 & HIZ_MODE_MASK)) { // HIZ_MODE_ENABLE
				ret += chip_reg_update_byte(CHG_REG2C, 0xFF, 0x0F);
				ret += chip_reg_update_byte(CHG_REG2B, 0xFF, 0x81); // Reactivate the ADC conversion
			}
			break ;
		}
		
		ret += chip_reg_read_word(CHG_REG2D, IBUS_ADC_VALUE_MASK, &dat);
		calc_tmp = binary_to_origin((int16_t)dat, 0x4000, 0x7FFF, IBUS_ADC_VALUE_SHIFT);
		chg->bus_curr = (int16_t)(calc_tmp * 25 / 10); // mA
		
		ret += chip_reg_read_word(CHG_REG2F, IBAT_ADC_VALUE_MASK, &dat);
		calc_tmp = binary_to_origin((int16_t)dat, 0x1000, 0x1FFF, IBAT_ADC_VALUE_SHIFT);
		chg->bat_curr = (int16_t)(calc_tmp * 5); // mA
		
		ret += chip_reg_read_word(CHG_REG31, VBUS_ADC_VALUE_MASK, &dat);
		dat >>= VBUS_ADC_VALUE_SHIFT;
		chg->bus_volt = (uint16_t)((uint32_t)dat * 5); // mV
		if (chg->bus_volt <= 10)
			chg->bus_volt = 0; // mV
		
		ret += chip_reg_read_word(CHG_REG35, VBAT_ADC_VALUE_MASK, &dat);
		dat >>= VBAT_ADC_VALUE_SHIFT;
		chg->bat_volt = (uint16_t)((uint32_t)dat * 125 / 100); // mV
		break;
	default:
		sta = 0;
		break;
	}

	return ret; // RET_SUCCESS(0);RET_FAILURE(-1);
}
/***************************************************************************************/
/**
  * @brief  初始化充电IC硬件接口
  * @param  None
  * @return None
  * @note   None
  */
int driver_charge_ic_init(void)
{
    int ret = 0;
    uint8_t deviceID = 0;

	gs_chg.xSemaphore = xSemaphoreCreateMutex(); // Create  Mutex

//	chg_reset_all_reg();
//	chg_mDelay(2);
	ret += chg_wdg_timer_cfg(WDG_DISABLE);

	for (uint8_t i=0; i<5; i++) {
		ret += chip_reg_read_byte(CHG_REG3F, 0xFF, &deviceID); // 0x04
		if (deviceID == BQ25638_DEV_ID) {
			break;
		} else {
			sm_log(SM_LOG_ERR,"bq25638 chip err!\n");
			return RET_FAILURE;
		}
		chg_mDelay(5);
	}

	sm_log(SM_LOG_INFO,"bq25638 id:0x%02x\n", deviceID);

	ret += chg_set_bat_curr_limit(PRE_INIT_IBAT); // 初始化配置电芯充电电流为接近预充电流:240mA, Step: 80mA
	ret += chg_set_bat_volt_limit(PRE_INIT_VBAT); // 初始化配置电芯充电电压为最大session数对应电压：3840mV,Step: 10mV
	ret += chip_reg_update_byte(CHG_REG15, VINDPM_BAT_TRACK_MASK, VINDPM_DISABLE); // 关闭track; Vbat+0.35V(default)
	ret += chg_set_input_volt_limit(PRE_INIT_VBUS); // Vbus最小大输入电压4600mV
	ret += chg_set_input_curr_limit(PRE_INIT_IBUS); // Vbus最大输入电流3000mA
	ret += chg_set_pre_charge_curr(PRE_INIT_IPRE); // 预充电流:200mA
	ret += chg_set_term_charge_curr(PRE_INIT_ITERM); // 截止电流100mA
#if 0
	chg_set_min_sys_volt_limit(3520u); // default:3520mV
	chg_batfet_ctrl(DEV_ENTER_IDLE_MODE);
	chg_external_Ilim_pin_ctrl(1); // default: enable
	chg_bat_cc_start_volt(BAT_PRECHG_VOLT_TH_3V)); // default:3V
#endif
	ret += chip_reg_update_byte(CHG_REG18, SYS_PWR_RST_MASK, SYS_PWR_RST_FORCE);
	ret += chip_reg_update_byte(CHG_REG18, BATFET_DELAY_MASK, BATFET_ACTION_DELAY_24MS); // delay 24ms enter BATFET mode
	ret += chip_reg_update_byte(CHG_REG1C, TS_IGNORE_MASK, TS_IGNORE); //Ignore the TS feedback
	ret += chip_reg_update_byte(CHG_REG17, 0xFF, 0x73); // Frequency Dither 3X, EMI: reduce drive strength three steps
	// REG0x14 use default:Pre-charge 2.3hrs,Fast charge 14hrs,enable timer
	ret += chip_reg_update_byte(CHG_REG2C, 0xFF, 0x0F); // select adc channel
	ret += chip_reg_update_byte(CHG_REG2B, 0xFF, 0x81); // ADC_EN | ADC_SAMPLE_RESOLUTION_11BIT | ADCIN_ADC_DISABLE

	if (!Cy_GPIO_Read(GPIO_PRT1, 4)) { // if usb insert
		ret += chg_control(CHG_EN);
	} else {
		ret += chg_control(CHG_DIS);
	}
	gs_chg.userConfig = 1;

    return ret; // RET_SUCCESS(0);RET_FAILURE(-1);
}

/**
  * @brief  去初始化充电IC硬件接口
  * @param  None
  * @return None
  * @note   None
  */
int driver_charge_ic_deinit(void)
{
    int ret = 0;

	xSemaphoreTake(gs_chg.xSemaphore, portMAX_DELAY);

//	ret += chg_control(CHG_DIS); // disable charge
	ret += chip_reg_update_byte(CHG_REG2B, ADC_EN_MASK, (ADC_DIS << ADC_EN_SHIFT));
	gs_chg.userConfig = 0;

	xSemaphoreGive(gs_chg.xSemaphore);
    vSemaphoreDelete(gs_chg.xSemaphore);

    return ret;
}

/**
  * @brief  对充电读操作
  * @param  pBuf:   要读入的数据
            len:    要读入数据的长度
  * @return 0：成功，-1：失败
  * @note
  */
int driver_charge_ic_read(uint8_t *pBuf, uint16_t len)
{
	int ret = 0;

    if(len==0) {
        return -1;
    }
    if(pBuf == NULL) {
        return -1;
    }

	xSemaphoreTake(gs_chg.xSemaphore, portMAX_DELAY);

	ret = chg_dat_get((ChgInfo_t *)pBuf);

    xSemaphoreGive(gs_chg.xSemaphore);

    return ret;
}

/**
  * @brief  对充电写操作
  * @param  pBuf:   要读入的数据
            len:    要读入数据的长度
  * @return 0：成功，-1：失败
  * @note
  */
int driver_charge_ic_write(uint8_t *pBuf, uint16_t len)
{
	int ret = 0;
	uint16_t val = 0;

    if(len==0) {
        return -1;
    }
    if(pBuf == NULL) {
        return -1;
    }

	xSemaphoreTake(gs_chg.xSemaphore, portMAX_DELAY);

	switch (pBuf[0]) {
	case CHG_SET_CTRL: // enable or disable charge
		ret += chg_control((charge_en_e)pBuf[1]);
		break;

	case CHG_SET_HIZ: // enable or disable HIZ mode
		ret += chg_HIZ_mode((charge_en_e)pBuf[1]);
		if (pBuf[1] == 0) { // 退出HIZ，延时等待充电IC系统恢复
			chg_mDelay(10);
		}
//		ret += chip_reg_update_byte(CHG_REG2C, 0xFF, 0x0F);
//		ret += chip_reg_update_byte(CHG_REG2B, 0xFF, 0x81); // Reactivate the ADC conversion
		break;

	case CHG_SET_IBAT: // setup battery curr
		val = (uint16_t)pBuf[1] << 8 | (uint16_t)pBuf[2];
		if (val > 3000u) {val = 3000u;} // max:2560mA,适当放开限制范围，方便调试
		ret += chg_set_bat_curr_limit(val); // iniValTbl[STEP1_CHG_CURR]
		break;
		
	case CHG_SET_VBAT: // setup battery volt
		val = (uint16_t)pBuf[1] << 8 | (uint16_t)pBuf[2];
		if (val > 4400u) {val = 4400u;} // max:4400mV
		ret += chg_set_bat_volt_limit(val); // iniValTbl[STEP1_CHG_VOLT])
		break;
		
	case CHG_SET_INIT: // init 
		ret += chg_wdg_timer_cfg(WDG_DISABLE);
		//ret += chg_set_bat_curr_limit(PRE_INIT_IBAT); // 初始化配置电芯充电电流为接近预充电流:240mA, Step: 80mA
		//ret += chg_set_bat_volt_limit(PRE_INIT_VBAT); // 初始化配置电芯充电电压为最大session数对应电压：3840mV,Step: 10mV
		ret += chip_reg_update_byte(CHG_REG15, VINDPM_BAT_TRACK_MASK, VINDPM_DISABLE); // 关闭track; Vbat+0.35V(default)
		ret += chg_set_input_volt_limit(PRE_INIT_VBUS); // Vbus最小大输入电压4600mV
		ret += chg_set_input_curr_limit(PRE_INIT_IBUS); // Vbus最大输入电流3000mA
		ret += chg_set_pre_charge_curr(PRE_INIT_IPRE); // 预充电流:200mA
		ret += chg_set_term_charge_curr(PRE_INIT_ITERM); // 截止电流100mA
#if 0
		chg_set_min_sys_volt_limit(3520u); // default:3520mV
		chg_batfet_ctrl(DEV_ENTER_IDLE_MODE);
		chg_external_Ilim_pin_ctrl(1); // default: enable
		chg_bat_cc_start_volt(BAT_PRECHG_VOLT_TH_3V)); // default:3V
#endif
		ret += chip_reg_update_byte(CHG_REG18, SYS_PWR_RST_MASK, SYS_PWR_RST_FORCE);
		ret += chip_reg_update_byte(CHG_REG18, BATFET_DELAY_MASK, BATFET_ACTION_DELAY_24MS); // delay 24ms enter BATFET mode
		ret += chip_reg_update_byte(CHG_REG1C, TS_IGNORE_MASK, TS_IGNORE); //Ignore the TS feedback
		ret += chip_reg_update_byte(CHG_REG17, 0xFF, 0x73); // Frequency Dither 3X, EMI: reduce drive strength three steps
		// REG0x14 use default:Pre-charge 2.3hrs,Fast charge 14hrs,enable timer
		ret += chip_reg_update_byte(CHG_REG2C, 0xFF, 0x0F); // select adc channel
		ret += chip_reg_update_byte(CHG_REG2B, 0xFF, 0x81); // ADC_EN | ADC_SAMPLE_RESOLUTION_11BIT | ADCIN_ADC_DISABLE
		ret += chg_control((charge_en_e)pBuf[1]);
		break;
		
	case CHG_SET_SHIP_MODE: // enter ship mode
		ret += chg_batfet_ctrl(DEV_ENTER_ULTRA_LOW_POWER_MODE);
		break;

	case CHG_SET_POWER_RST: // power rst
		ret += chg_batfet_ctrl(DEV_ENTER_SYS_POWER_RST);
		break;

//    case CHG_SET_DEBUG:
//		ret = RET_SUCCESS;
//		break;

	default:
		ret = RET_FAILURE;
		break;
    }
    xSemaphoreGive(gs_chg.xSemaphore);

    return ret;
}

/**************************************************************************/
#if 0
	uint8_t i=0;
	uint16_t  reg = 0;
	for (i=CHG_REG02; i<CHG_REG12; i+=2) {
		chip_reg_read_word(i, 0xFFFF, &reg); // 0x04
		sm_log(SM_LOG_ERR,"bq25638 reg0x%02x=0x%04x\n",i,reg);
		chg_mDelay(10);
	}

	for (i=CHG_REG14; i<CHG_REG1C; i++) {
		ret += chip_reg_read_byte(i, 0xFF, &reg); // 0x04
		sm_log(SM_LOG_ERR,"bq25638 reg0x%02x=0x%02x\n",i,reg);
		chg_mDelay(10);
	}
#endif
