/**
  ******************************************************************************
  * @file    app_bt_char.c
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

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include "app_bt_char.h"

/* Header file includes */
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
#include "public_typedef.h"

#include "app_bt_utils.h"
#include "app_bt_char_adapter.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

#include "ota_context.h"
#if (OTA_SERVICE_SUPPORT == 1)
/* OTA related header files */
#include "cy_ota_api.h"
#endif//#if (OTA_SERVICE_SUPPORT == 1)

#ifdef OTA_USE_EXTERNAL_FLASH
#include "ota_serial_flash.h"
#endif

//platform
#include "platform_io.h"
//middleware
#include "data_base_info.h"
//service
#include "system_status.h"
#include "task_system_service.h"
#include "system_interaction_logic.h"
//mbedtls
#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"

//nanopb decode
#include "uimage_decode.h"
#include "uimage_encode.h"
#include "uimage_data_record.h"

/* Private define ------------------------------------------------------------*/
#define UIMAGE_WRITE_SECTOR_SIZE    (1024)//4096
#define UIMAGE_READ_SECTOR_SIZE     (4096)//4096
#define DEVICE_MANAGE_PAYLOAD_SIZE  (1024)//4096

#define UIMAGE_CHALLENGE_SIZE             (32)
#define UIMAGE_CHALLENGE_SIGN_SIZE        (64)
#define UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE (64)
#define UIMAGE_VERIFY_FIX_SIZE            (UIMAGE_CHALLENGE_SIZE + UIMAGE_CHALLENGE_SIGN_SIZE + UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE)

#define MD5_SUPPORT         (0)
#define SHA256_SUPPORT      (1)
#define RLE_SUPPORT         (0)

/* Private macro -------------------------------------------------------------*/

/* Private Consts ------------------------------------------------------------*/
/*
* Public key extracted from PEM file
* Use this key with the micro-ecc library
*
* Note: microECC does not work directly with PEM or DER formats.
* It expects the public key to be in raw byte format, specifically
* as an uncompressed or compressed point on the elliptic curve.
*/
#if 0                                    /* R&D Key*/
const uint8_t myName_public_key[64] = {
   0x28, 0x9e, 0xc7, 0x34, 0x95, 0xcd, 0x48, 0x8f,
   0x20, 0xa4, 0x22, 0x5e, 0x4e, 0x40, 0x2d, 0x98,
   0x1a, 0xe2, 0x3b, 0xa8, 0x4f, 0x93, 0x96, 0x94,
   0xca, 0x3f, 0x76, 0x04, 0x5a, 0x80, 0xa4, 0x7c,
   0xc7, 0x2b, 0xf4, 0x36, 0x2a, 0x73, 0x4b, 0x4f,
   0xcb, 0x20, 0xf1, 0x71, 0x12, 0x51, 0x72, 0xc4,
   0x25, 0x73, 0x7d, 0x21, 0x43, 0xad, 0xd2, 0x3b,
   0x90, 0xb2, 0x18, 0xd3, 0xee, 0xed, 0x0d, 0x24,
};
#else                                    /* Production Key*/
const uint8_t myName_public_key[64] = {
   0xd9, 0x73, 0x85, 0xe9, 0xc2, 0x4d, 0xeb, 0x18,
   0x46, 0x30, 0x9e, 0x52, 0xdf, 0x9f, 0xd1, 0x71,
   0x1f, 0x4b, 0x0b, 0x6b, 0x98, 0x06, 0xb3, 0xbf,
   0x01, 0x91, 0x97, 0x78, 0xc0, 0x9d, 0x32, 0xb2,
   0x74, 0x38, 0xca, 0xe4, 0x26, 0xb9, 0x1d, 0x40,
   0x43, 0x95, 0x6c, 0x00, 0xde, 0x1c, 0x8e, 0x58,
   0xab, 0x7c, 0x41, 0x7a, 0xd8, 0x4f, 0x2e, 0x79,
   0x77, 0x13, 0xf7, 0xe7, 0xce, 0x7a, 0xd0, 0xdd,
};
#endif

/* Private typedef -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static device_management_service_version_char_t device_management_service_version;
//static mbedtls_md5_context md5_ctx;
static mbedtls_sha256_context sha256_ctx;

static uint8_t device_manage_payload_data_remain_count=0;
static uint16_t device_manage_paload_data_remain_offset = 0;
static uint32_t device_manage_payload_data_len=0;
static uint32_t device_manage_payload_package_cnt = 0;
static uint8_t device_manage_payload[DEVICE_MANAGE_PAYLOAD_SIZE] = {0};

static uint8_t bt_service_task_ntf_loop_exit_flag=0;

static volatile uint8_t s_UImgeMachineState=0;
static uint16_t UImageProtobuf_Heater = 0;

static uint32_t uImageSavedFlashADDR=0;
static uint8_t uimage_challenge[UIMAGE_CHALLENGE_SIZE];
static uint8_t uimage_challenge_signature[UIMAGE_CHALLENGE_SIGN_SIZE];
static uint8_t uimage_protobuf_payload_signature[UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE];
static uint32_t writeAddr_offset = 0;  

static volatile uint8_t gs_bt_service_char_ntf_event_hl; 

#ifdef GATT_CONGESTED_WORKAROUND
volatile uint16_t gatt_ntf_congested_duration;
volatile uint16_t bCongestedRebootCnt;
extern cy_semaphore_t	gatt_congested;
extern volatile uint16_t bGattCongestedTimeout;
#endif

/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
/*
 * Function Name: app_bt_char_device_manage_payload_length_get
 * 
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
uint32_t app_bt_char_device_manage_payload_length_get(void)
{
    return device_manage_payload_data_len;
}

void app_bt_char_device_manage_payload_length_clear(void)
{
    writeAddr_offset = 0;
    device_manage_payload_data_len = 0;
}

/*
 * Function Name: app_bt_service_task_char_ntf_event_get
 * 
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
uint8_t app_bt_service_task_char_ntf_event_get(void)
{
    return gs_bt_service_char_ntf_event_hl;
}

/*
 * Function Name: app_bt_service_task_char_ntf_event_set
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
void app_bt_service_task_char_ntf_event_set(uint8_t ntf_event)
{
    if (ntf_event == NONE_BT_SERVICE_NTF_EVT)
    {
        gs_bt_service_char_ntf_event_hl = 0;
        return;
    }

    gs_bt_service_char_ntf_event_hl |= ntf_event;
    
    // 通知bt任务
    TaskHandle_t *temp_handle;
    temp_handle = get_task_bt_handle();
    xTaskNotifyGive(*temp_handle);
}

/*
 * Function Name: app_bt_service_task_char_ntf_event_clear
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
void app_bt_service_task_char_ntf_event_clear(uint8_t ntf_event)
{
    gs_bt_service_char_ntf_event_hl &= (~ntf_event);
}

/*
 * Function Name: app_bt_char_bt_service_task_ntf_loop_exit_flag set/get
 * 
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
void app_bt_char_bt_service_task_ntf_loop_exit_flag_set(uint8_t flag)
{
    bt_service_task_ntf_loop_exit_flag = flag;
}
uint8_t app_bt_char_bt_service_task_ntf_loop_exit_flag_get(void)
{
    return bt_service_task_ntf_loop_exit_flag;
}

/****************************************************************************************
* @brief   app_bt_char_awaken_device
* @param
* @return  0: SUCCESS x:Failure
* @note
****************************************************************************************
**/
wiced_bt_gatt_status_t app_bt_char_awaken_device(uint32_t Wait_Ms)
{
	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

	// 如果系统在睡眠状态则先唤醒
    if (get_system_status() == SLEEP)
    {
        TaskHandle_t *temp_handle;
        temp_handle = get_task_system_handle();
        xTaskNotifyGive(*temp_handle);
        vTaskDelay(Wait_Ms); //300 等待系统初始化完成, 唤醒后初始化电量计，有延迟，电量计休眠前shutdow模式
    }

	return status; 
}

/*
 * Function Name: app_bt_char_device_name_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_name_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    static uint8_t gatt_get_device_name[DEV_NAME_LEN_MAX] = {0};
    static uint16_t gatt_get_length =0;
    status =  bt_adapter_device_information_get_adv_name(gatt_get_device_name, &gatt_get_length);

    app_gatt_db_ext_attr_tbl[0].cur_len = gatt_get_length;//1800 service
    app_gatt_db_ext_attr_tbl[0].p_data = gatt_get_device_name;

    sm_log(SM_LOG_DEBUG, "%s ,status: %d\n", __FUNCTION__, status);
    
    return status; 
}

/*
 * Function Name:
 * app_bt_char_device_name_write_handler
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_name_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if ((1 <= len)&&(len <= DEV_NAME_LEN_MAX))//msg len verify.
    {
        cy_bt_scan_rsp_packet_data[0].len = len;
        if (cy_bt_scan_rsp_packet_data[0].len > DEV_NAME_LEN_MAX)//retrict max len
            cy_bt_scan_rsp_packet_data[0].len = DEV_NAME_LEN_MAX;
        cy_bt_scan_rsp_packet_data[0].p_data = pBuf;

        memset(app_gap_device_name, 0, sizeof(app_gap_device_name));
        memcpy(app_gap_device_name, pBuf, len);

        bt_adapter_device_information_set_adv_name(cy_bt_scan_rsp_packet_data[0].p_data, cy_bt_scan_rsp_packet_data[0].len);

        /* Set Advertisement Data */
        wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                            cy_bt_adv_packet_data);
        wiced_bt_ble_set_raw_scan_response_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                            cy_bt_scan_rsp_packet_data);
        // /**
        //  * iOS need device indicate device_name has changed, and discovery again database*/
        // status = wiced_bt_gatt_server_send_indication(app_server_context.bt_conn_id,
        //                                         HDLC_GATT_SERVICE_CHANGED_VALUE,
        //                                         app_gatt_service_changed_len,
        //                                         app_gatt_service_changed,
        //                                         NULL);
        // sm_log(SM_LOG_DEBUG, "Device Name update, Indication Status: %d \n", status);
    }
    
    return status; 
}


