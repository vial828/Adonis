/**
  ******************************************************************************
  * @file    bq25638.h
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

#ifndef __DRIVER_CHARGE_IC_H
#define __DRIVER_CHARGE_IC_H

#include <stdint.h>
#include <stdio.h>

#include "driver_i2c.h"

#define BQ25638_7BIT_DEV_I2IC_ADDR	(0x6B)

/************************* 16bit reg **************************/
#define CHG_REG02					(0x02)
#define CHG_CURR_LIMIT_MASK				(0x0FC0)
#define CHG_CURR_LIMIT_SHIFT			(6)
/**************************************************************/
#define CHG_REG04					(0x04)
#define CHG_VOLT_LIMIT_MASK				(0x0FF8)
#define CHG_VOLT_LIMIT_SHIFT			(3)
/**************************************************************/
#define CHG_REG06                 	(0x06)
#define INPUT_CURR_LIMIT_MASK         	(0x0FF0)
#define INPUT_CURR_LIMIT_SHIFT         	(4)
/**************************************************************/
#define CHG_REG08                 	(0x08)
#define INPUT_VOLT_LIMIT_MASK           (0x3FE0)
#define INPUT_VOLT_LIMIT_SHIFT          (5)
/**************************************************************/
#define CHG_REG0A                 	(0x0A)
#define OTG_CURR_LIMIT_MASK				(0x0FF0)
#define OTG_CURR_LIMIT_SHIFT			(4)
/**************************************************************/
#define CHG_REG0C                 	(0x0C)
#define OTG_VOLT_LIMIT_MASK				(0x1FC0)
#define OTG_VOLT_LIMIT_SHIFT			(6)
/**************************************************************/
#define CHG_REG0E                 	(0x0E)
#define MIN_SYS_VOLT_MASK				(0x0FC0)
#define MIN_SYS_VOLT_SHIFT				(6)
/**************************************************************/
#define CHG_REG10					(0x10)
#define CHG_IPRE_MASK              		(0x03F0)
#define CHG_IPRE_SHIFT              	(4)
/**************************************************************/
#define CHG_REG12                 	(0x12)
#define CHG_ITERM_MASK              	(0x03F8)
#define CHG_ITERM_SHIFT             	(3)

/************************* 8bit reg ***************************/
#define CHG_REG14                 	(0x14) // Charge Timer Control

#define STAT_PIN_MASK         			(0x80)
#define STAT_PIN_ENABLE       			(0<<7) // (default)
#define STAT_PIN_DISABLE      			(1<<7)

#define TIM2X_EN_MASK            		(0x08)
#define TIM2X_ENABLE            		(1<<3) // (default)
#define TIM2X_DISABLE           		(0<<3)

#define SAFETY_TMRS_EN_MASK				(0x04)
#define SAFETY_TMRS_ENABLE				(1<<2) // (default)
#define SAFETY_TMRS_DISABLE				(0<<2)

#define PRECHG_TMR_MASK               	(0x02)
#define PRECHG_TMR_2P3H               	(0<<1) // (default)
#define PRECHG_TMR_0P6H               	(1<<1)

#define CHG_TMR_MASK                  	(0x01)
#define CHG_TMR_14H                   	(0<<0) // (default)
#define CHG_TMR_27H                   	(1<<0)
/**************************************************************/
#define CHG_REG15                 		(0x15) // Charger Control 0

#define Q1_FULLON_MASK               	(0x80)
#define Q1_FULLON_BY_IINDPM				(0<<7) // (default)
#define Q1_FULLON_FORCE_15MOHM			(1<<7)

#define Q4_FULLON_MASK               	(0x40)
#define Q4_FULLON_BY_ICHG				(0<<6) // (default)
#define Q4_FULLON_FORCE_7MOHM			(1<<6)

#define TRICKLE_CURR_MASK               (0x20)
#define TRICKLE_CURR_20MA				(0<<5)
#define TRICKLE_CURR_80MA				(1<<5) // (default)

