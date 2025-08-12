/***************************************************************************//**
* \file cyhal_ethernet.c
*
* \brief
* Provides a high level interface for interacting with the Infineon Ethernet.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low level functions
* can be used directly.
*
********************************************************************************
* \copyright
* Copyright 2023 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "cyhal_ethernet.h"
#include "cyhal_system.h"
#include "cyhal_gpio.h"
#include "cyhal_clock.h"
#include "cyhal_irq_impl.h"
#include "cyhal_utils_impl.h"
#include "cyhal_utils.h"
#include "cyhal_hwmgr.h"

#include <string.h>

#if (CYHAL_DRIVER_AVAILABLE_ETHERNET)

#if defined(__cplusplus)
extern "C" {
#endif

/*********************************************** MACRO HELPERS *********************************************************/

/* Macro to work with common for all modes pins */
#define _CYHAL_ETH_GET_PIN_GENERIC(pin_name, mode)\
                            _cyhal_eth_is_mode_mii(mode)   ? ethernet_pins->eth_type_mii.pin_name :\
                            _cyhal_eth_is_mode_gmii(mode)  ? ethernet_pins->eth_type_gmii.pin_name :\
                            _cyhal_eth_is_mode_rgmii(mode) ? ethernet_pins->eth_type_rgmii.pin_name :\
                            _cyhal_eth_is_mode_rmii(mode)  ? ethernet_pins->eth_type_rmii.pin_name : NC
#define _CYHAL_ETH_STORE_PIN_GENERIC(pin_name, mode, obj_pins, value)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { obj_pins.mii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_gmii(mode)) { obj_pins.gmii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_rgmii(mode)) { obj_pins.rgmii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_rmii(mode)) { obj_pins.rmii.pin_name = value; }\
                            else { CY_ASSERT(false); }\
                            } while(0)
#define _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(pin_name, mode, obj_pins)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.mii.pin_name)); }\
                            else if (_cyhal_eth_is_mode_gmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.gmii.pin_name)); }\
                            else if(_cyhal_eth_is_mode_rgmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.rgmii.pin_name)); }\
                            else if(_cyhal_eth_is_mode_rmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.rmii.pin_name)); }\
                            else { CY_ASSERT(false); }\
                            } while(0)

/* Macro to work with common for Mii, GMii and RGMii modes pins */
#define _CYHAL_ETH_GET_PIN_SET_M_GM_RGM(pin_name, mode)\
                            _cyhal_eth_is_mode_mii(mode)   ? ethernet_pins->eth_type_mii.pin_name :\
                            _cyhal_eth_is_mode_gmii(mode)  ? ethernet_pins->eth_type_gmii.pin_name :\
                            _cyhal_eth_is_mode_rgmii(mode) ? ethernet_pins->eth_type_rgmii.pin_name : NC
#define _CYHAL_ETH_STORE_PIN_SET_M_GM_RGM(pin_name, mode, obj_pins, value)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { obj_pins.mii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_gmii(mode)) { obj_pins.gmii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_rgmii(mode)) { obj_pins.rgmii.pin_name = value; }\
                            else { CY_ASSERT(false); }\
                            } while(0)
#define _CYHAL_ETH_FREE_IF_USED_PIN_M_GM_RGM(pin_name, mode, obj_pins)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.mii.pin_name)); }\
                            else if (_cyhal_eth_is_mode_gmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.gmii.pin_name)); }\
                            else if(_cyhal_eth_is_mode_rgmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.rgmii.pin_name)); }\
                            else { CY_ASSERT(false); }\
                            } while(0)

/* Macro to work with common for Mii, RMii and GMii modes pins */
#define _CYHAL_ETH_GET_PIN_SET_M_RM_GM(pin_name, mode)\
                            _cyhal_eth_is_mode_mii(mode)   ? ethernet_pins->eth_type_mii.pin_name :\
                            _cyhal_eth_is_mode_rmii(mode) ? ethernet_pins->eth_type_rmii.pin_name :\
                            _cyhal_eth_is_mode_gmii(mode)  ? ethernet_pins->eth_type_gmii.pin_name : NC
#define _CYHAL_ETH_STORE_PIN_SET_M_RM_GM(pin_name, mode, obj_pins, value)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { obj_pins.mii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_rmii(mode)) { obj_pins.rmii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_gmii(mode)) { obj_pins.gmii.pin_name = value; }\
                            else { CY_ASSERT(false); }\
                            } while(0)
#define _CYHAL_ETH_FREE_IF_USED_PIN_M_RM_GM(pin_name, mode, obj_pins)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.mii.pin_name)); }\
                            else if(_cyhal_eth_is_mode_rmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.rmii.pin_name)); }\
                            else if (_cyhal_eth_is_mode_gmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.gmii.pin_name)); }\
                            else { CY_ASSERT(false); }\
                            } while(0)

/* Macro to work with common for Mii and GMii modes pins */
#define _CYHAL_ETH_GET_PIN_SET_M_GM(pin_name, mode)\
                            _cyhal_eth_is_mode_mii(mode)   ? ethernet_pins->eth_type_mii.pin_name :\
                            _cyhal_eth_is_mode_gmii(mode)  ? ethernet_pins->eth_type_gmii.pin_name : NC
#define _CYHAL_ETH_STORE_PIN_SET_M_GM(pin_name, mode, obj_pins, value)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { obj_pins.mii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_gmii(mode)) { obj_pins.gmii.pin_name = value; }\
                            else { CY_ASSERT(false); }\
                            } while(0)
#define _CYHAL_ETH_FREE_IF_USED_PIN_M_GM(pin_name, mode, obj_pins)\
                            do {\
                            if (_cyhal_eth_is_mode_mii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.mii.pin_name)); }\
                            else if (_cyhal_eth_is_mode_gmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.gmii.pin_name)); }\
                            else { CY_ASSERT(false); }\
                            } while(0)

/* Macro to work with common for GMii and RGMii modes pins */
#define _CYHAL_ETH_GET_PIN_SET_GM_RGM(pin_name, mode)\
                            _cyhal_eth_is_mode_gmii(mode)  ? ethernet_pins->eth_type_gmii.pin_name :\
                            _cyhal_eth_is_mode_rgmii(mode) ? ethernet_pins->eth_type_rgmii.pin_name : NC
#define _CYHAL_ETH_STORE_PIN_SET_GM_RGM(pin_name, mode, obj_pins, value)\
                            do {\
                            if(_cyhal_eth_is_mode_gmii(mode)) { obj_pins.gmii.pin_name = value; }\
                            else if(_cyhal_eth_is_mode_rgmii(mode)) { obj_pins.rgmii.pin_name = value; }\
                            else { CY_ASSERT(false); }\
                            } while(0)
#define _CYHAL_ETH_FREE_IF_USED_PIN_GM_RGM(pin_name, mode, obj_pins)\
                            do {\
                            if (_cyhal_eth_is_mode_gmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.gmii.pin_name)); }\
                            else if(_cyhal_eth_is_mode_rgmii(mode)) { _cyhal_utils_release_if_used(&(obj_pins.rgmii.pin_name)); }\
                            else { CY_ASSERT(false); }\
                            } while(0)

#define _CYHAL_ETH_QUEUES_PER_INSTANCE  (3u)
#define _CYHAL_ETH_ARRAY_LEN(arr)       (sizeof(arr) / sizeof(arr[0]))

/*********************************************** INTERNAL DATA *********************************************************/
static ETH_Type *_CYHAL_ETH_BASE_ADDRESSES[CY_IP_MXETH_INSTANCES] = {
#if CY_IP_MXETH_INSTANCES > 0
    ETH0,
#endif
#if CY_IP_MXETH_INSTANCES > 1
    ETH1,
#endif
#if CY_IP_MXETH_INSTANCES > 2
#error "Unhandled number of Ethernet HW instances"
#endif
};

static en_clk_dst_t _cyhal_eth_pclks[CY_IP_MXETH_INSTANCES] = {
#if CY_IP_MXETH_INSTANCES > 0
    PCLK_ETH0_REF_CLK_INT
#endif
#if CY_IP_MXETH_INSTANCES > 1
#error "Unhandled number of Ethernet HW instances"
#endif
};

