/***************************************************************************//**
* \file cyip_ms_ctl_2_0_v2.h
*
* \brief
* MS_CTL_2_0 IP definitions
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

#ifndef _CYIP_MS_CTL_2_0_V2_H_
#define _CYIP_MS_CTL_2_0_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                  MS_CTL_2_0
*******************************************************************************/

#define MS_CTL_2_0_SECTION_SIZE                 0x00004000UL

/**
  * \brief Master control registers (MS_CTL_2_0)
  */
typedef struct {
        MS_Type  MS[32];                        /*!< 0x00000000 Master protection context control */
   __IM uint32_t RESERVED[896];
        MS_PC_Type MS_PC[32];                   /*!< 0x00001000 Master protection context value */
   __IM uint32_t RESERVED1[896];
  __IOM uint32_t CODE_MS0_MSC_ACG_CTL;          /*!< 0x00002000 CODE_MS0 master security Controller & ACG configuration */
   __IM uint32_t RESERVED2[3];
  __IOM uint32_t SYS_MS0_MSC_ACG_CTL;           /*!< 0x00002010 SYS_MS0  master security Controller & ACG configuration */
  __IOM uint32_t SYS_MS1_MSC_ACG_CTL;           /*!< 0x00002014 SYS_MS1  master security Controller & ACG configuration */
   __IM uint32_t RESERVED3[2];
  __IOM uint32_t EXP_MS_MSC_ACG_CTL;            /*!< 0x00002020 EXP_MS  master security Controller & ACG configuration */
   __IM uint32_t RESERVED4[3];
  __IOM uint32_t DMAC0_MSC_ACG_CTL;             /*!< 0x00002030 DMAC-0  master security Controller & ACG configuration */
   __IM uint32_t RESERVED5[3];
  __IOM uint32_t DMAC1_MSC_ACG_CTL;             /*!< 0x00002040 DMAC-1  master security Controller & ACG configuration */
} MS_CTL_2_0_Type;                              /*!< Size = 8260 (0x2044) */


/* MS_CTL_2_0.CODE_MS0_MSC_ACG_CTL */
#define MS_CTL_2_0_CODE_MS0_MSC_ACG_CTL_CFG_GATE_RESP_Pos 0UL
#define MS_CTL_2_0_CODE_MS0_MSC_ACG_CTL_CFG_GATE_RESP_Msk 0x1UL
#define MS_CTL_2_0_CODE_MS0_MSC_ACG_CTL_SEC_RESP_Pos 1UL
#define MS_CTL_2_0_CODE_MS0_MSC_ACG_CTL_SEC_RESP_Msk 0x2UL
/* MS_CTL_2_0.SYS_MS0_MSC_ACG_CTL */
#define MS_CTL_2_0_SYS_MS0_MSC_ACG_CTL_CFG_GATE_RESP_Pos 0UL
#define MS_CTL_2_0_SYS_MS0_MSC_ACG_CTL_CFG_GATE_RESP_Msk 0x1UL
#define MS_CTL_2_0_SYS_MS0_MSC_ACG_CTL_SEC_RESP_Pos 1UL
#define MS_CTL_2_0_SYS_MS0_MSC_ACG_CTL_SEC_RESP_Msk 0x2UL
/* MS_CTL_2_0.SYS_MS1_MSC_ACG_CTL */
#define MS_CTL_2_0_SYS_MS1_MSC_ACG_CTL_CFG_GATE_RESP_Pos 0UL
#define MS_CTL_2_0_SYS_MS1_MSC_ACG_CTL_CFG_GATE_RESP_Msk 0x1UL
#define MS_CTL_2_0_SYS_MS1_MSC_ACG_CTL_SEC_RESP_Pos 1UL
#define MS_CTL_2_0_SYS_MS1_MSC_ACG_CTL_SEC_RESP_Msk 0x2UL
/* MS_CTL_2_0.EXP_MS_MSC_ACG_CTL */
#define MS_CTL_2_0_EXP_MS_MSC_ACG_CTL_CFG_GATE_RESP_Pos 0UL
#define MS_CTL_2_0_EXP_MS_MSC_ACG_CTL_CFG_GATE_RESP_Msk 0x1UL
#define MS_CTL_2_0_EXP_MS_MSC_ACG_CTL_SEC_RESP_Pos 1UL
#define MS_CTL_2_0_EXP_MS_MSC_ACG_CTL_SEC_RESP_Msk 0x2UL
/* MS_CTL_2_0.DMAC0_MSC_ACG_CTL */
#define MS_CTL_2_0_DMAC0_MSC_ACG_CTL_CFG_GATE_RESP_Pos 0UL
#define MS_CTL_2_0_DMAC0_MSC_ACG_CTL_CFG_GATE_RESP_Msk 0x1UL
#define MS_CTL_2_0_DMAC0_MSC_ACG_CTL_SEC_RESP_Pos 1UL
#define MS_CTL_2_0_DMAC0_MSC_ACG_CTL_SEC_RESP_Msk 0x2UL
/* MS_CTL_2_0.DMAC1_MSC_ACG_CTL */
#define MS_CTL_2_0_DMAC1_MSC_ACG_CTL_CFG_GATE_RESP_Pos 0UL
#define MS_CTL_2_0_DMAC1_MSC_ACG_CTL_CFG_GATE_RESP_Msk 0x1UL
#define MS_CTL_2_0_DMAC1_MSC_ACG_CTL_SEC_RESP_Pos 1UL
#define MS_CTL_2_0_DMAC1_MSC_ACG_CTL_SEC_RESP_Msk 0x2UL


#endif /* _CYIP_MS_CTL_2_0_V2_H_ */


/* [] END OF FILE */
