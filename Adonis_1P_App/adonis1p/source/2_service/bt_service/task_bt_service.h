/**
  ******************************************************************************
  * @file    task_bt_service.h
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
  * 2024-07-09      V0.02      vincent.he@metextech.com     deploy char comm 
  * 2024-08-01      V0.03      vincent.he@metextech.com     add adonis ble prof
  *
  ******************************************************************************
  */
#ifndef __TASK_BT_SERVICE_H
#define __TASK_BT_SERVICE_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

#include <string.h>
#include "stdlib.h"
#include <inttypes.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_log.h"
#include "cybt_platform_trace.h"
#include "cycfg_gatt_db.h"
#include "cycfg_bt_settings.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_uuid.h"
#include "wiced_memory.h"
#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "cyhal_gpio.h"
#include "cyhal_wdt.h"
#include "wiced_bt_l2c.h"
#include "cyabs_rtos.h"
#include "sm_log.h"
#include "stdlib.h"
#include "ota.h"

#define BT_SERVICE_LOG_INFO SM_LOG_INFO //SM_LOG_OFF SM_LOG_INFO


typedef struct
{
    wiced_bt_device_address_t             remote_addr;   /* remote peer device address */
    uint16_t                              conn_id;       /* connection ID referenced by the stack */
    uint16_t                              mtu;           /* MTU exchanged after connection */
    double                                conn_interval; /* connection interval negotiated */
    wiced_bt_ble_host_phy_preferences_t   rx_phy;        /* RX PHY selected */
    wiced_bt_ble_host_phy_preferences_t   tx_phy;        /* TX PHY selected */

} conn_state_info_t;

/* Exported variables --------------------------------------------------------*/

TaskHandle_t* get_task_bt_handle(void);
void task_bt_service(void *pvParam);
void bt_init(void);

bool app_bt_service_char_findmy_device_is_found(void);
void app_bt_service_char_ntf_monitor(void);

bool procotol_get_ble_mac(uint8_t* rValue);
bool procotol_ble_opt(uint8_t opt);

void app_bt_adv_start(void);
void app_bt_adv_stop(void);
bool app_bt_enter_pairing_mode(void);

#endif

