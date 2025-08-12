/* SPDX-License-Identifier: GPL-2.0 */
/*******************************************************************************
 **** Copyright (C), 2020-2021, Awinic.All rights reserved. ************
 *******************************************************************************
 * Author		: awinic
 * Date		  : 2021-09-09
 * Description   : .C file function description
 * Version	   : 1.0
 * Function List :
 ******************************************************************************/
#ifndef AW35615_DRIVER_H
#define AW35615_DRIVER_H
#include "modules/dpm.h"

/* AW35615 Device ID */

/* AW35615 I2C Configuration */
#define AW35615SlaveAddr   0x22

#define AW35615AddrLength  1		/* One byte address */
#define AW35615IncSize  1		/* One byte increment */

/* AW35615 Register Addresses */
#define regDeviceID		0x01
#define regSwitches0	0x02
#define regSwitches1	0x03
#define regMeasure		0x04
#define regSlice		0x05
#define regControl0		0x06
#define regControl1		0x07
#define regControl2		0x08
#define regControl3		0x09
#define regMask			0x0A
#define regPower		0x0B
#define regReset		0x0C
#define regOCPreg		0x0D
#define regMaska		0x0E
#define regMaskb		0x0F
#define regControl4		0x10
#define regControl5		0x11
#define regControl7		0x13
#define regControl8		0x14
#define regVdl			0x3A
#define regVdh			0x3B
#define regStatus0a		0x3C
#define regStatus1a		0x3D
#define regInterrupta	0x3E
#define regInterruptb	0x3F
#define regStatus0		0x40
#define regStatus1		0x41
#define regInterrupt	0x42
#define regFIFO			0x43

/* Coded SOP values that arrive in the RX FIFO */
#define SOP_CODE_SOP		0xE0
#define SOP_CODE_SOP1		0xC0
#define SOP_CODE_SOP2		0xA0
#define SOP_CODE_SOP1_DEBUG	0x80
#define SOP_CODE_SOP2_DEBUG	0x60

/* Device TX FIFO Token Definitions */
#define TXON			0xA1
#define SYNC1_TOKEN		0x12
#define SYNC2_TOKEN		0x13
#define SYNC3_TOKEN		0x1B
#define RESET1			0x15
#define RESET2			0x16
#define PACKSYM			0x80
#define JAM_CRC			0xFF
#define EOP			 	0x14
#define TXOFF			0xFE

/*
 * Note: MDAC values are actually (MDAC + 1) * 42/420mV
 * Data sheet is incorrect.
 */
#define SDAC_DEFAULT		0x1F
#define MDAC_0P210V		0x04
#define MDAC_0P420V		0x09
#define MDAC_0P798V		0x12
#define MDAC_0P882V		0x14
#define MDAC_1P596V		0x25
#define MDAC_2P058V		0x30
#define MDAC_2P604V		0x3D
#define MDAC_2P646V		0x3E

#define VBUS_MDAC_0P84V		0x01
#define VBUS_MDAC_3P36		0x07
#define VBUS_MDAC_3P78		0x08
#define VBUS_MDAC_4P20		0x09
#define VBUS_MDAC_4P62		0x0A
#define VBUS_MDAC_5P04		0x0B
#define VBUS_MDAC_5P46		0x0C
#define VBUS_MDAC_7P14		0x10	/* (9V detach) */
#define VBUS_MDAC_8P40		0x13
#define VBUS_MDAC_9P66		0x16	/* (12V detach) */
#define VBUS_MDAC_11P34		0x1A
#define VBUS_MDAC_11P76		0x1B
#define VBUS_MDAC_12P18		0x1C	/* (15V detach) */
#define VBUS_MDAC_12P60		0x1D
#define VBUS_MDAC_14P28		0x21
#define VBUS_MDAC_15P96		0x25	/* (20V detach) */
#define VBUS_MDAC_18P90		0x2C
#define VBUS_MDAC_21P00		0x31

#define MDAC_MV_LSB		 420	 /* MDAC Resolution in mV */

#define VBUS_MV_VSAFE0V		840	 /* Closest value for MDAC resolution */
#define VBUS_MV_VSAFE0V_DISCH	600
#define VBUS_MV_VSAFE5V_DISC	3200
#define VBUS_MV_VSAFE5V_L	4150
#define VBUS_MV_VSAFE5V_H	5500

