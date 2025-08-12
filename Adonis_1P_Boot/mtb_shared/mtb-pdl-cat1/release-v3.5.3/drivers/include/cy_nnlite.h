/***************************************************************************//**
* \file cy_nnlite.h
* \version 1.0
*
* \brief
* Provides NNLite accelerator pdl API declarations.
*
********************************************************************************
* \copyright
* Copyright 2016-2022 Cypress Semiconductor Corporation
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
*
*
* \section group_nnlite_changelog Changelog
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>

*******************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#ifndef CY_NNLITE_PDL_H
#define CY_NNLITE_PDL_H

#include "cy_device.h"
#if defined (CY_IP_MXNNLITE)

/**
* \defgroup group_nnlite             NNLITE HW Accelerator (NNLITE)
* \{
* This driver provides NNLite accelerator pdl defines and API function definitions.
*
* \defgroup group_nnlite_macros Macro
* \defgroup group_nnlite_functions Functions
* \defgroup group_nnlite_data_structures Data structures
* \defgroup group_nnlite_enums Enumerated types
*/

#if defined(__cplusplus)
extern "C" {
#endif
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cy_syslib.h"


/** \addtogroup group_nnlite_macros
* \{
*/

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/

/** NNLite driver ID */
#define CY_NNLITE_ID                              CY_PDL_DRV_ID(0x48U)

/** NNLite type */
#define NNLITE_Type                               MXNNLITE_1_0_Type

/** Output value Clipping mask */
#define NNLITE_BYTE_CLIPING                       (0xFFU)

/** Output Scaling 1 mask */
#define NNLITE_NO_SCALING                         (0x3F800000U)

/** Error Interrupt */
#define NNLITE_INTR_ERRORS_MASK                      (MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_MEM_ERR_SPARSITY_Msk | \
                                                 MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_MEM_ERR_ACTIVATIONSTREAMER_Msk | \
                                                 MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_MEM_ERR_WEIGHTSTREAMER_Msk | \
                                                 MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_MEM_ERR_BIASSTREAMER_Msk | \
                                                 MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_MEM_ERR_OUTPUTSTREAMER_Msk)

/** Interrupt Enable mask */
#define NNLITE_INTR_ENABLE_MASK                 (MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_DONE_Msk | \
                                                 MXNNLITE_1_0_INTR_MASKED_INTR_MASKED_SATURATION_Msk | \
                                                 NNLITE_INTR_ERRORS_MASK)

/** \} group_nnlite_macros */


/** \addtogroup group_nnlite_enums
* \{
*/

/** NNLite pdl status codes */
typedef enum
{
    CY_NNLITE_SUCCESS = 0U, /**< Status successful  */

    CY_NNLITE_OP_QUEUED = 1U, /** Operation is Pending state */

    CY_NNLITE_MEM_ERR_SPARSITY = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR |
                                            MXNNLITE_1_0_INTR_INTR_MEM_ERR_SPARSITY_Msk), /**< Mem fetch Error for Sparsity */

    CY_NNLITE_MEM_ERR_ACTIVATION_STREAMER = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR |
                                            MXNNLITE_1_0_INTR_INTR_MEM_ERR_ACTIVATIONSTREAMER_Msk),  /**< Mem fetch Error for Activation pointer */

    CY_NNLITE_MEM_ERR_WEIGHT_STREAMER = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR |
                                            MXNNLITE_1_0_INTR_INTR_MEM_ERR_WEIGHTSTREAMER_Msk), /**< Mem fetch Error for Weight pointer */

    CY_NNLITE_MEM_ERR_BIAS_STREAMER = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR |
                                            MXNNLITE_1_0_INTR_INTR_MEM_ERR_BIASSTREAMER_Msk), /**< Mem fetch Error for Bias pointer */

    CY_NNLITE_MEM_ERR_OUTPUT_STREAMER = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR |
                                            MXNNLITE_1_0_INTR_INTR_MEM_ERR_OUTPUTSTREAMER_Msk),/**< Mem Error for Output pointer */

    CY_NNLITE_SATURATION = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR |
                                            MXNNLITE_1_0_INTR_INTR_SATURATION_Msk), /**< Saturation in output */

    CY_NNLITE_BAD_PARAM = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR | 0x41U),  /**< function call with invalid parameter */

    CY_NNLITE_BAD_STATE = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR | 0x42U), /**< Invalid state, order */

    CY_NNLITE_EXTERN_API_ERR = (CY_NNLITE_ID | CY_PDL_STATUS_ERROR | 0x43U), /**< Error code for system (RTOS) level API failure */

} cy_en_nnlite_status_t;

