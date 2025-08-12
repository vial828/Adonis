/*******************************************************************************
* File Name: cyhal_i3c.c
*
* Description:
* Provides a high level interface for interacting with the Infineon I3C. This is
* a wrapper around the lower level PDL API.
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

/**
 * \addtogroup group_hal_impl_i3c I3C (Improved Inter Integrated Circuit)
 * \ingroup group_hal_impl
 * \{
 * \section section_hal_impl_i3c_init_cfg Configurator-generated features limitations
 * List of I3C modes, which are currently not supported in I3C HAL driver on CAT1D devices:
 *  - I3C Target mode
 *  - I3C Secondary Controller mode
 *
 * \section section_hal_impl_i3c_free_resources I3C deinitialization limitations
 * Due to specifics HW issues we can't disable I3C block when it was started.
 *
 *
 * \} group_hal_impl_i3c
 */

#include <stdlib.h>
#include <string.h>
#include "cyhal_i3c.h"
#include "cyhal_gpio.h"
#include "cyhal_hwmgr.h"
#include "cyhal_system.h"
#include "cyhal_syspm.h"
#include "cyhal_utils.h"
#include "cyhal_irq_impl.h"
#include "cyhal_clock.h"

#if (CYHAL_DRIVER_AVAILABLE_I3C)

#if defined(__cplusplus)
extern "C"
{
#endif

#define _CYHAL_I3C_PENDING_NONE              0
#define _CYHAL_I3C_PENDING_RX                1
#define _CYHAL_I3C_PENDING_TX                2
#define _CYHAL_I3C_PENDING_TX_RX             3

#define _CYHAL_I3C_CONTROLLER_DEFAULT_PUSHPULL_FREQ_HZ       100000
#define _CYHAL_I3C_CONTROLLER_DEFAULT_OPENDRAIN_FREQ_HZ      100000

/* Oversampling factor for the PCLK to achieve desired data rate
    The minimum value is 10 as per documentation.
*/
#define _CYHAL_I3C_CLK_OVERSAMPLING          10

/* Indicates the invalid I3C block selected */
#define _CYHAL_I3C_BLOCK_ID_INVALID          (0xFF)

#define _CYHAL_I3C_ARRAY_SIZE                (CY_IP_MXI3C_INSTANCES)

const uint32_t _CYHAL_I3C_AVAILABLE_BLOCKS_MASK =
{
#ifdef I3C
    1 << 0u
#endif
};

const uint8_t _CYHAL_I3C_BASE_ADDRESS_INDEX[_CYHAL_I3C_ARRAY_SIZE] =
{
#ifdef I3C
    0u,
#endif
};

I3C_CORE_Type* const _CYHAL_I3C_BASE_ADDRESSES[_CYHAL_I3C_ARRAY_SIZE] =
{
#ifdef I3C
    I3C_CORE,
#endif
};

const _cyhal_system_irq_t _CYHAL_I3C_IRQ_N[_CYHAL_I3C_ARRAY_SIZE] =
{
#ifdef I3C
    i3c_interrupt_IRQn,
#endif
};

/* Function pointer to determine a specific I3C instance can is ready for low power transition. */
typedef bool (*cyhal_i3c_instance_pm_callback)(void *obj_ptr, cyhal_syspm_callback_state_t state, cy_en_syspm_callback_mode_t pdl_mode);

/** The configuration structs for the resource in use on each I3C block */
static void *_cyhal_i3c_config_objects[_CYHAL_I3C_ARRAY_SIZE];
/** The callback to use for each I3C instance */
static bool (*_cyhal_i3c_config_pm_callback[_CYHAL_I3C_ARRAY_SIZE]) (void *obj_ptr, cyhal_syspm_callback_state_t state, cy_en_syspm_callback_mode_t pdl_mode);

const cyhal_resource_pin_mapping_t* _cyhal_i3c_find_map(cyhal_gpio_t pin, const cyhal_resource_pin_mapping_t *pin_map,
        size_t count, const cyhal_resource_inst_t *block_res)
{
    for (size_t i = 0; i < count; i++)
    {
        if (pin == pin_map[i].pin)
        {
            /* Block is already found, check if certain pin can work for provided block  */
            if ((NULL != block_res) && (CYHAL_RSC_I3C == block_res->type))
            {
                if (_cyhal_utils_map_resource_equal(block_res, &(pin_map[i]), false))
                {
                    return &pin_map[i];
                }
            }
            /* No block provided */
            else
            {
                cyhal_resource_inst_t rsc = { CYHAL_RSC_I3C, pin_map[i].block_num, pin_map[i].channel_num };
                if (CY_RSLT_SUCCESS == cyhal_hwmgr_reserve(&rsc))
                {
                    cyhal_hwmgr_free(&rsc);
                    return &pin_map[i];
                }
            }
        }
    }
    return NULL;
}

uint32_t _cyhal_i3c_check_pin_affiliation(cyhal_gpio_t pin, const cyhal_resource_pin_mapping_t *pin_map,
        size_t count)
{
    uint32_t bitband_blocks = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (pin == pin_map[i].pin)
        {
            cyhal_resource_inst_t rsc = { CYHAL_RSC_I3C, pin_map[i].block_num, pin_map[i].channel_num };
            if (CY_RSLT_SUCCESS == cyhal_hwmgr_reserve(&rsc))
            {
                cyhal_hwmgr_free(&rsc);
                bitband_blocks |= 1 << pin_map[i].block_num;
            }
        }
    }
    return bitband_blocks;
}

/** Finds the en_clk_dst_t clock connection index for provided I3C block number
 *
 * @param[in] block_num Index of I3C block
 * @return en_clk_dst_t clock connection index
 */
static inline en_clk_dst_t _cyhal_i3c_get_clock_index(uint32_t block_num)
{
    en_clk_dst_t clk = (en_clk_dst_t)((uint32_t)PCLK_I3C_CLOCK_I3C_EN + block_num);
    return clk;
}

uint32_t _cyhal_i3c_set_data_rate(cyhal_i3c_t *obj, uint32_t freq, bool is_target)
{
    cyhal_clock_t *clock = &(obj->clock);
    uint32_t data_rate = (is_target)
        ? Cy_I3C_GetDataRate(obj->base, &obj->context)
        : Cy_I3C_SetDataRate(obj->base, freq, cyhal_clock_get_frequency(clock), &obj->context);
    return data_rate;
}

