/***************************************************************************//**
* \file cyip_mcpass_mmio.h
*
* \brief
* MCPASS_MMIO IP definitions
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

#ifndef _CYIP_MCPASS_MMIO_H_
#define _CYIP_MCPASS_MMIO_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                 MCPASS_MMIO
*******************************************************************************/

#define MCPASS_MMIO_MCPASS_FIFO_SECTION_SIZE    0x00000100UL
#define MCPASS_MMIO_SECTION_SIZE                0x00010000UL

/**
  * \brief FIFO Configuration (MCPASS_MMIO_MCPASS_FIFO)
  */
typedef struct {
  __IOM uint32_t CFG;                           /*!< 0x00000000 FIFO configuration register. */
  __IOM uint32_t CLR;                           /*!< 0x00000004 FIFO FW clear register */
   __IM uint32_t RESERVED[2];
  __IOM uint32_t LEVEL[4];                      /*!< 0x00000010 FIFO level register. */
   __IM uint32_t RESERVED1[4];
   __IM uint32_t RD_DATA[4];                    /*!< 0x00000030 FIFO 32-bit read data register */
   __IM uint32_t RESERVED2[4];
   __IM uint32_t USED[4];                       /*!< 0x00000050 FIFO used register */
   __IM uint32_t RESERVED3[4];
   __IM uint32_t STATUS[4];                     /*!< 0x00000070 FIFO status register */
   __IM uint32_t RESERVED4[32];
} MCPASS_MMIO_MCPASS_FIFO_Type;                 /*!< Size = 256 (0x100) */

/**
  * \brief MCPASS top-level MMIO (MCPASS_MMIO)
  */
typedef struct {
   __IM uint32_t FIFO_INTR;                     /*!< 0x00000000 FIFO Interrupt request register */
   __IM uint32_t RESERVED[3];
  __IOM uint32_t FIFO_INTR_SET;                 /*!< 0x00000010 FIFO Interrupt set request register */
   __IM uint32_t RESERVED1[3];
  __IOM uint32_t FIFO_INTR_MASK;                /*!< 0x00000020 FIFO Interrupt mask register */
   __IM uint32_t RESERVED2[3];
   __IM uint32_t FIFO_INTR_MASKED;              /*!< 0x00000030 FIFO Interrupt masked request register */
   __IM uint32_t RESERVED3[3];
  __IOM uint32_t MCPASS_INTR;                   /*!< 0x00000040 MCPASS Interrupt request register */
  __IOM uint32_t MCPASS_INTR_SET;               /*!< 0x00000044 MCPASS Interrupt set request register */
  __IOM uint32_t MCPASS_INTR_MASK;              /*!< 0x00000048 MCPASS Interrupt mask register */
   __IM uint32_t MCPASS_INTR_MASKED;            /*!< 0x0000004C MCPASS Interrupt masked request register */
  __IOM uint32_t TR_LEVEL_CFG;                  /*!< 0x00000050 LEVEL Trigger out configuration */
   __IM uint32_t RESERVED4[3];
  __IOM uint32_t TR_LEVEL_OUT[8];               /*!< 0x00000060 LEVEL Trigger out select register */
  __IOM uint32_t TR_PULSE_OUT[8];               /*!< 0x00000080 PULSE Trigger out select register */
   __IM uint32_t RESERVED5[24];
        MCPASS_MMIO_MCPASS_FIFO_Type MCPASS_FIFO; /*!< 0x00000100 FIFO Configuration */
} MCPASS_MMIO_Type;                             /*!< Size = 512 (0x200) */


