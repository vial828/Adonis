/***************************************************************************//**
* \file cy_i3c.c
* \version 1.00
*
* Provides API implementation for the I3C Controller.
*
********************************************************************************
* \copyright
* Copyright (c) (2020-2022), Cypress Semiconductor Corporation (an Infineon company)
* or an affiliate of Cypress Semiconductor Corporation.
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

#include "cy_device.h"

#if defined (CY_IP_MXI3C)

#include "cy_i3c.h"

#if defined(__cplusplus)
extern "C" {
#endif


CY_MISRA_DEVIATE_BLOCK_START('MISRA C-2012 Rule 14.3', 75, \
'Controlling expression are invariant.')

/*******************************************************************************
*                             Function Prototypes
*******************************************************************************/
static uint8_t even_parity(uint8_t address);
static uint32_t FirstSetBit(uint32_t value);
static void SetAddrslotStatus(uint8_t address, cy_en_i3c_addr_slot_status_t status, cy_stc_i3c_context_t *context);
static void InitAddrslots(cy_stc_i3c_context_t *context);
static void DeInitAddrslots(cy_stc_i3c_context_t *context);
static uint32_t GetI2CDevAddrPos(I3C_CORE_Type *base, uint8_t staticAddress, cy_stc_i3c_context_t *context);
static uint32_t GetI3CDevAddrPos(I3C_CORE_Type *base, uint8_t dynamicAddress, cy_stc_i3c_context_t *context);
static uint32_t GetDATFreePos(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static cy_en_i3c_addr_slot_status_t GetAaddrslotStatus(uint8_t address, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t ResponseError(uint32_t respCmd);
static uint32_t ResponseErrorEvent(uint32_t respCmd);
static void WriteArray(I3C_CORE_Type *base, void *buffer, uint32_t size);
static void ReadArray(I3C_CORE_Type *base, void *buffer, uint32_t size);
static void ControllerHandleDataTransmit(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static void ControllerHandleDataReceive(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t RetrieveI3CDeviceInfo(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, bool basicInfo, cy_stc_i3c_context_t *context);
static void CCC_Set(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static void CCC_Get(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t enec_disec_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t rstdaa_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t setmrwl_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static void setda_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t setnewda_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t entas_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t enthdr0_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getmrwl_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getpid_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getbcr_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getdcr_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getstatus_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getmxds_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t gethdrcap_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t getacccr_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t deftgts_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context);
static void ControllerHDRWrite(I3C_CORE_Type *base, cy_stc_i3c_hdr_cmd_t *hdrCmd, cy_stc_i3c_context_t *context);
static void ControllerHDRRead(I3C_CORE_Type *base, cy_stc_i3c_hdr_cmd_t *hdrCmd, cy_stc_i3c_context_t *context);
static void ControllerRespReadyStsHandle(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static void ControllerHandleWriteInterrupt(I3C_CORE_Type *base, uint32_t respCmdPort, cy_stc_i3c_context_t *context);
static void ControllerHandleReadInterrupt(I3C_CORE_Type *base, uint32_t respCmdPort, cy_stc_i3c_context_t *context);
static void ControllerHandleIBIInterrupt(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static void RearrangeAddrTable(I3C_CORE_Type *base, uint8_t devIndex, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t DeviceIBIControl(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, uint8_t cccCmd, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t CCCTargetAddressValidation(uint8_t address, bool unicastOnly, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t ControllerHandleCCCResponse(I3C_CORE_Type *base, uint32_t *resp, cy_stc_i3c_context_t *context);
static void TargetRespReadyStsHandle(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static void TargetHandleDataReceive(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static void TargetHandleDataTransmit(I3C_CORE_Type *base, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t SecondaryControllerInit(I3C_CORE_Type *base, bool isController, cy_stc_i3c_context_t *context);
static cy_en_i3c_status_t I3C_Check_Timeout(uint32_t *timeout);
static cy_en_syspm_status_t Cy_I3C_DeepSleepCallback_After(cy_stc_syspm_callback_params_t *callbackParams);

/*******************************************************************************
*  Function Name: Cy_I3C_Init
****************************************************************************//**
*
* Initializes the I3C block.
*
* \param base
* The pointer to the I3C instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_i3c_config_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
* \note
* For I3C target operations pin mux IO lines to GPIO before calling this API using \ref Cy_GPIO_SetHSIOM,
* and then pin mux IO lines to I3C functionality after this API call.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_Init(I3C_CORE_Type *base, cy_stc_i3c_config_t const *config, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == config) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    CY_ASSERT_L3(CY_I3C_IS_MODE_VALID(config->i3cMode));
    CY_ASSERT_L3(CY_I3C_IS_BUS_MODE_VALID(config->i3cBusMode));
    //CY_ASSERT_L2(config->i3cClockHz > 0UL);
    //CY_ASSERT_L2(CY_I3C_IS_SDR_DATA_RATE_VALID(dataRateHz));
    CY_ASSERT_L3(CY_I3C_IS_BUFFER_DEPTH_VALID(config->txEmptyBufThld));
    CY_ASSERT_L3(CY_I3C_IS_BUFFER_DEPTH_VALID(config->rxBufThld));
    CY_ASSERT_L3(CY_I3C_IS_BUFFER_DEPTH_VALID(config->txBufStartThld));
    CY_ASSERT_L3(CY_I3C_IS_BUFFER_DEPTH_VALID(config->rxBufStartThld));

    CY_ASSERT_L2(CY_IS_I3C_ADDR_NOT_RESERVED(config->dynamicAddr));
    CY_ASSERT_L2(CY_IS_I3C_QUEUE_THLD_VALID(config->cmdQueueEmptyThld));
    CY_ASSERT_L2(CY_IS_I3C_QUEUE_THLD_VALID(config->respQueueThld));
    CY_ASSERT_L2(CY_IS_I3C_QUEUE_THLD_VALID(config->ibiQueueThld));
    CY_ASSERT_L2(CY_IS_I3C_SDA_HOLD_TIME_VALID(config->sdaHoldTime));

     /* Error[Pa084]: pointless integer comparison, the result is always true */
#if 0
    CY_ASSERT_L2(CY_IS_I3C_BUS_FREE_TIME_VALID(config->busFreeTime));
    CY_ASSERT_L2(CY_IS_I3C_OD_CLK_CNT_VALID(config->openDrainHighCnt));
    CY_ASSERT_L2(CY_IS_I3C_OD_CLK_CNT_VALID(config->openDrainLowCnt));
    CY_ASSERT_L2(CY_IS_I3C_PP_CLK_CNT_VALID(config->pushPullHighCnt));
    CY_ASSERT_L2(CY_IS_I3C_PP_CLK_CNT_VALID(config->pushPullLowCnt));
    CY_ASSERT_L2(CY_IS_I2C_CLK_CNT_VALID(config->i2cFMLowCnt));
    CY_ASSERT_L2(CY_IS_I2C_CLK_CNT_VALID(config->i2cFMHighCnt));
    CY_ASSERT_L2(CY_IS_I2C_CLK_CNT_VALID(config->i2cFMPlusLowCnt));
    CY_ASSERT_L2(CY_IS_I2C_CLK_CNT_VALID(config->i2cFMPlusHighCnt));
    CY_ASSERT_L2(CY_IS_I3C_EXT_LCNT_VALID(config->extLowCnt1));
    CY_ASSERT_L2(CY_IS_I3C_EXT_LCNT_VALID(config->extLowCnt2));
    CY_ASSERT_L2(CY_IS_I3C_EXT_LCNT_VALID(config->extLowCnt3));
    CY_ASSERT_L2(CY_IS_I3C_EXT_LCNT_VALID(config->extLowCnt4));
    CY_ASSERT_L2(CY_IS_I3C_EXT_TERMINATION_LCNT_VALID(config->extTerminationLowCnt));
    CY_ASSERT_L2(CY_IS_I3C_BUS_AVAIL_TIME_VALID(config->busAvailTime));
    CY_ASSERT_L2(CY_IS_I3C_DCR_VALID(config->dcr));
#endif

    CY_ASSERT_L2(CY_IS_I3C_STATIC_ADDRESS_VALID(config->staticAddress));
    CY_ASSERT_L2(CY_IS_I3C_PID_VALID(config->pid));
    CY_ASSERT_L3(CY_IS_I3C_DEVICE_ROLE_CAP_VALID(config->deviceRoleCap));
    CY_ASSERT_L2(CY_IS_I3C_BUS_IDLE_TIME_VALID(config->busIdleTime));

    /* Initialize the context */
    context->i3cBusMode = config->i3cBusMode;
    context->i3cClockHz = config->i3cClockHz;
    context->openDrainSclRate = config->openDrainSclRate;
    context->state = CY_I3C_IDLE;

    /* Controller specific */
    context->controllerBuffer = NULL;
    context->controllerStatus = 0UL;
    context->controllerBufferIdx = 0UL;
    context->controllerBufferSize = 0UL;
    context->destDeviceAddr = 0U;
    context->hdrCmd = NULL;

    /*Target specific */
    context->targetStatus = 0UL;

    context->targetRxBuffer = NULL;
    context->targetRxBufferIdx = 0UL;
    context->targetRxBufferSize = 0UL;
    context->targetRxBufferCnt = 0UL;

    context->targetTxBuffer = NULL;
    context->targetTxBufferIdx = 0UL;
    context->targetTxBufferSize = 0UL;
    context->targetTxBufferCnt = 0UL;

    /* Unregister callbacks */
    context->cbEvents = NULL;
    context->cbIbi = NULL;

    /* Enabling the IP */
    I3C->CTRL= I3C_CTRL_ENABLED_Msk;

    if(CY_I3C_CONTROLLER == config->i3cMode)
    {
        cy_stc_i3c_controller_t *i3cController = &(context->i3cController);

        i3cController->freePos = 0x7FFUL;
        i3cController->lastAddress = 0U;
        i3cController->devCount = 0UL;
        i3cController->i2cDeviceCount = 0UL;
        i3cController->dynAddrDevCount = 0UL;

        /* Prepare the address slot status array */
        InitAddrslots(context);

        /* Setting the device operation mode to Controller - 0U */
        I3C_CORE_DEVICE_CTRL_EXTENDED(base) = _VAL2FLD(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE, I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_CONTROLLER);

        if(!config->manualDataRate)
        {
            (void)Cy_I3C_SetDataRate(base, config->i3cSclRate, config->i3cClockHz, context);
        }
        else
        {
             I3C_CORE_SCL_I3C_OD_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I3C_OD_TIMING_I3C_OD_HCNT, config->openDrainHighCnt) |
                                                _VAL2FLD(I3C_CORE_SCL_I3C_OD_TIMING_I3C_OD_LCNT, config->openDrainLowCnt);
             I3C_CORE_SCL_I3C_PP_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I3C_PP_TIMING_I3C_PP_HCNT, config->pushPullHighCnt) |
                                                _VAL2FLD(I3C_CORE_SCL_I3C_PP_TIMING_I3C_PP_LCNT, config->pushPullLowCnt);
             I3C_CORE_SCL_I2C_FM_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I2C_FM_TIMING_I2C_FM_HCNT, config->i2cFMHighCnt) |
                                                _VAL2FLD(I3C_CORE_SCL_I2C_FM_TIMING_I2C_FM_LCNT, config->i2cFMLowCnt);
             I3C_CORE_SCL_I2C_FMP_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT, config->i2cFMPlusHighCnt) |
                                                 _VAL2FLD(I3C_CORE_SCL_I2C_FMP_TIMING_I2C_FMP_LCNT, config->i2cFMPlusLowCnt);
             I3C_CORE_SCL_EXT_LCNT_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1, config->extLowCnt1) |
                                                  _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2, config->extLowCnt2) |
                                                  _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3, config->extLowCnt3) |
                                                  _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4, config->extLowCnt4);
            I3C_CORE_SCL_EXT_TERMN_LCNT_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_EXT_TERMN_LCNT_TIMING_I3C_EXT_TERMN_LCNT, config->extTerminationLowCnt);
        }

        I3C_CORE_DEVICE_CTRL(base) = _VAL2FLD(I3C_CORE_DEVICE_CTRL_IBA_INCLUDE, config->ibaInclude ? 1UL : 0UL) |
                                     _VAL2FLD(I3C_CORE_DEVICE_CTRL_HOT_JOIN_CTRL, config->hotJoinCtrl ? 1UL : 0UL);

        I3C_CORE_DEVICE_ADDR(base) = _VAL2FLD( I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR, config->dynamicAddr) |
                                     I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR_VALID_Msk;

        SetAddrslotStatus(config->dynamicAddr, CY_I3C_ADDR_SLOT_I3C_DEV, context);

        I3C_CORE_BUS_FREE_AVAIL_TIMING(base) |= _VAL2FLD(I3C_CORE_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME, config->busFreeTime);

        I3C_CORE_SDA_HOLD_SWITCH_DLY_TIMING(base) = _VAL2FLD(I3C_CORE_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD, config->sdaHoldTime);

        I3C_CORE_QUEUE_THLD_CTRL(base) = _VAL2FLD(I3C_CORE_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD, config->cmdQueueEmptyThld) |
                                         _VAL2FLD(I3C_CORE_QUEUE_THLD_CTRL_RESP_BUF_THLD, config->respQueueThld) |
                                         _VAL2FLD(I3C_CORE_QUEUE_THLD_CTRL_IBI_STATUS_THLD, config->ibiQueueThld);


        Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);

        context->i3cMode = config->i3cMode;

    }

    else
    {
         /* Setting the device operation mode to Target - 1U */
         I3C_CORE_DEVICE_CTRL_EXTENDED(base) = _VAL2FLD(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE, I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_TARGET);

        /* On setting this ADAPTIVE_I2C_I3C bit, the controller generates hot-join request only when it is in/changes to I3C mode of operation. */
        I3C_CORE_DEVICE_CTRL(base) = _VAL2FLD(I3C_CORE_DEVICE_CTRL_ADAPTIVE_I2C_I3C, config->adaptiveI2CI3C ? 1UL : 0UL);

        if(0U != config->staticAddress)
        {
            I3C_CORE_DEVICE_ADDR(base) |= _VAL2FLD(I3C_CORE_DEVICE_ADDR_STATIC_ADDR, config->staticAddress) |
                                          I3C_CORE_DEVICE_ADDR_STATIC_ADDR_VALID_Msk;
        }

        /*
            1. Configure the TGT_CHAR_CTRL register
            2. Configure the two PID registers
        */

        I3C_CORE_TGT_PID_VALUE(base) = (uint32_t)config->pid;

        CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 10.3','Intentional typecast from "unsigned 64-bit int" to narrower essential type "unsigned 32-bit int"');
        I3C_CORE_TGT_MIPI_ID_VALUE(base) = (config->pid) >> 32UL;

        I3C_CORE_TGT_CHAR_CTRL(base) &= _VAL2FLD(I3C_CORE_TGT_CHAR_CTRL_MAX_DATA_SPEED_LIMIT, config->speedLimit) |
                                        _VAL2FLD(I3C_CORE_TGT_CHAR_CTRL_HDR_CAPABLE, config->hdrCapable) |
                                        _VAL2FLD(I3C_CORE_TGT_CHAR_CTRL_DCR, config->dcr) |
                                        _VAL2FLD(I3C_CORE_TGT_CHAR_CTRL_DEVICE_ROLE, config->deviceRoleCap);

    /* Note:
        1. If this bit is set, the controller is allowed to send hot-join request intr on the bus. This bit, if set, can be set or cleared by the I3C controller
           via ENEC or DISEC CCCs.
        2. If this bit is set to 0, the controller will not be allowed to send hot-join request intr on the bus. The CCCs will not have any effects on this field.
    */
        I3C_CORE_TGT_EVENT_STATUS(base) &= (config->hotjoinEnable) ? I3C_CORE_TGT_EVENT_STATUS_HJ_EN_Msk : (~I3C_CORE_TGT_EVENT_STATUS_HJ_EN_Msk);

        I3C_CORE_BUS_FREE_AVAIL_TIMING(base) |=  _VAL2FLD(I3C_CORE_BUS_FREE_AVAIL_TIMING_BUS_AVAILABLE_TIME, config->busAvailTime);

        I3C_CORE_BUS_IDLE_TIMING(base) = _VAL2FLD(I3C_CORE_BUS_IDLE_TIMING_BUS_IDLE_TIME, config->busIdleTime);

        if(CY_I3C_SECONDARY_CONTROLLER == config->deviceRoleCap) /* secondary Controller Mode */
        {
            Cy_I3C_SetInterruptMask(base, CY_I3C_TGT_INTR_Msk | CY_I3C_INTR_DEFTGT_STS);
            Cy_I3C_SetInterruptStatusMask(base, CY_I3C_TGT_INTR_Msk | CY_I3C_INTR_DEFTGT_STS);
            context->i3cMode = CY_I3C_SECONDARY_CONTROLLER;
        }
        else
        {
            Cy_I3C_SetInterruptMask(base, CY_I3C_TGT_INTR_Msk);
            Cy_I3C_SetInterruptStatusMask(base, CY_I3C_TGT_INTR_Msk);
            context->i3cMode = config->i3cMode;
        }

        context->i3cSclRate = config->i3cSclRate;
    }

    I3C_CORE_DATA_BUFFER_THLD_CTRL(base) = _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD, config->rxBufThld) |
                                            _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD, config->txEmptyBufThld) |
                                            _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_RX_START_THLD, config->rxBufStartThld) |
                                            _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_TX_START_THLD, config->txBufStartThld);

    /* Config for DS wakeup restore */
    (void)memcpy(&(context->dsConfig), config, sizeof(cy_stc_i3c_config_t));
    context->intr_mask = 0UL;

    return CY_I3C_SUCCESS;

}


