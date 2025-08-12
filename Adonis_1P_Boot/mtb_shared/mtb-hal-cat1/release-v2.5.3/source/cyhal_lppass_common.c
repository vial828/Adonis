/***************************************************************************//**
* \file cyhal_lppass_common.c
*
* \brief
* Provides common functionality that needs to be shared among all drivers that
* interact with the Programmable Analog Subsystem.
*
********************************************************************************
* \copyright
* Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company) or
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
#include "cyhal_analog_common.h"
#include "cyhal_clock.h"
#include "cyhal_irq_impl.h"
#include "cyhal_syspm.h"

#if (CY_IP_MXS22LPPASS)
#include "cyhal_lppass_common.h"

static _cyhal_lppass_irq_handler_t _cyhal_lppass_irq_handlers[((size_t)_CYHAL_LPPASS_IRQ_MAX) + 1] = { 0 };

//Shared CTB configurations between Opamps and Comps
static cy_stc_autanalog_ctb_dyn_t _cyhal_lppass_ctb_dyn_cfg[CY_IP_MXS22LPPASS_CTB_INSTANCES*_CYHAL_OPAMP_OA_NUM_PER_CTB];
static cy_stc_autanalog_ctb_sta_t _cyhal_lppass_ctb_static_cfg[CY_IP_MXS22LPPASS_CTB_INSTANCES];

#define _CYHAL_LPPASS_STT_TOTAL_NUM         (4)

/* TBD - these should mostly just turn on/off */
cy_stc_autanalog_stt_prb_t _cyhal_lppass_prb_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] = {NULL};

cy_stc_autanalog_stt_ptcomp_t _cyhal_lppass_ptcomp_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] = {NULL};

/* The ADC State Transition Table */
static cy_stc_autanalog_stt_sar_t _cyhal_lppass_sar_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] =
{
    /* State#0, start-up/settling delay */
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .entryState = 0U,
    },
    /* State#1, Exernal trigger SAR conversion (used for one-shot) */
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .entryState = 0U,
    },
    /* State#2, SAR conversion (used for both continuous and one-shot) */
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = true, // Trigger is active
        .entryState = 0U,   // Sequencer Entry State value
    },
    /* State#3, return to state #1 (single/standby SAR) or state #2 (continuous SAR) */
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .entryState = 0U,
    },
};

/* The DAC State Transition Table */
static cy_stc_autanalog_stt_dac_t _cyhal_lppass_dac_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] =
{
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .channel    = 1U,
        .direction  = CY_AUTANALOG_DAC_DIRECTION_FORWARD,
    },

    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .channel    = 1U,
        .direction  = CY_AUTANALOG_DAC_DIRECTION_FORWARD,
    },
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .channel    = 1U,
        .direction  = CY_AUTANALOG_DAC_DIRECTION_FORWARD,
    },
    {
        .unlock     = true,
        .enable     = true,
        .trigger    = false,
        .channel    = 1U,
        .direction  = CY_AUTANALOG_DAC_DIRECTION_FORWARD,
    },
};

/* The CTB State Transition Table */
cy_stc_autanalog_stt_ctb_t _cyhal_lppass_ctb0_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] = 
{
    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 0,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 1,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },

    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 2,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 3,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 2,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 3,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 2,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 3,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
};

/* The CTB State Transition Table */
cy_stc_autanalog_stt_ctb_t _cyhal_lppass_ctb1_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] = 
{
    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 0,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 1,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },

    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 2,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 3,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 2,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 3,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
    {
        .unlock       = true,
        .enableOpamp0 = false,
        .cfgOpamp0    = 2,
        .gainOpamp0   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
        .enableOpamp1 = false,
        .cfgOpamp1    = 3,
        .gainOpamp1   = CY_AUTANALOG_STT_CTB_OA_GAIN_1_00,
    },
};

/* The PTComp State Transition Table */
#if (PASS_NR_PTCS) > 0
cy_stc_autanalog_stt_ptcomp_t _cyhal_lppass_ptc0_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] = 
{
    {
        .unlock         = true,
        .enableComp0    = false,
        .dynCfgIdxComp0 = 0,
        .enableComp1    = false,
        .dynCfgIdxComp1 = 1,
    },
    {
        .unlock         = false,
        .enableComp0    = false,
        .dynCfgIdxComp0 = 0,
        .enableComp1    = false,
        .dynCfgIdxComp1 = 1,
    },
    {
        .unlock         = false,
        .enableComp0    = false,
        .dynCfgIdxComp0 = 0,
        .enableComp1    = false,
        .dynCfgIdxComp1 = 1,
    },
    {
        .unlock         = false,
        .enableComp0    = false,
        .dynCfgIdxComp0 = 0,
        .enableComp1    = false,
        .dynCfgIdxComp1 = 1,
    },
};
#endif
#if (PASS_NR_PTCS) > 1
#error "Unhandled PTC count"
#endif


