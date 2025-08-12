/**
  ******************************************************************************
  * @file    task_bt_service.c
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
  * 2024-08-12      V0.04      vincent.he@metextech.com     div of protocol/char
  * 
  ******************************************************************************
**/

#include "task_bt_service.h"


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

#include "app_bt_utils.h"
#include "app_bt_char.h"
#include "app_bt_char_adapter.h"
#include "app_bt_bonding.h"
#include "kv_store_flash.h"


/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

#include "ota_context.h"
#if (OTA_SERVICE_SUPPORT == 1)
/* OTA related header files */
#include "cy_ota_api.h"
#endif

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

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define BT_ADV_START (1)
#define APP_BT_CHAR_NTF_INTERVAL (40) //unit: 5ms

/* 定义事件位 */
#define EVENT_NO_GATT_CONGESTION (1<<0)


#ifdef GATT_CONGESTED_WORKAROUND
#define GATT_CONGESTED_MAX_NUM		4 /*pre: 6*/
cy_semaphore_t	gatt_congested;
volatile uint16_t bGattCongestedTimeout;
#endif

/* Private Consts ------------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/
/* To check if the device has entered pairing mode to connect and bond with a new device */
bool pairing_mode = FALSE;

/* Private variables ---------------------------------------------------------*/
static uint8_t bBatPowerNeedNofity = 0;
static uint8_t bDeviceLockNeedNofity = 0;
static uint8_t bSessionStaNeedNofity = 0;
static uint8_t bLastErrorNeedNofity = 0;

/* This is the index for the link keys, cccd and privacy mode of the host we are 
 * currently bonded to */
static volatile uint8_t  bondindex = BOND_INDEX_MAX;//before is 0, modified to BOND_INDEX_MAX - 20250106.101355

/* 创建一个事件标志组 */
static EventGroupHandle_t bt_event_group;
/* Exported functions --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Typdef for function used to free allocated buffer to stack
 */
typedef void (*pfn_free_buffer_t)(uint8_t *);

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/* GATT Event Callback Functions */

static wiced_bt_gatt_status_t app_bt_gatt_req_read_handler          (uint16_t conn_id,
                                                                    wiced_bt_gatt_opcode_t opcode,
                                                                    wiced_bt_gatt_read_t *p_read_req,
                                                                    uint16_t len_requested, 
                                                                    uint16_t *p_error_handle);
static wiced_bt_gatt_status_t app_bt_gatt_req_read_multi_handler    (uint16_t conn_id,
                                                                    wiced_bt_gatt_opcode_t opcode,
                                                                    wiced_bt_gatt_read_multiple_req_t *p_read_req,
                                                                    uint16_t len_requested, 
                                                                    uint16_t *p_error_handle);
static wiced_bt_gatt_status_t app_bt_gatt_req_read_by_type_handler  (uint16_t conn_id,
                                                                    wiced_bt_gatt_opcode_t opcode,
                                                                    wiced_bt_gatt_read_by_type_t *p_read_req,
                                                                    uint16_t len_requested, 
                                                                    uint16_t *p_error_handle);
static wiced_bt_gatt_status_t app_bt_connect_event_handler          (wiced_bt_gatt_connection_status_t *p_conn_status);
static wiced_bt_gatt_status_t app_bt_server_event_handler           (wiced_bt_gatt_event_data_t *p_data, 
                                                                    uint16_t *p_error_handle);
static wiced_bt_gatt_status_t app_bt_gatt_event_callback            (wiced_bt_gatt_evt_t event,
                                                                    wiced_bt_gatt_event_data_t *p_event_data);
static wiced_bt_gatt_status_t app_bt_set_value                      (uint16_t attr_handle, 
                                                                    uint8_t *p_val, 
                                                                    uint16_t len);
/* Callback function for Bluetooth stack management type events */
static wiced_bt_dev_status_t  app_bt_management_callback            (wiced_bt_management_evt_t event,
                                                                    wiced_bt_management_evt_data_t *p_event_data);
static wiced_bt_gatt_status_t app_bt_write_handler                  (wiced_bt_gatt_event_data_t *p_data, 
                                                                    uint16_t *p_error_handle);

 
static void                   app_bt_init                           (void);
 
static wiced_bt_gatt_status_t app_bt_client_char_config_disable(void);

/**
 * @brief rate of change of battery level
 */
#define BATTERY_LEVEL_CHANGE (2)

#ifdef GATT_CONGESTED_WORKAROUND
static void hci_trace_cback(wiced_bt_hci_trace_type_t type,
                     uint16_t length, uint8_t* p_data);
#endif

/**
 * @brief This enumeration combines the advertising, connection states from two
 *        different callbacks to maintain the status in a single state variable
 */
typedef enum
{
    APP_BT_ADV_OFF_CONN_OFF = 0, /* Idle状态，非广播及连接态 */
    APP_BT_PAIRING,              /* 配对 */
    APP_BT_ADV_ON_CONN_OFF,      /* 定向广播 */
    APP_BT_ADV_OFF_CONN_ON       /* 连接状态 */
} app_bt_adv_conn_mode_t;

/**
 * @brief FreeRTOS variable to store handle of task created to update and send dummy
   values of temperature
 */
TaskHandle_t task_bt_handle;

 
/**
 * @brief variable to track connection and advertising state
 */
uint8_t app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_OFF;


/**
 * Function Name:
 * app_bt_adv_start
 *
 * Function Description:
 * @brief  This Function start advertisement
 * * @param 
 *   opt == 1, unbonding and must open adv
 *   opt == 0, unbonding and can not open adv, such as factory reset
 *
 */
void app_bt_adv_start(void)
{
	wiced_result_t result;
    wiced_bt_device_address_t bda = {0};

    static uint8_t device_name[16] = {0};
    static uint8_t length =0;
    bt_adapter_device_information_get_adv_name(device_name, (uint16_t*)&length);

    cy_bt_scan_rsp_packet_data[0].len = length;
    if (cy_bt_scan_rsp_packet_data[0].len > 16)//retrict max len
        cy_bt_scan_rsp_packet_data[0].len = 16;
    cy_bt_scan_rsp_packet_data[0].p_data = device_name;

    /* Initialize the application */
    uint64_t addr =  Cy_SysLib_GetUniqueId();
    /*
    * (Random Device Address), MAC ADDR Format: C4:65:8E:XX:XX:XX, Satisfied Address [47:46] MSB(0b11).
    */
    memcpy(cy_bt_device_address, (uint8_t *)&addr, 6);
    wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address, BLE_ADDR_RANDOM);// BLE_ADDR_PUBLIC BLE_ADDR_RANDOM
    /* Bluetooth is enabled */
    wiced_bt_dev_read_local_addr(bda);
    
    #if 0
    sm_log(BT_SERVICE_LOG_INFO, "0: %02x\n", cy_bt_scan_rsp_packet_data[0].advert_type);
    sm_log(BT_SERVICE_LOG_INFO, "0: %02x\n", cy_bt_scan_rsp_packet_data[0].len);
    for (uint8_t i=0; i<cy_bt_scan_rsp_packet_data[0].len; i++)
        sm_log(BT_SERVICE_LOG_INFO, " %02x", cy_bt_scan_rsp_packet_data[0].p_data[i]);
    sm_log(BT_SERVICE_LOG_INFO, "\n");
    sm_log(BT_SERVICE_LOG_INFO, "1: %02x\n", cy_bt_scan_rsp_packet_data[1].advert_type);
    sm_log(BT_SERVICE_LOG_INFO, "1: %02x\n", cy_bt_scan_rsp_packet_data[1].len);
    for (uint8_t i=0; i<cy_bt_scan_rsp_packet_data[1].len; i++)
        sm_log(BT_SERVICE_LOG_INFO, " %02x", cy_bt_scan_rsp_packet_data[1].p_data[i]);
    sm_log(BT_SERVICE_LOG_INFO, "\n");
    #endif

    uint8_t scan_rsp_temp[10] = { 0xff, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff, 0xff};//whether need global?
    memcpy((uint8_t *)&scan_rsp_temp[2], (uint8_t *)&bda, 6);
    data_array_invert_endianness(scan_rsp_temp, sizeof(scan_rsp_temp));
    scan_rsp_temp[8] = get_device_color();//device color, Color: Burgundy[1] Blue[2] Black[3] Rose[4] Green[5] Purple[6] Gold[7]
    cy_bt_scan_rsp_packet_data[1].p_data = scan_rsp_temp;

    sm_log(SM_LOG_NOTICE, "Local Bluetooth Address: ");
    print_bd_address(bda);

    wiced_bt_ble_set_raw_scan_response_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                        cy_bt_scan_rsp_packet_data); 

    cy_bt_cfg_adv_settings.high_duty_duration = CY_BT_HIGH_DUTY_ADV_DURATION; //bug1984533

	// if(app_bt_is_existed_bonded_devices())    
    if(CY_RSLT_SUCCESS == app_bt_restore_bond_data())
	{  
        if (FALSE == pairing_mode)//非配对模式下才add bonding addr to db
        {
            sm_log(SM_LOG_DEBUG, "==existed bond device, restore resolving list, and enable adv filter\n");
            sm_log(SM_LOG_DEBUG, "Keys found in NVRAM, add them to Addr Res DB\n");
            sm_log(SM_LOG_DEBUG, "pairing_mode: %d\n", pairing_mode);
            #if 1
            wiced_bt_ble_address_resolution_list_clear_and_disable();
            app_bt_add_devices_to_address_resolution_db();
            /* only allow pair-peer to connection */
            wiced_btm_ble_update_advertisement_filter_policy(BTM_BLE_ADV_POLICY_FILTER_CONN_FILTER_SCAN);// BTM_BLE_ADV_POLICY_FILTER_CONN_ACCEPT_SCAN
            #else
            sm_log(SM_LOG_DEBUG, "existed bond device, but need to clear/disable resolving list for to discover ADV.\n");
            wiced_bt_ble_address_resolution_list_clear_and_disable();
            #endif
        }
        
        result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
                                                    0, NULL);
        /* Failed to start advertisement. Stop program execution */
        if (WICED_BT_SUCCESS != result)
        {
            sm_log(SM_LOG_NOTICE,"Start advertisement failed: %d\n", result);
            CY_ASSERT(0);
        }
	}
    else
    {
        if (TRUE == pairing_mode)//pairing mode, not bonding info exists, start advertisement
        {
            cy_bt_cfg_adv_settings.high_duty_duration = CY_BT_HIGH_DUTY_ADV_DURATION_PAIRING; //bug1984533

            result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
                                                        0, NULL);
            /* Failed to start advertisement. Stop program execution */
            if (WICED_BT_SUCCESS != result)
            {
                sm_log(SM_LOG_NOTICE,"Start advertisement failed: %d\n", result);
                CY_ASSERT(0);
            }
        }
    }
}

/**
 * Function Name:
 * app_bt_adv_stop
 *
 * Function Description:
 * @brief  This Function start advertisement
 *
 */
void app_bt_adv_stop(void)
{
	wiced_result_t result;

    pairing_mode = FALSE;

    //a. 如果连接成功后，直接停止广播
    //b. 如果没有配对信息，也直接停止广播
    if ((app_server_context.bt_conn_id) || (app_bt_restore_bond_data()))
    // if (app_server_context.bt_conn_id)
    {
        result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF,     //stop adv
                                                0,
                                                NULL);
        if (result != WICED_BT_SUCCESS)
            sm_log(SM_LOG_INFO,"Stop advertisement failed: %d\n", result);
        return;
    }
    else
    {
        /* Update the adv/conn state */
        app_bt_adv_conn_state = APP_BT_ADV_ON_CONN_OFF;
    }
    
    app_bt_add_devices_to_address_resolution_db();
}

/**
 * Function Name:
 * app_bt_enter_pairing_mode
 *
 * Function Description:
 * @brief  This Function start advertisement
 *
 */