/*******************************************************************************
*  Function Name: Cy_I3C_Deinit
****************************************************************************//**
*
* Deinitializes the I3C block.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_I3C_DeInit(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);

    uint8_t index;
    if(_FLD2VAL(I3C_CTRL_ENABLED, I3C->CTRL) == 1U)
    {
        Cy_I3C_Disable(base);
        if(CY_I3C_TARGET != context->i3cMode) //Test: Should this be done for secondary controller too?
        {
            /* Set the address slot statuses to free */
            DeInitAddrslots(context);

            for(index = 0U; (index < CY_I3C_MAX_DEVS); index++)
            {
                /* Clears the DAT entries */
                Cy_I3C_WriteIntoDeviceAddressTable(base, index, 0UL);
            }
        }
        I3C_CORE_QUEUE_THLD_CTRL(base) = I3C_CORE_QUEUE_THLD_CTRL_DEF_VAL;
        I3C_CORE_DATA_BUFFER_THLD_CTRL(base) = I3C_CORE_DATA_BUFFER_THLD_CTRL_DEF_VAL;

        I3C_CORE_INTR_STATUS_EN(base) = 0UL;
        I3C_CORE_INTR_SIGNAL_EN(base) = 0UL;
    }

}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerAttachI2CDevice
****************************************************************************//**
*
* Attaches an I2C device to the bus as defined by \ref cy_stc_i2c_device_t.
* It is required to provide device static address and lvr information
* in \ref cy_stc_i2c_device_t.
*
* \param base
* The pointer to the I3C instance.
*
* \param i2cDevice
* The pointer to the I2C device description structure \ref cy_stc_i2c_device_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerAttachI2CDevice(I3C_CORE_Type *base, cy_stc_i2c_device_t *i2cDevice, cy_stc_i3c_context_t *context)
{

    if((NULL == base) || (NULL == i2cDevice) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    uint32_t value;
    uint8_t lvr, pos;

    /* Cannot add if there are already 11 devices on the bus */
    if(CY_I3C_MAX_DEVS <= (i3cController->devCount))
    {
        return CY_I3C_CONTROLLER_MAX_DEVS_PRESENT;
    }

    /* LVR: Legacy Virtual Register
    bit[7-5]: 000 - Legacy I2C only with 50ns IO Spike Filter
              001 and 010 - Legacy I2C only without 50ns IO Spike Filter
              others - reserved */
    lvr = i2cDevice->lvr;

    if( CY_I3C_ADDR_SLOT_FREE != GetAaddrslotStatus(i2cDevice->staticAddress, context))
    {
        return CY_I3C_CONTROLLER_FREE_ADDR_UNAVIAL;
    }
    /* BROS mentions not to support devices that lack the 50ns spike filters */
    if(0U != (CY_I3C_LVR_LEGACY_I2C_INDEX_MASK & lvr))
    {
        return CY_I3C_CONTROLLER_BAD_I2C_DEVICE;
    }

    SetAddrslotStatus(i2cDevice->staticAddress, CY_I3C_ADDR_SLOT_I2C_DEV, context);

    /* Populate the device address table with the static address of the device */
    value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR, i2cDevice->staticAddress) |
            I3C_CORE_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEVICE_Msk;

    pos = (uint8_t)(GetDATFreePos(base, context));

    Cy_I3C_WriteIntoDeviceAddressTable(base, pos, value);

    /* Maintaining a local list of i2c devices */
    Cy_I3C_UpdateI2CDevInList(i2cDevice, pos, context);

    /* Update the free position index of the device address table */
    (i3cController->freePos) = ~ (CY_I3C_BIT(pos));
    (i3cController->devCount)++;
    (i3cController->i2cDeviceCount)++;

    I3C_CORE_DEVICE_CTRL(base) |= I3C_CORE_DEVICE_CTRL_I2C_TARGET_PRESENT_Msk;

    return CY_I3C_SUCCESS;
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerDetachI2CDevice
****************************************************************************//**
*
* Detaches an I2C device from the bus defined by \ref cy_stc_i2c_device_t.
*
* \param base
* The pointer to the I3C instance.
*
* \param i2cDevice
* pointer to the I2C device description structure \ref cy_stc_i2c_device_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerDetachI2CDevice(I3C_CORE_Type *base, cy_stc_i2c_device_t *i2cDevice, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i2cDevice) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    uint32_t devIndex;

    /* "devIndex" - Position of the device in the DAT which has to be detached */
    devIndex = GetI2CDevAddrPos(base, i2cDevice->staticAddress, context);
    if(((uint32_t)CY_I3C_BAD_PARAM) == devIndex)
    {
        return CY_I3C_BAD_PARAM;
    }

    RearrangeAddrTable(base, (uint8_t)devIndex, context);

    context->i3cController.devCount--;
    context->i3cController.i2cDeviceCount--;
    SetAddrslotStatus((i2cDevice->staticAddress), CY_I3C_ADDR_SLOT_FREE, context);

    return CY_I3C_SUCCESS;
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerAttachI3CDevice
****************************************************************************//**
*
* Attaches an I3C device to the bus defined by \ref cy_stc_i3c_device_t.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDevice
* The pointer to the i3c device description structure \ref cy_stc_i3c_device_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
* \note
* This function is used to attach I3C devices with Static Addresses only.
*
*******************************************************************************/
/* This is blocking function, where we wait for the response packet,
   assuming the controller treats no response as NACK after waiting for a decent amount of time */
cy_en_i3c_status_t Cy_I3C_ControllerAttachI3CDevice(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i3cDevice) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    uint8_t dynamicAddress = 0U;
    cy_en_i3c_status_t retStatus;
    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    cy_stc_i3c_ccc_cmd_t cmd;
    cy_stc_i3c_ccc_setda_t i3cCccSetda;
    cy_stc_i3c_ccc_payload_t payload;

    /* Cannot add if there are already 11 devices on the bus */
    if(CY_I3C_MAX_DEVS <= (i3cController->devCount))
    {
        return CY_I3C_CONTROLLER_MAX_DEVS_PRESENT;
    }

    if(0U != i3cDevice->dynamicAddress)
    {
        /* The I3C device has an expected dynamic address */

        /* check if the expected dynamic address is available to be assigned */
        /* If not available, return */

        if(CY_I3C_ADDR_SLOT_FREE != GetAaddrslotStatus(i3cDevice->dynamicAddress, context))
        {
            return CY_I3C_CONTROLLER_FREE_ADDR_UNAVIAL;
        }

        dynamicAddress = i3cDevice->dynamicAddress;
    }

    else
    {
        /* If the Target device doesn't have an expected dynamic address, pick a free address from the list */
        retStatus = Cy_I3C_ControllerGetFreeDeviceAddress(base, &dynamicAddress, context);
        if(CY_I3C_CONTROLLER_FREE_ADDR_UNAVIAL == retStatus)
        {
            return retStatus;
        }

        i3cController->lastAddress = dynamicAddress;
    }

    /* Send SETDASA CCC command */
    i3cCccSetda.address = dynamicAddress;
    cmd.data = &payload;
    cmd.data->data = &(i3cCccSetda.address); //check
    cmd.data->len = (uint16_t)(sizeof(i3cCccSetda));
    cmd.cmd = (uint8_t)CY_I3C_CCC_SETDASA;
    cmd.address = i3cDevice->staticAddress;
    retStatus = Cy_I3C_SetDASA(base, &cmd, context);

    return retStatus;

}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerDetachI3CDevice
****************************************************************************//**
*
* Detaches an I3C device from the bus defined by \ref cy_stc_i3c_device_t.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDevice
* The pointer to the I3C device description structure \ref cy_stc_i3c_device_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
//TBD - Check if the device is hot-join capable, if yes send DISEC with HOT-JOIN event
cy_en_i3c_status_t Cy_I3C_ControllerDetachI3CDevice(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i3cDevice) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    uint32_t devIndex ;
    cy_en_i3c_status_t retstatus;
    cy_stc_i3c_ccc_cmd_t cccCmd;
    cy_stc_i3c_ccc_payload_t payload;

    devIndex = GetI3CDevAddrPos(base, i3cDevice->dynamicAddress, context);
    if(((uint32_t)CY_I3C_BAD_PARAM) == devIndex)
    {
        return CY_I3C_BAD_PARAM;
    }

    cccCmd.address = i3cDevice->dynamicAddress;
    cccCmd.cmd = (uint8_t)(CY_I3C_CCC_RSTDAA(false));
    cccCmd.data = &payload;
    cccCmd.data->data = NULL;
    cccCmd.data->len = 0U;

    retstatus = rstdaa_ccc(base, &cccCmd, context);
    if(CY_I3C_SUCCESS != retstatus)
    {
        return retstatus;
    }

    RearrangeAddrTable(base, (uint8_t)devIndex, context);

    context->i3cController.devCount--;
    SetAddrslotStatus((i3cDevice->dynamicAddress), CY_I3C_ADDR_SLOT_FREE, context);

    return CY_I3C_SUCCESS;
}


/*******************************************************************************
*  Function Name: Cy_I3C_Enable
****************************************************************************//**
* Enables the I3C block.
*
* \param base
* The pointer to the I3C instance.
*
*******************************************************************************/
void Cy_I3C_Enable(I3C_CORE_Type *base)
{
    CY_ASSERT_L1(NULL != base);

    I3C_CORE_DEVICE_CTRL(base) |= _VAL2FLD(I3C_CORE_DEVICE_CTRL_ENABLE, 1);
}


/*******************************************************************************
*  Function Name: Cy_I3C_Disable
****************************************************************************//**
* Disables the I3C block.
*
* \param base
* The pointer to the I3C instance.
*
* \note
* The I3C controller will complete any pending bus transaction before it gets
* disabled.
*******************************************************************************/
void Cy_I3C_Disable(I3C_CORE_Type *base)
{
    CY_ASSERT_L1(NULL != base);

    I3C_CORE_DEVICE_CTRL(base) &= ~I3C_CORE_DEVICE_CTRL_ENABLE_Msk;

    do
    {
        /*Wait until the controller comes to IDLE state */
    }while(0UL != (_FLD2VAL(I3C_CORE_PRESENT_STATE_CM_TFR_STS, I3C_CORE_PRESENT_STATE(base))));
}


/*******************************************************************************
*  Function Name: Cy_I3C_Resume
****************************************************************************//**
*
* Resumes the I3C Controller after an error state.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* When the controller enters any error condition it pauses all operation until
* user resumes manually. User must call this API once error status is returned
* for any I3C bus transactions like Read/Write or other operations.
*
*******************************************************************************/
void Cy_I3C_Resume(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);

    (void)context;

    /* uint32_t status;
    status = Cy_I3C_GetBusStatus(base, context);
    if(status &  CY_I3C_CONTROLLER_HALT_STATE)
    {
        I3C_CORE_DEVICE_CTRL(base) |= I3C_CORE_DEVICE_CTRL_RESUME_Msk;
    } */
    I3C_CORE_DEVICE_CTRL(base) |= I3C_CORE_DEVICE_CTRL_RESUME_Msk;

}


/*******************************************************************************
*  Function Name: Cy_I3C_GetI2CDeviceCount
****************************************************************************//**
*
* Provides the number of I2C devices attached to the bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Number of I2C devices attached to the bus.
*
*******************************************************************************/
uint32_t Cy_I3C_GetI2CDeviceCount(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return (uint32_t)CY_I3C_BAD_PARAM;
    }

    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->i3cController.i2cDeviceCount);
}


/*******************************************************************************
*  Function Name: Cy_I3C_GetI3CDeviceCount
****************************************************************************//**
*
* Provides the number of I3C devices attached to the bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Number of I3C devices attached to the bus.
*
*******************************************************************************/
uint32_t Cy_I3C_GetI3CDeviceCount(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return (uint32_t)CY_I3C_BAD_PARAM;
    }

    /* Suppress a compiler warning about unused variables */
    (void) base;

    return ((context->i3cController.devCount)-(context->i3cController.i2cDeviceCount));
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerGetI2CDevices
****************************************************************************//**
*
* Provides the list of I2C devices on the bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param i2cDeviceList
* The pointer to the I2C device list array.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerGetI2CDevices(I3C_CORE_Type *base, cy_stc_i2c_device_t *i2cDeviceList, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i2cDeviceList) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    uint8_t index;
    cy_stc_i3c_controller_devlist_t *ptr = context->devList;

    for(index = 0U; index < i3cController->devCount; index++)
    {
        if(ptr->i2c)
        {
            *i2cDeviceList = ptr->i2cDevice;
            i2cDeviceList++;
        }
        ptr++;
    }
    return CY_I3C_SUCCESS;
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerGetI3CDevices
****************************************************************************//**
*
* Provides the list of I3C devices on the bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDeviceList
* The pointer to the I3C device list array.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerGetI3CDevices(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDeviceList, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i3cDeviceList) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    uint8_t index, count;
    cy_stc_i3c_controller_devlist_t *ptr = context->devList;
    count = (uint8_t)(i3cController->devCount);

    for(index = 0U; index < count; index++)
    {
        if(!ptr->i2c)
        {
            *i3cDeviceList = ptr->i3cDevice;
            i3cDeviceList++;
        }
        ptr++;
    }
    return CY_I3C_SUCCESS;
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerGetFreeDeviceAddress
****************************************************************************//**
*
* Provides the next dynamic address available to be assigned.
*
* \param base
* The pointer to the I3C instance.
*
* \param address
* The pointer to the address.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerGetFreeDeviceAddress(I3C_CORE_Type *base, uint8_t *address, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == address) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    cy_en_i3c_addr_slot_status_t status;
    uint8_t freeAddress;
    uint8_t startAddress = (i3cController->lastAddress) + 1U;

    for(freeAddress = startAddress; freeAddress < CY_I3C_MAX_ADDR; freeAddress++)
    {
        status = GetAaddrslotStatus(freeAddress, context);
        if(CY_I3C_ADDR_SLOT_FREE == status)
        {
            *address = freeAddress;
            return CY_I3C_SUCCESS;
        }
    }

    /* case when a device is detached from the bus and the address is available */
    for(freeAddress = 1U; freeAddress < startAddress; freeAddress++)
    {
        status = GetAaddrslotStatus(freeAddress, context);
        if(CY_I3C_ADDR_SLOT_FREE == status)
        {
            *address = freeAddress;
            return CY_I3C_SUCCESS;
        }
    }

    return CY_I3C_CONTROLLER_FREE_ADDR_UNAVIAL;
}


/*******************************************************************************
*  Function Name: Cy_I3C_isCCCCmdSupported
****************************************************************************//**
*
* Checks if the CCC command is supported.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The CCC command to be checked.
*
* \return
* ture: If the specified command is supported
* false: If the specified command is not supported.
*
*******************************************************************************/
bool Cy_I3C_isCCCCmdSupported(I3C_CORE_Type *base, uint8_t cccCmd)
{

    /* Suppress a compiler warning about unused variables */
    (void) base;
    bool ret = false;

    switch(cccCmd)
        {
            case CY_I3C_CCC_ENEC(true):
            case CY_I3C_CCC_ENEC(false):
            case CY_I3C_CCC_DISEC(true):
            case CY_I3C_CCC_DISEC(false):
            case CY_I3C_CCC_ENTAS(0U, true):
            case CY_I3C_CCC_ENTAS(0U, false):
            case CY_I3C_CCC_RSTDAA(true):
            case CY_I3C_CCC_RSTDAA(false):
            case CY_I3C_CCC_ENTDAA:
            case CY_I3C_CCC_SETMWL(true):
            case CY_I3C_CCC_SETMWL(false):
            case CY_I3C_CCC_SETMRL(true):
            case CY_I3C_CCC_SETMRL(false):
            case CY_I3C_CCC_ENTHDR(0U):
            case CY_I3C_CCC_SETDASA:
            case CY_I3C_CCC_SETNEWDA:
            case CY_I3C_CCC_GETMWL:
            case CY_I3C_CCC_GETMRL:
            case CY_I3C_CCC_GETPID:
            case CY_I3C_CCC_GETBCR:
            case CY_I3C_CCC_GETDCR:
            case CY_I3C_CCC_GETSTATUS:
            case CY_I3C_CCC_GETMXDS:
            case CY_I3C_CCC_GETHDRCAP:
            case CY_I3C_CCC_GETACCCR:
            case CY_I3C_CCC_DEFTGTS:
            {
                ret = true;
                break;
            }
            default:
            {
                ret = false;
                break;
            }
        }
    return ret;
}


/*******************************************************************************
*  Function Name: Cy_I3C_SendCCCCmd
****************************************************************************//**
*
* Post the specified CCC command to command queue.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the CCC command description structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_SendCCCCmd(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == cccCmd) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_en_i3c_status_t retStatus;

    context->controllerStatus = CY_I3C_CONTROLLER_BUSY;
    switch(cccCmd->cmd)
    {
        case CY_I3C_CCC_ENEC(true):
        case CY_I3C_CCC_ENEC(false):
        case CY_I3C_CCC_DISEC(true):
        case CY_I3C_CCC_DISEC(false):
            {
                retStatus = enec_disec_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_RSTDAA(true):
        case CY_I3C_CCC_RSTDAA(false):
            {
                retStatus = rstdaa_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_SETMWL(true):
        case CY_I3C_CCC_SETMWL(false):
        case CY_I3C_CCC_SETMRL(true):
        case CY_I3C_CCC_SETMRL(false):
            {
                retStatus = setmrwl_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_SETNEWDA:
            {
                retStatus = setnewda_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETMWL:
        case CY_I3C_CCC_GETMRL:
            {
                retStatus = getmrwl_ccc(base, cccCmd, context);
                break;
            }

        case CY_I3C_CCC_GETPID:
            {
                retStatus = getpid_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETBCR:
            {
                retStatus = getbcr_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETDCR:
            {
                retStatus = getdcr_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETSTATUS:
            {
                retStatus = getstatus_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETMXDS:
            {
                retStatus = getmxds_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETHDRCAP:
            {
                retStatus = gethdrcap_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_ENTAS(0U, true):
        case CY_I3C_CCC_ENTAS(0U, false):
        case CY_I3C_CCC_ENTAS(1U, true):
        case CY_I3C_CCC_ENTAS(1U, false):
        case CY_I3C_CCC_ENTAS(2U, true):
        case CY_I3C_CCC_ENTAS(2U, false):
        case CY_I3C_CCC_ENTAS(3U, true):
        case CY_I3C_CCC_ENTAS(3U, false):
            {
                retStatus = entas_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_ENTHDR(0U):
            {
                retStatus = enthdr0_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_GETACCCR:
            {
                retStatus = getacccr_ccc(base, cccCmd, context);
                break;
            }
        case CY_I3C_CCC_DEFTGTS:
            {
                retStatus = deftgts_ccc(base, cccCmd, context);
                break;
            }
        default:
            {
                retStatus = CY_I3C_CONTROLLER_CCC_NOT_SUPPORTED;
                break;
            }
    }

    context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
    return retStatus;
}


/*******************************************************************************
*  Function Name: Cy_I3C_DisableDeviceIbi
****************************************************************************//**
*
* Disables all IBI events from specified device.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDevice
* pointer to the i3c device description structure \ref cy_stc_i3c_device_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_DisableDeviceIbi(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i3cDevice) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_en_i3c_status_t retStatus;

    retStatus = DeviceIBIControl(base, i3cDevice, CY_I3C_CCC_DISEC(false), context);

    return retStatus;
}


/*******************************************************************************
*  Function Name: Cy_I3C_EnableDeviceIbi
****************************************************************************//**
*
* Enables all IBI events from specified device.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDevice
* pointer to the i3c device description structure \ref cy_stc_i3c_device_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_config_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_EnableDeviceIbi(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == i3cDevice) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    uint32_t sirmap, bitpos, mrmap;
    cy_en_i3c_status_t retStatus;

    /* Target Interrupt Request */
    sirmap = I3C_CORE_IBI_TIR_REQ_REJECT(base);
    bitpos = CY_I3C_IBI_TIR_REQ_ID(i3cDevice->dynamicAddress);
    /*Setting the corresponding bit to 0: 0 -> Ack the SIR from the corresponding device */
    sirmap &= ~((1UL) << bitpos);
    I3C_CORE_IBI_TIR_REQ_REJECT(base) = sirmap;

    /* Controllership Request */
    mrmap = I3C_CORE_IBI_CR_REQ_REJECT(base);
    /* Setting the corresponding bit to 0: 0 -> Ack the MR from the corresponding device */
    mrmap &= ~((1UL) << bitpos);
    I3C_CORE_IBI_CR_REQ_REJECT(base) = mrmap;

    retStatus = DeviceIBIControl(base, i3cDevice, CY_I3C_CCC_ENEC(false), context);

    return retStatus;
}


/*******************************************************************************
*  Function Name: Cy_I3C_SetDataRate
****************************************************************************//**
*
* Sets desired maximum I3C bus data rate for transfers.
*
* \param base
* The pointer to the I3C instance.
*
* \param dataRateHz
* The desired I3C data rate in Hz.
*
* \param i3cClockHz
* The frequency of the clock connected to the I3C Block in Hz.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* The achieved data rate in Hz.
* When zero value is returned there is an error in the input parameters:
* data rate or clk_i3c is out of valid range.
*
*******************************************************************************/
uint32_t Cy_I3C_SetDataRate(I3C_CORE_Type *base, uint32_t dataRateHz, uint32_t i3cClockHz, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return (uint32_t)CY_I3C_BAD_PARAM;
    }

    CY_ASSERT_L2(i3cClockHz > 0UL);
    CY_ASSERT_L2(CY_I3C_IS_SDR_DATA_RATE_VALID(dataRateHz));

    uint32_t i3cClockPeriod; /* in nanoSeconds */
    uint32_t resDataRate;
    uint8_t hcnt,lcnt,lcnt_min;

    /* CY_I3C_DIV_ROUND_UP - quotient will be rounded to the next number in non-integer cases */
    i3cClockPeriod = CY_I3C_DIV_ROUND_UP(1000000000U, i3cClockHz); //time period in nanoseconds

    /* High Phase will be 41ns irrespective of whether the bus is in PureBusMode or MixedBusMode */
    hcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(CY_I3C_BUS_THIGH_MAX_NS, i3cClockPeriod) - 1U);
    if (hcnt < CY_I3C_SCL_I3C_TIMING_CNT_MIN)
    {
        hcnt = CY_I3C_SCL_I3C_TIMING_CNT_MIN;
    }

    if ((CY_I3C_DIV_ROUND_UP(i3cClockHz, dataRateHz) - hcnt) < (uint32_t)CY_I3C_SCL_I3C_TIMING_CNT_MIN){
        lcnt = CY_I3C_SCL_I3C_TIMING_CNT_MIN;
    }else{
        lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(i3cClockHz, dataRateHz) - hcnt);
    }

    resDataRate = CY_I3C_DIV_ROUND_UP(1000000000U,(((uint32_t)hcnt + (uint32_t)lcnt) * i3cClockPeriod));

    I3C_CORE_SCL_I3C_PP_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I3C_PP_TIMING_I3C_PP_HCNT, hcnt) |
                                      _VAL2FLD(I3C_CORE_SCL_I3C_PP_TIMING_I3C_PP_LCNT, lcnt);

    lcnt_min = (uint8_t)(CY_I3C_DIV_ROUND_UP(CY_I3C_BUS_TLOW_OD_MIN_NS, i3cClockPeriod)); /*minimum required lcnt value.*/
    if ((CY_I3C_DIV_ROUND_UP(i3cClockHz, context->openDrainSclRate) - hcnt) < (uint32_t)lcnt_min){
        lcnt = lcnt_min;
    }else{
        lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(i3cClockHz, context->openDrainSclRate) - hcnt);
    }

    I3C_CORE_SCL_I3C_OD_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I3C_OD_TIMING_I3C_OD_HCNT, hcnt) |
                                      _VAL2FLD(I3C_CORE_SCL_I3C_OD_TIMING_I3C_OD_LCNT, lcnt);

    I3C_CORE_BUS_FREE_AVAIL_TIMING(base) = _VAL2FLD(I3C_CORE_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME, lcnt);

    lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(i3cClockHz, CY_I3C_SDR1_DATA_RATE) - hcnt);
    I3C_CORE_SCL_EXT_LCNT_TIMING(base) |= _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1, lcnt);
    lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(i3cClockHz,  CY_I3C_SDR2_DATA_RATE) - hcnt);
    I3C_CORE_SCL_EXT_LCNT_TIMING(base) |= _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2, lcnt);
    lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(i3cClockHz, CY_I3C_SDR3_DATA_RATE) - hcnt);
    I3C_CORE_SCL_EXT_LCNT_TIMING(base) |= _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3, lcnt);
    lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(i3cClockHz, CY_I3C_SDR4_DATA_RATE) - hcnt);
    I3C_CORE_SCL_EXT_LCNT_TIMING(base) |= _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4, lcnt);

    if(CY_I3C_BUS_PURE != context->i3cBusMode)
    {
        /* Mixed mode bus */
        lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(CY_I3C_BUS_I2C_FMP_TLOW_MIN_NS, i3cClockPeriod));
        hcnt = ((uint8_t)CY_I3C_DIV_ROUND_UP(i3cClockHz, CY_I3C_I2C_FMP_DATA_RATE)) - lcnt;

        I3C_CORE_SCL_I2C_FMP_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT, hcnt) |
                                            _VAL2FLD(I3C_CORE_SCL_I2C_FMP_TIMING_I2C_FMP_LCNT, lcnt);

        lcnt = (uint8_t)(CY_I3C_DIV_ROUND_UP(CY_I3C_BUS_I2C_FM_TLOW_MIN_NS, i3cClockPeriod));
        hcnt = ((uint8_t)CY_I3C_DIV_ROUND_UP(i3cClockHz, CY_I3C_I2C_FM_DATA_RATE) - lcnt);

        I3C_CORE_SCL_I2C_FM_TIMING(base) = _VAL2FLD(I3C_CORE_SCL_I2C_FM_TIMING_I2C_FM_HCNT, hcnt) |
                                            _VAL2FLD(I3C_CORE_SCL_I2C_FM_TIMING_I2C_FM_LCNT, lcnt);

        /* Configure the BUS_FREE_AVAIL_TIMING register with lcnt from Fast-Mode */
        I3C_CORE_BUS_FREE_AVAIL_TIMING(base) = _VAL2FLD(I3C_CORE_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME, lcnt);

    }

    context->i3cSclRate = resDataRate;
    context->dsConfig.manualDataRate = false;
    return resDataRate;

}


