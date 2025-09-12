/*******************************************************************************
 * File Name: cycfg_routing.h
 *
 * Description:
 * Establishes all necessary connections between hardware elements.
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

#if !defined(CYCFG_ROUTING_H)
#define CYCFG_ROUTING_H

#include "cycfg_notices.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define ioss_0_port_6_pin_6_HSIOM P6_6_CPUSS_SWJ_SWDIO_TMS
#define ioss_0_port_6_pin_7_HSIOM P6_7_CPUSS_SWJ_SWCLK_TCLK
#define ioss_0_port_7_pin_0_HSIOM P7_0_SCB4_SPI_MOSI
#define ioss_0_port_7_pin_1_HSIOM P7_1_SCB4_SPI_MISO
#define ioss_0_port_7_pin_2_HSIOM P7_2_SCB4_SPI_CLK
#define ioss_0_port_7_pin_3_HSIOM P7_3_SCB4_SPI_SELECT0
#define ioss_0_port_9_pin_0_HSIOM P9_0_SCB2_UART_RX
#define ioss_0_port_9_pin_1_HSIOM P9_1_SCB2_UART_TX
#define CYBSP_AMOLED_SPI_DMA_TX_tr_in_0_TRIGGER_OUT TRIG0_OUT_CPUSS_DW0_TR_IN0
#define CYBSP_AMOLED_SPI_tr_tx_req_0_TRIGGER_IN_0 TRIG0_IN_TR_GROUP13_OUTPUT9
#define CYBSP_AMOLED_SPI_tr_tx_req_0_TRIGGER_IN_1 TRIG13_IN_SCB4_TR_TX_REQ

void init_cycfg_routing(void);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CYCFG_ROUTING_H */
