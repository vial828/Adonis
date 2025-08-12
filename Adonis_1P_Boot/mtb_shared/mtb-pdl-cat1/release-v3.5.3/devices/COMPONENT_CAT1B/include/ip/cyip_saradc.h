/***************************************************************************//**
* \file cyip_saradc.h
*
* \brief
* SARADC IP definitions
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

#ifndef _CYIP_SARADC_H_
#define _CYIP_SARADC_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    SARADC
*******************************************************************************/

#define SARADC_SECTION_SIZE                     0x00010000UL

/**
  * \brief SAR ADC Core (Calibration and Test) (SARADC)
  */
typedef struct {
  __IOM uint32_t CALCTRL;                       /*!< 0x00000000 SARADC Calibration Control Register */
  __IOM uint32_t CONVCTRL;                      /*!< 0x00000004 SARADC Conversion Control Register */
  __IOM uint32_t ADFTMUX;                       /*!< 0x00000008 SARADC Analog DFT Multiplexer Control Register */
   __IM uint32_t RESERVED;
  __IOM uint32_t CALOFFST[4];                   /*!< 0x00000010 SARADC Offset Calibration Values */
  __IOM uint32_t CALLIN[16];                    /*!< 0x00000020 SARADC Linearity Calibration Values */
  __IOM uint32_t CALGAINC;                      /*!< 0x00000060 SARADC Coarse-grained Gain Calibration Values */
  __IOM uint32_t CALGAINF;                      /*!< 0x00000064 SARADC Fine-grained Gain Calibration Values */
} SARADC_Type;                                  /*!< Size = 104 (0x68) */


/* SARADC.CALCTRL */
#define SARADC_CALCTRL_OFFDEP_Pos               0UL
#define SARADC_CALCTRL_OFFDEP_Msk               0x1UL
#define SARADC_CALCTRL_AVG_Pos                  8UL
#define SARADC_CALCTRL_AVG_Msk                  0x300UL
#define SARADC_CALCTRL_STC_Pos                  10UL
#define SARADC_CALCTRL_STC_Msk                  0xC00UL
/* SARADC.CONVCTRL */
#define SARADC_CONVCTRL_INIT_TC_Pos             0UL
#define SARADC_CONVCTRL_INIT_TC_Msk             0x3UL
#define SARADC_CONVCTRL_MSB_CTRL_Pos            4UL
#define SARADC_CONVCTRL_MSB_CTRL_Msk            0x10UL
#define SARADC_CONVCTRL_RES_RAW_Pos             5UL
#define SARADC_CONVCTRL_RES_RAW_Msk             0x20UL
#define SARADC_CONVCTRL_VCM_LATCH_Pos           8UL
#define SARADC_CONVCTRL_VCM_LATCH_Msk           0x700UL
/* SARADC.ADFTMUX */
#define SARADC_ADFTMUX_EN_Pos                   0UL
#define SARADC_ADFTMUX_EN_Msk                   0x1UL
#define SARADC_ADFTMUX_SEL_Pos                  8UL
#define SARADC_ADFTMUX_SEL_Msk                  0x1F00UL
/* SARADC.CALOFFST */
#define SARADC_CALOFFST_OFFSET_4N_0_Pos         0UL
#define SARADC_CALOFFST_OFFSET_4N_0_Msk         0x7FUL
#define SARADC_CALOFFST_OFFSET_4N_1_Pos         8UL
#define SARADC_CALOFFST_OFFSET_4N_1_Msk         0x7F00UL
#define SARADC_CALOFFST_OFFSET_4N_2_Pos         16UL
#define SARADC_CALOFFST_OFFSET_4N_2_Msk         0x7F0000UL
#define SARADC_CALOFFST_OFFSET_4N_3_Pos         24UL
#define SARADC_CALOFFST_OFFSET_4N_3_Msk         0x7F000000UL
/* SARADC.CALLIN */
#define SARADC_CALLIN_LIN9_Pos                  0UL
#define SARADC_CALLIN_LIN9_Msk                  0x7FUL
#define SARADC_CALLIN_LIIN10_Pos                8UL
#define SARADC_CALLIN_LIIN10_Msk                0x7F00UL
#define SARADC_CALLIN_LIN11_Pos                 16UL
#define SARADC_CALLIN_LIN11_Msk                 0x7F0000UL
#define SARADC_CALLIN_LIN12_Pos                 24UL
#define SARADC_CALLIN_LIN12_Msk                 0x7F000000UL
/* SARADC.CALGAINC */
#define SARADC_CALGAINC_GAINC3_Pos              0UL
#define SARADC_CALGAINC_GAINC3_Msk              0xFUL
#define SARADC_CALGAINC_GAINC6_Pos              8UL
#define SARADC_CALGAINC_GAINC6_Msk              0xF00UL
#define SARADC_CALGAINC_GAINC12_Pos             16UL
#define SARADC_CALGAINC_GAINC12_Msk             0xF0000UL
/* SARADC.CALGAINF */
#define SARADC_CALGAINF_GAINF3_Pos              0UL
#define SARADC_CALGAINF_GAINF3_Msk              0x7FUL
#define SARADC_CALGAINF_GAINF6_Pos              8UL
#define SARADC_CALGAINF_GAINF6_Msk              0x7F00UL
#define SARADC_CALGAINF_GAINF12_Pos             16UL
#define SARADC_CALGAINF_GAINF12_Msk             0x7F0000UL


#endif /* _CYIP_SARADC_H_ */


/* [] END OF FILE */
