/***************************************************************************//**
* \file cyip_infra.h
*
* \brief
* INFRA IP definitions
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

#ifndef _CYIP_INFRA_H_
#define _CYIP_INFRA_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    INFRA
*******************************************************************************/

#define INFRA_AREFV2_SECTION_SIZE               0x00000100UL
#define INFRA_SECTION_SIZE                      0x00010000UL

/**
  * \brief AREF Configuration (INFRA_AREFV2)
  */
typedef struct {
  __IOM uint32_t AREF_CTRL;                     /*!< 0x00000000 global AREF control */
   __IM uint32_t RESERVED[63];
} INFRA_AREFV2_Type;                            /*!< Size = 256 (0x100) */

/**
  * \brief MCPASS infrastructure (INFRA)
  */
typedef struct {
  __IOM uint32_t TR_IN_SEL;                     /*!< 0x00000000 Trigger Input Select */
  __IOM uint32_t HW_TR_MODE;                    /*!< 0x00000004 HW Trigger Mode */
  __IOM uint32_t FW_TR_PULSE;                   /*!< 0x00000008 FW Pulse Mode Trigger */
  __IOM uint32_t FW_TR_LEVEL;                   /*!< 0x0000000C FW Level Mode Trigger */
   __IM uint32_t RESERVED[4];
  __IOM uint32_t CLOCK_STARTUP_DIV;             /*!< 0x00000020 Startup Clock Divder */
   __IM uint32_t RESERVED1[3];
  __IOM uint32_t STARTUP_CFG[4];                /*!< 0x00000030 Startup configuration */
  __IOM uint32_t DDFT;                          /*!< 0x00000040 DDFT control register */
   __IM uint32_t RESERVED2[3];
   __IM uint32_t VDDA_STATUS;                   /*!< 0x00000050 Analog Voltage Status */
   __IM uint32_t RESERVED3[3];
  __IOM uint32_t SAR_CHAR_IF;                   /*!< 0x00000060 SAR Characterization Interface */
   __IM uint32_t RESERVED4[871];
        INFRA_AREFV2_Type AREFV2;               /*!< 0x00000E00 AREF Configuration */
  __IOM uint32_t VREF_TRIM0;                    /*!< 0x00000F00 VREF Trim bits */
  __IOM uint32_t VREF_TRIM1;                    /*!< 0x00000F04 VREF Trim bits */
  __IOM uint32_t VREF_TRIM2;                    /*!< 0x00000F08 VREF Trim bits */
  __IOM uint32_t VREF_TRIM3;                    /*!< 0x00000F0C VREF Trim bits */
  __IOM uint32_t IZTAT_TRIM0;                   /*!< 0x00000F10 IZTAT Trim bits */
  __IOM uint32_t IZTAT_TRIM1;                   /*!< 0x00000F14 IZTAT Trim bits */
  __IOM uint32_t IPTAT_TRIM0;                   /*!< 0x00000F18 IPTAT Trim bits */
  __IOM uint32_t ICTAT_TRIM0;                   /*!< 0x00000F1C ICTAT Trim bits */
} INFRA_Type;                                   /*!< Size = 3872 (0xF20) */


/* INFRA_AREFV2.AREF_CTRL */
#define INFRA_AREFV2_AREF_CTRL_AREF_MODE_Pos    0UL
#define INFRA_AREFV2_AREF_CTRL_AREF_MODE_Msk    0x1UL
#define INFRA_AREFV2_AREF_CTRL_AREF_BIAS_SCALE_Pos 2UL
#define INFRA_AREFV2_AREF_CTRL_AREF_BIAS_SCALE_Msk 0xCUL
#define INFRA_AREFV2_AREF_CTRL_AREF_RMB_Pos     4UL
#define INFRA_AREFV2_AREF_CTRL_AREF_RMB_Msk     0x70UL
#define INFRA_AREFV2_AREF_CTRL_CTB_IPTAT_SCALE_Pos 7UL
#define INFRA_AREFV2_AREF_CTRL_CTB_IPTAT_SCALE_Msk 0x80UL
#define INFRA_AREFV2_AREF_CTRL_CTB_IPTAT_REDIRECT_Pos 8UL
#define INFRA_AREFV2_AREF_CTRL_CTB_IPTAT_REDIRECT_Msk 0xFF00UL
#define INFRA_AREFV2_AREF_CTRL_IZTAT_SEL_Pos    16UL
#define INFRA_AREFV2_AREF_CTRL_IZTAT_SEL_Msk    0x10000UL
#define INFRA_AREFV2_AREF_CTRL_VREF_SEL_Pos     20UL
#define INFRA_AREFV2_AREF_CTRL_VREF_SEL_Msk     0x300000UL
#define INFRA_AREFV2_AREF_CTRL_ENABLED_Pos      31UL
#define INFRA_AREFV2_AREF_CTRL_ENABLED_Msk      0x80000000UL