#define VBUS_PD_TO_MV(v)   (v * 50)	 /* Convert 50mv PD values to mv */
#define VBUS_PPS_TO_MV(v)  (v * 20)	 /* Convert 20mv PD values to mv */
#define VBUS_MV_NEW_MAX(v) (v + (v/20)) /* Value in mv + 5% */
#define VBUS_MV_NEW_MIN(v) (v - (v/20)) /* Value in mv - 5% */
#define VBUS_MV_TO_MDAC(v) ((v/420)-1)  /* MDAC (VBUS) value is 420mv res - 1 */

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 REVISION_ID:2;
		AW_U8 PRODUCT_ID:2;
		AW_U8 VERSION_ID:4;
	};
} regDeviceID_t;

typedef union {
	AW_U16 word;
	AW_U8 byte[2];
	struct {
		/* Switches0 */
		AW_U8 PDWN1:1;
		AW_U8 PDWN2:1;
		AW_U8 MEAS_CC1:1;
		AW_U8 MEAS_CC2:1;
		AW_U8 VCONN_CC1:1;
		AW_U8 VCONN_CC2:1;
		AW_U8 PU_EN1:1;
		AW_U8 PU_EN2:1;
		/* Switches1 */
		AW_U8 TXCC1:1;
		AW_U8 TXCC2:1;
		AW_U8 AUTO_CRC:1;
		AW_U8:1;
		AW_U8 DATAROLE:1;
		AW_U8 SPECREV:2;
		AW_U8 POWERROLE:1;
	};
} regSwitches_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 MDAC:6;
		AW_U8 MEAS_VBUS:1;
		AW_U8:1;
	};
} regMeasure_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 SDAC:6;
		AW_U8 SDAC_HYS:2;
	};
} regSlice_t;

typedef union {
	AW_U32 dword;
	AW_U8 byte[4];
	struct {
		/* Control0 */
		AW_U8 TX_START:1;
		AW_U8 AUTO_PRE:1;
		AW_U8 HOST_CUR:2;
		AW_U8:1;
		AW_U8 INT_MASK:1;
		AW_U8 TX_FLUSH:1;
		AW_U8:1;
		/* Control1 */
		AW_U8 ENSOP1:1;
		AW_U8 ENSOP2:1;
		AW_U8 RX_FLUSH:1;
		AW_U8 FAST_I2C:1;
		AW_U8 BIST_MODE2:1;
		AW_U8 ENSOP1DP:1;
		AW_U8 ENSOP2DB:1;
		AW_U8:1;
		/* Control2 */
		AW_U8 TOGGLE:1;
		AW_U8 MODE:2;
		AW_U8 WAKE_EN:1;
		AW_U8 WAKE_SELF:1;
		AW_U8 TOG_RD_ONLY:1;
		AW_U8 TOG_SAVE_PWR:2;
		/* Control3 */
		AW_U8 AUTO_RETRY:1;
		AW_U8 N_RETRIES:2;
		AW_U8 AUTO_SOFTRESET:1;
		AW_U8 AUTO_HARDRESET:1;
		AW_U8 BIST_TMODE:1;
		AW_U8 SEND_HARDRESET:1;
		AW_U8:1;
	};
} regControl_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 M_BC_LVL:1;
		AW_U8 M_COLLISION:1;
		AW_U8 M_WAKE:1;
		AW_U8 M_ALERT:1;
		AW_U8 M_CRC_CHK:1;
		AW_U8 M_COMP_CHNG:1;
		AW_U8 M_ACTIVITY:1;
		AW_U8 M_VBUSOK:1;
	};
} regMask_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 POWER:4;
		AW_U8 POWER_4:1;
		AW_U8:3;
	};
} regPower_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 SW_RES:1;
		AW_U8 PD_RESET:1;
		AW_U8:6;
	};
} regReset_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 OCP_CUR:3;
		AW_U8 OCP_RANGE:1;
		AW_U8:4;
	};
} regOCPreg_t;

typedef union {
	AW_U16 word;
	AW_U8 byte[2];
	struct {
		/* Maska */
		AW_U8 M_HARDRST:1;
		AW_U8 M_SOFTRST:1;
		AW_U8 M_TXSENT:1;
		AW_U8 M_HARDSENT:1;
		AW_U8 M_RETRYFAIL:1;
		AW_U8 M_SOFTFAIL:1;
		AW_U8 M_TOGDONE:1;
		AW_U8 M_OCP_TEMP:1;
		/* Maskb */
		AW_U8 M_GCRCSENT:1;
		AW_U8 M_CC_OV:1;
		AW_U8 M_VCONN_OK:1;
		AW_U8:5;
	};
} regMaskAdv_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 TOG_EXIT_AUD:1;
		AW_U8 EN_PAR_CFG:1;
		AW_U8 DUMMY1:1;
		AW_U8 DUMMY2:1;
		AW_U8:4;
	};
} regControl4_t;

