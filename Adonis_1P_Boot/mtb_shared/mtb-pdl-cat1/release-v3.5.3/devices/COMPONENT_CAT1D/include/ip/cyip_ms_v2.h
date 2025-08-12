/***************************************************************************//**
* \file cyip_ms_v2.h
*
* \brief
* MS IP definitions
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

#ifndef _CYIP_MS_V2_H_
#define _CYIP_MS_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                      MS
*******************************************************************************/

#define MS_SECTION_SIZE                         0x00000010UL

/**
  * \brief Master protection context control (MS)
  */
typedef struct {
  __IOM uint32_t CTL;                           /*!< 0x00000000 Master 'x' protection context control */
   __IM uint32_t RESERVED[3];
} MS_Type;                                      /*!< Size = 16 (0x10) */


/* MS.CTL */
#define MS_CTL_P_Pos                            0UL
#define MS_CTL_P_Msk                            0x1UL
#define MS_CTL_NS_Pos                           1UL
#define MS_CTL_NS_Msk                           0x2UL
#define MS_CTL_PC_MASK_Pos                      16UL
#define MS_CTL_PC_MASK_Msk                      0xFFFF0000UL


#endif /* _CYIP_MS_V2_H_ */


/* [] END OF FILE */