/* MCPASS_MMIO_MCPASS_FIFO.CFG */
#define MCPASS_MMIO_MCPASS_FIFO_CFG_SEL_Pos     0UL
#define MCPASS_MMIO_MCPASS_FIFO_CFG_SEL_Msk     0x3UL
#define MCPASS_MMIO_MCPASS_FIFO_CFG_FIFO_TR_EN_Pos 4UL
#define MCPASS_MMIO_MCPASS_FIFO_CFG_FIFO_TR_EN_Msk 0xF0UL
/* MCPASS_MMIO_MCPASS_FIFO.CLR */
#define MCPASS_MMIO_MCPASS_FIFO_CLR_CLR_Pos     0UL
#define MCPASS_MMIO_MCPASS_FIFO_CLR_CLR_Msk     0xFUL
/* MCPASS_MMIO_MCPASS_FIFO.LEVEL */
#define MCPASS_MMIO_MCPASS_FIFO_LEVEL_LEVEL_Pos 0UL
#define MCPASS_MMIO_MCPASS_FIFO_LEVEL_LEVEL_Msk 0x1FUL
/* MCPASS_MMIO_MCPASS_FIFO.RD_DATA */
#define MCPASS_MMIO_MCPASS_FIFO_RD_DATA_RESULT_Pos 0UL
#define MCPASS_MMIO_MCPASS_FIFO_RD_DATA_RESULT_Msk 0x1FFFFFUL
/* MCPASS_MMIO_MCPASS_FIFO.USED */
#define MCPASS_MMIO_MCPASS_FIFO_USED_USED_Pos   0UL
#define MCPASS_MMIO_MCPASS_FIFO_USED_USED_Msk   0x3FUL
/* MCPASS_MMIO_MCPASS_FIFO.STATUS */
#define MCPASS_MMIO_MCPASS_FIFO_STATUS_RD_PTR_Pos 0UL
#define MCPASS_MMIO_MCPASS_FIFO_STATUS_RD_PTR_Msk 0x1FUL
#define MCPASS_MMIO_MCPASS_FIFO_STATUS_WR_PTR_Pos 16UL
#define MCPASS_MMIO_MCPASS_FIFO_STATUS_WR_PTR_Msk 0x1F0000UL