bool app_bt_enter_pairing_mode(void)
{
    wiced_result_t result;
    wiced_bt_device_address_t bda = {0};

    // //如果建立了链接 先断开链接，然后再关闭广播，否则直接关闭广播
    // if(app_server_context.bt_conn_id)
    // {
    //     wiced_bt_gatt_disconnect(app_server_context.bt_conn_id);
    // } 

    static uint8_t device_name[16] = {0};
    static uint8_t length =0;
    bt_adapter_device_information_get_adv_name(device_name, (uint16_t*)&length);

    cy_bt_scan_rsp_packet_data[0].len = length;
    if (cy_bt_scan_rsp_packet_data[0].len > 16)//retrict max len
        cy_bt_scan_rsp_packet_data[0].len = 16;
    cy_bt_scan_rsp_packet_data[0].p_data = device_name;

    /* Initialize the application */
    uint64_t addr =  Cy_SysLib_GetUniqueId();
    /*
    * (Random Device Address), MAC ADDR Format: C4:65:8E:XX:XX:XX, Satisfied Address [47:46] MSB(0b11).
    */
    memcpy(cy_bt_device_address, (uint8_t *)&addr, 6);
    wiced_bt_set_local_bdaddr((uint8_t *)cy_bt_device_address, BLE_ADDR_RANDOM);// BLE_ADDR_PUBLIC BLE_ADDR_RANDOM
    /* Bluetooth is enabled */
    wiced_bt_dev_read_local_addr(bda);
    
    #if 0
    sm_log(BT_SERVICE_LOG_INFO, "0: %02x\n", cy_bt_scan_rsp_packet_data[0].advert_type);
    sm_log(BT_SERVICE_LOG_INFO, "0: %02x\n", cy_bt_scan_rsp_packet_data[0].len);
    for (uint8_t i=0; i<cy_bt_scan_rsp_packet_data[0].len; i++)
        sm_log(BT_SERVICE_LOG_INFO, " %02x", cy_bt_scan_rsp_packet_data[0].p_data[i]);
    sm_log(BT_SERVICE_LOG_INFO, "\n");
    sm_log(BT_SERVICE_LOG_INFO, "1: %02x\n", cy_bt_scan_rsp_packet_data[1].advert_type);
    sm_log(BT_SERVICE_LOG_INFO, "1: %02x\n", cy_bt_scan_rsp_packet_data[1].len);
    for (uint8_t i=0; i<cy_bt_scan_rsp_packet_data[1].len; i++)
        sm_log(BT_SERVICE_LOG_INFO, " %02x", cy_bt_scan_rsp_packet_data[1].p_data[i]);
    sm_log(BT_SERVICE_LOG_INFO, "\n");
    #endif

    uint8_t scan_rsp_temp[10] = { 0xff, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff, 0xff};//whether need global?
    memcpy((uint8_t *)&scan_rsp_temp[2], (uint8_t *)&bda, 6);
    data_array_invert_endianness(scan_rsp_temp, sizeof(scan_rsp_temp));
    scan_rsp_temp[8] = get_device_color();//device color, Color: Burgundy[1] Blue[2] Black[3] Rose[4] Green[5] Purple[6] Gold[7]
    cy_bt_scan_rsp_packet_data[1].p_data = scan_rsp_temp;

    sm_log(SM_LOG_NOTICE, "Local Bluetooth Address: ");
    print_bd_address(bda);

    wiced_bt_ble_set_raw_scan_response_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                        cy_bt_scan_rsp_packet_data); 

    /**************************************************delete bond info*********************************************************/
    result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF,     //stop adv
                                            0,
                                            NULL);
    if (result != WICED_BT_SUCCESS)
    {
        sm_log(SM_LOG_INFO,"Stop advertisement failed: %d\n", result);
        return result;
    }
    
    /**************************************************irk clear*********************************************************/
    result = wiced_bt_ble_address_resolution_list_clear_and_disable();
    if(WICED_BT_SUCCESS != result)
    {
        sm_log(SM_LOG_INFO,"Failed to clear address resolution list \n");
        return result;
    }
    
    /* allow all master to scanning and connection */
    wiced_btm_ble_update_advertisement_filter_policy(BTM_BLE_ADV_POLICY_ACCEPT_CONN_AND_SCAN);

    /***************************************************start adv********************************************************/
    cy_bt_cfg_adv_settings.high_duty_duration = CY_BT_HIGH_DUTY_ADV_DURATION_PAIRING; //bug1984533
    result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
                                                0, NULL);
    /* Failed to start advertisement. Stop program execution */
    if (WICED_BT_SUCCESS != result)
    {
        sm_log(SM_LOG_INFO,"Start advertisement failed: %d\n", result);
        return result;
        CY_ASSERT(0);
    }

    sm_log(SM_LOG_INFO,"Entering Pairing Mode: Connect, Pair and Bond with a new peer device...\n");
    pairing_mode = TRUE;

    /* Update the adv/conn state */
    app_bt_adv_conn_state = APP_BT_PAIRING;

    return result;
}

/**
 * Function Name:
 * app_bt_alloc_buffer
 *
 * Function Description:
 * @brief  This Function allocates the buffer of requested length
 *
 * @param len            Length of the buffer
 *
 * @return uint8_t*      pointer to allocated buffer
 */
static uint8_t *app_bt_alloc_buffer(uint16_t len)
{
    uint8_t *p = (uint8_t *)malloc(len);
    sm_log(SM_LOG_DEBUG, "%s() len %d alloc %p \r\n", __FUNCTION__,
               len, p);
    return p;
}

/**
 * Function Name:
 * app_bt_free_buffer
 *
 * Function Description:
 * @brief  This Function frees the buffer requested
 *
 * @param p_data         pointer to the buffer to be freed
 *
 * @return void
 */
static void app_bt_free_buffer(uint8_t *p_data)
{
    if (p_data != NULL)
    {
        sm_log(SM_LOG_DEBUG, "%s()        free:%p \r\n",
                   __FUNCTION__, p_data);
        free(p_data);
    }
}

TaskHandle_t* get_task_bt_handle(void)
{
    return &task_bt_handle;
}

void bt_init(void)
{
    wiced_result_t  result;
    /* Initialising the HCI UART for Host contol */
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    /* set default values for battery server context */
    app_bt_initialize_default_values();

    /*Initialize the block device used by kv-store for performing
     * read/write operations to the flash*/
    app_kvstore_bd_config(&block_device);

    /* Register call back and configuration with stack */
    result = wiced_bt_stack_init(app_bt_management_callback, &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */
    if (WICED_BT_SUCCESS != result)
    {
        sm_log(SM_LOG_ERR,  "Bluetooth Stack Initialization failed!! \r\n");
        CY_ASSERT(0);
    }
}

/*
 * Function Name: app_bt_service_char_findmy_device_is_found
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
bool app_bt_service_char_findmy_device_is_found(void)
{
    bool status = false;
    uint16_t rslt = WICED_BT_GATT_SUCCESS;

    status = bt_adapter_findmy_running_state_get();
    if (status)
    {
        bt_adapter_findmy_running_state_restore();

        bt_adapter_findmy_device_get(app_session_service_ext_findmy_device_char);
        app_session_service_ext_findmy_device_char->device_alert = 0x00;

        uint8_t pBuf_t[2]={0};
        memcpy(pBuf_t, (uint8_t *)app_session_service_ext_findmy_device_char, 2);//close findmy device
        bt_adapter_findmy_device_set(pBuf_t, sizeof(pBuf_t));

        if (app_server_context.bt_conn_id && (app_session_service_findmy_device_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
        {
            rslt = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                HDLC_SESSION_SERVICE_FINDMY_DEVICE_VALUE,
                                app_session_service_ext_findmy_device_char_len,
                                (uint8_t *)app_session_service_ext_findmy_device_char,NULL);
            if (rslt == WICED_BT_GATT_SUCCESS)
            {
                sm_log(BT_SERVICE_LOG_INFO, "Sending Successful\n");
            }
        }
        else
        {
            sm_log(BT_SERVICE_LOG_INFO, "Notification Disable or Disconnected.\n");
        }
    }

    return status;
}

/*
 * Function Name: app_bt_service_char_ntf_monitor
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
void app_bt_service_char_ntf_monitor(void)
{
    
    ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
    ptIoDev msTickDev = io_dev_get_dev(DEV_MS_TICK);
    static uint32_t bt_service_1SecondPeriod = 0;
    uint16_t rslt = WICED_BT_GATT_SUCCESS;

#ifdef GATT_CONGESTED_WORKAROUND
    if (app_server_context.bt_conn_id) //connected
    {
        if (bCongestedRebootCnt >= CONGESTED_REBOOT_CNT_MAX)//2ms*2500 == 5000ms == 5S
        {
            bCongestedRebootCnt = 0;
            //setting Silent reset
            if (false == BleCongested_Read()) {
                BleCongested_Write(true);
            }
            sm_log(BT_SERVICE_LOG_INFO, "Setting Silent Reset Flag Done\r\n"); 
        }
    }
#endif

    if (true == bt_adapter_battery_power_remaining_change_happened(app_session_service_ext_battery_power_remaining_char))
    {
        bBatPowerNeedNofity = 1;
    }// battery remaining ntf
    if ((app_server_context.bt_conn_id && (app_session_service_battery_power_remaining_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))&&(bBatPowerNeedNofity))
    {
        // sm_log(BT_SERVICE_LOG_INFO, "battery power remaining handle NTF: ");  //commented by bug1986437
        // #ifdef GATT_CONGESTED_WORKAROUND
        //     cy_rtos_semaphore_get(&gatt_congested, 10000/*CY_RTOS_NEVER_TIMEOUT*/);
        // #endif
        rslt = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_SESSION_SERVICE_BATTERY_POWER_REMAINING_VALUE,
                            app_session_service_ext_battery_power_remaining_char_len,
                            (uint8_t *)app_session_service_ext_battery_power_remaining_char,NULL);
        if (rslt == WICED_BT_GATT_SUCCESS)
        {
            bBatPowerNeedNofity = 0;
            sm_log(BT_SERVICE_LOG_INFO, "batt_pwr NTF Succ\r\n");  //commented by bug1986437
        }
        else if(rslt == WICED_BT_GATT_CONGESTED)
        {
#ifdef GATT_CONGESTED_WORKAROUND
            bCongestedRebootCnt++;
            if (bCongestedRebootCnt%500 == 0)
            {
                sm_log(BT_SERVICE_LOG_INFO, "batt_pwr NTF Fail(0x%04X)\r\n", rslt);
            }
#endif
        }
    }
    // else
    // {
    //     sm_log(BT_SERVICE_LOG_INFO, "Notification Disable or Disconnected.\n");
    // }

    if (true == bt_adapter_device_lock_status_change_happened(app_session_service_ext_device_lock_char))
    {
        bDeviceLockNeedNofity = 1;
    }//device lock ntf
    if ((app_server_context.bt_conn_id && (app_session_service_device_lock_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))&&(bDeviceLockNeedNofity))
    {
        // sm_log(BT_SERVICE_LOG_INFO, "session status handle NTF: ");
        // #ifdef GATT_CONGESTED_WORKAROUND
        //     cy_rtos_semaphore_get(&gatt_congested, 10000/*CY_RTOS_NEVER_TIMEOUT*/);
        // #endif
        rslt = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_SESSION_SERVICE_DEVICE_LOCK_VALUE,
                            app_session_service_ext_device_lock_char_len,
                            (uint8_t *)app_session_service_ext_device_lock_char,NULL);
        if (rslt == WICED_BT_GATT_SUCCESS)
        {
            bDeviceLockNeedNofity = 0;
            sm_log(BT_SERVICE_LOG_INFO, "device_lock NTF Succ\r\n");
        }
        else if(rslt == WICED_BT_GATT_CONGESTED)
        {
#ifdef GATT_CONGESTED_WORKAROUND
            bCongestedRebootCnt++;
            if (bCongestedRebootCnt%500 == 0)
            {
                sm_log(BT_SERVICE_LOG_INFO, "device_lock NTF Fail(0x%04X)\r\n", rslt);
            }
#endif
        }
    }
    // else
    // {
    //     sm_log(BT_SERVICE_LOG_INFO, "Notification Disable or Disconnected.\n");
    // }

    if (true == bt_adapter_session_status_change_happened(app_session_service_ext_status_char))
    {
        bSessionStaNeedNofity = 1;
    }// session status ntf
    if ((app_server_context.bt_conn_id && (app_session_service_session_status_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))&&(bSessionStaNeedNofity))
    {
        // sm_log(BT_SERVICE_LOG_INFO, "session status handle NTF: ");
        // #ifdef GATT_CONGESTED_WORKAROUND
        //     cy_rtos_semaphore_get(&gatt_congested, 10000/*CY_RTOS_NEVER_TIMEOUT*/);
        // #endif
        rslt = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_SESSION_SERVICE_SESSION_STATUS_VALUE,
                            app_session_service_ext_status_char_len,
                            (uint8_t *)app_session_service_ext_status_char,NULL);
        if (rslt == WICED_BT_GATT_SUCCESS)
        {
            bSessionStaNeedNofity = 0;
            sm_log(BT_SERVICE_LOG_INFO, "session_sta NTF Succ\r\n");
        }
        else if(rslt == WICED_BT_GATT_CONGESTED)
        {
#ifdef GATT_CONGESTED_WORKAROUND
            bCongestedRebootCnt++;
            if (bCongestedRebootCnt%500 == 0)
            {
                sm_log(BT_SERVICE_LOG_INFO, "session_sta NTF Fail(0x%04X)\r\n", rslt);
            }
#endif
        }
    }
    // else
    // {
    //     sm_log(BT_SERVICE_LOG_INFO, "Notification Disable or Disconnected.\n");
    // }

    if (true == bt_adapter_last_error_change_happened(app_debug_service_ext_last_error_char))//error_code counter current_state
    {
        bLastErrorNeedNofity = 1;
        //swap to big-endian and transfer 
        u32_endian_swap(&app_debug_service_ext_last_error_char[0].timestamp);
        u16_endian_swap(&app_debug_service_ext_last_error_char[0].error_code);

    }// last error ntf
    if ((app_server_context.bt_conn_id && (app_debug_service_last_error_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))&&(bLastErrorNeedNofity))
    {
        // sm_log(BT_SERVICE_LOG_INFO, "last error handle NTF: ");
        // #ifdef GATT_CONGESTED_WORKAROUND
        //     cy_rtos_semaphore_get(&gatt_congested, 10000/*CY_RTOS_NEVER_TIMEOUT*/);
        // #endif
        rslt = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                            HDLC_DEBUG_SERVICE_LAST_ERROR_VALUE,
                            app_debug_service_ext_last_error_char_len,
                            (uint8_t *)app_debug_service_ext_last_error_char,NULL);
        if (rslt == WICED_BT_GATT_SUCCESS)
        {
            bLastErrorNeedNofity = 0;
            sm_log(BT_SERVICE_LOG_INFO, "last_err NTF Succ\r\n");
        }
        else if(rslt == WICED_BT_GATT_CONGESTED)
        {
#ifdef GATT_CONGESTED_WORKAROUND
            bCongestedRebootCnt++;
            if (bCongestedRebootCnt%500 == 0)
            {
                sm_log(BT_SERVICE_LOG_INFO, "last_err NTF Fail(0x%04X)\r\n", rslt);
            }
#endif
        }
    }
    // else
    // {
    //     sm_log(BT_SERVICE_LOG_INFO, "Notification Disable or Disconnected.\n");
    // }

    //for test
    {
        if ((msTickDev->read( (uint8_t*)&bt_service_1SecondPeriod, 4) -bt_service_1SecondPeriod) > 5000) 
        { 
            bt_service_1SecondPeriod = msTickDev->read( (uint8_t*)&bt_service_1SecondPeriod, 4);

            if (get_system_status() == BT_SERVICE_CHAR_COMM)
            {
                set_system_status(IDLE);
            }
            else if (get_system_status() == IDLE)
            {
                // set_system_status(BT_SERVICE_CHAR_COMM);//need to debug when setting the status to "NTF_COMM".
            }

            #if 0 //hash md5 test
            unsigned char temp_buf[10] = {0,1,2,3,4,5,6,7,8,9};
            unsigned char hash_value[32] = {0};
            if (0 == bt_adapter_sha256_calculate(temp_buf, 0x00000000, 10, 3, hash_value))
            {
                sm_log(BT_SERVICE_LOG_INFO, "bt_adapter_sha256_calculate successful, hash_value: \n");
                print_array(&hash_value[0], 32);
            }
            else
            {
                sm_log(BT_SERVICE_LOG_INFO, "bt_adapter_sha256_calculate failure.\n");
            }

            if (0 == bt_adapter_md5_calculate(temp_buf, 0x00000000, 10, 3, hash_value))
            {
                sm_log(BT_SERVICE_LOG_INFO, "bt_adapter_md5_calculate successful, hash_value: \n");
                print_array(&hash_value[0], 32);
            }
            else
            {
                sm_log(BT_SERVICE_LOG_INFO, "bt_adapter_md5_calculate failure.\n");
            }
            #endif

            #if 0 //ecdsa test
            bt_adapter_ecdsa_sign_verify_test();
            #endif
        }
        return;
    }
}

