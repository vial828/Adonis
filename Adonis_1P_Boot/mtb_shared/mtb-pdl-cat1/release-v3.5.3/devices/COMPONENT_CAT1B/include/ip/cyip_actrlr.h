/***************************************************************************//**
* \file cyip_actrlr.h
*
* \brief
* ACTRLR IP definitions
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
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

#ifndef _CYIP_ACTRLR_H_
#define _CYIP_ACTRLR_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    ACTRLR
*******************************************************************************/

#define ACTRLR_TTCFG_SECTION_SIZE               0x00000010UL
#define ACTRLR_SECTION_SIZE                     0x00010000UL

/**
  * \brief Timer Table Structure (ACTRLR_TTCFG)
  */
typedef struct {
  __IOM uint32_t TT_CFG0;                       /*!< 0x00000000 AC Configuration 0 */
  __IOM uint32_t TT_CFG1;                       /*!< 0x00000004 AC Configuration 1 */
  __IOM uint32_t TT_CFG2;                       /*!< 0x00000008 AC Configuration 2 */
  __IOM uint32_t TT_CFG3;                       /*!< 0x0000000C AC Configuration 3 */
} ACTRLR_TTCFG_Type;                            /*!< Size = 16 (0x10) */

/**
  * \brief Autonomnous Controller (ACTRLR)
  */
typedef struct {
  __IOM uint32_t CTRL;                          /*!< 0x00000000 Control register */
  __IOM uint32_t CMD_RUN;                       /*!< 0x00000004 Run Command register */
  __IOM uint32_t CMD_STATE;                     /*!< 0x00000008 State Override register */
   __IM uint32_t RESERVED;
   __IM uint32_t BLOCK_STATUS;                  /*!< 0x00000010 Block Ready status register */
   __IM uint32_t RESERVED1[3];
   __IM uint32_t STATUS;                        /*!< 0x00000020 AC Status register */
  __IOM uint32_t CFG;                           /*!< 0x00000024 AC Config register */
   __IM uint32_t RESERVED2[54];
   __IM uint32_t CNTR_STATUS[2];                /*!< 0x00000100 Loop/Interval counter status register */
   __IM uint32_t RESERVED3[446];
        ACTRLR_TTCFG_Type TTCFG[16];            /*!< 0x00000800 Timer Table Structure */
} ACTRLR_Type;                                  /*!< Size = 2304 (0x900) */