/** NNLite nn layer type*/
typedef enum
{
  CY_NNLITE_FC_LAYER = 0U, /**< NN FC or Dense Layer */
  CY_NNLITE_CONV_LAYER = 1U, /**< NN Convolution Layer */
} cy_en_nnlite_layer_t;

/** Out Activation type */
typedef enum
{
  CY_NNLITE_ACTIVATION_NONE = 0U, /**< Activation type none */
  CY_NNLITE_ACTIVATION_RELU = 1U, /**< Activation type RELU */
  CY_NNLITE_ACTIVATION_LEAKY_RELU = 2U, /**< Activation type leaky RELU */
} cy_en_nnlite_fused_activation_t;

/** Activation size */
typedef enum
{
  CY_NNLITE_ACTIVATION_8BIT = 0U, /**< 8 bits Activation  */
  CY_NNLITE_ACTIVATION_16BIT = 1U, /**< 16 bits Activation */
} cy_en_nnlite_activation_size_t;

/** Streamer id's */
typedef enum
{
  CY_NNLITE_ACTIVATION_STREAMER = 0U, /**< Activation Streamer ID */
  CY_NNLITE_WEIGHT_STREAMER = 1U, /**< Weight Streamer ID */
  CY_NNLITE_BIAS_STREAMER = 2U, /**< Bias Streamer ID */
  CY_NNLITE_OUT_STREAMER = 3U, /**< Output Streamer ID */
} cy_en_nnlite_streamer_id_t;

/** NNLite PDL Driver State Machine*/
typedef enum
{
  CY_NNLITE_DEINIT = 0U, /**< Deinitialized State */
  CY_NNLITE_INIT = 1U, /**< Init State */
  CY_NNLITE_CONFIG_STATE = 2U, /**< Configuration State */
  CY_NNLITE_OP_STARTED = 3U, /**< NN layer Operation Started */
  CY_NNLITE_OP_DONE = 4U, /**< Last NN operation completed */
} cy_en_nnlite_state_t;

/** \} group_nnlite_enums */

/**
* \addtogroup group_nnlite_data_structures
* \{
*/

/**
 *****************************************************************************
 ** \brief nnlite context structure
 *****************************************************************************/
typedef struct cy_nnlite_context
{
  volatile cy_en_nnlite_state_t nnliteState; /**< Current state of nnlite driver */
  volatile cy_en_nnlite_status_t opStatus; /**< Status of last operation */
} cy_nnlite_context_t;

/**
 *****************************************************************************
 ** \brief nnlite sparsity configuration
 *****************************************************************************/
typedef struct cy_nnlite_sparsity_cfg
{
    uint32_t nonZeroWtAddr; /**< non zero weight address */
    uint32_t sparsityBitMapAddr; /**< sparsity bit map address */
    uint32_t wtAddr; /**< weight address */
} cy_nnlite_sparsity_cfg_t;

 /** \} group_nnlite_data_structures */

/*******************************************************************************
*                           Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_nnlite_functions
* \{
*/

/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/


/**
 *****************************************************************************
 ** \brief  nnlite driver init function, validate context structure and
 ** set driver to init state
 **
 ** \param [in]  nnlite     base pointer of NNLite Register  Map.
 **
 ** \param [in]  context    pointer to the driver context structure.
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_Init(NNLITE_Type *nnlite, cy_nnlite_context_t *context);

/**
 *****************************************************************************
 ** \brief  nnlite driver deinit function check's for pending or ongoing
 ** operation and set driver state to deinit
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure.
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_DeInit(NNLITE_Type *nnlite,
                                        cy_nnlite_context_t *context);

/**
 *****************************************************************************
 ** \brief nnlite start operation, streamer should should be configured before
 ** calling this function, this function will write to start bit in CMD MEMIO
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure.
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_Start(NNLITE_Type *nnlite,
                                      cy_nnlite_context_t *context);

/**
 *****************************************************************************
 ** \brief  nnlite stop/abort operation, can be used to stop/abort current
 ** operation or to reset all configuration, API write abort in CMD MEMIO
 ** which will reset all the registers
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure.
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_Stop(NNLITE_Type *nnlite,
                                      cy_nnlite_context_t *context);

/**
 *****************************************************************************
 **  Compute size of sparsity map in bytes for given number of number of weights.
 **
 **
 **  \param [in]  num_weights    Number of weights.
 ** \retval Size in bytes.
 *****************************************************************************/
static inline uint32_t Cy_NNLite_SparsityMapSize(uint32_t num_weights) 
{
  /* Start is byte aligned */
  return (num_weights + 7u ) / 8u;
}