/*******************************************************************************
*  Function Name: Cy_I3C_I3CGetDataRate
****************************************************************************//**
*
* Provides the supported I3C bus data rate for I3C devices.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_data_speed_t
*
*******************************************************************************/
uint32_t Cy_I3C_GetDataRate(I3C_CORE_Type const *base, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return (uint32_t)CY_I3C_BAD_PARAM;
    }

    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->i3cSclRate);
}


/*******************************************************************************
*  Function Name: Cy_I3C_IsBusBusy
****************************************************************************//**
*
* Indicates the whether the bus is busy or not.
*
* \param base
* The pointer to the I3C instance.
*
* \return
* true: if bus is BUSY.
* false: if bus is IDLE.
*
*******************************************************************************/
bool Cy_I3C_IsBusBusy(I3C_CORE_Type const *base)
{
    CY_ASSERT_L1(NULL != base);

    uint32_t retStatus;

    retStatus = I3C_CORE_PRESENT_STATE(base) & I3C_CORE_PRESENT_STATE_CM_TFR_STS_Msk;

    if(0UL == retStatus)
    {
        return false;
    }

    return true;
}


/*******************************************************************************
*  Function Name: Cy_I3C_IsController
****************************************************************************//**
*
* Indicates the whether the bus is busy or not.
*
* \param base
* The pointer to the I3C instance.
*
* \return
* true: if controller is current bus controller.
* false: otherwise.
*
*******************************************************************************/
bool  Cy_I3C_IsController(I3C_CORE_Type const *base)
{
    CY_ASSERT_L1(NULL != base);

    uint32_t value = _FLD2VAL(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE, I3C_CORE_DEVICE_CTRL_EXTENDED(base));

    if(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_TARGET == value)
    {
        return false;
    }
    else
    {
        return true;
    }
}


/*******************************************************************************
*  Function Name: Cy_I3C_GetMode
****************************************************************************//**
*
* Provides the mode of the device.
*
* \param base
* The pointer to the I3C instance.
*
* \return
* \ref cy_en_i3c_mode_t
*
*******************************************************************************/
cy_en_i3c_mode_t Cy_I3C_GetMode(I3C_CORE_Type const *base)
{
    CY_ASSERT_L1(NULL != base);

    uint32_t mode;

    mode = I3C_CORE_DEVICE_CTRL_EXTENDED(base);
    mode = _FLD2VAL(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE, mode);

    if(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_TARGET == mode)
    {
        return CY_I3C_TARGET;
    }
    else
    {
        return CY_I3C_CONTROLLER;
    }
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerWrite
****************************************************************************//**
*
* Writes data provided by xferConfig structure \ref cy_stc_i3c_controller_xfer_config_t
* to a specific device.
*
* \param base
* The pointer to the I3C instance.
*
* \param xferConfig
* Controller transfer configuration structure.
* \ref cy_stc_i3c_controller_xfer_config_t
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerWrite(I3C_CORE_Type *base, cy_stc_i3c_controller_xfer_config_t *xferConfig, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == xferConfig) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    CY_ASSERT_L1(CY_IS_I3C_BUFFER_VALID(xferConfig->buffer, xferConfig->bufferSize));
    CY_ASSERT_L2(CY_IS_I3C_ADDR_VALID(xferConfig->targetAddress));

    if(CY_I3C_FIFO_SIZE < xferConfig->bufferSize)
    {
        return CY_I3C_BAD_BUFFER_SIZE;
    }

    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;
    
    if(0UL != (CY_I3C_IDLE_MASK & context->state))
    {
        uint8_t validBytes;
        cy_en_i3c_addr_slot_status_t res;
        uint8_t *data;
        cy_stc_i3c_ccc_t cmd;
        uint8_t pos = 0U;
        cy_stc_i3c_controller_devlist_t *i3cDeviceList;
        uint8_t writeDSMode = (uint8_t)CY_I3C_SDR0;

        context->controllerBuffer = xferConfig->buffer;
        context->controllerBufferSize = xferConfig->bufferSize;
        context->controllerBufferIdx = 0UL;
        data = context->controllerBuffer;
        context->controllerStatus = CY_I3C_CONTROLLER_BUSY;

        cmd.cmdHigh = 0UL;
        cmd.cmdLow = 0UL;

        /* Clear the interrupts */
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

        /* Reset the Tx FIFO */
        I3C_CORE_RESET_CTRL(base) |= I3C_CORE_RESET_CTRL_TX_FIFO_RST_Msk;
        do
        {
            /* wait till the reset is completed */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

        if (retStatus == CY_I3C_TIMEOUT){
            return retStatus;
        }  

        res = GetAaddrslotStatus(xferConfig->targetAddress, context);

        /* This helps to check if the device is actively present on the bus */
        if((CY_I3C_ADDR_SLOT_I3C_DEV != res ) && (CY_I3C_ADDR_SLOT_I2C_DEV != res))
        {
            return CY_I3C_BAD_PARAM;
        }

        if(CY_I3C_ADDR_SLOT_I2C_DEV == res)
        {
            /* The target device is an i2cDevice */
            pos = (uint8_t)(GetI2CDevAddrPos(base, xferConfig->targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            writeDSMode = (uint8_t)((0U != (i3cDeviceList->i2cDevice.lvr & CY_I3C_LVR_I2C_MODE_INDICATOR)) ? CY_I3C_FM_I2C: CY_I3C_FMP_I2C);
            context->controllerStatus |= CY_I3C_CONTROLLER_I2C_SDR_WR_XFER;
        }

        else
        {
            /* The target is an i3cDevice */
            pos = (uint8_t)(GetI3CDevAddrPos(base, xferConfig->targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            if(CY_I3C_CORE_BCR_MAX_DATA_SPEED_LIM_Msk == (i3cDeviceList->i3cDevice.bcr))
            {
                writeDSMode = (i3cDeviceList->i3cDevice.maxWriteDs) & 0x07U;
            }
            context->controllerStatus |= CY_I3C_CONTROLLER_I3C_SDR_WR_XFER;
        }

        switch(xferConfig->bufferSize)
        {
            case 1UL: {
                      validBytes = CY_I3C_BYTE_STROBE1;
                      cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, validBytes) |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0,*data);
                      context->controllerBufferSize -= 1U;
                      cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk |
                                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;
                      break; }
            case 2UL: {
                      validBytes = CY_I3C_BYTE_STROBE2;
                      cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, validBytes) |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0,*(data)) |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_1,*(data + 1U));
                      context->controllerBufferSize -= 2U;
                      cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk |
                                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;
                      break; }
            case 3UL: {
                      validBytes = CY_I3C_BYTE_STROBE3;
                      cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, validBytes) |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0,*data) |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_1,*(data + 1U)) |
                                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_2,*(data + 2u));
                      context->controllerBufferSize -= 3U;
                      cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk |
                                      I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;
                      break; }
            default: {

                      //Write transfer with data greater than 3 bytes
                      ControllerHandleDataTransmit(base, context);
                      if(xferConfig->toc)
                      {
                        cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;
                      }

                      cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                                     _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, context->controllerBufferSize);
                      break; }
        }

        cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                      _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TID, CY_I3C_CONTROLLER_SDR_WRITE_TID) |
                      _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                      _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SPEED, writeDSMode) |
                      I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk;

        context->state = CY_I3C_CONTROLLER_TX;

        Cy_I3C_SetInterruptMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));
        Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));

        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;

        retStatus = CY_I3C_SUCCESS;
    }

    return retStatus;
}

/*******************************************************************************
*  Function Name: Cy_I3C_ControllerRead
****************************************************************************//**
*
* Reads data from a device specified by xferConfig structure
* \ref cy_stc_i3c_controller_xfer_config_t.
*
* \param base
* The pointer to the I3C instance.
*
* \param xferConfig
* Controller transfer configuration structure.
* \ref cy_stc_i3c_controller_xfer_config_t
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerRead (I3C_CORE_Type *base, cy_stc_i3c_controller_xfer_config_t* xferConfig, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == xferConfig) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    CY_ASSERT_L1(CY_IS_I3C_BUFFER_VALID(xferConfig->buffer, xferConfig->bufferSize));
    CY_ASSERT_L2(CY_IS_I3C_ADDR_VALID(xferConfig->targetAddress));

    if(CY_I3C_FIFO_SIZE < xferConfig->bufferSize)
    {
        return CY_I3C_BAD_BUFFER_SIZE;
    }

    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

    if(0UL != (CY_I3C_IDLE_MASK & context->state))
    {
        cy_stc_i3c_ccc_t cmd;
        uint8_t pos = 0U;
        cy_stc_i3c_controller_devlist_t *i3cDeviceList;
        uint8_t readDSMode = (uint8_t)CY_I3C_SDR0;
        cy_en_i3c_addr_slot_status_t res;

        context->controllerBuffer = xferConfig->buffer;
        context->controllerBufferSize = xferConfig->bufferSize;
        context->controllerBufferIdx = 0UL;
        context->controllerStatus = CY_I3C_CONTROLLER_BUSY;

        cmd.cmdHigh = 0UL;
        cmd.cmdLow = 0UL;

        /* Clear the interrupts*/
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

        /* Reset the Rx FIFO */
        I3C_CORE_RESET_CTRL(base) |= I3C_CORE_RESET_CTRL_RX_FIFO_RST_Msk;
        do
        {
            /* wait till the reset is completed */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

        if (retStatus == CY_I3C_TIMEOUT){
            return retStatus;
        }
        res = GetAaddrslotStatus(xferConfig->targetAddress, context);

        /* This helps to check if the device is actively present on the bus */
        if((CY_I3C_ADDR_SLOT_I3C_DEV != res ) && (CY_I3C_ADDR_SLOT_I2C_DEV != res))
        {
            return CY_I3C_BAD_PARAM;
        }

        if(CY_I3C_ADDR_SLOT_I2C_DEV == res)
        {
            /* The target device is an i2cDevice */
            pos = (uint8_t)(GetI2CDevAddrPos(base, xferConfig->targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            readDSMode = (uint8_t)((0U != (i3cDeviceList->i2cDevice.lvr & CY_I3C_LVR_I2C_MODE_INDICATOR)) ? CY_I3C_FM_I2C: CY_I3C_FMP_I2C);
            context->controllerStatus |= CY_I3C_CONTROLLER_I2C_SDR_RD_XFER;
        }
        else
        {
            /* The target is an i3cDevice */
            pos = (uint8_t)(GetI3CDevAddrPos(base, xferConfig->targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            if(CY_I3C_CORE_BCR_MAX_DATA_SPEED_LIM_Msk == (i3cDeviceList->i3cDevice.bcr))
            {
                readDSMode = (i3cDeviceList->i3cDevice.maxReadDs) & 0x07U;
            }
            context->controllerStatus |= CY_I3C_CONTROLLER_I3C_SDR_RD_XFER;
        }

        if(xferConfig->toc)
        {
            /* There are no bytes remaining to be read from the next READ, so terminate this READ with STOP */
            cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;
        }
        cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                        _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, context->controllerBufferSize);

        cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TID, CY_I3C_CONTROLLER_SDR_READ_TID) |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SPEED, readDSMode) |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_RnW_Msk |
                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk;

        context->state = CY_I3C_CONTROLLER_RX;

        Cy_I3C_SetInterruptMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));
        Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));

        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;

        return CY_I3C_SUCCESS;
    }
    return retStatus;
}

/*******************************************************************************
*  Function Name: Cy_I3C_ControllerAbortTransfer
****************************************************************************//**
*
* Aborts an ongoing transfer.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_I3C_ControllerAbortTransfer(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);

    if(CY_I3C_IDLE != context->state)
    {
        uint32_t intrState;
        I3C_CORE_DEVICE_CTRL(base) |= I3C_CORE_DEVICE_CTRL_ABORT_Msk;

        intrState = Cy_SysLib_EnterCriticalSection();
        Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_TRANSFER_ABORT_STS);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_TRANSFER_ABORT_STS);
        Cy_SysLib_ExitCriticalSection(intrState);
    }
}

/*******************************************************************************
*  Function Name: Cy_I3C_ControllerWriteByte
****************************************************************************//**
*
* Sends one byte to a target.
* This function is blocking.
*
* \param base
* The pointer to the I3C instance.
*
* \param targetAddress
* The dynamic address of the target I3C target device.
*
* \param data
* The byte to write to the target.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerWriteByte(I3C_CORE_Type *base, uint8_t targetAddress, int8_t data, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    CY_ASSERT_L2(CY_IS_I3C_ADDR_VALID(targetAddress));

    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;
    
    if(0UL != (CY_I3C_IDLE_MASK & context->state))
    {
        cy_en_i3c_addr_slot_status_t res;
        cy_stc_i3c_ccc_t cmd;
        uint8_t pos = 0U;
        cy_stc_i3c_controller_devlist_t *i3cDeviceList;
        uint8_t writeDSMode = (uint8_t)CY_I3C_SDR0;
        uint32_t respCmdPort;

        context->controllerStatus = CY_I3C_CONTROLLER_BUSY;

        cmd.cmdHigh = 0UL;
        cmd.cmdLow = 0UL;

        /* Clear the interrupts */
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

        /* Reset the Tx FIFO */
        I3C_CORE_RESET_CTRL(base) |= I3C_CORE_RESET_CTRL_TX_FIFO_RST_Msk;
        do
        {
            /* wait till the reset is completed */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

        if (retStatus == CY_I3C_TIMEOUT){
            return retStatus;
        }

        res = GetAaddrslotStatus(targetAddress, context);

        /* This helps to check if the device is actively present on the bus */
        if((CY_I3C_ADDR_SLOT_I3C_DEV != res ) && (CY_I3C_ADDR_SLOT_I2C_DEV != res))
        {
            return CY_I3C_BAD_PARAM;
        }

        if(CY_I3C_ADDR_SLOT_I2C_DEV == res)
        {
            /* The target device is an i2cDevice */
            pos = (uint8_t)(GetI2CDevAddrPos(base, targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            writeDSMode = (uint8_t)((0U != (i3cDeviceList->i2cDevice.lvr & CY_I3C_LVR_I2C_MODE_INDICATOR)) ? CY_I3C_FM_I2C: CY_I3C_FMP_I2C);
            context->controllerStatus |= CY_I3C_CONTROLLER_I2C_SDR_WR_XFER;
        }

        else
        {
            /* The target is an i3cDevice */
            pos = (uint8_t)(GetI3CDevAddrPos(base, targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            if(CY_I3C_CORE_BCR_MAX_DATA_SPEED_LIM_Msk == (i3cDeviceList->i3cDevice.bcr))
            {
                writeDSMode = (i3cDeviceList->i3cDevice.maxWriteDs) & 0x07U;
            }
            context->controllerStatus |= CY_I3C_CONTROLLER_I3C_SDR_WR_XFER;
        }

        cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, CY_I3C_BYTE_STROBE1) |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0, data);

        cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                      _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                      _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SPEED, writeDSMode) |
                      I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk |
                      I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk |
                      I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;

        context->state = CY_I3C_CONTROLLER_TX;

        /* Disable the interrupt signals by clearing the bits */
        Cy_I3C_SetInterruptMask(base, 0UL);
        Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS));

        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;

        do
        {
            /* wait till the interrupt of response is received */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && 0UL == ((CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS) & Cy_I3C_GetInterruptStatus(base)));

        if (retStatus == CY_I3C_TIMEOUT){
            return retStatus;
        }

        respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        Cy_I3C_SetInterruptStatusMask(base, 0UL);

        if(0UL != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS_Msk))
        {
            /* unsuccessful due to transfer error */
            retStatus = ResponseError(respCmdPort);
            context->controllerStatus |= CY_I3C_CONTROLLER_HALT_STATE;
        }

        else
        {
            retStatus = CY_I3C_SUCCESS;
        }
        context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
        context->state = CY_I3C_IDLE;
    }

    return retStatus;
}

/*******************************************************************************
*  Function Name: Cy_I3C_ControllerReadByte
****************************************************************************//**
*
* Reads one byte from a target.
* This function is blocking.
*
* \param base
* The pointer to the I3C instance.
*
* \param targetAddress
* The dynamic address of the target I3C target device.
*
* \param data
* The pointer to the location to store the Read byte.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerReadByte(I3C_CORE_Type *base, uint8_t targetAddress, uint8_t *data, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    CY_ASSERT_L2(CY_IS_I3C_ADDR_VALID(targetAddress));

    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;

    if(0UL != (CY_I3C_IDLE_MASK & context->state))
    {
        cy_en_i3c_addr_slot_status_t res;
        cy_stc_i3c_ccc_t cmd;
        uint8_t pos = 0U;
        cy_stc_i3c_controller_devlist_t *i3cDeviceList;
        uint8_t readDSMode = (uint8_t)CY_I3C_SDR0;
        uint32_t respCmdPort, datalen;
        uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

        context->controllerStatus = CY_I3C_CONTROLLER_BUSY;

        cmd.cmdHigh = 0UL;
        cmd.cmdLow = 0UL;

        /* Clear the interrupts */
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

        /* Reset the Tx FIFO */
        I3C_CORE_RESET_CTRL(base) |= I3C_CORE_RESET_CTRL_TX_FIFO_RST_Msk;

        do
        {
            /* wait till the reset is completed */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

        if (retStatus == CY_I3C_TIMEOUT){
            return retStatus;
        }
        
        res = GetAaddrslotStatus(targetAddress, context);

        /* This helps to check if the device is actively present on the bus */
        if((CY_I3C_ADDR_SLOT_I3C_DEV != res ) && (CY_I3C_ADDR_SLOT_I2C_DEV != res))
        {
            return CY_I3C_BAD_PARAM;
        }

        if(CY_I3C_ADDR_SLOT_I2C_DEV == res)
        {
            /* The target device is an i2cDevice */
            pos = (uint8_t)(GetI2CDevAddrPos(base, targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            readDSMode = (uint8_t)((0U != (i3cDeviceList->i2cDevice.lvr & CY_I3C_LVR_I2C_MODE_INDICATOR)) ? CY_I3C_FM_I2C: CY_I3C_FMP_I2C);
            context->controllerStatus |= CY_I3C_CONTROLLER_I2C_SDR_RD_XFER;
        }

        else
        {
            /* The target is an i3cDevice */
            pos = (uint8_t)(GetI3CDevAddrPos(base, targetAddress, context));
            i3cDeviceList = &(context->devList[pos]); /* pointing to the device position in the local list of devices on the bus */
            if(CY_I3C_CORE_BCR_MAX_DATA_SPEED_LIM_Msk == (i3cDeviceList->i3cDevice.bcr))
            {
                readDSMode = (i3cDeviceList->i3cDevice.maxReadDs) & 0x07U;
            }
            context->controllerStatus |= CY_I3C_CONTROLLER_I3C_SDR_RD_XFER;
        }

        cmd.cmdHigh |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, 1U);

        cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SPEED, readDSMode) |
                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_RnW_Msk |
                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk |
                       I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;

        context->state = CY_I3C_CONTROLLER_RX;

        /* Disable the interrupt signals by clearing the bits */
        Cy_I3C_SetInterruptMask(base, 0UL);
        Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS));

        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;

        do
        {
            /* wait till the interrupt of response is received */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && 0UL == ((CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS) & Cy_I3C_GetInterruptStatus(base)));

        if(retStatus == CY_I3C_TIMEOUT){
            return retStatus;
        }

        respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        Cy_I3C_SetInterruptStatusMask(base, 0UL);

        if(0UL != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS_Msk))
        {
            retStatus = ResponseError(respCmdPort);    //unsuccessful due to transfer error
            context->controllerStatus |= CY_I3C_CONTROLLER_HALT_STATE;
        }

        else
        {
            datalen = (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk);
            if(1UL == datalen)
            {
                *data = (uint8_t)Cy_I3C_ReadRxFIFO(base);
                retStatus = CY_I3C_SUCCESS;
            }
            else
            {
                retStatus = CY_I3C_CONTROLLER_ERROR_CE0;
            }
        }
        context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
        context->state = CY_I3C_IDLE;
    }

    return retStatus;
}

/*******************************************************************************
*  Function Name: Cy_I3C_GetBusStatus
****************************************************************************//**
*
* Returns the current I3C bus status.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref group_i3c_macros_controller_status.
* Note that not all I3C controller statuses are returned by this function. Refer to
* more details of each status.
*
*******************************************************************************/
uint32_t Cy_I3C_GetBusStatus(I3C_CORE_Type const *base, cy_stc_i3c_context_t const *context)
{
    if((NULL == base) || (NULL == context))
    {
        return (uint32_t)CY_I3C_BAD_PARAM;
    }

    (void) base;

    /*
    uint32_t ret;
    ret = _FLD2VAL(I3C_CORE_PRESENT_STATE_CM_TFR_STS, I3C_CORE_PRESENT_STATE(base));
    //Gives the state of the transfer currently being executed by the controller

    return ret;
    */

    if(CY_I3C_CONTROLLER == Cy_I3C_GetMode(base))
    {
        return (context->controllerStatus);
    }
    else
    {
        return(context->targetStatus);
    }

}