static const _cyhal_system_irq_t _cyhal_eth_irq_n[CY_IP_MXETH_INSTANCES * _CYHAL_ETH_QUEUES_PER_INSTANCE] = {
#if defined(COMPONENT_CAT1C)
#if CY_IP_MXETH_INSTANCES > 0
    eth_0_interrupt_eth_0_IRQn,
    eth_0_interrupt_eth_1_IRQn,
    eth_0_interrupt_eth_2_IRQn,
#endif
#if CY_IP_MXETH_INSTANCES > 1
    eth_1_interrupt_eth_0_IRQn,
    eth_1_interrupt_eth_1_IRQn,
    eth_1_interrupt_eth_2_IRQn,
#endif
#if CY_IP_MXETH_INSTANCES > 2
#error "Unhandled number of Ethernet HW instances"
#endif
#elif defined(COMPONENT_CAT1D)
#if CY_IP_MXETH_INSTANCES > 0
    eth_interrupt_eth_0_IRQn,
    eth_interrupt_eth_1_IRQn,
    eth_interrupt_eth_2_IRQn,
#else
#error "Unhandled number of Ethernet HW instances"
#endif
#else
#error "Unhandled device architecture"
#endif
};

static uint8_t _cyhal_eth_tsu_clock_hf_num =
#if defined(COMPONENT_CAT1C)
    5u
#elif defined(COMPONENT_CAT1D)
    6u
#endif
;

/* Block input clock should be not lower than 10 MHz according to IP Block documentation */
#define _CYHAL_ETH_MIN_HF_CLK_HZ        (_CYHAL_UTILS_MHZ_TO_HZ(10))

static cyhal_ethernet_t *_cyhal_eth_objects[CY_IP_MXETH_INSTANCES];
static bool _cyhal_eth_driver_initialized = false;

/* Standard Wrapper configuration   */
static const cy_stc_ethif_wrapper_config_t _cyhal_eth_def_wrapper_cfg = {
    /* Interface selection for ENET */
    .stcInterfaceSel = CY_ETHIF_CTL_MII_10,
    /* Reference clock selection */
    .bRefClockSource = CY_ETHIF_EXTERNAL_HSIO,
    /* Source clock divider */
    .u8RefClkDiv = 1
};

/* Default Timer register values   */
static cy_stc_ethif_1588_timer_val_t _cyhal_eth_1588_timer_value = {
    .secsUpper = 0,
    .secsLower = 0,
    .nanosecs = 0,
};

/** TSU configuration   */
static cy_stc_ethif_tsu_config_t _cyhal_eth_tsu_cfg = {
    /* start value for the counter */
    .pstcTimerValue    = &_cyhal_eth_1588_timer_value,
    /* Increment value for each clock cycle, will be calculated during initialization */
    .pstcTimerIncValue = NULL,
    /* If true, replace timestamp field in the 1588 header for TX Sync Frames with current TSU timer value */
    .bOneStepTxSyncEnable = true,
    .enTxDescStoreTimeStamp = CY_ETHIF_TX_TS_DISABLED,
    .enRxDescStoreTimeStamp = CY_ETHIF_RX_TS_DISABLED,
    .bStoreNSinRxDesc = false,
};

/* General Ethernet configuration  */
static const cy_stc_ethif_mac_config_t _cyhal_eth_def_pdl_config = {
    /* Interrupt enable  */
    .bintrEnable         = true,
    /* fixed burst length for DMA data transfers */
    .dmaDataBurstLen     = CY_ETHIF_DMA_DBUR_LEN_4,
    /* DMA config register bits 24, 25 & 26 */
    .u8dmaCfgFlags       = CY_ETHIF_CFG_DMA_FRCE_TX_BRST,
    /* divisor to generate MDC from pclk */
    .mdcPclkDiv          = CY_ETHIF_MDC_DIV_BY_48,
    /* enable discard of frames with length field error */
    .u8rxLenErrDisc      = 0,
    /* disable copying Rx pause frames to memory */
    .u8disCopyPause      = 0,
    /* Checksum for both Tx and Rx disabled    */
    .u8chkSumOffEn       = 0,
    /* Enable receive frame up to 1536 */
    .u8rx1536ByteEn      = 1,
    .u8rxJumboFrEn       = 0,
    .u8enRxBadPreamble   = 1,
    .u8ignoreIpgRxEr     = 0,
    .u8storeUdpTcpOffset = 0,
    /* Value must be > 0   */
    .u8aw2wMaxPipeline   = 2,
    /* Value must be > 0   */
    .u8ar2rMaxPipeline   = 2,
    .u8pfcMultiQuantum   = 0,
    /* wrapper cfg is being added later in code */
    .pstcWrapperConfig   = NULL,
    /* TSU settings    */
    .pstcTSUConfig       = &_cyhal_eth_tsu_cfg,
    /* Tx Q0 Enabled   */
    .btxq0enable         = 1,
    /* Tx Q1 Disabled  */
    .btxq1enable         = 0,
    /* Tx Q2 Disabled  */
    .btxq2enable         = 0,
    /* Rx Q0 Enabled   */
    .brxq0enable         = 1,
    /* Rx Q1 Disabled  */
    .brxq1enable         = 0,
    /* Rx Q2 Disabled  */
    .brxq2enable         = 0,
};

/** Interrupt configurations    */
static const cy_stc_ethif_intr_config_t _cyhal_eth_def_intr_config = {
    /** Time stamp unit time match event */
    .btsu_time_match        = 0,
    /** Wake on LAN event received */
    .bwol_rx                = 0,
    /** LPI indication status bit change received */
    .blpi_ch_rx             = 0,
    /** TSU seconds register increment */
    .btsu_sec_inc           = 1,
    /** PTP pdelay_resp frame transmitted */
    .bptp_tx_pdly_rsp       = 0,
    /** PTP pdelay_req frame transmitted */
    .bptp_tx_pdly_req       = 0,
    /** PTP pdelay_resp frame received */
    .bptp_rx_pdly_rsp       = 0,
    /** PTP pdelay_req frame received */
    .bptp_rx_pdly_req       = 0,
    /** PTP sync frame transmitted */
    .bptp_tx_sync           = 0,
    /** PTP delay_req frame transmitted */
    .bptp_tx_dly_req        = 0,
    /** PTP sync frame received */
    .bptp_rx_sync           = 0,
    /** PTP delay_req frame received */
    .bptp_rx_dly_req        = 0,
    /** External input interrupt detected */
    .bext_intr              = 0,
    /** Pause frame transmitted */
    .bpause_frame_tx        = 0,
    /** Pause time reaches zero or zeroq pause frame received */
    .bpause_time_zero       = 0,
    /** Pause frame with non-zero quantum received */
    .bpause_nz_qu_rx        = 0,
    /** DMA hresp not OK */
    .bhresp_not_ok          = 0,
    /** Rx overrun error */
    .brx_overrun            = 0,
    /** Link status change detected by PCS */
    .bpcs_link_change_det   = 0,
    /** Frame has been transmitted successfully */
    .btx_complete           = 1,
    /** Tx frame corruption */
    .btx_fr_corrupt         = 1,
    /** Retry limit exceeded or late collision */
    .btx_retry_ex_late_coll = 0,
    /** Tx underrun */
    .btx_underrun           = 1,
    /** Used bit set has been read in Tx descriptor list */
    .btx_used_read          = 0,
    /** Used bit set has been read in Rx descriptor list */
    .brx_used_read          = 0,
    /** Frame received successfully and stored */
    .brx_complete           = 1,
    /** Management Frame Sent */
    .bman_frame             = 0,
};

/*************************************** INTERNAL FUNCTIONS PROTOTYPES *************************************************/

static cyhal_ethernet_t *_cyhal_eth_get_irq_obj(void);
static cyhal_ethernet_t *_cyhal_eth_get_obj_by_base(ETH_Type *base);
static void _cyhal_eth_increment_rx_buff_index(size_t *buf_ptr);
static void _cyhal_eth_check_n_call_cb(cyhal_ethernet_t *obj, cyhal_ethernet_event_t event);

/************************************** Interrupt and Callbacks handlers ***********************************************/