/* INFRA.TR_IN_SEL */
#define INFRA_TR_IN_SEL_TR0_SEL_Pos             0UL
#define INFRA_TR_IN_SEL_TR0_SEL_Msk             0x7UL
#define INFRA_TR_IN_SEL_TR1_SEL_Pos             4UL
#define INFRA_TR_IN_SEL_TR1_SEL_Msk             0x70UL
#define INFRA_TR_IN_SEL_TR2_SEL_Pos             8UL
#define INFRA_TR_IN_SEL_TR2_SEL_Msk             0x700UL
#define INFRA_TR_IN_SEL_TR3_SEL_Pos             12UL
#define INFRA_TR_IN_SEL_TR3_SEL_Msk             0x7000UL
#define INFRA_TR_IN_SEL_TR4_SEL_Pos             16UL
#define INFRA_TR_IN_SEL_TR4_SEL_Msk             0x70000UL
#define INFRA_TR_IN_SEL_TR5_SEL_Pos             20UL
#define INFRA_TR_IN_SEL_TR5_SEL_Msk             0x700000UL
#define INFRA_TR_IN_SEL_TR6_SEL_Pos             24UL
#define INFRA_TR_IN_SEL_TR6_SEL_Msk             0x7000000UL
#define INFRA_TR_IN_SEL_TR7_SEL_Pos             28UL
#define INFRA_TR_IN_SEL_TR7_SEL_Msk             0x70000000UL
/* INFRA.HW_TR_MODE */
#define INFRA_HW_TR_MODE_HW_TR0_MODE_Pos        0UL
#define INFRA_HW_TR_MODE_HW_TR0_MODE_Msk        0xFUL
#define INFRA_HW_TR_MODE_HW_TR1_MODE_Pos        4UL
#define INFRA_HW_TR_MODE_HW_TR1_MODE_Msk        0xF0UL
#define INFRA_HW_TR_MODE_HW_TR2_MODE_Pos        8UL
#define INFRA_HW_TR_MODE_HW_TR2_MODE_Msk        0xF00UL
#define INFRA_HW_TR_MODE_HW_TR3_MODE_Pos        12UL
#define INFRA_HW_TR_MODE_HW_TR3_MODE_Msk        0xF000UL
#define INFRA_HW_TR_MODE_HW_TR4_MODE_Pos        16UL
#define INFRA_HW_TR_MODE_HW_TR4_MODE_Msk        0xF0000UL
#define INFRA_HW_TR_MODE_HW_TR5_MODE_Pos        20UL
#define INFRA_HW_TR_MODE_HW_TR5_MODE_Msk        0xF00000UL
#define INFRA_HW_TR_MODE_HW_TR6_MODE_Pos        24UL
#define INFRA_HW_TR_MODE_HW_TR6_MODE_Msk        0xF000000UL
#define INFRA_HW_TR_MODE_HW_TR7_MODE_Pos        28UL
#define INFRA_HW_TR_MODE_HW_TR7_MODE_Msk        0xF0000000UL
/* INFRA.FW_TR_PULSE */
#define INFRA_FW_TR_PULSE_FW_TR_PULSE_Pos       0UL
#define INFRA_FW_TR_PULSE_FW_TR_PULSE_Msk       0xFFUL
/* INFRA.FW_TR_LEVEL */
#define INFRA_FW_TR_LEVEL_FW_TR_LEVEL_Pos       0UL
#define INFRA_FW_TR_LEVEL_FW_TR_LEVEL_Msk       0xFFUL
/* INFRA.CLOCK_STARTUP_DIV */
#define INFRA_CLOCK_STARTUP_DIV_DIV_VAL_Pos     0UL
#define INFRA_CLOCK_STARTUP_DIV_DIV_VAL_Msk     0xFFUL
/* INFRA.STARTUP_CFG */
#define INFRA_STARTUP_CFG_COUNT_VAL_Pos         0UL
#define INFRA_STARTUP_CFG_COUNT_VAL_Msk         0xFFUL
#define INFRA_STARTUP_CFG_SAR_EN_Pos            8UL
#define INFRA_STARTUP_CFG_SAR_EN_Msk            0x100UL
#define INFRA_STARTUP_CFG_CSG_CH_EN_Pos         9UL
#define INFRA_STARTUP_CFG_CSG_CH_EN_Msk         0x200UL
#define INFRA_STARTUP_CFG_CSG_PWR_EN_SLICE_Pos  10UL
#define INFRA_STARTUP_CFG_CSG_PWR_EN_SLICE_Msk  0x400UL
#define INFRA_STARTUP_CFG_CSG_READY_Pos         11UL
#define INFRA_STARTUP_CFG_CSG_READY_Msk         0x800UL
/* INFRA.DDFT */
#define INFRA_DDFT_DDFT0_SEL_Pos                0UL
#define INFRA_DDFT_DDFT0_SEL_Msk                0x1FUL
#define INFRA_DDFT_DDFT1_SEL_Pos                8UL
#define INFRA_DDFT_DDFT1_SEL_Msk                0x1F00UL
/* INFRA.VDDA_STATUS */
#define INFRA_VDDA_STATUS_VDDA_OK_Pos           0UL
#define INFRA_VDDA_STATUS_VDDA_OK_Msk           0x1UL
/* INFRA.SAR_CHAR_IF */
#define INFRA_SAR_CHAR_IF_DOUT_EN_Pos           0UL
#define INFRA_SAR_CHAR_IF_DOUT_EN_Msk           0x1UL
#define INFRA_SAR_CHAR_IF_SEND_SYNC_Pos         1UL
#define INFRA_SAR_CHAR_IF_SEND_SYNC_Msk         0x2UL
#define INFRA_SAR_CHAR_IF_START_Pos             2UL
#define INFRA_SAR_CHAR_IF_START_Msk             0x4UL
/* INFRA.VREF_TRIM0 */
#define INFRA_VREF_TRIM0_VREF_ABS_TRIM_Pos      0UL
#define INFRA_VREF_TRIM0_VREF_ABS_TRIM_Msk      0xFFUL
/* INFRA.VREF_TRIM1 */
#define INFRA_VREF_TRIM1_VREF_TEMPCO_TRIM_Pos   0UL
#define INFRA_VREF_TRIM1_VREF_TEMPCO_TRIM_Msk   0xFFUL
/* INFRA.VREF_TRIM2 */
#define INFRA_VREF_TRIM2_VREF_CURV_TRIM_Pos     0UL
#define INFRA_VREF_TRIM2_VREF_CURV_TRIM_Msk     0xFFUL
/* INFRA.VREF_TRIM3 */
#define INFRA_VREF_TRIM3_VREF_ATTEN_TRIM_Pos    0UL
#define INFRA_VREF_TRIM3_VREF_ATTEN_TRIM_Msk    0xFUL
/* INFRA.IZTAT_TRIM0 */
#define INFRA_IZTAT_TRIM0_IZTAT_ABS_TRIM_Pos    0UL
#define INFRA_IZTAT_TRIM0_IZTAT_ABS_TRIM_Msk    0xFFUL
/* INFRA.IZTAT_TRIM1 */
#define INFRA_IZTAT_TRIM1_IZTAT_TC_TRIM_Pos     0UL
#define INFRA_IZTAT_TRIM1_IZTAT_TC_TRIM_Msk     0xFFUL
/* INFRA.IPTAT_TRIM0 */
#define INFRA_IPTAT_TRIM0_IPTAT_CORE_TRIM_Pos   0UL
#define INFRA_IPTAT_TRIM0_IPTAT_CORE_TRIM_Msk   0xFUL
#define INFRA_IPTAT_TRIM0_IPTAT_CTBM_TRIM_Pos   4UL
#define INFRA_IPTAT_TRIM0_IPTAT_CTBM_TRIM_Msk   0xF0UL
/* INFRA.ICTAT_TRIM0 */
#define INFRA_ICTAT_TRIM0_ICTAT_TRIM_Pos        0UL
#define INFRA_ICTAT_TRIM0_ICTAT_TRIM_Msk        0xFUL


#endif /* _CYIP_INFRA_H_ */


/* [] END OF FILE */