/* DAA */
/* About DAA:
1. This has the responsibility of finding the number of devices required to be assigned dynamic addresses
2. Getting the free addresses
3. Populating the DAT
4. Populating the COMMAND DATA PORT
5. Poll for the response and update the DAT free pos index based on the response received */

/*******************************************************************************
*  Function Name: Cy_I3C_ControllerStartEntDaa
****************************************************************************//**
*
* Issues ENTDAA CCC command to discover the i3c devices on the bus and assigns
* valid dynamic addresses to the discovered devices.
* This CCC is to be issued also when a device hot joins the bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerStartEntDaa(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    cy_stc_i3c_device_t i3cDev;
    uint32_t i3cDevCount; /* to store the number of devices to be assigned the dynamic address */
    uint8_t dynamicAddress;
    uint8_t index, parity, pos;
    uint32_t respCmdPort;
    uint16_t respLeftDevCount;
    uint32_t value;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;
    cy_en_i3c_status_t retStatus;
    cy_stc_i3c_ccc_cmd_t cccCmd;
    cy_stc_i3c_ccc_payload_t payload;

    if(CY_I3C_MAX_DEVS <= (i3cController->devCount)) /* cannot initialize the device as the bus already has 11 devices on it */
    {
        return CY_I3C_CONTROLLER_MAX_DEVS_PRESENT;
    }

    i3cDevCount = CY_I3C_MAX_DEVS - (i3cController->devCount);

    /* Get "free address" position */
    pos = (uint8_t)(GetDATFreePos(base, context));

    /* Get the 'i3cDevCount' number of free addresses and populate the DAT before issuing ENTDAA CCC */
    for(index = pos; index < i3cDevCount; index++)
    {
        retStatus = Cy_I3C_ControllerGetFreeDeviceAddress(base, &dynamicAddress, context);
        if(CY_I3C_CONTROLLER_FREE_ADDR_UNAVIAL == retStatus)
        {
            return retStatus;
        }

        parity = even_parity(dynamicAddress);
        i3cController->lastAddress = dynamicAddress;
        dynamicAddress |= (parity << 7U);

        /* populate the device address table with the dynamic address of the device */
        value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR, dynamicAddress);

        Cy_I3C_WriteIntoDeviceAddressTable(base, index, value);
    }

    context->controllerStatus = CY_I3C_CONTROLLER_BUSY | CY_I3C_CONTROLLER_ENTDAA_XFER;

    /* Disable the interrupt signals by clearing the bits */
    Cy_I3C_SetInterruptMask(base, 0UL);
    Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS));

    CY_MISRA_DEVIATE_BLOCK_START('MISRA C-2012 Rule 10.8', 1, 'Intentional typecast of CY_I3C_CCC_ENTDAA to uint32_t.')
    I3C_CORE_COMMAND_QUEUE_PORT(base) = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_ADDR_ASSGN_CMD |
                                        _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_CMD, CY_I3C_CCC_ENTDAA) |
                                        _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_DEV_INDX, pos) |
                                        _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_DEV_COUNT, i3cDevCount) |
                                        I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_ROC_Msk |
                                        I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_TOC_Msk;
    CY_MISRA_BLOCK_END('MISRA C-2012 Rule 10.8')

    /* Handling the response */
    do
    {
        retStatus = I3C_Check_Timeout(&timeout);
        /* wait till the interrupt of response is received */
    }while(retStatus == CY_I3C_SUCCESS && 0UL == (CY_I3C_INTR_MASK & Cy_I3C_GetInterruptStatus(base)));

    if(retStatus == CY_I3C_TIMEOUT){
        return retStatus;
    }

    respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);
    Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

    respLeftDevCount = (uint16_t)(respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk); /* Represents the remaining device count */

    i3cController->lastAddress = i3cController->lastAddress - (uint8_t)respLeftDevCount;

    if(0UL != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS_Msk))
    {
        if(I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_BROADCAST_ADDR_NACK_ERROR != (_FLD2VAL(I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS, respCmdPort)))
        {
            /* Error in transfer */
            retStatus = ResponseError(respCmdPort);
            context->controllerStatus |= CY_I3C_CONTROLLER_HALT_STATE;
            context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
            return retStatus;
        }
    }

    Cy_I3C_Resume(base, context);

    i3cController->dynAddrDevCount = (uint32_t)(i3cDevCount - respLeftDevCount);
    i3cController->devCount += i3cController->dynAddrDevCount;

    /* Maintaining a local list of i3c devices */
    for( index = 0U; index < i3cController->dynAddrDevCount; index++)
    {
        Cy_I3C_ReadFromDevCharTable(base, index, &i3cDev);
        SetAddrslotStatus(i3cDev.dynamicAddress, CY_I3C_ADDR_SLOT_I3C_DEV, context);
        (void)RetrieveI3CDeviceInfo(base, &i3cDev, false, context);
        i3cDev.staticAddress = 0U;
        Cy_I3C_UpdateI3CDevInList(&i3cDev, pos, context);
        i3cController->freePos &= ~(CY_I3C_BIT(pos));
        pos++;
    }

    if(0UL != i3cController->dynAddrDevCount)
    {
        /* Send DEFTGTS and ENEC CCC commands */
        cccCmd.address = CY_I3C_BROADCAST_ADDR;
        cccCmd.cmd = CY_I3C_CCC_DEFTGTS;
        cccCmd.data = &payload;
        (void)deftgts_ccc(base, &cccCmd, context);
    }

    context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: Cy_I3C_SetDASA
****************************************************************************//**
*
* Set Dynamic Address from Static Address Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_SetDASA(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint8_t pos ;
    cy_en_i3c_status_t retStatus;
    uint8_t dynAddr;
    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    cy_stc_i3c_device_t i3cDevice = {0UL, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, {0U, 0U, 0U}, 0U, false, false};

    /* Expects 1 byte of data */
    if(0U == cccCmd->data->len)
    {
        return CY_I3C_BAD_PARAM;
    }

    context->controllerStatus |= CY_I3C_CONTROLLER_DIRECTED_CCC_WR_XFER;

    pos = (uint8_t)(GetDATFreePos(base, context));

    setda_ccc(base, cccCmd, context);

    /* Handling the response */
    retStatus = ControllerHandleCCCResponse(base, NULL, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    dynAddr = *(uint8_t *)(cccCmd->data->data);

    /* Maintaining a local list of i3c devices */
    i3cDevice.dynamicAddress = dynAddr;
    i3cDevice.staticAddress = cccCmd->address;
    SetAddrslotStatus(dynAddr, CY_I3C_ADDR_SLOT_I3C_DEV, context);
    (void)RetrieveI3CDeviceInfo(base, &i3cDevice, true, context);
    Cy_I3C_UpdateI3CDevInList(&i3cDevice, pos, context);

    (i3cController->freePos) =  ~ (CY_I3C_BIT(pos));
    (i3cController->devCount)++;

    return CY_I3C_SUCCESS;
}


/*******************************************************************************
*  Function Name: Cy_I3C_ControllerSendHdrCmds
****************************************************************************//**
*
* Writes data provided by xferConfig structure \ref cy_stc_i3c_controller_xfer_config_t
* to a specific device.
*
* \param base
* The pointer to the I3C instance.
*
* \param targetAddress
* The dynamic address of the target I3C device.
*
* \param hdrCmd
* The pointer to HDR command description structure \ref cy_stc_i3c_hdr_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* ref cy_en_i3c_status_t.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_ControllerSendHdrCmds(I3C_CORE_Type *base, uint8_t targetAddress, cy_stc_i3c_hdr_cmd_t *hdrCmd, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == hdrCmd) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    uint8_t pos;
    cy_en_i3c_addr_slot_status_t res;
    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;

    if(0UL != (CY_I3C_IDLE_MASK & context->state))
    {
        res = GetAaddrslotStatus(targetAddress, context);

        /* This helps to check if the device is actively present on the bus */
        if((CY_I3C_ADDR_SLOT_I3C_DEV != res ) && (CY_I3C_ADDR_SLOT_I2C_DEV != res))
        {
            return CY_I3C_BAD_PARAM;
        }

        pos = (uint8_t)(GetI3CDevAddrPos(base, targetAddress, context));

        cy_stc_i3c_controller_devlist_t *i3cDevice = &(context->devList[pos]);

        if(false == (i3cDevice->i3cDevice.hdrSupport))
        {
            /* The device doesn't support HDR mode */
            return CY_I3C_NOT_HDR_CAP;
        }

        if(0UL != (hdrCmd->code & CY_I3C_HDR_IS_READ_CMD))
        {
            /* Read command */
            uint32_t numToRead;
            numToRead = (hdrCmd->ndatawords * 2UL);
            context->hdrCmd = hdrCmd;
            context->controllerBuffer = (uint8_t *)hdrCmd->data.in;
            context->controllerBufferSize = numToRead;
            context->destDeviceAddr = targetAddress;
            context->controllerStatus = CY_I3C_CONTROLLER_BUSY;

            ControllerHDRRead(base, hdrCmd, context);
        }

        else
        {
            /* WRITE command */
            context->hdrCmd = hdrCmd;
            context->controllerBuffer = (uint8_t *)hdrCmd->data.out;
            context->controllerBufferSize = (hdrCmd->ndatawords * 2UL);
            context->destDeviceAddr = targetAddress;
            context->controllerStatus = CY_I3C_CONTROLLER_BUSY;

            ControllerHDRWrite(base, hdrCmd, context);
        }
        retStatus = CY_I3C_SUCCESS;
    }

    return retStatus;
}

/*******************************************************************************
*  Function Name: Cy_I3C_Interrupt
****************************************************************************//**
*
* This is an I3C interrupt handler helper function.
* This function must be called inside the user-defined interrupt service.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_I3C_Interrupt (I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);

    if (0UL == (I3C_CORE_DEVICE_CTRL_EXTENDED(base) & I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_Msk))
    {
        /* Execute a transfer as a Controller */
        Cy_I3C_ControllerInterrupt(base, context);
    }
    else
    {
        /* Execute a transfer as a Target */
        Cy_I3C_TargetInterrupt(base, context);
    }
}


/*******************************************************************************
* Function Name: Cy_I3C_RegisterEventCallback
****************************************************************************//**
*
* Registers an event handler callback function of type cy_cb_i3c_handle_events_t
* which will be invoked by the PDL to indicate i3c events and results.
*
* \param base
* The pointer to the I3C instance.
*
* \param callback
* The pointer to a callback function.
* See \ref cy_cb_i3c_handle_events_t for the function prototype.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* To remove the callback, pass NULL as the pointer to the callback function.
*
*******************************************************************************/
void Cy_I3C_RegisterEventCallback(I3C_CORE_Type const *base, cy_cb_i3c_handle_events_t callback, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);

    /* Suppress a compiler warning about unused variable */
    (void) base;

    context->cbEvents = callback;
}

/*******************************************************************************
* Function Name: Cy_I3C_RegisterIbiCallback
****************************************************************************//**
*
* Registers an IBI handler callback function of type cy_cb_i3c_handle_ibi_t
* which will be invoked when an IBI event is triggered on bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param callback
* The pointer to a callback function.
* See \ref cy_cb_i3c_handle_events_t for the function prototype.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* To remove the callback, pass NULL as the pointer to the callback function.
*
*******************************************************************************/
void Cy_I3C_RegisterIbiCallback (I3C_CORE_Type const *base, cy_cb_i3c_handle_ibi_t callback, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);

    /* Suppress a compiler warning about unused variable */
    (void) base;

    context->cbIbi = callback;
}


/*******************************************************************************
*  Function Name: Cy_I3C_TargetGetDynamicAddress
****************************************************************************//**
*
* Provides the controller assigned dynamic address of the target device.
*
* \param base
* The pointer to the I3C instance.
*
* \param address
* The pointer to the address.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_TargetGetDynamicAddress(I3C_CORE_Type const *base, uint8_t *address, cy_stc_i3c_context_t const *context)
{
    if((NULL == base) || (NULL == address) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }
    if(0UL != (_FLD2VAL(I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR_VALID, I3C_CORE_DEVICE_ADDR(base))))
    {
        *address = (uint8_t)_FLD2VAL(I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR, I3C_CORE_DEVICE_ADDR(base));
        return CY_I3C_SUCCESS;
    }

    return CY_I3C_ADDR_INVALID;
}

/*******************************************************************************
*  Function Name: Cy_I3C_TargetGetMaxReadLength
****************************************************************************//**
*
* Provides the maximum data read length of the target.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Max data read length.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
uint32_t Cy_I3C_TargetGetMaxReadLength(I3C_CORE_Type const *base, cy_stc_i3c_context_t const *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);
    CY_UNUSED_PARAMETER(context); /* Suppress a compiler warning about unused variables */

    return (uint32_t)_FLD2VAL(I3C_CORE_TGT_MAX_LEN_MRL, I3C_CORE_TGT_MAX_LEN(base));
}


/*******************************************************************************
*  Function Name: Cy_I3C_TargetGetMaxWriteLength
****************************************************************************//**
*
* Provides the maximum data write length of the target.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Max write data length.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
uint32_t Cy_I3C_TargetGetMaxWriteLength(I3C_CORE_Type const *base, cy_stc_i3c_context_t const *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);
    CY_UNUSED_PARAMETER(context); /* Suppress a compiler warning about unused variables */

    return (uint32_t)_FLD2VAL(I3C_CORE_TGT_MAX_LEN_MWL, I3C_CORE_TGT_MAX_LEN(base));
}


/*******************************************************************************
*  Function Name: Cy_I3C_TargetGenerateIbi
****************************************************************************//**
*
* Generates the specified IBI on the bus.
*
* \param base
* The pointer to the I3C instance.
*
* \param ibitype
* The pointer to the ibi structure \ref cy_stc_i3c_ibi_t containing the
* type of ibi event to be generated.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_TargetGenerateIbi(I3C_CORE_Type *base, cy_stc_i3c_ibi_t *ibitype, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == ibitype) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    uint8_t res;
    uint32_t value = 0UL;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;
    cy_en_i3c_status_t ret = CY_I3C_SUCCESS;

    /* Generate a SIR */
    if(CY_I3C_IBI_SIR == ibitype->event)
    {
        if(0UL != (_FLD2VAL(I3C_CORE_TGT_EVENT_STATUS_TIR_EN, I3C_CORE_TGT_EVENT_STATUS(base))))
        {
            /* When set, the controller attempts to issue the SIR on the bus. Once set, the application cannot clear this bit
            SIR_CTRL field of this reg is to be set to 0 */

            Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
            /*  Disable the interrupt signals by clearing the bits */
            Cy_I3C_SetInterruptMask(base, 0UL);
            Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_UPDATED_STS);
            I3C_CORE_TGT_INTR_REQ(base) = I3C_CORE_TGT_INTR_REQ_TIR_Msk;
        }
        else
        {
            /* Return that Controller has disabled SIR IBI */
            return CY_I3C_SIR_DISABLED;
        }
    }

    /* Generate a controllership request */
    else if((CY_I3C_IBI_CONTROLLER_REQ == ibitype->event) && (CY_I3C_SECONDARY_CONTROLLER == context->i3cMode))
    {
        if(0UL != (_FLD2VAL(I3C_CORE_TGT_EVENT_STATUS_CR_EN, I3C_CORE_TGT_EVENT_STATUS(base))))
        {

            Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
            /* Disable the interrupt signals by clearing the bits */
            Cy_I3C_SetInterruptMask(base, 0UL);
            Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_UPDATED_STS | I3C_CORE_INTR_STATUS_BUSOWNER_UPDATED_STS_Msk);
            /* When set, the controller attempts to issue the MR on the bus. Once set, the application cannot clear this bit */
            I3C_CORE_TGT_INTR_REQ(base) = I3C_CORE_TGT_INTR_REQ_CR_Msk;
        }
        else
        {
            /* Return that Controller has disabled MR IBI */
            return CY_I3C_MR_DISABLED;
        }
    }

    else
    {
        /*
            When targets initiates a hot-join ibi generation event or
            when a non-secondary controller tries requesting for Controllership
        */
        return CY_I3C_BAD_EVENT_REQ;
    }

    /* Handling the response */
    do
    {
        ret = I3C_Check_Timeout(&timeout);
        /* wait till the interrupt of response is received */
    }while(ret == CY_I3C_SUCCESS && 0UL == (CY_I3C_INTR_IBI_UPDATED_STS & Cy_I3C_GetInterruptStatus(base)));

    if(ret == CY_I3C_TIMEOUT){
        return ret;
    }

    /* Handle the response */
    res = (uint8_t)_FLD2VAL(I3C_CORE_TGT_INTR_REQ_IBI_STS, I3C_CORE_TGT_INTR_REQ(base));

    if(I3C_CORE_TGT_INTR_REQ_IBI_STS_IBI_NOT_ATTEMPTED == res)
    {
        /* IBI not attempted, if
        *  1. Controller has not assigned the dynamic address
        *  2. Controller has cleared the assigned dynamic address
        *  3. Controller has disabled the IBI (DISEC)
        *  4. The controller has switched to controller mode */

        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        Cy_I3C_SetInterruptMask(base, CY_I3C_TGT_INTR_Msk);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_TGT_INTR_Msk);
        ret = CY_I3C_IBI_NOT_ATTEMPTED;
    }

    else
    {
        /* IBI is ACKed */
        if(CY_I3C_IBI_CONTROLLER_REQ == ibitype->event)
        {
            /* IBI is acked, check if the controllership is delivered */
            Cy_SysLib_Delay(1000UL);

            value = _FLD2VAL(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE, I3C_CORE_DEVICE_CTRL_EXTENDED(base));

            Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

            if(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_TARGET == value)
            {
                ret = CY_I3C_MR_DISABLED;
                Cy_I3C_SetInterruptMask(base, CY_I3C_TGT_INTR_Msk);
                Cy_I3C_SetInterruptStatusMask(base, CY_I3C_TGT_INTR_Msk);
            }

            else
            {
                Cy_I3C_Resume(base, context);
                (void)SecondaryControllerInit(base, true, context);
            }
        }
    }
    return ret;
}


/*******************************************************************************
* Function Name: Cy_I3C_TargetConfigReadBuf
****************************************************************************//**
*
* Configures the buffer pointer and the read buffer size. This is the buffer
* from which the controller reads data. After this function is called, data
* transfer from the read buffer to the controller is handled by
* \ref Cy_I3C_Interrupt.
*
* When the Read transaction is completed \ref CY_I3C_TARGET_WR_CMPLT is set.
* Also the \ref CY_I3C_TARGET_WR_CMPLT_EVENT event is generated.
*
* \param base
* The pointer to the I3C instance.
*
* \param buffer
* The pointer to the buffer with data to be read by the controller.
*
* \param size
* Size of the buffer.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
void Cy_I3C_TargetConfigReadBuf (I3C_CORE_Type const *base, uint8_t *buffer, uint32_t size, cy_stc_i3c_context_t *context)
{
     CY_ASSERT_L1(NULL != base);
     CY_ASSERT_L1(NULL != context);
     CY_ASSERT_L1(CY_IS_I3C_BUFFER_VALID(buffer, size));

    /* Suppress a compiler warning about unused variables */
    (void) base;

    context->targetTxBuffer     = buffer;
    context->targetTxBufferSize = size;
    context->targetTxBufferIdx  = 0UL;
    context->targetTxBufferCnt  = 0UL;
}


/*******************************************************************************
* Function Name: Cy_I3C_TargetGetReadTransferCount
****************************************************************************//**
*
* Returns the number of bytes read by the controller since the last time
* \ref Cy_I3C_TargetConfigReadBuf was called.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* The number of bytes read by the controller.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
uint32_t Cy_I3C_TargetGetReadTransferCount (I3C_CORE_Type const *base, cy_stc_i3c_context_t const *context)
{
    CY_ASSERT_L1(NULL != base);

    /* Suppress a compiler warning about unused variables */
    (void)base;

    return (context->targetTxBufferCnt);
}

/*******************************************************************************
* Function Name: Cy_I3C_TargetConfigWriteBuf
****************************************************************************//**
*
* Configures the buffer pointer and the write buffer size. This is the buffer
* that the controller writes data to. After this function is called, data
* transfer from the controller into the write buffer is handled by
* \ref Cy_I3C_Interrupt.
*
* When the write transaction is completed \ref CY_I3C_TARGET_RD_CMPLT is set.
* Also the \ref CY_I3C_TARGET_RD_CMPLT_EVENT event is generated.
*
* \param base
* The pointer to the I3C instance.
*
* \param buffer
* The pointer to buffer to store data written by the controller.
*
* \param size
* Size of the buffer.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
void Cy_I3C_TargetConfigWriteBuf(I3C_CORE_Type const *base, uint8_t *buffer, uint32_t size, cy_stc_i3c_context_t *context)
{
    CY_ASSERT_L1(NULL != base);
    CY_ASSERT_L1(NULL != context);
    CY_ASSERT_L1(CY_IS_I3C_BUFFER_VALID(buffer, size));

    /* Suppress a compiler warning about unused variables */
    (void) base;

    context->targetRxBuffer     = buffer;
    context->targetRxBufferSize = size;
    context->targetRxBufferIdx  = 0UL;
}


/*******************************************************************************
* Function Name: Cy_I3C_TargetGetWriteTransferCount
****************************************************************************//**
*
* Returns the number of bytes written by the controller since the last time
* \ref Cy_I3C_TargetConfigWriteBuf was called.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* The number of bytes written by the controller.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
uint32_t Cy_I3C_TargetGetWriteTransferCount (I3C_CORE_Type const *base, cy_stc_i3c_context_t const *context)
{
    CY_ASSERT_L1(NULL != base);

    /* Suppress a compiler warning about unused variables */
    (void) base;

    return (context->targetRxBufferCnt);
}