cy_rslt_t _cyhal_i3c_set_peri_divider(cyhal_i3c_t *obj, uint32_t freq)
{
    uint32_t block_num = obj->resource.block_num;
    cyhal_clock_t *clock = &(obj->clock);
    bool is_clock_owned = obj->is_clock_owned;

    cy_rslt_t status = CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
    if (freq != 0)
    {
        uint32_t peri_freq = freq * _CYHAL_I3C_CLK_OVERSAMPLING;
        status = _cyhal_utils_peri_pclk_assign_divider(_cyhal_i3c_get_clock_index(block_num), clock);

        if (peri_freq > 0 && CY_SYSCLK_SUCCESS == status)
        {
            if (is_clock_owned)
            {
                status = cyhal_clock_set_enabled(clock, false, false);
                if (status == CY_RSLT_SUCCESS)
                {
                    status = cyhal_clock_set_frequency(clock, peri_freq, NULL);
                }
                if (status == CY_RSLT_SUCCESS)
                {
                    status = cyhal_clock_set_enabled(clock, true, false);
                }
            }
        }
    }
    return status;
}

static uint8_t _cyhal_i3c_get_block_index(uint8_t i3c_block_num)
{
    uint8_t arr_index = _CYHAL_I3C_BLOCK_ID_INVALID;
    for (uint8_t instance_idx = 0; instance_idx < _CYHAL_I3C_ARRAY_SIZE; instance_idx++)
    {
        if (_CYHAL_I3C_BASE_ADDRESS_INDEX[instance_idx] == i3c_block_num)
        {
            arr_index = instance_idx;
            break;
        }
    }
    return arr_index;
}

static uint8_t _cyhal_i3c_get_block_from_irqn(_cyhal_system_irq_t irqn)
{
    uint8_t i3c_block_num = 0;
    switch (irqn)
    {
    #ifdef I3C
        case i3c_interrupt_IRQn:
            i3c_block_num = 0;
            break;
    #endif
    #if (_CYHAL_I3C_ARRAY_SIZE > 1)
    #error "Unhandled I3C count"
    #endif
        default:
            CY_ASSERT(false); // Should never be called with a non-I3C IRQn
            i3c_block_num = _CYHAL_I3C_BLOCK_ID_INVALID;
    }

    return _cyhal_i3c_get_block_index(i3c_block_num);
}

void *_cyhal_i3c_get_irq_obj(void)
{
    _cyhal_system_irq_t irqn = _cyhal_irq_get_active();
    if ((_cyhal_system_irq_t)unconnected_IRQn == irqn)
    {
        return NULL;
    }
    else
    {
        uint8_t block = _cyhal_i3c_get_block_from_irqn(irqn);
        return _cyhal_i3c_config_objects[block];
    }
}

static const _cyhal_buffer_info_t _cyhal_i3c_buff_info_default = {
    .addr = {NULL},
    .size = 0,
};

static const cy_stc_i3c_config_t _cyhal_i3c_default_config = {

    /* Specifies the mode of I3C controller operation. Overriden by cyhal_i3c_cfg_t.i3c_mode */
    .i3cMode = CY_I3C_CONTROLLER,
    /* Specifies the mode of I3C bus operation */
    .i3cBusMode = CY_I3C_BUS_MIXED_FAST,
    /* Use the SDMA for Rx/Tx */
    .useDma = false,
    /* True - Enables the user to configure data rate related parameters for Controller Mode */
    .manualDataRate = false,
    /* The frequency of the clock connected to the I3C block in Hz. Overriden by cyhal_i3c_t.clock */
    .i3cClockHz = 0,
    /* The desired I3C data Rate in Hz. Overriden by cyhal_i3c_cfg_t.i3c_data_rate. */
    .i3cSclRate = 0,
    /* Opendrain data rate in Hz. Overriden by cyhal_i3c_cfg_t.i2c_data_rate. */
    .openDrainSclRate = 100,
    /* Specifies the number of empty locations(or above) in the Transmit FIFO that triggers the Transmit Buffer Threshold Status interrupt */
    .txEmptyBufThld = CY_I3C_1_WORD_DEPTH,
    /* Specifies the number of entries (or above) in the Receive FIFO that triggers the Receive Buffer Threshold Status interrupt */
    .rxBufThld = CY_I3C_1_WORD_DEPTH,
    /*
    * Controller Mode : Specifies the number of Transmit FIFO filled locations count that triggers the transmission.
    *  Target Mode : Specifies the number of Transmit FIFO filled locations count that triggers the transmission.
    */
    .txBufStartThld = CY_I3C_1_WORD_DEPTH,
    /*
    * Controller Mode : Specifies the number of empty locations count in the Receive FIFO that triggers the reception.
    *  Target Mode : Specifies the number of empty locations count in the Receive FIFO that triggers the transmission.
    */
    .rxBufStartThld = CY_I3C_1_WORD_DEPTH,

    /* Below members are only applicable for the Controller mode */

    /* True - I3C broadcast address (0x7E) is used for private transfer */
    .ibaInclude = false,
    /* Specifies whether the Controller ACK/NACK the Hot-Join request from Target */
    .hotJoinCtrl = false,
    /* Specifies the device Dynamic Address */
    .dynamicAddr = 8,
    /* Specifies the number of empty locations(or greater) in the Command Queue that triggers the Command Queue Ready Status interrupt */
    .cmdQueueEmptyThld = 1,
    /* Specifies the number of entries(or greater) in the Response Queue that triggers the Response Queue Ready Status interrupt */
    .respQueueThld = 0,
    /* Specifies the number of IBI status entries(or greater) in the IBI Queue that triggers the IBI Buffer Threshold Status interrupt */
    .ibiQueueThld = 1,
    /* SDA hold time (in terms of number of I3C block clock cycles) of the transmit data with respect to the SCL edge in FM, FM+, SDR and DDR speed mode of operations */
    .sdaHoldTime = 1,
    /*  Specifies    the I3C bus free count value */
    .busFreeTime = 32,

    /* For Manual Data Rate Control */

    /* I3C Open Drain low count value */
    .openDrainLowCnt = 0,
    /* I3C Open Drain high count value */
    .openDrainHighCnt = 0,
    /* I3C Push Pull low count value */
    .pushPullLowCnt = 0,
    /*  I3C Push Pull high count value */
    .pushPullHighCnt = 0,
    /*  I2C FM Mode low count value */
    .i2cFMLowCnt = 0,
    /* I2C FM Mode high count value */
    .i2cFMHighCnt = 0,
    /* I2C FM Plus Mode low count value */
    .i2cFMPlusLowCnt = 0,
    /* I2C FM Plus Mode high count value */
    .i2cFMPlusHighCnt = 0,
    /* I3C Extended Low Count for SDR1 Mode */
    .extLowCnt1 = 11,
    /* I3C Extended Low Count for SDR2 Mode */
    .extLowCnt2 = 16,
    /* I3C Extended Low Count for SDR3 Mode */
    .extLowCnt3 = 27,
    /* I3C Extended Low Count for SDR4 Mode */
    .extLowCnt4 = 58,
    /* I3C Read Termination Bit Low count */
    .extTerminationLowCnt = 0,

    /* Below members are only applicable for the Target mode */

    /*  Specifies whether the target uses adaptive I2C I3C mode.
    * It is required to be set only if the device is not aware of the type of the bus to which the target controller is connected
    */
    .adaptiveI2CI3C = false,
    /* The static address of the I3C Target Device, if present */
    .staticAddress = 0,
    /* The Provisional ID of the I3C Target Device */
    .pid = 0,
    /* The device characteristic value of the I3C Target Device */
    .dcr = 0,
    /*
    * Max data speed limitation.
    * True: Limitation on max data speed. maxReadDs, maxWriteDs, maxReadTurnaround members for the device are valid
    * False: No limitation on max data speed
    */
    .speedLimit = false,
    /*
    * SDR only or SDR and HDR capable
      True: SDR and HDR
      False: SDR only
    */
    .hdrCapable = false,
    /* Specifies the Device Role field in Bus Characteristic Register */
    .deviceRoleCap = CY_I3C_SECONDARY_CONTROLLER,
    /*
    * Specifies whether the Hot-Join Request Interrupts are allowed on the I3C bus or not
    * When disabled the Target will not initiate Hot-Join and will take part in Address Assignment without initiating Hot-Join
    */
    .hotjoinEnable = false,
    /* Specifies the I3C bus available count value */
    .busAvailTime = 0,
    /* Specifies the I3C bus idle count value */
    .busIdleTime = 0,
};