/**
 *****************************************************************************
 **  parse sparsity map, API will  parse sparsity bit map and update non
 **  zero weight pointer, sparsity bit map address and weight pointer in
 **  sparCfg, sparsity configuration structure
 **
 **  \param [in]  nnlite base pointer of register map.
 **
 **  \param [in]  sparsitybaseAddr sparsity map base address
 **
 **  \param [in]  activationRepeats  activation streamer repeat count
 **
 **  \param [in]  sparseBitMapLen sparsity bit map length
 **
 **  \param [out] sparCfg sparsity configuration
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
 cy_en_nnlite_status_t
 Cy_NNLite_ParseSparsity(NNLITE_Type *nnlite, const void *sparsitybaseAddr,
    uint32_t activationRepeats, uint32_t sparseBitMapLen, cy_nnlite_sparsity_cfg_t *sparCfg);
/**
 *****************************************************************************
 **  configure sparsity from valid cy_nnlite_sparsity_cfg
 **  write non zero wt pointer and sparsity bit map MEMIO and enable sparsity
 **  in streamer mode MEMIO
 **
 ** \param [in]  nnlite base pointer of register map.
 **
 ** \param [out]  sparCfg sparsity configuration
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_SparsityCfg(NNLITE_Type *nnlite, cy_nnlite_sparsity_cfg_t *sparCfg);


/**
 *****************************************************************************
 ** \brief nnlite enable sparsity in streamer mode MEMIO
 **
 ** \param [in]  nnlite base pointer of register map.
 **
 ** \param [in]  sparsityEn sparsity enable
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_SparsityEnable(NNLITE_Type *nnlite, bool sparsityEn);

/**
 *****************************************************************************
 ** \brief  nnlite activation streamer configuration set, API will configure
 ** parameters for activation streamer
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure
 **
 ** \param [in]  filterWidth  filter width
 **
 ** \param [in]  filterHeight  fFilter height
 **
 ** \param [in]  activationRepeats  activation streamer repeat count
 **
 ** \param [in]  inputWidth  input activation Width
 **
 ** \param [in]  inputHeight  input activation Height
 **
 ** \param [in]  inputChannel  input activation channels
 **
 ** \param [in]  startCol  starting column for activation
 **
 ** \param [in]  startRow  starting row for activation
 **
 ** \param [in]  padVal  padding value
 **
 ** \param [in]  padHeight  padding Height
 **
 ** \param [in]  padWidth  padding Width
 **
 ** \param [in]  strideCol  Stride column
 **
 ** \param [in]  strideRow  Stride rows
 **
 ** \param [in]  inputOffset input offset value
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_ActivationStreamerCfg(NNLITE_Type *nnlite, cy_nnlite_context_t *context,
                        uint32_t filterWidth, uint32_t filterHeight,
                        uint32_t activationRepeats, uint32_t inputWidth,
                        uint32_t inputHeight, uint32_t inputChannel,
                        uint32_t startCol, uint32_t startRow,
                        int8_t padVal, uint8_t padWidth,
                        uint8_t padHeight, uint8_t strideCol,
                        uint8_t strideRow, int32_t inputOffset);
/**
 *****************************************************************************
 ** \brief  nnlite weight streamer configuration set, API will configure offset
 ** for weight streamer and weights per neuron parameter*
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure
 **
 ** \param [in]  weightPerNeuron  weight/filter elements count per neuron
 **
 ** \param [in]  filterOffset  weight/filter elements offset value
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_WeightStreamerCfg(NNLITE_Type *nnlite, cy_nnlite_context_t *context,
                          uint32_t weightPerNeuron, int32_t filterOffset);
/**
 *****************************************************************************
 ** \brief  nnlite out streamer configuration set, API will set clipping mask,
 **  offset, scaling factor ,width and height for output streamer
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure
 **
 ** \param [in]  clippingMask  output clipping mask for max or min value
 **
 ** \param [in]  outputOffset  output elements offset value
 **
 ** \param [in]  outputWidth  output width
 **
 ** \param [in]  outputHeight  output height
 **
 ** \param [in]  outputScalingFactor  output scaling factor
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_OutputStreamerCfg(NNLITE_Type *nnlite, cy_nnlite_context_t *context,
                        uint32_t clippingMask, int32_t outputOffset,
                        uint32_t outputWidth, uint32_t outputHeight,
                        float outputScalingFactor);

/**
 *****************************************************************************
 ** \brief nnlite bias streamer enable
 **
 ** \param [in]  nnlite base pointer of register map.
 **
 ** \param [in]  biasEn bias streamer enable
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_BiasStreamerEnable(NNLITE_Type *nnlite, bool biasEn);

/**
 *****************************************************************************
 ** \brief  set nnlite streamer base address
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  strmId     streamer id
 **
 ** \param [in]  baseAddr   base address of buffer
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_StreamerBaseAddrSet(NNLITE_Type *nnlite,
                        cy_en_nnlite_streamer_id_t strmId, const void *baseAddr);

/**
 *****************************************************************************
 ** \brief  set activation type control
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  nnLayer    NN Layer operation type
 **
 ** \param [in]  actEn      Output Activation enable
 **
 ** \param [in]  actDataSize    Activation size
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_ActivationTypeCtrl(NNLITE_Type *nnlite,
                        cy_en_nnlite_layer_t nnLayer, bool actEn,
                        cy_en_nnlite_activation_size_t actDataSize);

/**
 *****************************************************************************
 ** \brief  nnlite set Interpolation lookup table to be used along
 **  with Cy_NNLite_SetinterpolationParam. For lut 0 and lut 1 both,
 **  Cy_NNLite_SetinterpolationParam need to be set.
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  lut        lookup table
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_SetInterpolationLUTAddr(NNLITE_Type *nnlite, bool lut);

/**
 *****************************************************************************
 ** \brief  set interpolation parameter
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  slope      slope value for interpolation
 **
 ** \param [in]  intercept  Y-intercept value for calculation of interpolation
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_SetInterpolationParam(NNLITE_Type *nnlite,
                                        uint16_t slope, uint16_t intercept);

/**
 *****************************************************************************
 ** \brief  nnlite set interrupt mask, available interrupt are operation STATUS Done,
 **   MEM Fetch Error different streamers, Output Saturation interrupt
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  intrMask    nnlite interrupt mask register
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_SetInterruptMask(NNLITE_Type *nnlite,
                                                    uint32_t intrMask);
/**
 *****************************************************************************
 ** \brief  nnlite set datawire trigger control, trigger datawire for next layer
 **  when trigger is 1
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  trigEn  datawire trigger mask
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_DatawireTriggerEnable(NNLITE_Type *nnlite,
                                                          bool trigEn);

/**
 *****************************************************************************
 ** \brief  nnlite get nnlite operation status, can be used to poll operation status
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [out]  opStatus    nnlite last operation status
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_GetOperationStatus(NNLITE_Type *nnlite, uint32_t *opStatus);

/**
 *****************************************************************************
 ** \brief  get nnlite interrupt status
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [out]  intrStatus    nnlite interrupt status register
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_GetInterruptStatus(NNLITE_Type *nnlite,
                                                      uint32_t *intrStatus);

/**
 *****************************************************************************
 ** \brief  get nnlite interrupt mask
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [out] intrMask   interrupt mask value
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_GetInterruptMask(NNLITE_Type *nnlite, uint32_t *intrMask);

/**
 *****************************************************************************
 ** \brief  get nnlite driver state
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    pointer to the driver context structure
 **
 ** \param [out] nnliteState   driver current state
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_GetDriverState(NNLITE_Type *nnlite,
                                     cy_nnlite_context_t *context,
                                     cy_en_nnlite_state_t *nnliteState);

/**
 *****************************************************************************
 ** \brief  clear nnlite interrupts
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  intrMask    nnlite interrupts to be cleared
 **
 ** \retval Refer \ref cy_en_nnlite_status_t
 **
 *****************************************************************************/
cy_en_nnlite_status_t Cy_NNLite_InterruptClear(NNLITE_Type *nnlite,
                                                uint32_t intrMask);

/**
 *****************************************************************************
 ** \brief  nnlite ISR handler
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    nnlite context structure pointer
 **
 *****************************************************************************/
void Cy_NNLite_InterruptHandler(NNLITE_Type *nnlite,
                                  cy_nnlite_context_t *context);

/**
 *****************************************************************************
 ** \brief  wait for completion of operation in busy loop
 **
 ** \param [in]  nnlite     base pointer of NNLIte register map.
 **
 ** \param [in]  context    nnlite context structure pointer
 **
 *****************************************************************************/
cy_en_nnlite_status_t
Cy_NNLite_WaitForCompletion(NNLITE_Type *nnlite, cy_nnlite_context_t *context);
/** \} group_nnlite_functions */
#ifdef __cplusplus
}
#endif
#endif /* CY_IP_MXNNLITE */

/** \} group_nnlite */
#endif /* CY_NNLITE_PDL_H */

/* [] END OF FILE */
