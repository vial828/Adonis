#ifndef ___BQ27426_H___
#define ___BQ27426_H___

#include <stdint.h>
#include "bq27426_port.h"

#define BQ27426_I2C_TIMEOUT 2000

///////////////////////
// General Constants //
///////////////////////
#define BQ27426_UNSEAL_KEY	0x8000 // Secret code to unseal the BQ27426-G1A
#define BQ27426_DEVICE_ID	0x0426 // Default device ID

///////////////////////
// Standard Commands //
///////////////////////
// The fuel gauge uses a series of 2-byte standard commands to enable system 
// reading and writing of battery information. Each command has an associated
// sequential command-code pair.
#define BQ27426_COMMAND_CONTROL			0x00 // Control()
#define BQ27426_COMMAND_TEMP			0x02 // Temperature()
#define BQ27426_COMMAND_VOLTAGE			0x04 // Voltage()
#define BQ27426_COMMAND_FLAGS			0x06 // Flags()
#define BQ27426_COMMAND_NOM_CAPACITY	0x08 // NominalAvailableCapacity()
#define BQ27426_COMMAND_AVAIL_CAPACITY	0x0A // FullAvailableCapacity()
#define BQ27426_COMMAND_REM_CAPACITY	0x0C // RemainingCapacity()
#define BQ27426_COMMAND_FULL_CAPACITY	0x0E // FullChargeCapacity()
#define BQ27426_COMMAND_AVG_CURRENT		0x10 // AverageCurrent()
#define BQ27426_COMMAND_STDBY_CURRENT	0x12 // StandbyCurrent()
#define BQ27426_COMMAND_MAX_CURRENT		0x14 // MaxLoadCurrent()
#define BQ27426_COMMAND_AVG_POWER		0x18 // AveragePower()
#define BQ27426_COMMAND_SOC				0x1C // StateOfCharge()
#define BQ27426_COMMAND_INT_TEMP		0x1E // InternalTemperature()
#define BQ27426_COMMAND_SOH				0x20 // StateOfHealth()
#define BQ27426_COMMAND_REM_CAP_UNFL	0x28 // RemainingCapacityUnfiltered()
#define BQ27426_COMMAND_REM_CAP_FIL		0x2A // RemainingCapacityFiltered()
#define BQ27426_COMMAND_FULL_CAP_UNFL	0x2C // FullChargeCapacityUnfiltered()
#define BQ27426_COMMAND_FULL_CAP_FIL	0x2E // FullChargeCapacityFiltered()
#define BQ27426_COMMAND_SOC_UNFL		0x30 // StateOfChargeUnfiltered()

//手册未列出寄存器
#define BQ27426_COMMAND_QMAX            0x16
#define BQ27426_COMMAND_PRESENT_DOD     0x1a
#define BQ27426_COMMAND_OCV_CURRENT     0x22
#define BQ27426_COMMAND_OCV_VOLTAGE     0x24
#define BQ27426_COMMAND_SIMULTANEOUSL   0x26
#define BQ27426_COMMAND_TRUE_SOC        0x30
#define BQ27426_COMMAND_DOD0            0x66
#define BQ27426_COMMAND_DODATEOC        0x68
#define BQ27426_COMMAND_TRUE_REM_CAP    0x6a
#define BQ27426_COMMAND_PASSED_CHARGE   0x6c
#define BQ27426_COMMAND_QSTART          0x6e
#define BQ27426_COMMAND_DOD_FINAL       0x70

//////////////////////////
// Control Sub-commands //
//////////////////////////
// Issuing a Control() command requires a subsequent 2-byte subcommand. These
// additional bytes specify the particular control function desired. The 
// Control() command allows the system to control specific features of the fuel
// gauge during normal operation and additional features when the device is in 
// different access modes.
#define BQ27426_CONTROL_STATUS			0x00
#define BQ27426_CONTROL_DEVICE_TYPE		0x01
#define BQ27426_CONTROL_FW_VERSION		0x02
#define BQ27426_CONTROL_DM_CODE			0x04
#define BQ27426_CONTROL_PREV_MACWRITE	0x07
#define BQ27426_CONTROL_CHEM_ID			0x08
#define BQ27426_CONTROL_BAT_INSERT		0x0C
#define BQ27426_CONTROL_BAT_REMOVE		0x0D
#define BQ27426_CONTROL_SET_HIBERNATE	0x11
#define BQ27426_CONTROL_CLEAR_HIBERNATE	0x12
#define BQ27426_CONTROL_SET_CFGUPDATE	0x13
#define BQ27426_CONTROL_SMOOTH_SYNC		0x19
#define BQ27426_CONTROL_SHUTDOWN_ENABLE	0x1B
#define BQ27426_CONTROL_SHUTDOWN		0x1C
#define BQ27426_CONTROL_SEALED			0x20
#define BQ27426_CONTROL_PULSE_SOC_INT	0x23
#define BQ27426_CONTROL_RESET			0x41
#define BQ27426_CONTROL_SOFT_RESET		0x42
#define BQ27426_CONTROL_EXIT_CFGUPDATE	0x43
#define BQ27426_CONTROL_EXIT_RESIM		0x44