#define TOPOFF_TMR_MASK               	(0x18)
#define TOPOFF_TMR_DISABLE        		(0<<3) // (default)
#define TOPOFF_TMR_17P5_MINS        	(1<<3)
#define TOPOFF_TMR_35_MINS          	(2<<3)
#define TOPOFF_TMR_52_MINS          	(3<<3)

#define TERM_EN_MASK					(0x04)
#define TERM_ENABLE						(1<<2) // (default)
#define TERM_DISABLE					(0<<2)

#define VINDPM_BAT_TRACK_MASK         	(0x02)
#define VINDPM_DISABLE               	(0<<1)
#define VINDPM_BAT_ADD_350MV        	(1<<1) // (default)

#define RECHG_VOLT_MASK                 (0x01)
#define RECHG_VOLT_BELOW_100MV          (0<<0) // (default)
#define RECHG_VOLT_BELOW_200MV          (1<<0)

/**************************************************************/
#define CHG_REG16                 	(0x16) // Charger Control 1

#define BAT_OVP_AUTO_DSCHG_MASK 		(0x80)
#define BAT_OVP_AUTO_DSCHG_ENABLE       (1<<7) // (default)
#define BAT_OVP_AUTO_DSCHG_DISABLE      (0<<7)

#define BAT_FORCE_DSCHG_MASK			(0x40)
#define BAT_PULL_DOWN_ENABLE         	(1<<6)
#define BAT_PULL_DOWN_DISABLE        	(0<<6) // (default)

#define CHG_CFG_MASK              		(0x20)
#define CHG_CFG_ENABLE               	(1<<5) // (default)
#define CHG_CFG_DISABLE              	(0<<5)

#define HIZ_MODE_MASK         			(0x10)
#define HIZ_MODE_ENABLE               	(1<<4)
#define HIZ_MODE_DISABLE              	(0<<4) // (default)

#define PMID_DSCHG_MASK        			(0x08)
#define PMID_PULL_DOWN_ENABLE           (1<<3)
#define PMID_PULL_DOWN_DISABLE          (0<<3) // (default)

#define WDG_RESET_MASK                  (0x04)
#define WDG_NORMAL                      (0<<2) // (default)
#define WDG_RESET                      	(1<<2)

#define WDG_TIMER_MASK                	(0x03)
#define WDG_DISABLE                   	(0<<0)
#define WDG_40S                       	(1<<0) // (default)
#define WDG_80S                       	(2<<0)
#define WDG_160S                      	(3<<0)
/**************************************************************/
#define CHG_REG17                 	(0x17) // Charger Control 2

#define RESET_REG_MASK                	(0x80)
#define REG_NORMAL                     	(0<<7) // (default)
#define REG_RESET                      	(1<<7)

#define TERMAL_REGULATION_MASK          (0x40)
#define TERMAL_THRESHOLD_60C            (0<<6)
#define TERMAL_THRESHOLD_120C           (1<<6) // (default)

#define DITHER_EN_MASK          		(0x30) // (default)
#define SET_CONVERTER_DRIVE_MASK        (0x0C) // (default)
#define SET_BATFET_DRIVE_MASK          	(0x02) // (default)

#define VBUS_OVP_MASK                 	(0x01)
#define VBUS_OVP_6P3V                 	(0<<0)
#define VBUS_OVP_18P5V                	(1<<0) // (default)
/**************************************************************/
#define CHG_REG18                 	(0x18) // Charger Control 3

#define OTG_MODE_MASK                 	(0x40)
#define OTG_MODE_ENABLE               	(1<<6)
#define OTG_MODE_DISABLE              	(0<<6) // (default)

#define PFM_OTG_CFG_MASK         		(0x20)
#define PFM_OTG_ENABLE                	(0<<5) // (default)
#define PFM_OTG_DISABLE               	(1<<5)

#define PFM_FWD_CFG_MASK             	(0x10)
#define PFM_FWD_ENABLE                	(0<<4) // (default)
#define PFM_FWD_DISABLE               	(1<<4)