static void _cyhal_eth_cb_rx_frame(ETH_Type *base, uint8_t *rx_buffer, uint32_t length)
{
    CY_ASSERT(NULL != base);
    CY_UNUSED_PARAM(rx_buffer);

    cyhal_ethernet_t *obj = _cyhal_eth_get_obj_by_base(base);
    if (NULL != obj)
    {
        /* Determine buff index based on rx_buffer array address */
        for (size_t buff_idx = 0; buff_idx < CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE; ++buff_idx)
        {
            if ((uint32_t)rx_buffer == (uint32_t)(&obj->rx_frames_buffs[buff_idx][0]))
            {
                obj->rx_frames_sizes[buff_idx] = (uint16_t)length;
            }
        }

        obj->buffs_occupied++;

        if (obj->buffs_occupied > CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE + 1)
        {
            /* Point, at which user didn't read any of CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE frames, that are stored in buffers.
            * In this case, consider current oldest frame lost and incrementing read pointer */
            _cyhal_eth_increment_rx_buff_index(&obj->rx_read_ptr);
        }

        _cyhal_eth_check_n_call_cb(obj, CYHAL_ETHERNET_RX_FRAME_EVENT);
    }
}

static void _cyhal_eth_cb_get_buff(ETH_Type *base, uint8_t **u8RxBuffer, uint32_t *u32Length)
{
    CY_ASSERT(NULL != base);
    CY_ASSERT(NULL != *u8RxBuffer);

    cyhal_ethernet_t *obj = _cyhal_eth_get_obj_by_base(base);
    if (NULL != obj)
    {
        *u8RxBuffer = &(obj->rx_frames_buffs[obj->rx_write_ptr][0]);
        *u32Length = CY_ETH_SIZE_MAX_FRAME;
        _cyhal_eth_increment_rx_buff_index(&obj->rx_write_ptr);
    }
}

static void _cyhal_eth_cb_tx_frame_failure(ETH_Type *base, uint8_t queue_index)
{
    CY_UNUSED_PARAMETER(queue_index);
    CY_ASSERT(NULL != base);

    cyhal_ethernet_t *obj = _cyhal_eth_get_obj_by_base(base);
    if (NULL != obj)
    {
        _cyhal_eth_check_n_call_cb(obj, CYHAL_ETHERNET_TX_ERR_OCCURED_EVENT);
    }
}

static void _cyhal_eth_cb_tx_frame_successful(ETH_Type *base, uint8_t queue_index)
{
    CY_UNUSED_PARAMETER(queue_index);
    CY_ASSERT(NULL != base);

    cyhal_ethernet_t *obj = _cyhal_eth_get_obj_by_base(base);
    if (NULL != obj)
    {
        _cyhal_eth_check_n_call_cb(obj, CYHAL_ETHERNET_TX_COMPLETE_EVENT);
    }
}

static void _cyhal_eth_cb_tsu_increment(ETH_Type *base)
{
    CY_UNUSED_PARAMETER(base);

    cyhal_ethernet_t *obj = _cyhal_eth_get_obj_by_base(base);
    if (NULL != obj)
    {
        _cyhal_eth_check_n_call_cb(obj, CYHAL_ETHERNET_TSU_SECOND_INC_EVENT);
    }
}

static volatile cyhal_ethernet_t* _cyhal_eth_irq_obj = NULL;

void _cyhal_eth_interrupt_handler(void)
{
    /* Save the old value and store it aftewards in case we get into a nested IRQ situation */
    /* Safe to cast away volatile because we don't expect this pointer to be changed while we're in here, they
     * just might change where the original pointer points */
    cyhal_ethernet_t* old_irq_obj = (cyhal_ethernet_t*)_cyhal_eth_irq_obj;
    _cyhal_eth_irq_obj = (cyhal_ethernet_t*) _cyhal_eth_get_irq_obj();

    Cy_ETHIF_DecodeEvent(_cyhal_eth_irq_obj->base);

    _cyhal_eth_irq_obj = old_irq_obj;
}

/************************************************** HELPERS ************************************************************/

static void _cyhal_eth_check_n_call_cb(cyhal_ethernet_t *obj, cyhal_ethernet_event_t event)
{
    CY_ASSERT(NULL != obj);

    if (obj->irq_cause & event)
    {
        cyhal_ethernet_event_callback_t callback = (cyhal_ethernet_event_callback_t)obj->callback_data.callback;
        if(NULL != callback)
        {
            callback(obj->callback_data.callback_arg, event);
        }
    }
}

static void _cyhal_eth_increment_rx_buff_index(size_t *buf_ptr)
{
    CY_ASSERT(NULL != buf_ptr);

    (*buf_ptr)++;
    if (*buf_ptr >= CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE)
    {
        *buf_ptr = 0;
    }
}

static inline bool _cyhal_eth_is_mode_mii(cyhal_ethernet_mac_drive_mode_t mode)
{
    return (CYHAL_ETHERNET_MII_10 == mode) || (CYHAL_ETHERNET_MII_100 == mode);
}

static inline bool _cyhal_eth_is_mode_gmii(cyhal_ethernet_mac_drive_mode_t mode)
{
    return (CYHAL_ETHERNET_GMII_1000 == mode);
}

static inline bool _cyhal_eth_is_mode_rgmii(cyhal_ethernet_mac_drive_mode_t mode)
{
    return (CYHAL_ETHERNET_RGMII_10 == mode) || (CYHAL_ETHERNET_RGMII_100 == mode) ||
            (CYHAL_ETHERNET_RGMII_1000 == mode);
}

static inline bool _cyhal_eth_is_mode_rmii(cyhal_ethernet_mac_drive_mode_t mode)
{
    return (CYHAL_ETHERNET_RMII_10 == mode) || (CYHAL_ETHERNET_RMII_100 == mode);
}

