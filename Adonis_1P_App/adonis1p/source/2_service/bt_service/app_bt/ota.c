/******************************************************************************
* File Name:   ota.c
*
* Description: This file handles ota events and notifications.
*
*
* Related Document: See Readme.md
*
********************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/
#include "wiced_bt_gatt.h"
#include "cycfg_gatt_db.h"
#include "sm_log.h"
#include "ota.h"
#include "cyabs_rtos.h"

#include "ota_context.h"
#if (OTA_SERVICE_SUPPORT == 1)
/* OTA related header files */
#include "cy_ota_api.h"
#endif //#if (OTA_SERVICE_SUPPORT == 1)


/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

#include "protocol_usb.h"
#include "platform_io.h"

//service
#include "system_status.h"
#include "system_interaction_logic.h"
#include "app_bt_char.h"

/******************************************************
 *               Function Declarations
 ******************************************************/

cy_rslt_t              app_bt_ota_init                        (app_context_t *ota);

static bool ota_download_verify_flag = false;

/*******************************************************************************
*        Variable Definitions
*******************************************************************************/
/**
 * @brief App context parameters
 */
app_context_t app_server_context ={0};

#if (OTA_SERVICE_SUPPORT == 1)
/**
 * @brief Agent parameters for OTA
 */
cy_ota_agent_params_t ota_agent_params = {0};

/**
 * @brief network parameters for OTA
 */
cy_ota_network_params_t ota_network_params = {CY_OTA_CONNECTION_UNKNOWN};

/*
 * Function Name:
 * app_bt_ota_write_handler
 *
 * Function Description:
 * @brief  The function is invoked when GATTS_REQ_TYPE_WRITE is received from the
 *         client device and invokes GATT Server Event Callback function. This
 *         handles OTA related "Write Requests" received from Client device.
 *
 * @param p_write_req   Pointer to BLE GATT write request
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status
 */