/*
 Function name:
 task_comm

 Function Description:
 @brief  This task updates dummy battery value every time it is notified
         and sends a notification to the connected peer

 @param  void*: unused

 @return void
 */
void task_bt_service(void *pvParam)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    static volatile uint8_t s_heating_profile_proc_state=0;
    static volatile uint8_t s_session_records_proc_state=0;
    static volatile uint8_t s_event_log_proc_state=0;
    uint8_t heating_profile_num=0;
    uint8_t heating_profile_index=0;
    static volatile uint32_t StateMachineCounter=0;

    bt_event_group = xEventGroupCreate();

    while(true)
    {
        sm_log(BT_SERVICE_LOG_INFO, "%s ,waiting task_bt_handle to notify!\n", __FUNCTION__);
        // clr_task_bt_heartbeat_ticks();
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // set_task_bt_heartbeat_ticks();       //"bt service may be running over 5S of notify" added by zshe.

        for(;;)
        {
            /*******************************************************Heating Profile Reported Data**************************************************************/
            // sm_log(BT_SERVICE_LOG_INFO, "%s app_bt_service_task_char_ntf_event_get(): %d\n", __FUNCTION__, app_bt_service_task_char_ntf_event_get());
            if (app_bt_service_task_char_ntf_event_get() & (SESSION_SERVICE_HEATING_PROF_SEL_NTF_EVT))
            {
                if (app_server_context.bt_conn_id && (app_session_service_heating_prof_sel_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
                {
                    sm_log(BT_SERVICE_LOG_INFO, "%s s_heating_profile_proc_state: %d\n", __FUNCTION__, s_heating_profile_proc_state);
                    switch (s_heating_profile_proc_state)
                    {
                        case HEATING_PROFILE_REPORTED_HEADER:
                        {
                                // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                            status = app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_header(&heating_profile_num);
                            if ((status == WICED_BT_GATT_ERROR)||(heating_profile_num == 0))
                            {
                                app_bt_service_task_char_ntf_event_clear(SESSION_SERVICE_HEATING_PROF_SEL_NTF_EVT);//clear
                            }
                            else
                            {
                                s_heating_profile_proc_state = HEATING_PROFILE_REPORTED_DATA;
                                heating_profile_index=0;
                            }
                        }
                            break;
                        case HEATING_PROFILE_REPORTED_DATA:
                        {
                                // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL); 
                            if (heating_profile_index < heating_profile_num)
                            {
                                if (StateMachineCounter%APP_BT_CHAR_NTF_INTERVAL == 0)
                                {
                                    status = app_bt_char_session_service_heating_prof_sel_opcode04_profiles_payload_proc(heating_profile_index);//need more time to deal with this event.
                                    if (status == WICED_BT_GATT_ERROR)
                                    {
                                        s_heating_profile_proc_state = HEATING_PROFILE_REPORTED_HEADER;
                                        app_bt_service_task_char_ntf_event_clear(SESSION_SERVICE_HEATING_PROF_SEL_NTF_EVT);//clear
                                    }
                                    heating_profile_index++;
                                }
                            }
                            else
                            {
                                s_heating_profile_proc_state = HEATING_PROFILE_REPORTED_HEADER;
                                app_bt_service_task_char_ntf_event_clear(SESSION_SERVICE_HEATING_PROF_SEL_NTF_EVT);//clear
                            }
                        }
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    s_heating_profile_proc_state = HEATING_PROFILE_REPORTED_HEADER;
                }
            }
            
            /**
             * device managemant service uimage signatrue verify event*/
            if (app_bt_service_task_char_ntf_event_get() & (DEVICE_MANAGEMENT_SERVICE_UIMAGE_SIGN_VERIFY_EVT))
            {
                update_idl_delay_time();				// 阻止休眠
                app_bt_char_device_management_service_uimage_data_integrity_validation();
                
                app_bt_service_task_char_ntf_event_clear(DEVICE_MANAGEMENT_SERVICE_UIMAGE_SIGN_VERIFY_EVT);//clear
            }

            /**
             * session service factory reset event*/
            if (app_bt_service_task_char_ntf_event_get() & (SESSION_SERVICE_FACTORY_RESET_SET_EVT))
            {
                update_idl_delay_time();				// 阻止休眠
                sm_log(BT_SERVICE_LOG_INFO, "app_bt_service_task_char_ntf_event_get: 0x%02X\n", app_bt_service_task_char_ntf_event_get());

				uint8_t dat[1];
				dat[0] = 1;
				bt_adapter_factory_reset_ble(dat ,1);
                app_bt_service_task_char_ntf_event_clear(SESSION_SERVICE_FACTORY_RESET_SET_EVT);//clear
            }

            /**
             * debug service dummy session record insert proc*/
            if (app_bt_service_task_char_ntf_event_get() & (SESSION_SERVICE_DUMMY_SESSION_RECORD_SET_EVT))
            {
                // sm_log(BT_SERVICE_LOG_INFO, "app_bt_service_task_char_ntf_event_get: 0x%02X\n", app_bt_service_task_char_ntf_event_get());
                update_idl_delay_time();				// 阻止休眠

                uint32_t session_record_remain_number = 0;
                status =  bt_adapter_session_records_dummy_debug_set_proc(&session_record_remain_number);
                if (status == WICED_BT_GATT_ERROR)
                {
                    app_bt_service_task_char_ntf_event_clear(SESSION_SERVICE_DUMMY_SESSION_RECORD_SET_EVT);//clear
                }
                else if (session_record_remain_number == 0)
                {
                    app_bt_service_task_char_ntf_event_clear(SESSION_SERVICE_DUMMY_SESSION_RECORD_SET_EVT);//clear
                }
            }

            /**
             * debug service dummy event log insert proc*/
            if (app_bt_service_task_char_ntf_event_get() & (DEBUG_SERVICE_DUMMY_EVENT_LOG_SET_EVT))
            {
                // sm_log(BT_SERVICE_LOG_INFO, "app_bt_service_task_char_ntf_event_get: 0x%02X\n", app_bt_service_task_char_ntf_event_get());

                update_idl_delay_time();				// 阻止休眠
                uint32_t event_log_remain_number = 0;
                status =  bt_adapter_event_log_dummy_debug_set_proc(&event_log_remain_number);
                if (status == WICED_BT_GATT_ERROR)
                {
                    app_bt_service_task_char_ntf_event_clear(DEBUG_SERVICE_DUMMY_EVENT_LOG_SET_EVT);//clear
                }
                else if (event_log_remain_number == 0)
                {
                    app_bt_service_task_char_ntf_event_clear(DEBUG_SERVICE_DUMMY_EVENT_LOG_SET_EVT);//clear
                }
            }

            /*******************************************************Session Records Reported Data**************************************************************/
            if (app_server_context.bt_conn_id && (app_session_service_session_records_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
            {
                // sm_log(BT_SERVICE_LOG_INFO, "%s s_session_records_proc_state: %d\n", __FUNCTION__, s_session_records_proc_state);
                switch (s_session_records_proc_state)
                {
                    case SESSION_RECORDS_START:
                    {
                        if (bt_adapter_session_records_number_get() > 0)
                        {
                            update_idl_delay_time();				// 阻止休眠
                            // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                            app_bt_char_session_records_and_event_log_transmit_start(HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE);
                            s_session_records_proc_state = SESSION_RECORDS_REPORTED_DATA;
                        }
                        else if (bt_adapter_session_records_number_get() == 0) //bug1977774
                        {
                            s_session_records_proc_state = SESSION_RECORDS_LIVE_MODE;
                        }
                    }
                        break;
                    case SESSION_RECORDS_REPORTED_DATA:
                    {
                        // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                        if (bt_adapter_session_records_number_get() > 0)
                        {
                            update_idl_delay_time();				// 阻止休眠
                            
                            if (StateMachineCounter%APP_BT_CHAR_NTF_INTERVAL == 0)
                            {
                                status = app_bt_char_session_service_session_records_payload_proc();
                                if(WICED_BT_GATT_CONGESTED == status)
                                {
                                    sm_log(SM_LOG_NOTICE, "%s waiting no congestion event\n", __func__);
                                    /* 等待事件位 EVENT_NO_GATT_CONGESTION 被设置 */
                                    EventBits_t bits = xEventGroupWaitBits(bt_event_group, EVENT_NO_GATT_CONGESTION, pdTRUE, pdFALSE, GATT_CONGESTED_TIMEOUT/*portMAX_DELAY*/);
                            
                                    if(bits & EVENT_NO_GATT_CONGESTION)
                                    {
                                        /* 处理事件 EVENT_NO_GATT_CONGESTION */
                                    }
                                }
                                else if (status != WICED_BT_GATT_SUCCESS)
                                {
                                    s_session_records_proc_state = SESSION_RECORDS_FINISH;
                                }
                            }
                        }
                        else
                        {
                            s_session_records_proc_state = SESSION_RECORDS_FINISH;//first send finish 
                        }
                    }
                        break;
                    case SESSION_RECORDS_LIVE_MODE:
                    {
                        // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                        if (bt_adapter_session_records_number_get() > 0)
                        {
                            update_idl_delay_time();				// 阻止休眠
                            
                            if (StateMachineCounter%APP_BT_CHAR_NTF_INTERVAL == 0)
                            {
                                status = app_bt_char_session_service_session_records_payload_proc();
                                if(WICED_BT_GATT_CONGESTED == status)
                                {
                                    sm_log(SM_LOG_NOTICE, "%s waiting no congestion event\n", __func__);
                                    /* 等待事件位 EVENT_NO_GATT_CONGESTION 被设置 */
                                    EventBits_t bits = xEventGroupWaitBits(bt_event_group, EVENT_NO_GATT_CONGESTION, pdTRUE, pdFALSE, GATT_CONGESTED_TIMEOUT/*portMAX_DELAY*/);
                            
                                    if(bits & EVENT_NO_GATT_CONGESTION)
                                    {
                                        /* 处理事件 EVENT_NO_GATT_CONGESTION */
                                    }
                                }
                                else if (status != WICED_BT_GATT_SUCCESS)
                                {
                                    s_event_log_proc_state = EVENT_LOG_FINISH;
                                }
                            }
                        }
                    }
                        break;
                    case SESSION_RECORDS_FINISH:
                    {
                        cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);//bug2113187
                        app_bt_char_session_records_and_event_log_transmit_finish(HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE);
                        s_session_records_proc_state = SESSION_RECORDS_LIVE_MODE;//bug 1920431
                    }
                        break;
                    default:
                        //to be continue.
                        break;
                }
            }
            else
            {    
                //default value
                s_session_records_proc_state = SESSION_RECORDS_START;
            }
            /*******************************************************Event log Reported Data**************************************************************/
            if (app_server_context.bt_conn_id && (app_debug_service_event_log_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))//exchage to state machine
            {
                // sm_log(BT_SERVICE_LOG_INFO, "%s s_event_log_proc_state: %d\n", __FUNCTION__, s_event_log_proc_state);
                switch (s_event_log_proc_state)
                {
                    case EVENT_LOG_START:
                    {
                        if (bt_adapter_event_log_number_get() > 0)
                        {
                            update_idl_delay_time();				// 阻止休眠
                            // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                            app_bt_char_session_records_and_event_log_transmit_start(HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE);
                            s_event_log_proc_state = EVENT_LOG_REPORTED_DATA;
                        }
                        else if (bt_adapter_event_log_number_get() == 0) //bug1977774
                        {
                            s_event_log_proc_state = EVENT_LOG_LIVE_MODE;
                        }
                    }
                        break;
                    case EVENT_LOG_REPORTED_DATA:
                    {
                        // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                        if (bt_adapter_event_log_number_get() > 0)
                        {
                            update_idl_delay_time();				// 阻止休眠
                            
                            if (StateMachineCounter%APP_BT_CHAR_NTF_INTERVAL == 0)
                            {
                                status = app_bt_char_debug_service_event_log_payload_proc();
                                if(WICED_BT_GATT_CONGESTED == status)
                                {
                                    sm_log(SM_LOG_NOTICE, "%s waiting no congestion event\n", __func__);
                                    /* 等待事件位 EVENT_NO_GATT_CONGESTION 被设置 */
                                    EventBits_t bits = xEventGroupWaitBits(bt_event_group, EVENT_NO_GATT_CONGESTION, pdTRUE, pdFALSE, GATT_CONGESTED_TIMEOUT/*portMAX_DELAY*/);
                            
                                    if(bits & EVENT_NO_GATT_CONGESTION)
                                    {
                                        /* 处理事件 EVENT_NO_GATT_CONGESTION */
                                    }
                                }
                                else if (status != WICED_BT_GATT_SUCCESS)
                                {
                                    s_event_log_proc_state = EVENT_LOG_FINISH;
                                }
                            }
                        }
                        else
                        {
                            s_event_log_proc_state = EVENT_LOG_FINISH;
                        }
                    }
                        break;
                    
                    case EVENT_LOG_LIVE_MODE:
                    {
                        // cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);
                        if (bt_adapter_event_log_number_get() > 0)
                        {
                            update_idl_delay_time();				// 阻止休眠
                            
                            if (StateMachineCounter%APP_BT_CHAR_NTF_INTERVAL == 0)
                            {
                                status = app_bt_char_debug_service_event_log_payload_proc();
                                if(WICED_BT_GATT_CONGESTED == status)
                                {
                                    sm_log(SM_LOG_NOTICE, "%s waiting no congestion event\n", __func__);
                                    /* 等待事件位 EVENT_NO_GATT_CONGESTION 被设置 */
                                    EventBits_t bits = xEventGroupWaitBits(bt_event_group, EVENT_NO_GATT_CONGESTION, pdTRUE, pdFALSE, GATT_CONGESTED_TIMEOUT/*portMAX_DELAY*//*6*/);
                            
                                    if(bits & EVENT_NO_GATT_CONGESTION)
                                    {
                                        /* 处理事件 EVENT_NO_GATT_CONGESTION */
                                    }
                                }
                                else if (status != WICED_BT_GATT_SUCCESS)
                                {
                                    s_event_log_proc_state = EVENT_LOG_FINISH;
                                }
                            }
                        }
                    }
                        break;
                    case EVENT_LOG_FINISH:
                    {
                        cy_rtos_delay_milliseconds(APP_BT_CHAR_NTF_INTERVAL);//bug2113187
                        app_bt_char_session_records_and_event_log_transmit_finish(HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE);
                        s_event_log_proc_state = EVENT_LOG_LIVE_MODE;//bug 1920431
                    }
                        break;
                    default:
                        //to be continue.
                        break;
                }
            }
            else
            {    
                //default value
                s_event_log_proc_state = EVENT_LOG_START;
            }
            
            if ((((app_server_context.bt_conn_id && (app_session_service_session_records_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)) == 0)&&
               ((app_server_context.bt_conn_id && (app_debug_service_event_log_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)) == 0)&&
               (app_bt_service_task_char_ntf_event_get() == NONE_BT_SERVICE_NTF_EVT)) || (app_bt_char_bt_service_task_ntf_loop_exit_flag_get()))
            {
                if (app_server_context.bt_conn_id == 0) //bug 1977774
                {
                    s_session_records_proc_state = SESSION_RECORDS_START;
                    s_event_log_proc_state = EVENT_LOG_START;
                }

                sm_log(BT_SERVICE_LOG_INFO, "app_server_context.bt_conn_id: %d\n", app_server_context.bt_conn_id);
                sm_log(BT_SERVICE_LOG_INFO, "app_session_service_session_records_client_char_config[0]: %d\n", app_session_service_session_records_client_char_config[0]);
                sm_log(BT_SERVICE_LOG_INFO, "app_debug_service_event_log_char_config[0]: %d\n", app_debug_service_event_log_char_config[0]);
                break; //wait for ulTaskNotifyTake event
            }

            StateMachineCounter++;
            if (0xFFFFFFFF == StateMachineCounter)
            {
                StateMachineCounter=0;
            }
            cy_rtos_delay_milliseconds(5); //uint: 5ms
        }
    }
}

/**
 * Function Name:
 * app_bt_gatt_event_callback
 *
 * Function Description:
 * @brief  This Function handles the all the GATT events - GATT Event Handler
 *
 * @param event            Bluetooth LE GATT event type
 * @param p_event_data     Pointer to Bluetooth LE GATT event data
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_gatt_event_callback(wiced_bt_gatt_evt_t event,
                                                         wiced_bt_gatt_event_data_t *p_event_data)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint16_t error_handle = 0;

    wiced_bt_gatt_attribute_request_t *p_attr_req = &p_event_data->attribute_request;

    // 如果系统在睡眠状态则先唤醒
    app_bt_char_awaken_device(500);//ms

    // sm_log(SM_LOG_DEBUG, "%s() p_attr_req, conn_id: 0x%02x len_requested: 0x%02x opcode: 0x%02X data:\r\n", __func__, 
    //                                                                                                         p_attr_req->conn_id, 
    //                                                                                                         p_attr_req->len_requested, 
    //                                                                                                         p_attr_req->opcode);

    /* Call the appropriate callback function based on the GATT event type,
     * and pass the relevant event
     * parameters to the callback function */
    switch (event)
    {
    case GATT_CONNECTION_STATUS_EVT:
        status = app_bt_connect_event_handler (&p_event_data->connection_status);
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        status = app_bt_server_event_handler (p_event_data, 
                                              &error_handle);
        if(status != WICED_BT_GATT_SUCCESS)
        {
           wiced_bt_gatt_server_send_error_rsp(p_attr_req->conn_id, 
                                               p_attr_req->opcode, 
                                               error_handle, 
                                               status);
        }

        break;
        /* GATT buffer request, typically sized to max of bearer mtu - 1 */
    case GATT_GET_RESPONSE_BUFFER_EVT:
        sm_log(SM_LOG_DEBUG, "%s() GATT_GET_RESPONSE_BUFFER_EVT\r\n", __func__);
        p_event_data->buffer_request.buffer.p_app_rsp_buffer = app_bt_alloc_buffer(p_event_data->buffer_request.len_requested);
        p_event_data->buffer_request.buffer.p_app_ctxt = (void *)app_bt_free_buffer;
        status = WICED_BT_GATT_SUCCESS;
        break;
        /* GATT buffer transmitted event,  check \ref wiced_bt_gatt_buffer_transmitted_t*/
    case GATT_APP_BUFFER_TRANSMITTED_EVT:
        pfn_free_buffer_t pfn_free = (pfn_free_buffer_t)p_event_data->buffer_xmitted.p_app_ctxt;
        // sm_log(SM_LOG_DEBUG, "\r\n\n%s() GATT_APP_BUFFER_TRANSMITTED_EVT, pfn_free: %p\r\n", __func__, pfn_free);
        {
            /* If the buffer is dynamic, the context will point to a function to free it. */
            if (pfn_free)
                pfn_free(p_event_data->buffer_xmitted.p_app_data);

            status = WICED_BT_GATT_SUCCESS;
        }
        break;

    case GATT_CONGESTION_EVT:
    {
        sm_log(SM_LOG_NOTICE, "%s() GATT_CONGESTION_EVT, sta:%d\r\n", __func__, p_event_data->congestion.congested);
        if(!p_event_data->congestion.congested)
        {
            /* 设置事件位 EVENT_NO_GATT_CONGESTION */
            xEventGroupSetBits(bt_event_group, EVENT_NO_GATT_CONGESTION);
        }
        status = WICED_BT_GATT_SUCCESS;
    }
        break;
    default:
        sm_log(SM_LOG_INFO, " Unhandled GATT Event \r\n");
        status = WICED_BT_GATT_ERROR;
        break;
    }

    return status;
}

/**
 * Function Name
 * app_bt_client_char_config_disable
 *
 * Function Description
 * @brief   This callback function disable client char config
 *
 * @param   None
 *
 * @return  wiced_bt_gatt_status_t See possible status codes in wiced_bt_gatt_status_e
 * in wiced_bt_gatt.h
 */

static wiced_bt_gatt_status_t app_bt_client_char_config_disable(void)
{
    app_session_service_device_information_client_char_config[0] = 0;
    app_session_service_battery_power_remaining_client_char_config[0] = 0;
    app_session_service_device_lock_client_char_config[0] = 0;
    app_session_service_session_records_client_char_config[0] = 0;
    app_session_service_session_status_client_char_config[0] = 0;
    app_session_service_findmy_device_client_char_config[0] = 0;
    app_session_service_heating_prof_sel_client_char_config[0] = 0;
    app_debug_service_last_error_char_config[0] = 0;
    app_debug_service_event_log_char_config[0] = 0;
    app_debug_service_lifecycle_data_char_config[0] = 0;
    app_device_management_service_control_char_config[0] = 0;
    app_device_management_service_challenge_char_config[0] = 0;

    return WICED_BT_GATT_SUCCESS;
}

/**
 * Function Name
 * app_bt_connect_event_handler
 *
 * Function Description
 * @brief   This callback function handles connection status changes.
 *
 * @param p_conn_status    Pointer to data that has connection details
 *
 * @return wiced_bt_gatt_status_t See possible status codes in wiced_bt_gatt_status_e
 * in wiced_bt_gatt.h
 */

static wiced_bt_gatt_status_t app_bt_connect_event_handler (wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

    if (NULL != p_conn_status)
    {
        if (p_conn_status->connected)
        {
            /* Device has connected */
            sm_log(SM_LOG_NOTICE, "Connected : BDA ");
            print_bd_address(p_conn_status->bd_addr);

#ifdef GATT_CONGESTED_WORKAROUND
            bCongestedRebootCnt = 0;
            gatt_ntf_congested_duration = 0;

            size_t gatt_congested_cnt;
            cy_rtos_semaphore_get_count(&gatt_congested, &gatt_congested_cnt);
            sm_log(SM_LOG_NOTICE, "\r\n1:gatt_congested_cnt:%d\r\n", gatt_congested_cnt);
            if ((GATT_CONGESTED_MAX_NUM - gatt_congested_cnt) > 0)
            {
                for (uint8_t i=0; i<(GATT_CONGESTED_MAX_NUM - gatt_congested_cnt);i++)
                    cy_rtos_semaphore_set(&gatt_congested);
            }
            cy_rtos_semaphore_get_count(&gatt_congested, &gatt_congested_cnt);
            sm_log(SM_LOG_NOTICE, "\r\n2:gatt_congested_cnt:%d\r\n", gatt_congested_cnt);
#endif
            /* 判断是否是已绑定的主机来连接的? if yes, and setting pairing success event*/
            if ((BOND_INDEX_MAX != app_bt_find_device_in_flash(p_conn_status->bd_addr))||
                (BOND_INDEX_MAX != app_bt_find_device_in_flash_RPA(p_conn_status->bd_addr)))
            {
                if(TRUE == pairing_mode)
                {
                    /* setting pari event*/
                    set_system_external_even(EXT_EVENT_PARI_OK);//it do not need to display PARI_OK
                }
            }
            else if(FALSE == pairing_mode) /* 判断是否是已绑定的主机来连接的? if not, and disconnecting*/
            {
                sm_log(SM_LOG_NOTICE, "Disconnection ID '%d'\r\n", p_conn_status->conn_id);
                wiced_bt_gatt_disconnect(p_conn_status->conn_id);
                return status; 
            }

            sm_log(SM_LOG_NOTICE, "Connection ID '%d'\r\n", p_conn_status->conn_id);

            /* Store the connection ID and peer BD Address */
            app_server_context.bt_conn_id = p_conn_status->conn_id;
            /* Save BT peer ADDRESS in application data structure */
            memcpy(app_server_context.bt_peer_addr, p_conn_status->bd_addr, BD_ADDR_LEN);

            /* Update the adv/conn state */
            app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_ON;

            // /* Save BT peer ADDRESS in application data structure */
            // memcpy(app_server_context.bt_peer_addr, p_conn_status->bd_addr, BD_ADDR_LEN);

            sm_log(SM_LOG_NOTICE, "get_subSystem_status(): %d\n\n", get_subSystem_status());
            #if 0 //BAT indicate that it need to 'PAIRING_CANCLE' after bonding successful. - 20241112 vincent.he
            /* UI display pairing stop*/
            if (get_subSystem_status() == SUBSTATUS_PARING)
            {
                set_system_external_even(EXT_EVENT_PARING_CANCLE);
                cy_rtos_delay_milliseconds(5);
            }
            #endif
            /* restore ntf_loop_exit_flag*/
            app_bt_char_bt_service_task_ntf_loop_exit_flag_set(0);

#if 0       /* Trigger 2M PHY update request */
            wiced_bt_ble_phy_preferences_t phy_preferences;

            phy_preferences.rx_phys = BTM_BLE_PREFER_2M_PHY;
            phy_preferences.tx_phys = BTM_BLE_PREFER_2M_PHY;
            memcpy(phy_preferences.remote_bd_addr, app_server_context.bt_peer_addr,
                                                                BD_ADDR_LEN);
            result = wiced_bt_ble_set_phy(&phy_preferences);
            if (result == WICED_BT_SUCCESS)
            {
                sm_log(SM_LOG_NOTICE, "Request sent to switch PHY to %dM\n",
                                    phy_preferences.tx_phys);
            }
            else
            {
                sm_log(SM_LOG_NOTICE, "PHY switch request failed, result: %d\n", status);
                CY_ASSERT(0);
            }
#endif      /* Trigger 2M PHY update request */
        }
        else
        {
            /* Device has disconnected */
            sm_log(SM_LOG_NOTICE, "Disconnected : BDA ");
            print_bd_address(p_conn_status->bd_addr);
            sm_log(SM_LOG_NOTICE,"Connection ID '%d', Reason '%s'\r\n", p_conn_status->conn_id,
                      get_bt_gatt_disconn_reason_name(p_conn_status->reason));

            /* Set the connection id to zero to indicate disconnected state */
            app_server_context.bt_conn_id = 0;

            /* BT task Exit flag set */
            app_bt_char_bt_service_task_ntf_loop_exit_flag_set(1);

            /* restore key detect*/
            set_update_ui_timer_cnt(0);

            /* UI display loading stop*/    
            sm_log(SM_LOG_NOTICE, "get_subSystem_status(): %d\n\n", get_subSystem_status());
            if (get_subSystem_status() == SUBSTATUS_LOADING)
            {
                set_system_external_even(EXT_EVENT_LOADING_STOP);
                cy_rtos_delay_milliseconds(5);
            }

            /* Clear ntf event */
            app_bt_service_task_char_ntf_event_set(NONE_BT_SERVICE_NTF_EVT);
    
            /* DeviceManagement Payload/EXT-Flash sha256 validation*/
            if (app_bt_char_device_manage_payload_length_get() != 0)
            {
                app_bt_char_device_manage_payload_length_clear();
                app_bt_service_task_char_ntf_event_set(DEVICE_MANAGEMENT_SERVICE_UIMAGE_SIGN_VERIFY_EVT);//bug1951347
            }

            /* client char config disable*/
            app_bt_client_char_config_disable(); //bug1990596

#if BT_ADV_START
            sm_log(BT_SERVICE_LOG_INFO, "\n disconnect, bond_info(%d): \r\n",sizeof(wiced_bt_device_sec_keys_t));
            print_array(&bond_info.link_keys[0].key_data, sizeof(wiced_bt_device_sec_keys_t));
            sm_log(SM_LOG_DEBUG, "\nDevice added to address resolution database: ");
            print_bd_address((uint8_t *)&bond_info.link_keys[0].bd_addr);
            app_bt_adv_start();
#endif
        }

        status = WICED_BT_GATT_SUCCESS;
    }

    return status;
}

/**
 * Function Name:
 * app_bt_server_event_handler
 *
 * Function Description:
 * @brief  The callback function is invoked when GATT_ATTRIBUTE_REQUEST_EVT occurs
 *         in GATT Event handler function. GATT Server Event Callback function.
 *
 * @param p_data   Pointer to Bluetooth LE GATT request data
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_server_event_handler (wiced_bt_gatt_event_data_t *p_data, 
                                                           uint16_t *p_error_handle)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    wiced_bt_gatt_attribute_request_t   *p_att_req = &p_data->attribute_request;

    switch (p_att_req->opcode)
    {
    /* Attribute read notification (attribute value internally read from GATT database) */
    case GATT_REQ_READ:
    case GATT_REQ_READ_BLOB:
        sm_log(SM_LOG_DEBUG, "  %s() GATTS_REQ_TYPE_READ\r\n", __func__);
        status = app_bt_gatt_req_read_handler(p_att_req->conn_id, p_att_req->opcode,
                                              &p_att_req->data.read_req,
                                              p_att_req->len_requested, 
                                              p_error_handle);
        break;

    case GATT_REQ_READ_BY_TYPE:
        status = app_bt_gatt_req_read_by_type_handler(p_att_req->conn_id, p_att_req->opcode,
                                                      &p_att_req->data.read_by_type,
                                                      p_att_req->len_requested, 
                                                      p_error_handle);
        break;

    case GATT_REQ_READ_MULTI:
    case GATT_REQ_READ_MULTI_VAR_LENGTH:
        status = app_bt_gatt_req_read_multi_handler(p_att_req->conn_id, p_att_req->opcode,
                                                    &p_att_req->data.read_multiple_req,
                                                    p_att_req->len_requested, 
                                                    p_error_handle);
        break;

    case GATT_REQ_WRITE:
    case GATT_CMD_WRITE:
    case GATT_CMD_SIGNED_WRITE:
        status = app_bt_write_handler(p_data, p_error_handle);
        if ((p_att_req->opcode == GATT_REQ_WRITE) && (status == WICED_BT_GATT_SUCCESS))
        {
            // sm_log(BT_SERVICE_LOG_INFO, "  %s() GATTS_REQ_WRITE\r\n", __func__);
            wiced_bt_gatt_write_req_t *p_write_request = &p_att_req->data.write_req;
            wiced_bt_gatt_server_send_write_rsp(p_att_req->conn_id, 
                                                p_att_req->opcode,
                                                p_write_request->handle);
        }
        else if ((p_att_req->opcode == GATT_CMD_WRITE) && (status == WICED_BT_GATT_SUCCESS))
        {
            // sm_log(BT_SERVICE_LOG_INFO, "  %s() GATT_CMD_WRITE\r\n", __func__);
        }
        break;

    case GATT_REQ_PREPARE_WRITE:
        sm_log(SM_LOG_DEBUG, "  %s() GATT_REQ_PREPARE_WRITE\r\n", __func__);
        status = WICED_BT_GATT_SUCCESS;
        break;

    case GATT_REQ_EXECUTE_WRITE:
        sm_log(SM_LOG_DEBUG, "  %s() GATTS_REQ_TYPE_WRITE_EXEC\r\n",
                  __func__);
        wiced_bt_gatt_server_send_execute_write_rsp(p_att_req->conn_id, 
                                                    p_att_req->opcode);
        status = WICED_BT_GATT_SUCCESS;
        break;

    case GATT_REQ_MTU:
        /* Application calls wiced_bt_gatt_server_send_mtu_rsp() with the desired mtu */
        sm_log(SM_LOG_DEBUG, "  %s() GATTS_REQ_TYPE_MTU\r\n", __func__);
        status = wiced_bt_gatt_server_send_mtu_rsp(p_att_req->conn_id,
                                                   p_att_req->data.remote_mtu,
                                                   wiced_bt_cfg_settings.p_ble_cfg->ble_max_rx_pdu_size);
       sm_log(SM_LOG_NOTICE, "    Set MTU size to: %d  status: 0x%d\r\n",
                   p_att_req->data.remote_mtu, status);
        break;

    case GATT_HANDLE_VALUE_CONF: /* Value confirmation */
       sm_log(SM_LOG_DEBUG, "  %s() GATTS_REQ_TYPE_CONF\r\n",
                  __func__);
#if (OTA_SERVICE_SUPPORT == 1)
        cy_ota_agent_state_t ota_lib_state;
        cy_ota_get_state(app_server_context.ota_context, &ota_lib_state);
        if ((ota_lib_state == CY_OTA_STATE_OTA_COMPLETE) && /* Check if we completed the download before rebooting */
            (app_server_context.reboot_at_end != 0))
        {
            if (app_bt_gatt_ota_download_verify_is_succeed())//if OTA verify fail ,and then don't reset system.
            {
                sm_log(SM_LOG_NOTICE, "%s()   RESETTING NOW !!!!\r\n",
                        __func__);

                //set_time_stamp_to_flash();
                flash_data_save_change(0);		// 带时间戳

                cy_rtos_delay_milliseconds(1000);
                NVIC_SystemReset();
            }
            else
            {
                /* UI display loading stop*/
                set_system_external_even(EXT_EVENT_LOADING_STOP);
                cy_rtos_delay_milliseconds(500);
                /* restore key detect*/
                set_update_ui_timer_cnt(10);
                
                sm_log(SM_LOG_NOTICE, "%s()   cy_ota_agent_stop!!!!, ota_lib_state:%d\r\n",
                        __func__, ota_lib_state);
                cy_ota_agent_stop(&app_server_context.ota_context); /* Stop OTA */
            }
        }
        else
        {
            if (CY_OTA_STATE_NOT_INITIALIZED == ota_lib_state)
            {
                sm_log(SM_LOG_NOTICE, "%s()   ota_lib_state:CY_OTA_STATE_NOT_INITIALIZED\r\n",
                        __func__);
            }
            else
            {
                sm_log(SM_LOG_NOTICE, "%s()   cy_ota_agent_stop!!!!, ota_lib_state:%d\r\n",
                        __func__, ota_lib_state);
                cy_ota_agent_stop(&app_server_context.ota_context); /* Stop OTA */
            }
        }
#endif
        status = WICED_BT_GATT_SUCCESS;
        break;

    case GATT_HANDLE_VALUE_NOTIF:
        // sm_log(SM_LOG_DEBUG, "  %s() GATT_HANDLE_VALUE_NOTIF - Notification Sent\r\n", __func__);
        status = WICED_BT_GATT_SUCCESS;
        break;

    default:
        sm_log(SM_LOG_DEBUG, "  %s() Unhandled Event opcode: %d\r\n",
                  __func__, p_att_req->opcode);
        status = WICED_BT_GATT_ERROR;
        break;
    }
    return status;
}