#define SYS_PWR_RST_MASK         		(0x08)
#define SYS_PWR_RST_BY_VBUS_REMOVED		(0<<3) // (default)
#define SYS_PWR_RST_FORCE              	(1<<3)

#define BATFET_DELAY_MASK             	(0x04)
#define BATFET_ACTION_DELAY_24MS       	(0<<2)
#define BATFET_ACTION_DELAY_12S        	(1<<2) // (default)

#define BATFET_CTRL_MASK     			(0x03)
#define DEV_ENTER_IDLE_MODE             (0<<0) // (default)
#define DEV_ENTER_SHUTDOWN_MODE         (1<<0)
#define DEV_ENTER_ULTRA_LOW_POWER_MODE  (2<<0)
#define DEV_ENTER_SYS_POWER_RST         (3<<0)
/**************************************************************/
#define CHG_REG19                 	(0x19) // Charger Control 4

#define IBAT_DSCHG_OCP_MASK           	(0xC0)
#define IBAT_DSCHG_OCP_3A             	(0<<6)
#define IBAT_DSCHG_OCP_6A             	(1<<6)
#define IBAT_DSCHG_OCP_9A             	(2<<6) // (default)

#define VBAT_UVLO_MASK                	(0x20)
#define VBAT_UVLO_2P2V                	(0<<5) // (default)
#define VBAT_UVLO_1P8V                	(1<<5)

#define VBAT_OTG_MIN_MASK             	(0x10)
#define VBAT_OTG_MIN_3V_2P8V          	(0<<4) // (default)
#define VBAT_OTG_MIN_2P4_2P2V         	(1<<4)

#define EXT_ILIM_PIN_CTRL_MASK        	(0x04)
#define EXT_ILIM_PIN_ENABLE             (1<<2) // (default)
#define EXT_ILIM_PIN_DISABLE            (0<<2)

#define FORCE_ICO_MASK                 	(0x02)
#define FORCE_ICO_START                	(1<<1)
#define FORCE_ICO_CLOSED               	(0<<1) // (default)

#define ICO_CTRL_MASK                 	(0x01)
#define ICO_ENABLE                    	(1<<0) // (default)
#define ICO_DISABLE                   	(0<<0)
/**************************************************************/
#define CHG_REG1A                 	(0x1A) // Charger Control 5

#define PG_INDICATOR_TH_MASK          	(0xE0)
#define PG_INDICATOR_TH_3P7V          	(0<<5) // (default)
#define PG_INDICATOR_TH_7P4V          	(1<<5)
#define PG_INDICATOR_TH_8V            	(2<<5)
#define PG_INDICATOR_TH_10P4V         	(3<<5)
#define PG_INDICATOR_TH_11V           	(4<<5)
#define PG_INDICATOR_TH_13P4V         	(5<<5)
#define PG_INDICATOR_TH_14V           	(6<<5)

#define TQON_RST_MASK          			(0x10) // System reset (tQON_RST) control
#define TQON_RST_11S                 	(0<<4) // (default)
#define TQON_RST_21S                	(1<<4)

#define TSM_EXIT_MASK          			(0x08) // Ultra-Low Power Mode exit (tSM_EXIT) control
#define TSM_EXIT_0P7S                	(0<<3) // (default)
#define TSM_EXIT_10P5MS               	(1<<3)

#define FORCE_ISYS_DSCHG_MASK          	(0x40) //SYS pull down current source
#define ISYS_DSCHG_ENABLE              	(1<<2)
#define ISYS_DSCHG_DISABLE            	(0<<2) // (default)

#define BAT_PRECHG_VOLT_TH_MASK       	(0x03) //Battery precharge to fast-charge threshold
#define BAT_PRECHG_VOLT_TH_3V         	(0<<0) // (default)
#define BAT_PRECHG_VOLT_TH_2P8V       	(1<<0)
#define BAT_PRECHG_VOLT_TH_2P7V       	(2<<0)
#define BAT_PRECHG_VOLT_TH_2P5V       	(3<<0)
/**************************************************************/
#define CHG_REG1C                 	(0x1C) // NTC Control 0

