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


#ifndef __APP_BT_CHAR_ADAPTER_H__
#define __APP_BT_CHAR_ADAPTER_H__

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"
#include <stdio.h>

#include "cybt_platform_trace.h"
#include "cycfg_gatt_db.h"
#include "cycfg_bt_settings.h"


/* Public macro -------------------------------------------------------------*/
#define MAX_NUM_HEATING_PROFILE	4

//Screen Upgrade 384KB
// #define UIMAGE_START_ADDR                (0x1E20000)
// #define USER_UIMAGE_INTRO_START_ADDR     (0x1E20000)
// #define USER_UIMAGE_GREETING_START_ADDR  (0x1E40000)
// #define USER_UIMAGE_OUTRO_START_ADDR     (0x1E60000)
// #define UIMAGE_END_ADDR                  (0x1E80000)

/* Private macro -------------------------------------------------------------*/
#define	DEV_NAME_LEN_MAX	16
#define	HEATING_PROFILE_LEN	108

/******************************************************************************
 *                                Constants
 ******************************************************************************/


/****************************************************************************
 *                              FUNCTION DECLARATIONS
 ***************************************************************************/
#define	FLASH_DATA_AREA_BLE	3

typedef struct Ble_Flash_Data_t
{
	uint32_t magic;
	uint16_t length;

	uint8_t heatProfile[2048];	// 需要存储10条Profile
	uint8_t bt_dev_name[32];
//	uint8_t led_brightness[32];	// 拆分以下2个字段
	uint8_t led_brightness[16];
	uint8_t last_error[16];

//	uint8_t haptic_strength[32];	// 拆分以下2个字段
	uint8_t haptic_strength[16];
	uint8_t	Eos_prompt[16];

//	uint8_t lock_mode[32];	// 拆分以下2个字段
	uint8_t lock_mode[16];
	uint8_t	Clean_prompt[16];
	
//	uint8_t buz_loudness[32];	// 拆分以下2个字段
	uint8_t buz_loudness[16];
	uint8_t ble_Congested[16];

	uint8_t heatProfileSel[32];
	uint8_t bt_find_me[128];
//	uint8_t life_cycle[128];
//	uint8_t time_stamp[32]; 	// 20240923增加
	uint8_t ble_bonding_data[4096];// 蓝牙邦定信息
	
	uint16_t crc16;
} Ble_Flash_Data_t;

typedef struct FindMe_t				// 数据体结构待完善，目前预留128字节，可用字节为124
{
	uint32_t	cal;
	uint8_t		dat[2];
} FindMe_t;

Ble_Flash_Data_t* get_ble_flash_data_handle(void);

bool app_param_read(uint8_t index, uint8_t *pData, uint16_t len);
bool app_param_write(uint8_t index, uint8_t *pData, uint16_t len);






uint32_t u32_endian_swap(uint32_t *pData);
uint16_t u16_endian_swap(uint16_t *pData);

int bt_adapter_md5_calculate(const unsigned char *pBuf, uint32_t memory_addr, size_t len, uint16_t repeat_cnt, unsigned char hash_value[]);
int bt_adapter_sha256_calculate(const unsigned char *pBuf, uint32_t memory_addr, size_t len, uint16_t repeat_cnt, unsigned char hash_value[]);
int bt_adapter_ecdsa_sign(uint8_t* hash, size_t hlen,
    				uint8_t* key_x, uint8_t* key_y,
    				uint8_t* sign_r, uint8_t* sign_s);
int bt_adapter_ecdsa_sign_verify(uint8_t* hash, size_t hlen,
                        uint8_t* key_x, uint8_t* key_y,
                        uint8_t* sign_r, uint8_t* sign_s);
int bt_adapter_ecdsa_message_sign_verify(uint8_t *message, size_t message_size, 
								                         uint8_t* public_key, uint8_t* signature);
int bt_adapter_ecdsa_sign_verify_test(void);

int bt_adapter_uECC_sha256_sign_verify(uint8_t *hash, size_t hlen, 
								uint8_t* public_key, uint8_t* signature);
int bt_adapter_uECC_message_sign_verify(uint8_t *message, size_t message_size, 
								uint8_t* public_key, uint8_t* signature);
                
