/***************************************************************************//**
* \file cyip_csg.h
*
* \brief
* CSG IP definitions
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

#ifndef _CYIP_CSG_H_
#define _CYIP_CSG_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     CSG
*******************************************************************************/

#define CSG_SLICE_SECTION_SIZE                  0x00000040UL
#define CSG_LUT_CFG_SECTION_SIZE                0x00000200UL
#define CSG_SECTION_SIZE                        0x00010000UL

/**
  * \brief CSG Slice configuration registers (CSG_SLICE)
  */
typedef struct {
  __IOM uint32_t CMP_CFG;                       /*!< 0x00000000 Comparator Control Register */
  __IOM uint32_t DAC_CFG;                       /*!< 0x00000004 DAC Control Register */
  __IOM uint32_t DAC_PARAM_SYNC;                /*!< 0x00000008 DAC Parameter Synchronized Update */
  __IOM uint32_t DAC_MODE_START;                /*!< 0x0000000C DAC Mode Start */
  __IOM uint32_t DAC_VAL_A;                     /*!< 0x00000010 DAC Vaue A Buffer */
  __IOM uint32_t DAC_VAL_B;                     /*!< 0x00000014 DAC Value B Buffer */
  __IOM uint32_t DAC_PERIOD;                    /*!< 0x00000018 DAC Period Buffer */
  __IOM uint32_t DAC_VAL;                       /*!< 0x0000001C DAC FW write */
   __IM uint32_t DAC_STATUS;                    /*!< 0x00000020 Read of Current DAC Value */
   __IM uint32_t CMP_STATUS;                    /*!< 0x00000024 Read of Comparator Value */
   __IM uint32_t RESERVED[6];
} CSG_SLICE_Type;                               /*!< Size = 64 (0x40) */

/**
  * \brief DAC LUT waveform data (CSG_LUT_CFG)
  */
typedef struct {
  __IOM uint32_t LUT_DATA[128];                 /*!< 0x00000000 LUT Waveform Data */
} CSG_LUT_CFG_Type;                             /*!< Size = 512 (0x200) */

/**
  * \brief Comparator Slope Generator (CSG)
  */
typedef struct {
        CSG_SLICE_Type SLICE[5];                /*!< 0x00000000 CSG Slice configuration registers */
   __IM uint32_t RESERVED[176];
        CSG_LUT_CFG_Type LUT_CFG[2];            /*!< 0x00000400 DAC LUT waveform data */
   __IM uint32_t RESERVED1[128];
  __IOM uint32_t CSG_CTRL;                      /*!< 0x00000A00 CSG control register */
   __IM uint32_t RESERVED2[63];
  __IOM uint32_t DAC_INTR;                      /*!< 0x00000B00 DAC Interrupt register */
  __IOM uint32_t DAC_INTR_SET;                  /*!< 0x00000B04 DAC Interrupt request set register */
  __IOM uint32_t DAC_INTR_MASK;                 /*!< 0x00000B08 DAC Interrupt request mask */
   __IM uint32_t DAC_INTR_MASKED;               /*!< 0x00000B0C DAC Interrupt request masked */
  __IOM uint32_t CMP_INTR;                      /*!< 0x00000B10 CMP Interrupt register */
  __IOM uint32_t CMP_INTR_SET;                  /*!< 0x00000B14 CMP Interrupt request set register */
  __IOM uint32_t CMP_INTR_MASK;                 /*!< 0x00000B18 CMP Interrupt request mask */
   __IM uint32_t CMP_INTR_MASKED;               /*!< 0x00000B1C CMP Interrupt request masked */
   __IM uint32_t RESERVED3[184];
  __IOM uint32_t CSG_TEST;                      /*!< 0x00000E00 CSG test register */
   __IM uint32_t RESERVED4[63];
  __IOM uint32_t CSG_TRIM;                      /*!< 0x00000F00 CSG trim register */
} CSG_Type;                                     /*!< Size = 3844 (0xF04) */