/*
 * Function Name: app_bt_char_session_service_device_information_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_device_information_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    status = bt_adapter_device_information_get(app_session_service_ext_device_information_char);

    sm_log(SM_LOG_DEBUG, "%s ,status: %d\n", __FUNCTION__, status);
    
    return status; 
}

/*
 * Function Name:
 * app_bt_char_session_service_device_information_write_handler
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_device_information_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{           
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif
    
#define DEVICE_INFO_CODE_0001 (0x0001)
#define DEVICE_INFO_CODE_0002 (0x0002)
#define DEVICE_INFO_CODE_0003 (0x0003)

    if (len == 2)//device info code 0002 cmd
    {
        if (DEVICE_INFO_CODE_0001 == APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]))
        {
            sm_log(SM_LOG_DEBUG, "device info code 0001 pair: %04x", APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]));
            if (app_session_service_device_information_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)
            {
                                                                        /*payload code: 0x0000*/
                uint8_t device_info_code0001_response[4] = {0x00, 0x00, 0x00, 0x00};

                device_info_code0001_response[1] = 0x01; //only update the byte.

                // data_array_invert_endianness(device_info_code0001_response, sizeof(device_info_code0001_response));//conversion to big endian.

                wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                handle,
                                sizeof(device_info_code0001_response),
                                (uint8_t *)device_info_code0001_response,NULL);

                sm_log(SM_LOG_DEBUG, "Sending Notification.\r\n");
            }
            else
            {
                sm_log(SM_LOG_DEBUG, "Notification Not Enable.\r\n");
            }
        }
        else if (DEVICE_INFO_CODE_0003 == APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]))
        {
            sm_log(SM_LOG_DEBUG, "device info code 0003 pair: %04x", APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]));
            
            uint8_t device_info_code0003_response[256] = {0}; 
            uint16_t device_info_code0003_response_len = 0;

            sm_log(SM_LOG_DEBUG, "Intro_isExisted: %d\r\n", MyName_uImage_Intro_isExisted());
            sm_log(SM_LOG_DEBUG, "Greeting_isExisted: %d\r\n", MyName_uImage_Greeting_isExisted());
            sm_log(SM_LOG_DEBUG, "Outro_isExisted: %d\r\n", MyName_uImage_Outro_isExisted());

            uimage_screen_update_nanopb_encode(device_info_code0003_response, &device_info_code0003_response_len);

            uimage_screen_update_nanopb_decode(device_info_code0003_response, device_info_code0003_response_len, NULL);

            wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            handle,
                            device_info_code0003_response_len,
                            (uint8_t *)device_info_code0003_response,NULL);

            sm_log(SM_LOG_DEBUG, "Sending Notification.\r\n");
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "device info code 0001/0003 error:%04x", APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]));
        }
    }

    if ((2 < len)&&(len <= (DEV_NAME_LEN_MAX+2)))//msg len verify, Bug 1904695:
    {
        if (DEVICE_INFO_CODE_0002 == APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]))
        {
            sm_log(SM_LOG_DEBUG, "device info code 0002 pair: %04x", APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]));

            #if 0
            sm_log(SM_LOG_DEBUG, "0: %02x\n", cy_bt_adv_packet_data[0].advert_type);
            sm_log(SM_LOG_DEBUG, "0: %02x\n", cy_bt_adv_packet_data[0].len);
            for (uint8_t i=0; i<cy_bt_adv_packet_data[0].len; i++)
                sm_log(SM_LOG_DEBUG, " %02x", cy_bt_adv_packet_data[0].p_data[i]);
            sm_log(SM_LOG_DEBUG, "\n");
            sm_log(SM_LOG_DEBUG, "1: %02x\n", cy_bt_adv_packet_data[1].advert_type);
            sm_log(SM_LOG_DEBUG, "1: %02x\n", cy_bt_adv_packet_data[1].len);
            for (uint8_t i=0; i<cy_bt_adv_packet_data[1].len; i++)
                sm_log(SM_LOG_DEBUG, " %02x", cy_bt_adv_packet_data[1].p_data[i]);
            sm_log(SM_LOG_DEBUG, "\n");
            #endif 

            cy_bt_scan_rsp_packet_data[0].len = len-2;
            if (cy_bt_scan_rsp_packet_data[0].len > DEV_NAME_LEN_MAX)//retrict max len
                cy_bt_scan_rsp_packet_data[0].len = DEV_NAME_LEN_MAX;
            cy_bt_scan_rsp_packet_data[0].p_data = pBuf+2;

            memset(app_gap_device_name, 0, sizeof(app_gap_device_name));
            memcpy(app_gap_device_name, &pBuf[2], cy_bt_scan_rsp_packet_data[0].len);

            bt_adapter_device_information_set_adv_name(cy_bt_scan_rsp_packet_data[0].p_data, cy_bt_scan_rsp_packet_data[0].len);

            /* Set Advertisement Data */
            wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                                cy_bt_adv_packet_data);
            wiced_bt_ble_set_raw_scan_response_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                                cy_bt_scan_rsp_packet_data);

            if (app_session_service_device_information_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)
            {
                uint8_t device_info_code0002_response[18] = {0}; 

                device_info_code0002_response[1] = 0x02;//conversion to big endian for 0x0002.
                memcpy(device_info_code0002_response+2, &cy_bt_scan_rsp_packet_data[0].p_data[0], cy_bt_scan_rsp_packet_data[0].len);

                wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                handle,
                                (cy_bt_scan_rsp_packet_data[0].len+2),
                                (uint8_t *)device_info_code0002_response,NULL);

                sm_log(SM_LOG_DEBUG, "Sending Notification.\r\n");
            }
            else
            {
                sm_log(SM_LOG_DEBUG, "Notification Not Enable.\r\n");
            }
            // /**
            //  * iOS need device indicate device_name has changed, and discovery again database*/
            // status = wiced_bt_gatt_server_send_indication(app_server_context.bt_conn_id,
            //                                         HDLC_GATT_SERVICE_CHANGED_VALUE,
            //                                         app_gatt_service_changed_len,
            //                                         app_gatt_service_changed,
            //                                         NULL);
            // sm_log(SM_LOG_DEBUG, "Device Name update, Indication Status: %d \n", status);
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "device info code 0002 error:%04x", APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]));
        }
    }

    return status; 
}

/*
 * Function Name: app_bt_char_session_service_time_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_time_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_time_get(app_session_service_ext_time_char);
    
    return status; 
}


/*
 * Function Name:
 * app_bt_char_session_service_time_write_handler
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_time_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 4)//msg len verify.
    {
        bt_adapter_time_set(pBuf ,len);
    }
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_battery_power_remaining_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_battery_power_remaining_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_battery_power_remaining_get(app_session_service_ext_battery_power_remaining_char);
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_device_lock_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_device_lock_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_device_lock_get(app_session_service_ext_device_lock_char);
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_device_lock_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_device_lock_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    SysStatus_u cur_sysSta=0; 

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    /** When heated, do not lock **/
    cur_sysSta = get_system_status();
    if ((HEATTING_TEST == cur_sysSta)||
        (HEATTING_STANDARD == cur_sysSta)||
        (HEATTING_BOOST == cur_sysSta)||
        (HEATTING_CLEAN == cur_sysSta)||
        (HEAT_MODE_TEST_VOLTAGE == cur_sysSta)||
        (HEAT_MODE_TEST_POWER == cur_sysSta)||
        (HEAT_MODE_TEST_TEMP == cur_sysSta)||
        (HEATTING_CLEAN == cur_sysSta)||
        (bt_adapter_findmy_running_state_get()))//fixing bugs 1950795
        //(CHARGING == cur_sysSta))//fixing bugs:1954879
    {
        return status; 
    }

    if (len == 1)//msg len verify.
    {
        bt_adapter_device_lock_set(pBuf ,len);
    }
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_records_and_event_log_transmit_start
 * 
 *
 * Function Description:
 * @brief  The function is invoked by report records .
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_records_and_event_log_transmit_start(uint16_t handle)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
    
    session_service_records_xof_t SoF[] = {{0, 0},};

    SoF->data = 0xFFFFFFFE;

    if (handle == HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE)
    {
        SoF->total_records_count = bt_adapter_session_records_number_get();
    }
    else if (handle == HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE)
    {
        SoF->total_records_count = bt_adapter_event_log_number_get();
    }

    update_idl_delay_time();				// 阻止休眠

    u32_endian_swap(&SoF[0].data);
    u32_endian_swap(&SoF[0].total_records_count);

    sm_log(SM_LOG_DEBUG, "session records handle NTF: ");
    /***************************************************Start of File*********************************************************/
    
    cy_rslt_t rslt = CY_RTOS_TIMEOUT;
    #ifdef GATT_CONGESTED_WORKAROUND
    rslt = cy_rtos_semaphore_get(&gatt_congested, GATT_CONGESTED_TIMEOUT/*CY_RTOS_NEVER_TIMEOUT*/);
    #endif
    status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                        handle,
                        sizeof(SoF[0]),
                        (uint8_t *)SoF,NULL);
    if (status == WICED_BT_GATT_SUCCESS)
    {
        gatt_ntf_congested_duration = 0;
        sm_log(SM_LOG_DEBUG, "SoF Sending Successful\n");
    }
    else if((status == WICED_BT_GATT_CONGESTED)&&(rslt == CY_RTOS_TIMEOUT))
    {
    #ifdef GATT_CONGESTED_WORKAROUND
        gatt_ntf_congested_duration = gatt_ntf_congested_duration + GATT_CONGESTED_TIMEOUT;
        if (gatt_ntf_congested_duration >= bGattCongestedTimeout)//start calcu when congested.
        {
            bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
        }
    #endif
        sm_log(SM_LOG_NOTICE, "%s NTF Fail, sta: 0x%04X\n", __func__, status);
    }

    return status; 
}

/*
 * Function Name: app_bt_char_session_records_and_event_log_transmit_finish
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_records_and_event_log_transmit_finish(uint16_t handle)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    const uint8_t EoF[4] = {0xFF, 0xFF, 0xFF, 0xFD};//bug2113187, define to const variable.
    
    update_idl_delay_time();				// 阻止休眠
    
    /***************************************************End of File*********************************************************/
    cy_rslt_t rslt = CY_RTOS_TIMEOUT;
    #ifdef GATT_CONGESTED_WORKAROUND
    rslt = cy_rtos_semaphore_get(&gatt_congested, GATT_CONGESTED_TIMEOUT/*CY_RTOS_NEVER_TIMEOUT*/);
    #endif

    sm_log(SM_LOG_DEBUG, "EOF Data: %02x %02x %02x %02x \r\n", EoF[0], EoF[1], EoF[2], EoF[3]);
	
    status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                        handle,
                        sizeof(EoF),
                        (uint8_t *)EoF,NULL);
    if (status == WICED_BT_GATT_SUCCESS)
    {
        gatt_ntf_congested_duration = 0;
        sm_log(SM_LOG_DEBUG, "%d, EoF Sending Successful\n", handle);//bug2113187
    }
    else  if((status == WICED_BT_GATT_CONGESTED)&&(rslt == CY_RTOS_TIMEOUT))
    {
    #ifdef GATT_CONGESTED_WORKAROUND
        gatt_ntf_congested_duration = gatt_ntf_congested_duration + GATT_CONGESTED_TIMEOUT;
        if (gatt_ntf_congested_duration >= bGattCongestedTimeout)//start calcu when congested.
        {
            bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
        }
    #endif
        sm_log(SM_LOG_NOTICE, "%s NTF Fail, sta: 0x%04X\n", __func__, status);
    }

    return status; 
}