/* Power management functionality  */
static bool _cyhal_i3c_pm_transition_pending_value = false;

bool _cyhal_i3c_pm_transition_pending(void)
{
    return _cyhal_i3c_pm_transition_pending_value;
}

static bool _cyhal_i3c_pm_callback_index(uint8_t index, cyhal_syspm_callback_state_t state, cy_en_syspm_callback_mode_t pdl_mode)
{
    void *obj = _cyhal_i3c_config_objects[index];
    cyhal_i3c_instance_pm_callback callback = _cyhal_i3c_config_pm_callback[index];
    return ((NULL != obj) && (callback != NULL)) ? callback(obj, state, pdl_mode) : true;
}

static bool _cyhal_i3c_pm_callback_common(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void* callback_arg)
{
    CY_UNUSED_PARAMETER(callback_arg);
    bool allow = true;
    cy_en_syspm_callback_mode_t pdl_mode = _cyhal_utils_convert_haltopdl_pm_mode(mode);
    for (uint8_t instance_idx = 0; instance_idx < _CYHAL_I3C_ARRAY_SIZE; instance_idx++)
    {
        allow = _cyhal_i3c_pm_callback_index(instance_idx, state, pdl_mode);

        if (!allow && mode == CYHAL_SYSPM_CHECK_READY)
        {
            for (uint8_t revert_idx = 0; revert_idx < instance_idx; revert_idx++)
            {
                _cyhal_i3c_pm_callback_index(revert_idx, state, CY_SYSPM_CHECK_FAIL);
            }
            break;
        }
    }

    if (mode == CYHAL_SYSPM_CHECK_FAIL || mode == CYHAL_SYSPM_AFTER_TRANSITION)
    {
        _cyhal_i3c_pm_transition_pending_value = false;
    }
    else if (mode == CYHAL_SYSPM_CHECK_READY && allow)
    {
        _cyhal_i3c_pm_transition_pending_value = true;
    }
    return allow;
}

cyhal_syspm_callback_data_t _cyhal_i3c_pm_callback_data =
{
    .callback = &_cyhal_i3c_pm_callback_common,
    .states = (cyhal_syspm_callback_state_t)(CYHAL_SYSPM_CB_CPU_DEEPSLEEP | CYHAL_SYSPM_CB_CPU_DEEPSLEEP_RAM | CYHAL_SYSPM_CB_SYSTEM_HIBERNATE),
    .args = NULL,
    .next = NULL,
    .ignore_modes = (cyhal_syspm_callback_mode_t)0,
};

/* Async read/write functionality */
static cy_rslt_t _cyhal_i3c_wait_transfer_complete(cyhal_i3c_t *obj, uint32_t timeout_ms)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t timeout = timeout_ms;

    if (0 == timeout_ms)
    {
        /* Wait until async transfer will be completed */
        while (_CYHAL_I3C_PENDING_NONE != obj->pending)
        {
            cyhal_system_delay_us(1);
        }
    }
    else
    {
        /* Wait until async transfer will be completed or timeout occured */
        while (_CYHAL_I3C_PENDING_NONE != obj->pending)
        {
            if (0 == timeout)
            {
                result = CYHAL_I3C_RSLT_ERR_OPERATION_TIMEOUT;
                break;
            }
            else
            {
                cyhal_system_delay_ms(1);
                timeout--;
            }
        }
    }
    return result;
}

static void _cyhal_i3c_reset_block(cyhal_i3c_t *obj)
{
    /* We have to wait some time until reset actually will be done, because
     * FIFO and CMD buffers are unavailable during the reset procedure
     */
    uint8_t timeout_us = 10;
    if (obj->base != NULL)
    {
        /* Made soft reset of the I3C IP block */
        I3C_CORE_RESET_CTRL(obj->base) |= (I3C_CORE_RESET_CTRL_SOFT_RST_Msk |
                                           I3C_CORE_RESET_CTRL_TX_FIFO_RST_Msk | I3C_CORE_RESET_CTRL_RX_FIFO_RST_Msk |
                                           I3C_CORE_RESET_CTRL_RESP_QUEUE_RST_Msk | I3C_CORE_RESET_CTRL_CMD_QUEUE_RST_Msk);
        while (timeout_us > 0 && (0U != I3C_CORE_RESET_CTRL(obj->base)))
        {
            cyhal_system_delay_us(1);
            timeout_us--;
        }
        Cy_I3C_Resume(obj->base, &obj->context);
    }
}

/* ---------------------------------------- */


/* The PDL clears the IRQ status during Cy_I3C_Interrupt which prevents _cyhal_i3c_get_irq_obj()
 * from working properly in _cyhal_i3c_cb_wrapper on devices with muxed IRQs, because they can't tell
 * at that point which system IRQ caused the CPU IRQ. So we need to save this value at the beginning of the
 * IRQ handler when we are able to determine what it is */
static volatile cyhal_i3c_t* _cyhal_i3c_irq_obj = NULL;

static cyhal_i3c_event_t _cyhal_i3c_convert_interrupt_cause(uint32_t pdl_cause)
{
    static const uint32_t status_map1[] =
    {
        (uint32_t)CYHAL_I3C_EVENT_NONE,                         // Default no event
        (uint32_t)CYHAL_I3C_TARGET_RD_BUF_EMPTY_EVENT,           // CY_I3C_TARGET_RD_BUF_EMPTY_EVENT
    };
    uint32_t set1 = _cyhal_utils_convert_flags(status_map1, sizeof(status_map1) / sizeof(uint32_t), pdl_cause & 0xFF);

    static const uint32_t status_map2[] =
    {
        (uint32_t)CYHAL_I3C_EVENT_NONE,                         // Default no event
        (uint32_t)CYHAL_I3C_TARGET_RD_CMPLT_EVENT,               // CY_I3C_TARGET_RD_CMPLT_EVENT
        (uint32_t)CYHAL_I3C_TARGET_WR_CMPLT_EVENT,               // CY_I3C_TARGET_WR_CMPLT_EVENT
    };
    uint32_t set2 = _cyhal_utils_convert_flags(status_map2, sizeof(status_map2) / sizeof(uint32_t), pdl_cause >> 16);

    return (cyhal_i3c_event_t)(set1 | set2);
}

