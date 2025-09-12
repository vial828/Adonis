/*******************************************************************************
 * File Name: cycfg_dmas.h
 *
 * Description:
 * DMA configuration
 * This file was automatically generated and should not be modified.
 * Configurator Backend 3.20.0
 * device-db 4.11.1.5194
 * mtb-pdl-cat1 3.9.0.29592
 *
 *******************************************************************************
 * Copyright 2025 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
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
 ******************************************************************************/

#if !defined(CYCFG_DMAS_H)
#define CYCFG_DMAS_H

#include "cycfg_notices.h"
#include "cy_dma.h"

#if defined (CY_USING_HAL)
#include "cyhal_hwmgr.h"
#endif /* defined (CY_USING_HAL) */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define CYBSP_AMOLED_SPI_DMA_TX_ENABLED 1U
#define CYBSP_AMOLED_SPI_DMA_TX_HW DW0
#define CYBSP_AMOLED_SPI_DMA_TX_CHANNEL 0U
#define CYBSP_AMOLED_SPI_DMA_TX_IRQ cpuss_interrupts_dw0_0_IRQn

extern const cy_stc_dma_descriptor_config_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0_config;
extern const cy_stc_dma_descriptor_config_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1_config;
extern const cy_stc_dma_descriptor_config_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2_config;
extern cy_stc_dma_descriptor_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0;
extern cy_stc_dma_descriptor_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1;
extern cy_stc_dma_descriptor_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2;
extern const cy_stc_dma_channel_config_t CYBSP_AMOLED_SPI_DMA_TX_channelConfig;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_AMOLED_SPI_DMA_TX_obj;
#endif /* defined (CY_USING_HAL) */

void reserve_cycfg_dmas(void);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CYCFG_DMAS_H */