#define TS_IGNORE_MASK          		(0x80) //Ignore the TS feedback
#define TS_IGNORE		              	(1<<7)
#define TS_NOT_IGNORE	            	(0<<7) // (default)

#define CHG_RATE_MASK          			(0x60)
#define TS_TH_OTG_HOT_MASK				(0x18)
#define TS_TH_OTG_COLD_MASK				(0x04)
#define TS_TH1_MASK						(0x02)
#define TS_TH6_MASK						(0x01)
/**************************************************************/
#define CHG_REG1D                 	(0x1D) // NTC Control 1
#define CHG_REG1E                 	(0x1E) // NTC Control 2
#define CHG_REG1F                 	(0x1F) // NTC Control 3
/**************************************************************/
#define CHG_REG20                 	(0x20) // Charger Status 0

#define PG_STAT_MASK					(0x80) // Power Good Indicator Status
#define PG_STAT_SHIFT					(7)
#define PG_STAT_VBUS_BELOW_PG_TH		(0)
#define PG_STAT_VBUS_ABOVE_PG_TH		(1)

#define ADC_DONE_STAT_MASK				(0x40)
#define ADC_DONE_STAT_SHIFT				(6)
#define ADC_DONE_STAT_COMPLETE			(1)

#define TREG_STAT_MASK					(0x20) // IC Thermal regulation status
#define TREG_STAT_SHIFT					(5)
#define TREG_STAT_THERMAL_REGULATION	(1)

#define VSYS_STAT_MASK					(0x10) // VSYS Regulation Status (forward mode)
#define VSYS_STAT_SHIFT					(4)
#define VSYS_STAT_IN_VSYSMIN			(1) // BAT < VSYSMIN

#define IINDPM_STAT_MASK				(0x08) // IINDPM status (forward mode) or IOTG status (OTG mode)
#define IINDPM_STAT_SHIFT				(3)
#define IINDPM_STAT_IN_REGULATION		(1) // In IINDPM regulation or IOTG regulation

#define VINDPM_STAT_MASK				(0x04) // VINDPM status (forward mode) or VOTG status (OTG mode)
#define VINDPM_STAT_SHIFT				(2)
#define VINDPM_STAT_IN_REGULATION		(1) // In VINDPM regulation or VOTG regulation

#define SAFETY_TMR_STAT_MASK			(0x02) // Fast charge, trickle charge and pre-charge timer status
#define SAFETY_TMR_STAT_SHIFT			(1)
#define SAFETY_TMR_STAT_EXPIRED			(1) // Safety timer expired

#define WDG_TMR_STAT_MASK				(0x01) // Fast charge, trickle charge and pre-charge timer status
#define WDG_TMR_STAT_SHIFT				(0)
#define WDG_TMR_STAT_EXPIRED			(1) // watchdog timer expired
/**************************************************************/
#define CHG_REG21                 	(0x21) // Charger Status 1

#define ICO_STAT_MASK					(0xC0) // Input Current Optimizer (ICO) Status
#define ICO_STAT_SHIFT					(6)
#define ICO_STAT_DISABLE				(0)
#define ICO_STAT_OPTIMIZATION			(1)
#define ICO_STAT_MAX_I_IN_DETECT		(2)
#define ICO_STAT_ROUTINE_SUSPEND		(3)

#define CHG_STAT_MASK					(0x38)
#define CHG_STAT_SHIFT					(3)
#define CHG_STAT_NOT_CHARGING			(0)
#define CHG_STAT_TRICKLE_CHARGING		(1)
#define CHG_STAT_PRE_CHARGING			(2)
#define CHG_STAT_FAST_CHARGING			(3) // CC mode
#define CHG_STAT_TAPER_CHARGING			(4) // CV mode
#define CHG_STAT_RESERVED				(5)
#define CHG_STAT_TOPOFF_TIM_ACTIVE		(6)
#define CHG_STAT_TERMINATION_DONE		(7)

