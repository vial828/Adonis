/*******************************************************************************
 * File Name: cycfg_dmas.c
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

#include "cycfg_dmas.h"

const cy_stc_dma_descriptor_config_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0_config =
{
    .retrigger = CY_DMA_RETRIG_IM,
    .interruptType = CY_DMA_DESCR_CHAIN,
    .triggerOutType = CY_DMA_1ELEMENT,
    .channelState = CY_DMA_CHANNEL_ENABLED,
    .triggerInType = CY_DMA_1ELEMENT,
    .dataSize = CY_DMA_BYTE,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_WORD,
    .descriptorType = CY_DMA_2D_TRANSFER,
    .srcAddress = NULL,
    .dstAddress = NULL,
    .srcXincrement = 1,
    .dstXincrement = 0,
    .xCount = 16,
    .srcYincrement = 16,
    .dstYincrement = 0,
    .yCount = 2,
    .nextDescriptor = &CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,
};
const cy_stc_dma_descriptor_config_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1_config =
{
    .retrigger = CY_DMA_RETRIG_IM,
    .interruptType = CY_DMA_DESCR_CHAIN,
    .triggerOutType = CY_DMA_1ELEMENT,
    .channelState = CY_DMA_CHANNEL_ENABLED,
    .triggerInType = CY_DMA_1ELEMENT,
    .dataSize = CY_DMA_BYTE,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_WORD,
    .descriptorType = CY_DMA_2D_TRANSFER,
    .srcAddress = NULL,
    .dstAddress = NULL,
    .srcXincrement = 1,
    .dstXincrement = 0,
    .xCount = 16,
    .srcYincrement = 16,
    .dstYincrement = 0,
    .yCount = 2,
    .nextDescriptor = &CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2,
};
const cy_stc_dma_descriptor_config_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2_config =
{
    .retrigger = CY_DMA_RETRIG_IM,
    .interruptType = CY_DMA_DESCR_CHAIN,
    .triggerOutType = CY_DMA_1ELEMENT,
    .channelState = CY_DMA_CHANNEL_ENABLED,
    .triggerInType = CY_DMA_1ELEMENT,
    .dataSize = CY_DMA_BYTE,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_WORD,
    .descriptorType = CY_DMA_2D_TRANSFER,
    .srcAddress = NULL,
    .dstAddress = NULL,
    .srcXincrement = 1,
    .dstXincrement = 0,
    .xCount = 16,
    .srcYincrement = 16,
    .dstYincrement = 0,
    .yCount = 2,
    .nextDescriptor = NULL,
};
cy_stc_dma_descriptor_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0 =
{
    .ctl = 0UL,
    .src = 0UL,
    .dst = 0UL,
    .xCtl = 0UL,
    .yCtl = 0UL,
    .nextPtr = 0UL,
};
cy_stc_dma_descriptor_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1 =
{
    .ctl = 0UL,
    .src = 0UL,
    .dst = 0UL,
    .xCtl = 0UL,
    .yCtl = 0UL,
    .nextPtr = 0UL,
};
cy_stc_dma_descriptor_t CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2 =
{
    .ctl = 0UL,
    .src = 0UL,
    .dst = 0UL,
    .xCtl = 0UL,
    .yCtl = 0UL,
    .nextPtr = 0UL,
};
const cy_stc_dma_channel_config_t CYBSP_AMOLED_SPI_DMA_TX_channelConfig =
{
    .descriptor = &CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,
    .preemptable = false,
    .priority = 3,
    .enable = false,
    .bufferable = false,
};

#if defined (CY_USING_HAL)
const cyhal_resource_inst_t CYBSP_AMOLED_SPI_DMA_TX_obj =
{
    .type = CYHAL_RSC_DW,
    .block_num = 0U,
    .channel_num = CYBSP_AMOLED_SPI_DMA_TX_CHANNEL,
};
#endif /* defined (CY_USING_HAL) */

void reserve_cycfg_dmas(void)
{
#if defined (CY_USING_HAL)
    cyhal_hwmgr_reserve(&CYBSP_AMOLED_SPI_DMA_TX_obj);
#endif /* defined (CY_USING_HAL) */
}
