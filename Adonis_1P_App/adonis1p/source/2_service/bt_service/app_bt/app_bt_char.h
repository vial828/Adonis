/**
  ******************************************************************************
  * @file    app_bt_char.h
  * @author  vincent.he@metextech.com 
  * @date    2024/08/12
  * @version V0.01
  * @brief   Brief description.
  *
  * Description: This file consists of the bt char functions that will help
  *              debugging and developing the applications easier with much
  *              more meaningful information.
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
  * 2024-08-12      V0.01      vincent.he@metextech.com     the first version
  * 
  ******************************************************************************
**/


#ifndef __APP_BT_CHAR_H__
#define __APP_BT_CHAR_H__

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"
#include <stdio.h>


/* Private define ------------------------------------------------------------*/
#define HEATING_PROFILE_BUF_SIZE (111) //108 + OPCODE + INDEX + STEPS 
#define HEATING_PROFILE_STEPS (27) //Number of Steps in Profile (unit8)

#define GATT_CONGESTED_WORKAROUND

#ifdef GATT_CONGESTED_WORKAROUND
#define CONGESTED_REBOOT_CNT_MAX    (2500) /*UINT: 2MS*/
#define GATT_CONGESTED_TIMEOUT      (1000) /*S*/
extern volatile uint16_t bCongestedRebootCnt;
extern volatile uint16_t gatt_ntf_congested_duration;
#endif

/**
  * @brief using app_bt_service_task_char_ntf_event_set() setting notification event,
  *        and report payload to APK.
  * 
  */
typedef enum 
{
    NONE_BT_SERVICE_NTF_EVT                            = (0x00),
    SESSION_SERVICE_HEATING_PROF_SEL_NTF_EVT           = (0x01),
    DEVICE_MANAGEMENT_SERVICE_UIMAGE_SIGN_VERIFY_EVT   = (0x02),
    DEBUG_SERVICE_DUMMY_EVENT_LOG_SET_EVT              = (0x04),
    SESSION_SERVICE_DUMMY_SESSION_RECORD_SET_EVT       = (0x08),
    SESSION_SERVICE_FACTORY_RESET_SET_EVT              = (0x10),
    RESERVED02                                         = (0x20),
    RESERVED03                                         = (0x40),
    RESERVED04                                         = (0x80),
} bt_service_notification_event_e;

/**
  * @brief using State Machine,
  *        and process "session records" or "event log" or "heating profile" payload to APK.
  * 
  */
typedef enum 
{
    HEATING_PROFILE_REPORTED_HEADER = 0,
    HEATING_PROFILE_REPORTED_DATA,
    SESSION_RECORDS_START = 0,
    SESSION_RECORDS_REPORTED_DATA,
    SESSION_RECORDS_FINISH,
    SESSION_RECORDS_LIVE_MODE,
    EVENT_LOG_START = 0,
    EVENT_LOG_REPORTED_DATA,
    EVENT_LOG_FINISH,
    EVENT_LOG_LIVE_MODE,
} bt_service_notification_payload_managemant_e;

/**
  * @brief using State Machine,
  *        and process "UI image" payload from APK.
  * 
  */
typedef enum 
{
    UI_IMAGE_PROTOBUF_HEADER = 0,
    UI_IMAGE_PROTOBUF_PAYLOAD_DECODE,
    UI_IMAGE_PAYLOAD_PROCESS,
    UI_IMAGE_CHALLENGE,
    UI_IMAGE_CHALLENGE_SIGNATURE,
    UI_IMAGE_PROTOBUF_PAYLOAD_HASH_SIGNATURE,
} bt_service_ui_image_payload_managemant_e;

/******************************************************************************
 *                                Constants
 ******************************************************************************/


/****************************************************************************
 *                              FUNCTION DECLARATIONS
 ***************************************************************************/

uint32_t app_bt_char_device_manage_payload_length_get(void);
void app_bt_char_device_manage_payload_length_clear(void);

uint8_t app_bt_service_task_char_ntf_event_get(void);
void app_bt_service_task_char_ntf_event_set(uint8_t ntf_event);
void app_bt_service_task_char_ntf_event_clear(uint8_t ntf_event);

void app_bt_char_bt_service_task_ntf_loop_exit_flag_set(uint8_t flag);
uint8_t app_bt_char_bt_service_task_ntf_loop_exit_flag_get(void);

wiced_bt_gatt_status_t app_bt_char_awaken_device(uint32_t Wait_Ms);

wiced_bt_gatt_status_t app_bt_char_device_name_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_device_name_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);

wiced_bt_gatt_status_t app_bt_char_session_service_device_information_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_device_information_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_time_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_time_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_battery_power_remaining_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_device_lock_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_device_lock_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);

wiced_bt_gatt_status_t app_bt_char_session_records_and_event_log_transmit_start(uint16_t handle);
wiced_bt_gatt_status_t app_bt_char_session_records_and_event_log_transmit_finish(uint16_t handle);
wiced_bt_gatt_status_t app_bt_char_session_service_session_records_payload_proc(void);
wiced_bt_gatt_status_t app_bt_char_session_service_session_records_read_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode);
wiced_bt_gatt_status_t app_bt_char_session_service_session_records_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);

wiced_bt_gatt_status_t app_bt_char_session_service_session_status_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_session_status_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_findmy_device_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_findmy_device_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_screen_led_control_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_screen_led_control_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_factory_reset_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_header(uint8_t *heating_profile_num);
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_proc(uint8_t heating_profile_index);
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_haptic_set_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_haptic_set_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_session_service_buzzer_speaker_set_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_session_service_buzzer_speaker_set_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);

wiced_bt_gatt_status_t app_bt_char_debug_service_last_error_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_debug_service_event_log_payload_proc(void);
wiced_bt_gatt_status_t app_bt_char_debug_service_event_log_read_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode);
wiced_bt_gatt_status_t app_bt_char_debug_service_event_log_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_debug_service_lifecycle_data_read_handler(void);
wiced_bt_gatt_status_t app_bt_char_debug_service_lifecycle_data_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);

wiced_bt_gatt_status_t app_bt_char_device_management_service_version_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_device_management_service_control_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_device_management_service_payload_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t app_bt_char_device_management_service_challenge_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len);

wiced_bt_gatt_status_t app_bt_char_device_management_service_uimage_data_integrity_validation(void);











#endif      /*__APP_BT_CHAR_H__ */


/* [] END OF FILE */