#define VBUS_STAT_MASK					(0x07)
#define VBUS_STAT_SHIFT					(0)
#define VBUS_STAT_NO_POWERED			(0)
#define VBUS_STAT_UNKNOWN_ADAPTOR		(4) // 100
#define VBUS_STAT_IN_BOOST_OTG			(7) // 111
/**************************************************************/
#define CHG_REG22                 	(0x22) // FAULT0 Status

#define VBUS_FAULT0_MASK				(0x80)
#define VBUS_FAULT0_SHIFT				(7)
#define VBUS_FAULT0_IN_OVP				(1) // Device in over voltage protection

#define BAT_FAULT0_MASK					(0x40)
#define BAT_FAULT0_SHIFT				(6)
#define BAT_FAULT0_IN_DEAD_OR_OVP		(1) // Dead or over-voltage battery detected

#define VSYS_FAULT0_MASK				(0x20) // VSYS under voltage and over voltage status
#define VSYS_FAULT0_SHIFT				(5)
#define VSYS_FAULT0_IN_SC_OR_OVP		(1) // SYS short circuit or over voltage

#define OTG_FAULT0_MASK					(0x10) // OTG under voltage and over voltage status.
#define OTG_FAULT0_SHIFT				(4)
#define OTG_FAULT0_IN_UVP_OR_OVP		(1)

#define TSHUT_FAULT0_MASK				(0x08) // IC temperature shutdown status
#define TSHUT_FAULT0_SHIFT				(3)
#define TSHUT_FAULT0_IN_OTP				(1) // Device in thermal shutdown protection

#define TS_FAULT0_MASK					(0x07) // The TS temperature zone.
#define TS_FAULT0_SHIFT					(0)
#define TS_FAULT0_NORMAL				(0)
#define TS_FAULT0_COLD					(1)
#define TS_FAULT0_HOT					(2)
#define TS_FAULT0_COOL					(3)
#define TS_FAULT0_WARM					(4)
#define TS_FAULT0_PRECOOL				(5)
#define TS_FAULT0_PREWARM				(6)
#define TS_FAULT0_RESERVED				(7)
/**************************************************************/
#define CHG_REG23                 	(0x23) // Charger Flag0

#define PG_FLAG_MASK					(0x80)
#define PG_FLAG_SHIFT					(7)
#define PG_FLAG_TRIG					(1) // PG status changed

#define ADC_DONE_FLAG 					(0x40)
#define ADC_DONE_FLAG_SHIFT				(6)
#define ADC_DONE_FLAG_TRIG				(1) // Conversion completed

#define TREG_FLAG_MASK 					(0x20) // IC Thermal regulation flag
#define TREG_FLAG_SHIFT					(5)
#define TREG_FLAG_TRIG					(1) // TREG signal rising threshold detected

#define VSYS_FLAG_MASK 					(0x10) // VSYS min regulation flag
#define VSYS_FLAG_SHIFT					(4)
#define VSYS_FLAG_TRIG					(1) // Entered or exited VSYS min regulation

#define IINDPM_FLAG_MASK 				(0x08) // IINDPM or IOTG flag
#define IINDPM_FLAG_SHIFT				(3)
#define IINDPM_FLAG_TRIG				(1) // IINDPM signal rising edge detected

#define VINDPM_FLAG_MASK 				(0x04) // VINDPM or VOTG flag
#define VINDPM_FLAG_SHIFT				(2)
#define VINDPM_FLAG_TRIG				(1) // VINDPM regulation signal rising edge detected

#define SAFETY_TMR_FLAG_MASK 			(0x02) // Fast charge, trickle charge and pre-charge timer flag
#define SAFETY_TMR_FLAG_SHIFT			(1)
#define SAFETY_TMR_FLAG_TRIG			(1) // Fast chargeg timer expired rising edge detected

#define WDG_FLAG_MASK 					(0x01) // I2C watchdog timer flag
#define WDG_FLAG_SHIFT					(0)
#define WDG_FLAG_TRIG					(1) // WDG timer signal rising edge detected
/**************************************************************/
#define CHG_REG24                 	(0x24) // Charger Flag1

