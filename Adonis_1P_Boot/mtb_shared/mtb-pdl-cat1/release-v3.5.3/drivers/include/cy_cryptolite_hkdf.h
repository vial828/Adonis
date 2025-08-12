/***************************************************************************//**
* \file cy_cryptolite_hkdf.h
* \version 2.30
*
* \brief
*  This file provides constants and function prototypes
*  for the API for the HKDF method in the cryptolite block driver.
*
********************************************************************************
* \copyright
* Copyright (c) (2020-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/


#if !defined (CY_CRYPTOLITE_HKDF__H)
#define CY_CRYPTOLITE_HKDF__H

#include "cy_device.h"


#if defined(CY_IP_MXCRYPTOLITE)

#if defined(__cplusplus)
extern "C" {
#endif

#include "cy_cryptolite_common.h"

#if (CRYPTOLITE_SHA_PRESENT == 1)
#if defined(CY_CRYPTOLITE_CFG_HKDF_C) && defined(CY_CRYPTOLITE_CFG_HMAC_C)

#include "cy_cryptolite_hmac.h"


cy_en_cryptolite_status_t Cy_Cryptolite_Hkdf_Extract(CRYPTOLITE_Type *base,     
                                          uint8_t  const *salt,
                                          uint32_t saltLength,
                                          uint8_t  const *ikm,
                                          uint32_t ikmLength,
                                          uint8_t *prk);

cy_en_cryptolite_status_t Cy_Cryptolite_Hkdf_Expand(CRYPTOLITE_Type *base,       
                                          uint8_t  const *prk,
                                          uint32_t prkLength,
                                          uint8_t  const *info,
                                          uint32_t infoLength,
                                          uint8_t *okm,
                                          uint32_t okmLength);
                                                                                   
cy_en_cryptolite_status_t Cy_Cryptolite_Hkdf(CRYPTOLITE_Type *base,      
                                          uint8_t  const *salt,
                                          uint32_t saltLength,
                                          uint8_t  const *ikm,
                                          uint32_t ikmLength,
                                          uint8_t  const *info,
                                          uint32_t infoLength,
                                          uint8_t *okm,
                                          uint32_t okmLength);

#endif /* defined(CY_CRYPTOLITE_CFG_HKDF_C) && defined(CY_CRYPTOLITE_CFG_HMAC_C) */
#endif /*CRYPTOLITE_SHA_PRESENT*/

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXCRYPTOLITE */

#endif /* #if !defined (CY_CRYPTOLITE_HKDF__H) */


/* [] END OF FILE */
