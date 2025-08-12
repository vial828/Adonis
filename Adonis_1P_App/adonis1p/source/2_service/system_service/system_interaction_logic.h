/**
  ******************************************************************************
  * @file    system_interaction_logic.h
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

#ifndef __SYSTEM_INTERACTION_LOGIC_H
#define __SYSTEM_INTERACTION_LOGIC_H

#include "stdint.h"
#include "stdbool.h"

/**
  * @brief 外部事件无buf
  */
typedef enum
{
    EXT_EVENT_NONE = 0,
    EXT_EVENT_KEY_STANDARD_SHORT_PRESS,
    EXT_EVENT_KEY_BOOST_SHORT_PRESS,
    EXT_EVENT_KEY_STANDARD_HEAT_PRESS,
    EXT_EVENT_KEY_BOOST_HEAT_PRESS,
    EXT_EVENT_KEY_CLEAN_HEAT_PRESS,
    EXT_EVENT_KEY_STANDARD_STOP_HEAT_PRESS,
    EXT_EVENT_KEY_BOOST_STOP_HEAT_PRESS,
    EXT_EVENT_KEY_CLEAN_STOP_HEAT_PRESS,
    EXT_EVENT_KEY_EXIT_SHIPPING_MODE_PRESS,
    EXT_EVENT_KEY_BT_PAIRED_PRESS,
    EXT_EVENT_PCTOOL_CMD_STANDARD_HEAT,
    EXT_EVENT_PCTOOL_CMD_BOOST_HEAT,
    EXT_EVENT_PCTOOL_CMD_CLEAN_HEAT,
    EXT_EVENT_PCTOOL_CMD_TEST_HEAT,
    EXT_EVENT_PCTOOL_CMD_REF_BASE_HEAT,
    EXT_EVENT_PCTOOL_CMD_REF_BOOST_HEAT,
//    EXT_EVENT_PCTOOL_CMD_BOOST_HEAT,
//    EXT_EVENT_PCTOOL_CMD_STANDAE_HEAT,
    EXT_EVENT_PCTOOL_CMD_STOP_HEAT,
    EXT_EVENT_USB_PLUG,
    EXT_EVENT_USB_UNPLUG,
    EXT_EVENT_BOOT,
    EXT_EVENT_BOOT_TIP,

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
	EXT_EVENT_USB_PLUG_HAL,					// bug 1968519
	EXT_EVENT_USB_UNPLUG_HAL,

	EXT_EVENT_LOCK,
	EXT_EVENT_UNLOCK,
	EXT_EVENT_FIND_MY_GLO,
	EXT_EVENT_FIND_MY_GLO_CANCLE,
	EXT_EVENT_PARING_QRCODE,
	EXT_EVENT_PARING,
	EXT_EVENT_PARING_CANCLE,
	EXT_EVENT_PARI_OK,
	EXT_EVENT_PARI_FAIL,
	EXT_EVENT_LOADING,
	EXT_EVENT_LOADING_STOP,
	EXT_EVENT_LOADING_OK,
	EXT_EVENT_UIMAGE_CUSTOMIZE,
	EXT_EVENT_HEATING_PROFILES,
	EXT_EVENT_HEATING_PROFILES_BOOST,
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

}ExternalEven_e;
void set_charge_full_status(bool status);
void set_system_external_even(ExternalEven_e even);
void system_error_recover_proc(void);
void system_interaction_logic_proc(void);

bool ble_interaction_delay_handle_for_eol(void);		// 因为显示EOL而延迟处理的事件，无，则返回false

#endif
 
 