/* CSG_SLICE.CMP_CFG */
#define CSG_SLICE_CMP_CFG_CMP_AIN_SEL_Pos       0UL
#define CSG_SLICE_CMP_CFG_CMP_AIN_SEL_Msk       0x1UL
#define CSG_SLICE_CMP_CFG_CMP_POLARITY_Pos      1UL
#define CSG_SLICE_CMP_CFG_CMP_POLARITY_Msk      0x2UL
#define CSG_SLICE_CMP_CFG_CMP_BLANK_MODE_Pos    4UL
#define CSG_SLICE_CMP_CFG_CMP_BLANK_MODE_Msk    0x30UL
#define CSG_SLICE_CMP_CFG_CMP_BLANK_TR_SEL_Pos  8UL
#define CSG_SLICE_CMP_CFG_CMP_BLANK_TR_SEL_Msk  0xF00UL
#define CSG_SLICE_CMP_CFG_CMP_EDGE_MODE_Pos     12UL
#define CSG_SLICE_CMP_CFG_CMP_EDGE_MODE_Msk     0x3000UL
/* CSG_SLICE.DAC_CFG */
#define CSG_SLICE_DAC_CFG_DAC_TR_START_SEL_Pos  0UL
#define CSG_SLICE_DAC_CFG_DAC_TR_START_SEL_Msk  0xFUL
#define CSG_SLICE_DAC_CFG_DAC_TR_UPDATE_SEL_Pos 4UL
#define CSG_SLICE_DAC_CFG_DAC_TR_UPDATE_SEL_Msk 0xF0UL
#define CSG_SLICE_DAC_CFG_DAC_MODE_Pos          8UL
#define CSG_SLICE_DAC_CFG_DAC_MODE_Msk          0x700UL
#define CSG_SLICE_DAC_CFG_DAC_CONT_Pos          12UL
#define CSG_SLICE_DAC_CFG_DAC_CONT_Msk          0x1000UL
#define CSG_SLICE_DAC_CFG_DAC_SKIP_TR_EN_Pos    13UL
#define CSG_SLICE_DAC_CFG_DAC_SKIP_TR_EN_Msk    0x2000UL
#define CSG_SLICE_DAC_CFG_DAC_CASCADE_EN_Pos    14UL
#define CSG_SLICE_DAC_CFG_DAC_CASCADE_EN_Msk    0x4000UL
#define CSG_SLICE_DAC_CFG_DAC_PARAM_SYNC_EN_Pos 15UL
#define CSG_SLICE_DAC_CFG_DAC_PARAM_SYNC_EN_Msk 0x8000UL
#define CSG_SLICE_DAC_CFG_DAC_STEP_Pos          20UL
#define CSG_SLICE_DAC_CFG_DAC_STEP_Msk          0x3F00000UL
#define CSG_SLICE_DAC_CFG_DAC_DEGLITCH_Pos      28UL
#define CSG_SLICE_DAC_CFG_DAC_DEGLITCH_Msk      0x70000000UL
/* CSG_SLICE.DAC_PARAM_SYNC */
#define CSG_SLICE_DAC_PARAM_SYNC_READY_Pos      0UL
#define CSG_SLICE_DAC_PARAM_SYNC_READY_Msk      0x1UL
/* CSG_SLICE.DAC_MODE_START */
#define CSG_SLICE_DAC_MODE_START_BUSY_Pos       0UL
#define CSG_SLICE_DAC_MODE_START_BUSY_Msk       0x1UL
#define CSG_SLICE_DAC_MODE_START_HW_START_Pos   30UL
#define CSG_SLICE_DAC_MODE_START_HW_START_Msk   0x40000000UL
#define CSG_SLICE_DAC_MODE_START_FW_START_Pos   31UL
#define CSG_SLICE_DAC_MODE_START_FW_START_Msk   0x80000000UL
/* CSG_SLICE.DAC_VAL_A */
#define CSG_SLICE_DAC_VAL_A_VALUE_Pos           0UL
#define CSG_SLICE_DAC_VAL_A_VALUE_Msk           0x3FFUL
/* CSG_SLICE.DAC_VAL_B */
#define CSG_SLICE_DAC_VAL_B_VALUE_Pos           0UL
#define CSG_SLICE_DAC_VAL_B_VALUE_Msk           0x3FFUL
/* CSG_SLICE.DAC_PERIOD */
#define CSG_SLICE_DAC_PERIOD_PERIOD_FRAC_Pos    3UL
#define CSG_SLICE_DAC_PERIOD_PERIOD_FRAC_Msk    0xF8UL
#define CSG_SLICE_DAC_PERIOD_PERIOD_INT_Pos     8UL
#define CSG_SLICE_DAC_PERIOD_PERIOD_INT_Msk     0xFFF00UL
/* CSG_SLICE.DAC_VAL */
#define CSG_SLICE_DAC_VAL_VALUE_Pos             0UL
#define CSG_SLICE_DAC_VAL_VALUE_Msk             0x3FFUL
/* CSG_SLICE.DAC_STATUS */
#define CSG_SLICE_DAC_STATUS_DAC_VAL_Pos        0UL
#define CSG_SLICE_DAC_STATUS_DAC_VAL_Msk        0x3FFUL
/* CSG_SLICE.CMP_STATUS */
#define CSG_SLICE_CMP_STATUS_CMP_VAL_Pos        0UL
#define CSG_SLICE_CMP_STATUS_CMP_VAL_Msk        0x1UL