static cy_rslt_t _cyhal_eth_setup_resources(cyhal_ethernet_t* obj, const cyhal_ethernet_pins_t* ethernet_pins,
        cyhal_ethernet_mac_drive_mode_t mode, const cyhal_clock_t* clk)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    // Explicitly marked not allocated resources as invalid to prevent freeing them.
    obj->resource.type = CYHAL_RSC_INVALID;
    memset(&obj->pins, NC, sizeof(obj->pins));

    /* Processing common for all modes pins */

    cyhal_gpio_t mdc_pin = _CYHAL_ETH_GET_PIN_GENERIC(mdc, mode);
    const cyhal_resource_pin_mapping_t *mdc_map = _cyhal_utils_get_resource(mdc_pin, cyhal_pin_map_eth_mdc,
            _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_mdc), NULL, false);

    if (NULL == mdc_map)
    {
        return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
    }
    else
    {
        result = _cyhal_utils_reserve_and_connect(mdc_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_MDC);
        if (CY_RSLT_SUCCESS == result)
        {
            _CYHAL_ETH_STORE_PIN_GENERIC(mdc, mode, obj->pins, mdc_pin);
        }
    }

    /* Ethernet HW block allocation */

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_resource_inst_t eth_rsc = { CYHAL_RSC_ETH, mdc_map->block_num, mdc_map->channel_num };
        result = cyhal_hwmgr_reserve(&eth_rsc);
        if (CY_RSLT_SUCCESS == result)
        {
            obj->resource = eth_rsc;
        }
    }

    _cyhal_utils_enable_ip_block(&obj->resource);

    /* Processing common for all modes pins */

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_gpio_t mdio_pin = _CYHAL_ETH_GET_PIN_GENERIC(mdio, mode);
        const cyhal_resource_pin_mapping_t *mdio_map = _cyhal_utils_get_resource(mdio_pin, cyhal_pin_map_eth_mdio,
            _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_mdio), &(obj->resource), false);
        if (NULL == mdio_map)
        {
            return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
        }
        else
        {
            result = _cyhal_utils_reserve_and_connect(mdio_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_MDIO);
            if (CY_RSLT_SUCCESS == result)
            {
                _CYHAL_ETH_STORE_PIN_GENERIC(mdio, mode, obj->pins, mdio_pin);
            }
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_gpio_t tx_d0_pin = _CYHAL_ETH_GET_PIN_GENERIC(tx_d0, mode);
        const cyhal_resource_pin_mapping_t *tx_d0_map = _cyhal_utils_get_resource(tx_d0_pin, cyhal_pin_map_eth_txd,
            _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
        if (NULL == tx_d0_map)
        {
            return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
        }
        else
        {
            result = _cyhal_utils_reserve_and_connect(tx_d0_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
            if (CY_RSLT_SUCCESS == result)
            {
                _CYHAL_ETH_STORE_PIN_GENERIC(tx_d0, mode, obj->pins, tx_d0_pin);
            }
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_gpio_t tx_d1_pin = _CYHAL_ETH_GET_PIN_GENERIC(tx_d1, mode);
        const cyhal_resource_pin_mapping_t *tx_d1_map = _cyhal_utils_get_resource(tx_d1_pin, cyhal_pin_map_eth_txd,
            _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
        if (NULL == tx_d1_map)
        {
            return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
        }
        else
        {
            result = _cyhal_utils_reserve_and_connect(tx_d1_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
            if (CY_RSLT_SUCCESS == result)
            {
                _CYHAL_ETH_STORE_PIN_GENERIC(tx_d1, mode, obj->pins, tx_d1_pin);
            }
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_gpio_t rx_d0_pin = _CYHAL_ETH_GET_PIN_GENERIC(rx_d0, mode);
        const cyhal_resource_pin_mapping_t *rx_d0_map = _cyhal_utils_get_resource(rx_d0_pin, cyhal_pin_map_eth_rxd,
            _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
        if (NULL == rx_d0_map)
        {
            return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
        }
        else
        {
            result = _cyhal_utils_reserve_and_connect(rx_d0_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
            if (CY_RSLT_SUCCESS == result)
            {
                _CYHAL_ETH_STORE_PIN_GENERIC(rx_d0, mode, obj->pins, rx_d0_pin);
            }
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        cyhal_gpio_t rx_d1_pin = _CYHAL_ETH_GET_PIN_GENERIC(rx_d1, mode);
        const cyhal_resource_pin_mapping_t *rx_d1_map = _cyhal_utils_get_resource(rx_d1_pin, cyhal_pin_map_eth_rxd,
            _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
        if (NULL == rx_d1_map)
        {
            return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
        }
        else
        {
            result = _cyhal_utils_reserve_and_connect(rx_d1_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
            if (CY_RSLT_SUCCESS == result)
            {
                _CYHAL_ETH_STORE_PIN_GENERIC(rx_d1, mode, obj->pins, rx_d1_pin);
            }
        }
    }

    /* Common for mii, gmii and rgmii modes pins */
    if (!_cyhal_eth_is_mode_rmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_d2_pin = _CYHAL_ETH_GET_PIN_SET_M_GM_RGM(tx_d2, mode);
            const cyhal_resource_pin_mapping_t *tx_d2_map = _cyhal_utils_get_resource(tx_d2_pin, cyhal_pin_map_eth_txd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
            if (NULL == tx_d2_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_d2_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM_RGM(tx_d2, mode, obj->pins, tx_d2_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_d3_pin = _CYHAL_ETH_GET_PIN_SET_M_GM_RGM(tx_d3, mode);
            const cyhal_resource_pin_mapping_t *tx_d3_map = _cyhal_utils_get_resource(tx_d3_pin, cyhal_pin_map_eth_txd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
            if (NULL == tx_d3_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_d3_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM_RGM(tx_d3, mode, obj->pins, tx_d3_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_d2_pin = _CYHAL_ETH_GET_PIN_SET_M_GM_RGM(rx_d2, mode);
            const cyhal_resource_pin_mapping_t *rx_d2_map = _cyhal_utils_get_resource(rx_d2_pin, cyhal_pin_map_eth_rxd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_d2_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_d2_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM_RGM(rx_d2, mode, obj->pins, rx_d2_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_d3_pin = _CYHAL_ETH_GET_PIN_SET_M_GM_RGM(rx_d3, mode);
            const cyhal_resource_pin_mapping_t *rx_d3_map = _cyhal_utils_get_resource(rx_d3_pin, cyhal_pin_map_eth_rxd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_d3_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_d3_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM_RGM(rx_d3, mode, obj->pins, rx_d3_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_clk_pin = _CYHAL_ETH_GET_PIN_SET_M_GM_RGM(rx_clk, mode);
            const cyhal_resource_pin_mapping_t *rx_clk_map = _cyhal_utils_get_resource(rx_clk_pin, cyhal_pin_map_eth_rx_clk,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_clk_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_clk_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RX_CLK);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM_RGM(rx_clk, mode, obj->pins, rx_clk_pin);
                }
            }
        }
    }

    /* Common for mii, rmii and gmii modes pins */
    if (!_cyhal_eth_is_mode_rgmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_en_pin = _CYHAL_ETH_GET_PIN_SET_M_RM_GM(tx_en, mode);
            const cyhal_resource_pin_mapping_t *tx_en_map = _cyhal_utils_get_resource(tx_en_pin, cyhal_pin_map_eth_tx_ctl,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_tx_ctl), &(obj->resource), true);
            if (NULL == tx_en_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_en_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TX_CTL);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_RM_GM(tx_en, mode, obj->pins, tx_en_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_err_pin = _CYHAL_ETH_GET_PIN_SET_M_RM_GM(rx_err, mode);
            const cyhal_resource_pin_mapping_t *rx_err_map = _cyhal_utils_get_resource(rx_err_pin, cyhal_pin_map_eth_rx_er,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rx_er), &(obj->resource), true);
            if (NULL == rx_err_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_err_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RX_ER);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_RM_GM(rx_err, mode, obj->pins, rx_err_pin);
                }
            }
        }
    }

    /* Common for mii and gmii modes pins */
    if (_cyhal_eth_is_mode_mii(mode) || _cyhal_eth_is_mode_gmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_err_pin = _CYHAL_ETH_GET_PIN_SET_M_GM(tx_err, mode);
            const cyhal_resource_pin_mapping_t *tx_err_map = _cyhal_utils_get_resource(tx_err_pin, cyhal_pin_map_eth_tx_er,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_tx_er), &(obj->resource), true);
            if (NULL == tx_err_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_err_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TX_ER);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM(tx_err, mode, obj->pins, tx_err_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_dv_pin = _CYHAL_ETH_GET_PIN_SET_M_GM(rx_dv, mode);
            const cyhal_resource_pin_mapping_t *rx_dv_map = _cyhal_utils_get_resource(rx_dv_pin, cyhal_pin_map_eth_rx_ctl,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rx_ctl), &(obj->resource), true);
            if (NULL == rx_dv_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_dv_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RX_CTL);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM(rx_dv, mode, obj->pins, rx_dv_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_crs_pin = _CYHAL_ETH_GET_PIN_SET_M_GM(rx_crs, mode);
            const cyhal_resource_pin_mapping_t *rx_crs_map = _cyhal_utils_get_resource(rx_crs_pin, cyhal_pin_map_eth_rx_ctl,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rx_ctl), &(obj->resource), true);
            if (NULL == rx_crs_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_crs_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RX_CTL);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_M_GM(rx_crs, mode, obj->pins, rx_crs_pin);
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            if ((_cyhal_eth_is_mode_mii(mode) && (NC != ethernet_pins->eth_type_mii.rx_col)) ||
                (_cyhal_eth_is_mode_gmii(mode) && (NC != ethernet_pins->eth_type_gmii.rx_col)))
            {
                /* No pin is available in HW Block being used */
                result = CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
        }
    }

    /* Common for gmii and rgmii modes pins */
    if (_cyhal_eth_is_mode_gmii(mode) || _cyhal_eth_is_mode_rgmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t gtx_clk_pin = _CYHAL_ETH_GET_PIN_SET_GM_RGM(gtx_clk, mode);
            const cyhal_resource_pin_mapping_t *gtx_clk_map = _cyhal_utils_get_resource(gtx_clk_pin, cyhal_pin_map_eth_tx_clk,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_tx_clk), &(obj->resource), true);
            if (NULL == gtx_clk_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(gtx_clk_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TX_CLK);
                if (CY_RSLT_SUCCESS == result)
                {
                    _CYHAL_ETH_STORE_PIN_SET_GM_RGM(gtx_clk, mode, obj->pins, gtx_clk_pin);
                }
            }
        }
    }

    /* Handling only mii mode pins */
    if (_cyhal_eth_is_mode_mii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_clk_pin = ethernet_pins->eth_type_mii.tx_clk;
            const cyhal_resource_pin_mapping_t *tx_clk_map = _cyhal_utils_get_resource(tx_clk_pin, cyhal_pin_map_eth_tx_clk,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_tx_clk), &(obj->resource), true);
            if (NULL == tx_clk_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_clk_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TX_CLK);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.mii.tx_clk = tx_clk_pin;
                }
            }
        }
    }

    /* Handling only rmii mode pins */
    if (_cyhal_eth_is_mode_rmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t ref_clk_pin = ethernet_pins->eth_type_rmii.ref_clk;
            const cyhal_resource_pin_mapping_t *ref_clk_map = _cyhal_utils_get_resource(ref_clk_pin, cyhal_pin_map_eth_ref_clk,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_ref_clk), &(obj->resource), true);
            if (NULL == ref_clk_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(ref_clk_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_REF_CLK);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.rmii.ref_clk = ref_clk_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_dv_crs_pin = ethernet_pins->eth_type_rmii.rx_dv_crs;
            const cyhal_resource_pin_mapping_t *rx_dv_crs_map = _cyhal_utils_get_resource(rx_dv_crs_pin, cyhal_pin_map_eth_rx_ctl,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rx_ctl), &(obj->resource), true);
            if (NULL == rx_dv_crs_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_dv_crs_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RX_CTL);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.rmii.rx_dv_crs = rx_dv_crs_pin;
                }
            }
        }
    }

    /* Handling only gmii mode pins */
    if (_cyhal_eth_is_mode_gmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_d4_pin = ethernet_pins->eth_type_gmii.tx_d4;
            const cyhal_resource_pin_mapping_t *tx_d4_map = _cyhal_utils_get_resource(tx_d4_pin, cyhal_pin_map_eth_txd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
            if (NULL == tx_d4_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_d4_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.tx_d4 = tx_d4_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_d5_pin = ethernet_pins->eth_type_gmii.tx_d5;
            const cyhal_resource_pin_mapping_t *tx_d5_map = _cyhal_utils_get_resource(tx_d5_pin, cyhal_pin_map_eth_txd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
            if (NULL == tx_d5_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_d5_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.tx_d5 = tx_d5_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_d6_pin = ethernet_pins->eth_type_gmii.tx_d6;
            const cyhal_resource_pin_mapping_t *tx_d6_map = _cyhal_utils_get_resource(tx_d6_pin, cyhal_pin_map_eth_txd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
            if (NULL == tx_d6_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_d6_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.tx_d6 = tx_d6_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_d7_pin = ethernet_pins->eth_type_gmii.tx_d7;
            const cyhal_resource_pin_mapping_t *tx_d7_map = _cyhal_utils_get_resource(tx_d7_pin, cyhal_pin_map_eth_txd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_txd), &(obj->resource), true);
            if (NULL == tx_d7_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_d7_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.tx_d7 = tx_d7_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_d4_pin = ethernet_pins->eth_type_gmii.rx_d4;
            const cyhal_resource_pin_mapping_t *rx_d4_map = _cyhal_utils_get_resource(rx_d4_pin, cyhal_pin_map_eth_rxd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_d4_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_d4_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.rx_d4 = rx_d4_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_d5_pin = ethernet_pins->eth_type_gmii.rx_d5;
            const cyhal_resource_pin_mapping_t *rx_d5_map = _cyhal_utils_get_resource(rx_d5_pin, cyhal_pin_map_eth_rxd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_d5_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_d5_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.rx_d5 = rx_d5_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_d6_pin = ethernet_pins->eth_type_gmii.rx_d6;
            const cyhal_resource_pin_mapping_t *rx_d6_map = _cyhal_utils_get_resource(rx_d6_pin, cyhal_pin_map_eth_rxd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_d6_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_d6_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.rx_d6 = rx_d6_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_d7_pin = ethernet_pins->eth_type_gmii.rx_d7;
            const cyhal_resource_pin_mapping_t *rx_d7_map = _cyhal_utils_get_resource(rx_d7_pin, cyhal_pin_map_eth_rxd,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rxd), &(obj->resource), true);
            if (NULL == rx_d7_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_d7_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RXD);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.gmii.rx_d7 = rx_d7_pin;
                }
            }
        }
    }

    /* Handling only rgmii mode pins */
    if (_cyhal_eth_is_mode_rgmii(mode))
    {
        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t tx_ctl_pin = ethernet_pins->eth_type_rgmii.tx_ctl;
            const cyhal_resource_pin_mapping_t *tx_ctl_map = _cyhal_utils_get_resource(tx_ctl_pin, cyhal_pin_map_eth_tx_ctl,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_tx_ctl), &(obj->resource), true);
            if (NULL == tx_ctl_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(tx_ctl_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_TX_CTL);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.rgmii.tx_ctl = tx_ctl_pin;
                }
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            cyhal_gpio_t rx_ctl_pin = ethernet_pins->eth_type_rgmii.rx_ctl;
            const cyhal_resource_pin_mapping_t *rx_ctl_map = _cyhal_utils_get_resource(rx_ctl_pin, cyhal_pin_map_eth_rx_ctl,
                _CYHAL_UTILS_GET_ARR_EL_NUM(cyhal_pin_map_eth_rx_ctl), &(obj->resource), true);
            if (NULL == rx_ctl_map)
            {
                return CYHAL_ETHERNET_RSLT_ERR_INVALID_PIN;
            }
            else
            {
                result = _cyhal_utils_reserve_and_connect(rx_ctl_map, CYHAL_PIN_MAP_DRIVE_MODE_ETH_RX_CTL);
                if (CY_RSLT_SUCCESS == result)
                {
                    obj->pins.rgmii.rx_ctl = rx_ctl_pin;
                }
            }
        }

        if (result == CY_RSLT_SUCCESS)
        {
            if (clk == NULL)
            {
                result = _cyhal_utils_allocate_clock(&(obj->clock), &obj->resource, CYHAL_CLOCK_BLOCK_PERIPHERAL_8BIT, false);
                obj->is_clock_owned = (CY_RSLT_SUCCESS == result);
            }
            else
            {
                obj->is_clock_owned = false;
                obj->clock = *clk;
            }
        }

        if (result == CY_RSLT_SUCCESS)
        {
            result = _cyhal_utils_peri_pclk_assign_divider(_cyhal_eth_pclks[obj->resource.block_num], &(obj->clock));
        }

        if (result == CY_RSLT_SUCCESS)
        {
            cyhal_clock_t clock_tsu;
            cyhal_resource_inst_t clock_tsu_rsc = { .type = CYHAL_RSC_CLOCK, .block_num = CYHAL_CLOCK_BLOCK_HF,
                .channel_num = _cyhal_eth_tsu_clock_hf_num };

            result = cyhal_clock_get(&clock_tsu, &clock_tsu_rsc);
            if (result == CY_RSLT_SUCCESS)
            {
                if (!cyhal_clock_is_enabled(&clock_tsu))
                {
                    result = CYHAL_ETHERNET_RSLT_ERR_TSU_CLK_OFF;
                }
                else
                {
                    obj->clock_tsu = clock_tsu;
                }
            }
        }
    }

    return result;
}

static void _cyhal_eth_phy_read_common(ETH_Type *eth_instance, uint32_t phy_id, uint32_t reg_address, uint32_t *value)
{
    *value = Cy_ETHIF_PhyRegRead(eth_instance, reg_address, phy_id);
}

static void _cyhal_eth_phy_read_eth0(uint32_t phy_id, uint32_t reg_address, uint32_t *value)
{
    _cyhal_eth_phy_read_common(_CYHAL_ETH_BASE_ADDRESSES[0], phy_id, reg_address, value);
}

static void _cyhal_eth_phy_read_eth1(uint32_t phy_id, uint32_t reg_address, uint32_t *value)
{
    _cyhal_eth_phy_read_common(_CYHAL_ETH_BASE_ADDRESSES[1], phy_id, reg_address, value);
}

static void _cyhal_eth_phy_write_common(ETH_Type *eth_instance, uint32_t phy_id, uint32_t reg_address, uint32_t value)
{
    (void)Cy_ETHIF_PhyRegWrite(eth_instance, reg_address, value, phy_id);
}

static void _cyhal_eth_phy_write_eth0(uint32_t phy_id, uint32_t reg_address, uint32_t value)
{
    _cyhal_eth_phy_write_common(_CYHAL_ETH_BASE_ADDRESSES[0], phy_id, reg_address, value);
}

static void _cyhal_eth_phy_write_eth1(uint32_t phy_id, uint32_t reg_address, uint32_t value)
{
    _cyhal_eth_phy_write_common(_CYHAL_ETH_BASE_ADDRESSES[1], phy_id, reg_address, value);
}

static cyhal_ethernet_t *_cyhal_eth_get_obj_by_base(ETH_Type *base)
{
    CY_ASSERT(NULL != base);

    for (uint8_t block_idx = 0; block_idx < CY_IP_MXETH_INSTANCES; ++block_idx)
    {
        if ((NULL != _cyhal_eth_objects[block_idx]) && (_cyhal_eth_objects[block_idx]->base == base))
        {
            return _cyhal_eth_objects[block_idx];
        }
    }

    return NULL;
}

static cy_rslt_t _cyhal_eth_find_best_div(uint32_t hz_src, uint32_t desired_hz,
        const cyhal_clock_tolerance_t *tolerance, bool only_below_desired, uint32_t *div, uint32_t min_hf_freq)
{
    CY_UNUSED_PARAMETER(tolerance);
    CY_UNUSED_PARAMETER(only_below_desired);
    CY_UNUSED_PARAMETER(min_hf_freq);

    if (hz_src > desired_hz)
    {
        float32_t calculated_div_float = hz_src / (float32_t)desired_hz;
        uint32_t calculated_div_uint = (uint32_t)calculated_div_float;
        calculated_div_uint = ((calculated_div_float - calculated_div_uint) < 0.5f) ?
                calculated_div_uint : calculated_div_uint + 1;
        /* We operate only with 8 bit divider */
        if (calculated_div_uint > 0xFF)
        {
            *div = 0xFF;
        }
        else
        {
            *div = (uint8_t)calculated_div_uint;
        }
    }
    else
    {
        *div = 1;
    }

    return CY_RSLT_SUCCESS;
}

static cy_rslt_t _cyhal_eth_configure_speed(cyhal_ethernet_t* obj, cy_stc_ethif_wrapper_config_t* wrapper_cfg)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cyhal_ethernet_mac_drive_mode_t drive_mode = (cyhal_ethernet_mac_drive_mode_t)obj->mode;

    uint32_t required_frequency;
    if ((CYHAL_ETHERNET_MII_10 == drive_mode) || (CYHAL_ETHERNET_RGMII_10 == drive_mode) ||
            (CYHAL_ETHERNET_RMII_10 == drive_mode))
    {
        /* 2.5 MHz is required by link speed 10 Mbit/s */
        required_frequency = _CYHAL_UTILS_MHZ_TO_HZ(2.5);
    }
    else if ((CYHAL_ETHERNET_MII_100 == drive_mode) || (CYHAL_ETHERNET_RGMII_100 == drive_mode) ||
                (CYHAL_ETHERNET_RMII_100 == drive_mode))
    {
        /* 25 MHz is required by link speed 100 Mbit/s */
        required_frequency = _CYHAL_UTILS_MHZ_TO_HZ(25);
    }
    else
    {
        /* 125 MHz is required by link speed 1000 Mbit/s */
        required_frequency = _CYHAL_UTILS_MHZ_TO_HZ(125);
    }

    uint32_t most_suitable_div = 0;
    /* Ethernet tollerance is 100 PPM */
    cyhal_clock_tolerance_t tolerance = { .type = CYHAL_TOLERANCE_PPM, .value = 100 };

    if (obj->is_clock_owned)
    {

        /*  Find most suitable HF clock source and divider for it to achieve closest frequency to desired.
        *   No clock settings are being changed here. */

        cyhal_clock_t most_suitable_hf_source;
        result = _cyhal_utils_find_hf_source_n_divider(&(obj->clock), required_frequency, NULL, _cyhal_eth_find_best_div,
                &most_suitable_hf_source, &most_suitable_div, _CYHAL_ETH_MIN_HF_CLK_HZ);

        if (CY_RSLT_SUCCESS == result)
        {
            result = cyhal_clock_set_source(&(obj->clock), &most_suitable_hf_source);
        }
    }
    else
    {
        uint32_t src_clock_freq = cyhal_clock_get_frequency(&(obj->clock));
        result =  _cyhal_eth_find_best_div(src_clock_freq, required_frequency, &tolerance, false,
                &most_suitable_div, _CYHAL_ETH_MIN_HF_CLK_HZ);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        wrapper_cfg->u8RefClkDiv = (uint8_t)most_suitable_div;

        switch(drive_mode)
        {
            case CYHAL_ETHERNET_MII_10:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_MII_10;
                break;
            case CYHAL_ETHERNET_MII_100:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_MII_100;
                break;
            case CYHAL_ETHERNET_GMII_1000:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_GMII_1000;
                break;
            case CYHAL_ETHERNET_RGMII_10:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_RGMII_10;
                break;
            case CYHAL_ETHERNET_RGMII_100:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_RGMII_100;
                break;
            case CYHAL_ETHERNET_RGMII_1000:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_RGMII_1000;
                break;
            case CYHAL_ETHERNET_RMII_10:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_RMII_10;
                break;
            case CYHAL_ETHERNET_RMII_100:
                wrapper_cfg->stcInterfaceSel = CY_ETHIF_CTL_RMII_100;
                break;
            default:
                /* Unexpected drive mode */
                CY_ASSERT(false);
        }
    }

    return result;
}

static cy_rslt_t _cyhal_eth_prepare_tsu_inc_cfg(cyhal_ethernet_t* obj, cy_stc_ethif_timer_increment_t* tsu_inc_cfg)
{
    /** To calculate the value to write to the sub-ns register you take the decimal value of
    the sub-nanosecond value, then multiply it by 2 to the power of 24 (16777216) and
    convert the result to hexadecimal. For example for a sub-nanosecond value of 0.33333333
    you would write 0x555555.  */

    uint32_t tsu_src_freq = cyhal_clock_get_frequency(&(obj->clock_tsu));
    if (0 != tsu_src_freq)
    {
        float32_t src_period_ns = 1e6f / (float32_t)tsu_src_freq;
        /** Whole nanoseconds to increment timer each clock cycle, leaving only integer part */
        tsu_inc_cfg->nanoSecsInc = (uint32_t)src_period_ns;

        uint32_t sub_ns_val = (uint32_t)((float32_t)(src_period_ns - tsu_inc_cfg->nanoSecsInc));
        /* MSB of Sub-nanoseconds to increment the timer (16 bits) */
        tsu_inc_cfg->subNsInc = (uint16_t)((sub_ns_val & 0xFFFF00u) >> 8);
        /* Lower 8 bits of sub-nanoseconds to increment the timer */
        tsu_inc_cfg->lsbSubNsInc = (uint8_t)(sub_ns_val & 0xFF);
        /* Number of increments before changing to alternative increment. If = 0 then never use alternative increment. */
        tsu_inc_cfg->altIncCount = 0;
        /* Alternative nanoseconds increment to apply */
        tsu_inc_cfg->altNanoSInc = 0;
    }
    else
    {
        return CYHAL_ETHERNET_RSLT_ERR_TSU_CLK_OFF;
    }

    return CY_RSLT_SUCCESS;
}

static cy_rslt_t _cyhal_eth_init_hw(cyhal_ethernet_t* obj, cy_stc_ethif_mac_config_t* ethif_mac_cfg,
        cy_stc_ethif_intr_config_t* ethif_intr_cfg)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (!_cyhal_eth_driver_initialized)
    {
        memset(_cyhal_eth_objects, 0, sizeof(_cyhal_eth_objects));
        _cyhal_eth_driver_initialized = true;
    }

    obj->rx_write_ptr = 0;
    obj->rx_read_ptr = 0;
    obj->buffs_occupied = 0;
    memset(&(obj->rx_frames_sizes), 0, sizeof(obj->rx_frames_sizes));

    if (CY_RSLT_SUCCESS == result)
    {
        _cyhal_eth_objects[obj->resource.block_num] = obj;

        phy_read_handle read_handle = (0 == obj->resource.block_num) ? _cyhal_eth_phy_read_eth0 : _cyhal_eth_phy_read_eth1;
        phy_write_handle write_handle = (0 == obj->resource.block_num) ? _cyhal_eth_phy_write_eth0 : _cyhal_eth_phy_write_eth1;

        /* initialize phy */
        result = (cy_rslt_t)Cy_EPHY_Init(&obj->phy_obj, read_handle, write_handle);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        if (CY_RSLT_SUCCESS == result)
        {
            for (size_t bd_idx = 0; bd_idx < CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE; ++bd_idx)
            {
                obj->rx_frames_buffs_ptrs[bd_idx] = &(obj->rx_frames_buffs[bd_idx][0]);
            }

            ethif_mac_cfg->pRxQbuffPool[0] = (cy_ethif_buffpool_t *)(obj->rx_frames_buffs_ptrs);
            result = (cy_rslt_t)Cy_ETHIF_Init(obj->base, ethif_mac_cfg, ethif_intr_cfg);
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        memset(&(obj->pdl_callbacks), 0, sizeof(obj->pdl_callbacks));
        /* rxframecb and rxgetbuff are critical for RX operation. Therefore, registering it right during the initialization */
        obj->pdl_callbacks.rxframecb = _cyhal_eth_cb_rx_frame;
        obj->pdl_callbacks.rxgetbuff = _cyhal_eth_cb_get_buff;

        Cy_ETHIF_RegisterCallbacks(obj->base, &(obj->pdl_callbacks));

        obj->irq_cause = CYHAL_ETHERNET_EVENT_NONE;

        /* MAC configured to capture all the network traffic */
        Cy_ETHIF_SetPromiscuousMode(obj->base, true);
        /* MAC is configured to capture broadcast type frames */
        Cy_ETHIF_SetNoBroadCast(obj->base, false);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        for (uint8_t irq_idx = (obj->resource.block_num * _CYHAL_ETH_QUEUES_PER_INSTANCE);
                     irq_idx < (obj->resource.block_num * _CYHAL_ETH_QUEUES_PER_INSTANCE + _CYHAL_ETH_QUEUES_PER_INSTANCE);
                     ++irq_idx
        )
        {
            result = _cyhal_irq_register(_cyhal_eth_irq_n[irq_idx], CYHAL_ISR_PRIORITY_DEFAULT, _cyhal_eth_interrupt_handler);
            _cyhal_irq_enable(_cyhal_eth_irq_n[irq_idx]);
            if (CY_RSLT_SUCCESS != result)
            {
                break;
            }
        }
    }

    return result;
}

static inline uint8_t _cyhal_eth_get_block_from_irqn(_cyhal_system_irq_t irqn)
{
    for(uint8_t i = 0; i < _CYHAL_ETH_ARRAY_LEN(_cyhal_eth_irq_n); ++i)
    {
        if (_cyhal_eth_irq_n[i] == irqn)
        {
            return i / _CYHAL_ETH_QUEUES_PER_INSTANCE;
        }
    }
    CY_ASSERT(false);
    return 0;
}

static cyhal_ethernet_t *_cyhal_eth_get_irq_obj(void)
{
    _cyhal_system_irq_t irqn = _cyhal_irq_get_active();
    uint8_t block = _cyhal_eth_get_block_from_irqn(irqn);
    return _cyhal_eth_objects[block];
}

/********************************************** PUBLIC FUNCTIONS *******************************************************/

cy_rslt_t cyhal_ethernet_init(cyhal_ethernet_t* obj, const cyhal_ethernet_config_t* eth_config,
        const cyhal_ethernet_pins_t* ethernet_pins, const cyhal_clock_t* clk)
{
    CY_ASSERT(obj != NULL);
    CY_ASSERT(eth_config != NULL);
    CY_ASSERT(ethernet_pins != NULL);

    memset(obj, 0, sizeof(cyhal_ethernet_t));

    cy_rslt_t result = _cyhal_eth_setup_resources(obj, ethernet_pins, eth_config->drive_mode, clk);
    cy_stc_ethif_wrapper_config_t eth_pdl_wrapper_cfg = _cyhal_eth_def_wrapper_cfg;

    if (CY_RSLT_SUCCESS == result)
    {
        obj->base = _CYHAL_ETH_BASE_ADDRESSES[obj->resource.block_num];
        result = _cyhal_eth_configure_speed(obj, &eth_pdl_wrapper_cfg);
    }

    cy_stc_ethif_mac_config_t eth_pdl_cfg = _cyhal_eth_def_pdl_config;
    cy_stc_ethif_intr_config_t eth_pdl_intr_cfg = _cyhal_eth_def_intr_config;
    cy_stc_ethif_timer_increment_t tsu_inc_cfg;

    if (CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_eth_prepare_tsu_inc_cfg(obj, &tsu_inc_cfg);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        /* We are about to use global variables, that can be used by another thread and this
        * will lead to data corruption. Entering critical section to prevent this. */
        uint32_t cs_int_status = cyhal_system_critical_section_enter();

        /* pstcTimerIncValue is address to global variable value, which is shared between multiple
         * threads / cyhal_ethernet_t instances */
        eth_pdl_cfg.pstcTSUConfig->pstcTimerIncValue = &tsu_inc_cfg;

        eth_pdl_cfg.pstcWrapperConfig = &eth_pdl_wrapper_cfg;
        obj->mode = (uint8_t)eth_config->drive_mode;
        result = _cyhal_eth_init_hw(obj, &eth_pdl_cfg, &eth_pdl_intr_cfg);

        cyhal_system_critical_section_exit(cs_int_status);

    }

    if (CY_RSLT_SUCCESS != result)
    {
        cyhal_ethernet_free(obj);
    }

    return result;
}

cy_rslt_t cyhal_ethernet_init_cfg(cyhal_ethernet_t *obj, const cyhal_ethernet_configurator_t *cfg)
{
    CY_ASSERT(obj != NULL);
    CY_ASSERT(cfg != NULL);

    memset(obj, 0, sizeof(cyhal_ethernet_t));

    obj->base = _CYHAL_ETH_BASE_ADDRESSES[cfg->resource->block_num];
    obj->dc_configured = true;
    obj->resource = *(cfg->resource);
    obj->is_clock_owned = false;
    obj->clock = *(cfg->clock);

    cy_stc_ethif_mac_config_t pdl_cfg = *(cfg->ethif_mac_cfg);
    cy_stc_ethif_intr_config_t pdl_intr_cfg = *(cfg->ethif_intr_config);

    return _cyhal_eth_init_hw(obj, &pdl_cfg, &pdl_intr_cfg);
}

void cyhal_ethernet_free(cyhal_ethernet_t *obj)
{
    CY_ASSERT(NULL != obj);

    if (obj->resource.type != CYHAL_RSC_INVALID)
    {
        if (_cyhal_eth_driver_initialized && (NULL != _cyhal_eth_objects[obj->resource.block_num]))
        {
            _cyhal_eth_objects[obj->resource.block_num] = NULL;
        }

        //_cyhal_irq_free(irqn);
        cyhal_hwmgr_free(&(obj->resource));
        obj->resource.type = CYHAL_RSC_INVALID;
    }

    if (!obj->dc_configured)
    {
        cyhal_ethernet_mac_drive_mode_t mode = (cyhal_ethernet_mac_drive_mode_t)obj->mode;

        /* Common for all mods pins */
        _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(mdc, mode, obj->pins);
        _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(mdio, mode, obj->pins);
        _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(tx_d0, mode, obj->pins);
        _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(tx_d1, mode, obj->pins);
        _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(rx_d0, mode, obj->pins);
        _CYHAL_ETH_FREE_IF_USED_PIN_GENERIC(rx_d1, mode, obj->pins);

        /* Common for mii, gmii and rgmii modes pins */
        if (!_cyhal_eth_is_mode_rmii(mode))
        {
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM_RGM(tx_d2, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM_RGM(tx_d3, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM_RGM(rx_d2, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM_RGM(rx_d3, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM_RGM(rx_clk, mode, obj->pins);
        }

        /* Common for mii, rmii and gmii modes pins */
        if (!_cyhal_eth_is_mode_rgmii(mode))
        {
            _CYHAL_ETH_FREE_IF_USED_PIN_M_RM_GM(tx_en, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_RM_GM(rx_err, mode, obj->pins);
        }

        /* Common for mii and gmii modes pins */
        if (_cyhal_eth_is_mode_mii(mode) || _cyhal_eth_is_mode_gmii(mode))
        {
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM(tx_err, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM(rx_dv, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_M_GM(rx_crs, mode, obj->pins);
        }

        /* Common for gmii and rgmii modes pins */
        if (_cyhal_eth_is_mode_gmii(mode) || _cyhal_eth_is_mode_rgmii(mode))
        {
            _CYHAL_ETH_FREE_IF_USED_PIN_GM_RGM(gtx_clk, mode, obj->pins);
            _CYHAL_ETH_FREE_IF_USED_PIN_GM_RGM(gtx_clk, mode, obj->pins);
        }

        /* Handling only mii mode pins */
        if (_cyhal_eth_is_mode_mii(mode))
        {
            _cyhal_utils_release_if_used(&(obj->pins.mii.tx_clk));
        }

        /* Handling only rmii mode pins */
        if (_cyhal_eth_is_mode_rmii(mode))
        {
            _cyhal_utils_release_if_used(&(obj->pins.rmii.ref_clk));
            _cyhal_utils_release_if_used(&(obj->pins.rmii.rx_dv_crs));
        }

        /* Handling only gmii mode pins */
        if (_cyhal_eth_is_mode_gmii(mode))
        {
            _cyhal_utils_release_if_used(&(obj->pins.gmii.tx_d4));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.tx_d5));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.tx_d6));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.tx_d7));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.rx_d4));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.rx_d5));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.rx_d6));
            _cyhal_utils_release_if_used(&(obj->pins.gmii.rx_d7));
        }

        /* Handling only rgmii mode pins */
        if (_cyhal_eth_is_mode_rgmii(mode))
        {
            _cyhal_utils_release_if_used(&(obj->pins.rgmii.tx_ctl));
            _cyhal_utils_release_if_used(&(obj->pins.rgmii.rx_ctl));
        }

        memset(&obj->pins, NC, sizeof(obj->pins));

        if (obj->is_clock_owned)
        {
            cyhal_clock_free(&(obj->clock));
        }
    }
}