/*******************************************************************************
* Function Name: Cy_I3C_DeliverControllership
****************************************************************************//**
* Delivers the bus controllership to the requesting secondary controller.
*
* \param base
* The pointer to the I3C instance.
*
* \param SecControllerAddress
* The address of the I3C device to which the controllership has to be delivered.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* This should be called by the user when a controllership request IBI is received
* through cy_cb_i3c_handle_ibi_t callback and user wants to deliver controller ship.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_DeliverControllership(I3C_CORE_Type *base, uint8_t SecControllerAddress, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }
    cy_stc_i3c_ccc_cmd_t cccCmd;
    cy_stc_i3c_ccc_getacccr_t getacccr;
    cy_stc_i3c_ccc_payload_t payload;
    uint8_t addr;
    cy_en_i3c_status_t errorStatus;

    addr = SecControllerAddress;

    cccCmd.address = addr;
    cccCmd.cmd = ((uint8_t)CY_I3C_CCC_GETACCCR);
    cccCmd.data = &payload;
    cccCmd.data->data = &(getacccr.newcontroller);

    errorStatus = getacccr_ccc(base, &cccCmd, context);
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

    if(CY_I3C_SUCCESS == errorStatus)
    {
        if(addr != (getacccr.newcontroller >> 1U))
        {
            errorStatus = CY_I3C_ADDR_MISMATCH;
        }

        else
        {
            /* Controllership is transferred and the controller is in the Target Mode */

            uint32_t value;

            value = _FLD2VAL(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE,I3C_CORE_DEVICE_CTRL_EXTENDED(base));

            if(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_TARGET == value)
            {
                /* Mode is changed to target */
                cy_en_i3c_status_t  status;

                Cy_I3C_Resume(base, context);
                I3C_CORE_RESET_CTRL(base) |= (I3C_CORE_RESET_CTRL_IBI_QUEUE_RST_Msk | I3C_CORE_RESET_CTRL_SOFT_RST_Msk);

                do
                {
                /* wait till the reset is completed */
                errorStatus = I3C_Check_Timeout(&timeout);
                }while(errorStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

                if (errorStatus == CY_I3C_TIMEOUT){
                return errorStatus;
                }
                
                status = SecondaryControllerInit(base, false, context);

                return status;
            }

        }
    }
    return errorStatus;
}


/*******************************************************************************
* Function Name: Cy_I3C_RequestControllership
****************************************************************************//**
*
* Requests controllership from the current controller.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_RequestControllership(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    if((NULL == base) || (NULL == context))
    {
        return CY_I3C_BAD_PARAM;
    }

    if(CY_I3C_SECONDARY_CONTROLLER != context->i3cMode)
    {
        return CY_I3C_NOT_SECONDARY_CONTROLLER;
    }

    cy_en_i3c_status_t retStatus;
    cy_stc_i3c_ibi_t ibi;

    retStatus = Cy_I3C_TargetGetDynamicAddress(base, &(ibi.targetAddress), context);

    if(CY_I3C_SUCCESS == retStatus)
    {
        ibi.event = CY_I3C_IBI_CONTROLLER_REQ;
        retStatus = Cy_I3C_TargetGenerateIbi(base, &ibi, context);
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: SecondaryControllerInit
****************************************************************************//**
*
* This function does the basic Controller Mode configurations when the operation
* mode of the Secondary Controller is changed from I3C Target to I3C Controller.
*
* \param base
* The pointer to the I3C instance.
*
* \param isController
* true: Controller mode initializations.
* false: Target mode initializations.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static cy_en_i3c_status_t SecondaryControllerInit(I3C_CORE_Type *base, bool isController, cy_stc_i3c_context_t *context)
{
    if(isController)
    {
        cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
        uint8_t address, idx;
        uint32_t value;
        uint8_t staticAddress, dynamicAddress, dcr, bcr, i2c;
        cy_stc_i3c_device_t i3cDevice;
        cy_stc_i2c_device_t i2cDevice;

        i3cController->freePos = 0x7FFUL;
        i3cController->lastAddress = 0U;
        i3cController->devCount = 0U;
        i3cController->i2cDeviceCount = 0U;
        i3cController->dynAddrDevCount = 0U;

        address = (uint8_t)(_FLD2VAL(I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR, I3C_CORE_DEVICE_ADDR(base)));

        InitAddrslots(context);
        SetAddrslotStatus(address, CY_I3C_ADDR_SLOT_I3C_DEV, context);

        (void)Cy_I3C_SetDataRate(base, context->i3cSclRate, context->i3cClockHz, context);

        Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);

        for(idx = 0U; idx < context->targetRxBufferCnt; idx++)
        {
            dynamicAddress = (uint8_t)(I3C_CORE_DEV_CHAR_TABLE1_LOC1(base) & 0xFFUL);
            staticAddress = (uint8_t)((I3C_CORE_DEV_CHAR_TABLE1_LOC1(base) & 0xFF000000UL) >> 24UL);
            dcr = (uint8_t)((I3C_CORE_DEV_CHAR_TABLE1_LOC1(base) & 0xFF00UL) >> 8UL);
            bcr = (uint8_t)((I3C_CORE_DEV_CHAR_TABLE1_LOC1(base) & 0xFF0000UL) >> 16UL);

            i2c = (0U != dynamicAddress) ? 0U : 1U;

            value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR, dynamicAddress) |
                    _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR, staticAddress) |
                    _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC10_LEGACY_I2C_DEVICE, i2c);

            Cy_I3C_WriteIntoDeviceAddressTable(base, idx, value);

            if(0UL != i2c)
            {
                /* I2C Device */
                SetAddrslotStatus(staticAddress, CY_I3C_ADDR_SLOT_I2C_DEV, context);
                i2cDevice.lvr = dcr;
                i2cDevice.staticAddress = staticAddress;

                Cy_I3C_UpdateI2CDevInList(&i2cDevice, idx, context);

                (i3cController->freePos) = ~ (CY_I3C_BIT(idx));
                (i3cController->devCount)++;
                (i3cController->i2cDeviceCount)++;
            }
            else
            {
                /* I3C Device */
                SetAddrslotStatus(dynamicAddress, CY_I3C_ADDR_SLOT_I3C_DEV, context);

                i3cDevice.bcr = bcr;
                i3cDevice.dcr = dcr;
                i3cDevice.dynamicAddress = dynamicAddress;
                (void)RetrieveI3CDeviceInfo(base, &i3cDevice, false, context);
                i3cDevice.staticAddress = 0U;
                Cy_I3C_UpdateI3CDevInList(&i3cDevice, idx, context);

                i3cController->freePos &= ~(CY_I3C_BIT(idx));
                (i3cController->devCount)++;
            }
        }
    }

    else
    {
        I3C_CORE_TGT_EVENT_STATUS(base) &= (~I3C_CORE_TGT_EVENT_STATUS_HJ_EN_Msk);

        /* Sets the number of entries in the Receive FIFO that trigger the interrupt to 1 word */
        I3C_CORE_DATA_BUFFER_THLD_CTRL(base) &= _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD, 5UL) |
                                                _VAL2FLD(I3C_CORE_DATA_BUFFER_STATUS_LEVEL_TX_BUF_EMPTY_LOC, 1UL);

        Cy_I3C_SetInterruptMask(base, CY_I3C_TGT_INTR_Msk | CY_I3C_INTR_DEFTGT_STS);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_TGT_INTR_Msk | CY_I3C_INTR_DEFTGT_STS);
    }

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_I3C_TargetInterrupt
****************************************************************************//**
*
* This is the interrupt function for the I3C configured in target mode.
* This function should be called inside the user-defined interrupt service routine.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \note
* I3C supports only Primary Controller mode. Secondary controller mode and Target mode are not supported in the current release.
*
*******************************************************************************/
void Cy_I3C_TargetInterrupt (I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    uint32_t intrCause;
    intrCause = Cy_I3C_GetInterruptStatus(base);

    if(0UL != (CY_I3C_INTR_TX_BUFFER_THLD_STS & intrCause))
    {
        TargetHandleDataTransmit(base, context);
    }

    if(0UL != (CY_I3C_INTR_RX_BUFFER_THLD_STS & intrCause))
    {
        TargetHandleDataReceive(base, context);
    }

    if(0UL != (CY_I3C_INTR_RESP_READY_STS & intrCause))
    {
        TargetRespReadyStsHandle(base, context);
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_RESP_READY_STS);
    }

    if(0UL != (CY_I3C_INTR_CCC_UPDATED_STS & intrCause))
    {
        if(0UL != (_FLD2VAL(I3C_CORE_TGT_EVENT_STATUS_MRL_UPDATED, I3C_CORE_TGT_EVENT_STATUS(base))))
        {
            context->cbEvents(CY_I3C_TARGET_MAX_RD_LEN_UPDT_EVENT);
            I3C_CORE_TGT_EVENT_STATUS(base) |= I3C_CORE_TGT_EVENT_STATUS_MRL_UPDATED_Msk;
        }
        else if(0UL != (_FLD2VAL(I3C_CORE_TGT_EVENT_STATUS_MWL_UPDATED, I3C_CORE_TGT_EVENT_STATUS(base))))
        {
            context->cbEvents(CY_I3C_TARGET_MAX_WR_LEN_UPDT_EVENT);
            I3C_CORE_TGT_EVENT_STATUS(base) |= I3C_CORE_TGT_EVENT_STATUS_MWL_UPDATED_Msk;
        }
        else
        {
            context->cbEvents(CY_I3C_TARGET_CCC_REG_UPDATED_EVENT);
        }

        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_CCC_UPDATED_STS);
    }

    if(0UL != (CY_I3C_INTR_DYN_ADDR_ASSGN_STS & intrCause))
    {
        context->cbEvents(CY_I3C_TARGET_ASSIGNED_DYN_ADDR_EVENT);
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_DYN_ADDR_ASSGN_STS);
    }

    /* Read request from the controller */
    if(0UL != (CY_I3C_INTR_READ_REQ_RECV_STS & intrCause))
    {
        if(NULL != (context->cbEvents))
        {
            /* Callback - in case of error */
            uint32_t event;
            event = CY_I3C_TARGET_NO_VALID_CMD_IN_CMDQ_EVENT;
            if(0UL != (_FLD2VAL(I3C_CORE_CCC_DEVICE_STATUS_DATA_NOT_READY, I3C_CORE_CCC_DEVICE_STATUS(base))))
            {
                event |= CY_I3C_TARGET_DATA_NOT_READY_EVENT;
            }
            context->cbEvents(event);
        }
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
    }

    if(0UL != (CY_I3C_INTR_BUSOWNER_UPDATED_STS & intrCause))
    {
        if(NULL != (context->cbEvents))
        {
            /* Callback - in case of error  */
            context->cbEvents(CY_I3C_CONTROLLER_ROLE_UPDATED_EVENT);
        }
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        return;
    }

    if(0UL != (CY_I3C_INTR_DEFTGT_STS & intrCause))
    {
        uint32_t idx;
        idx = context->targetRxBufferCnt;

        I3C_CORE_DEV_CHAR_TABLE_POINTER(base) |= _VAL2FLD(I3C_CORE_DEV_CHAR_TABLE_POINTER_PRESENT_DEV_CHAR_TABLE_INDX,idx);
        if(NULL != (context->cbEvents))
        {
            /* Callback - in case of error  */
            context->cbEvents(CY_I3C_DEFTGT_EVENT);
        }
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        return;
    }
}


/*******************************************************************************
* Function Name: Cy_I3C_ControllerInterrupt
****************************************************************************//**
*
* This is the interrupt function for the I3C configured in controller mode.
* This function should be called inside the user-defined interrupt service routine.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
void Cy_I3C_ControllerInterrupt (I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    uint32_t intrCause;
    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;
    intrCause = Cy_I3C_GetInterruptStatus(base);

    if(0UL != (CY_I3C_INTR_RESP_READY_STS & intrCause))
    {
        if(0UL != (CY_I3C_INTR_TRANSFER_ERR_STS & intrCause))
        {
            uint32_t respCmdPort, errorEvent;
            respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);
            errorEvent = ResponseErrorEvent(respCmdPort);
            context->controllerStatus |= CY_I3C_CONTROLLER_HALT_STATE;
            context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
            context->state = CY_I3C_IDLE;

            if(NULL != (context->cbEvents))
            {
                /* Callback - in case of error  */
                context->cbEvents(errorEvent);
            }
            Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK); //Check if this clearing in not harmful
            Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
            Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
            return;
        }

        else
        {
            ControllerRespReadyStsHandle(base, context);
        }

        Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
        return;
    }

    if(0UL != (CY_I3C_INTR_TRANSFER_ABORT_STS & intrCause))
    {
        //Test - Check: Does this set the TRANSFER_ERROR_STS and the error field too?
        context->controllerStatus |= CY_I3C_CONTROLLER_XFER_ABORTED | CY_I3C_CONTROLLER_HALT_STATE;
        context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
        context->state = CY_I3C_IDLE;

        I3C_CORE_RESET_CTRL(base) = I3C_CORE_RESET_CTRL_SOFT_RST_Msk;   
        do
        {
            /* wait till the reset is completed */
            retStatus = I3C_Check_Timeout(&timeout);
        }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

        if(NULL != context->cbEvents)
        {
            context->cbEvents(CY_I3C_XFER_ABORTED_ERROR_EVENT);
        }

        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
        Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);

        return;
    }

    if(0UL != (CY_I3C_INTR_IBI_BUFFER_THLD_STS & intrCause))
    {
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

        ControllerHandleIBIInterrupt(base, context);

        return;
    }

    if(0UL != (CY_I3C_INTR_BUSOWNER_UPDATED_STS & intrCause))
    {
        if(NULL != (context->cbEvents))
        {
            /* Callback - in case of error  */
            context->cbEvents(CY_I3C_CONTROLLER_ROLE_UPDATED_EVENT);
        }
        Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);
        return;
    }

}

/*******************************************************************************
* Function Name: Cy_I3C_DeepSleepCallback_After
****************************************************************************//**
*
* This function handles transition of I3C controller after wake up from
* deep sleep mode.
*
* \param callbackParams
* The pointer to the callback parameters structure
* \ref cy_stc_syspm_callback_params_t.
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
static cy_en_syspm_status_t Cy_I3C_DeepSleepCallback_After(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t result = CY_SYSPM_FAIL;

    I3C_CORE_Type *locBase = (I3C_CORE_Type *) callbackParams->base;
    cy_stc_i3c_context_t *locContext = (cy_stc_i3c_context_t *) callbackParams->context;
    cy_stc_i3c_config_t *locConfig = &locContext->dsConfig;

    switch (locContext->i3cMode)
    {
        case CY_I3C_CONTROLLER:

            /* Setting the device operation mode to Controller - 0U */
            I3C_CORE_DEVICE_CTRL_EXTENDED(locBase) = _VAL2FLD(I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE, I3C_CORE_DEVICE_CTRL_EXTENDED_DEV_OPERATION_MODE_CONTROLLER);

            if(!locConfig->manualDataRate)
            {
                (void)Cy_I3C_SetDataRate(locBase, locContext->i3cSclRate, locContext->i3cClockHz, locContext);
            }
            else
            {
                 I3C_CORE_SCL_I3C_OD_TIMING(locBase) = _VAL2FLD(I3C_CORE_SCL_I3C_OD_TIMING_I3C_OD_HCNT, locConfig->openDrainHighCnt) |
                                                    _VAL2FLD(I3C_CORE_SCL_I3C_OD_TIMING_I3C_OD_LCNT, locConfig->openDrainLowCnt);
                 I3C_CORE_SCL_I3C_PP_TIMING(locBase) = _VAL2FLD(I3C_CORE_SCL_I3C_PP_TIMING_I3C_PP_HCNT, locConfig->pushPullHighCnt) |
                                                    _VAL2FLD(I3C_CORE_SCL_I3C_PP_TIMING_I3C_PP_LCNT, locConfig->pushPullLowCnt);
                 I3C_CORE_SCL_I2C_FM_TIMING(locBase) = _VAL2FLD(I3C_CORE_SCL_I2C_FM_TIMING_I2C_FM_HCNT, locConfig->i2cFMHighCnt) |
                                                    _VAL2FLD(I3C_CORE_SCL_I2C_FM_TIMING_I2C_FM_LCNT, locConfig->i2cFMLowCnt);
                 I3C_CORE_SCL_I2C_FMP_TIMING(locBase) = _VAL2FLD(I3C_CORE_SCL_I2C_FMP_TIMING_I2C_FMP_HCNT, locConfig->i2cFMPlusHighCnt) |
                                                     _VAL2FLD(I3C_CORE_SCL_I2C_FMP_TIMING_I2C_FMP_LCNT, locConfig->i2cFMPlusLowCnt);
                 I3C_CORE_SCL_EXT_LCNT_TIMING(locBase) = _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_1, locConfig->extLowCnt1) |
                                                      _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_2, locConfig->extLowCnt2) |
                                                      _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_3, locConfig->extLowCnt3) |
                                                      _VAL2FLD(I3C_CORE_SCL_EXT_LCNT_TIMING_I3C_EXT_LCNT_4, locConfig->extLowCnt4);
                I3C_CORE_SCL_EXT_TERMN_LCNT_TIMING(locBase) = _VAL2FLD(I3C_CORE_SCL_EXT_TERMN_LCNT_TIMING_I3C_EXT_TERMN_LCNT, locConfig->extTerminationLowCnt);
            }

            I3C_CORE_DEVICE_CTRL(locBase) = _VAL2FLD(I3C_CORE_DEVICE_CTRL_IBA_INCLUDE, locConfig->ibaInclude ? 1UL : 0UL) |
                                         _VAL2FLD(I3C_CORE_DEVICE_CTRL_HOT_JOIN_CTRL, locConfig->hotJoinCtrl ? 1UL : 0UL);

            I3C_CORE_DEVICE_ADDR(locBase) = _VAL2FLD( I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR, locConfig->dynamicAddr) |
                                         I3C_CORE_DEVICE_ADDR_DYNAMIC_ADDR_VALID_Msk;

            I3C_CORE_BUS_FREE_AVAIL_TIMING(locBase) |= _VAL2FLD(I3C_CORE_BUS_FREE_AVAIL_TIMING_BUS_FREE_TIME, locConfig->busFreeTime);

            I3C_CORE_SDA_HOLD_SWITCH_DLY_TIMING(locBase) = _VAL2FLD(I3C_CORE_SDA_HOLD_SWITCH_DLY_TIMING_SDA_TX_HOLD, locConfig->sdaHoldTime);

            I3C_CORE_QUEUE_THLD_CTRL(locBase) = _VAL2FLD(I3C_CORE_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD, locConfig->cmdQueueEmptyThld) |
                                             _VAL2FLD(I3C_CORE_QUEUE_THLD_CTRL_RESP_BUF_THLD, locConfig->respQueueThld) |
                                             _VAL2FLD(I3C_CORE_QUEUE_THLD_CTRL_IBI_STATUS_THLD, locConfig->ibiQueueThld);

            I3C_CORE_DATA_BUFFER_THLD_CTRL(locBase) = _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD, locConfig->rxBufThld) |
                                                    _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD, locConfig->txEmptyBufThld) |
                                                    _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_RX_START_THLD, locConfig->rxBufStartThld) |
                                                    _VAL2FLD(I3C_CORE_DATA_BUFFER_THLD_CTRL_TX_START_THLD, locConfig->txBufStartThld);

            result = CY_SYSPM_SUCCESS;

        break;

        case CY_I3C_TARGET:
            result = CY_SYSPM_SUCCESS;
        break;

        case CY_I3C_SECONDARY_CONTROLLER:
            result = CY_SYSPM_SUCCESS;
        break;
        default:
            result = CY_SYSPM_SUCCESS;
        break;
    }

    /* restore interrupt mask*/
    Cy_I3C_SetInterruptMask(locBase, locContext->intr_mask);
    Cy_I3C_SetInterruptStatusMask(locBase, locContext->intr_mask);
    Cy_I3C_Enable(locBase);

    return result;

}
/*******************************************************************************
* Function Name: Cy_I3C_DeepSleepCallback
****************************************************************************//**
*
* This function handles transition of I3C controller into and out of
* deep sleep mode.
*
* \param callbackParams
* The pointer to the callback parameters structure
* \ref cy_stc_syspm_callback_params_t.
*
* \param mode
* Callback mode, see \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
cy_en_syspm_status_t Cy_I3C_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
   I3C_CORE_Type *locBase = (I3C_CORE_Type *) callbackParams->base;
    cy_stc_i3c_context_t *locContext = (cy_stc_i3c_context_t *) callbackParams->context;
    cy_en_syspm_status_t retStatus = CY_SYSPM_FAIL;

    switch(mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Check for cmd/rsp/ibi/Rx/Tx FIFO is empty and bus condition is idle*/
            if(false == Cy_I3C_IsBusBusy((I3C_CORE_Type const *)locBase) && (locContext->state == CY_I3C_IDLE))
            {
                /*save the intr mask and disable all intr*/
                locContext->intr_mask = Cy_I3C_GetInterruptMask(locBase);
                Cy_I3C_SetInterruptMask(locBase, 0UL);
                Cy_I3C_SetInterruptStatusMask(locBase, 0UL);
                Cy_I3C_Disable(locBase);
                retStatus = CY_SYSPM_SUCCESS;
            }
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* enable IP */
            Cy_I3C_SetInterruptMask(locBase, locContext->intr_mask);
            Cy_I3C_SetInterruptStatusMask(locBase, locContext->intr_mask);
            Cy_I3C_Enable(locBase);

            retStatus = CY_SYSPM_SUCCESS;
        }
        break;

        case CY_SYSPM_BEFORE_TRANSITION:
        {
            /* Clear the interrupts*/
            Cy_I3C_ClearInterrupt(locBase, CY_I3C_INTR_MASK);
            retStatus = CY_SYSPM_SUCCESS;
        }
        break;

        case CY_SYSPM_AFTER_TRANSITION:
        {
            /* Apply all config, restore interrupts and enable IP */
           retStatus = Cy_I3C_DeepSleepCallback_After(callbackParams);
        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
        break;
    }

    return (retStatus);
}