static inline uint32_t _cyhal_i3c_get_num_in_tx_fifo(cyhal_i3c_t* obj)
{
    return ((CY_I3C_FIFO_SIZE/4UL) - Cy_I3C_GetFreeEntriesInTxFifo(obj->base));
}


static void _cyhal_i3c_irq_handler(void)
{
    /* Save the old value and store it afterwards in case we get into a nested IRQ situation */
    /* Safe to cast away volatile because we don't expect this pointer to be changed while we're in here, they
     * just might change where the original pointer points */
    cyhal_i3c_t* old_irq_obj = (cyhal_i3c_t*)_cyhal_i3c_irq_obj;
    _cyhal_i3c_irq_obj = (cyhal_i3c_t*) _cyhal_i3c_get_irq_obj();
    cyhal_i3c_t* obj = (cyhal_i3c_t*)_cyhal_i3c_irq_obj;

    Cy_I3C_Interrupt(obj->base, &(obj->context));

    if (obj->pending)
    {
        /* This code is part of cyhal_i3c_controller_transfer_async() API functionality */
        /* cyhal_i3c_controller_transfer_async() API uses this interrupt handler for RX transfer */
        cy_rslt_t result = CY_RSLT_SUCCESS;

        /* Check if TX is completed and run RX in case when TX and RX are enabled */
        if (obj->pending == _CYHAL_I3C_PENDING_TX_RX)
        {
            /* Start RX transfer */
            obj->pending = _CYHAL_I3C_PENDING_RX;
            result = Cy_I3C_ControllerRead(obj->base, &obj->rx_config, &obj->context);
            if (CY_RSLT_SUCCESS != result)
            {
                Cy_I3C_Resume(obj->base, &obj->context);
            }
        }
        else
        {
            /* Finish async TX or RX separate transfer */
            obj->pending = _CYHAL_I3C_PENDING_NONE;
        }
    }

    _cyhal_i3c_irq_obj = old_irq_obj;
}

static void _cyhal_i3c_cb_wrapper(uint32_t event)
{
    /* Safe to cast away volatile because we don't expect this pointer to be changed while we're in here, they
     * just might change where the original pointer points */
    cyhal_i3c_t *obj = (cyhal_i3c_t*)_cyhal_i3c_irq_obj;
    cyhal_i3c_event_t anded_events = (cyhal_i3c_event_t)(obj->irq_cause & (uint32_t)_cyhal_i3c_convert_interrupt_cause(event));
    if (anded_events)
    {
        /* Indicates read/write operations will be in a callback */
        obj->op_in_callback = true;
        cyhal_i3c_event_callback_t callback = (cyhal_i3c_event_callback_t) obj->callback_data.callback;
        callback(obj->callback_data.callback_arg, anded_events);
        obj->op_in_callback = false;
    }
}

static void _cyhal_i3c_ibi_wrapper(cy_stc_i3c_ibi_t* ibi)
{
    /* Not implemented yet */

    cyhal_i3c_t *obj = (cyhal_i3c_t*)_cyhal_i3c_irq_obj;
    cyhal_i3c_ibi_callback_t callback = (cyhal_i3c_ibi_callback_t) obj->in_band_callback_data.callback;
    callback(obj->callback_data.callback_arg, (cyhal_i3c_ibi_event_t)ibi->event, ibi->targetAddress);
}

static bool _cyhal_i3c_pm_callback_instance(void *obj_ptr, cyhal_syspm_callback_state_t state, cy_en_syspm_callback_mode_t pdl_mode)
{
    cyhal_i3c_t *obj = (cyhal_i3c_t*)obj_ptr;
    cy_stc_syspm_callback_params_t i2c_callback_params = {
        .base = (void *) (obj->base),
        .context = (void *) &(obj->context)
    };
    bool allow = true;

    if (CYHAL_SYSPM_CB_CPU_DEEPSLEEP == state)
        allow = (CY_SYSPM_SUCCESS == Cy_I3C_DeepSleepCallback(&i2c_callback_params, pdl_mode));
    else if (CYHAL_SYSPM_CB_SYSTEM_HIBERNATE == state)
        allow = (CY_SYSPM_SUCCESS == Cy_I3C_HibernateCallback(&i2c_callback_params, pdl_mode));

    return allow;
}

void _cyhal_i3c_update_instance_data(uint8_t block_num, void *obj, cyhal_i3c_instance_pm_callback pm_callback)
{
    uint8_t i3c_arr_index = _cyhal_i3c_get_block_index(block_num);

    _cyhal_i3c_config_objects[i3c_arr_index] = obj;
    _cyhal_i3c_config_pm_callback[i3c_arr_index] = pm_callback;

    int count = 0;
    for (uint8_t i = 0; i < _CYHAL_I3C_ARRAY_SIZE; i++)
    {
        if (NULL != _cyhal_i3c_config_objects[i])
        {
            if (count == 1)
            {
                return;
            }
            count++;
        }
    }

    if (count == 0)
    {
        CY_ASSERT(obj == NULL);
        #if (CYHAL_DRIVER_AVAILABLE_SYSPM)
        _cyhal_syspm_unregister_peripheral_callback(&_cyhal_i3c_pm_callback_data);
        #endif
    }
    else if (count == 1 && obj != NULL)
    {
        #if (CYHAL_DRIVER_AVAILABLE_SYSPM)
        _cyhal_syspm_register_peripheral_callback(&_cyhal_i3c_pm_callback_data);
        #endif
    }
}