///////////////////////////////////////////
// Control Status Word - Bit Definitions //
///////////////////////////////////////////
// Bit positions for the 16-bit data of CONTROL_STATUS.
// CONTROL_STATUS instructs the fuel gauge to return status information to 
// Control() addresses 0x00 and 0x01. The read-only status word contains status
// bits that are set or cleared either automatically as conditions warrant or
// through using specified subcommands.
#define BQ27426_STATUS_SHUTDOWNEN	(1<<15)
#define BQ27426_STATUS_WDRESET		(1<<14)
#define BQ27426_STATUS_SS			(1<<13)
#define BQ27426_STATUS_CALMODE		(1<<12)
#define BQ27426_STATUS_CCA			(1<<11)
#define BQ27426_STATUS_BCA			(1<<10)
#define BQ27426_STATUS_QMAX_UP		(1<<9)
#define BQ27426_STATUS_RES_UP		(1<<8)
#define BQ27426_STATUS_INITCOMP		(1<<7)
#define BQ27426_STATUS_HIBERNATE	(1<<6)
#define BQ27426_STATUS_SLEEP		(1<<4)
#define BQ27426_STATUS_LDMD			(1<<3)
#define BQ27426_STATUS_RUP_DIS		(1<<2)
#define BQ27426_STATUS_VOK			(1<<1)

////////////////////////////////////
// Flag Command - Bit Definitions //
////////////////////////////////////
// Bit positions for the 16-bit data of Flags()
// This read-word function returns the contents of the fuel gauging status
// register, depicting the current operating status.
#define BQ27426_FLAG_OT			(1<<15)
#define BQ27426_FLAG_UT			(1<<14)
#define BQ27426_FLAG_FC			(1<<9)
#define BQ27426_FLAG_CHG		(1<<8)
#define BQ27426_FLAG_OCVTAKEN	(1<<7)
#define BQ27426_FLAG_ITPOR		(1<<5)
#define BQ27426_FLAG_CFGUPMODE	(1<<4)
#define BQ27426_FLAG_BAT_DET	(1<<3)
#define BQ27426_FLAG_SOC1		(1<<2)
#define BQ27426_FLAG_SOCF		(1<<1)
#define BQ27426_FLAG_DSG		(1<<0)

////////////////////////////
// Extended Data Commands //
////////////////////////////
// Extended data commands offer additional functionality beyond the standard
// set of commands. They are used in the same manner; however, unlike standard
// commands, extended commands are not limited to 2-byte words.
#define BQ27426_EXTENDED_DATACLASS	0x3E // DataClass()
#define BQ27426_EXTENDED_DATABLOCK	0x3F // DataBlock()
#define BQ27426_EXTENDED_BLOCKDATA	0x40 // BlockData()
#define BQ27426_EXTENDED_CHECKSUM	0x60 // BlockDataCheckSum()
#define BQ27426_EXTENDED_CONTROL	0x61 // BlockDataControl()

////////////////////////////////////////
// Configuration Class, Subclass ID's //
////////////////////////////////////////
// To access a subclass of the extended data, set the DataClass() function
// with one of these values.
// Configuration Classes
#define BQ27426_ID_SAFETY			2   // Safety
#define BQ27426_ID_CHG_TERMINATION	36  // Charge Termination
#define BQ27426_ID_CONFIG_DATA		48  // Data
#define BQ27426_ID_DISCHARGE		49  // Discharge
#define BQ27426_ID_REGISTERS		64  // Registers
// Gas Gauging Classes
#define BQ27426_ID_IT_CFG			80  // IT Cfg
#define BQ27426_ID_CURRENT_THRESH	81  // Current Thresholds
#define BQ27426_ID_STATE			82  // State
// Ra Tables Classes
#define BQ27426_ID_R_A_RAM			89  // R_a RAM
// Calibration Classes
#define BQ27426_ID_CALIB_DATA		104 // Data
#define BQ27426_ID_CC_CAL			105 // CC Cal
#define BQ27426_ID_CURRENT			107 // Current
// Security Classes
#define BQ27426_ID_CODES			112 // Codes

/////////////////////////////////////////
// OpConfig Register - Bit Definitions //
/////////////////////////////////////////
// Bit positions of the OpConfig Register
#define BQ27426_OPCONFIG_BIE      (1<<13)
#define BQ27426_OPCONFIG_BI_PU_EN (1<<12)
#define BQ27426_OPCONFIG_GPIOPOL  (1<<11)
#define BQ27426_OPCONFIG_SLEEP    (1<<5)
#define BQ27426_OPCONFIG_RMFCC    (1<<4)
#define BQ27426_OPCONFIG_BATLOWEN (1<<2)
#define BQ27426_OPCONFIG_TEMPS1   (1<<1)
#define BQ27426_OPCONFIG_TEMPS0   (1<<0)