/*******************************************************************************
* Function Name: Cy_I3C_HibernateCallback
****************************************************************************//**
*
* This function handles transition of I3C controller into hibernate mode.
*
* \param callbackParams
* The pointer to the callback parameters structure
* \ref cy_stc_syspm_callback_params_t.
*
* \param mode
* Callback mode, see \ref cy_en_syspm_callback_mode_t
*
* \return
* \ref cy_en_syspm_status_t
*
*******************************************************************************/
cy_en_syspm_status_t Cy_I3C_HibernateCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    CY_UNUSED_PARAMETER(callbackParams); /* Suppress a compiler warning about unused variables */
    CY_UNUSED_PARAMETER(mode); /* Suppress a compiler warning about unused variables */

    /* There is no retention of any REG, For hibernate all data
       needed should be managed by the user in backup domain retained memory*/

    return CY_SYSPM_FAIL;
}

/*******************************************************************************
* Function Name: even_parity
****************************************************************************//**
*
* Finds the parity of the address.
*
* \param address
* 7-bit right justified address.
*
* \return
* The parity of the address.
* 0: odd number of one bits in the address
* 1: even number of one bits in the address.
*
*******************************************************************************/
static uint8_t even_parity(uint8_t address)
{
    address ^= address >> 4U;
    address &= 0xFU;

    return (uint8_t)((0x9669U >> address) & 1U);
}

/*******************************************************************************
* Function Name: ffsq
****************************************************************************//**
*
* Finds the position of the first set bit in the given value.
*
* \param value
* Input value.
*
* \return
* The position of the first set bit.
*
*******************************************************************************/
static uint32_t FirstSetBit(uint32_t value)
{
    uint32_t r = 1UL;

    if (!((bool)value))
    {
        return 0UL;
    }
    if (!((bool)(value & 0xFFFFUL)))
    {
        value >>= 16UL;
        r += 16UL;
    }
    if (!((bool)(value & 0xFFUL)))
    {
        value >>= 8UL;
        r += 8UL;
    }
    if (!((bool)(value & 0xFUL)))
    {
        value >>= 4UL;
        r += 4UL;
    }
    if (!((bool)(value & 3UL)))
    {
        value >>= 2UL;
        r += 2UL;
    }
    if (!((bool)(value & 1UL)))
    {
        r += 1UL;
    }
    return r;
}

/*******************************************************************************
*  Function Name: SetAddrslotStatus
****************************************************************************//**
*
* Marks the status of the address parameter as defined
* by \ref cy_en_i3c_addr_slot_status_t.
*
* \param address
* 7-bit right justified address.
*
* \param status
* The status to be assigned to the address.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_config_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void SetAddrslotStatus(uint8_t address, cy_en_i3c_addr_slot_status_t status, cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    uint8_t bitPos = address * 2U;
    unsigned long *ptr;

    ptr = i3cController->addrslotsStatusArray + (bitPos / CY_I3C_BITS_PER_LONG);
    *ptr &= ~(CY_I3C_ADDR_SLOT_STATUS_MASK << (bitPos % CY_I3C_BITS_PER_LONG));
    *ptr |= (unsigned long)status << (bitPos % CY_I3C_BITS_PER_LONG);
}

/*******************************************************************************
*  Function Name: InitAddrslots
****************************************************************************//**
*
* Initializes the status \ref cy_en_i3c_addr_slot_status_t of the addresses.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_config_t allocated
* by the user. The structure is used during the I2C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void InitAddrslots(cy_stc_i3c_context_t *context)
{
    uint8_t index;

    /* Set all the addresses as free initially */
    for(index = 0U; index < ((CY_I3C_MAX_ADDR + 1U) * 2U) / CY_I3C_BITS_PER_LONG; index++)
    {
        SetAddrslotStatus(index, CY_I3C_ADDR_SLOT_FREE, context);
    }

    /* Assigning reserved status for addresses from 1-7 */
    for(index = 0U; index < 8U; index++)
    {
        SetAddrslotStatus(index, CY_I3C_ADDR_SLOT_RSVD, context);
    }

    /* Assigning reserved status for the broadcast */
    SetAddrslotStatus(CY_I3C_BROADCAST_ADDR, CY_I3C_ADDR_SLOT_RSVD, context);

    /* Assigning reserved status for the following broadcast addresses:
        0x7E, 0x7D, 0x7C, 0x7A, 0x7B, 0x78, 0x79 */
    for (index = 1U; index < 8U; index++)
    {
        SetAddrslotStatus((uint8_t)(CY_I3C_BROADCAST_ADDR ^ CY_I3C_BIT(index)), CY_I3C_ADDR_SLOT_RSVD, context);
    }

}

/*******************************************************************************
*  Function Name: DeInitAddrslots
****************************************************************************//**
*
* Sets the status of the addresses to free.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void DeInitAddrslots(cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    uint8_t index;
    /*  Sets status of all the addresses as free */
    for(index = 0U; index < (((CY_I3C_MAX_ADDR + 1U) * 2U) / CY_I3C_BITS_PER_LONG); index++)
    {
            i3cController->addrslotsStatusArray[index] = 0UL;
    }
}

/*******************************************************************************
*  Function Name: GetI2CDevAddrPos
****************************************************************************//**
*
* Obtains the position of the I2C device with the specified static address
* in the DAT.
*
* \param base
* The pointer to the I3C instance.
*
* \param staticAddress
* The static address of the I2C device.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Position of the I2C device in the DAT.
*
*******************************************************************************/
static uint32_t GetI2CDevAddrPos(I3C_CORE_Type *base, uint8_t staticAddress, cy_stc_i3c_context_t *context)
{
    uint8_t index;

    CY_UNUSED_PARAMETER(context);

    for(index = 0U; index < CY_I3C_MAX_DEVS; index++)
    {
        if(staticAddress == Cy_I3C_ReadStaticAddrFromDAT(base, index))
        {
            return (uint32_t)index;
        }
    }

    return (uint32_t)CY_I3C_BAD_PARAM;
}

/*******************************************************************************
*  Function Name: GetI3CDevAddrPos
****************************************************************************//**
*
* Obtains the position of the I3C device with the specified dynamic address
* in the DAT.
*
* \param base
* The pointer to the I3C instance.
*
* \param dynamicAddress
* The dynamic address of the I3C device.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Position of the I3C device in the DAT.
*
*******************************************************************************/
static uint32_t GetI3CDevAddrPos(I3C_CORE_Type *base, uint8_t dynamicAddress, cy_stc_i3c_context_t *context)
{
    uint8_t index;

    CY_UNUSED_PARAMETER(context); /* Suppress a compiler warning about unused variables */

    for(index = 0U; index < CY_I3C_MAX_DEVS; index++)
    {
        uint32_t ret;
        ret = Cy_I3C_ReadDynAddrFromDAT(base, index);
        if(dynamicAddress == ret)
        {
            return (uint32_t)index;
        }
    }

    return (uint32_t)CY_I3C_BAD_PARAM;
}

/*******************************************************************************
*  Function Name: GetDATFreePos
****************************************************************************//**
*
* Obtains the position of free location in the DAT.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allo * by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* Position of the free location in the DAT.
*
*******************************************************************************/
static uint32_t GetDATFreePos(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    uint32_t ret;
    ret = FirstSetBit(context->i3cController.freePos) - 1UL;
    CY_UNUSED_PARAMETER(base); /* Suppress a compiler warning about unused variables */

    return ret;
}

/*******************************************************************************
*  Function Name: GetAaddrslotStatus
****************************************************************************//**
*
* Obtains the status of the address parameter.
*
* \param address
* 7-bit right justified address.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_addr_slot_status_t.
*
*******************************************************************************/
static cy_en_i3c_addr_slot_status_t GetAaddrslotStatus(uint8_t address, cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    uint8_t bitPos = address * 2U;
    uint32_t status;

    if (address > CY_I3C_MAX_ADDR)
    {
        return CY_I3C_ADDR_SLOT_RSVD;
    }

    status = (i3cController->addrslotsStatusArray[bitPos / CY_I3C_BITS_PER_LONG]);
    status >>= bitPos % CY_I3C_BITS_PER_LONG;

    CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 10.8','Intentional typecast to cy_en_i3c_addr_slot_status_t enum.');
    return (cy_en_i3c_addr_slot_status_t)(status & CY_I3C_ADDR_SLOT_STATUS_MASK);
}

/*******************************************************************************
*  Function Name: ResponseError
****************************************************************************//**
*
* Provides the error type received in the response status.
*
* \param respCmd
* The value read from the response queue port register
*
* \return
* \ref cy_en_i3c_status_t.

*******************************************************************************/
static cy_en_i3c_status_t ResponseError(uint32_t respCmd)
{
    uint8_t error;
    error = (uint8_t)(_FLD2VAL(I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS, respCmd));
    cy_en_i3c_status_t ret = (cy_en_i3c_status_t)0U;

    switch (error)
    {
        case 1U: ret = CY_I3C_CONTROLLER_CRC_ERROR; break;
        case 2U: ret = CY_I3C_CONTROLLER_PARITY_ERROR; break;
        case 3U: ret = CY_I3C_CONTROLLER_FRAME_ERROR; break;
        case 4U: ret = CY_I3C_CONTROLLER_BROADCAST_ADDR_NACK_ERROR; break;
        case 5U: ret = CY_I3C_CONTROLLER_ADDR_NACK_ERROR; break; //This is same as ERROR CE2
        case 6U: ret = CY_I3C_CONTROLLER_BUFFER_OVERFLOW_ERROR; break;
        case 8U: ret = CY_I3C_CONTROLLER_XFER_ABORTED_ERROR; break;
        case 9U: ret = CY_I3C_CONTROLLER_I2C_TGT_WDATA_NACK_ERROR; break;
        default: /* Unknown Error */ break;
    }
    return ret;
}

/*******************************************************************************
*  Function Name: ResponseErrorEvent
****************************************************************************//**
*
* Provides error events to be passed by \ref cy_cb_i3c_handle_events_t callback
* based on the type of error type received in the response status.
*
* \param respCmd
* The value read from the response queue port register.
*
* \return
* \ref group_i3c_macros_callback_events.

******************************************************************************/
static uint32_t ResponseErrorEvent(uint32_t respCmd)
{
    uint32_t error;
    error = _FLD2VAL(I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS, respCmd);
    uint32_t ret = 0UL;

    switch (error)
    {
        case 1U: ret = CY_I3C_CRC_ERROR_EVENT; break;
        case 2U: ret = CY_I3C_PARITY_ERROR_EVENT; break;
        case 3U: ret = CY_I3C_FRAME_ERROR_EVENT; break;
        case 4U: ret = CY_I3C_BROADCAST_ADDR_NACK_ERROR_EVENT; break;
        case 5U: ret = CY_I3C_ADDR_NACK_ERROR_EVENT; break;
        case 6U: ret = CY_I3C_BUFFER_OVERFLOW_ERROR_EVENT; break;
        case 8U: ret = CY_I3C_XFER_ABORTED_ERROR_EVENT; break;
        case 9U: ret = CY_I3C_I2C_TGT_WDATA_NACK_ERROR_EVENT; break;
        case 10U: ret = CY_I3C_CONTROLLER_EARLY_TERMINATION_EVENT; break;
        default: /* Unknown Error */ break;

    }
    return ret;
}

/*******************************************************************************
* Function Name: WriteArray
****************************************************************************//**
*
* Places an array of data in the transmit FIFO.
* This function does not block. It returns how many data elements were
* placed in the transmit FIFO.
*
* \param base
* The pointer to the I3C instance.
*
* \param buffer
* The pointer to data to place in the transmit FIFO.
*
* \param size
* The number of data elements to transmit.
*
*******************************************************************************/
static void WriteArray(I3C_CORE_Type *base, void *buffer, uint32_t size)
{
    uint32_t *buf = buffer;
    uint32_t index;

    /*Put data in TX FIFO */
    for(index = 0UL; index < (size / 4UL); ++index)
    {
        Cy_I3C_WriteTxFIFO(base, *(buf + index));
    }

    if(0UL != (size & 3UL))
    {
        uint8_t cnt = (uint8_t)(size & 3UL);
        uint8_t *srcPtr, *destPtr;
        uint32_t value = 0UL;
        destPtr = (uint8_t *)(&value);
        srcPtr = (uint8_t *)(buf + index);

        for(index = 0UL; index < cnt; index++)
        {
            *destPtr = *srcPtr;
            destPtr++;
            srcPtr++;
        }
        Cy_I3C_WriteTxFIFO(base, value);
    }
}


/*******************************************************************************
* Function Name: ReadArray
****************************************************************************//**
*
* Reads an array of data out of the receive FIFO.
* This function does not block.
*
* \param base
* The pointer to the I3C instance.
*
* \param buffer
* The pointer to location to place data read from receive FIFO.
*
* \param size
* The number of data elements to read from the receive FIFO.
*
*******************************************************************************/
static void ReadArray(I3C_CORE_Type *base, void *buffer, uint32_t size)
{
    uint32_t index;
    uint8_t *buf = buffer;
    uint32_t value = 0UL;

    for(index = 0UL; index < size ; index++)
    {
        if (index % 4UL == 0UL){
            value = Cy_I3C_ReadRxFIFO(base);
        }
        *(buf + index) = (uint8_t)(value & 0xFFUL);
        value = value>>8UL;
    }

}

/*******************************************************************************
* Function Name: ControllerHandleDataTransmit
****************************************************************************//**
*
* Loads TX FIFO with data provided by \ref Cy_I3C_ControllerWrite.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void ControllerHandleDataTransmit(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
        if (context->controllerBufferSize > 1UL)
        {
            /* Get the number of bytes to copy into TX FIFO */
            uint32_t numToCopy = context->controllerBufferSize;

            /* Write data into TX FIFO */
            WriteArray(base, context->controllerBuffer, numToCopy);
        }
}

/*******************************************************************************
* Function Name: ControllerHandleDataReceive
****************************************************************************//**
*
* Reads data from RX FIFO into the buffer provided by \ref Cy_I3C_ControllerRead.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void ControllerHandleDataReceive(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    uint32_t numToRead;
    numToRead = context->controllerBufferSize;
    //Read from the RX FIFO
    ReadArray(base, context->controllerBuffer, numToRead);
}


/*******************************************************************************
*  Function Name: RetrieveI3CDeviceInfo
****************************************************************************//**
*
* To retrieve device information which includes: max write len, max read len,
* max data speed, HDR capability, BCR, DCR and PID.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDevice
* The pointer to the I3C device description structure \ref cy_stc_i3c_device_t.
*
* \param basicInfo
* true: obtains BCR, DCR and PID of the device
* false: otherwise
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t RetrieveI3CDeviceInfo(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, bool basicInfo, cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_ccc_cmd_t cccCmd;
    cy_stc_i3c_ccc_payload_t payload;
    cccCmd.address = i3cDevice->dynamicAddress;
    cccCmd.data = &payload;
    cy_en_i3c_status_t retStatus;

    if(true == basicInfo)
    {
        cy_stc_i3c_ccc_getbcr_t bcr;
        cccCmd.data->data = &bcr;
        cccCmd.cmd = CY_I3C_CCC_GETBCR;
        retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        i3cDevice->bcr = bcr.bcr;

        cy_stc_i3c_ccc_getdcr_t dcr;
        cccCmd.data->data = &dcr;
        cccCmd.cmd = CY_I3C_CCC_GETDCR;
        retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        i3cDevice->dcr = dcr.dcr;

        cy_stc_i3c_ccc_getpid_t pid;
        cccCmd.data->data = &pid;
        cccCmd.cmd = CY_I3C_CCC_GETPID;
        retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        uint8_t *ptr;
        uint8_t cnt;
        uint32_t shift;
        ptr = pid.pid;
        i3cDevice->provisonalID = 0UL;

        for(cnt = 0U; cnt < sizeof(pid); cnt++)
        {
            shift = (sizeof(pid) - cnt - 1U) * 8U;
            (i3cDevice->provisonalID) |= ((uint64_t)(*ptr)) << shift;
            ptr++;
        }
    }

    /* Get max read len */
    cy_stc_i3c_ccc_mrwl_t mrwl;
    cccCmd.data->data = &mrwl;
    cccCmd.cmd = CY_I3C_CCC_GETMRL;
    retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }
    i3cDevice->mrl = mrwl.len;

    /* Get max write len */
    cccCmd.cmd = CY_I3C_CCC_GETMWL;
    retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }
    i3cDevice->mwl = mrwl.len;

    if(0U != (CY_I3C_CORE_BCR_MAX_DATA_SPEED_LIM_Msk & (i3cDevice->bcr)))
    {
        cy_stc_i3c_ccc_getmxds_t mxds;
        i3cDevice->speedLimit = true;
        cccCmd.data->data = &mxds;
        cccCmd.cmd = CY_I3C_CCC_GETMXDS;
        retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        i3cDevice->maxReadDs = mxds.maxrd;
        i3cDevice->maxWriteDs = mxds.maxwr;
        i3cDevice->maxReadTurnaround[0] = mxds.maxrdturn[0];
        i3cDevice->maxReadTurnaround[1] = mxds.maxrdturn[1];
        i3cDevice->maxReadTurnaround[2] = mxds.maxrdturn[2];
    }

    else
    {
        i3cDevice->speedLimit = false;
    }

    if(0U != (CY_I3C_CORE_BCR_HDR_CAP_Msk & (i3cDevice->bcr)))
    {
        cy_stc_i3c_ccc_gethdrcap_t hdrcap;
        i3cDevice->hdrSupport = true;
        cccCmd.data->data = &hdrcap;
        cccCmd.cmd = CY_I3C_CCC_GETHDRCAP;
        retStatus = Cy_I3C_SendCCCCmd(base, &cccCmd, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        i3cDevice->HDRCap = hdrcap.modes;
    }

    else
    {
        i3cDevice->hdrSupport = false;
    }
    return retStatus;
}

/*******************************************************************************
*  Function Name: CCC_Set
****************************************************************************//**
*
* Prepares commands for scheduling CCC transfers where the CCCs are not required
* to retrieve data from the target device.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void CCC_Set(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd,  cy_stc_i3c_context_t *context)
{
    uint32_t pos = 0UL;   //pos = 0 in case of broadcast command.
    uint16_t dataLen = cccCmd->data->len;
    cy_stc_i3c_ccc_t cmd;

    /* Check if the cmd is a Direct or broadcast command;
    If Direct command, get the address offset in the DAT */
    if(0U != (cccCmd->cmd & CY_I3C_CCC_DIRECT))
    {
        pos = GetI3CDevAddrPos(base, cccCmd->address, context);
    }

      uint8_t validBytes;
      uint8_t *data;
      data = (uint8_t *)cccCmd->data->data;

      switch(dataLen)
       {
        case 0U: { cmd.cmdHigh = 0UL;
                  cmd.cmdLow = 0UL;
                  break; }

        case 1U: {
                  validBytes = CY_I3C_BYTE_STROBE1;
                  cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, validBytes) |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0,*data);

                  cmd.cmdLow = I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk;
                  break; }
        case 2U: {
                  validBytes = CY_I3C_BYTE_STROBE2;
                  cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, validBytes) |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0,*(data + 1U)) |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_1,*(data));

                  cmd.cmdLow = I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk;

                  break; }
        case 3U: {
                  validBytes = CY_I3C_BYTE_STROBE3;
                  cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_SHORT_DATA_ARG |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_BYTE_STRB, validBytes) |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_0,*(data + 2))|
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_1,*(data + 1)) |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_SHORT_DATA_ARG_DATA_BYTE_2,*data);

                  cmd.cmdLow = I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SDAP_Msk;

                  break; }
        default: {
                  //Commands with greater than 3 bytes of payload
                  //Commands without payload; Also broadcast commands
                  cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, dataLen);

                  cmd.cmdLow = 0UL;
                  break; }
    }

    cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                   I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CP_Msk |
                   _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CMD, cccCmd->cmd) |
                   _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                   I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk |
                   I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;

    /*For broadcast CCC set RnW bit*/
    if(0U == (cccCmd->cmd & CY_I3C_CCC_DIRECT))
    {
       cmd.cmdLow |= I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_RnW_Msk;
    }
    // Disable the interrupt signals by clearing the bits
    Cy_I3C_SetInterruptMask(base, 0UL);
    Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS));

    if(cmd.cmdHigh != 0UL){
        I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
    }
    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;
}

/*******************************************************************************
*  Function Name: CCC_Get
****************************************************************************//**
*
* Prepares commands for scheduling CCC transfers where the CCCs should retrieve
* data from the target device.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void CCC_Get(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint32_t pos = 0UL;
    cy_stc_i3c_ccc_t cmd;
    uint16_t dataLen = cccCmd->data->len;

    /* Direct command, get the address offset in the DAT */
    pos = GetI3CDevAddrPos(base, cccCmd->address, context);

    /* DATA_LENGTH field indicates the expected number of bytes from the Target.
        DATA_LENGTH should be provided by the calling API and not the user */

    cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                    _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, dataLen);


    cmd.cmdLow = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                 I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CP_Msk |
                 _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CMD, cccCmd->cmd) |
                 _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                 I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_RnW_Msk |
                 I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk |
                 I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;

    /* Disable the interrupt signals by clearing the bits */
    Cy_I3C_SetInterruptMask(base, 0UL);
    Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS));

    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh ;
    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow ;
}