/**
 * Function Name:
 * app_bt_write_handler
 *
 * Function Description:
 * @brief  The function is invoked when GATTS_REQ_TYPE_WRITE is received from the
 *         client device and is invoked GATT Server Event Callback function. This
 *         handles "Write Requests" received from Client device.
 *
 * @param p_write_req   Pointer to Bluetooth LE GATT write request
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_write_handler(wiced_bt_gatt_event_data_t *p_data, 
                                                   uint16_t *p_error_handle)
{
    wiced_bt_gatt_write_req_t *p_write_req = &p_data->attribute_request.data.write_req;;
    cy_rslt_t result;

    *p_error_handle = p_write_req->handle;

    CY_ASSERT(( NULL != p_data ) && (NULL != p_write_req));

    switch (p_write_req->handle)
    {        
        case HDLC_GAP_DEVICE_NAME_VALUE:
        {
            /*Call app_bt_char_device_name_write_handler related writes*/
            result = app_bt_char_device_name_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLD_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_CLIENT_CHAR_CONFIG:
        case HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE:
        case HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_DATA_VALUE:
        {
            if (get_system_status() == SLEEP)
            {
                cy_rtos_delay_milliseconds(300);
            }
#if (OTA_SERVICE_SUPPORT == 1)
            /*Call OTA write handler to handle OTA related writes*/
            result = app_bt_ota_write_handler(p_data, p_error_handle);
#endif
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_DEVICE_INFORMATION_VALUE:
        {
            /*Call app_bt_char_session_service_device_information_write_handler related writes*/
            result = app_bt_char_session_service_device_information_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_TIME_VALUE:
        {
            /*Call app_bt_char_session_service_time_write_handler related writes*/
            result = app_bt_char_session_service_time_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_DEVICE_LOCK_VALUE:
        {
            /*Call app_bt_char_session_service_device_lock_write_handler related writes*/
            result = app_bt_char_session_service_device_lock_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE:
        {
            /*Call app_bt_char_session_service_session_records_write_handler related writes*/
            result = app_bt_char_session_service_session_records_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_SESSION_STATUS_VALUE:
        {
            /*Call app_bt_char_session_service_session_status_write_handler related writes*/
            result = app_bt_char_session_service_session_status_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_FINDMY_DEVICE_VALUE:
        {
            /*Call app_bt_char_session_service_findmy_device_write_handler related writes*/
            result = app_bt_char_session_service_findmy_device_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_SCREEN_LED_CONTROL_VALUE:
        {
            /*Call app_bt_char_session_service_screen_led_control_write_handler related writes*/
            result = app_bt_char_session_service_screen_led_control_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_FACTORY_RESET_VALUE:
        {
            /*Call app_bt_char_session_service_factory_reset_write_handler related writes*/
            result = app_bt_char_session_service_factory_reset_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_HEATING_PROF_SEL_VALUE:
        {
            /*Call app_bt_char_session_service_heating_prof_sel_write_handler related writes*/
            result = app_bt_char_session_service_heating_prof_sel_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_HAPTIC_SET_VALUE:
        {
            /*Call app_bt_char_session_service_haptic_set_write_handler related writes*/
            result = app_bt_char_session_service_haptic_set_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_SESSION_SERVICE_BUZZER_SPEAKER_SET_VALUE:
        {
            /*Call app_bt_char_session_service_buzzer_speaker_set_write_handler related writes*/
            result = app_bt_char_session_service_buzzer_speaker_set_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE:
        {            
            /*Call app_bt_char_debug_service_event_log_write_handler related writes*/
            result = app_bt_char_debug_service_event_log_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break; 
        case HDLC_DEBUG_SERVICE_LIFECYCLE_DATA_VALUE:
        {            
            /*Call app_bt_char_debug_service_lifecycle_data_write_handler related writes*/
            result = app_bt_char_debug_service_lifecycle_data_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_DEVICE_MANAGEMENT_SERVICE_VERSION_VALUE:
        { 
            /*Call app_bt_char_device_management_service_version_write_handler related writes*/
            result = app_bt_char_device_management_service_version_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_DEVICE_MANAGEMENT_SERVICE_CONTROL_VALUE:
        {
            /*Call app_bt_char_device_management_service_control_write_handler related writes*/
            result = app_bt_char_device_management_service_control_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_DEVICE_MANAGEMENT_SERVICE_PAYLOAD_VALUE:
        {
            /*Call app_bt_char_device_management_service_payload_write_handler related writes*/
            result = app_bt_char_device_management_service_payload_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        case HDLC_DEVICE_MANAGEMENT_SERVICE_CHALLENGE_VALUE:
        {
            /*Call app_bt_char_device_management_service_challenge_write_handler related writes*/
            result = app_bt_char_device_management_service_challenge_write_handler(p_write_req->handle, p_write_req->p_val, p_write_req->val_len);
            return (result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;
        }
            break;
        default:
            /* Handle normal (non-OTA) indication confirmation requests here */
            /* Attempt to perform the Write Request */
            return app_bt_set_value(p_write_req->handle,
                                    p_write_req->p_val,
                                    p_write_req->val_len);
            break;
    }
    
    return WICED_BT_GATT_SUCCESS;
}


/**
 * Function Name:
 * app_bt_set_value
 *
 * Function Description:
 * @brief  The function is invoked by app_bt_write_handler to set a value
 *         to GATT DB.
 *
 * @param attr_handle  GATT attribute handle
 * @param p_val        Pointer to Bluetooth LE GATT write request value
 * @param len          length of GATT write request
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_set_value(uint16_t attr_handle, 
                                               uint8_t *p_val,
                                               uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_INVALID_HANDLE;

    sm_log(SM_LOG_NOTICE, "%s() handle : 0x%x len: %d char_config: 0x%02x %02x\r\n", __func__,
               attr_handle, len, p_val[0], p_val[1]);

    for (int i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        /* Check for a matching handle entry */
        if (app_gatt_db_ext_attr_tbl[i].handle == attr_handle)
        {
            /* Detected a matching handle in the external lookup table */
            if (app_gatt_db_ext_attr_tbl[i].max_len >= len)
            {
                /* Value fits within the supplied buffer; copy over the value */
                app_gatt_db_ext_attr_tbl[i].cur_len = len;
                memset(app_gatt_db_ext_attr_tbl[i].p_data, 0x00, app_gatt_db_ext_attr_tbl[i].max_len);
                memcpy(app_gatt_db_ext_attr_tbl[i].p_data, p_val, app_gatt_db_ext_attr_tbl[i].cur_len);//update char config ，such as 0x2902 enable

                if (memcmp(app_gatt_db_ext_attr_tbl[i].p_data, p_val, app_gatt_db_ext_attr_tbl[i].cur_len) == 0)
                {
                    status = WICED_BT_GATT_SUCCESS;
                }
                if ((HDLD_DEBUG_SERVICE_EVENT_LOG_CLIENT_CHAR_CONFIG == attr_handle)||(HDLD_SESSION_SERVICE_SESSION_RECORDS_CLIENT_CHAR_CONFIG == attr_handle))
                {
                    // 通知bt任务
                    TaskHandle_t *temp_handle;
                    temp_handle = get_task_bt_handle();
                    xTaskNotifyGive(*temp_handle);
                }
            }
            else
            {
                /* Value to write will not fit within the table */
                status = WICED_BT_GATT_INVALID_ATTR_LEN;
                sm_log(SM_LOG_ERR, "%d,%dInvalid attribute length\r\n",attr_handle,len);
                sm_log(SM_LOG_ERR, "%s\r\n",p_val);
            }
            break;
        }
    }
    if (WICED_BT_GATT_SUCCESS != status)
    {
        sm_log(SM_LOG_ERR, "%s() FAILED %d \r\n", __func__, status);
    }
    return status;
}
 



/**
 * Function Name:
 * app_bt_find_by_handle
 *
 * Function Description:
 * @brief  Find attribute description by handle
 *
 * @param handle    handle to look up
 *
 * @return gatt_db_lookup_table_t   pointer containing handle data
 */
static gatt_db_lookup_table_t *app_bt_find_by_handle(uint16_t handle)
{
    int i;
    for (i = 0; i < app_gatt_db_ext_attr_tbl_size; i++)
    {
        if (app_gatt_db_ext_attr_tbl[i].handle == handle)
        {
            return (&app_gatt_db_ext_attr_tbl[i]);
        }
    }
    return NULL;
}

/**
 * Function Name:
 * app_bt_gatt_req_read_handler
 *
 * Function Description:
 * @brief  This Function Process read request from peer device
 *
 * @param conn_id       Connection ID
 * @param opcode        Bluetooth LE GATT request type opcode
 * @param p_read_req    Pointer to read request containing the handle to read
 * @param len_requested length of data requested
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_gatt_req_read_handler(uint16_t conn_id,
                                                           wiced_bt_gatt_opcode_t opcode,
                                                           wiced_bt_gatt_read_t *p_read_req,
                                                           uint16_t len_requested, 
                                                           uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    uint16_t attr_len_to_copy, to_send;
    uint8_t *from;
    // static uint8_t battery_lev_temp=50;
    
    ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);

    *p_error_handle = p_read_req->handle;

    if ((puAttribute = app_bt_find_by_handle(p_read_req->handle)) == NULL)//verify handle.
    {
        sm_log(SM_LOG_ERR, "%s()  Attribute not found, Handle: 0x%04x\r\n",
                    __func__, p_read_req->handle);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    switch (p_read_req->handle)
    {
        case HDLC_GAP_DEVICE_NAME_VALUE:
        {
            /*Call app_bt_char_device_name_read_handler related reading*/
            app_bt_char_device_name_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_DEVICE_INFORMATION_VALUE:
        {
            /*Call app_bt_char_session_service_device_information_read_handler related reading*/
            app_bt_char_session_service_device_information_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_TIME_VALUE:
        {
            /*Call app_bt_char_session_service_time_read_handler related reading*/
            app_bt_char_session_service_time_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_BATTERY_POWER_REMAINING_VALUE:
        {
            /*Call app_bt_char_session_service_battery_power_remaining_read_handler related reading*/
            app_bt_char_session_service_battery_power_remaining_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_DEVICE_LOCK_VALUE:
        {
            /*Call app_bt_char_session_service_device_lock_read_handler related reading*/
            app_bt_char_session_service_device_lock_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_SESSION_RECORDS_VALUE:
        {
            /*Call app_bt_char_session_service_session_records_read_handler related reading*/
            return app_bt_char_session_service_session_records_read_handler(conn_id, opcode);
        }
            break;
        case HDLC_SESSION_SERVICE_SESSION_STATUS_VALUE:
        {
            /*Call app_bt_char_session_service_session_status_read_handler related reading*/
            app_bt_char_session_service_session_status_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_FINDMY_DEVICE_VALUE:
        {
            /*Call app_bt_char_session_service_findmy_device_read_handler related reading*/
            app_bt_char_session_service_findmy_device_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_SCREEN_LED_CONTROL_VALUE:
        {
            /*Call app_bt_char_session_service_screen_led_control_read_handler related reading*/
            app_bt_char_session_service_screen_led_control_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_HEATING_PROF_SEL_VALUE:
        {
            /*Call app_bt_char_session_service_heating_prof_sel_read_handler related reading*/
            // app_bt_char_session_service_heating_prof_sel_read_handler();
            return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, 0, NULL, NULL); /* No need for context, as buff not allocated */
        }
            break;
        case HDLC_SESSION_SERVICE_HAPTIC_SET_VALUE:
        {
            /*Call app_bt_char_session_service_haptic_set_read_handler related reading*/
            app_bt_char_session_service_haptic_set_read_handler();
        }
            break;
        case HDLC_SESSION_SERVICE_BUZZER_SPEAKER_SET_VALUE:
        {
            /*Call app_bt_char_session_service_buzzer_speaker_set_read_handler related reading*/
            app_bt_char_session_service_buzzer_speaker_set_read_handler();
        }
            break;
        case HDLC_DEBUG_SERVICE_LAST_ERROR_VALUE:
        {
            /*Call app_bt_char_debug_service_last_error_read_handler related reading*/
            app_bt_char_debug_service_last_error_read_handler();
        }
            break;
        case HDLC_DEBUG_SERVICE_EVENT_LOG_VALUE:
        {
            /*Call app_bt_char_debug_service_event_log_read_handler related reading*/
            return app_bt_char_debug_service_event_log_read_handler(conn_id, opcode);
        }
            break;
        case HDLC_DEBUG_SERVICE_LIFECYCLE_DATA_VALUE:
        {
            /*Call app_bt_char_debug_service_lifecycle_data_read_handler related reading*/
            // app_bt_char_debug_service_lifecycle_data_read_handler();
            return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, 0, NULL, NULL); /* No need for context, as buff not allocated */
        }
            break;
        default:
            //to be continue.
            break;
    }

    attr_len_to_copy = puAttribute->cur_len;

    sm_log(SM_LOG_NOTICE, "%s() conn_id: %d handle:0x%04x offset:%d len:%d\r\n", __func__,
               conn_id, p_read_req->handle, p_read_req->offset, attr_len_to_copy);

    if (p_read_req->offset >= puAttribute->cur_len)
    {
        sm_log(SM_LOG_ERR, "%s() offset:%d larger than attribute length:%d\r\n", __func__,
                   p_read_req->offset, puAttribute->cur_len);

        return WICED_BT_GATT_INVALID_OFFSET;
    }

    to_send = MIN(len_requested, attr_len_to_copy - p_read_req->offset);
    from = puAttribute->p_data + p_read_req->offset;//this is uuid char ext data.

    return wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode, to_send, from, NULL); /* No need for context, as buff not allocated */
}

/**
 * Function Name:
 * app_bt_gatt_req_read_by_type_handler
 *
 * Function Description:
 * @brief  Process read-by-type request from peer device
 *
 * @param conn_id       Connection ID
 * @param opcode        Bluetooth LE GATT request type opcode
 * @param p_read_req    Pointer to read request containing the handle to read
 * @param len_requested length of data requested
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_gatt_req_read_by_type_handler(uint16_t conn_id,
                                                                   wiced_bt_gatt_opcode_t opcode,
                                                                   wiced_bt_gatt_read_by_type_t *p_read_req,
                                                                   uint16_t len_requested, 
                                                                   uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    uint16_t last_handle = 0;
    uint16_t attr_handle = p_read_req->s_handle;
    uint8_t *p_rsp = app_bt_alloc_buffer(len_requested);
    uint8_t pair_len = 0;
    int used = 0;

    if (p_rsp == NULL)
    {
        sm_log(SM_LOG_ERR, "%s() No memory, len_requested: %d!!\r\n",
                   __func__, len_requested);

        return WICED_BT_GATT_INSUF_RESOURCE;
    }

    /* Read by type returns all attributes of the specified type, between the start and end handles */
    while (WICED_TRUE)
    {
        *p_error_handle = attr_handle;
        last_handle = attr_handle;
        attr_handle = wiced_bt_gatt_find_handle_by_type(attr_handle, 
                                                        p_read_req->e_handle,
                                                        &p_read_req->uuid);

        if (attr_handle == 0)
            break;

        if ((puAttribute = app_bt_find_by_handle(attr_handle)) == NULL)
        {
            sm_log(SM_LOG_ERR, "%s()  found type but no attribute for %d \r\n",
                       __func__, last_handle);
            app_bt_free_buffer(p_rsp);
            return WICED_BT_GATT_INVALID_HANDLE;
        }

        {
            int filled = wiced_bt_gatt_put_read_by_type_rsp_in_stream(p_rsp + used,
                                                                      len_requested - used,
                                                                      &pair_len,
                                                                      attr_handle,
                                                                      puAttribute->cur_len,
                                                                      puAttribute->p_data);
            if (filled == 0)
            {
                break;
            }
            used += filled;
        }

        /* Increment starting handle for next search to one past current */
        attr_handle++;
    }

    if (used == 0)
    {
        sm_log(SM_LOG_ERR, "%s()  attr not found  start_handle: 0x%04x  "
                   "end_handle: 0x%04x  Type: 0x%04x\r\n",
                   __func__, p_read_req->s_handle, p_read_req->e_handle,
                   p_read_req->uuid.uu.uuid16);

        app_bt_free_buffer(p_rsp);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Send the response */
    wiced_bt_gatt_server_send_read_by_type_rsp(conn_id, opcode, pair_len, used,
                                               p_rsp, (void *)app_bt_free_buffer);

    return WICED_BT_GATT_SUCCESS;
}

/**
 * Function Name:
 * app_bt_gatt_req_read_multi_handler
 *
 * Function Description:
 * @brief  Process write read multi request from peer device
 *
 * @param conn_id       Connection ID
 * @param opcode        Bluetooth LE GATT request type opcode
 * @param p_read_req    Pointer to read request containing the handle to read
 * @param len_requested length of data requested
 *
 * @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t app_bt_gatt_req_read_multi_handler(uint16_t conn_id,
                                                                 wiced_bt_gatt_opcode_t opcode,
                                                                 wiced_bt_gatt_read_multiple_req_t *p_read_req,
                                                                 uint16_t len_requested, 
                                                                 uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    uint8_t *p_rsp = app_bt_alloc_buffer(len_requested);
    int used = 0;
    int xx;
    uint16_t handle = wiced_bt_gatt_get_handle_from_stream(p_read_req->p_handle_stream, 0);
    *p_error_handle = handle;

    if (p_rsp == NULL)
    {
        sm_log(SM_LOG_ERR, "%s() No memory len_requested: %d!!\r\n",
                    __func__, len_requested);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Read by type returns all attributes of the specified type, between the
     * start and end handles */
    for (xx = 0; xx < p_read_req->num_handles; xx++)
    {
        handle = wiced_bt_gatt_get_handle_from_stream(p_read_req->p_handle_stream, xx);
        *p_error_handle = handle;
        if ((puAttribute = app_bt_find_by_handle(handle)) == NULL)
        {
            sm_log(SM_LOG_ERR, "%s()  no handle 0x%04x\r\n",
                       __func__, handle);
            app_bt_free_buffer(p_rsp);
            return WICED_BT_GATT_ERR_UNLIKELY;
        }

        {
            int filled = wiced_bt_gatt_put_read_multi_rsp_in_stream(opcode, p_rsp + used,
                                                                    len_requested - used,
                                                                    puAttribute->handle,
                                                                    puAttribute->cur_len,
                                                                    puAttribute->p_data);
            if (!filled)
            {
                break;
            }
            used += filled;
        }
    }

    if (used == 0)
    {
        sm_log(SM_LOG_ERR, "%s() no attr found\r\n", __func__);

        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Send the response */
    wiced_bt_gatt_server_send_read_multiple_rsp(conn_id, opcode, used, p_rsp,
                                                (void *)app_bt_free_buffer);

    return WICED_BT_GATT_SUCCESS;
}

/**
* Function Name: app_bt_management_callback
*
* Function Description:
* @brief
*  This is a Bluetooth stack event handler function to receive management events
*  from the Bluetooth stack and process as per the application.
*
* @param wiced_bt_management_evt_t       Bluetooth LE event code of one byte length
* @param wiced_bt_management_evt_data_t  Pointer to Bluetooth LE management event
*                                        structures
*
* @return wiced_result_t Error code from WICED_RESULT_LIST or BT_RESULT_LIST
*
*/

wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
                                       wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t result  = WICED_BT_ERROR;
    cy_rslt_t rslt;
    // wiced_bt_device_address_t bda = {0};
    wiced_bt_ble_advert_mode_t *p_adv_mode = NULL;
    wiced_bt_dev_encryption_status_t *p_status = NULL;

    sm_log(SM_LOG_INFO, "\r\n%s() Event: (%d) %s\r\n",
           __func__, event, get_bt_event_name(event));

    switch (event)
    {
        case BTM_ENABLED_EVT:
        {
            app_kv_store_test();//KV test
            /* Bluetooth Controller and Host Stack Enabled */
            if (WICED_BT_SUCCESS == p_event_data->enabled.status)
            {
                /* Perform application-specific initialization */
                app_bt_init();
                result = WICED_BT_SUCCESS;
            }
            else
            {
                sm_log(SM_LOG_ERR, "Failed to initialize Bluetooth controller and stack \r\n");
            }
        }
        break;
        case BTM_USER_CONFIRMATION_REQUEST_EVT:
        {
            sm_log(SM_LOG_NOTICE,"* TM_USER_CONFIRMATION_REQUEST_EVT: Numeric_value = %"PRIu32" *\r",
                p_event_data->user_confirmation_request.numeric_value);

            wiced_bt_dev_confirm_req_reply(WICED_BT_SUCCESS, p_event_data->user_confirmation_request.bd_addr);
            result = WICED_BT_SUCCESS;
        }
        break;
        case BTM_PASSKEY_NOTIFICATION_EVT:
        {
            sm_log(SM_LOG_NOTICE,"PassKey Notification from BDA: ");
            sm_log(SM_LOG_NOTICE,"PassKey: %"PRIu32" \n", p_event_data->user_passkey_notification.passkey );
            result = WICED_BT_SUCCESS;
        }
        break;
        case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
        {
            sm_log(SM_LOG_NOTICE, "  BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT\r\n");
            p_event_data->pairing_io_capabilities_ble_request.local_io_cap = BTM_IO_CAPABILITIES_NONE;
            p_event_data->pairing_io_capabilities_ble_request.oob_data = BTM_OOB_NONE;
            p_event_data->pairing_io_capabilities_ble_request.auth_req = BTM_LE_AUTH_REQ_BOND | BTM_LE_AUTH_REQ_MITM;
            p_event_data->pairing_io_capabilities_ble_request.max_key_size = 0x10;
            p_event_data->pairing_io_capabilities_ble_request.init_keys = BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
            p_event_data->pairing_io_capabilities_ble_request.resp_keys = BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
            result = WICED_BT_SUCCESS;
        }
        break;
        case BTM_PAIRING_COMPLETE_EVT:
        {
            if (p_event_data->pairing_complete.pairing_complete_info.ble.reason == SMP_SUCCESS)
            {
                bt_adv_set_en(1);//paring successful ,need to setting "adv enable" on startup

                if(TRUE == pairing_mode)
                {
                    /* setting pari event*/
                    set_system_external_even(EXT_EVENT_PARI_OK);
                }

                sm_log(SM_LOG_NOTICE, "  Pairing Complete: SMP_SUCCESS\r\n");
                /* Update Num of bonded devices and next free slot in slot data*/
                rslt = app_bt_update_slot_data();

                result = WICED_BT_SUCCESS;
            }
            else if (p_event_data->pairing_complete.pairing_complete_info.ble.reason == SMP_PAIR_NOT_SUPPORT)
            {
                /* setting pair event*/
                set_system_external_even(EXT_EVENT_PARI_FAIL);

                if (app_server_context.bt_conn_id != 0)
                {
                    wiced_bt_gatt_disconnect(app_server_context.bt_conn_id);
                }

                sm_log(SM_LOG_NOTICE, "  Pairing Failure, reason: SMP_PAIR_NOT_SUPPORT(%d) ",
                    p_event_data->pairing_complete.pairing_complete_info.ble.reason);

                result = WICED_BT_ERROR;
            }
            else
            {   
                /* setting pair event*/
                set_system_external_even(EXT_EVENT_PARI_FAIL);

                if (app_server_context.bt_conn_id != 0)
                {
                    wiced_bt_gatt_disconnect(app_server_context.bt_conn_id);
                }
                
                sm_log(SM_LOG_NOTICE, "  Pairing Failure, reason: (%d) ",
                    p_event_data->pairing_complete.pairing_complete_info.ble.reason);

                result = WICED_BT_ERROR;
            }
        }
        break;

        case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
            /* save device keys to NVRAM */
            rslt = app_bt_save_device_link_keys(&(p_event_data->paired_device_link_keys_update));
            if (CY_RSLT_SUCCESS == rslt)
            {
                sm_log(SM_LOG_INFO, "Successfully Bonded to ");
                print_bd_address(p_event_data->paired_device_link_keys_update.bd_addr);
                memcpy(cy_bt_remote_address, (uint8_t *)p_event_data->paired_device_link_keys_update.bd_addr, 6);//when update
                sm_log(SM_LOG_INFO, "\n paired_device_link_keys_update.key_data(%d):", sizeof(wiced_bt_device_sec_keys_t));
                print_array(&(p_event_data->paired_device_link_keys_update.key_data), sizeof(wiced_bt_device_sec_keys_t));
            }
            else
            {
                sm_log(SM_LOG_INFO, "Failed to bond! \n");
            }
            break;

        case  BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
            /* Paired Device Link Keys Request */
            sm_log(SM_LOG_INFO, "Paired Device Link keys Request Event for device: ");
            print_bd_address((uint8_t *)(p_event_data->paired_device_link_keys_request.bd_addr));

            /* Need to search to see if the BD_ADDR we are
             * looking for is in NVRAM. If not, we return WICED_BT_ERROR
             * and the stack will generate keys and will then call
             * BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT so that they
             * can be stored
             */

            /* Assume the device won't be found.
             * If it is, we will set this back to WICED_BT_SUCCESS */
            if(pairing_mode == TRUE) //in pairing mode, we need to resolve the RPA to find the device
            {
            	bondindex = app_bt_find_device_in_flash_RPA(p_event_data->paired_device_link_keys_request.bd_addr);//

				if(BOND_INDEX_MAX > bondindex)
				{
					/* Copy the keys to where the stack wants it */
					memcpy(&(p_event_data->paired_device_link_keys_request),
						   &bond_info.link_keys[bondindex],
						   sizeof(wiced_bt_device_link_keys_t));

					sm_log(SM_LOG_INFO, "Device Link Keys found in the database! \n");
					result = WICED_BT_SUCCESS;
				}
				else
				{
					sm_log(SM_LOG_INFO, "Device Link Keys not found in the database! \n");
					result = WICED_BT_ERROR;
				}
            }
            else //in non-pairing mode, use the addr comparing method to find the device
            {
				bondindex = app_bt_find_device_in_flash(p_event_data->paired_device_link_keys_request.bd_addr);
				if(BOND_INDEX_MAX > bondindex)
				{
					/* Copy the keys to where the stack wants it */
					memcpy(&(p_event_data->paired_device_link_keys_request),
						   &bond_info.link_keys[bondindex],
						   sizeof(wiced_bt_device_link_keys_t));
					result = WICED_BT_SUCCESS;
				}
				else
				{
					sm_log(SM_LOG_INFO, "Device Link Keys not found in the database! \n");
					result = WICED_BT_ERROR;
				}
            }
            break;

        case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
            /* Update of local privacy keys - save to NVRAM */
            rslt = app_bt_save_local_identity_key(p_event_data->local_identity_keys_update);
            if (CY_RSLT_SUCCESS != rslt)
            {
                result = WICED_BT_ERROR;
            }
            break;

        case  BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
            app_kv_store_init();
            /* Read Local Identity Resolution Keys if present in NVRAM*/
            rslt = app_bt_read_local_identity_keys();
            if(CY_RSLT_SUCCESS == rslt)
            {
                memcpy(&(p_event_data->local_identity_keys_request),
                       &(identity_keys), sizeof(wiced_bt_local_identity_keys_t));
                print_array(&identity_keys, sizeof(wiced_bt_local_identity_keys_t));
                result = WICED_BT_SUCCESS;
            }
            else
            {
                result = WICED_BT_ERROR;
            }
            break;

        case BTM_ENCRYPTION_STATUS_EVT:
        {
            sm_log(SM_LOG_NOTICE, "  Encryption Status Event for : ");

            // result = WICED_BT_SUCCESS;
            p_status = &p_event_data->encryption_status;
            sm_log(SM_LOG_INFO, "Encryption status changed, BDA:");
            print_bd_address(p_status->bd_addr);
            sm_log(SM_LOG_NOTICE, "  res: %d \r\n", p_status->result);

            /* Check and retreive the index of the bond data of the device that
             * got connected */
            /* This call will return BOND_INDEX_MAX if the device is not found */
            bondindex = app_bt_find_device_in_flash(p_event_data->encryption_status.bd_addr);
            if(bondindex < BOND_INDEX_MAX)
            {
                app_bt_restore_bond_data();
                app_bt_restore_cccd();
                /* Set CCCD value from the value that was previously saved in the NVRAM */
                // app_hello_sensor_notify_client_char_config[0] = peer_cccd_data[bondindex];   //need to be update!
                sm_log(SM_LOG_INFO, "Bond info present in Flash for device: ");
                print_bd_address(p_event_data->encryption_status.bd_addr);
            }
            else
            {
                sm_log(SM_LOG_INFO, "No Bond info present in Flash for device: ");
                print_bd_address(p_event_data->encryption_status.bd_addr);
                bondindex=BOND_INDEX_MAX;//before is 0, modified to BOND_INDEX_MAX - 20250106.101355
            }
        }
        break;
        case BTM_SECURITY_REQUEST_EVT:
        {
            sm_log(SM_LOG_NOTICE, "  BTM_SECURITY_REQUEST_EVT\r\n");
            wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr,
                                        WICED_BT_SUCCESS);
            result = WICED_BT_SUCCESS;
        }
        break;
        case BTM_BLE_CONNECTION_PARAM_UPDATE:
        {
            sm_log(SM_LOG_NOTICE, "BTM_BLE_CONNECTION_PARAM_UPDATE \r\n");
            sm_log(SM_LOG_NOTICE, "ble_connection_param_update.bd_addr: ");
            print_bd_address(p_event_data->ble_connection_param_update.bd_addr);
            sm_log(SM_LOG_NOTICE, "ble_connection_param_update.conn_interval       : %d\r\n",
                    p_event_data->ble_connection_param_update.conn_interval);
            sm_log(SM_LOG_NOTICE, "ble_connection_param_update.conn_latency        : %d\r\n",
                    p_event_data->ble_connection_param_update.conn_latency);
            sm_log(SM_LOG_NOTICE, "ble_connection_param_update.supervision_timeout : %d\r\n",
                    p_event_data->ble_connection_param_update.supervision_timeout);

            #ifdef GATT_CONGESTED_WORKAROUND
            bGattCongestedTimeout = p_event_data->ble_connection_param_update.supervision_timeout*10 + 3000;//uint:10ms , == suervision timeout + 3S!
            #endif

            sm_log(SM_LOG_NOTICE, "ble_connection_param_update.status              : %d\r\n\n",
                    p_event_data->ble_connection_param_update.status);
            result = WICED_BT_SUCCESS;
        }
        break;
        case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
        {
            /* Advertisement State Changed */
            p_adv_mode = &p_event_data->ble_advert_state_changed;
            sm_log(SM_LOG_NOTICE, "Advertisement State Change: %s\r\n",
                    get_bt_advert_mode_name(*p_adv_mode));

            if (BTM_BLE_ADVERT_OFF == *p_adv_mode)
            {
                /* Advertisement Stopped */
                sm_log(SM_LOG_NOTICE, "Advertisement stopped\r\n");

                /* Check connection status after advertisement stops */
                if (app_server_context.bt_conn_id == 0)
                {
                    app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_OFF;
                }
                else
                {
                    app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_ON;
                }
            }
            else
            {
                /* Advertisement Started */
                sm_log(SM_LOG_NOTICE, "Advertisement started\r\n");
                app_bt_adv_conn_state = APP_BT_ADV_ON_CONN_OFF;
            }
            result = WICED_BT_SUCCESS;
        }
        break;
        case BTM_BLE_PHY_UPDATE_EVT:
        {
            /* Print the updated BLE physical link*/
            sm_log(SM_LOG_NOTICE, "Selected TX PHY - %dM\n Selected RX PHY - %dM\n",
                    p_event_data->ble_phy_update_event.tx_phy,
                    p_event_data->ble_phy_update_event.rx_phy);
        }
        break;
        case BTM_BLE_DATA_LENGTH_UPDATE_EVENT:
        {
            /* Print the updated BLE physical link LLData PDU*/
            sm_log(SM_LOG_NOTICE, "Selected TX LLData PDU Size- %d bytes, Max_time- %d\n Selected RX LLData PDU Size- %d bytes, Max_time- %d\n",
                    p_event_data->ble_data_length_update_event.max_tx_octets,
                    p_event_data->ble_data_length_update_event.max_tx_time,
                    p_event_data->ble_data_length_update_event.max_rx_octets,
                    p_event_data->ble_data_length_update_event.max_rx_time);
#if 0
            /*** The value of connSlaveLatency should not cause a Supervision Timeout (see
                 Section 4.5.2). connSlaveLatency shall be an integer in the range 0 to
                 ((connSupervisionTimeout / (connInterval*2)) - 1). refer to spec 5.2
             */
			/*Variable to store connection parameter status*/
            wiced_bool_t conn_param_status = FALSE;
			
            /* Send connection interval update if required */
            conn_param_status = wiced_bt_l2cap_update_ble_conn_params(cy_bt_remote_address,
                                                    40,
                                                    40,
                                                    0,
                                                    1000);// (5000/(50*2)-1 == 49 
            if(TRUE == conn_param_status)
            {
                sm_log(SM_LOG_NOTICE, "Connection parameters update has started \n");
            }
            else
            {
                sm_log(SM_LOG_NOTICE, "Failed to send connection parameter update\n");
            }
#endif
        }
        break;
        default:
        {
            sm_log(SM_LOG_INFO, "Unhandled Bluetooth Management Event: 0x%x %s\r\n",
                    event, get_bt_event_name(event));
        }
        break;
    }
    return result;
}

/**
*  Function Name:
*  app_bt_init
*
*  Function Description:
*  @brief  This function handles application level initialization tasks and is
*          called from the BT management callback once the Bluetooth LE stack enabled event
*          (BTM_ENABLED_EVT) is triggered This function is executed in the BTM_ENABLED_EVT
*           management callback.
*
*  @param void
*
*  @return wiced_result_t WICED_SUCCESS or WICED_failure
*/
static void app_bt_init(void)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;

#ifdef GATT_CONGESTED_WORKAROUND
    cy_rtos_semaphore_init(&gatt_congested, GATT_CONGESTED_MAX_NUM, 0);
    wiced_bt_dev_register_hci_trace(hci_trace_cback);
#endif

    /* Allow peer to pair */
    wiced_bt_set_pairable_mode(WICED_TRUE, FALSE);

    /* Set Advertisement Data */
    wiced_bt_ble_set_raw_advertisement_data(CY_BT_ADV_PACKET_DATA_SIZE,
                                           cy_bt_adv_packet_data);
                                           
    /* Register with BT stack to receive GATT callback */
    status = wiced_bt_gatt_register(app_bt_gatt_event_callback);
    sm_log(SM_LOG_NOTICE, "GATT event Handler registration status: %s \r\n",
             get_bt_gatt_status_name(status));

    /* Initialize GATT Database */
    status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);
    sm_log(SM_LOG_NOTICE, "GATT database initialization status: %s \r\n",
             get_bt_gatt_status_name(status));

    /* Update the adv/conn state */
    app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_OFF;

#if BT_ADV_START
    app_bt_adv_start();
#endif

    sm_log(SM_LOG_NOTICE,"***********************************************\r\n");
    sm_log(SM_LOG_NOTICE,"**Discover device with \"%s\" name*\r\n", cy_bt_scan_rsp_packet_data[0].p_data);
    sm_log(SM_LOG_NOTICE,"***********************************************\r\n\n");
}

// 协议获取 BLE MAC 地址
bool procotol_get_ble_mac(uint8_t* rValue) 
{
    wiced_bt_device_address_t bt_mac_addr = {0};
    wiced_bt_dev_read_local_addr(bt_mac_addr);
    memcpy(rValue,bt_mac_addr,6);

    return true;
}

/**
*  Function Name:
*  procotol_ble_opt
*
*  Function Description:
*  @brief  This function be called to open or close pairing_mode!
*
*  @param void
*
*  @return true / false
*/  
bool procotol_ble_opt(uint8_t opt)
{
    wiced_result_t result;
    
#ifndef  DEF_BLE_EN
            return false;
#endif
    if(1 == opt)
    { 
        //如果建立了链接 先断开链接，然后再关闭广播，否则直接关闭广播
        if(app_server_context.bt_conn_id)
        {
            sm_log(SM_LOG_INFO,"Connected, we don't need to start advertisement.\n");
            return true;
        } 

        result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
        if (result != WICED_BT_SUCCESS)
        {    sm_log(SM_LOG_INFO,"Start advertisement failed: %d\n", result);}
        else 
        {    sm_log(SM_LOG_INFO,"Start advertisement successful!\n");}

    }
    else if(0 == opt)
    {
        //如果建立了链接 先断开链接，然后再关闭广播
        if(app_server_context.bt_conn_id)
        {
            wiced_bt_gatt_disconnect(app_server_context.bt_conn_id);
        } 

        cy_rtos_delay_milliseconds(1000);//bug1989000

        sm_log(SM_LOG_INFO,"Disconnect ,and Stop advertisement...\n");

        result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF, 0, NULL);//stop adv
        if (result != WICED_BT_SUCCESS)
            sm_log(SM_LOG_INFO,"Stop advertisement failed: %d\n", result);
    }
    else if(0xFD == opt)
    {
        sm_log(SM_LOG_INFO, "%s ,opt: 0x%02X\r\n", __FUNCTION__, opt);
        #ifdef GATT_CONGESTED_WORKAROUND
        bCongestedRebootCnt = CONGESTED_REBOOT_CNT_MAX;
        #endif
    }
    else if(0xFF == opt)
    {
    #define TASKs_LIST_LOG_SIZE  512
        char CPU_RunInfo[TASKs_LIST_LOG_SIZE] = {0}; //保存任务运行时间信息

        vTaskList(CPU_RunInfo); //获取任务运行时间信息

        sm_log(SM_LOG_INFO, "TaskName       State    Prio  LeftStack  TaskSeqNum\r\n");
        sm_log(SM_LOG_INFO, "-----------------------------------------------------------\r\n");
        sm_log(SM_LOG_INFO, "%s", CPU_RunInfo);
        cy_rtos_delay_milliseconds(100);
        sm_log(SM_LOG_INFO, "-----------------------------------------------------------\r\n");
        cy_rtos_delay_milliseconds(100);
    }


    return true; 
}

#ifdef GATT_CONGESTED_WORKAROUND
static void hci_trace_cback(wiced_bt_hci_trace_type_t type,
                     uint16_t length, uint8_t* p_data)
{
	if(HCI_TRACE_EVENT == type)
	{
		if(p_data[0] == 0x13)//0x13 indicate that number_of_sending_completed_packets event!
		{
            bCongestedRebootCnt = 0;
			cy_rtos_semaphore_set(&gatt_congested);
		}
	}
}
#endif