static cy_stc_autanalog_stt_ac_t _cyhal_lppass_ac_stt[_CYHAL_LPPASS_STT_TOTAL_NUM] =
{
    /* State#0, start-up/settling delay */
    {
        .unlock      = true,
        .lpMode      = false,
        .condition   = CY_AUTANALOG_STT_AC_CONDITION_BLOCK_READY,
        .action      = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = _CYHAL_LPPASS_STT_START + 1,
        .trigInt     = false,
        .count       = 0UL,
        .gpioOut     = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    /* State#1, Exernal trigger SAR conversion (used for one-shot) */
    {
        .unlock      = true,
        .lpMode      = false,
        .condition   = CY_AUTANALOG_STT_AC_CONDITION_TR_AUTANALOG_IN0,
        .action      = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = _CYHAL_LPPASS_STT_TRIGGER + 1,
        .trigInt     = false,
        .count       = 0UL,
        .gpioOut     = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    /* State#2, SAR conversion (used for both continuous and one-shot) */
    {
        .unlock      = true,
        .lpMode      = false,
        .condition   = CY_AUTANALOG_STT_AC_CONDITION_SAR_DONE,
        .action      = CY_AUTANALOG_STT_AC_ACTION_WAIT_FOR,
        .branchState = _CYHAL_LPPASS_STT_ACQUISITION + 1,
        .trigInt     = false,
        .count       = 0UL,
        .gpioOut     = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
    /* State#3, return to state #1 (single/standby SAR) or state #2 (continuous SAR) */
    {
        .unlock      = true,
        .lpMode      = false,
        .condition   = CY_AUTANALOG_STT_AC_CONDITION_TRUE,
        .action      = CY_AUTANALOG_STT_AC_ACTION_BRANCH_IF_TRUE,
        .branchState = _CYHAL_LPPASS_STT_TRIGGER, // Update this to switch between single/continuous mode
        .trigInt     = false,
        .count       = 0L,
        .gpioOut     = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    },
};

static cy_stc_autanalog_ac_t _cyhal_lppass_ctb_ac_cfg =
{
    /* Control GPIO output */
    .gpioOutEn   = CY_AUTANALOG_STT_AC_GPIO_OUT_DISABLED,
    /* Output trigger masks */
    .mask ={NULL},
    /** Waik-up timer */
    .timer.enable = false,
    .timer.clkSrc = CY_AUTANALOG_TIMER_CLK_LF,
    .timer.period = 0U
};

// Indices need to align with _cyhal_lppass_stt_t enum members
static cy_stc_autanalog_stt_t _cyhal_lppass_stt_entries[((size_t)_CYHAL_LPPASS_STT_MAX) + 1] = { 0 };

const cyhal_clock_t* _cyhal_lppass_hf_clock = &(CYHAL_CLOCK_HF[9]); // dedicated clock for lppass

#define _CYHAL_LPPASS_PSE84_HF_IDX          (9u)
#define _CYHAL_LPPASS_REQUIRED_HF_FREQ_MIN  (4096000u)
#define _CYHAL_LPPASS_REQUIRED_HF_FREQ_TYP  (20000000u)
#define _CYHAL_LPPASS_REQUIRED_HF_FREQ_MAX  (80000000u)

void _cyhal_lppass_irq_handler()
{
    uint32_t cause = Cy_AutAnalog_GetInterruptCause();
    _cyhal_lppass_irq_handler_t handler = NULL;
    
    if(0u != (cause & _CYHAL_LPPASS_ADC_INTR_MASK))
    {
        handler = _cyhal_lppass_irq_handlers[(size_t)_CYHAL_LPPASS_IRQ_ADC];
    }
    else if(0u != (cause & _CYHAL_LPPASS_CTB_COMP_INTR_MASK))
    {
        handler = _cyhal_lppass_irq_handlers[(size_t)_CYHAL_LPPASS_IRQ_CTB];
    }
    else if(0u != (cause & _CYHAL_LPPASS_PTC_INTR_MASK))
    {
        handler = _cyhal_lppass_irq_handlers[(size_t)_CYHAL_LPPASS_IRQ_PTC];
    }

    if(NULL != handler) // This can happen if an interrupt cause is masked after the interrupt request line is asserted
    {
        handler();
    }
}

cy_stc_autanalog_stt_t* _cyhal_lppass_get_stt_entry(_cyhal_lppass_stt_t stt_user)
{
    return &(_cyhal_lppass_stt_entries[(size_t)stt_user]);
}

cy_rslt_t _cyhal_lppass_apply_state(_cyhal_lppass_stt_t stt_user)
{
    Cy_AutAnalog_PauseAutonomousControl();
    /* Pause will not take effect while we're waiting on a trigger, which ADC uses, so fire the trigger
    * once and then wait for the controller to pause. */
    Cy_AutAnalog_FwTrigger(CY_AUTANALOG_FW_TRIGGER0);
    cy_en_autanalog_ac_status_t ac_status;
    do
    {
        cy_stc_autanalog_state_t state;
        Cy_AutAnalog_GetControllerState(&state);
        ac_status = state.ac.status;
    } while(CY_AUTANALOG_AC_STATUS_RUNNING == ac_status);

    cy_rslt_t result = (cy_rslt_t)Cy_AutAnalog_UpdateStateTransitionTable(1, &_cyhal_lppass_stt_entries[stt_user], stt_user);
    if(CY_RSLT_SUCCESS == result)
    {
        Cy_AutAnalog_RunControllerState(stt_user);
    }
    return result;
}

cy_rslt_t _cyhal_lppass_init()
{
    bool clock_in_range = false;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    // Not doing this through cyhal_utils_enable_ip_block because that is defined in terms of CYHAL_RSC_
    // and LPPASS spans CYHAL_RSC_ block types
    Cy_SysClk_PeriGroupSlaveInit(CY_MMIO_PASS_PERI_NR , CY_MMIO_PASS_GROUP_NR, CY_MMIO_PASS_SLAVE_NR, CY_MMIO_PASS_CLK_HF_NR); 

    //Try to see if current frequency is already within acceptable values
    uint32_t currentFrequency = cyhal_clock_get_frequency(&(CYHAL_CLOCK_HF[_CYHAL_LPPASS_PSE84_HF_IDX]));
    if(currentFrequency >= _CYHAL_LPPASS_REQUIRED_HF_FREQ_MIN && currentFrequency <= _CYHAL_LPPASS_REQUIRED_HF_FREQ_MAX)
    {
        clock_in_range = true;
    }

    if(false == clock_in_range)
    {
        // We can't change the clock frequency because it is shared with other peri groups. So if it's not in range,
        // report the error up to the application so that it can adjust the clocks taking into account all of the clients
        result = CYHAL_ANALOG_ERROR_INCORRECT_CLOCK_FREQ;
    } 

    if (CY_RSLT_SUCCESS == result)
    {
        cy_stc_autanalog_cfg_t autanalog_cfg;
        memset(&autanalog_cfg, 0, sizeof(autanalog_cfg));
        autanalog_cfg.ac = &_cyhal_lppass_ctb_ac_cfg;
        result = (cy_rslt_t)Cy_AutAnalog_LoadConfig(&autanalog_cfg);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        // Set up the AC states
        for(uint8_t count = 0u; count < _CYHAL_LPPASS_STT_TOTAL_NUM; count++)
        {
            cy_stc_autanalog_stt_t* ac_stt = _cyhal_lppass_get_stt_entry(count);
            ac_stt->ac = &_cyhal_lppass_ac_stt[count];
            ac_stt->sar[0] = &_cyhal_lppass_sar_stt[count];
            ac_stt->dac[0] = &_cyhal_lppass_dac_stt[count];
            ac_stt->ctb[0] = &_cyhal_lppass_ctb0_stt[count];
            ac_stt->ctb[1] = &_cyhal_lppass_ctb1_stt[count];
            ac_stt->ptcomp[0] = &_cyhal_lppass_ptc0_stt[count];
        }
        result = (cy_rslt_t)Cy_AutAnalog_LoadStateTransitionTable(_CYHAL_LPPASS_STT_TOTAL_NUM, _cyhal_lppass_get_stt_entry(0));
        Cy_AutAnalog_StartAutonomousControl();
    }
    
    if (CY_RSLT_SUCCESS == result)
    {
        _cyhal_irq_register(pass_interrupt_lppass_IRQn, CYHAL_ISR_PRIORITY_DEFAULT, _cyhal_lppass_irq_handler);
        _cyhal_irq_enable(pass_interrupt_lppass_IRQn);
    }
    return result;
}

cy_rslt_t _cyhal_lppass_enable_ctb(_cyhal_lppass_drivers_t driver, uint32_t instance, uint32_t channel, bool enable)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(driver == _CYHAL_LPPASS_DRIVER_CTB)
    {
        for(uint8_t count = 0u; count < _CYHAL_LPPASS_STT_TOTAL_NUM; count++)
        {
            if(instance == 0)
            {
                if (channel == 0)
                {
                    _cyhal_lppass_ctb0_stt[count].enableOpamp0 = enable ;
                }
                else
                {
                    _cyhal_lppass_ctb0_stt[count].enableOpamp1 = enable;
                }
                
            }
            else
            {
                if (channel == 0)
                {
                    _cyhal_lppass_ctb1_stt[count].enableOpamp0 = enable ;
                }
                else
                {
                    _cyhal_lppass_ctb1_stt[count].enableOpamp1 = enable;
                }
                
            }
        }

    }
    
    // Set up the AC states
    for(uint8_t count = 0u; count < _CYHAL_LPPASS_STT_TOTAL_NUM; count++)
    {
        cy_stc_autanalog_stt_t* ac_stt = _cyhal_lppass_get_stt_entry(count);
        if(instance == 0)
        {
            ac_stt->ctb[instance] = &_cyhal_lppass_ctb0_stt[count];
        }
        else
        {
            ac_stt->ctb[instance] = &_cyhal_lppass_ctb1_stt[count];
        }
        
    }

    result = (cy_rslt_t)Cy_AutAnalog_LoadStateTransitionTable(_CYHAL_LPPASS_STT_TOTAL_NUM, _cyhal_lppass_get_stt_entry(0));

    if(result == CY_RSLT_SUCCESS)
    {
        result = _cyhal_lppass_apply_state(_CYHAL_LPPASS_STT_START);
    }
    

    return result;
}

void _cyhal_lppass_free()
{
    Cy_AutAnalog_Disable();
}

void _cyhal_lppass_register_irq_handler(_cyhal_lppass_irq_t irq, _cyhal_lppass_irq_handler_t handler)
{
    CY_ASSERT((size_t)irq <= _CYHAL_LPPASS_IRQ_MAX);
    _cyhal_lppass_irq_handlers[(size_t)irq] = handler;
}

void _cyhal_lppass_set_irq_priority(uint8_t intr_priority)
{
    // Note: If we ever introduce a second driver that depends on the lppass interrupt, changing the priority
    // of one will change the priority of both. When/if this happens, this should be documented in the implementation
    // specific sections of the corresponding drivers.
    _cyhal_irq_set_priority(pass_interrupt_lppass_IRQn, intr_priority);
}

uint32_t _cyhal_lppass_prb_convert_tap(cy_en_autanalog_prb_tap_t prb_tap)
{
    switch(prb_tap)
    {
        case CY_AUTANALOG_PRB_TAP_0:
            return 6;
        case CY_AUTANALOG_PRB_TAP_1:
            return 13;
        case CY_AUTANALOG_PRB_TAP_2:
            return 19;
        case CY_AUTANALOG_PRB_TAP_3:
            return 25;
        case CY_AUTANALOG_PRB_TAP_4:
            return 31;
        case CY_AUTANALOG_PRB_TAP_5:
            return 38;
        case CY_AUTANALOG_PRB_TAP_6:
            return 44;
        case CY_AUTANALOG_PRB_TAP_7:
            return 50;
        case CY_AUTANALOG_PRB_TAP_8:
            return 56;
        case CY_AUTANALOG_PRB_TAP_9:
            return 63;
        case CY_AUTANALOG_PRB_TAP_10:
            return 69;
        case CY_AUTANALOG_PRB_TAP_11:
            return 75;
        case CY_AUTANALOG_PRB_TAP_12:
            return 81;
        case CY_AUTANALOG_PRB_TAP_13:
            return 88;
        case CY_AUTANALOG_PRB_TAP_14:
            return 94;
        case CY_AUTANALOG_PRB_TAP_15:
            return 100;
        default: 
            CY_ASSERT(false);
            return 0;
    }
}


uint32_t _cyhal_lppass_prb_convert_src(cy_en_autanalog_prb_src_t prb_src)
{
    switch(prb_src)
    {
        case CY_AUTANALOG_PRB_VBGR:
            return _CYHAL_LPPASS_VBGR_VALUE;
        case CY_AUTANALOG_PRB_VDDA:
            return cyhal_syspm_get_supply_voltage(CYHAL_VOLTAGE_SUPPLY_VDDA);
        default:
            CY_ASSERT(false);
            return 0;
    }
}


cy_stc_autanalog_ctb_sta_t* _cyhal_lppass_get_ctb_static_cfg( uint32_t index )
{
    return &_cyhal_lppass_ctb_static_cfg[index];
}

cy_stc_autanalog_ctb_dyn_t* _cyhal_lppass_get_ctb_dynamic_cfg( uint32_t index )
{
    return &_cyhal_lppass_ctb_dyn_cfg[index];
}

#endif //  defined(CY_IP_MXS22LPPASS)