void cyhal_ethernet_register_callback(cyhal_ethernet_t *obj, cyhal_ethernet_event_callback_t callback, void *callback_arg)
{
    CY_ASSERT(NULL != obj);

    uint32_t saved_intr_status = cyhal_system_critical_section_enter();
    obj->callback_data.callback = (cy_israddress) callback;
    obj->callback_data.callback_arg = callback_arg;
    cyhal_system_critical_section_exit(saved_intr_status);
}

void cyhal_ethernet_enable_event(cyhal_ethernet_t *obj, cyhal_ethernet_event_t event, uint8_t intr_priority, bool enable)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);

    if (enable)
    {
        obj->irq_cause |= (uint32_t)event;
    }
    else
    {
        obj->irq_cause &= ~(uint32_t)event;
    }

    if (event & CYHAL_ETHERNET_TX_COMPLETE_EVENT)
    {
        obj->pdl_callbacks.txcompletecb = enable ? _cyhal_eth_cb_tx_frame_successful : NULL;
    }
    if (event & CYHAL_ETHERNET_TX_ERR_OCCURED_EVENT)
    {
        obj->pdl_callbacks.txerrorcb = enable ? _cyhal_eth_cb_tx_frame_failure : NULL;
    }
    if (event & CYHAL_ETHERNET_TSU_SECOND_INC_EVENT)
    {
        obj->pdl_callbacks.tsuSecondInccb = enable ? _cyhal_eth_cb_tsu_increment : NULL;
    }

    Cy_ETHIF_RegisterCallbacks(obj->base, &obj->pdl_callbacks);

    for(uint8_t i = (obj->resource.block_num * _CYHAL_ETH_QUEUES_PER_INSTANCE);
        i < (obj->resource.block_num * _CYHAL_ETH_QUEUES_PER_INSTANCE + _CYHAL_ETH_QUEUES_PER_INSTANCE); ++i)
    {
        _cyhal_system_irq_t irqn = _cyhal_eth_irq_n[obj->resource.block_num];
        _cyhal_irq_set_priority(irqn, intr_priority);
    }
}