/*
 * Function Name: app_bt_char_session_service_session_records_payload_proc
 * 
 *
 * Function Description: 
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_session_records_payload_proc(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;
    static bool b_NoResendSta = true;

    // uint8_t ntf_payload_buf[128] = {0};
    // uint8_t ntf_payload_len = 0;
    uint16_t session_record_num = 0;

    if (b_NoResendSta == false)
    {
        cy_rslt_t rslt = CY_RTOS_TIMEOUT;
        #ifdef GATT_CONGESTED_WORKAROUND
        rslt = cy_rtos_semaphore_get(&gatt_congested, GATT_CONGESTED_TIMEOUT/*CY_RTOS_NEVER_TIMEOUT*/);
        #endif
        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE,
                            sizeof(app_session_service_ext_records_char[0]),
                            (uint8_t *)app_session_service_ext_records_char,NULL);

        if (status == WICED_BT_GATT_SUCCESS)
        {   
            gatt_ntf_congested_duration = 0;
            b_NoResendSta = true;
//            sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
        }
        else  if((status == WICED_BT_GATT_CONGESTED)&&(rslt == CY_RTOS_TIMEOUT))
        {
        #ifdef GATT_CONGESTED_WORKAROUND
            gatt_ntf_congested_duration = gatt_ntf_congested_duration + GATT_CONGESTED_TIMEOUT;
            if (gatt_ntf_congested_duration >= bGattCongestedTimeout)//start calcu when congested.
            {
                bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
            }
        #endif
            b_NoResendSta = false;
            sm_log(SM_LOG_NOTICE, "%s NTF Fail, sta: 0x%04X\n", __func__, status);
        }
        return status;
    }

    session_record_num = bt_adapter_session_records_number_get();

    sm_log(SM_LOG_DEBUG, "session_record_num:%d\n", session_record_num);

    #if 0
    sm_log(SM_LOG_DEBUG, "len: %d data: ", ntf_payload_len);
    for(uint8_t i=0; i<ntf_payload_len; i++)
        sm_log(SM_LOG_DEBUG, " 0x%02x", ntf_payload_buf[i]);
    #endif

    /***************************************************Payload *********************************************************/
    if (session_record_num > 0)
    {
        update_idl_delay_time();				// 阻止休眠

        status = bt_adapter_session_records_get_next(app_session_service_ext_records_char);
        if (status == WICED_BT_GATT_ERROR)
        {
            sm_log(SM_LOG_DEBUG, "bt_adapter_session_records_get_next error, status: %d\n", status);
            
            b_NoResendSta = true;
            return status;
        }

        u32_endian_swap(&app_session_service_ext_records_char[0].count);
        u32_endian_swap(&app_session_service_ext_records_char[0].time_stamp);
        u16_endian_swap(&app_session_service_ext_records_char[0].duration);
        u16_endian_swap(&app_session_service_ext_records_char[0].session_exit_code);
        u16_endian_swap(&app_session_service_ext_records_char[0].z1_max_temp);
        //u16_endian_swap(&app_session_service_ext_records_char[0].z2_max_temp);
        u16_endian_swap(&app_session_service_ext_records_char[0].battery_max_temp);

        cy_rslt_t rslt = CY_RTOS_TIMEOUT;
        #ifdef GATT_CONGESTED_WORKAROUND
        rslt = cy_rtos_semaphore_get(&gatt_congested, GATT_CONGESTED_TIMEOUT/*CY_RTOS_NEVER_TIMEOUT*/);
        #endif
        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE,
                            sizeof(app_session_service_ext_records_char[0]),
                            (uint8_t *)app_session_service_ext_records_char,NULL);

        if (status == WICED_BT_GATT_SUCCESS)
        {   
            gatt_ntf_congested_duration = 0;
            b_NoResendSta = true;
//            sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
        }
        else  if((status == WICED_BT_GATT_CONGESTED)&&(rslt == CY_RTOS_TIMEOUT))
        {
        #ifdef GATT_CONGESTED_WORKAROUND
            gatt_ntf_congested_duration = gatt_ntf_congested_duration + GATT_CONGESTED_TIMEOUT;
            if (gatt_ntf_congested_duration >= bGattCongestedTimeout)//start calcu when congested.
            {
                bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
            }
        #endif
            b_NoResendSta = false;
            sm_log(SM_LOG_NOTICE, "%s NTF Fail, sta: 0x%04X\n", __func__, status);
        }
    }
    else
    {
        sm_log(SM_LOG_DEBUG, "Notification Disable or Disconnected.\n");
        b_NoResendSta = true;
    }

    return status;
}

/*
 * Function Name: app_bt_char_session_service_session_records_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_session_records_read_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint16_t session_record_num = bt_adapter_session_records_number_get();

    sm_log(SM_LOG_DEBUG, "session_record_num: %ld\n", session_record_num);

    u16_endian_swap(&session_record_num);

    return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, 2, &session_record_num, NULL); /* No need for context, as buff not allocated */
}

/*
 * Function Name: 
 * 
 *
 * Function Description: app_bt_char_session_service_session_records_write_handler
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_session_records_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;// WICED_BT_GATT_ERROR

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 2)//msg len verify.
    {
        uint16_t dummy_record_number = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]);
        sm_log(SM_LOG_DEBUG, "%s dummy_record_number:%d \r\n", __FUNCTION__, dummy_record_number);
        if (4095 < dummy_record_number)
        {
            // dummy_record_number = 4095;
            return status; 
        }
        update_idl_delay_time();				// 阻止休眠
        bt_adapter_session_records_dummy_debug_set(app_session_service_ext_records_char ,dummy_record_number);
        /**
         * sync&setting dummy session record event***/
        app_bt_service_task_char_ntf_event_set(SESSION_SERVICE_DUMMY_SESSION_RECORD_SET_EVT);
        cy_rtos_delay_milliseconds(10);
    }
    else if (len == 4)//msg len verify.
    {
        uint32 *pData = pBuf;
        if (*pData == 0xFDFFFFFF)
        {
            bt_adapter_session_records_clear_all();
        }
    }
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_session_status_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_session_status_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_session_status_get(app_session_service_ext_status_char);
    
    return status; 
}

/*
 * Function Name: 
 * 
 *
 * Function Description: app_bt_char_session_service_session_status_write_handler
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_session_status_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;// WICED_BT_GATT_ERROR

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

#define EndOfSession (1)
#define SelfCleaning (2)

    if (len == 2)//msg len verify.
    {
        if (EndOfSession == pBuf[0])
        {
            bt_adapter_eos_prompt_set((uint8_t*)&pBuf[1], 1);
        }
        else if (SelfCleaning == pBuf[0])
        {
#ifdef DEF_DRY_CLEAN_EN
            bt_adapter_clean_prompt_set((uint8_t*)&pBuf[1], 1);
#else
			return WICED_BT_GATT_ERROR;
#endif
		}
    }

    return status; 
}

/*
 * Function Name: app_bt_char_session_service_findmy_device_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_findmy_device_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_findmy_device_get(app_session_service_ext_findmy_device_char);
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_findmy_device_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_findmy_device_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 2)//msg len verify.
    {
        bt_adapter_findmy_device_set(pBuf ,len);
    }
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_screen_led_control_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_screen_led_control_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_screen_led_brightness_control_get(app_session_service_ext_screen_led_control_char);

    return status; 
}

/*
 * Function Name: app_bt_char_session_service_screen_led_control_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_screen_led_control_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 1)//msg len verify.
    {
        if ((0 <= pBuf[0])&&(pBuf[0] <= 100))
        {
            bt_adapter_screen_led_brightness_control_set(pBuf ,len);
        }
    }

    //to be continue
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_factory_reset_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_factory_reset_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 1)//msg len verify.
    {
        if (pBuf[0] == 0x01)
        {
            /**
             * sync&setting factory reset event***/
            app_bt_service_task_char_ntf_event_set(SESSION_SERVICE_FACTORY_RESET_SET_EVT);
            return status;
        }

        bt_adapter_factory_reset_ble(pBuf ,len);
    }
    
    return status;
}

/*
 * Function Name: app_bt_char_session_service_heating_prof_sel_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);


    //to be continue
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_header
 * 
 *
 * Function Description: 
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_header(uint8_t *heating_profile_num)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

    uint8_t ntf_payload_buf[2] = {0};
    // uint8_t ntf_payload_len = 0, restart_profile_index = 0;
    // uint8_t heating_profile_num = 0;
    uint8_t ntf_rsp_opcode_failure_t[3] = {HEATING_PROF_SEL_OPCODE04, 0xDE, 0xAD};

    uint8_t heating_profile_num_temp[12] = {0};

    *heating_profile_num = bt_adapter_heating_prof_sel_profiles_installed_index_number_get(&heating_profile_num_temp[0]);
    // restart_profile_index = 0;
    
    sm_log(SM_LOG_DEBUG, "%s heating_profile_num:%d\n", __FUNCTION__, *heating_profile_num);

    #if 0
    sm_log(SM_LOG_DEBUG, "len: %d data: ", ntf_payload_len);
    for(uint8_t i=0; i<ntf_payload_len; i++)
        sm_log(SM_LOG_DEBUG, " 0x%02x", ntf_payload_buf[i]);
    #endif

    update_idl_delay_time();				// 阻止休眠

    if (*heating_profile_num > 10)
    {
        *heating_profile_num = 0;
        wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                HDLC_SESSION_SERVICE_HEATING_PROF_SEL_VALUE,
                                                sizeof(ntf_rsp_opcode_failure_t),
                                                (uint8_t *)ntf_rsp_opcode_failure_t,NULL);
        return status;
    }
    else
    {
    /***************************************************Payload Heater*********************************************************/
        ntf_payload_buf[0] = HEATING_PROF_SEL_OPCODE04;
        ntf_payload_buf[1] = *heating_profile_num;
        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                        HDLC_SESSION_SERVICE_HEATING_PROF_SEL_VALUE,
                                                        2,
                                                        (uint8_t *)ntf_payload_buf,NULL);                                   
        if (status == WICED_BT_GATT_SUCCESS)
        {
            sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
        }
    }

    return status;
}

/*
 * Function Name: app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_proc
 * 
 *
 * Function Description: 
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_proc(uint8_t heating_profile_index)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint8_t ntf_payload_buf[HEATING_PROFILE_BUF_SIZE] = {0};
    uint8_t ntf_payload_len = 0;//, restart_profile_index = 0;
    uint16_t heating_profile_num = 0;

    uint8_t heating_profile_num_temp[12] = {0};

    heating_profile_num = bt_adapter_heating_prof_sel_profiles_installed_index_number_get(&heating_profile_num_temp[0]);
    // restart_profile_index = 0;
    
    sm_log(SM_LOG_DEBUG, "%s heating_profile_num:%d\n", __FUNCTION__, heating_profile_num);

    #if 0
    sm_log(SM_LOG_DEBUG, "len: %d data: ", ntf_payload_len);
    for(uint8_t i=0; i<ntf_payload_len; i++)
        sm_log(SM_LOG_DEBUG, " 0x%02x", ntf_payload_buf[i]);
    #endif

    /***************************************************Payload Data***********************************************************/
    update_idl_delay_time();				// 阻止休眠
    /**********RSP DATA STRUCT*************
     *  1. Original OpCode (uint8)
        2. Profile Index (uint8)

        3. Number of Steps in Profile (unit8)
        4. Each of the steps as:
            1. Time in m/s (uint32)
            2. Z1 Target Temp (uint16)
            3. Z2 Target Temp (unit16)
            
        3. Number of Steps in Profile (unit8)
        4. Each of the steps as:
            1. Time in m/s (uint32)
            2. Z1 Target Temp (uint16)
            3. Z2 Target Temp (unit16)
    **************************************** */ 
    memset(ntf_payload_buf, 0x00, sizeof(ntf_payload_buf));
    ntf_payload_buf[0] = heating_profile_index;//set prof index
    ntf_payload_buf[1] = HEATING_PROFILE_STEPS;//set Number of Steps in Profile (unit8)

    status = bt_adapter_heating_prof_sel_index_payload_get((uint8_t *)&ntf_payload_buf[2], &ntf_payload_len, heating_profile_index);
    if (status == WICED_BT_GATT_ERROR)
    {
        sm_log(SM_LOG_DEBUG, "bt_adapter_heating_prof_sel_index_payload_get error, status: %d\n", status);
        
        return status;
    }

    status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                        HDLC_SESSION_SERVICE_HEATING_PROF_SEL_VALUE,
                        ntf_payload_len+2,//add index&steps
                        (uint8_t *)ntf_payload_buf,NULL);
    if (status == WICED_BT_GATT_SUCCESS)
    {
        sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
    }

    return status;
}