wiced_bt_gatt_status_t app_bt_ota_write_handler(wiced_bt_gatt_event_data_t *p_data, 
                                                uint16_t *p_error_handle)
{
    wiced_bt_gatt_write_req_t *p_write_req = &p_data->attribute_request.data.write_req;
    cy_rslt_t cy_result;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    SysStatus_u cur_sysSta=0; 
    *p_error_handle = p_write_req->handle;

    CY_ASSERT(( NULL != p_data ) && (NULL != p_write_req));

    cy_ota_agent_state_t ota_lib_state;
    cy_ota_get_state(app_server_context.ota_context, &ota_lib_state);

    // sm_log(SM_LOG_DEBUG, "%s() p_write_req->handle: %d ota_cur_state: %d\r\n",
    //                     __func__, p_write_req->handle, ota_lib_state);

    static bool ui_loading_flag = 0;

    update_idl_delay_time();				// 阻止休眠

    /** When heated, do not upgrade **/
    cur_sysSta = get_system_status();
    if ((HEATTING_TEST == cur_sysSta)||
        (HEATTING_STANDARD == cur_sysSta)||
        (HEATTING_BOOST == cur_sysSta)||
        (HEATTING_CLEAN == cur_sysSta)||
        (HEAT_MODE_TEST_VOLTAGE == cur_sysSta)||
        (HEAT_MODE_TEST_POWER == cur_sysSta)||
        (HEAT_MODE_TEST_TEMP == cur_sysSta))
    {
        sm_log(SM_LOG_DEBUG, "%s It is currently heating and cannot perform OTA!\r\n", __func__);
        return WICED_BT_GATT_ERROR;
    }
    
    switch (p_write_req->handle)
    {
    case HDLD_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_CLIENT_CHAR_CONFIG:
        sm_log(SM_LOG_DEBUG, "%s() HDLD_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_CLIENT_CHAR_CONFIG\r\n",
                          __func__);
        /* Save Configuration descriptor in Application data structure (Notify & Indicate flags) */
        app_server_context.bt_ota_config_descriptor = p_write_req->p_val[0];
        sm_log(SM_LOG_NOTICE, "app_server_context.bt_ota_config_descriptor: %d %s\r\n",
                   app_server_context.bt_ota_config_descriptor,
        (app_server_context.bt_ota_config_descriptor == GATT_CLIENT_CONFIG_NOTIFICATION) ? "Notify" :
        (app_server_context.bt_ota_config_descriptor == GATT_CLIENT_CONFIG_INDICATION) ? "Indicate": "Unknown");

        return WICED_BT_GATT_SUCCESS;

    case HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE:
        sm_log(SM_LOG_DEBUG, "%s() HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE, COMMAND: %d\r\n", __func__, p_write_req->p_val[0]);
        switch (p_write_req->p_val[0])
        {
            case CY_OTA_UPGRADE_COMMAND_PREPARE_DOWNLOAD:
            {
                /* BT task Exit flag set */
                app_bt_char_bt_service_task_ntf_loop_exit_flag_set(1);

                if (CY_OTA_STATE_NOT_INITIALIZED == ota_lib_state)
                {
                    /* Call application-level OTA initialization (calls cy_ota_agent_start() ) */
                    cy_result = app_bt_ota_init(&app_server_context);
                    if (CY_RSLT_SUCCESS != cy_result)
                    {
                        sm_log(SM_LOG_ERR, "OTA initialization Failed - result: 0x%lx\r\n",
                                cy_result);
                        return WICED_BT_GATT_ERROR;
                    }
                }

                ui_loading_flag = 1;
                ota_download_verify_flag = false;

                cy_result = cy_ota_ble_download_prepare(app_server_context.ota_context,
                                                        app_server_context.bt_conn_id,
                                                        app_server_context.bt_ota_config_descriptor);
                if (CY_RSLT_SUCCESS != cy_result)
                {
                    sm_log(SM_LOG_ERR, "Download preparation Failed - result: 0x%lx, CY_RSLT_OTA_ERROR_BASE: 0x%lx , offset: 0x%lx\r\n", cy_result, CY_RSLT_OTA_ERROR_BASE, (cy_result-CY_RSLT_OTA_ERROR_BASE));
                    return WICED_BT_GATT_ERROR;
                }
                
                return WICED_BT_GATT_SUCCESS;
            }
                break;
            case CY_OTA_UPGRADE_COMMAND_DOWNLOAD:
            {
                /* let OTA lib know what is going on */
                sm_log(SM_LOG_DEBUG, "%s() HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_CONTROL_POINT_VALUE : CY_OTA_UPGRADE_COMMAND_DOWNLOAD\r\n", __func__);
                cy_result = cy_ota_ble_download(app_server_context.ota_context, p_data,
                                                app_server_context.bt_conn_id,
                                                app_server_context.bt_ota_config_descriptor);
                if (CY_RSLT_SUCCESS != cy_result)
                {
                    sm_log(SM_LOG_ERR, "Download Failed - result: 0x%lx\r\n", cy_result);
                    return WICED_BT_GATT_ERROR;
                }
                return WICED_BT_GATT_SUCCESS;
            }
                break;
            case CY_OTA_UPGRADE_COMMAND_VERIFY:
            {
                cy_result = cy_ota_ble_download_verify(app_server_context.ota_context, p_data,
                                                    app_server_context.bt_conn_id);
                if (CY_RSLT_SUCCESS != cy_result)
                {
                    ota_download_verify_flag = false;
                    sm_log(SM_LOG_ERR, "verification and Indication failed: 0x%d\r\n",
                            cy_result);
                    return WICED_BT_GATT_ERROR;
                }
                else
                {
                    ota_download_verify_flag = true;
                    /***
                     * Added by vincent.he, 对 magic && version_control 存储区域更新一遍。
                     */
                    int flashStatus = app_uart_write_app_magic();//app_uart_write_magic();
                    sm_log(SM_LOG_ERR, "magic_write: %d, verification successfully!\r\n", flashStatus);
                    cy_rtos_delay_milliseconds(50);
                }
                return status;
            }
                break;
            case CY_OTA_UPGRADE_COMMAND_ABORT: 
            {              
                if (CY_OTA_STATE_NOT_INITIALIZED != ota_lib_state) //soloving not initazlized OTA agent and active ota abort operation bugs.
                {
                    cy_result = cy_ota_ble_download_abort(app_server_context.ota_context); 
                    
                    /* UI display loading stop*/
                    set_system_external_even(EXT_EVENT_LOADING_STOP);
                    cy_rtos_delay_milliseconds(500);
                    /* restore key detect*/
                    set_update_ui_timer_cnt(10);
                    /* restore ntf_loop_exit_flag*/
                    app_bt_char_bt_service_task_ntf_loop_exit_flag_set(0);
                }

                return WICED_BT_GATT_SUCCESS;
            }
                break;
            default:
            {
                sm_log(SM_LOG_ERR, "Not found the COMMAND: %d\r\n", p_write_req->p_val[0]);
            }
                break;
        }

    case HDLC_OTA_FW_UPGRADE_SERVICE_OTA_UPGRADE_DATA_VALUE:
        cy_result = cy_ota_ble_download_write(app_server_context.ota_context, p_data);
        if (cy_result == CY_RSLT_SUCCESS) //bug1985800
        {
            if (ui_loading_flag == 1)//flag
            {
                ui_loading_flag = 0;
                /* UI display start and loading*/
                set_system_external_even(EXT_EVENT_LOADING);
                cy_rtos_delay_milliseconds(500);
            }
        }

        sm_log(SM_LOG_DEBUG, ".");
        set_update_ui_timer_cnt(5000*6*3);      // 阻止按键触发事件
        cy_rtos_delay_milliseconds(5);
        
        if ((SUBSTATUS_IDLE == get_subSystem_status()) || (cy_result == CY_RSLT_OTA_ERROR_BLE_GATT))  //bug1985800  //set_subSystem_status(SUBSTATUS_IDLE); //press button cancle OTA
        {
            cy_result = cy_ota_ble_download_abort(app_server_context.ota_context); 
            /* UI display loading stop*/
            set_system_external_even(EXT_EVENT_LOADING_STOP);
            /* restore key detect*/
            set_update_ui_timer_cnt(200);
            /* restore ntf_loop_exit_flag*/
            app_bt_char_bt_service_task_ntf_loop_exit_flag_set(0);

            return WICED_BT_GATT_ERROR;
        }
        return (cy_result == CY_RSLT_SUCCESS) ? WICED_BT_GATT_SUCCESS : WICED_BT_GATT_ERROR;

    default:
        sm_log(SM_LOG_DEBUG,"UNHANDLED OTA WRITE \r\n");
        break;
    }
    return WICED_BT_GATT_REQ_NOT_SUPPORTED;
}