#define ICO_FLAG_MASK					(0x40) // Input Current Optimizer (ICO) flag
#define ICO_FLAG_SHIFT					(6)
#define ICO_FLAG_TRIG					(1) //  ICO_STAT[1:0] changed (transition to any state)

#define CHG_FLAG_MASK 					(0x08)
#define CHG_FLAG_SHIFT					(3)
#define CHG_FLAG_TRIG					(1) // Charge status changed

#define VBUS_FLAG_MASK 					(0x01)
#define VBUS_FLAG_SHIFT					(0)
#define VBUS_FLAG_TRIG					(1) // VBUS status changed
/**************************************************************/
#define CHG_REG25                 	(0x25) // FAULT1 Flag

#define VBUS_FAULT1_MASK				(0x80)
#define VBUS_FAULT1_SHIFT				(7)
#define VBUS_FAULT1_ENTER_OVP			(1) // Entered VBUS OVP

#define BAT_FAULT1_MASK					(0x40)
#define BAT_FAULT1_SHIFT				(6)
#define BAT_FAULT1_ENTER_DEAD_OR_OVP	(1) // Entered VBAT OVP

#define VSYS_FAULT1_MASK				(0x20) // VSYS over voltage and SYS short flag
#define VSYS_FAULT1_SHIFT				(5)
#define VSYS_FAULT1_ENTER_SC_OR_OVP		(1) // Stopped switching due to system over-voltage or SYS short fault

#define OTG_FAULT1_MASK					(0x10) // OTG under voltage and over voltage status.
#define OTG_FAULT1_SHIFT				(4)
#define OTG_FAULT1_ENTER_UVP_OR_OVP		(1) // Stopped OTG due to VBUS under voltage or over voltage fault

#define TSHUT_FAULT1_MASK				(0x08) // IC thermal shutdown flag
#define TSHUT_FAULT1_SHIFT				(3)
#define TSHUT_FAULT1_ENTER_OTP			(1) //  TS shutdown signal rising threshold detected

#define TS_FAULT1_MASK					(0x01) // TS status flag
#define TS_FAULT1_SHIFT					(0)
#define TS_FAULT1_NORMAL				(0) // A change to TS status was detected
/**************************************************************/
#define CHG_REG26                 	(0x26) // Charger Mask 0

#define CHG0_INT_MASK_					(0xFF)
// 0b : Does produce INT; 1b : Does not produce INT;
#define PG_INT_MASK_BIT					(0x80)
#define ADC_DONE_INT_MASK_BIT			(0x40)
#define TREG_INT_MASK_BIT 				(0x20)
#define VSYS_INT_MASK_BIT 				(0x10)
#define IINDPM_INT_MASK_BIT 			(0x08)
#define VINDPM_INT_MASK_BIT 			(0x04)
#define SAFETY_TMR_INT_MASK_BIT 		(0x02)
#define WDG_INT_MASK_BIT 				(0x01)
/**************************************************************/
#define CHG_REG27                 	(0x27) // Charger Mask 1

#define CHG1_INT_MASK_					(0x49)
// 0b : Does produce INT; 1b : Does not produce INT;
#define ICO_INT_MASK_BIT				(0x40)
#define CHG_INT_MASK_BIT 				(0x08)
#define VBUS_INT_MASK_BIT 				(0x01)
/**************************************************************/
#define CHG_REG28                 	(0x28) // FAULT Mask

#define FAULT1_INT_MASK					(0xF9)
// 0b : Does produce INT; 1b : Does not produce INT;
#define VBUS_FAULT1_INT_MASK_BIT		(0x80)
#define BAT_FAULT1_INT_MASK_BIT			(0x40)
#define VSYS_FAULT1_INT_MASK_BIT		(0x20)
#define OTG_FAULT1_INT_MASK_BIT			(0x10)
#define TSHUT_FAULT1_INT_MASK_BIT		(0x08)
#define TS_FAULT1_INT_MASK_BIT			(0x01)
/************************* 16bit reg **************************/
#define CHG_REG29                 	(0x29) // ICO Current Limit