/*
 * Function Name: app_bt_char_session_service_heating_prof_sel_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_heating_prof_sel_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint8_t ntf_rsp_opcode03_success_t[3] = {0x03, 0xBE, 0xEF};
    uint8_t ntf_rsp_opcode_failure_t[3] = {HEATING_PROF_SEL_OPCODE03, 0xDE, 0xAD};

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    switch (len)
    {
        case 1:   //opcode 00/01/04
        {
            if (pBuf[0] == HEATING_PROF_SEL_OPCODE00)
            {
                /**********RSP DATA STRUCT*************
                 *  1. Original OpCode (uint8)
                    2. Base Profile Type (uint8): 0 = Base 1 = Boost
                    3. Base Profile Index Number (uint8)
                    4. Boost Profile Type (uint8):0 = Base 1 = Boost
                    5. Boost Profile Index Number (uint8)
                    * ****************************** */ 
                uint8_t ntf_rsp_opcode00_t[5] = {HEATING_PROF_SEL_OPCODE00, 0x00, 0x05, 0x01, 0x0a};
                uint8_t base_profile_index, boost_profile_index;

                bt_adapter_heating_prof_sel_index_number_get(&base_profile_index, &boost_profile_index);
                ntf_rsp_opcode00_t[2] = base_profile_index;
                ntf_rsp_opcode00_t[4] = boost_profile_index;

                if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                {
                    status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                                    handle,
                                                                    sizeof(ntf_rsp_opcode00_t),
                                                                    (uint8_t *)ntf_rsp_opcode00_t,NULL);
                    if (status == WICED_BT_GATT_SUCCESS)
                    {
                        sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
                    }
                    else
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Sending Fail Reason: %d\n", status);
                    }
                }
            }
            else if (pBuf[0] == HEATING_PROF_SEL_OPCODE01)
            {
                /**********RSP DATA STRUCT*************
                 *  1. Original OpCode (uint8)
                    2. Profile Index Number (uint8)
                    3. Profile Index Number (uint8)
                    . . .
                    4. Profile Index Number (uint8)
                    . . .
                    5. Profile Index Number (uint8)
                    . . .
                    6. Profile Index Number (uint8)
                    . . .
                    7. EoI Number [FF] (uint8)
                    * ****************************** */ 
                uint8_t ntf_rsp_opcode01_t[12] = {0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF};
                uint8_t installed_prof_number = 0;

                installed_prof_number = bt_adapter_heating_prof_sel_profiles_installed_index_number_get(&ntf_rsp_opcode01_t[1]);

                if (installed_prof_number == 0)
                {
                    ntf_rsp_opcode_failure_t[0] = HEATING_PROF_SEL_OPCODE01;
                    
                    if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                                        handle,
                                                                        sizeof(ntf_rsp_opcode_failure_t),
                                                                        (uint8_t *)ntf_rsp_opcode_failure_t,NULL);
                    }
                }
                else
                {
                    if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                                        handle,
                                                                        (installed_prof_number+2),
                                                                        (uint8_t *)ntf_rsp_opcode01_t,NULL);
                    }
                }

                if (status == WICED_BT_GATT_SUCCESS)
                {
                    sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
                }
                else
                {
                    sm_log(SM_LOG_DEBUG, "NTF Sending Fail Reason: %d\n", status);
                }
            }
            // else if (pBuf[0] == HEATING_PROF_SEL_OPCODE04) //opcode 04, rob indicate that Remove heating profile retrieve opcodes 0x02&0x04!
            // {
            //     /**********RSP DATA STRUCT*************
            //      *  1. Original OpCode (uint8) 04
            //         2. Number of Profiles Being Sent (uint8) 255
            //         3. Index Number of Profile
            //         4. Number of Steps in Profile (unit8)
            //         5. Each of the steps as:
            //             1. Time in m/s (uint32)
            //             2. Z1 Target Temp (uint16)
            //             3. Z2 Target Temp (unit16)
            //         6. Index Number of Profile
            //         7. Number of Steps in Profile (unit8)
            //         8. Each of the steps as:
            //             4. Time in m/s (uint32)
            //             5. Z1 Target Temp (uint16)
            //             6. Z2 Target Temp (unit16)
            //         * ****************************** */ 
            //     uint8_t ntf_rsp_opcode04_t[35] = {0x04, 0x01, 0x04, 0x00, 0x01, 0x86, 0xA0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x02, 0x98, 0x10, 0x00, 0xF0, 0x00, 0x96, 0x00,
            //                                     0x03, 0x6E, 0xE8, 0x00, 0xD2, 0x00, 0xDC, 0x00, 0x03, 0xF7, 0xA0, 0x00, 0xD2, 0x00, 0xDC,};
            //     /**
            //      * report profiles payload buf***/
            //     app_bt_service_task_char_ntf_event_set(SESSION_SERVICE_HEATING_PROF_SEL_NTF_EVT);
            // }
        }
            break;
        case 2:   //opcode 02, rob indicate that Remove heating profile retrieve opcodes 0x02&0x04!
        {
            // if (pBuf[0] == HEATING_PROF_SEL_OPCODE02)
            // {
            //     /**********RSP DATA STRUCT*************
            //      *  1. Original OpCode (uint8)
            //         2. Profile Index (uint8)

            //         3. Number of Steps in Profile (unit8)
            //         4. Each of the steps as:
            //             1. Time in m/s (uint32)
            //             2. Z1 Target Temp (uint16)
            //             3. Z2 Target Temp (unit16)

            //         3. Number of Steps in Profile (unit8)
            //         4. Each of the steps as:
            //             1. Time in m/s (uint32)
            //             2. Z1 Target Temp (uint16)
            //             3. Z2 Target Temp (unit16)
            //         * ****************************** */ 
            //     // uint8_t ntf_rsp_opcode02_t[110] = {0x02, 0xBE, 0x04, 0x00, 0x01, 0x86, 0xA0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x02, 0x98, 0x10, 0x00, 0xF0, 0x00, 0x96, 0x00,
            //     //                                 0x03, 0x6E, 0xE8, 0x00, 0xD2, 0x00, 0xDC, 0x00, 0x03, 0xF7, 0xA0, 0x00, 0xD2, 0x00, 0xDC,};
            //     uint8_t ntf_rsp_opcode02_t[HEATING_PROFILE_BUF_SIZE] = {0};
            //     uint8_t length = 0;
            //     ntf_rsp_opcode02_t[0] = pBuf[0];//set opcode(0x02)
            //     ntf_rsp_opcode02_t[1] = pBuf[1];//set prof index(0xBE)
            //     ntf_rsp_opcode02_t[2] = HEATING_PROFILE_STEPS;//set Number of Steps in Profile (unit8)
            //     /**
            //      * select profile data and update rsp buf***/
            //     status = bt_adapter_heating_prof_sel_index_payload_get((uint8_t *)&ntf_rsp_opcode02_t[3], &length, pBuf[1]);
            //     sm_log(SM_LOG_DEBUG, "status: %d ,length: %d\n", status, length);

            //     if (length > 0)
            //     {
            //         status =  wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
            //                             handle,
            //                             sizeof(ntf_rsp_opcode02_t),
            //                             (uint8_t *)ntf_rsp_opcode02_t, NULL);
            //         if (status == WICED_BT_GATT_SUCCESS)
            //         {
            //             sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
            //         }
            //         else
            //         {
            //             sm_log(SM_LOG_DEBUG, "NTF Sending Fail Reason: %d\n", status);
            //         }
            //     }
            //     else
            //     {
            //         ntf_rsp_opcode_failure_t[0] = HEATING_PROF_SEL_OPCODE02;
            //         status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
            //                                                         handle,
            //                                                         sizeof(ntf_rsp_opcode_failure_t),
            //                                                         (uint8_t *)ntf_rsp_opcode_failure_t,NULL);
            //     }
            // }
        }
            break;
        case 3:   //opcode 03
        {
            if (pBuf[0] == HEATING_PROF_SEL_OPCODE03)
            {
                uint8_t heating_mode = pBuf[1];
                uint8_t profile_index = pBuf[2];
                SysStatus_u cur_sysSta=0; 

                /**********RSP DATA STRUCT*************
                 *  1. Original OpCode (uint8)
                    2. Profile Index (uint8)
                    3. 0xEF, if setting successful
                    or,
                    1. Original OpCode (uint8)
                    2. 0xDEAD(uint16), if setting failure
                    * ****************************** */ 

                /** When heated, do not upgrade **/
                cur_sysSta = get_system_status();
                if ((HEATTING_TEST == cur_sysSta)||
                    (HEATTING_STANDARD == cur_sysSta)||
                    (HEATTING_BOOST == cur_sysSta)||
                    (HEATTING_CLEAN == cur_sysSta)||
                    (HEAT_MODE_TEST_VOLTAGE == cur_sysSta)||
                    (HEAT_MODE_TEST_POWER == cur_sysSta)||
                    (HEAT_MODE_TEST_TEMP == cur_sysSta)||
                    (status != WICED_BT_GATT_SUCCESS))//setting failure 
                {
                    sm_log(SM_LOG_DEBUG, "%s It is currently heating and cannot update heating profile!\r\n", __func__);
                    
                    if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            handle,
                            sizeof(ntf_rsp_opcode_failure_t),
                            (uint8_t *)ntf_rsp_opcode_failure_t,NULL);
                    }
                    return status; 
                }

                /** setting profile data to specified heating mode, judge whehter success**/
                status = bt_adapter_heating_prof_sel_index_set(heating_mode, profile_index);

                sm_log(SM_LOG_DEBUG, "bt_adapter_heating_prof_sel_index_set status: %d\n", status);

                if (status == WICED_BT_GATT_SUCCESS)
                {
                    if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            handle,
                            sizeof(ntf_rsp_opcode03_success_t),
                            (uint8_t *)ntf_rsp_opcode03_success_t,NULL);
                    }
                }
                else
                {
                    if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            handle,
                            sizeof(ntf_rsp_opcode_failure_t),
                            (uint8_t *)ntf_rsp_opcode_failure_t,NULL);
                    }
                }

                if (status == WICED_BT_GATT_SUCCESS)
                {
                    sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
                }
                else
                {
                    sm_log(SM_LOG_DEBUG, "NTF Sending Fail Reason: %d\n", status);
                }
                return WICED_BT_GATT_SUCCESS;
            }
        }
            break;
        default:
            sm_log(SM_LOG_DEBUG, "opcode len error!\n");
            break;
    }

    return status; 
}