cy_rslt_t cyhal_i3c_controller_attach_targets(cyhal_i3c_t *obj, cyhal_i3c_device_info_t* dev_list, uint32_t dev_list_size)
{
    uint8_t i;
    cy_rslt_t status = CY_RSLT_SUCCESS;

    if (dev_list_size > CYHAL_I3C_MAX_ATTACHED_DEVICES)
    {
        return CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
    }

    /* Send Reset Dynamic Address CCC command to the bus */
    cy_stc_i3c_ccc_cmd_t cccCmd;
    cy_stc_i3c_ccc_payload_t payload;
    cccCmd.address = CYHAL_I3C_BROADCAST_ADDR;
    cccCmd.cmd = CY_I3C_CCC_RSTDAA(true);
    cccCmd.data = &payload;
    cccCmd.data->data = NULL;
    cccCmd.data->len = 0U;
    status = Cy_I3C_SendCCCCmd(obj->base, &cccCmd, &obj->context);

    if (NULL != dev_list)
    {
        for(i = 0; i < dev_list_size; i++)
        {
            if (CYHAL_I3C_TARGET_TYPE_I2C == dev_list[i].device_type)
            {
                cy_stc_i2c_device_t i2c_device = {
                    .staticAddress = dev_list[i].static_address,
                    .lvr = 0,
                };
                status = Cy_I3C_ControllerAttachI2CDevice(obj->base, &i2c_device, &obj->context);
                if(status != CY_I3C_SUCCESS)
                {
                    break;
                }
            }
            else
            {
                cy_stc_i3c_device_t i3c_device = {
                    .staticAddress = dev_list[i].static_address,
                    .dynamicAddress = dev_list[i].dynamic_address,
                };
                status = Cy_I3C_ControllerAttachI3CDevice(obj->base, &i3c_device, &obj->context);
                if(status != CY_I3C_SUCCESS)
                {
                    break;
                }
                else
                {
                    if (0 == dev_list[i].dynamic_address)
                    {
                        dev_list[i].dynamic_address = obj->context.i3cController.lastAddress;
                    }
                }
            }
        }
    }
    if (status == CY_I3C_SUCCESS)
    {
        status = Cy_I3C_ControllerStartEntDaa(obj->base, &obj->context);
    }

    if (CY_I3C_SUCCESS != status)
    {
        _cyhal_i3c_reset_block(obj);
    }
    return status;
}

static cy_rslt_t _cyhal_i3c_init_resources(cyhal_i3c_t *obj, cyhal_gpio_t sda, cyhal_gpio_t scl, const cyhal_clock_t *clk)
{
    /* Explicitly marked not allocated resources as invalid to prevent freeing them. */
    obj->resource.type = CYHAL_RSC_INVALID;
    obj->pin_scl = CYHAL_NC_PIN_VALUE;
    obj->pin_sda = CYHAL_NC_PIN_VALUE;
    obj->is_clock_owned = false;

    // pins_blocks will contain bit representation of blocks, that are connected to specified pin
    // 1 block - 1 bit, so, for example, pin_blocks = 0x00000006 means that certain pin
    // can belong to next non-reserved blocks I3C2 and I3C1
    uint32_t pins_blocks = _CYHAL_I3C_AVAILABLE_BLOCKS_MASK;
    if (NC != sda)
    {
        pins_blocks &= _cyhal_i3c_check_pin_affiliation(sda, cyhal_pin_map_i3c_i3c_sda, sizeof(cyhal_pin_map_i3c_i3c_sda)/sizeof(cyhal_resource_pin_mapping_t));
    }
    if (NC != scl)
    {
        pins_blocks &= _cyhal_i3c_check_pin_affiliation(scl, cyhal_pin_map_i3c_i3c_scl, sizeof(cyhal_pin_map_i3c_i3c_scl)/sizeof(cyhal_resource_pin_mapping_t));
    }

    // One (or more) pin does not belong to any I3C instance or all corresponding I3C instances
    // are reserved
    if (0 == pins_blocks)
    {
        return CYHAL_I3C_RSLT_ERR_INVALID_PIN;
    }

    uint8_t found_block_idx = 0;
    while(((pins_blocks >> found_block_idx) & 0x1) == 0)
    {
        found_block_idx++;
    }

    cyhal_resource_inst_t i2c_rsc = { CYHAL_RSC_I3C, found_block_idx, 0 };

    /* Reserve the I3C */
    const cyhal_resource_pin_mapping_t *sda_map = _cyhal_i3c_find_map(sda, cyhal_pin_map_i3c_i3c_sda, sizeof(cyhal_pin_map_i3c_i3c_sda)/sizeof(cyhal_resource_pin_mapping_t), &i2c_rsc);
    const cyhal_resource_pin_mapping_t *scl_map = _cyhal_i3c_find_map(scl, cyhal_pin_map_i3c_i3c_scl, sizeof(cyhal_pin_map_i3c_i3c_scl)/sizeof(cyhal_resource_pin_mapping_t), &i2c_rsc);
    uint8_t i3c_arr_index = 0;

    if ((NULL == sda_map) || (NULL == scl_map) || !_cyhal_utils_map_resources_equal(sda_map, scl_map))
    {
        return CYHAL_I3C_RSLT_ERR_INVALID_PIN;
    }

    cy_rslt_t result = cyhal_hwmgr_reserve(&i2c_rsc);

    if (result == CY_RSLT_SUCCESS)
    {
        i3c_arr_index = _cyhal_i3c_get_block_index(i2c_rsc.block_num);
        obj->base = _CYHAL_I3C_BASE_ADDRESSES[i3c_arr_index];
        obj->resource = i2c_rsc;

        result = _cyhal_utils_reserve_and_connect(sda_map, (uint8_t)CYHAL_PIN_MAP_DRIVE_MODE_I3C_I3C_SDA);
        if (result == CY_RSLT_SUCCESS)
            obj->pin_sda = sda;
    }

    if (result == CY_RSLT_SUCCESS)
    {
        result = _cyhal_utils_reserve_and_connect(scl_map, (uint8_t)CYHAL_PIN_MAP_DRIVE_MODE_I3C_I3C_SCL);
        if (result == CY_RSLT_SUCCESS)
            obj->pin_scl = scl;
    }

    if (result == CY_RSLT_SUCCESS)
    {
        if (clk == NULL)
        {
            result = _cyhal_utils_allocate_clock(&(obj->clock), &(obj->resource), CYHAL_CLOCK_BLOCK_PERIPHERAL_8BIT, true);
            obj->is_clock_owned = (result == CY_RSLT_SUCCESS);
        }
        else
        {
            obj->clock = *clk;
        }
    }

    if (result == CY_RSLT_SUCCESS)
    {
        result = _cyhal_i3c_set_peri_divider(obj, _CYHAL_I3C_CONTROLLER_DEFAULT_PUSHPULL_FREQ_HZ);
    }

    return result;
}

static cy_rslt_t _cyhal_i3c_init_hw(cyhal_i3c_t *obj, const cy_stc_i3c_config_t *pdl_cfg)
{
    CY_ASSERT(NULL != obj->base);

    /* Initial value for async operations */
    obj->pending = _CYHAL_I3C_PENDING_NONE;
    /* Initial value for read/write operations in callback */
    obj->op_in_callback = false;

    obj->rx_target_buff = _cyhal_i3c_buff_info_default;
    obj->tx_target_buff = _cyhal_i3c_buff_info_default;

    /* Configure I3C to operate */
    _cyhal_utils_enable_ip_block(&obj->resource);
    cy_rslt_t result = (cy_rslt_t)Cy_I3C_Init(obj->base, pdl_cfg, &(obj->context));
    uint8_t i3c_arr_index = _cyhal_i3c_get_block_index(obj->resource.block_num);

    if (result == CY_RSLT_SUCCESS)
    {
        _cyhal_i3c_update_instance_data(obj->resource.block_num, (void*)obj, &_cyhal_i3c_pm_callback_instance);
        /* Enable I3C to operate */
        Cy_I3C_Enable(obj->base);
        Cy_I3C_Resume(obj->base, &obj->context);

        obj->callback_data.callback = NULL;
        obj->callback_data.callback_arg = NULL;
        obj->irq_cause = CYHAL_I3C_EVENT_NONE;

        _cyhal_irq_register(_CYHAL_I3C_IRQ_N[i3c_arr_index], CYHAL_ISR_PRIORITY_DEFAULT, (cy_israddress)_cyhal_i3c_irq_handler);
        _cyhal_irq_enable(_CYHAL_I3C_IRQ_N[i3c_arr_index]);
    }

    return result;
}