/* ACTRLR_TTCFG.TT_CFG0 */
#define ACTRLR_TTCFG_TT_CFG0_SAR_UNLOCK_Pos     4UL
#define ACTRLR_TTCFG_TT_CFG0_SAR_UNLOCK_Msk     0x10UL
#define ACTRLR_TTCFG_TT_CFG0_CSG_UNLOCK_Pos     8UL
#define ACTRLR_TTCFG_TT_CFG0_CSG_UNLOCK_Msk     0xFF00UL
#define ACTRLR_TTCFG_TT_CFG0_DOUT_UNLOCK_Pos    16UL
#define ACTRLR_TTCFG_TT_CFG0_DOUT_UNLOCK_Msk    0x10000UL
#define ACTRLR_TTCFG_TT_CFG0_DOUT_Pos           20UL
#define ACTRLR_TTCFG_TT_CFG0_DOUT_Msk           0x1F00000UL
/* ACTRLR_TTCFG.TT_CFG1 */
#define ACTRLR_TTCFG_TT_CFG1_BR_ADDR_Pos        0UL
#define ACTRLR_TTCFG_TT_CFG1_BR_ADDR_Msk        0xFUL
#define ACTRLR_TTCFG_TT_CFG1_RESERVED_0_Pos     6UL
#define ACTRLR_TTCFG_TT_CFG1_RESERVED_0_Msk     0xC0UL
#define ACTRLR_TTCFG_TT_CFG1_COND_Pos           8UL
#define ACTRLR_TTCFG_TT_CFG1_COND_Msk           0x3F00UL
#define ACTRLR_TTCFG_TT_CFG1_RESERVED_NC_Pos    14UL
#define ACTRLR_TTCFG_TT_CFG1_RESERVED_NC_Msk    0xC000UL
#define ACTRLR_TTCFG_TT_CFG1_ACTION_Pos         16UL
#define ACTRLR_TTCFG_TT_CFG1_ACTION_Msk         0x70000UL
#define ACTRLR_TTCFG_TT_CFG1_INTR_SET_Pos       19UL
#define ACTRLR_TTCFG_TT_CFG1_INTR_SET_Msk       0x80000UL
#define ACTRLR_TTCFG_TT_CFG1_CNT_Pos            20UL
#define ACTRLR_TTCFG_TT_CFG1_CNT_Msk            0xFFF00000UL
/* ACTRLR_TTCFG.TT_CFG2 */
#define ACTRLR_TTCFG_TT_CFG2_CSG_EN_Pos         0UL
#define ACTRLR_TTCFG_TT_CFG2_CSG_EN_Msk         0xFFUL
#define ACTRLR_TTCFG_TT_CFG2_CSG_DAC_TR_Pos     8UL
#define ACTRLR_TTCFG_TT_CFG2_CSG_DAC_TR_Msk     0xFF00UL
/* ACTRLR_TTCFG.TT_CFG3 */
#define ACTRLR_TTCFG_TT_CFG3_SAR_TR_Pos         0UL
#define ACTRLR_TTCFG_TT_CFG3_SAR_TR_Msk         0xFFUL
#define ACTRLR_TTCFG_TT_CFG3_SAR_EN_Pos         8UL
#define ACTRLR_TTCFG_TT_CFG3_SAR_EN_Msk         0x100UL
#define ACTRLR_TTCFG_TT_CFG3_SAR_AROUTE_TR_Pos  12UL
#define ACTRLR_TTCFG_TT_CFG3_SAR_AROUTE_TR_Msk  0xF000UL
#define ACTRLR_TTCFG_TT_CFG3_SAR_AROUTE_SEL_Pos 16UL
#define ACTRLR_TTCFG_TT_CFG3_SAR_AROUTE_SEL_Msk 0xFF0000UL


/* ACTRLR.CTRL */
#define ACTRLR_CTRL_ENABLED_Pos                 31UL
#define ACTRLR_CTRL_ENABLED_Msk                 0x80000000UL
/* ACTRLR.CMD_RUN */
#define ACTRLR_CMD_RUN_RUN_Pos                  0UL
#define ACTRLR_CMD_RUN_RUN_Msk                  0x1UL
/* ACTRLR.CMD_STATE */
#define ACTRLR_CMD_STATE_STATE_Pos              0UL
#define ACTRLR_CMD_STATE_STATE_Msk              0xFUL
/* ACTRLR.BLOCK_STATUS */
#define ACTRLR_BLOCK_STATUS_READY_Pos           0UL
#define ACTRLR_BLOCK_STATUS_READY_Msk           0x1UL
/* ACTRLR.STATUS */
#define ACTRLR_STATUS_CUR_STATE_Pos             0UL
#define ACTRLR_STATUS_CUR_STATE_Msk             0xFUL
#define ACTRLR_STATUS_STATUS_Pos                8UL
#define ACTRLR_STATUS_STATUS_Msk                0x300UL
/* ACTRLR.CFG */
#define ACTRLR_CFG_DOUT_EN_Pos                  0UL
#define ACTRLR_CFG_DOUT_EN_Msk                  0x1FUL
/* ACTRLR.CNTR_STATUS */
#define ACTRLR_CNTR_STATUS_CUR_STATE_Pos        0UL
#define ACTRLR_CNTR_STATUS_CUR_STATE_Msk        0xFUL
#define ACTRLR_CNTR_STATUS_CUR_CNT_Pos          8UL
#define ACTRLR_CNTR_STATUS_CUR_CNT_Msk          0xFFF00UL
#define ACTRLR_CNTR_STATUS_BUSY_Pos             31UL
#define ACTRLR_CNTR_STATUS_BUSY_Msk             0x80000000UL


#endif /* _CYIP_ACTRLR_H_ */


/* [] END OF FILE */