/*
 * Function Name: app_bt_char_session_service_haptic_set_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_haptic_set_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_haptic_get(app_session_service_ext_haptic_set_char);
    
    return status; 
}

/*
 * Function Name: 
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_haptic_set_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 1)//msg len verify.
    {        
        if ((0 <= pBuf[0])&&(pBuf[0] <= 100))
        {
            bt_adapter_haptic_set(pBuf, len);
        }
    }
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_buzzer_speaker_set_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_buzzer_speaker_set_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_buzzer_speaker_loudness_get(app_session_service_ext_buzzer_speaker_set_char);
    
    return status; 
}

/*
 * Function Name: app_bt_char_session_service_buzzer_speaker_set_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_session_service_buzzer_speaker_set_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif
    
    if (len == 1)//msg len verify.
    {
        bt_adapter_buzzer_speaker_loudness_set(pBuf, len);
    }
    
    return status; 
}


/*
 * Function Name: app_bt_char_debug_service_last_error_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_debug_service_last_error_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    bt_adapter_last_error_get(app_debug_service_ext_last_error_char);

    //swap to big-endian and transfer
    u32_endian_swap(&app_debug_service_ext_last_error_char[0].timestamp);
    u16_endian_swap(&app_debug_service_ext_last_error_char[0].error_code);
    
    return status; 
}

/*
 * Function Name: app_bt_char_debug_service_event_log_payload_proc
 * 
 *
 * Function Description: 
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_debug_service_event_log_payload_proc(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

    static bool b_NoResendSta = true;

    // uint8_t ntf_payload_buf[128] = {0};
    // uint8_t ntf_payload_len = 0;
    uint16_t event_log_num = 0;

    if (b_NoResendSta == false)
    {
        cy_rslt_t rslt = CY_RTOS_TIMEOUT;
        #ifdef GATT_CONGESTED_WORKAROUND
        rslt = cy_rtos_semaphore_get(&gatt_congested, GATT_CONGESTED_TIMEOUT/*CY_RTOS_NEVER_TIMEOUT*/);
        #endif
        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE,
                            sizeof(app_debug_service_ext_event_log_char[0]),
                            (uint8_t *)app_debug_service_ext_event_log_char,NULL);

        if (status == WICED_BT_GATT_SUCCESS)
        {
            gatt_ntf_congested_duration = 0;
            b_NoResendSta = true;
//            sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
        }
        else  if((status == WICED_BT_GATT_CONGESTED)&&(rslt == CY_RTOS_TIMEOUT))
        {
        #ifdef GATT_CONGESTED_WORKAROUND
            gatt_ntf_congested_duration = gatt_ntf_congested_duration + GATT_CONGESTED_TIMEOUT;
            if (gatt_ntf_congested_duration >= bGattCongestedTimeout)//start calcu when congested.
            {
                bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
            }
        #endif
            b_NoResendSta = false;
            sm_log(SM_LOG_NOTICE, "%s NTF Fail, sta: 0x%04X\n", __func__, status);
        }
        return status;
    }

    event_log_num = bt_adapter_event_log_number_get();
    
    sm_log(SM_LOG_DEBUG, "event_log_num:%d\n", event_log_num);

    #if 0
    sm_log(SM_LOG_DEBUG, "len: %d data: ", ntf_payload_len);
    for(uint8_t i=0; i<ntf_payload_len; i++)
        sm_log(SM_LOG_DEBUG, " 0x%02x", ntf_payload_buf[i]);
    #endif

    /***************************************************Payload *********************************************************/
    if (event_log_num > 0)
    {
        update_idl_delay_time();				// 阻止休眠

        status = bt_adapter_event_log_get_next(app_debug_service_ext_event_log_char);
        if (status == WICED_BT_GATT_ERROR)
        {
            sm_log(SM_LOG_DEBUG, "bt_adapter_event_log_get_next error, status: %d\n", status);

            b_NoResendSta = true;
            return status;
        }

        u32_endian_swap(&app_debug_service_ext_event_log_char[0].count);
        u32_endian_swap(&app_debug_service_ext_event_log_char[0].timestamp);

        cy_rslt_t rslt = CY_RTOS_TIMEOUT;
        #ifdef GATT_CONGESTED_WORKAROUND
        rslt = cy_rtos_semaphore_get(&gatt_congested, GATT_CONGESTED_TIMEOUT/*CY_RTOS_NEVER_TIMEOUT*/);
        #endif
        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE,
                            sizeof(app_debug_service_ext_event_log_char[0]),
                            (uint8_t *)app_debug_service_ext_event_log_char,NULL);

        if (status == WICED_BT_GATT_SUCCESS)
        {
            gatt_ntf_congested_duration = 0;
            b_NoResendSta = true;
//            sm_log(SM_LOG_DEBUG, "Payload Sending Successful\n");
        }
        else  if((status == WICED_BT_GATT_CONGESTED)&&(rslt == CY_RTOS_TIMEOUT))
        {
        #ifdef GATT_CONGESTED_WORKAROUND
            gatt_ntf_congested_duration = gatt_ntf_congested_duration + GATT_CONGESTED_TIMEOUT;
            if (gatt_ntf_congested_duration >= bGattCongestedTimeout)//start calcu when congested.
            {
                bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
            }
        #endif
            b_NoResendSta = false;
            sm_log(SM_LOG_NOTICE, "%s NTF Fail, sta: 0x%04X\n", __func__, status);
        }
    }
    else
    {
        b_NoResendSta = true;
    }

    return status;
}

/*
 * Function Name: app_bt_char_debug_service_event_log_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_debug_service_event_log_read_handler(uint16_t conn_id, wiced_bt_gatt_opcode_t opcode)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint16_t event_log_num = bt_adapter_event_log_number_get();

    sm_log(SM_LOG_DEBUG, "event_log_num: %ld\n", event_log_num);

    u16_endian_swap(&event_log_num);

    return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, 2, &event_log_num, NULL); /* No need for context, as buff not allocated */
}

/*
 * Function Name: app_bt_char_debug_service_event_log_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_debug_service_event_log_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 2)//msg len verify.
    {
        uint16_t dummy_record_number = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]);
        sm_log(SM_LOG_DEBUG, "%s dummy_record_number:%d \r\n", __FUNCTION__, dummy_record_number);
        if (4095 < dummy_record_number)
        {
            // dummy_record_number = 4095;
            return status; 
        }
        update_idl_delay_time();				// 阻止休眠
        bt_adapter_event_log_dummy_debug_set(app_debug_service_ext_event_log_char ,dummy_record_number);
        /**
         * sync&setting dummy event log event***/
        app_bt_service_task_char_ntf_event_set(DEBUG_SERVICE_DUMMY_EVENT_LOG_SET_EVT);
        cy_rtos_delay_milliseconds(10);
    }
    else if (len == 4)//msg len verify.
    {
        uint32 *pData = pBuf;
        if (*pData == 0xFDFFFFFF)
        {
            bt_adapter_event_log_clear_all();
        }
    }
    
    return status; 
}

/*
 * Function Name: app_bt_char_debug_service_lifecycle_data_read_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_debug_service_lifecycle_data_read_handler(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    //to be continue
    
    return status; 
}

/*
 * Function Name: app_bt_char_debug_service_lifecycle_data_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_debug_service_lifecycle_data_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 1)//msg len verify.
    {
        debug_service_lifecycle_data_opcode01_rsp_t lifecycle_data;

        if (pBuf[0] == 0x01)//Opcode 01, Return All Data
        {
            /*
                int16_t Min_battery_temperature;
                int16_t Max_battery_temperature;
                uint16_t Min_battery_voltage;
                uint16_t Max_battery_voltage;
                int16_t Max_battery_charge_current;
                int16_t Max_battery_discharge_current;
                uint32_t Total_charge_time;
                uint32_t No_of_fully_charged_count;
                uint32_t No_of_complete_session;
                uint32_t No_of_incomplete_session;
                int16_t Min_Zone1_temperature;
                int16_t Max_Zone1_temperature;
                int16_t Min_Zone2_temperature;
                int16_t Max_Zone2_temperature;
            */
            lifecycle_data.Min_battery_temperature = 0xFFFF;
            lifecycle_data.Max_battery_temperature = 0xFFFF;
            lifecycle_data.Min_battery_voltage = 0xFFFF;
            lifecycle_data.Max_battery_voltage = 0xFFFF;
            lifecycle_data.Max_battery_charge_current = 0xFFFF;
            lifecycle_data.Max_battery_discharge_current = 0xFFFF;
            lifecycle_data.Total_charge_time = 0xFFFFFFFF;
            lifecycle_data.No_of_fully_charged_count = 0xFFFFFFFF;
            lifecycle_data.No_of_complete_session = 0xFFFFFFFF;
            lifecycle_data.No_of_incomplete_session = 0xFFFFFFFF;
            lifecycle_data.Min_Zone1_temperature = 0xFFFF;
            lifecycle_data.Max_Zone1_temperature = 0xFFFF;
//            lifecycle_data.Min_Zone2_temperature = 0xFFFF;
//            lifecycle_data.Max_Zone2_temperature = 0xFFFF;

            bt_adapter_lifecycle_data_all_field_get(&lifecycle_data);

            lifecycle_data.Min_battery_temperature = SWAP_WORD(lifecycle_data.Min_battery_temperature);
            lifecycle_data.Max_battery_temperature = SWAP_WORD(lifecycle_data.Max_battery_temperature);
            lifecycle_data.Min_battery_voltage = SWAP_WORD(lifecycle_data.Min_battery_voltage);
            lifecycle_data.Max_battery_voltage = SWAP_WORD(lifecycle_data.Max_battery_voltage);
            lifecycle_data.Max_battery_charge_current = SWAP_WORD(lifecycle_data.Max_battery_charge_current);
            lifecycle_data.Max_battery_discharge_current = SWAP_WORD(lifecycle_data.Max_battery_discharge_current);
            
            u32_endian_swap(&lifecycle_data.Total_charge_time);
            u32_endian_swap(&lifecycle_data.No_of_fully_charged_count);
            u32_endian_swap(&lifecycle_data.No_of_complete_session);
            u32_endian_swap(&lifecycle_data.No_of_incomplete_session);
            
            lifecycle_data.Min_Zone1_temperature = SWAP_WORD(lifecycle_data.Min_Zone1_temperature);
            lifecycle_data.Max_Zone1_temperature = SWAP_WORD(lifecycle_data.Max_Zone1_temperature);