/**
 * Function Name:
 * app_bt_ota_init
 *
 * Function Description :
 * @brief Initialize and start the OTA update
 *
 * @param app_context  pointer to Application context
 *
 * @return cy_rslt_t Result of initialization
 */
cy_rslt_t app_bt_ota_init(app_context_t *app_context)
{
    cy_rslt_t cy_result;

    if (app_context == NULL || app_context->tag != OTA_APP_TAG_VALID)
    {
        return CY_RSLT_OTA_ERROR_BADARG;
    }

    memset(&ota_network_params, 0, sizeof(ota_network_params));
    memset(&ota_agent_params, 0, sizeof(ota_agent_params));

    /* Common Network Parameters */
    ota_network_params.initial_connection = app_context->connection_type;

    /* OTA Agent parameters - used for ALL transport types*/
    /* Validate after reboot so that we can test revert */
    ota_agent_params.validate_after_reboot = 1;

    cy_result = cy_ota_agent_start(&ota_network_params, &ota_agent_params,
                                &app_server_context.ota_context);
    if (CY_RSLT_SUCCESS != cy_result)
    {
        sm_log(SM_LOG_ERR, "cy_ota_agent_start() Failed - result: 0x%lx\r\n",
                   cy_result);
        while (true)
        {
            cy_rtos_delay_milliseconds(10);
        }
    }
    sm_log(SM_LOG_NOTICE, "OTA Agent Started \r\n");

    return cy_result;
}

#endif //#if (OTA_SERVICE_SUPPORT == 1)

/**
 * Function Name
 * app_bt_initialize_default_values
 *
 * Function Description:
 * @brief  Initialize default context values
 *
 * @param  None
 * @return void
 */
void app_bt_initialize_default_values(void)
{

    app_server_context.tag = OTA_APP_TAG_VALID;
#if (OTA_SERVICE_SUPPORT == 1)
    app_server_context.connection_type = CY_OTA_CONNECTION_BLE;
#endif
    app_server_context.bt_conn_id = 0;
    app_server_context.reboot_at_end = 1;
}

/**
 * Function Name:
 * app_bt_ota_upgrade_is_running
 *
 * Function Description :
 * @brief get the OTA update state whether is running
 *
 * @param void
 *
 * @return bool true /false 
 */
bool app_bt_ota_upgrade_is_running(void)
{
    bool status = true;


    cy_ota_agent_state_t ota_lib_state;
    cy_ota_get_state(app_server_context.ota_context, &ota_lib_state);
    
    
    // sm_log(SM_LOG_ERR, "%s cy_ota_get_state: %d\r\n", __FUNCTION__, ota_lib_state);

    if ((CY_OTA_STATE_NOT_INITIALIZED == ota_lib_state)||(CY_OTA_STATE_NOT_INITIALIZED == ota_lib_state))//0/4
    {
        status = false;
    }

    return status;
}

/**
 * Function Name:
 * app_bt_gatt_is_connected
 *
 * Function Description :
 * @brief get the OTA update state whether is running
 *
 * @param void
 *
 * @return bool true /false 
 */
bool app_bt_gatt_is_connected(void)
{
    bool status = true;

    // sm_log(SM_LOG_ERR, "%s cy_ota_get_state: %d\r\n", __FUNCTION__, ota_lib_state);

    if (app_server_context.bt_conn_id == 0)
    {
        status = false;
    }

    return status;
}

/**
 * Function Name:
 * app_bt_gatt_ota_download_verify_is_succeed
 *
 * Function Description :
 * @brief get the OTA update state whether is running
 *
 * @param void
 *
 * @return bool true /false 
 */
bool app_bt_gatt_ota_download_verify_is_succeed(void)
{
    // sm_log(SM_LOG_ERR, "%s cy_ota_get_state: %d\r\n", __FUNCTION__, ota_lib_state);

    return ota_download_verify_flag;
}