/*******************************************************************************
*  Function Name: enec_disec_ccc
****************************************************************************//**
*
* Enable/Disable Target Events Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t enec_disec_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;

    retStatus = CCCTargetAddressValidation(cccCmd->address, false, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    /* Expects 1 byte of data */
    if(0U == cccCmd->data->len)
    {
        return CY_I3C_BAD_PARAM;
    }

    CCC_Set(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, NULL, context);

    return retStatus;
}

/*******************************************************************************
*  Function Name: rstdaa_ccc
****************************************************************************//**
*
* Reset Dynamic Address Assignment Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t rstdaa_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    uint32_t pos = 0UL;
    uint32_t devCount;

    retStatus = CCCTargetAddressValidation(cccCmd->address, false, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    CCC_Set(base, cccCmd, context);

    /* Handling of unicast command */
    if(CY_I3C_BROADCAST_ADDR != cccCmd->address)
    {
        retStatus = ControllerHandleCCCResponse(base, NULL, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        pos = GetI3CDevAddrPos(base, cccCmd->address, context);

        Cy_I3C_WriteIntoDeviceAddressTable(base, (uint8_t)pos, 0UL);

        context->i3cController.freePos |= CY_I3C_BIT(pos);
        context->i3cController.devCount--;

        SetAddrslotStatus(cccCmd->address, CY_I3C_ADDR_SLOT_FREE, context);
    }

    /* Handling of broadcast command */
    else
    {
        retStatus = ControllerHandleCCCResponse(base, NULL, context);
        if(CY_I3C_SUCCESS != retStatus)
        {
            return retStatus;
        }
        devCount = context->i3cController.devCount;
        for(pos = 0UL; pos < devCount; pos++)
        {
            Cy_I3C_WriteIntoDeviceAddressTable(base, (uint8_t)pos, 0UL);

            context->i3cController.freePos |= CY_I3C_BIT(pos);
            context->i3cController.devCount--;
            context->i3cController.dynAddrDevCount--;
        }

        context->i3cController.lastAddress = 0U;
        DeInitAddrslots(context);
        InitAddrslots(context);
        SetAddrslotStatus(context->dsConfig.dynamicAddr, CY_I3C_ADDR_SLOT_I3C_DEV, context);

    }

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: setmrwl_ccc
****************************************************************************//**
*
* Set Maximum Read/Write Length Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t setmrwl_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint8_t pos;
    uint8_t idx;
    cy_en_i3c_status_t retStatus;
    uint16_t *data;

    retStatus = CCCTargetAddressValidation(cccCmd->address, false, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    /* Expects 2 byte of data */
    if(0U == cccCmd->data->len)
    {
        return CY_I3C_BAD_PARAM;
    }

    CCC_Set(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, NULL, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    /* Set maximum read length for the target device */
    if( CY_I3C_CCC_SETMRL(false) == cccCmd->cmd)
    {
        pos = (uint8_t)(GetI3CDevAddrPos(base, cccCmd->address, context));
        data = (uint16_t *)(cccCmd->data->data);
        context->devList[pos].i3cDevice.mrl = *data;
    }

    /* Set maximum write length for the target device */
    else if(CY_I3C_CCC_SETMWL(false) == cccCmd->cmd)
    {
        pos = (uint8_t)(GetI3CDevAddrPos(base, cccCmd->address, context));
        data = (uint16_t *)(cccCmd->data->data);
        context->devList[pos].i3cDevice.mwl = *data;
    }

    /* Set maximum read length for all the target I3C devices - broadcast command */
    else if(CY_I3C_CCC_SETMRL(true) == cccCmd->cmd)
    {
        cy_stc_i3c_controller_devlist_t *devList = context->devList;
        for(idx = 0U; idx < (Cy_I3C_GetI3CDeviceCount(base, context)); idx++)
        {
            if(!devList->i2c)
            {
                data = (uint16_t *)(cccCmd->data->data);
                devList->i3cDevice.mrl = *data;
            }
            devList++;
        }
    }

    /* Set maximum write length for all the target I3C devices - broadcast command */
    else
    {
        cy_stc_i3c_controller_devlist_t *devList = context->devList;
        for(idx = 0U; idx < (Cy_I3C_GetI3CDeviceCount(base, context)); idx++)
        {
            if(!devList->i2c)
            {
                data = (uint16_t *)(cccCmd->data->data);
                devList->i3cDevice.mwl = *data;
            }
            devList++;
        }
    }

    context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
    return CY_I3C_SUCCESS;

}

/*******************************************************************************
*  Function Name: setda_ccc
****************************************************************************//**
*
* Prepares and sends commands into the command queue for SETDASA and SETNEWDA CCCs.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void setda_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint8_t pos;
    uint8_t *data;
    uint32_t value;
    data = (uint8_t *)cccCmd->data->data;

    value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR, *data) |
            _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR, cccCmd->address);

    pos = (uint8_t)(GetDATFreePos(base, context));
    Cy_I3C_WriteIntoDeviceAddressTable(base, pos, value);

    /* Disable the interrupt signals by clearing the bits */
    Cy_I3C_SetInterruptMask(base, 0UL);
    Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS));

    I3C_CORE_COMMAND_QUEUE_PORT(base)= I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_ADDR_ASSGN_CMD |
                                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_CMD, cccCmd->cmd) |
                                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_DEV_INDX, pos) |
                                       _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_DEV_COUNT, 1U) |
                                       I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_ROC_Msk |
                                       I3C_CORE_COMMAND_QUEUE_PORT_ADDR_ASSGN_CMD_TOC_Msk;

}

/*******************************************************************************
*  Function Name: setnewda_ccc
****************************************************************************//**
*
* Set New Dynamic Address Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t setnewda_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint32_t  value;
    cy_en_i3c_status_t retStatus;
    uint8_t *data;
    uint8_t parity, pos;

    /* Expects 1 byte of data */
    if(0U == cccCmd->data->len)
    {
        return CY_I3C_BAD_PARAM;
    }

    data = (uint8_t *)cccCmd->data->data;

    (*data) = (*data << 1U);

    cccCmd->data->data = data;

    context->controllerStatus |= CY_I3C_CONTROLLER_DIRECTED_CCC_WR_XFER;

    CCC_Set(base, cccCmd, context);

    (*data) = (*data >> 1U);

    /* Handling the response */
    retStatus = ControllerHandleCCCResponse(base, NULL, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    pos = (uint8_t)(GetI3CDevAddrPos(base, cccCmd->address, context));

    SetAddrslotStatus(*data, CY_I3C_ADDR_SLOT_I3C_DEV, context);

    context->devList[pos].i3cDevice.dynamicAddress = *data; /* Updating the local list */

    parity = even_parity(*data);

    (*data) |= (parity << 7U);

    value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR, *data);

    Cy_I3C_WriteIntoDeviceAddressTable(base, pos, value); //Updating the Device Address Table

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: entas_ccc
****************************************************************************//**
*
* Enter Activity State Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t entas_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;

    retStatus = CCCTargetAddressValidation(cccCmd->address, false, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    CCC_Set(base, cccCmd, context);

    Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

    Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
    Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);

    return retStatus;
}

/*******************************************************************************
*  Function Name: enthdr0_ccc
****************************************************************************//**
*
* Enter HDR0 Mode Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t enthdr0_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    
    if(CY_I3C_BROADCAST_ADDR != cccCmd->address)
    {
        return CY_I3C_BAD_PARAM;
    }

    cccCmd->data->len = 0U;

    context->controllerStatus |= CY_I3C_CONTROLLER_BROADCAST_CCC_WR_XFER;

    CCC_Set(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, NULL, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getmrwl_ccc
****************************************************************************//**
*
* Get Maximum Read/Write Length Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getmrwl_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    uint16_t data;
    cy_stc_i3c_ccc_mrwl_t *mrwl;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    /* Expects 2 bytes of data */
    cccCmd->data->len = ((uint16_t)sizeof(*mrwl));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    /* if(sizeof(*mrwl) != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    /* Read data from the FIFO to the data_field of the cmd structure */
    data = (uint16_t)(Cy_I3C_ReadRxFIFO(base));
    data = CY_I3C_SWAP16(data); /* MSB byte will be returned first and then LSB */

    mrwl = (cy_stc_i3c_ccc_mrwl_t *)cccCmd->data->data;
    mrwl->len = data;

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getpid_ccc
****************************************************************************//**
*
* Get Provisional ID Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getpid_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint8_t index;
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    uint8_t data[6U] = {0};
    cy_stc_i3c_ccc_getpid_t *getpid;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }
    /* Expects 2 bytes of data */
    cccCmd->data->len = (uint16_t)(sizeof(*getpid));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    /*if(sizeof(*getpid) != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    /* Read data from the FIFO to the data_field of the cmd structure */
    ReadArray(base, data, sizeof(*getpid));
    getpid = (cy_stc_i3c_ccc_getpid_t *)cccCmd->data->data;

    for(index = 0U; index< sizeof(*getpid); index++)
    {
        getpid->pid[5U - index] = data[index];/* MSB byte will be received first */
    }

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getbcr_ccc
****************************************************************************//**
*
* Get Bus Characteristic Register Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getbcr_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    uint8_t data;
    cy_stc_i3c_ccc_getbcr_t *getbcr;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }
   /* Expects 1 byte of data */
    cccCmd->data->len = (uint16_t)(sizeof(*getbcr));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

/*    if(sizeof(*getbcr) != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    /* Read data from the FIFO to the data_field of the cmd structure */
    data = (uint8_t)(Cy_I3C_ReadRxFIFO(base));

    getbcr = (cy_stc_i3c_ccc_getbcr_t *)cccCmd->data->data;
    getbcr->bcr = data;

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getdcr_ccc
****************************************************************************//**
*
* Get Bus Characteristic Register Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getdcr_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    uint8_t data;
    cy_stc_i3c_ccc_getdcr_t *getdcr;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }
    /* Expects 1 byte of data */
    cccCmd->data->len = (uint16_t)(sizeof(*getdcr));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    /*if(sizeof(*getdcr) != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    /* Read data from the FIFO to the data_field of the cmd structure */
    data =(uint8_t)(Cy_I3C_ReadRxFIFO(base));

    getdcr = (cy_stc_i3c_ccc_getdcr_t *)cccCmd->data->data;
    getdcr->dcr = data;

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getstatus_ccc
****************************************************************************//**
*
* Get Device Status Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getstatus_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    uint16_t data;
    cy_stc_i3c_ccc_getstatus_t *getStatus;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    /* Expects 2 bytes of data */
    cccCmd->data->len =(uint16_t)( sizeof(*getStatus));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

/*    if(sizeof(*getStatus) != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    /* Read data from the FIFO to the data_field of the cmd structure */
    data = (uint16_t)(Cy_I3C_ReadRxFIFO(base));
    data = CY_I3C_SWAP16(data);

    getStatus = (cy_stc_i3c_ccc_getstatus_t *)cccCmd->data->data;
    getStatus->status = data;

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getmxds_ccc
****************************************************************************//**
*
* Get Max Data Speed Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getmxds_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{

    uint8_t idx;
    uint16_t data;
    uint8_t dataMTR[5U]={0};
    uint32_t recvDataLen;
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    cy_stc_i3c_ccc_getmxds_t *getmxds;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    /* Expects 2 or 5 bytes of data */
    cccCmd->data->len = (uint16_t)(sizeof(*getmxds));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }

    recvDataLen = respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk;

    if((0UL != (2UL & recvDataLen)) || (0UL != (5UL & recvDataLen)))
    {
        return CY_I3C_CONTROLLER_ERROR_CE0; /* ERROR_CE0 - Illegally formatted CCC */
    }

    /* Read data from the FIFO to the data_field of the cmd structure */
    if(2UL == recvDataLen)
    {
        data = (uint16_t)(Cy_I3C_ReadRxFIFO(base));
        getmxds = (cy_stc_i3c_ccc_getmxds_t *)cccCmd->data->data;

        getmxds->maxwr = (uint8_t)data;
        getmxds->maxrd = (uint8_t)(data >> 8U);
    }

    /* Read data from the FIFO to the data_field of the cmd structure */
    else
    {
        /* 5 == recvDataLen */
        ReadArray(base, dataMTR, sizeof(*getmxds));
        getmxds = (cy_stc_i3c_ccc_getmxds_t *)cccCmd->data->data;
        getmxds->maxwr = dataMTR[0U];
        getmxds->maxrd = dataMTR[1U];

        for( idx = 2U; idx < sizeof(*getmxds); idx++)
        {
            getmxds->maxrdturn[idx - 2U] = dataMTR[idx]; //CHECK - if MSB of maxrdturn is copied into first element of struct array while checking
        }
    }
    return CY_I3C_SUCCESS;

}

/*******************************************************************************
*  Function Name: gethdrcap_ccc
****************************************************************************//**
*
* Get HDR Capability Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t gethdrcap_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    uint32_t respCmdPort = 0UL;
    uint8_t data;
    cy_stc_i3c_ccc_gethdrcap_t *gethdrcap;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    /* Expects 1 byte of data */
    cccCmd->data->len = (uint16_t)(sizeof(*gethdrcap));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }


/*    if(sizeof(*gethdrcap) != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    /* Read data from the FIFO to the data_field of the cmd structure */
    data = (uint8_t)(Cy_I3C_ReadRxFIFO(base));

    gethdrcap = (cy_stc_i3c_ccc_gethdrcap_t *)cccCmd->data->data;
    gethdrcap->modes = data;

    return CY_I3C_SUCCESS;
}

/*******************************************************************************
*  Function Name: getacccr_ccc
****************************************************************************//**
*
* Get Accept Controllership Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t getacccr_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_status_t retStatus;
    //uint32_t recvDataLen;
    uint32_t respCmdPort = 0UL;
    uint8_t data;
    cy_stc_i3c_ccc_getacccr_t *getacccr;

    retStatus = CCCTargetAddressValidation(cccCmd->address, true, context);

    if(retStatus != CY_I3C_SUCCESS)
    {
        return retStatus;
    }

    cccCmd->data->len = (uint16_t)(sizeof(getacccr));

    CCC_Get(base, cccCmd, context);

    retStatus = ControllerHandleCCCResponse(base, &respCmdPort, context);

    if(CY_I3C_SUCCESS != retStatus)
    {
        return retStatus;
    }


/*    recvDataLen = respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk;
    if(sizeof(*getacccr) & recvDataLen)
        return CY_I3C_CONTROLLER_ERROR_CE0; //ERROR_CE0 - Illegally formatted CCC */

    data = (uint8_t)(Cy_I3C_ReadRxFIFO(base));

    getacccr = (cy_stc_i3c_ccc_getacccr_t *)cccCmd->data->data;
    getacccr->newcontroller = data;

    return CY_I3C_SUCCESS;

}


/*******************************************************************************
*  Function Name: deftgts_ccc
****************************************************************************//**
*
* Define List of Targets Command.
*
* \param base
* The pointer to the I3C instance.
*
* \param cccCmd
* The pointer to the I3C CCC structure \ref cy_stc_i3c_ccc_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t deftgts_ccc(I3C_CORE_Type *base, cy_stc_i3c_ccc_cmd_t *cccCmd, cy_stc_i3c_context_t *context)
{
    uint8_t index;
    cy_stc_i3c_controller_t *i3cController = &(context->i3cController);
    cy_stc_i3c_controller_devlist_t *devList = context->devList;
    uint8_t i3cDevCnt;
    bool send = false;
    /* Stores the list to be sent - the targets list */
    cy_stc_i3c_ccc_deftgts_t deftgts;
    cy_stc_i3c_ccc_dev_desc_t targets[CY_I3C_MAX_DEVS];

    /* TBD - Where to get the controller related info - like bcr, dcr and    pid?! */

    /* The data to this CCC is prepared here */
    /* Check for the presence of any secondary controllers on the bus */
    i3cDevCnt = (uint8_t)(i3cController->devCount);

    for(index = 0U; index < (i3cDevCnt); index++)
    {
        if(!devList->i2c)
        {
            /* It is an i3c device */
            if((devList->i3cDevice.bcr & CY_I3C_CORE_BCR_DEVICE_ROLE_Msk) == CY_I3C_CORE_BCR_I3C_CONTROLLER)
            {
                /* The device is secondary controller capable
                Secondary Controller will also have CONTROLLER value in this field */
                send = true;
            }
        }
        devList++;
    }

    if(!send)
    {
        return CY_I3C_NO_SECONDARY_CONTROLLER_DEVICES;
    }

    context->controllerStatus |= CY_I3C_CONTROLLER_BROADCAST_CCC_WR_XFER;

    /* Maintain the list of devices */
    devList = context->devList;

    for(index = 0U; index < (i3cController->devCount); index++)
    {
        if(devList->i2c)
        {
            /* i2c device */
            targets[index].staticAddress = (devList->i2cDevice.staticAddress) << 1U;
            targets[index].dynAddress = 0U;
            targets[index].lvr = devList->i2cDevice.lvr;
            targets[index].bcr = 0U;
        }

        else
        {
            /* i3c device */
            targets[index].staticAddress = (devList->i3cDevice.staticAddress) << 1;
            targets[index].dynAddress = (devList->i3cDevice.dynamicAddress) << 1;
            targets[index].dcr = devList->i3cDevice.dcr;
            targets[index].bcr = devList->i3cDevice.bcr;
        }
    }

    //DOUBT - Where to get the controller info from?
    //deftgts.controller =
    deftgts.count = (uint8_t)(i3cController->devCount); //DOUBT - Should this count include the main controller too?
    deftgts.targets = targets;

    cccCmd->data->data = &(deftgts);
    /* The second term here is the number of bytes holding the I3C Targets' information */
    CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 10.8','Intentional typecast to uint32_t');
    cccCmd->data->len = (uint16_t)(sizeof(deftgts) + ((uint32_t)((sizeof(targets) / CY_I3C_MAX_DEVS)  * (index - 1U))) - sizeof(cy_stc_i3c_ccc_dev_desc_t));

    /* Put the data into the TX fifo */
    WriteArray(base, &deftgts, cccCmd->data->len);

    CCC_Set(base, cccCmd, context);

    //Should the response be handled here?
    return CY_I3C_SUCCESS;

}

/*******************************************************************************
*  Function Name: ControllerHDRWrite
****************************************************************************//**
*
* Writes data provided in the HDR command structure \ref cy_stc_i3c_hdr_cmd_t
* to a specific device in HDR Mode.
*
* \param base
* The pointer to the I3C instance.
*
* \param hdrCmd
* The pointer to the I3C HDR command structure \ref cy_stc_i3c_hdr_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static void ControllerHDRWrite(I3C_CORE_Type *base, cy_stc_i3c_hdr_cmd_t *hdrCmd, cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_ccc_t cmd;
    uint32_t pos;
    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

    /* Clear the interrupts */
    Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

    /* Reset the Tx FIFO */
    I3C_CORE_RESET_CTRL(base) |= I3C_CORE_RESET_CTRL_TX_FIFO_RST_Msk;
    do
    {
        /* wait till the reset is completed */
         retStatus = I3C_Check_Timeout(&timeout);
    }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

    pos = GetI3CDevAddrPos(base, context->destDeviceAddr, context);

    /* Prepare the data for the transfer */
    ControllerHandleDataTransmit(base, context);

    cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                   _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, (context->controllerBufferSize));


    cmd.cmdLow = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CP_Msk |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TID, CY_I3C_CONTROLLER_HDR_WRITE_TID) |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CMD, hdrCmd->code) |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SPEED, CY_I3C_HDR_DDR) |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;

    context->controllerStatus |= CY_I3C_CONTROLLER_HDR_DDR_WR_XFER;
    context->state = CY_I3C_CONTROLLER_TX;

    Cy_I3C_SetInterruptMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));
    Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));

    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;

}

/*******************************************************************************
*  Function Name: ControllerHDRRead
****************************************************************************//**
*
* Reads data from a device specified by HDR command structure
* \ref cy_stc_i3c_hdr_cmd_t.
*
* \param base
* The pointer to the I3C instance.
*
* \param hdrCmd
* The pointer to the I3C HDR command structure \ref cy_stc_i3c_hdr_cmd_t.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static void ControllerHDRRead(I3C_CORE_Type *base, cy_stc_i3c_hdr_cmd_t *hdrCmd, cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_ccc_t cmd;
    uint32_t pos;

    pos = GetI3CDevAddrPos(base, context->destDeviceAddr, context);

    cmd.cmdHigh = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_ARG |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_ARG_DATA_LENGTH, (context->controllerBufferSize));

    cmd.cmdLow = I3C_CORE_COMMAND_QUEUE_PORT_CMD_ATTR_TRANSFER_CMD |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CP_Msk |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TID, CY_I3C_CONTROLLER_HDR_READ_TID) |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_CMD, hdrCmd->code) |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_DEV_INDX, pos) |
                  _VAL2FLD(I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_SPEED, CY_I3C_HDR_DDR) |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_RnW_Msk |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_ROC_Msk |
                  I3C_CORE_COMMAND_QUEUE_PORT_TRANSFER_CMD_TOC_Msk;

    context->controllerStatus |= CY_I3C_CONTROLLER_HDR_DDR_RD_XFER;
    context->state = CY_I3C_CONTROLLER_RX;

    Cy_I3C_SetInterruptMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));
    Cy_I3C_SetInterruptStatusMask(base, (CY_I3C_INTR_TRANSFER_ERR_STS | CY_I3C_INTR_RESP_READY_STS));

    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdHigh;
    I3C_CORE_COMMAND_QUEUE_PORT(base) = cmd.cmdLow;

}


/*******************************************************************************
*  Function Name: ControllerRespReadyStsHandle
****************************************************************************//**
*
* Handles the response queue ready status interrupt \ref group_i3c_macros_intr_macros
* for controller mode of operation.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void ControllerRespReadyStsHandle(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    uint32_t respCmdPort;
    uint8_t tid;


    respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);
    tid = (uint8_t)(_FLD2VAL(I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_TID, respCmdPort));


    /* Handling the receive response for Controller SDR WRITE transfer */
    /* --------------------SDR/HDR WRITE TRANSFER----------------*/
    if((((uint8_t)CY_I3C_CONTROLLER_SDR_WRITE_TID) == tid) || (((uint8_t)CY_I3C_CONTROLLER_HDR_WRITE_TID) == tid))
    {
        ControllerHandleWriteInterrupt(base, respCmdPort, context);
    }

    /* Handling the receive response for Controller SDR READ transfer */
    /* --------------------SDR/HDR READ TRANSFER----------------*/

    else if((((uint8_t)CY_I3C_CONTROLLER_SDR_READ_TID) == tid) || (((uint8_t)CY_I3C_CONTROLLER_HDR_READ_TID) == tid))
    {
        ControllerHandleReadInterrupt(base, respCmdPort, context);
    }

    else
    {
        /* Unknown TID
        Do Nothing */
    }
}


