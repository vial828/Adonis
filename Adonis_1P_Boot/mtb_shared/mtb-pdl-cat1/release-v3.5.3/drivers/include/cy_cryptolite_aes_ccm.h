/***************************************************************************//**
* \file cy_cryptolite_aes_ccm.h
* \version 2.20
*
* \brief
*  This file provides common constants and parameters
*  for the Cryptolite AES CCM driver.
*
********************************************************************************
* Copyright 2023 Cypress Semiconductor Corporation
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

#if !defined (CY_CRYPTOLITE_AES_CCM_H)
#define CY_CRYPTOLITE_AES_CCM_H

#include "cy_device.h"

#if defined (CY_IP_MXCRYPTOLITE)

#if defined(__cplusplus)
extern "C" {
#endif

#include "cy_cryptolite_common.h"

#if defined(CY_CRYPTOLITE_CFG_AES_C)
#if (CRYPTOLITE_AES_PRESENT == 1) 
#if defined(CY_CRYPTOLITE_CFG_CIPHER_MODE_CCM)

#include "cy_cryptolite_aes.h"
#include "cy_cryptolite_vu.h"

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Init(CRYPTOLITE_Type *base,
                                            cy_stc_cryptolite_aes_ccm_buffers_t * aesCcm_buffer, cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_SetKey(CRYPTOLITE_Type *base,
                                            uint8_t const *key, cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Set_Length(CRYPTOLITE_Type *base,
                                            uint32_t aadSize,  uint32_t textSize, uint32_t tagLength,
                                            cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Start(CRYPTOLITE_Type *base,
                                                        cy_en_cryptolite_dir_mode_t dirMode,    
                                                        uint32_t ivSize, uint8_t const * iv,
                                                        cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Update_Aad(CRYPTOLITE_Type *base,
                                                            uint32_t aadSize,
                                                            uint8_t const *aad,
                                                            cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Update(CRYPTOLITE_Type *base, 
                                                        uint32_t srcSize,
                                                        uint8_t *dst,
                                                        uint8_t const *src,
                                                        cy_stc_cryptolite_aes_ccm_state_t *aesCcmState)  ;

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Finish(CRYPTOLITE_Type *base, uint8_t *tag, cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Encrypt_Tag(CRYPTOLITE_Type *base,
                                                            uint32_t ivSize, uint8_t const * iv,
                                                            uint32_t aadSize, uint8_t const *aad,
                                                            uint32_t srcSize, uint8_t *cipherTxt, uint8_t const *plainTxt,
                                                            uint32_t tagSize, uint8_t *tag,
                                                            cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Decrypt(CRYPTOLITE_Type *base,
                                                            uint32_t ivSize, uint8_t const * iv,
                                                            uint32_t aadSize, uint8_t const *aad,
                                                            uint32_t srcSize, uint8_t *plainTxt, uint8_t const *cipherTxt,
                                                            uint32_t tagSize, uint8_t *tag, cy_en_cryptolite_ccm_auth_result_t *isValid,
                                                            cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);

cy_en_cryptolite_status_t Cy_Cryptolite_Aes_Ccm_Free(CRYPTOLITE_Type *base, cy_stc_cryptolite_aes_ccm_state_t *aesCcmState);


#endif /*CY_CRYPTOLITE_CFG_CIPHER_MODE_CCM*/
#endif /* #if (CY_CRYPTOLITE_CFG_AES_C)*/
#endif /* #if CRYPTOLITE_AES_PRESENT*/

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXCRYPTOLITE */

#endif /* #if !defined (CY_CRYPTOLITE_AES_CCM_H) */

/* [] END OF FILE */