//            lifecycle_data.Min_Zone2_temperature = SWAP_WORD(lifecycle_data.Min_Zone2_temperature);
//            lifecycle_data.Max_Zone2_temperature = SWAP_WORD(lifecycle_data.Max_Zone2_temperature);

            if (app_server_context.bt_conn_id && (app_debug_service_lifecycle_data_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
            {
                status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                                handle,
                                                                sizeof(lifecycle_data),
                                                                (uint8_t *)&lifecycle_data,NULL);
                if (status == WICED_BT_GATT_SUCCESS)
                {
                    sm_log(SM_LOG_DEBUG, "NTF Sending Successful\n");
                }
                else
                {
                    sm_log(SM_LOG_DEBUG, "NTF Sending Failure, Please update MTU.\n");
                }
            }
            else
            {
                sm_log(SM_LOG_DEBUG, "NTF Not Enable!\n");
            }
        }
        else if (pBuf[0] == 0x02)//Opcode 02, Clear/Reset Data Fields.
        {
            //clear all parameter
            status = bt_adapter_lifecycle_data_clear_all();
            if (status == WICED_BT_GATT_SUCCESS)
            {
                uint8_t opcode02_rsp_temp = 0x00;
                if (app_server_context.bt_conn_id && (app_debug_service_lifecycle_data_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                {
                    status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                                    handle,
                                                                    1,
                                                                    (uint8_t *)&opcode02_rsp_temp,NULL);
                    if (status == WICED_BT_GATT_SUCCESS)
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Sending Successful\n");
                    }
                    else
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Sending Failure, or Please update MTU.\n");
                    }
                }
                else
                {
                    sm_log(SM_LOG_DEBUG, "NTF Not Enable!\n");
                }
            }
        }
    }
    else if (len == 2)//msg len verify.
    {
        uint8_t opcode00_rsp_len = 0;

        if (pBuf[0] == 0x00)//Opcode 00, Return Data for individual field
        {
            switch (pBuf[1])
            {
                case MIN_BAT_TEMP:
                case MAX_BAT_TEMP:
                case MIN_BAT_VOL:
                case MAX_BAT_VOL:
                case MAX_BAT_CHARGE_CURRENT:
                case MAX_BAT_DISCHARGE_CURRENT:
                case MIN_ZONE1_TEMP:
                case MAX_ZONE1_TEMP:
//                case MIN_ZONE2_TEMP:
//                case MAX_ZONE2_TEMP://2 octets
                {
                    uint16_t value_16 = (uint16_t)bt_adapter_lifecycle_data_individual_field_get(pBuf[1]);

                    value_16 = SWAP_WORD(value_16);//swap to big endian
                    opcode00_rsp_len = 2;
                    
                    sm_log(SM_LOG_DEBUG, "%s swap value_16: 0x%04x\n", __FUNCTION__, value_16);

                    if (app_server_context.bt_conn_id && (app_debug_service_lifecycle_data_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            handle,
                            opcode00_rsp_len,
                            (uint8_t *)&value_16, NULL);
                    }
                    else
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Not Enable!\n");
                    }
                }
                    break;
                case TOTAL_CHARGE_TIME:
                case FULLY_CHARGED_COUNT:
                case COMPLETE_SESSION:
                case INCOMPLETE_SESSION://4 octets
                {
                    uint32_t value_32 = (uint32_t)bt_adapter_lifecycle_data_individual_field_get(pBuf[1]);

                    u32_endian_swap(&value_32);
                    opcode00_rsp_len = 4;

                    sm_log(SM_LOG_DEBUG, "%s swap value_32: 0x%08x\n", __FUNCTION__, value_32);

                    if (app_server_context.bt_conn_id && (app_debug_service_lifecycle_data_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                    {
                        wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            handle,
                            opcode00_rsp_len,
                            (uint8_t *)&value_32, NULL);
                    }
                    else
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Not Enable!\n");
                    }
                    break;
                }
                default:
                    sm_log(SM_LOG_DEBUG, "%s param error\n", __FUNCTION__);
                    break;
            }
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "%s Opcode00 error\n", __FUNCTION__);
        }
    }

    return status; 
}

/*
 * Function Name: app_bt_char_device_management_service_version_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_management_service_version_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint8_t rsp_temp_buf[1] = {CONTROL_START};

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d ver_len:%d cur_len:%d p_Buf:\r\n", __FUNCTION__, handle, sizeof(device_management_service_version), len);

    #if 1
    for(uint16_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == sizeof(device_management_service_version))//msg len verify.
    {
        device_management_service_version.major_version = pBuf[0];//u8
        device_management_service_version.minor_version = pBuf[1];//u8
        device_management_service_version.sw_revision_patch = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[2], pBuf[3]);//u16
        device_management_service_version.object_length = APP_BT_MERGE_4BYTES_BIG_ENDDIAN(pBuf[4], pBuf[5], pBuf[6], pBuf[7]);//u32
        device_management_service_version.object_id = pBuf[8];//u8
        memcpy(device_management_service_version.object_md5, &pBuf[9], 5); //md5 length = 5, 

        sm_log(SM_LOG_DEBUG, "%s major_version: %02x\n", __FUNCTION__, device_management_service_version.major_version);
        sm_log(SM_LOG_DEBUG, "%s minor_version: %02x\n", __FUNCTION__, device_management_service_version.minor_version);
        sm_log(SM_LOG_DEBUG, "%s sw_revision_patch: %04x\n", __FUNCTION__, device_management_service_version.sw_revision_patch);
        sm_log(SM_LOG_DEBUG, "%s object_length: %08x\n", __FUNCTION__, device_management_service_version.object_length);
        sm_log(SM_LOG_DEBUG, "%s object_id: %02x\n", __FUNCTION__, device_management_service_version.object_id);
        sm_log(SM_LOG_DEBUG, "%s object_md5: 0x", __FUNCTION__);
        for(uint8_t i=0; i<5; i++)
        {
            sm_log(SM_LOG_DEBUG, "%02x", device_management_service_version.object_md5[i]);
        }
        sm_log(SM_LOG_DEBUG, "\n");

        //clear 0 as a start
        device_manage_payload_data_len = 0;        
        device_manage_payload_package_cnt = 0;
        writeAddr_offset = 0;

        
        if (device_management_service_version.object_id == HEATING_PROFILE_OBJECT)//heating profile 1
        {
            //Display loading UI
            set_system_external_even(EXT_EVENT_LOADING);
        }
        else if (device_management_service_version.object_id == SCREEN_CONFIG_OBJECT)//ui image 
        {
            #if (MD5_SUPPORT == 1)
            //to be continue
            mbedtls_md5_init(&md5_ctx);
            mbedtls_md5_starts(&md5_ctx);
            #elif (SHA256_SUPPORT == 1)
            /**
             * sha256 initialize
             */
            mbedtls_sha256_init( &sha256_ctx );
            if( mbedtls_sha256_starts_ret( &sha256_ctx, 0 ) != 0 )
            {
                mbedtls_sha256_free( &sha256_ctx );
                sm_log(SM_LOG_DEBUG, "\n%s mbedtls_sha256_starts_ret error, and reinit...!\n", __FUNCTION__);

                /* reinit*/
                mbedtls_sha256_init( &sha256_ctx );
                if( mbedtls_sha256_starts_ret( &sha256_ctx, 0 ) != 0 )
                {
                    mbedtls_sha256_free( &sha256_ctx );
                    sm_log(SM_LOG_DEBUG, "\n%s mbedtls_sha256_starts_ret error!\n", __FUNCTION__);
                }
            }
            #endif

            //Display loading UI
            set_system_external_even(EXT_EVENT_LOADING);
        }

        //notify to control uuid 'CONTROL_START'
        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                        HDLC_DEVICE_MANAGEMENT_SERVICE_CONTROL_VALUE,
                                                        sizeof(rsp_temp_buf),
                                                        (uint8_t *)rsp_temp_buf,NULL);
        if (status == WICED_BT_GATT_SUCCESS)
        {
            sm_log(SM_LOG_DEBUG, "NTF Sending Successful\n");
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "NTF Sending Failure, Please update MTU.\n");
        }
    }

    return status; 
}

/*
 * Function Name: app_bt_char_device_management_service_control_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_management_service_control_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    uint8_t rsp_temp_buf[1] = {CONTROL_ERROR};

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint16_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

    if (len == 1)//msg len verify.
    {
        switch(pBuf[0])
        {
            case CONTROL_TRANSFER_COMPLETE:
            {
                if (device_management_service_version.object_id == MARKET_SPEC_SOFT_OBJECT)//maket specfic 0
                {
                    device_manage_payload_data_len = 0;
                    //to be continue

                }
                else if (device_management_service_version.object_id == HEATING_PROFILE_OBJECT)//heating profile 1
                {
                    device_manage_payload_data_len = 0;
                    
                    //Display Upgrade Successful         
                    set_system_external_even(EXT_EVENT_HEATING_PROFILES);
                }
                else if (device_management_service_version.object_id == SCREEN_CONFIG_OBJECT)//ui image 2
                {
                    #if (MD5_SUPPORT == 1)
                    uint8_t md5_check_buf[32]={0};
                    mbedtls_md5_finish(&md5_ctx, md5_check_buf);
                    mbedtls_md5_free(&md5_ctx);    

                    sm_log(SM_LOG_DEBUG, "Current rev packet:%d  total len:%d MD5 caculate value:\r\n", device_manage_payload_package_cnt, device_manage_payload_data_len);

                    #if 1
                    for(uint16_t i=0; i<16; i++)
                    {
                        sm_log(SM_LOG_DEBUG, "%02x", md5_check_buf[i]);
                    }
                    sm_log(SM_LOG_DEBUG, "\r\n");
                    #endif

                    if(data_array_is_equal(&device_management_service_version.object_md5[0], md5_check_buf, 5))
                    {
                        rsp_temp_buf[0] = CONTROL_SUCCESS;
                    }
                    #elif (SHA256_SUPPORT == 1)
                    uint8_t hash_value[32]={0};
                    mbedtls_sha256_finish_ret( &sha256_ctx, hash_value );
                    mbedtls_sha256_free( &sha256_ctx ); 

                    sm_log(SM_LOG_DEBUG, "\nCurrent rev packet:%d  payload total len:%d sha256 caculate value:\r\n", device_manage_payload_package_cnt, (device_manage_payload_data_len-160) );

                    #if 1
                    for(uint16_t i=0; i<sizeof(hash_value); i++)
                    {
                        sm_log(SM_LOG_DEBUG, "%02x", hash_value[i]);
                    }
                    sm_log(SM_LOG_DEBUG, "\r\n");
                    #endif

                    if (1 == bt_adapter_uECC_message_sign_verify(uimage_challenge, sizeof(uimage_challenge), myName_public_key, uimage_challenge_signature))
                    {
                        sm_log(SM_LOG_INFO, "\n uimage_challenge_signature verify successfully!\r\n");
                    }
                    else
                    {
                        sm_log(SM_LOG_INFO, "\n uimage_challenge_signature verify failure!\r\n");
                    }

                    if (1 == bt_adapter_uECC_sha256_sign_verify(hash_value, sizeof(hash_value), myName_public_key, uimage_protobuf_payload_signature))
                    {
                        rsp_temp_buf[0] = CONTROL_SUCCESS;

                        //Display Upgrade Successful         
                        set_system_external_even(EXT_EVENT_UIMAGE_CUSTOMIZE);

                        sm_log(SM_LOG_INFO, "\n uimage_protobuf_payload_signature verify successfully!\r\n");
                    }
                    else
                    {
                        uimage_record_erase_sector(uImageSavedFlashADDR, 4096);//擦除 4 K, 主要把nanopb buf擦除掉
                        sm_log(SM_LOG_INFO, "\n uimage_protobuf_payload_signature verify failure!\r\n");
                        
                        //STOP Display Loading UI        
                        set_system_external_even(EXT_EVENT_LOADING_STOP);
                    }
					#endif

                    //notify to control uuid 
                    status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                                    HDLC_DEVICE_MANAGEMENT_SERVICE_CONTROL_VALUE,
                                                                    sizeof(rsp_temp_buf),
                                                                    (uint8_t *)rsp_temp_buf,NULL);
                    if (status == WICED_BT_GATT_SUCCESS)
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Sending Successful\n");
                    }
                    else
                    {
                        sm_log(SM_LOG_DEBUG, "NTF Sending Failure, Please update MTU.\n");
                    }
                    device_manage_payload_data_len = 0;
                }
            }
                break;
            
            default:
            {
                sm_log(SM_LOG_DEBUG, "%s error, opcode: %d\n", __FUNCTION__, pBuf[0]);
            }
                break;
        }

    }

    return status; 
}

/*
 * Function Name: app_bt_char_device_management_service_payload_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_management_service_payload_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    uint8_t rsp_temp_buf[1] = {CONTROL_NEXT_PAYLOAD};
    static uint16_t rev_temp_buffer_len = 0;
    static uint16_t RLE_save_temp_buffer_len = 0;
    static uint16_t offset = 0;//相对4K payload进行管理的
	static uint16_t RLE_buf_offset = 0;
    // static uint32_t writeAddr_offset = 0;  //Bug 1951347: [myName] If new value transfer is interrupted, myname read returns new value, rendered image may be broken

#if 0
    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);
    for(uint16_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
#endif

    if (device_management_service_version.object_id == MARKET_SPEC_SOFT_OBJECT)//maket specfic 0
    {
        //to be continue

    }
    else if (device_management_service_version.object_id == HEATING_PROFILE_OBJECT)//heating profile 1
    {
        /**
         * heating_prof len :1080 = (HEATING_PROFILE_LEN * MAX_NUM_HEATING_PROFILE) Bytes 
         * */ 