static cy_rslt_t _cyhal_i3c_controller_async_transfer(cyhal_i3c_t *obj, uint16_t address, const void *tx, size_t tx_size, bool tx_full_transfer, void *rx, size_t rx_size, bool rx_full_transfer)
{
    if (_cyhal_i3c_pm_transition_pending())
        return CYHAL_SYSPM_RSLT_ERR_PM_PENDING;

    cy_rslt_t result = CY_RSLT_SUCCESS;

    obj->rx_config.targetAddress = (uint8_t)address;
    obj->tx_config.targetAddress = (uint8_t)address;

    obj->rx_config.buffer = rx;
    obj->rx_config.bufferSize = rx_size;
    obj->rx_config.toc = rx_full_transfer;

    obj->tx_config.buffer = (void *)tx;
    obj->tx_config.bufferSize = tx_size;
    obj->tx_config.toc = tx_full_transfer;

    if (!obj->pending)
    {
        /* Validate input data and do appropriate action */
        if (tx_size)
        {
            obj->pending = (rx_size)
                ? _CYHAL_I3C_PENDING_TX_RX
                : _CYHAL_I3C_PENDING_TX;
            result = Cy_I3C_ControllerWrite(obj->base, &obj->tx_config, &obj->context);
            if (CY_RSLT_SUCCESS != result)
            {
                Cy_I3C_Resume(obj->base, &obj->context);
            }
            /* Receive covered by interrupt handler - _cyhal_i3c_irq_handler() */
        }
        else if (rx_size)
        {
            obj->pending = _CYHAL_I3C_PENDING_RX;
            result = Cy_I3C_ControllerRead(obj->base, &obj->rx_config, &obj->context);
            if (CY_RSLT_SUCCESS != result)
            {
                Cy_I3C_Resume(obj->base, &obj->context);
            }
        }
        else
        {
            result = CYHAL_I3C_RSLT_ERR_TX_RX_BUFFERS_ARE_EMPTY;
        }
    }
    else
    {
        result = CYHAL_I3C_RSLT_ERR_PREVIOUS_ASYNC_PENDING;
    }
    return result;
}

/* Start API implementing */
cy_rslt_t cyhal_i3c_init(cyhal_i3c_t *obj, cyhal_gpio_t sda, cyhal_gpio_t scl, const cyhal_clock_t *clk)
{
    CY_ASSERT(NULL != obj);
    obj->dc_configured = false;
    memset(obj, 0, sizeof(cyhal_i3c_t));

    cy_rslt_t result = _cyhal_i3c_init_resources(obj, sda, scl, clk);
    if (CY_RSLT_SUCCESS == result)
    {
        /* Use updated configuration structure with I3C clock setting */
        cy_stc_i3c_config_t _config_structure = _cyhal_i3c_default_config;
        _config_structure.i3cClockHz = cyhal_clock_get_frequency(&(obj->clock));
        _config_structure.i3cSclRate = _CYHAL_I3C_CONTROLLER_DEFAULT_PUSHPULL_FREQ_HZ;
        _config_structure.openDrainSclRate = _CYHAL_I3C_CONTROLLER_DEFAULT_OPENDRAIN_FREQ_HZ;

        result = _cyhal_i3c_init_hw(obj, &_config_structure);
    }
    if (CY_RSLT_SUCCESS != result)
    {
        cyhal_i3c_free(obj);
    }
    return result;
}

cy_rslt_t cyhal_i3c_init_cfg(cyhal_i3c_t *obj, const cyhal_i3c_configurator_t *cfg)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != cfg);
    CY_ASSERT(NULL != cfg->config);

    memset(obj, 0, sizeof(cyhal_i3c_t));

    obj->dc_configured = true;
    obj->resource = *cfg->resource;
    obj->clock = *cfg->clock;
    obj->is_clock_owned = false;

    uint8_t i3c_arr_index = _cyhal_i3c_get_block_index(obj->resource.block_num);
    obj->base = _CYHAL_I3C_BASE_ADDRESSES[i3c_arr_index];

    return _cyhal_i3c_init_hw(obj, cfg->config);
}

void cyhal_i3c_free(cyhal_i3c_t *obj)
{
    CY_ASSERT(NULL != obj);

    if (NULL != obj->base)
    {
        _cyhal_i3c_reset_block(obj);
        Cy_I3C_DeInit(obj->base, &obj->context);
        obj->base = NULL;
    }

    if (CYHAL_RSC_INVALID != obj->resource.type)
    {
        uint8_t i3c_arr_index = _cyhal_i3c_get_block_index(obj->resource.block_num);
        _cyhal_i3c_update_instance_data(obj->resource.block_num, NULL, NULL);
        _cyhal_system_irq_t irqn = _CYHAL_I3C_IRQ_N[i3c_arr_index];
        _cyhal_irq_free(irqn);

        if (!obj->dc_configured)
        {
            cyhal_hwmgr_free(&(obj->resource));
        }

        obj->resource.type = CYHAL_RSC_INVALID;
    }

    if (!obj->dc_configured)
    {
        _cyhal_utils_release_if_used(&(obj->pin_sda));
        _cyhal_utils_release_if_used(&(obj->pin_scl));

        if (obj->is_clock_owned)
        {
            cyhal_clock_free(&(obj->clock));
        }
    }
}

