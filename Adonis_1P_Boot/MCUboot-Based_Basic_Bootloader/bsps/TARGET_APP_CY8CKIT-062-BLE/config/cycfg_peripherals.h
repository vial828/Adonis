/*******************************************************************************
 * File Name: cycfg_peripherals.h
 *
 * Description:
 * Peripheral Hardware Block configuration
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

#if !defined(CYCFG_PERIPHERALS_H)
#define CYCFG_PERIPHERALS_H

#include "cycfg_notices.h"
#include "cy_scb_uart.h"
#include "cy_sysclk.h"
#include "cy_scb_spi.h"

#if defined (CY_USING_HAL)
#include "cyhal_hwmgr.h"
#endif /* defined (CY_USING_HAL) */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define CYBSP_UART_ENABLED 1U
#define CYBSP_UART_HW SCB2
#define CYBSP_UART_IRQ scb_2_interrupt_IRQn
#define CYBSP_AMOLED_SPI_ENABLED 1U
#define CYBSP_AMOLED_SPI_HW SCB4
#define CYBSP_AMOLED_SPI_IRQ scb_4_interrupt_IRQn

extern const cy_stc_scb_uart_config_t CYBSP_UART_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_UART_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_scb_spi_config_t CYBSP_AMOLED_SPI_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_AMOLED_SPI_obj;
#endif /* defined (CY_USING_HAL) */

void init_cycfg_peripherals(void);
void reserve_cycfg_peripherals(void);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CYCFG_PERIPHERALS_H */