#define ALL_HEATING_PROFILE_LEN (HEATING_PROFILE_LEN * MAX_NUM_HEATING_PROFILE)

        if ((device_manage_payload_data_len == 0)||(offset >= ALL_HEATING_PROFILE_LEN))
        {
            offset = 0;
            memset(device_manage_payload, 0, sizeof(device_manage_payload));
        }

        device_manage_payload_data_len+=len;

        if (device_manage_payload_data_len > ALL_HEATING_PROFILE_LEN)
        {
            memcpy(&device_manage_payload[offset], pBuf, (ALL_HEATING_PROFILE_LEN - offset));//remain, Intercept and complete
            offset += len;
            
            #if 1
            sm_log(SM_LOG_DEBUG, "device_manage_payload(%d): offset(%d): \r\n", device_manage_payload_data_len, offset);
            for(uint16_t i=0; i<ALL_HEATING_PROFILE_LEN; i++)
            {
                sm_log(SM_LOG_DEBUG, " %02x", device_manage_payload[i]);
            }
            sm_log(SM_LOG_DEBUG, "\n");
            #endif

            status = bt_adapter_heating_prof_sel_profiles_installed_index_array_set(device_manage_payload, ALL_HEATING_PROFILE_LEN);
            
            sm_log(SM_LOG_DEBUG, "ALL_HEATING_PROFILE_LEN setting, status:  %d\r\n", status);

            if (status != WICED_BT_GATT_SUCCESS)
            {
                sm_log(SM_LOG_DEBUG, "%s ,error status: %d\r\n", __FUNCTION__, status);
            }
        }
        else
        {
            memcpy(&device_manage_payload[offset], pBuf, len);
            offset += len;

            if (device_manage_payload_data_len == ALL_HEATING_PROFILE_LEN)
            {
                #if 1
                sm_log(SM_LOG_DEBUG, "device_manage_payload(%d): offset(%d): \r\n", device_manage_payload_data_len, offset);
                for(uint16_t i=0; i<ALL_HEATING_PROFILE_LEN; i++)
                {
                    sm_log(SM_LOG_DEBUG, " %02x", device_manage_payload[i]);
                }
                sm_log(SM_LOG_DEBUG, "\n");
                #endif

                status = bt_adapter_heating_prof_sel_profiles_installed_index_array_set(device_manage_payload, ALL_HEATING_PROFILE_LEN);

                sm_log(SM_LOG_DEBUG, "ALL_HEATING_PROFILE_LEN setting, status:  %d\r\n", status);

                if (status != WICED_BT_GATT_SUCCESS)
                {
                    sm_log(SM_LOG_DEBUG, "%s ,error status: %d\r\n", __FUNCTION__, status);
                }
            }
        }
    }
    else if (device_management_service_version.object_id == SCREEN_CONFIG_OBJECT)//ui image 2
    {
        /******************Header Protobuf***************************************************************** */
        if ((device_manage_payload_data_len == 0)&&(2 <= len))
        {
            s_UImgeMachineState = UI_IMAGE_PROTOBUF_HEADER;
            offset = 0;
            rev_temp_buffer_len = 0;
            memset(device_manage_payload, 0, sizeof(device_manage_payload));
        }

        device_manage_payload_package_cnt++;

        /******************Header+protobuf+payload SHA256 Update***************************************************************** */
        #if (MD5_SUPPORT == 1)
        if (device_manage_payload_data_len <= (device_management_service_version.object_length -160))
        {
            mbedtls_md5_update(&md5_ctx, pBuf, len);
        }
        #elif (SHA256_SUPPORT == 1)
        int32_t object_remain_len = 0;
        object_remain_len = device_management_service_version.object_length - (UIMAGE_CHALLENGE_SIZE + UIMAGE_CHALLENGE_SIGN_SIZE + UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE) -device_manage_payload_data_len;
        // sm_log(SM_LOG_DEBUG, "\n object_remain_len(%ld)\n", object_remain_len);
        if (0 < object_remain_len)//remain
        {
            if (object_remain_len >= len)
            {
                object_remain_len = len;
            }
            // sm_log(SM_LOG_DEBUG, "\n pBUF_len:%ld ,sha256 update(%d), pkg(%ld): %ld\n", len, object_remain_len, device_manage_payload_package_cnt, device_manage_payload_data_len+object_remain_len);

            if( mbedtls_sha256_update_ret( &sha256_ctx, pBuf, object_remain_len) != 0 )
            {
                mbedtls_sha256_free( &sha256_ctx );

                sm_log(SM_LOG_DEBUG, "\n%s mbedtls_sha256_update_ret error!\n", __FUNCTION__);
            }
        }
        #endif
    #if (RLE_SUPPORT == 1)
        rev_temp_buffer_len = len;
    #else
        rev_temp_buffer_len += len;
    #endif
        device_manage_payload_data_len+=len;
        
        switch (s_UImgeMachineState)
        {
            case UI_IMAGE_PROTOBUF_HEADER:
            {
                UImageProtobuf_Heater = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(pBuf[0], pBuf[1]);//protobuf length

                if ((UImageProtobuf_Heater > rev_temp_buffer_len/* 大于当前接收数据大小，继续接收*/)&&(rev_temp_buffer_len < DEVICE_MANAGE_PAYLOAD_SIZE))
                {
                    memcpy(&device_manage_payload[offset], pBuf, len);
                }
                else if (rev_temp_buffer_len >= DEVICE_MANAGE_PAYLOAD_SIZE)
                {
                    s_UImgeMachineState = UI_IMAGE_PROTOBUF_HEADER;
                    rev_temp_buffer_len = 0;
                    offset = 0;
                    memset(device_manage_payload, 0, sizeof(device_manage_payload));
                }
                else if (UImageProtobuf_Heater <= rev_temp_buffer_len)// UImageProtobuf_Heater length
                {
                    memcpy(&device_manage_payload[offset], pBuf, len);

                    /******************Extract Protobuf***************************************************************** */
                    uimage_screen_update_nanopb_decode(&device_manage_payload[2], UImageProtobuf_Heater, &uImageSavedFlashADDR);//header len ==2bytes

                    //Setting current upgrade UI type
                    MyName_uImage_isUpdating_Set(uImageSavedFlashADDR);
                    
                    //if decode success ,change machinstate
                    s_UImgeMachineState = UI_IMAGE_PAYLOAD_PROCESS;

                #if (RLE_SUPPORT == 1)
                    RLE_buf_offset = (2 + UImageProtobuf_Heater);//Header 2 bytes + protobufLen
                    offset = RLE_buf_offset;
                    RLE_save_temp_buffer_len = rev_temp_buffer_len;//=len
                #endif
                }
                #if (RLE_SUPPORT == 0)
                    offset += len;
                #endif
            }
                break;
            case UI_IMAGE_PAYLOAD_PROCESS:
            {
            #if (RLE_SUPPORT == 1)
                if (device_manage_payload_data_remain_count != 0)
                {
                    RLE_buf_offset = 1;//remain ，即单数，需补全
                }
                else
                {
                    RLE_buf_offset = 0;
                }
                
                offset = device_manage_paload_data_remain_offset;

                uint32_t remain_len = device_management_service_version.object_length - device_manage_payload_data_len;
 
                if(remain_len !=0)
                {
                    if(remain_len >= UIMAGE_VERIFY_FIX_SIZE)
                        RLE_save_temp_buffer_len = rev_temp_buffer_len;
                    else
                        RLE_save_temp_buffer_len = rev_temp_buffer_len - (UIMAGE_VERIFY_FIX_SIZE - (device_management_service_version.object_length - device_manage_payload_data_len));
                }
                else
                {
                    if(rev_temp_buffer_len >= UIMAGE_VERIFY_FIX_SIZE)
                        RLE_save_temp_buffer_len = rev_temp_buffer_len - UIMAGE_VERIFY_FIX_SIZE;
                    else
                        RLE_save_temp_buffer_len = 0;
                }                         
 			#else
                if (rev_temp_buffer_len > DEVICE_MANAGE_PAYLOAD_SIZE)
                {
                    memcpy(&device_manage_payload[offset], pBuf, (DEVICE_MANAGE_PAYLOAD_SIZE - offset));//remain, Intercept and complete

                    #if 0
                    sm_log(SM_LOG_DEBUG, "device_manage_payload(%ld): offset(%ld): \r\n", device_manage_payload_data_len, offset);
                    for(uint16_t i=0; i<128; i++)
                    {
                        sm_log(SM_LOG_DEBUG, " %02x", device_manage_payload[i]);
                    }
                    sm_log(SM_LOG_DEBUG, "\n");
                    #endif

                    uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);
                    writeAddr_offset += UIMAGE_WRITE_SECTOR_SIZE;

                    rev_temp_buffer_len = rev_temp_buffer_len - DEVICE_MANAGE_PAYLOAD_SIZE;//剩下这些还需保存起来

                    memset(device_manage_payload, 0, sizeof(device_manage_payload));
                    memcpy(&device_manage_payload[0], pBuf[DEVICE_MANAGE_PAYLOAD_SIZE-offset], rev_temp_buffer_len);//remain, Intercept and complete

                    offset = 0; //相对4K的偏移量
                    offset += rev_temp_buffer_len;
                }
                else
                {
                    memcpy(&device_manage_payload[offset], pBuf, len);
                    offset += len;
                    if (rev_temp_buffer_len == DEVICE_MANAGE_PAYLOAD_SIZE)
                    {
                        #if 0
                        sm_log(SM_LOG_DEBUG, "device_manage_payload(%ld): offset(%ld): \r\n", device_manage_payload_data_len, offset);
                        for(uint16_t i=0; i<128; i++)
                        {
                            sm_log(SM_LOG_DEBUG, " %02x", device_manage_payload[i]);
                        }
                        sm_log(SM_LOG_DEBUG, "\n");
                        #endif

                        uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);
                        writeAddr_offset += UIMAGE_WRITE_SECTOR_SIZE;
                        
                        offset = 0; //相对4K的偏移量
                        rev_temp_buffer_len = 0;
                        memset(device_manage_payload, 0, sizeof(device_manage_payload));
                    }
                }
 			#endif
            }
                break;
            default:
                break;
        }
        #if (RLE_SUPPORT == 1)
        uint32_t remain_len = device_management_service_version.object_length - device_manage_payload_data_len;
        if(remain_len > 0 || ((remain_len==0) && (rev_temp_buffer_len > UIMAGE_VERIFY_FIX_SIZE)))//=len
        {
            if (device_manage_payload_data_remain_count != 0)
            {
                for(uint8_t z=0; z<device_manage_payload_data_remain_count; z++)
                {
                    device_manage_payload[offset++] = pBuf[0];
                    if(offset >= UIMAGE_WRITE_SECTOR_SIZE)
                    {
                        uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);
                        memset(device_manage_payload, 0, sizeof(device_manage_payload));
                        writeAddr_offset +=UIMAGE_WRITE_SECTOR_SIZE;
                        offset = 0;
                    }
                }
                device_manage_payload_data_remain_count =0;
            }

            for (uint16_t i=RLE_buf_offset; i<RLE_save_temp_buffer_len; i+=2)
            {
                if(i == RLE_save_temp_buffer_len-1)
                {
                    device_manage_payload_data_remain_count = pBuf[i];
                    break;                    
                }
                for(uint8_t j=0; j<pBuf[i]; j++)/** 根据第一个字节作为长度，填充第二个数据到device_manage_payload， 填满1024Bytes后，写入外部flash*/
                {
                    device_manage_payload[offset++] = pBuf[i+1];
                    if(offset >= UIMAGE_WRITE_SECTOR_SIZE)
                    {
                        uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);
                        memset(device_manage_payload, 0, sizeof(device_manage_payload));
                        writeAddr_offset +=UIMAGE_WRITE_SECTOR_SIZE;
                        offset = 0;
                        RLE_buf_offset = 0;
                        device_manage_paload_data_remain_offset = 0;
                    }                  
                }                          
            }
            device_manage_paload_data_remain_offset = offset;//上一次剩余的偏移量
        }
        
        if((remain_len > 0) && (remain_len < UIMAGE_VERIFY_FIX_SIZE))//直接写入剩余1K数据
        {
            uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);
            memset(device_manage_payload, 0, sizeof(device_manage_payload));
            sm_log(SM_LOG_DEBUG, "Save verify RLE_save_temp_buffer_len %02X \r\n",RLE_save_temp_buffer_len);
            for(uint8_t i=0;i<UIMAGE_VERIFY_FIX_SIZE;i++)
            {
                device_manage_payload[i] = pBuf[RLE_save_temp_buffer_len++];   
            }
            device_manage_payload_data_remain_count = 0;
        }

        if(remain_len == 0)
        {
            if(rev_temp_buffer_len <  UIMAGE_VERIFY_FIX_SIZE)//len
            {
                memcpy(&device_manage_payload[UIMAGE_VERIFY_FIX_SIZE - rev_temp_buffer_len], pBuf, rev_temp_buffer_len);//如果倒数第二包不足UIMAGE_VERIFY_FIX_SIZE，补全最后一包数据
            }
            else
            {
                uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);
                memset(device_manage_payload, 0, sizeof(device_manage_payload));
                for(uint8_t i=0;i<UIMAGE_VERIFY_FIX_SIZE;i++)
                    device_manage_payload[i] = pBuf[RLE_save_temp_buffer_len++];//copy
            }
            print_array(&device_manage_payload[0], UIMAGE_VERIFY_FIX_SIZE); 
            memcpy(&uimage_challenge[0], device_manage_payload, UIMAGE_CHALLENGE_SIZE);
            memcpy(&uimage_challenge_signature[0], &device_manage_payload[UIMAGE_CHALLENGE_SIZE], UIMAGE_CHALLENGE_SIGN_SIZE);
            memcpy(&uimage_protobuf_payload_signature[0], &device_manage_payload[UIMAGE_CHALLENGE_SIZE+UIMAGE_CHALLENGE_SIGN_SIZE], UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE);   
            rev_temp_buffer_len = 0;
            offset = 0; //相对4K的偏移量
            writeAddr_offset = 0;
        }
        #else
        /******************Extract uimage_challenge &&uimage_challenge_signature &&uimage_protobuf_payload_signature***************************************************************** */
        if (device_manage_payload_data_len == device_management_service_version.object_length)//[protobuf_payload(n)]+[challenge(32)]+[challenge_signature(64)]+[protobuf)payload_hash_signature(64)]
        {
            uimage_record_payload_upgrade(uImageSavedFlashADDR+writeAddr_offset, device_manage_payload, UIMAGE_WRITE_SECTOR_SIZE);

            //UI_IMAGE_CHALLENGE
            if (UIMAGE_VERIFY_FIX_SIZE < rev_temp_buffer_len)//rev_temp_buffer_len ，IF device_manage_payload in {160<offset<1024} area
            {
                uint16_t offset_t = 0;

                offset_t = UIMAGE_CHALLENGE_SIZE + UIMAGE_CHALLENGE_SIGN_SIZE + UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE;
                memcpy(&uimage_challenge[0], &device_manage_payload[rev_temp_buffer_len - offset_t], UIMAGE_CHALLENGE_SIZE);
                // sm_log(SM_LOG_INFO, "\n uimage_challenge(%d): \r\n",sizeof(uimage_challenge));
                // print_array(&uimage_challenge[0], sizeof(uimage_challenge));

                offset_t = UIMAGE_CHALLENGE_SIGN_SIZE + UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE;
                memcpy(&uimage_challenge_signature[0], &device_manage_payload[rev_temp_buffer_len - offset_t], UIMAGE_CHALLENGE_SIGN_SIZE);
                // sm_log(SM_LOG_INFO, "\n uimage_challenge_signature(%d): \r\n",sizeof(uimage_challenge_signature));
                // print_array(&uimage_challenge_signature[0], sizeof(uimage_challenge_signature));

                offset_t = UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE;
                memcpy(&uimage_protobuf_payload_signature[0], &device_manage_payload[rev_temp_buffer_len - offset_t], UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE);
                // sm_log(SM_LOG_INFO, "\n uimage_protobuf_payload_signature(%d): \r\n",sizeof(uimage_protobuf_payload_signature));
                // print_array(&uimage_protobuf_payload_signature[0], sizeof(uimage_protobuf_payload_signature));
            }
            else
            {
                //from EXT-Flash reading 
                uint8_t read_offset = UIMAGE_VERIFY_FIX_SIZE - len;
                uimage_record_payload_read((uImageSavedFlashADDR+writeAddr_offset-read_offset), read_offset, device_manage_payload);
                memcpy(&device_manage_payload[read_offset], pBuf, len);//如果刚好1K被写完到flash, 最后第一包不足 UIMAGE_VERIFY_FIX_SIZE，回读补全到UIMAGE_VERIFY_FIX_SIZE大小

                uint16_t offset_t = 0;
                memcpy(&uimage_challenge[0], &device_manage_payload[offset_t], UIMAGE_CHALLENGE_SIZE);
                offset_t = UIMAGE_CHALLENGE_SIZE;
                memcpy(&uimage_challenge_signature[0], &device_manage_payload[offset_t], UIMAGE_CHALLENGE_SIGN_SIZE);
                offset_t = UIMAGE_CHALLENGE_SIZE + UIMAGE_CHALLENGE_SIGN_SIZE;
                memcpy(&uimage_protobuf_payload_signature[0], &device_manage_payload[offset_t], UIMAGE_PROTOBUG_PAYLOAD_SIGN_SIZE);
            }

            rev_temp_buffer_len = 0;
            offset = 0; //相对4K的偏移量
            writeAddr_offset = 0;
        }
 		#endif
        cy_rtos_delay_milliseconds(2);
    }
    
    return status; 
}