/*******************************************************************************
*  Function Name: ControllerHandleWriteInterrupt
****************************************************************************//**
*
* Handles the response queue ready status interrupt  \ref group_i3c_macros_intr_macros
* with HDR/SDR write TID \ref cy_en_i3c_tid_t in the response status.
*
* \param base
* The pointer to the I3C instance.
*
* \param respCmdPort
* The value read from the response queue port register.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static void ControllerHandleWriteInterrupt(I3C_CORE_Type *base, uint32_t respCmdPort, cy_stc_i3c_context_t *context)
{
    /* Expected number of bytes were not transmitted, WRITE terminated early */
    context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);
    context->state = CY_I3C_IDLE;

    uint32_t dataLen;

    dataLen = (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk);

    if(NULL != context->cbEvents)
    {
        (dataLen == 0UL) ? context->cbEvents(CY_I3C_CONTROLLER_WR_CMPLT_EVENT)  : context->cbEvents(CY_I3C_CONTROLLER_WR_EARLY_TERMINATION_EVENT);
    }
    CY_UNUSED_PARAMETER(base); /* Suppress a compiler warning about unused variables */
}

/*******************************************************************************
*  Function Name: ControllerHandleReadInterrupt
****************************************************************************//**
*
* Handles the response queue ready status interrupt \ref group_i3c_macros_intr_macros
* with HDR/SDR read TID \ref cy_en_i3c_tid_t in the response status.
*
* \param base
* The pointer to the I3C instance.
*
* \param respCmdPort
* The value read from the response queue port register.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static void ControllerHandleReadInterrupt(I3C_CORE_Type *base, uint32_t respCmdPort, cy_stc_i3c_context_t *context)
{
    if(0UL != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_DATA_LENGTH_Msk))
    {
        ControllerHandleDataReceive(base, context);
        context->state = CY_I3C_IDLE;
        context->controllerStatus &= (~CY_I3C_CONTROLLER_BUSY);

        if(NULL != context->cbEvents)
        {
            context->cbEvents(CY_I3C_CONTROLLER_RD_CMPLT_EVENT);
        }
    }
}

/*******************************************************************************
*  Function Name: ControllerHandleIBIInterrupt
****************************************************************************//**
*
* Handles the IBI interrupts \ref group_i3c_macros_intr_macros.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void ControllerHandleIBIInterrupt(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    (void)base;
    (void)context;

        uint32_t ibiData;
        uint8_t ibiType, ibiStatus;
        cy_stc_i3c_ibi_t ibiCallback;

        ibiData = I3C_CORE_IBI_QUEUE_STATUS(base);

        ibiType =  (uint8_t)(_FLD2VAL(I3C_CORE_IBI_QUEUE_STATUS_IBI_ID, ibiData));
        ibiStatus = (uint8_t)(_FLD2VAL(I3C_CORE_IBI_QUEUE_STATUS_IBI_RESP_STS, ibiData));

        /* Hot-join */
        if(false != (CY_I3C_IBI_TYPE_HOT_JOIN(ibiType)))
        {
            ibiCallback.event = CY_I3C_IBI_HOTJOIN;
            ibiCallback.targetAddress = 0U;
            ibiCallback.status = (uint32_t)((0U == ibiStatus) ? CY_I3C_CONTROLLER_HOTJOIN_IBI_ACK: CY_I3C_CONTROLLER_IBI_NACK);

            if(NULL != context->cbIbi)
            {
                context->cbIbi(&ibiCallback);
            }
        }

        /* Controllership request */
        else if(false != (CY_I3C_IBI_TYPE_CONTROLLERSHIP_REQUEST(ibiType)))
        {
            ibiCallback.event = CY_I3C_IBI_CONTROLLER_REQ;
            ibiCallback.targetAddress = (uint8_t)(_FLD2VAL(I3C_CORE_IBI_QUEUE_STATUS_IBI_ID, ibiData) & 0x7FUL); //Check the msb/lsb alignment
            ibiCallback.status = (uint32_t)((0U == ibiStatus) ? CY_I3C_CONTROLLER_MR_IBI_ACK : CY_I3C_CONTROLLER_IBI_NACK);

            if(NULL != context->cbIbi)
            {
                context->cbIbi(&ibiCallback);
            }
        }

        /* Target Interrupt Request */
        else if(false != (CY_I3C_IBI_TYPE_TIR_REQUEST(ibiType)))
        {
            ibiCallback.event = CY_I3C_IBI_SIR;
            ibiCallback.targetAddress = (uint8_t)(_FLD2VAL(I3C_CORE_IBI_QUEUE_STATUS_IBI_ID, ibiData) & 0x7FUL);
            ibiCallback.status = (uint32_t)((0U == ibiStatus) ? CY_I3C_CONTROLLER_SIR_IBI_ACK : CY_I3C_CONTROLLER_IBI_NACK);

            if(NULL != context->cbIbi)
            {
                context->cbIbi(&ibiCallback);
            }
        }

        else
        {
            /* Do Nothing */
        }
}


/*******************************************************************************
*  Function Name: RearrangeAddrTable
****************************************************************************//**
*
* Rearranges the DAT and local list of devices /ref cy_stc_i3c_controller_devlist_t
* when a device detaches from the bus and thus maintaining continuous free
* locations on the top of the DAT and local list of devices.
*
* \param base
* The pointer to the I3C instance.
*
* \param devIndex
* The position of the device being detached.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static void RearrangeAddrTable(I3C_CORE_Type *base, uint8_t devIndex,  cy_stc_i3c_context_t *context)
{
    cy_stc_i3c_device_t i3cDeviceTop;
    cy_stc_i2c_device_t i2cDeviceTop;
    uint32_t topDevPos, value;

    /* Case where the device being detached is not the top device on the list */
    topDevPos = (context->i3cController.devCount) - 1UL;

        if(devIndex != topDevPos)
        {
        /*
            1. Check if the top device is an i2c device or i3c device
            2. Update the DAT and the local list
            3. Also check for the case if the device to be detached is the top most device? Then do not perform the exchange
        */
            if(context->devList[topDevPos].i2c)
            {
                /* Top device is an i2c device */

                /* Moving the top device to the detached device position */
                i2cDeviceTop = context->devList[topDevPos].i2cDevice;
                context->devList[devIndex].i2cDevice=i2cDeviceTop;
                context->devList[devIndex].i2c = true;

                /* Erase the top position entries and mark the position free */
                context->i3cController.freePos |= CY_I3C_BIT(topDevPos);

                value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_STATIC_ADDR, i2cDeviceTop.staticAddress) |
                        I3C_CORE_DEV_ADDR_TABLE_LOC1_LEGACY_I2C_DEVICE_Msk;
            }

            else
            {
                /* Top Device is an i3c device */

                /* Moving the top device to the detached device position */
                i3cDeviceTop = context->devList[topDevPos].i3cDevice;
                context->devList[devIndex].i3cDevice = i3cDeviceTop;
                context->devList[devIndex].i2c = false;

                /* Erase the top position entries and mark the position free */
                context->i3cController.freePos |= CY_I3C_BIT(topDevPos);

                value = _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR, i3cDeviceTop.dynamicAddress) |
                        _VAL2FLD(I3C_CORE_DEV_ADDR_TABLE_LOC1_DEV_DYNAMIC_ADDR, i3cDeviceTop.staticAddress);
            }

        Cy_I3C_WriteIntoDeviceAddressTable(base, (uint8_t)devIndex, value);
        Cy_I3C_WriteIntoDeviceAddressTable(base, (uint8_t)topDevPos, 0UL);
        }

        /* Case where the device getting detached is the top device on the list and thus no rearrangement is not required */
        else
        {
            Cy_I3C_WriteIntoDeviceAddressTable(base, devIndex, 0UL);
            context->i3cController.freePos |= CY_I3C_BIT(devIndex);
        }

}


/*******************************************************************************
*  Function Name: DeviceIBIControl
****************************************************************************//**
*
* Handles enabling and disabling of IBI events from specified devices.
*
* \param base
* The pointer to the I3C instance.
*
* \param i3cDevice
* The pointer to the i3c device description structure \ref cy_stc_i3c_device_t.
*
* \param cccCmd
* The CCC command to be sent ENEC/DISEC.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t DeviceIBIControl(I3C_CORE_Type *base, cy_stc_i3c_device_t *i3cDevice, uint8_t cccCmd, cy_stc_i3c_context_t *context)
{
    uint32_t sirmap, bitpos, mrmap;
    cy_en_i3c_status_t retStatus;
    cy_stc_i3c_ccc_cmd_t cmd;
    cy_stc_i3c_ccc_events_t i3cCccENEC;
    cy_stc_i3c_ccc_payload_t payload;

    /* Sending DISEC command */
    i3cCccENEC.events = (uint8_t)(CY_I3C_CCC_EVENT_SIR | CY_I3C_CCC_EVENT_MR);
    cmd.cmd = cccCmd;
    cmd.data = &payload;
    cmd.data->len = ((uint16_t)sizeof(i3cCccENEC));
    cmd.data->data = &(i3cCccENEC.events);
    cmd.address = i3cDevice->dynamicAddress;

    retStatus = enec_disec_ccc(base, &cmd, context); //Should the response be checked for any errors?

    /* In case of successful CCC command */
    if(retStatus == CY_I3C_SUCCESS)
    {
        sirmap = I3C_CORE_IBI_TIR_REQ_REJECT(base);
        bitpos = CY_I3C_IBI_TIR_REQ_ID(i3cDevice->dynamicAddress);
        sirmap |= ((1UL) << bitpos); /* Setting the corresponding bit to 1: 1 -> Nack the SIR from the corresponding device */
        I3C_CORE_IBI_TIR_REQ_REJECT(base) = sirmap;

        mrmap = I3C_CORE_IBI_CR_REQ_REJECT(base);
        mrmap |= ((1UL) << bitpos); /* Setting the corresponding bit to 1: 1 -> Nack the MR from the corresponding device */
        I3C_CORE_IBI_CR_REQ_REJECT(base) = mrmap;
    }

    return retStatus;
}


/*******************************************************************************
*  Function Name: CCCTargetAddressValidation
****************************************************************************//**
*
* Validates whether the target address is an I3C device address on the bus/
* broadcast address.
*
* \param address
* The address to be validated.
*
* \param uincastOnly
* true: address is checked for I3C device address on the bus only.
* false: otherwise.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t CCCTargetAddressValidation(uint8_t address, bool unicastOnly, cy_stc_i3c_context_t *context)
{
    cy_en_i3c_addr_slot_status_t res;

    res = GetAaddrslotStatus(address, context);

    if(unicastOnly)
    {
        if(CY_I3C_ADDR_SLOT_I3C_DEV != res)
        {
            return CY_I3C_BAD_PARAM;
        }

        context->controllerStatus |=  CY_I3C_CONTROLLER_DIRECTED_CCC_WR_XFER;

        return CY_I3C_SUCCESS;
    }

    else
    {
        /* This helps to check if the device is actively present on the bus */
        if((CY_I3C_ADDR_SLOT_I3C_DEV != res ) && (CY_I3C_ADDR_SLOT_RSVD != res))
        {
            return CY_I3C_BAD_PARAM;
        }

        if(CY_I3C_BROADCAST_ADDR == address)
        {
            context->controllerStatus |= CY_I3C_CONTROLLER_BROADCAST_CCC_WR_XFER;
        }

        else
        {
            context->controllerStatus |=  CY_I3C_CONTROLLER_DIRECTED_CCC_WR_XFER;
        }

        return CY_I3C_SUCCESS;
    }
}


/*******************************************************************************
*  Function Name: ControllerHandleCCCResponse
****************************************************************************//**
*
* Handles the response of CCCs sent.
*
* \param base
* The pointer to the I3C instance.
*
* \param resp
* The pointer to the response queue port register value.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
* \return
* \ref cy_en_i3c_status_t
*
*******************************************************************************/
static cy_en_i3c_status_t ControllerHandleCCCResponse(I3C_CORE_Type *base, uint32_t *resp, cy_stc_i3c_context_t *context)
{
    uint32_t respCmdPort;
    cy_en_i3c_status_t retStatus = CY_I3C_SUCCESS;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

    /* Handling the response */
    do
    {
        retStatus = I3C_Check_Timeout(&timeout);
        /* wait until the response ready interrupt is received */
    }while(retStatus == CY_I3C_SUCCESS && 0UL == ((CY_I3C_INTR_RESP_READY_STS | CY_I3C_INTR_TRANSFER_ERR_STS) & Cy_I3C_GetInterruptStatus(base)));

    if(retStatus == CY_I3C_TIMEOUT){
        return retStatus;
    }

    respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);
    Cy_I3C_ClearInterrupt(base, CY_I3C_INTR_MASK);

    if(0UL != (respCmdPort & I3C_CORE_CONTROLLER_RESPONSE_QUEUE_PORT_ERR_STS_Msk))
    {
        retStatus = ResponseError(respCmdPort);    /* unsuccessful due to transfer error */
        context->controllerStatus |= CY_I3C_CONTROLLER_HALT_STATE;
    }

    Cy_I3C_SetInterruptMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
    Cy_I3C_SetInterruptStatusMask(base, CY_I3C_INTR_IBI_BUFFER_THLD_STS);
    CY_UNUSED_PARAMETER(resp); /* Suppress a compiler warning about unused variables */

    return retStatus;
}

/*******************************************************************************
*  Function Name: TargetRespReadyStsHandle
****************************************************************************//**
*
* Handles the response queue ready status interrupt \ref group_i3c_macros_intr_macros
* for target mode of operation.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void TargetRespReadyStsHandle(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    uint32_t respCmdPort;
    uint32_t event;
    cy_en_i3c_status_t retStatus = CY_I3C_CONTROLLER_NOT_READY;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

    respCmdPort = I3C_CORE_RESPONSE_QUEUE_PORT(base);

    if(0UL != (respCmdPort & I3C_CORE_TARGET_RESPONSE_QUEUE_PORT_ERR_STS_Msk))
    {
        uint32_t errorEvent;

        errorEvent = ResponseErrorEvent(respCmdPort);
        if(NULL != context->cbEvents)
        {
            context->cbEvents(errorEvent);
        }
        return;
    }
    if(0UL != (respCmdPort & I3C_CORE_TARGET_RESPONSE_QUEUE_PORT_RX_RSP_Msk))
    {
        context->targetRxBufferCnt = _FLD2VAL(I3C_CORE_TARGET_RESPONSE_QUEUE_PORT_DATA_LENGTH, respCmdPort);

        if(CY_I3C_CCC_DEFTGTS != _FLD2VAL(I3C_CORE_TARGET_RESPONSE_QUEUE_PORT_CCC_HDR_HEADER, respCmdPort))
        {
            /* Receive Response - controller write */
            TargetHandleDataReceive(base, context);

            I3C_CORE_RESET_CTRL(base) |= I3C_CORE_RESET_CTRL_RX_FIFO_RST_Msk;
            do
            {
                /* wait till the reset is completed */
                retStatus = I3C_Check_Timeout(&timeout);
            }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

            context->targetStatus = CY_I3C_TARGET_RD_CMPLT;
            event = CY_I3C_TARGET_RD_CMPLT_EVENT;

            context->state = CY_I3C_IDLE;
            if(NULL != context->cbEvents)
            {
                context->cbEvents(event);
            }
        }
    }

    else
    {
        /* Transmit Response - controller read */
        uint32_t size;

        /* Gets the number of entries in the Tx Fifo */
        size = (CY_I3C_FIFO_SIZE/4UL) - Cy_I3C_GetFreeEntriesInTxFifo(base);

        context->targetTxBufferCnt = (context->targetTxBufferIdx) - size;
        context->targetStatus = CY_I3C_TARGET_WR_CMPLT;
        event = CY_I3C_TARGET_WR_CMPLT_EVENT;

        context->state = CY_I3C_IDLE;
        if(NULL != context->cbEvents)
        {
            context->cbEvents(event);
        }
    }
}


/*******************************************************************************
* Function Name: TargetHandleDataReceive
****************************************************************************//**
*
* Reads data from RX FIFO into the buffer provided by
* \ref Cy_I3C_TargetConfigWriteBuf.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void TargetHandleDataReceive(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    if(context->targetRxBufferSize > 0UL)
    {
        context->state = CY_I3C_TARGET_RX;
        context->targetStatus = CY_I3C_TARGET_RD_BUSY;
        uint32_t numToCopy;

        numToCopy =  ((CY_I3C_FIFO_SIZE / 4UL) - Cy_I3C_GetFreeEntriesInRxFifo(base));

        ReadArray(base, context->targetRxBuffer, numToCopy * 4UL);
        context->targetRxBufferIdx += numToCopy;
        context->targetRxBufferSize -= numToCopy;

        context->targetRxBuffer = &(context->targetRxBuffer[context->targetRxBufferIdx]);
    }
}


/*******************************************************************************
* Function Name: TargetHandleDataTransmit
****************************************************************************//**
*
* Loads TX FIFO with data provided by \ref Cy_I3C_TargetConfigReadBuf.
*
* \param base
* The pointer to the I3C instance.
*
* \param context
* The pointer to the context structure \ref cy_stc_i3c_context_t allocated
* by the user. The structure is used during the I3C operation for internal
* configuration and data retention. The user must not modify anything
* in this structure.
*
*******************************************************************************/
static void TargetHandleDataTransmit(I3C_CORE_Type *base, cy_stc_i3c_context_t *context)
{
    if(0UL != context->targetTxBufferSize)
    {
        context->state = CY_I3C_TARGET_TX;
        context->targetStatus = CY_I3C_TARGET_WR_BUSY;

        uint32_t numToCopy;

        numToCopy = Cy_I3C_GetFreeEntriesInTxFifo(base) * 4UL;

        if(numToCopy > (context->targetTxBufferSize))
        {
            numToCopy = context->targetTxBufferSize;
        }
        WriteArray(base, context->targetTxBuffer, numToCopy);
        context->targetTxBufferIdx += numToCopy;
        context->targetTxBufferSize -= numToCopy;
        context->targetTxBuffer = &context->targetTxBuffer[context->targetTxBufferIdx];
    }
}


/******************************************************************************
* Function Name: I3C_Check_Timeout
****************************************************************************//**
*
* Waits for one micro second before unblock code execution.
* Note that if a timeout value is 0, this function does nothing and returns 0.
*
* \param timeout
* The pointer to a timeout value.
*
* \return
* Returns CY_I3C_SUCCESS if a timeout does not expire or the CY_I3C_TIMEOUT if time out is reached.
*
*******************************************************************************/
static cy_en_i3c_status_t I3C_Check_Timeout(uint32_t *timeout)
{
    cy_en_i3c_status_t status = CY_I3C_SUCCESS;

    /* If the timeout equal to 0. Ignore the timeout */
    if (*timeout > 0UL)
    {
        Cy_SysLib_DelayUs(1U);
        --(*timeout);

        if (0UL == *timeout)
        {
            status = CY_I3C_TIMEOUT;
        }
    }

    return (status);
}

/*******************************************************************************
* Function Name: Cy_I3C_SoftReset
****************************************************************************//**
*
* Soft reset for I3C. It resets all buffers: Transmit, Receive, command, Response.
*
* \param base
* The pointer to the I3C instance.
*
* \return
* reset successful status - CY_I3C_TIMEOUT / CY_I3C_SUCCESS.
*
*******************************************************************************/
cy_en_i3c_status_t Cy_I3C_SoftReset(I3C_CORE_Type const *base)
{
    cy_en_i3c_status_t retStatus = CY_I3C_SUCCESS;
    uint32_t timeout = MAX_I3C_TRANSACTION_TIMEOUT;

    I3C_CORE_RESET_CTRL(base) |=  I3C_CORE_RESET_CTRL_SOFT_RST_Msk;
    do
    {
        /* wait till the reset is completed */
        retStatus = I3C_Check_Timeout(&timeout);
    }while(retStatus == CY_I3C_SUCCESS && I3C_CORE_RESET_CTRL(base) != 0U);

    return retStatus;
}

CY_MISRA_BLOCK_END('MISRA C-2012 Rule 14.3')

#if defined(__cplusplus)
    }
#endif

#endif /* CY_IP_MXI3C */
/* [] END OF FILE */