cy_rslt_t cyhal_i3c_configure(cyhal_i3c_t *obj, const cyhal_i3c_cfg_t *cfg)
{
    CY_ASSERT(NULL != obj);
    CY_ASSERT(NULL != cfg);

    cy_stc_i3c_config_t _config_structure = _cyhal_i3c_default_config;

    if (((CYHAL_I3C_MODE_COMBINED == cfg->i3c_bus_mode) && ((0 == cfg->i2c_data_rate) || (0 == cfg->i3c_data_rate))) ||
       ((CYHAL_I3C_MODE_PURE == cfg->i3c_bus_mode) && (0 == cfg->i3c_data_rate)))
    {
        return CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
    }

#if defined(CY_DEVICE_PSE84)
    /* PSE84 A0 supports only Controller mode */
    if (CYHAL_I3C_MODE_CONTROLLER != cfg->i3c_mode)
    {
        return CYHAL_I3C_RSLT_ERR_UNSUPPORTED;
    }
#endif /* defined(CY_DEVICE_PSE84) */

    _config_structure.i3cMode = (cy_en_i3c_mode_t)cfg->i3c_mode;
    if (CYHAL_I3C_MODE_CONTROLLER != cfg->i3c_mode)
    {
        _config_structure.staticAddress = cfg->target_address;
    }
    cy_rslt_t result = _cyhal_i3c_set_peri_divider(obj, cfg->i3c_data_rate);

    if (CY_RSLT_SUCCESS == result)
    {
        _config_structure.i3cClockHz = cyhal_clock_get_frequency(&(obj->clock));
        _config_structure.i3cSclRate = cfg->i3c_data_rate;
        _config_structure.openDrainSclRate = cfg->i2c_data_rate;

        _cyhal_utils_enable_ip_block(&obj->resource);
        result = (cy_rslt_t)Cy_I3C_Init(obj->base, &_config_structure, &(obj->context));
    }

    if (CY_RSLT_SUCCESS == result)
    {
        /* Set data rate */
        uint32_t dataRate = _cyhal_i3c_set_data_rate(obj, cfg->i3c_data_rate, (CYHAL_I3C_MODE_TARGET == cfg->i3c_mode));
        if (dataRate == 0)
        {
            /* Can not reach desired data rate */
            return CYHAL_I3C_RSLT_ERR_CAN_NOT_REACH_DR;
        }

        Cy_I3C_Enable(obj->base);
        Cy_I3C_Resume(obj->base, &obj->context);
    }
    return result;
}

cy_rslt_t cyhal_i3c_controller_write(cyhal_i3c_t *obj, uint16_t dev_addr, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (_cyhal_i3c_pm_transition_pending())
        return CYHAL_SYSPM_RSLT_ERR_PM_PENDING;

    CY_UNUSED_PARAMETER(timeout);

    cy_rslt_t status = CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;

    while (size > 0)
    {
        status = Cy_I3C_ControllerWriteByte(obj->base, dev_addr, *data, &obj->context);
        if (status != CY_I3C_SUCCESS)
        {
            Cy_I3C_Resume(obj->base, &obj->context);
            break;
        }
        --size;
        ++data;
    }

    return status;
}

cy_rslt_t cyhal_i3c_controller_read(cyhal_i3c_t *obj, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (_cyhal_i3c_pm_transition_pending())
        return CYHAL_SYSPM_RSLT_ERR_PM_PENDING;

    CY_UNUSED_PARAMETER(timeout);

    cy_rslt_t status = CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;

    while (size > 0) {
        status = Cy_I3C_ControllerReadByte(obj->base, dev_addr, (uint8_t *)data, &obj->context);
        if (status != CY_I3C_SUCCESS)
        {
            Cy_I3C_Resume(obj->base, &obj->context);
            break;
        }
        --size;
        ++data;
    }

    return status;
}

cy_rslt_t cyhal_i3c_target_config_write_buffer(cyhal_i3c_t *obj, const uint8_t *data, uint16_t size)
{
    cy_rslt_t result = CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
    if (size > 0 && data != NULL)
    {
        if (obj->context.state == CY_I3C_IDLE)
        {
            /* Casting the const away in this case is safe, because the data buffer is not modified by HAL */
            Cy_I3C_TargetConfigWriteBuf(obj->base, (uint8_t *)data, size, &obj->context);
            obj->rx_target_buff.addr.u8 = (uint8_t *)data;
            obj->rx_target_buff.size = size;
            result = CY_RSLT_SUCCESS;
        }
        else
        {
            result = CYHAL_I3C_RSLT_WARN_DEVICE_BUSY;
        }
    }
    return result;
}

cy_rslt_t cyhal_i3c_target_config_read_buffer(cyhal_i3c_t *obj, uint8_t *data, uint16_t size)
{
    cy_rslt_t result = CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
    if (size > 0 && data != NULL)
    {
        if (obj->context.state == CY_I3C_IDLE)
        {
            Cy_I3C_TargetConfigReadBuf(obj->base, data, size, &obj->context);
            obj->tx_target_buff.addr.u8 = data;
            obj->tx_target_buff.size = size;
            result = CY_RSLT_SUCCESS;
        }
        else
        {
            result = CYHAL_I3C_RSLT_WARN_DEVICE_BUSY;
        }
    }
    return result;
}

cy_rslt_t cyhal_i3c_controller_mem_write(cyhal_i3c_t *obj, uint16_t address, uint16_t mem_addr, uint16_t mem_addr_size, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (_cyhal_i3c_pm_transition_pending())
        return CYHAL_SYSPM_RSLT_ERR_PM_PENDING;
    cy_rslt_t status;
    uint8_t mem_addr_buf[4];
    uint8_t mem_addr_ind = 0;
    uint16_t curr_data_size = size;
    uint8_t *data_ptr = (uint8_t *)data;

    if (mem_addr_size == 1)
    {
        mem_addr_buf[mem_addr_ind] = (uint8_t)mem_addr;
        mem_addr_ind++;
    }
    else if (mem_addr_size == 2)
    {
        mem_addr_buf[mem_addr_ind] = (uint8_t)(mem_addr >> 8);
        mem_addr_ind++;
        mem_addr_buf[mem_addr_ind] = (uint8_t)mem_addr;
        mem_addr_ind++;
    }
    else
    {
        return CYHAL_I3C_RSLT_ERR_INVALID_ADDRESS_SIZE;
    }

    while (mem_addr_ind < 4 && curr_data_size > 0)
    {
        mem_addr_buf[mem_addr_ind] = *data_ptr;
        mem_addr_ind++;
        curr_data_size--;
        data_ptr++;
    }

    if ((mem_addr_size + size) >= 4)
    {
        status = _cyhal_i3c_controller_async_transfer(obj, address, mem_addr_buf, mem_addr_ind, false, NULL, 0, true);
        if (status == CY_RSLT_SUCCESS)
        {
            status = _cyhal_i3c_controller_async_transfer(obj, address, data_ptr, curr_data_size, true, NULL, 0, true);
        }
    }
    else
    {
        status = _cyhal_i3c_controller_async_transfer(obj, address, mem_addr_buf, mem_addr_ind, false, NULL, 0, true);
    }

    if (status == CY_RSLT_SUCCESS)
    {
        status = _cyhal_i3c_wait_transfer_complete(obj, timeout);
    }

    if (CY_RSLT_SUCCESS != status)
    {
        _cyhal_i3c_reset_block(obj);
    }

    return status;
}