/**************************************************************/
// Parameters for the current() function, to specify which current to read
typedef enum {
	AVG,  // Average Current (DEFAULT)
	STBY, // Standby Current
	MAX_CURENT   // Max Current
} current_measure;

// Parameters for the capacity() function, to specify which capacity to read
typedef enum {
	REMAIN,     // Remaining Capacity (DEFAULT)
	FULL,       // Full Capacity
	AVAIL,      // Available Capacity
	AVAIL_FULL, // Full Available Capacity
	REMAIN_F,   // Remaining Capacity Filtered
	REMAIN_UF,  // Remaining Capacity Unfiltered
	FULL_F,     // Full Capacity Filtered
	FULL_UF,    // Full Capacity Unfiltered
	DESIGN      // Design Capacity
} capacity_measure;

// Parameters for the soc() function
typedef enum {
	FILTERED,  // State of Charge Filtered (DEFAULT)
	UNFILTERED // State of Charge Unfiltered
} soc_measure;

// Parameters for the soh() function
typedef enum {
	PERCENT,  // State of Health Percentage (DEFAULT)
	SOH_STAT  // State of Health Status Bits
} soh_measure;

// Parameters for the temperature() function
typedef enum {
	BATTERY,      // Battery Temperature (DEFAULT)
	INTERNAL_TEMP // Internal IC Temperature
} temp_measure;

// Parameters for the setGPOUTFunction() funciton
typedef enum {
	SOC_INT, // Set GPOUT to SOC_INT functionality
	BAT_LOW  // Set GPOUT to BAT_LOW functionality
} gpout_function;
	
// fuel gauge chip's battery state
typedef struct BatInfo_t {
	uint8_t init;

	uint16_t ctrl_status; // control()
	uint16_t flags;		  // flags()

	uint16_t voltage;	 // battery voltage (mV)
	int16_t current;	 // charge or discharge current (mA)
	int16_t power;		 // average power draw (mW)
	int16_t temperature; // battery temperature (℃)
	int16_t pcbtemperature; // battery temperature (℃)

	uint16_t soc;	   // state-of-capacity (%)
	uint16_t true_soc; //    (%)
	uint16_t remap_soc; //    (%)
	uint16_t soh;	   // state-of-health (%)

	uint16_t passed_charge;	 // passed charge (mAh)
	uint16_t capacityFull;	 // full capacity (mAh)
	uint16_t capacityRemain; // remaining capacity (mAh)

	uint16_t ture_rm;	   // (mAh)
	uint16_t ture_fcc;	   // (mAh)
	uint16_t filtered_rm;  // (mAh)
	uint16_t filtered_fcc; // (mAh)

	uint16_t qmax;		   // QMAX          (NUM or mAh)    0x16
	uint16_t dod0;		   // DOD0          (NUM)  0x66
	uint16_t dodateoc;	   // DODatEOC      (NUM)  0x68
	uint16_t paresent_dod; // ParesentDOD   (NUM)  0x1a

	uint8_t bat_id;  //电芯型号id
	uint32_t cycle; // charge-discharge cycle times
	uint16_t taper_volt;
} BatInfo_t;


/**************************************************************/
/**************************************************************/
int8_t bq27426_set_taper_rate(uint16_t taper_rate);
int8_t bq27426_set_taper_voltage(uint16_t taper_voltage);
int8_t bq27426_set_opconfig(uint8_t enSleep,uint8_t enExNtc);
int8_t bq27426_get_design_capacity(uint16_t *design_cap);
int8_t bq27426_get_taper_rate(uint16_t *taper_rate);
int8_t bq27426_get_taper_voltage(uint16_t *taper_volt);
uint16_t bq27426_voltage(void);
int16_t bq27426_current(current_measure type);
uint16_t bq27426_capacity(capacity_measure type);
int16_t bq27426_power(void);
uint16_t bq27426_soc(soc_measure type);
uint8_t bq27426_soh(soh_measure type);
int16_t bq27426_temperature(temp_measure type);
uint16_t bq27426_passed_charge(void);
int8_t bq27426_shutdown(void); //shutdown
int8_t bq27426_read_param(BatInfo_t *fg_info); //读取IC工作参数

int8_t bq27426_get_init_status(void); //初始化状态
void bq27426_set_smooth_sync(bool flg);
int8_t bq27426_init(uint8_t mode); // 初始化
int8_t bq27426_deinit(void);	   // 休眠前调用一次
int8_t bq27426_save_gmfs(void);	   // 从ic读取数据->保存到本地
int8_t bq27426_flash_data_clear(void);//清除本地数据
int16_t bq27426_passed_mah(uint8_t type); //电量统计  0-开始；1-结束，并返回（0-1）电量变化  必须成对使用

void bq27426_print_gmfs(void);

#endif