cy_rslt_t cyhal_ethernet_transmit_frame(cyhal_ethernet_t* obj, uint8_t* frame_data, uint16_t size)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);
    CY_ASSERT(NULL != frame_data);

    return (cy_rslt_t)Cy_ETHIF_TransmitFrame(obj->base, frame_data, size, CY_ETH_QS0_0, true);
}

cy_rslt_t cyhal_ethernet_read_frame(cyhal_ethernet_t* obj, uint8_t* frame_data, uint16_t* size)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);
    CY_ASSERT(NULL != frame_data);
    CY_ASSERT(NULL != size);

    if (0 != obj->buffs_occupied)
    {
        *size = obj->rx_frames_sizes[obj->rx_read_ptr];
        memcpy(frame_data, &obj->rx_frames_buffs[obj->rx_read_ptr], *size);
        obj->buffs_occupied--;
        _cyhal_eth_increment_rx_buff_index(&obj->rx_read_ptr);
    }
    else
    {
        *size = 0;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cyhal_ethernet_get_1588_timer_value(cyhal_ethernet_t* obj, cyhal_ethernet_1588_timer_val_t* timer_val)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);
    CY_ASSERT(NULL != timer_val);

    cy_stc_ethif_1588_timer_val_t pdl_timer_val;
    cy_rslt_t result = (cy_rslt_t)Cy_ETHIF_Get1588TimerValue(obj->base, &pdl_timer_val);
    if (CY_RSLT_SUCCESS == result)
    {
        timer_val->secs_upper = pdl_timer_val.secsUpper;
        timer_val->secs_lower = pdl_timer_val.secsLower;
        timer_val->nano_secs = pdl_timer_val.nanosecs;
    }
    return result;
}