#define ICO_IINDPM_MASK              	(0x0FF0) // Optimized Input Current Limit when ICO is enabled
#define ICO_IINDPM_SHIFT             	(4)
/**************************************************************/
#define CHG_REG2B                 	(0x2B) // ADC Control

#define ADC_EN_MASK						(0x80)
#define ADC_EN_SHIFT					(7)
#define ADC_EN							(1)
#define ADC_DIS							(0)

#define ADC_RATE_MASK					(0x40)
#define ADC_RATE_SHIFT					(6)
#define ADC_RATE_CONTINUOUS				(0) //  (default)
#define ADC_RATE_ONE_SHOT				(1)

#define ADC_SAMPLE_MASK					(0x30)
#define ADC_SAMPLE_SHIFT				(4)
#define ADC_SAMPLE_RESOLUTION_11BIT		(0)
#define ADC_SAMPLE_RESOLUTION_10BIT		(1)
#define ADC_SAMPLE_RESOLUTION_9BIT		(2)
#define ADC_SAMPLE_RESOLUTION_8BIT		(3) // (default)

#define ADC_AVG_MASK					(0x08)
#define ADC_AVG_SHIFT					(3)
#define ADC_AVG_SINGLE_VALUE			(0) // (default)
#define ADC_AVG_RUNNING					(1)

#define ADC_AVG_INIT_MASK				(0x04)
#define ADC_AVG_INIT_SHIFT				(2)
#define ADC_AVG_INIT_EXIST_VALUE		(0) // (default)
#define ADC_AVG_INIT_NEW_VALUE			(1)

#define ADCIN_ADC_MASK					(0x01) // ADCIN ADC channel disable
#define ADCIN_ADC_SHIFT					(0)
#define ADCIN_ADC_DISABLE				(1)
/**************************************************************/
#define CHG_REG2C                 	(0x2C) // ADC Channel Disable

#define DIS_ADC_CH_MASK					(0xFF)
// 0b : Enable; 1b : Disable;
#define DIS_IBUS_ADC_MASK_BIT			(0x80)
#define DIS_IBAT_ADC_MASK_BIT			(0x40)
#define DIS_VBUS_ADC_MASK_BIT 			(0x20)
#define DIS_VBAT_ADC_MASK_BIT 			(0x10)
#define DIS_VSYS_ADC_MASK_BIT 			(0x08)
#define DIS_TS_ADC_MASK_BIT 			(0x04)
#define DIS_TDIE_ADC_MASK_BIT 			(0x02)
#define DIS_VPMID_ADC_MASK_BIT 			(0x01)
/**************************************************************/
#define CHG_REG2D                 	(0x2D) // IBUS ADC
// range: -5000mA - 5000mA (7830h-7D0h); Bit Step: 2.5mA; 2s Complement
#define IBUS_ADC_VALUE_MASK             (0xFFFE)
#define IBUS_ADC_VALUE_SHIFT            (1)
/**************************************************************/
#define CHG_REG2F                 	(0x2F) // IBAT ADC
// Range: -10000mA-5025mA (1830h-3EDh); Bit Step: 5mA; 2s Complement
#define IBAT_ADC_VALUE_MASK             (0xFFF8)
#define IBAT_ADC_VALUE_SHIFT            (3)
/**************************************************************/
#define CHG_REG31                 	(0x31) // VBUS ADC
// Range: 0mV-20000mV (0h-FA0h); Bit Step: 5mV;
#define VBUS_ADC_VALUE_MASK             (0x7FFC)
#define VBUS_ADC_VALUE_SHIFT            (2)
/**************************************************************/
#define CHG_REG33                 	(0x33) // VPMID ADC
// Range: 0mV-20000mV (0h-FA0h); Bit Step: 5mV;
#define VPMID_ADC_VALUE_MASK            (0x7FFC)
#define VPMID_ADC_VALUE_SHIFT           (2)
/**************************************************************/
#define CHG_REG35                 	(0x35) // VBAT ADC
// Range: 0mV-5000mV (0h-FA0h); Bit Step: 1.25mV;
#define VBAT_ADC_VALUE_MASK            (0x1FFE)
#define VBAT_ADC_VALUE_SHIFT           (1)
/**************************************************************/
#define CHG_REG37                 	(0x37) // VSYS ADC
// Range: 0mV-5000mV (0h-FA0h); Bit Step: 1.25mV;
#define VSYS_ADC_VALUE_MASK            	(0x1FFE)
#define VSYS_ADC_VALUE_SHIFT           	(1)
/**************************************************************/
#define CHG_REG39                 	(0x39) // TS ADC
// Range: 0% - 99.90234375% (0h-3FFh); Bit Step: 0.09765625%;
#define TS_ADC_VALUE_MASK            	(0x0FFF)
#define TS_ADC_VALUE_SHIFT           	(0)
/**************************************************************/
#define CHG_REG3B                 	(0x3B) // TDIE ADC
// Range:-40°C - 150°C (FB0h-12Ch); Bit Step: 0.5°C
#define TDIE_ADC_VALUE_MASK             (0x0FFF)
#define TDIE_ADC_VALUE_SHIFT           	(0)
/**************************************************************/
#define CHG_REG3D                 	(0x3D) // ADCIN ADC
// Range: 0mV - 1000mV (0h-FA0h); Bit Step: 0.25mV
#define ADCIN_ADC_VALUE_MASK             (0x0FFF)
#define ADCIN_ADC_VALUE_SHIFT           (0)
/**************************************************************/
#define CHG_REG3F                 	(0x3F) // Device information
#define BQ25638_DEV_ID				(0x04)
/**************************************************************/
#define CHG_REG80                 	(0x80) // Virtual Control 0
#define CHG_REG81                 	(0x81) // Virtual Control 1
/**************************************************************/
typedef enum {
	CHG_DIS,
	CHG_EN = !CHG_DIS
} charge_en_e;