typedef union {
	AW_U8 byte;
	struct {
		AW_U8 EN_PD3_MSG:1;
		AW_U8:2;
		AW_U8 VBUS_DIS_SEL:2;
		AW_U8:3;
	};
} regControl5_t;

typedef union {
	AW_U8 byte[7];
	struct {
		AW_U16  StatusAdv;
		AW_U16  InterruptAdv;
		AW_U16  Status;
		AW_U8   Interrupt1;
	};
	struct {
		/* Status0a */
		AW_U8 HARDRST:1;
		AW_U8 SOFTRST:1;
		AW_U8 POWER23:2;
		AW_U8 RETRYFAIL:1;
		AW_U8 SOFTFAIL:1;
		AW_U8 CC_OV:1;
		AW_U8 VCONN_OK:1;
		/* Status1a */
		AW_U8 RXSOP:1;
		AW_U8 RXSOP1DB:1;
		AW_U8 RXSOP2DB:1;
		AW_U8 TOGSS:3;
		AW_U8:2;
		/* Interrupta */
		AW_U8 I_HARDRST:1;
		AW_U8 I_SOFTRST:1;
		AW_U8 I_TXSENT:1;
		AW_U8 I_HARDSENT:1;
		AW_U8 I_RETRYFAIL:1;
		AW_U8 I_SOFTFAIL:1;
		AW_U8 I_TOGDONE:1;
		AW_U8 I_OCP_TEMP:1;
		/* Interruptb */
		AW_U8 I_GCRCSENT:1;
		AW_U8 I_CC_OV:1;
		AW_U8 I_VCONN_OK:1;
		AW_U8:5;
		/* Status0 */
		AW_U8 BC_LVL:2;
		AW_U8 WAKE:1;
		AW_U8 ALERT:1;
		AW_U8 CRC_CHK:1;
		AW_U8 COMP:1;
		AW_U8 ACTIVITY:1;
		AW_U8 VBUSOK:1;
		/* Status1 */
		AW_U8 OCP:1;
		AW_U8 OVRTEMP:1;
		AW_U8 TX_FULL:1;
		AW_U8 TX_EMPTY:1;
		AW_U8 RX_FULL:1;
		AW_U8 RX_EMPTY:1;
		AW_U8 RXSOP1:1;
		AW_U8 RXSOP2:1;
		/* Interrupt */
		AW_U8 I_BC_LVL:1;
		AW_U8 I_COLLISION:1;
		AW_U8 I_WAKE:1;
		AW_U8 I_ALERT:1;
		AW_U8 I_CRC_CHK:1;
		AW_U8 I_COMP_CHNG:1;
		AW_U8 I_ACTIVITY:1;
		AW_U8 I_VBUSOK:1;
	};
} regStatus_t;

typedef struct {
	regDeviceID_t	DeviceID;
	regSwitches_t	Switches;
	regMeasure_t	Measure;
	regSlice_t	Slice;
	regControl_t	Control;
	regMask_t	Mask;
	regPower_t	Power;
	regReset_t	Reset;
	regOCPreg_t	 OCPreg;
	regMaskAdv_t	MaskAdv;
	regControl4_t	Control4;
	regControl5_t	Control5;
	regStatus_t	 Status;
} DeviceReg_t;

#define TICK_SCALE_TO_MS			(1)

/**
 * VBus switch levels
 */
typedef enum
{
    VBUS_LVL_5V,
    VBUS_LVL_HV,
    VBUS_LVL_ALL
} VBUS_LVL;

typedef enum{
	SET_VOUT_0000MV = 0,      // 0V
	SET_VOUT_5000MV = 5000,   // 5V
	SET_VOUT_9000MV = 9000,   // 9V
	SET_VOUT_12000MV = 12000, // 12V
	SET_VOUT_15000MV = 15000, // 15V
	SET_VOUT_20000MV = 20000, // 20V
}SET_VOUT_MV;

/**
 * Events that platform uses to notify modules listening to the events.
 * The subscriber to event signal can subscribe to individual events or
 * a event in group.
 */
