/***************************************************************************//**
* \file cyip_ms_pc_v2.h
*
* \brief
* MS_PC IP definitions
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

#ifndef _CYIP_MS_PC_V2_H_
#define _CYIP_MS_PC_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    MS_PC
*******************************************************************************/

#define MS_PC_SECTION_SIZE                      0x00000010UL

/**
  * \brief Master protection context value (MS_PC)
  */
typedef struct {
  __IOM uint32_t PC;                            /*!< 0x00000000 Master 'x' protection context value */
   __IM uint32_t PC_READ_MIR;                   /*!< 0x00000004 Master 'x' protection context value read mirror register */
   __IM uint32_t RESERVED[2];
} MS_PC_Type;                                   /*!< Size = 16 (0x10) */


/* MS_PC.PC */
#define MS_PC_PC_PC_Pos                         0UL
#define MS_PC_PC_PC_Msk                         0xFUL
#define MS_PC_PC_PC_SAVED_Pos                   16UL
#define MS_PC_PC_PC_SAVED_Msk                   0xF0000UL
/* MS_PC.PC_READ_MIR */
#define MS_PC_PC_READ_MIR_PC_Pos                0UL
#define MS_PC_PC_READ_MIR_PC_Msk                0xFUL
#define MS_PC_PC_READ_MIR_PC_SAVED_Pos          16UL
#define MS_PC_PC_READ_MIR_PC_SAVED_Msk          0xF0000UL


#endif /* _CYIP_MS_PC_V2_H_ */


/* [] END OF FILE */