typedef enum {
	CHG_SET_CTRL = 1,
	CHG_SET_HIZ,
	CHG_SET_IBAT,
	CHG_SET_VBAT,
	CHG_SET_INIT,
	CHG_SET_SHIP_MODE,
	CHG_SET_ADC,
	CHG_SET_POWER_RST,
	CHG_SET_DEBUG
} ChgSet_e;

// chip's adc dat and state
typedef struct ChgInfo_t {
	 int16_t bus_curr; // bus current (mA)
	 int16_t bat_curr; // battery current (mA)
	uint16_t bus_volt; // bus voltage (mV)
	uint16_t bat_volt; // battery voltage (mV)
	uint16_t sys_volt; // sys voltage (mV)
	 int16_t chip_temp; // temperature (℃)
#if 0
	uint8_t reg14_ctrlTimer; // state of charge
	uint8_t reg15_ctrl0;
	uint8_t reg16_ctrl1;
	uint8_t reg17_ctrl2;
	uint8_t reg18_ctrl3;
	uint8_t reg19_ctrl4;
	uint8_t reg1A_ctrl5;
#endif
	uint8_t reg16_ctrl1;
	uint8_t reg20_state; // state of charge
	uint8_t reg21_state;
	uint8_t reg22_fault;
	uint8_t reg23_flag;
	uint8_t reg24_flag;
	uint8_t reg25_fault;
	uint8_t reg2B_adcCfg;
} ChgInfo_t;

#define PRE_INIT_IBAT	(240) // step:80 mA
#define PRE_INIT_VBAT	(4400) // step:10 mV
#define PRE_INIT_VBUS	(4600) // step:40 mA
#define PRE_INIT_IBUS	(3000) // step:20 mA
#define PRE_INIT_IPRE	(200) // step:20 mA
#define PRE_INIT_ITERM	(100) // step:10 mA

/**************************************************************/
extern int driver_charge_ic_init(void);
extern int driver_charge_ic_deinit(void);
extern int driver_charge_ic_read(uint8_t *pBuf, uint16_t len);
extern int driver_charge_ic_write(uint8_t *pBuf, uint16_t len);

/**************************************************************/

#endif