typedef enum
{
	CC1_ORIENT				= 0x1,
	CC2_ORIENT				= 0x2,
	CC_AUDIO_ORIENT		= 0x3,
	CC_NO_ORIENT			= 0x4,
	CC_AUDIO_OPEN			= 0x5,
	CC_ORIENT_ALL			=	CC1_ORIENT | CC2_ORIENT | CC_NO_ORIENT,
	PD_NEW_CONTRACT		= 0x8,
	PD_NO_CONTRACT		= 0x9,
	PD_CONTRACT_ALL		= PD_NEW_CONTRACT | PD_NO_CONTRACT,
	PD_STATE_CHANGED	= 0xa,
	ACC_UNSUPPORTED		= 0xb,
	BIST_DISABLED			= 0xc,
	BIST_ENABLED			= 0xd,
	BIST_ALL					= BIST_ENABLED | BIST_DISABLED,
	ALERT_EVENT				= 0x10,
	POWER_ROLE				= 0x11,
	DATA_ROLE					= 0x12,
	AUDIO_ACC					= 0x13,
	CUSTOM_SRC				= 0x14,
	VBUS_OK						= 0x15,
	VBUS_DIS					= 0x16,
	WATERPROOFING			= 0x17,
	CC1_AND_CC2				= 0x18,
	EVENT_ALL					= 0xFF,
} Events_t;

/**
 * @brief Perform a blocking delay.
 *
 * @param delayCount - Number of 10us delays to wait
 * @return None
 */
void platform_delay_10us(AW_U32 delayCount);

AW_U32 get_system_time_ms(void);

/**
 * @brief Set or return programmable supply (PPS) voltage and current limit.
 *
 * @param port ID for multiple port controls
 * @param mv Voltage in millivolts
 * @return None or Value in mv/ma.
 */
void platform_set_pps_voltage(AW_U8 port, AW_U32 mv);

/**
 * @brief The function gets the current VBUS level supplied by PPS supply
 *
 * If VBUS is not enabled by the PPS supply the return type is undefined.
 *
 * @param port ID for multiple port controls
 * @return VBUS level supplied by PPS in milivolt resolution
 */
AW_U16 platform_get_pps_voltage(AW_U8 port);

/**
 * @brief Set the maximum current that can be supplied by PPS source
 * @param port ID for multiple port controls
 * @param ma Current in milliamps
 * @return None
 */
void platform_set_pps_current(AW_U8 port, AW_U32 ma);

/**
 * @brief Get the maximum current that the PPS supply is configured to provide
 *
 * If the PPS supply is not currently supplying current the return value is
 * undefined.
 *
 * @param port ID for multiple port controls
 * @return Current in milliamps
 */
AW_U16 platform_get_pps_current(AW_U8 port);

/**
 * @brief Enable/Disable VConn path
 *
 * Optional for platforms with separate VConn switch
 *
 * @param port ID for multiple port controls
 * @param enable AW_TRUE = VConn path ON.
 * @return None
 */
void platform_set_vconn(AW_U8 port, AW_BOOL enable);

/**
 * @brief The current state of the device interrupt pin
 *
 * @param port ID for multiple port controls
 * @return AW_TRUE if interrupt condition present.  Note: pin is active low.
 */
AW_BOOL platform_get_device_irq_state(AW_U8 port);

void aw35615_init_event_handler(void);

#ifdef AW_HAVE_DP
/******************************************************************************
 * Function:        platform_dp_enable_pins
 * Input:           enable - If false put dp pins to safe state and config is
 *                           don't care. When true configure the pins with valid
 *                           config.
 *                  config - 32-bit port partner config. Same as type in
 *                  DisplayPortConfig_t in display_port_types.h.
 * Return:          AW_TRUE - pin config succeeded, AW_FALSE - pin config failed
 * Description:     enable/disable display port pins. If enable is true, check
 *                  the configuration bits[1:0] and the pin assignment
 *                  bits[15:8] to decide the appropriate configuration.
 ******************************************************************************/
AW_BOOL platform_dp_enable_pins(AW_BOOL enable, AW_U32 config);

/******************************************************************************
 * Function:        platform_dp_status_update
 * Input:           status - 32-bit status value. Same as DisplayPortStatus_t
 *                  in display_port_types.h
 * Return:          None
 * Description:     Called when new status is available from port partner
 ******************************************************************************/
void platform_dp_status_update(AW_U32 status);
#endif

Port_t* aw35615_GetChip(void);
DevicePolicyPtr_t aw35615_GetDpm(void);

AW_BOOL DeviceWrite(Port_t *port, AW_U8 regAddr, AW_U8 length, AW_U8* data);
AW_BOOL DeviceRead(Port_t *port, AW_U8 regAddr, AW_U8 length, AW_U8* data);

int aw35615_init(void);
int aw35615_deinit(void);

void aw35615_request_val(AW_U8 pos);
void aw35615_proc(void);

#endif /* AW35615_DRIVER_H */