/*
 * Function Name: app_bt_char_device_management_service_uimage_data_integrity_validation
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param 
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_management_service_uimage_data_integrity_validation(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint8_t hash_value[32]={0};
    mbedtls_sha256_finish_ret( &sha256_ctx, hash_value );
    mbedtls_sha256_free( &sha256_ctx ); 

    sm_log(SM_LOG_DEBUG, "\nCurrent rev packet:%d  payload total len:%d sha256 caculate value:\r\n", device_manage_payload_package_cnt, (device_manage_payload_data_len-160) );

    #if 1
    for(uint16_t i=0; i<sizeof(hash_value); i++)
    {
        sm_log(SM_LOG_DEBUG, "%02x", hash_value[i]);
    }
    sm_log(SM_LOG_DEBUG, "\r\n");
    #endif

    if (1 == bt_adapter_uECC_sha256_sign_verify(hash_value, sizeof(hash_value), myName_public_key, uimage_protobuf_payload_signature))
    {
        sm_log(SM_LOG_INFO, "\n uimage_protobuf_payload_signature verify successfully!\r\n");
    }
    else
    {
        status = WICED_BT_GATT_ERROR;

        uimage_record_erase_sector(uImageSavedFlashADDR, 4096);//擦除 4 K, 主要把nanopb buf擦除掉
        sm_log(SM_LOG_INFO, "\n uImageSavedFlashADDR: 0X%08x, uimage_protobuf_payload_signature verify failure!\r\n", uImageSavedFlashADDR);
    }

    return status;
}

/*
 * Function Name: app_bt_char_device_management_service_challenge_write_handler
 * 
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler.
 *
 * @param handle   Pointer to characteristic handle
 * @param length   Pointer to characteristic date length
 * @param *p_data   Pointer to characteristic dat buf
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t app_bt_char_device_management_service_challenge_write_handler(uint16_t handle, uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s handle: %d len:%d p_Buf:\r\n", __FUNCTION__, handle, len);

    #if 1
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif
    
    if (len == 2)//msg len verify.
    {
        device_management_service_challenge_char_t challenge_char;

        challenge_char.payload_code[0] = pBuf[0];
        challenge_char.payload_code[1] = pBuf[1];
        uint8_t length = 30;
        bt_adapter_random_data_get((uint8_t *)&challenge_char.random_data[0], length);

        #if 1
        sm_log(SM_LOG_DEBUG, " challenge_char.random_data: \n");
        for(uint8_t i=0; i<length; i++)
        {
            sm_log(SM_LOG_DEBUG, " %02x", challenge_char.random_data[i]);
        }
        sm_log(SM_LOG_DEBUG, "\n");
        #endif

        status = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                                        handle,
                                                        sizeof(challenge_char),
                                                        (uint8_t *)&challenge_char,NULL);
        if (status == WICED_BT_GATT_SUCCESS)
        {
            sm_log(SM_LOG_DEBUG, "NTF Sending Successful\n");
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "NTF Sending Failure, Please update MTU.\n");
        }
    }
    
    return status; 
}











/* [] END OF FILE */