wiced_bt_gatt_status_t bt_adapter_device_information_get(session_service_device_information_char_read_t *device_information);
wiced_bt_gatt_status_t bt_adapter_device_information_set_adv_name(const uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t bt_adapter_device_information_get_adv_name(uint8_t *pBuf, uint16_t *len);
wiced_bt_gatt_status_t bt_adapter_time_set(const uint8_t *pBuf, uint16_t len);
wiced_bt_gatt_status_t bt_adapter_time_get(session_service_time_char_write_read_t *timestamp);
uint8_t Get_Rtc_Trusted(void);

wiced_bt_gatt_status_t bt_adapter_battery_power_remaining_get(session_service_battery_power_remaining_char_read_ntf_t *battery_power_remaining);
bool bt_adapter_battery_power_remaining_change_happened(session_service_battery_power_remaining_char_read_ntf_t *battery_power_remaining);

bool bt_adapter_findmy_running_state_get(void);
void bt_adapter_findmy_running_state_restore(void);
wiced_bt_gatt_status_t bt_adapter_findmy_device_get(session_service_findmy_device_char_t *findmy_device);
wiced_bt_gatt_status_t bt_adapter_findmy_device_set(const uint8_t *pBuf, uint16_t len);
void bt_adapter_findmy_device_clr(void);			// 清除查找状态，但不改变存储的时间

void get_oled_brightness_from_eeprom(void);
wiced_bt_gatt_status_t bt_adapter_screen_led_brightness_control_get(session_service_screen_led_control_char_t *screen_led_brightness_control);
wiced_bt_gatt_status_t bt_adapter_screen_led_brightness_control_set(const uint8_t *pBuf, uint16_t len);

void get_motor_strength_from_eeprom(void);
wiced_bt_gatt_status_t bt_adapter_haptic_get(session_service_haptic_set_char_t *haptic_set_char);
wiced_bt_gatt_status_t bt_adapter_haptic_set(const uint8_t *pBuf, uint16_t len);

bool get_eos_prompt(void);
bool get_eos_prompt_from_eeprom(void);
wiced_bt_gatt_status_t bt_adapter_eos_prompt_get(uint8_t *dat);
wiced_bt_gatt_status_t bt_adapter_eos_prompt_set(const uint8_t *pBuf, uint16_t len);

bool get_clean_prompt(void);
bool get_clean_prompt_from_eeprom(void);
wiced_bt_gatt_status_t bt_adapter_clean_prompt_get(uint8_t *dat);
wiced_bt_gatt_status_t bt_adapter_clean_prompt_set(const uint8_t *pBuf, uint16_t len);

void get_beep_mode_from_eeprom(void);
wiced_bt_gatt_status_t bt_adapter_buzzer_speaker_loudness_get(session_service_buzzer_speaker_set_char_t *buzzer_speaker_set_char);
wiced_bt_gatt_status_t bt_adapter_buzzer_speaker_loudness_set(const uint8_t *pBuf, uint16_t len);

bool get_lock_mode(void);
bool get_lock_mode_from_eeprom(void);
bool get_update_lock_status(void);
void set_update_lock_status(bool status);
void lockStatus_update_to_eeprom(void);
wiced_bt_gatt_status_t bt_adapter_device_lock_get(session_service_device_lock_char_t *device_lock);
wiced_bt_gatt_status_t bt_adapter_device_lock_set(const uint8_t *pBuf, uint16_t len);
bool bt_adapter_device_lock_status_change_happened(session_service_device_lock_char_t *device_lock);

void get_time_stamp_from_flash(void);
void set_time_stamp_to_flash(void);

//void clr_time_stamp_trush(void);
void handle_time_stamp_trush_Long_Press_Reset(void);
void BleCongested_Write(bool sta);
bool BleCongested_Read(void);

wiced_bt_gatt_status_t bt_adapter_session_records_get(session_service_records_char_t *session_records_char);
wiced_bt_gatt_status_t bt_adapter_session_records_get_next(session_service_records_char_t *session_records_char_next);
uint32_t bt_adapter_session_records_number_get(void);
wiced_bt_gatt_status_t bt_adapter_session_records_clear_all(void);
wiced_bt_gatt_status_t bt_adapter_session_records_dummy_debug_set(session_service_records_char_t *session_records_char, uint32_t number);
wiced_bt_gatt_status_t bt_adapter_session_records_dummy_debug_set_proc(uint32_t* remain_number);

wiced_bt_gatt_status_t bt_adapter_session_status_get(session_service_status_char_t *session_status);
bool bt_adapter_session_status_change_happened(session_service_status_char_t *session_status);

wiced_bt_gatt_status_t bt_adapter_factory_reset_ble(const uint8_t *pBuf, uint16_t len);
void bt_adapter_factory_reset_all(void);

bool IsCustomHeatingProfile(void);		//选择曲线0时返回0，曲线1和2返回1；用于显示定制化
bool IsCustomHeatingProfileBoost(void);		//选择曲线0时返回0，曲线1和2返回1；用于显示定制化

wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_index_number_get(uint8_t *base_profile_index, uint8_t *boost_profile_index); 			// ��ȡ base �� boostģʽ��Index
uint8_t bt_adapter_heating_prof_sel_profiles_installed_index_number_get(uint8_t *pBuf);																	// Index������
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_index_payload_get(uint8_t *pBuf, uint8_t *len, uint8_t restart_index);							// ��ȡĳ��Index������
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_index_set(uint8_t mode, uint8_t profile_index);													// ����base/boost��Indexֵ
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_profiles_installed_index_array_get(uint8_t *pBuf, uint16_t *len);								// ��ȡ����Index������
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_profiles_installed_index_array_set(uint8_t *pBuf, uint16_t len);									// ��������Index������

wiced_bt_gatt_status_t bt_adapter_event_log_get(debug_service_event_log_char_t *event_log_char);
wiced_bt_gatt_status_t bt_adapter_event_log_get_next(debug_service_event_log_char_t *event_log_char_next);
uint32_t bt_adapter_event_log_number_get(void);
wiced_bt_gatt_status_t bt_adapter_event_log_clear_all(void);
wiced_bt_gatt_status_t bt_adapter_event_log_dummy_debug_set(debug_service_event_log_char_t *event_log_char, uint32_t number);
wiced_bt_gatt_status_t bt_adapter_event_log_dummy_debug_set_proc(uint32_t* remain_number);

uint32_t bt_adapter_lifecycle_data_individual_field_get(debug_service_lifecycle_data_opcode01_type_e opcode01_type);
wiced_bt_gatt_status_t bt_adapter_lifecycle_data_all_field_get(debug_service_lifecycle_data_opcode01_rsp_t *lifecycle_data);
wiced_bt_gatt_status_t bt_adapter_lifecycle_data_clear_all(void);

wiced_bt_gatt_status_t bt_adapter_random_data_get(uint8_t *pBuf, const uint8_t length);
int bt_adapter_mbedtls_ctr_drbg_random( void *p_rng, unsigned char *output,
                             size_t output_len );

wiced_bt_gatt_status_t bt_adapter_last_error_get(debug_service_last_error_char_t *last_error_char);
bool bt_adapter_last_error_change_happened(debug_service_last_error_char_t *last_error_char);


bool get_update_last_error(void);
void set_update_last_error(bool status);
void get_last_error_from_eeprom(void);
void last_error_update_to_eeprom(void);


wiced_bt_gatt_status_t bt_adapter_uimage_payload_upgrade(uint32_t uimage_writeAddr, uint8_t* pBuf, size_t len);

void bt_adapter_user_uimage_Intro_length_set(uint32_t length);
uint32_t bt_adapter_user_uimage_Intro_length_get(void);
void bt_adapter_user_uimage_Greeting_length_set(uint32_t length);
uint32_t bt_adapter_user_uimage_Greeting_length_get(void);
void bt_adapter_user_uimage_Outro_length_set(uint32_t length);
uint32_t bt_adapter_user_uimage_Outro_length_get(void);

bool MyName_uImage_Intro_isExisted(void);
bool MyName_uImage_Greeting_isExisted(void);
bool MyName_uImage_Outro_isExisted(void);

uint32_t MyName_uImage_Intro_Addr_Get(void);
uint32_t MyName_uImage_Greeting_Addr_Get(void);
uint32_t MyName_uImage_Outro_Addr_Get(void);

void MyName_uImage_isUpdating_Set(uint32_t uimage_addr);
uint8_t MyName_uImage_isUpdating_Get(void);

bool get_update_BleData_status(void);
void set_update_BleData_status(bool status);
void flash_data_save_change(bool bNoTimeStamp);

#endif      /*__APP_BT_CHAR_H__ */


/* [] END OF FILE */