/* CSG_LUT_CFG.LUT_DATA */
#define CSG_LUT_CFG_LUT_DATA_DATA_Pos           0UL
#define CSG_LUT_CFG_LUT_DATA_DATA_Msk           0x3FFUL


/* CSG.CSG_CTRL */
#define CSG_CSG_CTRL_VDAC_OUT_SEL_Pos           0UL
#define CSG_CSG_CTRL_VDAC_OUT_SEL_Msk           0x7UL
#define CSG_CSG_CTRL_CSG_VREF_SEL0_Pos          4UL
#define CSG_CSG_CTRL_CSG_VREF_SEL0_Msk          0x10UL
#define CSG_CSG_CTRL_CSG_VREF_SEL1_Pos          5UL
#define CSG_CSG_CTRL_CSG_VREF_SEL1_Msk          0x20UL
#define CSG_CSG_CTRL_CSG_CLK_DIV_BYP_Pos        8UL
#define CSG_CSG_CTRL_CSG_CLK_DIV_BYP_Msk        0x100UL
/* CSG.DAC_INTR */
#define CSG_DAC_INTR_DAC_HW_START_Pos           0UL
#define CSG_DAC_INTR_DAC_HW_START_Msk           0xFFUL
#define CSG_DAC_INTR_DAC_SLOPE_DONE_Pos         8UL
#define CSG_DAC_INTR_DAC_SLOPE_DONE_Msk         0xFF00UL
#define CSG_DAC_INTR_DAC_BUF_EMPTY_Pos          16UL
#define CSG_DAC_INTR_DAC_BUF_EMPTY_Msk          0xFF0000UL
/* CSG.DAC_INTR_SET */
#define CSG_DAC_INTR_SET_DAC_HW_START_Pos       0UL
#define CSG_DAC_INTR_SET_DAC_HW_START_Msk       0xFFUL
#define CSG_DAC_INTR_SET_DAC_SLOPE_DONE_Pos     8UL
#define CSG_DAC_INTR_SET_DAC_SLOPE_DONE_Msk     0xFF00UL
#define CSG_DAC_INTR_SET_DAC_BUF_EMPTY_Pos      16UL
#define CSG_DAC_INTR_SET_DAC_BUF_EMPTY_Msk      0xFF0000UL
/* CSG.DAC_INTR_MASK */
#define CSG_DAC_INTR_MASK_DAC_HW_START_Pos      0UL
#define CSG_DAC_INTR_MASK_DAC_HW_START_Msk      0xFFUL
#define CSG_DAC_INTR_MASK_DAC_SLOPE_DONE_Pos    8UL
#define CSG_DAC_INTR_MASK_DAC_SLOPE_DONE_Msk    0xFF00UL
#define CSG_DAC_INTR_MASK_DAC_BUF_EMPTY_Pos     16UL
#define CSG_DAC_INTR_MASK_DAC_BUF_EMPTY_Msk     0xFF0000UL
/* CSG.DAC_INTR_MASKED */
#define CSG_DAC_INTR_MASKED_DAC_HW_START_Pos    0UL
#define CSG_DAC_INTR_MASKED_DAC_HW_START_Msk    0xFFUL
#define CSG_DAC_INTR_MASKED_DAC_SLOPE_DONE_Pos  8UL
#define CSG_DAC_INTR_MASKED_DAC_SLOPE_DONE_Msk  0xFF00UL
#define CSG_DAC_INTR_MASKED_DAC_BUF_EMPTY_Pos   16UL
#define CSG_DAC_INTR_MASKED_DAC_BUF_EMPTY_Msk   0xFF0000UL
/* CSG.CMP_INTR */
#define CSG_CMP_INTR_CMP_Pos                    0UL
#define CSG_CMP_INTR_CMP_Msk                    0xFFUL
/* CSG.CMP_INTR_SET */
#define CSG_CMP_INTR_SET_CMP_Pos                0UL
#define CSG_CMP_INTR_SET_CMP_Msk                0xFFUL
/* CSG.CMP_INTR_MASK */
#define CSG_CMP_INTR_MASK_CMP_Pos               0UL
#define CSG_CMP_INTR_MASK_CMP_Msk               0xFFUL
/* CSG.CMP_INTR_MASKED */
#define CSG_CMP_INTR_MASKED_CMP_Pos             0UL
#define CSG_CMP_INTR_MASKED_CMP_Msk             0xFFUL
/* CSG.CSG_TEST */
#define CSG_CSG_TEST_ADFT_SEL_Pos               0UL
#define CSG_CSG_TEST_ADFT_SEL_Msk               0x7UL
/* CSG.CSG_TRIM */
#define CSG_CSG_TRIM_TRIM_TBD_Pos               0UL
#define CSG_CSG_TRIM_TRIM_TBD_Msk               0xFFUL


#endif /* _CYIP_CSG_H_ */


/* [] END OF FILE */