cy_rslt_t cyhal_i3c_controller_mem_read(cyhal_i3c_t *obj, uint16_t address, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (_cyhal_i3c_pm_transition_pending())
        return CYHAL_SYSPM_RSLT_ERR_PM_PENDING;

    cy_rslt_t status;
    uint8_t mem_addr_buf[2];
    if (mem_addr_size == 1)
    {
        mem_addr_buf[0] = (uint8_t)mem_addr;
    }
    else if (mem_addr_size == 2)
    {
        mem_addr_buf[0] = (uint8_t)(mem_addr >> 8);
        mem_addr_buf[1] = (uint8_t)mem_addr;
    }
    else
    {
        return CYHAL_I3C_RSLT_ERR_INVALID_ADDRESS_SIZE;
    }

    status = _cyhal_i3c_controller_async_transfer(obj, address, (const void *)mem_addr_buf, mem_addr_size, true, data, size, true);
    if (status == CY_RSLT_SUCCESS)
    {
        status = _cyhal_i3c_wait_transfer_complete(obj, timeout);
    }
    return status;
}

cy_rslt_t cyhal_i3c_controller_transfer_async(cyhal_i3c_t *obj, uint16_t address, const void *tx, size_t tx_size, void *rx, size_t rx_size)
{
    return _cyhal_i3c_controller_async_transfer(obj, address, tx, tx_size, true, rx, rx_size, true);
}

cy_rslt_t cyhal_i3c_controller_abort_async(cyhal_i3c_t *obj)
{
    uint16_t timeout_us = 10000;
    if (obj->pending != _CYHAL_I3C_PENDING_NONE)
    {
        if (obj->pending == _CYHAL_I3C_PENDING_RX)
        {
            Cy_I3C_ControllerAbortTransfer(obj->base, &obj->context);
        }
        /* After abort, next I3C operation can be initiated only after CY_I3C_CONTROLLER_BUSY is cleared,
        *   so waiting for that event to occur. */
        while ((CY_I3C_CONTROLLER_BUSY & obj->context.controllerStatus) && (timeout_us != 0))
        {
            cyhal_system_delay_us(1);
            timeout_us--;
        }
        if (0 == timeout_us)
        {
            return CYHAL_I3C_RSLT_ERR_OPERATION_TIMEOUT;
        }
        obj->pending = _CYHAL_I3C_PENDING_NONE;
    }
    return CY_RSLT_SUCCESS;
}

void cyhal_i3c_register_callback(cyhal_i3c_t *obj, cyhal_i3c_event_callback_t callback, void *callback_arg)
{
    uint32_t savedIntrStatus = cyhal_system_critical_section_enter();
    obj->callback_data.callback = (cy_israddress) callback;
    obj->callback_data.callback_arg = callback_arg;
    cyhal_system_critical_section_exit(savedIntrStatus);
    Cy_I3C_RegisterEventCallback(obj->base, _cyhal_i3c_cb_wrapper, &(obj->context));
}

void cyhal_i3c_register_ibi_callback(cyhal_i3c_t *obj, cyhal_i3c_ibi_callback_t callback, void *callback_arg)
{
    uint32_t savedIntrStatus = cyhal_system_critical_section_enter();
    obj->in_band_callback_data.callback = (cy_israddress) callback;
    obj->in_band_callback_data.callback_arg = callback_arg;
    cyhal_system_critical_section_exit(savedIntrStatus);
    Cy_I3C_RegisterIbiCallback(obj->base, _cyhal_i3c_ibi_wrapper, &(obj->context));
    obj->ibi_irq_cause = CYHAL_I3C_IBI_NONE;
}

void cyhal_i3c_enable_event(cyhal_i3c_t *obj, cyhal_i3c_event_t event, uint8_t intr_priority, bool enable)
{
    uint8_t i3c_arr_index = _cyhal_i3c_get_block_index(obj->resource.block_num);
    if (enable)
    {
        obj->irq_cause |= event;
    }
    else
    {
        obj->irq_cause &= ~event;
    }

    _cyhal_system_irq_t irqn = _CYHAL_I3C_IRQ_N[i3c_arr_index];
    _cyhal_irq_set_priority(irqn, intr_priority);
}

cy_rslt_t cyhal_i3c_set_fifo_level(cyhal_i3c_t *obj, cyhal_i3c_fifo_type_t type, uint16_t level)
{
    if(type == CYHAL_I3C_FIFO_RX)
    {
        Cy_I3C_SetRxFifoLevel(obj->base, level);
        return CY_RSLT_SUCCESS;
    }
    else if(type == CYHAL_I3C_FIFO_TX)
    {
        Cy_I3C_SetTxEmptyThldLevel(obj->base, level);
        return CY_RSLT_SUCCESS;
    }

    return CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
}

cy_rslt_t cyhal_i3c_enable_output(cyhal_i3c_t *obj, cyhal_i3c_output_t output, cyhal_source_t *source)
{
    // This just returns a proper cyhal_source_t. Use cyhal_i3c_set_fifo_level
    // to actually set level.
    cyhal_internal_source_t src_int;
    uint8_t i3c_arr_index = _cyhal_i3c_get_block_index(obj->resource.block_num);
    if(output == CYHAL_I3C_OUTPUT_TRIGGER_RX_FIFO_LEVEL_REACHED)
    {
        src_int = (cyhal_internal_source_t)(_CYHAL_TRIGGER_I3C_TR_RX_REQ + i3c_arr_index);
        *source = (cyhal_source_t)_CYHAL_TRIGGER_CREATE_SOURCE(src_int, CYHAL_SIGNAL_TYPE_EDGE);
        return CY_RSLT_SUCCESS;
    }
    // This just returns a proper cyhal_source_t. Use cyhal_i3c_set_fifo_level
    // to actually set level.
    else if(output == CYHAL_I3C_OUTPUT_TRIGGER_TX_FIFO_LEVEL_REACHED)
    {
        src_int = (cyhal_internal_source_t)(_CYHAL_TRIGGER_I3C_TR_TX_REQ + i3c_arr_index);
        *source = (cyhal_source_t)_CYHAL_TRIGGER_CREATE_SOURCE(src_int, CYHAL_SIGNAL_TYPE_EDGE);
        return CY_RSLT_SUCCESS;
    }
    return CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
}

cy_rslt_t cyhal_i3c_disable_output(cyhal_i3c_t *obj, cyhal_i3c_output_t output)
{
    CY_UNUSED_PARAMETER(obj);

    if (output == CYHAL_I3C_OUTPUT_TRIGGER_RX_FIFO_LEVEL_REACHED ||
        output == CYHAL_I3C_OUTPUT_TRIGGER_TX_FIFO_LEVEL_REACHED)
    {
        return CY_RSLT_SUCCESS;
    }
    else
    {
        return CYHAL_I3C_RSLT_ERR_BAD_ARGUMENT;
    }
}

uint32_t cyhal_i3c_target_readable(cyhal_i3c_t *obj)
{
    return Cy_I3C_TargetGetWriteTransferCount(obj->base, &obj->context);
}

uint32_t cyhal_i3c_target_writable(cyhal_i3c_t *obj)
{
    return (obj->tx_target_buff.size - Cy_I3C_TargetGetReadTransferCount(obj->base, &obj->context));
}


#if defined(__cplusplus)
}
#endif

#endif /* CYHAL_DRIVER_AVAILABLE_I3C */