/* MCPASS_MMIO.FIFO_INTR */
#define MCPASS_MMIO_FIFO_INTR_FIFO_LEVEL_Pos    0UL
#define MCPASS_MMIO_FIFO_INTR_FIFO_LEVEL_Msk    0xFUL
/* MCPASS_MMIO.FIFO_INTR_SET */
#define MCPASS_MMIO_FIFO_INTR_SET_FIFO_LEVEL_SET_Pos 0UL
#define MCPASS_MMIO_FIFO_INTR_SET_FIFO_LEVEL_SET_Msk 0xFUL
/* MCPASS_MMIO.FIFO_INTR_MASK */
#define MCPASS_MMIO_FIFO_INTR_MASK_FIFO_LEVEL_MASK_Pos 0UL
#define MCPASS_MMIO_FIFO_INTR_MASK_FIFO_LEVEL_MASK_Msk 0xFUL
/* MCPASS_MMIO.FIFO_INTR_MASKED */
#define MCPASS_MMIO_FIFO_INTR_MASKED_FIFO_LEVEL_MASKED_Pos 0UL
#define MCPASS_MMIO_FIFO_INTR_MASKED_FIFO_LEVEL_MASKED_Msk 0xFUL
/* MCPASS_MMIO.MCPASS_INTR */
#define MCPASS_MMIO_MCPASS_INTR_FIFO_OVERFLOW_Pos 0UL
#define MCPASS_MMIO_MCPASS_INTR_FIFO_OVERFLOW_Msk 0xFUL
#define MCPASS_MMIO_MCPASS_INTR_FIFO_UNDERFLOW_Pos 4UL
#define MCPASS_MMIO_MCPASS_INTR_FIFO_UNDERFLOW_Msk 0xF0UL
#define MCPASS_MMIO_MCPASS_INTR_RESULT_OVERFLOW_Pos 8UL
#define MCPASS_MMIO_MCPASS_INTR_RESULT_OVERFLOW_Msk 0x100UL
#define MCPASS_MMIO_MCPASS_INTR_ENTRY_TR_COLLISION_Pos 9UL
#define MCPASS_MMIO_MCPASS_INTR_ENTRY_TR_COLLISION_Msk 0x200UL
#define MCPASS_MMIO_MCPASS_INTR_ENTRY_HOLD_VIOLATION_Pos 10UL
#define MCPASS_MMIO_MCPASS_INTR_ENTRY_HOLD_VIOLATION_Msk 0x400UL
#define MCPASS_MMIO_MCPASS_INTR_AC_INT_Pos      12UL
#define MCPASS_MMIO_MCPASS_INTR_AC_INT_Msk      0x1000UL
/* MCPASS_MMIO.MCPASS_INTR_SET */
#define MCPASS_MMIO_MCPASS_INTR_SET_FIFO_OVERFLOW_SET_Pos 0UL
#define MCPASS_MMIO_MCPASS_INTR_SET_FIFO_OVERFLOW_SET_Msk 0xFUL
#define MCPASS_MMIO_MCPASS_INTR_SET_FIFO_UNDERFLOW_SET_Pos 4UL
#define MCPASS_MMIO_MCPASS_INTR_SET_FIFO_UNDERFLOW_SET_Msk 0xF0UL
#define MCPASS_MMIO_MCPASS_INTR_SET_RESULT_OVERFLOW_SET_Pos 8UL
#define MCPASS_MMIO_MCPASS_INTR_SET_RESULT_OVERFLOW_SET_Msk 0x100UL
#define MCPASS_MMIO_MCPASS_INTR_SET_ENTRY_TR_COLLISION_SET_Pos 9UL
#define MCPASS_MMIO_MCPASS_INTR_SET_ENTRY_TR_COLLISION_SET_Msk 0x200UL
#define MCPASS_MMIO_MCPASS_INTR_SET_ENTRY_HOLD_VIOLATION_SET_Pos 10UL
#define MCPASS_MMIO_MCPASS_INTR_SET_ENTRY_HOLD_VIOLATION_SET_Msk 0x400UL
#define MCPASS_MMIO_MCPASS_INTR_SET_AC_INT_SET_Pos 12UL
#define MCPASS_MMIO_MCPASS_INTR_SET_AC_INT_SET_Msk 0x1000UL
/* MCPASS_MMIO.MCPASS_INTR_MASK */
#define MCPASS_MMIO_MCPASS_INTR_MASK_FIFO_OVERFLOW_MASK_Pos 0UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_FIFO_OVERFLOW_MASK_Msk 0xFUL
#define MCPASS_MMIO_MCPASS_INTR_MASK_FIFO_UNDERFLOW_MASK_Pos 4UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_FIFO_UNDERFLOW_MASK_Msk 0xF0UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_RESULT_OVERFLOW_MASK_Pos 8UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_RESULT_OVERFLOW_MASK_Msk 0x100UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_ENTRY_TR_COLLISION_MASK_Pos 9UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_ENTRY_TR_COLLISION_MASK_Msk 0x200UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_ENTRY_HOLD_VIOLATION_MASK_Pos 10UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_ENTRY_HOLD_VIOLATION_MASK_Msk 0x400UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_AC_INT_MASK_Pos 12UL
#define MCPASS_MMIO_MCPASS_INTR_MASK_AC_INT_MASK_Msk 0x1000UL
/* MCPASS_MMIO.MCPASS_INTR_MASKED */
#define MCPASS_MMIO_MCPASS_INTR_MASKED_FIFO_OVERFLOW_MASKED_Pos 0UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_FIFO_OVERFLOW_MASKED_Msk 0xFUL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_FIFO_UNDERFLOW_MASKED_Pos 4UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_FIFO_UNDERFLOW_MASKED_Msk 0xF0UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_RESULT_OVERFLOW_MASKED_Pos 8UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_RESULT_OVERFLOW_MASKED_Msk 0x100UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_ENTRY_TR_COLLISION_MASKED_Pos 9UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_ENTRY_TR_COLLISION_MASKED_Msk 0x200UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_ENTRY_HOLD_VIOLATION_MASKED_Pos 10UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_ENTRY_HOLD_VIOLATION_MASKED_Msk 0x400UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_AC_INT_MASKED_Pos 12UL
#define MCPASS_MMIO_MCPASS_INTR_MASKED_AC_INT_MASKED_Msk 0x1000UL
/* MCPASS_MMIO.TR_LEVEL_CFG */
#define MCPASS_MMIO_TR_LEVEL_CFG_BYPASS_SYNC_Pos 0UL
#define MCPASS_MMIO_TR_LEVEL_CFG_BYPASS_SYNC_Msk 0xFFUL
/* MCPASS_MMIO.TR_LEVEL_OUT */
#define MCPASS_MMIO_TR_LEVEL_OUT_CMP_TR_Pos     0UL
#define MCPASS_MMIO_TR_LEVEL_OUT_CMP_TR_Msk     0xFFUL
#define MCPASS_MMIO_TR_LEVEL_OUT_SAR_RANGE_TR_Pos 8UL
#define MCPASS_MMIO_TR_LEVEL_OUT_SAR_RANGE_TR_Msk 0xFF00UL
/* MCPASS_MMIO.TR_PULSE_OUT */
#define MCPASS_MMIO_TR_PULSE_OUT_SEL_Pos        0UL
#define MCPASS_MMIO_TR_PULSE_OUT_SEL_Msk        0xFUL


#endif /* _CYIP_MCPASS_MMIO_H_ */


/* [] END OF FILE */
