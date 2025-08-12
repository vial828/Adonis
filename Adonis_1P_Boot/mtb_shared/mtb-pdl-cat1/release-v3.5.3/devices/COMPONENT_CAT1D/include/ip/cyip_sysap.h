/***************************************************************************//**
* \file cyip_sysap.h
*
* \brief
* SYSAP IP definitions
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

#ifndef _CYIP_SYSAP_H_
#define _CYIP_SYSAP_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    SYSAP
*******************************************************************************/

#define SYSAP_ROMTABLE_SECTION_SIZE             0x00001000UL
#define SYSAP_SECTION_SIZE                      0x20000000UL

/**
  * \brief System Debug Access Port ROM-Table with Cypress Vendor/Silicon ID (SYSAP_ROMTABLE)
  */
typedef struct {
   __IM uint32_t RESERVED[1011];
   __IM uint32_t DID;                           /*!< 0x00000FCC Device Type Identifier register. */
   __IM uint32_t PID4;                          /*!< 0x00000FD0 Peripheral Identification Register 4. */
   __IM uint32_t PID5;                          /*!< 0x00000FD4 Peripheral Identification Register 5. */
   __IM uint32_t PID6;                          /*!< 0x00000FD8 Peripheral Identification Register 6. */
   __IM uint32_t PID7;                          /*!< 0x00000FDC Peripheral Identification Register 7. */
   __IM uint32_t PID0;                          /*!< 0x00000FE0 Peripheral Identification Register 0. */
   __IM uint32_t PID1;                          /*!< 0x00000FE4 Peripheral Identification Register 1. */
   __IM uint32_t PID2;                          /*!< 0x00000FE8 Peripheral Identification Register 2. */
   __IM uint32_t PID3;                          /*!< 0x00000FEC Peripheral Identification Register 3. */
   __IM uint32_t CID0;                          /*!< 0x00000FF0 Component Identification Register 0. */
   __IM uint32_t CID1;                          /*!< 0x00000FF4 Component Identification Register 1. */
   __IM uint32_t CID2;                          /*!< 0x00000FF8 Component Identification Register 2. */
   __IM uint32_t CID3;                          /*!< 0x00000FFC Component Identification Register 3. */
} SYSAP_ROMTABLE_Type;                          /*!< Size = 4096 (0x1000) */

/**
  * \brief System debug Access Port (SYSAP) registers (SYSAP)
  */
typedef struct {
   __IM uint32_t RESERVED[194560];
        SYSAP_ROMTABLE_Type ROMTABLE;           /*!< 0x000BE000 System Debug Access Port ROM-Table with Cypress Vendor/Silicon
                                                                ID */
} SYSAP_Type;                                   /*!< Size = 782336 (0xBF000) */


/* SYSAP_ROMTABLE.DID */
#define SYSAP_ROMTABLE_DID_VALUE_Pos            0UL
#define SYSAP_ROMTABLE_DID_VALUE_Msk            0xFFFFFFFFUL
/* SYSAP_ROMTABLE.PID4 */
#define SYSAP_ROMTABLE_PID4_JEP_CONTINUATION_Pos 0UL
#define SYSAP_ROMTABLE_PID4_JEP_CONTINUATION_Msk 0xFUL
#define SYSAP_ROMTABLE_PID4_COUNT_Pos           4UL
#define SYSAP_ROMTABLE_PID4_COUNT_Msk           0xF0UL
/* SYSAP_ROMTABLE.PID5 */
#define SYSAP_ROMTABLE_PID5_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_PID5_VALUE_Msk           0xFFFFFFFFUL
/* SYSAP_ROMTABLE.PID6 */
#define SYSAP_ROMTABLE_PID6_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_PID6_VALUE_Msk           0xFFFFFFFFUL
/* SYSAP_ROMTABLE.PID7 */
#define SYSAP_ROMTABLE_PID7_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_PID7_VALUE_Msk           0xFFFFFFFFUL
/* SYSAP_ROMTABLE.PID0 */
#define SYSAP_ROMTABLE_PID0_PN_MIN_Pos          0UL
#define SYSAP_ROMTABLE_PID0_PN_MIN_Msk          0xFFUL
/* SYSAP_ROMTABLE.PID1 */
#define SYSAP_ROMTABLE_PID1_PN_MAJ_Pos          0UL
#define SYSAP_ROMTABLE_PID1_PN_MAJ_Msk          0xFUL
#define SYSAP_ROMTABLE_PID1_JEPID_MIN_Pos       4UL
#define SYSAP_ROMTABLE_PID1_JEPID_MIN_Msk       0xF0UL
/* SYSAP_ROMTABLE.PID2 */
#define SYSAP_ROMTABLE_PID2_JEPID_MAJ_Pos       0UL
#define SYSAP_ROMTABLE_PID2_JEPID_MAJ_Msk       0x7UL
#define SYSAP_ROMTABLE_PID2_REV_Pos             4UL
#define SYSAP_ROMTABLE_PID2_REV_Msk             0xF0UL
/* SYSAP_ROMTABLE.PID3 */
#define SYSAP_ROMTABLE_PID3_CM_Pos              0UL
#define SYSAP_ROMTABLE_PID3_CM_Msk              0xFUL
#define SYSAP_ROMTABLE_PID3_REV_AND_Pos         4UL
#define SYSAP_ROMTABLE_PID3_REV_AND_Msk         0xF0UL
/* SYSAP_ROMTABLE.CID0 */
#define SYSAP_ROMTABLE_CID0_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_CID0_VALUE_Msk           0xFFFFFFFFUL
/* SYSAP_ROMTABLE.CID1 */
#define SYSAP_ROMTABLE_CID1_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_CID1_VALUE_Msk           0xFFFFFFFFUL
/* SYSAP_ROMTABLE.CID2 */
#define SYSAP_ROMTABLE_CID2_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_CID2_VALUE_Msk           0xFFFFFFFFUL
/* SYSAP_ROMTABLE.CID3 */
#define SYSAP_ROMTABLE_CID3_VALUE_Pos           0UL
#define SYSAP_ROMTABLE_CID3_VALUE_Msk           0xFFFFFFFFUL


#endif /* _CYIP_SYSAP_H_ */


/* [] END OF FILE */