cy_rslt_t cyhal_ethernet_set_1588_timer_value(cyhal_ethernet_t* obj, const cyhal_ethernet_1588_timer_val_t* timer_val)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);
    CY_ASSERT(NULL != timer_val);

    return (cy_rslt_t)Cy_ETHIF_Set1588TimerValue(obj->base, (cy_stc_ethif_1588_timer_val_t*)timer_val);
}

cy_rslt_t cyhal_ethernet_config_pause(cyhal_ethernet_t* obj, const uint16_t pause_qunta)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);

    return (cy_rslt_t)Cy_ETHIF_ConfigPause(obj->base, pause_qunta);
}

cy_rslt_t cyhal_ethernet_transmit_pause_frame(cyhal_ethernet_t* obj, const bool start_pause)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);

    return (cy_rslt_t)Cy_ETHIF_TxPauseFrame(obj->base, start_pause);
}

uint8_t cyhal_ethernet_get_num_filter(cyhal_ethernet_t* obj)
{
    CY_UNUSED_PARAM(obj);

    return (CY_ETHIF_FILTER_NUM_INV - 1u);
}

cy_rslt_t cyhal_ethernet_set_filter_address(cyhal_ethernet_t* obj, const uint8_t filter_num, const cyhal_ethernet_filter_config_t* config)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);
    CY_ASSERT(NULL != config);

    return (cy_rslt_t)Cy_ETHIF_SetFilterAddress(obj->base, filter_num, (const cy_stc_ethif_filter_config_t*)config);
}

cy_rslt_t cyhal_ethernet_set_promiscuous_mode(cyhal_ethernet_t* obj, const bool copy_all_frames)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);

    Cy_ETHIF_SetPromiscuousMode(obj->base, copy_all_frames);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cyhal_ethernet_enable_broadcast_receive(cyhal_ethernet_t* obj, const bool enable_broadcast)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);

    Cy_ETHIF_SetNoBroadCast(obj->base, !enable_broadcast);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cyhal_ethernet_phy_reg_write(cyhal_ethernet_t *obj, const uint8_t reg_number, const uint32_t value)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);

    return (cy_rslt_t)Cy_ETHIF_PhyRegWrite(obj->base, reg_number, (uint16_t)value, 0u);
}

cy_rslt_t cyhal_ethernet_phy_reg_read(cyhal_ethernet_t *obj, const uint8_t reg_number, uint32_t *value)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != obj->base);
    CY_ASSERT(NULL != value);

    *value = Cy_ETHIF_PhyRegRead(obj->base, reg_number, 0u);

    return CY_RSLT_SUCCESS;
}

#if defined(__cplusplus)
}
#endif

#endif /* CYHAL_DRIVER_AVAILABLE_ETHERNET */
