/*******************************************************************************
 * File Name: cycfg_pins.h
 *
 * Description:
 * Pin configuration
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

#if !defined(CYCFG_PINS_H)
#define CYCFG_PINS_H

#include "cycfg_notices.h"
#include "cy_gpio.h"
#include "cycfg_routing.h"

#if defined (CY_USING_HAL)
#include "cyhal_hwmgr.h"
#endif /* defined (CY_USING_HAL) */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#if defined (CY_USING_HAL)
#define CYBSP_WCO_IN (P0_0)
#define CYBSP_WCO_OUT (P0_1)
#define CYBSP_D7 (P0_2)
#define CYBSP_LED_RGB_RED (P0_3)
#define CYBSP_USER_LED3 CYBSP_LED_RGB_RED
#define CYBSP_SW2 (P0_4)
#define CYBSP_USER_BTN CYBSP_SW2
#define CYBSP_USER_BTN1 CYBSP_SW2
#define CYBSP_A0 (P10_0)
#define CYBSP_J2_1 CYBSP_A0
#define CYBSP_A1 (P10_1)
#define CYBSP_J2_3 CYBSP_A1
#define CYBSP_A2 (P10_2)
#define CYBSP_J2_5 CYBSP_A2
#define CYBSP_A3 (P10_3)
#define CYBSP_J2_7 CYBSP_A3
#define CYBSP_A4 (P10_4)
#define CYBSP_J2_9 CYBSP_A4
#define CYBSP_A5 (P10_5)
#define CYBSP_J2_11 CYBSP_A5
#define CYBSP_A6 (P10_6)
#define CYBSP_J2_13 CYBSP_A6
#define CYBSP_QSPI_FRAM_SSEL (P11_0)
#define CYBSP_LED_RGB_BLUE (P11_1)
#define CYBSP_USER_LED5 CYBSP_LED_RGB_BLUE
#define CYBSP_QSPI_SS (P11_2)
#define CYBSP_QSPI_FLASH_SSEL CYBSP_QSPI_SS
#define CYBSP_QSPI_D3 (P11_3)
#define CYBSP_QSPI_D2 (P11_4)
#define CYBSP_QSPI_D1 (P11_5)
#define CYBSP_QSPI_D0 (P11_6)
#define CYBSP_QSPI_SCK (P11_7)
#define CYBSP_D11 (P12_0)
#define CYBSP_D12 (P12_1)
#define CYBSP_D13 (P12_2)
#define CYBSP_D10 (P12_3)
#define CYBSP_D8 (P13_0)
#define CYBSP_D9 (P13_1)
#define CYBSP_J2_19 (P13_6)
#define CYBSP_LED9 (P13_7)
#define CYBSP_USER_LED2 CYBSP_LED9
#define CYBSP_J2_20 CYBSP_LED9
#define CYBSP_CSD_TX (P1_0)
#define CYBSP_CS_TX CYBSP_CSD_TX
#define CYBSP_LED_RGB_GREEN (P1_1)
#define CYBSP_USER_LED4 CYBSP_LED_RGB_GREEN
#define CYBSP_LED8 (P1_5)
#define CYBSP_USER_LED CYBSP_LED8
#define CYBSP_USER_LED1 CYBSP_LED8
#define CYBSP_D0 (P5_0)
#define CYBSP_D1 (P5_1)
#define CYBSP_D2 (P5_2)
#define CYBSP_D3 (P5_3)
#define CYBSP_D4 (P5_4)
#define CYBSP_D5 (P5_5)
#define CYBSP_D6 (P5_6)
#define CYBSP_I2C_SCL (P6_0)
#define CYBSP_D15 CYBSP_I2C_SCL
#define CYBSP_I2C_SDA (P6_1)
#define CYBSP_D14 CYBSP_I2C_SDA
#define CYBSP_A7 (P6_2)
#define CYBSP_J2_15 CYBSP_A7
#define CYBSP_J2_17 (P6_3)
#define CYBSP_SWO (P6_4)
#define CYBSP_VCC_EN (P6_5)
#endif /* defined (CY_USING_HAL) */

#define CYBSP_SWDIO_ENABLED 1U
#define CYBSP_SWDIO_PORT GPIO_PRT6
#define CYBSP_SWDIO_PORT_NUM 6U
#define CYBSP_SWDIO_PIN 6U
#define CYBSP_SWDIO_NUM 6U
#define CYBSP_SWDIO_DRIVEMODE CY_GPIO_DM_PULLUP
#define CYBSP_SWDIO_INIT_DRIVESTATE 1
#ifndef ioss_0_port_6_pin_6_HSIOM
    #define ioss_0_port_6_pin_6_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_SWDIO_HSIOM ioss_0_port_6_pin_6_HSIOM
#define CYBSP_SWDIO_IRQ ioss_interrupts_gpio_6_IRQn

#if defined (CY_USING_HAL)
#define CYBSP_SWDIO_HAL_PORT_PIN P6_6
#define CYBSP_SWDIO P6_6
#define CYBSP_SWDIO_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_SWDIO_HAL_DIR CYHAL_GPIO_DIR_BIDIRECTIONAL 
#define CYBSP_SWDIO_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_PULLUP
#endif /* defined (CY_USING_HAL) */

#define CYBSP_SWDCK_ENABLED 1U
#define CYBSP_SWDCK_PORT GPIO_PRT6
#define CYBSP_SWDCK_PORT_NUM 6U
#define CYBSP_SWDCK_PIN 7U
#define CYBSP_SWDCK_NUM 7U
#define CYBSP_SWDCK_DRIVEMODE CY_GPIO_DM_PULLDOWN
#define CYBSP_SWDCK_INIT_DRIVESTATE 1
#ifndef ioss_0_port_6_pin_7_HSIOM
    #define ioss_0_port_6_pin_7_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_SWDCK_HSIOM ioss_0_port_6_pin_7_HSIOM
#define CYBSP_SWDCK_IRQ ioss_interrupts_gpio_6_IRQn

#if defined (CY_USING_HAL)
#define CYBSP_SWDCK_HAL_PORT_PIN P6_7
#define CYBSP_SWDCK P6_7
#define CYBSP_SWDCK_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_SWDCK_HAL_DIR CYHAL_GPIO_DIR_BIDIRECTIONAL 
#define CYBSP_SWDCK_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_PULLDOWN
#endif /* defined (CY_USING_HAL) */

#define CYBSP_SPI_MOSI_ENABLED 1U
#define CYBSP_SPI_MOSI_PORT GPIO_PRT7
#define CYBSP_SPI_MOSI_PORT_NUM 7U
#define CYBSP_SPI_MOSI_PIN 0U
#define CYBSP_SPI_MOSI_NUM 0U
#define CYBSP_SPI_MOSI_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define CYBSP_SPI_MOSI_INIT_DRIVESTATE 1
#ifndef ioss_0_port_7_pin_0_HSIOM
    #define ioss_0_port_7_pin_0_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_SPI_MOSI_HSIOM ioss_0_port_7_pin_0_HSIOM
#define CYBSP_SPI_MOSI_IRQ ioss_interrupts_gpio_7_IRQn

#if defined (CY_USING_HAL)
#define CYBSP_SPI_MOSI_HAL_PORT_PIN P7_0
#define CYBSP_SPI_MOSI P7_0
#define CYBSP_SPI_MOSI_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_SPI_MOSI_HAL_DIR CYHAL_GPIO_DIR_OUTPUT 
#define CYBSP_SPI_MOSI_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_STRONG
#endif /* defined (CY_USING_HAL) */

#define CYBSP_SPI_MISO_ENABLED 1U
#define CYBSP_SPI_MISO_PORT GPIO_PRT7
#define CYBSP_SPI_MISO_PORT_NUM 7U
#define CYBSP_SPI_MISO_PIN 1U
#define CYBSP_SPI_MISO_NUM 1U
#define CYBSP_SPI_MISO_DRIVEMODE CY_GPIO_DM_HIGHZ
#define CYBSP_SPI_MISO_INIT_DRIVESTATE 1
#ifndef ioss_0_port_7_pin_1_HSIOM
    #define ioss_0_port_7_pin_1_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_SPI_MISO_HSIOM ioss_0_port_7_pin_1_HSIOM
#define CYBSP_SPI_MISO_IRQ ioss_interrupts_gpio_7_IRQn

#if defined (CY_USING_HAL)
#define CYBSP_SPI_MISO_HAL_PORT_PIN P7_1
#define CYBSP_SPI_MISO P7_1
#define CYBSP_SPI_MISO_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_SPI_MISO_HAL_DIR CYHAL_GPIO_DIR_INPUT 
#define CYBSP_SPI_MISO_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_NONE
#endif /* defined (CY_USING_HAL) */

#define CYBSP_SPI_SCK_ENABLED 1U
#define CYBSP_SPI_SCK_PORT GPIO_PRT7
#define CYBSP_SPI_SCK_PORT_NUM 7U
#define CYBSP_SPI_SCK_PIN 2U
#define CYBSP_SPI_SCK_NUM 2U
#define CYBSP_SPI_SCK_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define CYBSP_SPI_SCK_INIT_DRIVESTATE 1
#ifndef ioss_0_port_7_pin_2_HSIOM
    #define ioss_0_port_7_pin_2_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_SPI_SCK_HSIOM ioss_0_port_7_pin_2_HSIOM
#define CYBSP_SPI_SCK_IRQ ioss_interrupts_gpio_7_IRQn

#if defined (CY_USING_HAL)
#define CYBSP_SPI_SCK_HAL_PORT_PIN P7_2
#define CYBSP_SPI_SCK P7_2
#define CYBSP_SPI_SCK_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_SPI_SCK_HAL_DIR CYHAL_GPIO_DIR_OUTPUT 
#define CYBSP_SPI_SCK_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_STRONG
#endif /* defined (CY_USING_HAL) */

#define CYBSP_SPI_CSN_ENABLED 1U
#define CYBSP_SPI_CSN_PORT GPIO_PRT7
#define CYBSP_SPI_CSN_PORT_NUM 7U
#define CYBSP_SPI_CSN_PIN 3U
#define CYBSP_SPI_CSN_NUM 3U
#define CYBSP_SPI_CSN_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define CYBSP_SPI_CSN_INIT_DRIVESTATE 1
#ifndef ioss_0_port_7_pin_3_HSIOM
    #define ioss_0_port_7_pin_3_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_SPI_CSN_HSIOM ioss_0_port_7_pin_3_HSIOM
#define CYBSP_SPI_CSN_IRQ ioss_interrupts_gpio_7_IRQn

#if defined (CY_USING_HAL)
#define CYBSP_SPI_CSN_HAL_PORT_PIN P7_3
#define CYBSP_SPI_CSN P7_3
#define CYBSP_SPI_CSN_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_SPI_CSN_HAL_DIR CYHAL_GPIO_DIR_OUTPUT 
#define CYBSP_SPI_CSN_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_STRONG
#define CYBSP_IOVCC_EN (P7_7)
#define CYBSP_CSD_BTN0 (P8_1)
#define CYBSP_CS_BTN0 CYBSP_CSD_BTN0
#define CYBSP_CSD_BTN1 (P8_2)
#define CYBSP_CS_BTN1 CYBSP_CSD_BTN1
#define CYBSP_CSD_SLD0 (P8_3)
#define CYBSP_CS_SLD0 CYBSP_CSD_SLD0
#define CYBSP_CSD_SLD1 (P8_4)
#define CYBSP_CS_SLD1 CYBSP_CSD_SLD1
#define CYBSP_CSD_SLD2 (P8_5)
#define CYBSP_CS_SLD2 CYBSP_CSD_SLD2
#define CYBSP_CSD_SLD3 (P8_6)
#define CYBSP_CS_SLD3 CYBSP_CSD_SLD3
#define CYBSP_CSD_SLD4 (P8_7)
#define CYBSP_CS_SLD4 CYBSP_CSD_SLD4
#endif /* defined (CY_USING_HAL) */

#define CYBSP_DEBUG_UART_RX_ENABLED 1U
#define CYBSP_A8_ENABLED CYBSP_DEBUG_UART_RX_ENABLED
#define CYBSP_J2_2_ENABLED CYBSP_DEBUG_UART_RX_ENABLED
#define CYBSP_DEBUG_UART_RX_PORT GPIO_PRT9
#define CYBSP_A8_PORT CYBSP_DEBUG_UART_RX_PORT
#define CYBSP_J2_2_PORT CYBSP_DEBUG_UART_RX_PORT
#define CYBSP_DEBUG_UART_RX_PORT_NUM 9U
#define CYBSP_A8_PORT_NUM CYBSP_DEBUG_UART_RX_PORT_NUM
#define CYBSP_J2_2_PORT_NUM CYBSP_DEBUG_UART_RX_PORT_NUM
#define CYBSP_DEBUG_UART_RX_PIN 0U
#define CYBSP_A8_PIN CYBSP_DEBUG_UART_RX_PIN
#define CYBSP_J2_2_PIN CYBSP_DEBUG_UART_RX_PIN
#define CYBSP_DEBUG_UART_RX_NUM 0U
#define CYBSP_A8_NUM CYBSP_DEBUG_UART_RX_NUM
#define CYBSP_J2_2_NUM CYBSP_DEBUG_UART_RX_NUM
#define CYBSP_DEBUG_UART_RX_DRIVEMODE CY_GPIO_DM_HIGHZ
#define CYBSP_A8_DRIVEMODE CYBSP_DEBUG_UART_RX_DRIVEMODE
#define CYBSP_J2_2_DRIVEMODE CYBSP_DEBUG_UART_RX_DRIVEMODE
#define CYBSP_DEBUG_UART_RX_INIT_DRIVESTATE 1
#define CYBSP_A8_INIT_DRIVESTATE CYBSP_DEBUG_UART_RX_INIT_DRIVESTATE
#define CYBSP_J2_2_INIT_DRIVESTATE CYBSP_DEBUG_UART_RX_INIT_DRIVESTATE
#ifndef ioss_0_port_9_pin_0_HSIOM
    #define ioss_0_port_9_pin_0_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_DEBUG_UART_RX_HSIOM ioss_0_port_9_pin_0_HSIOM
#define CYBSP_A8_HSIOM CYBSP_DEBUG_UART_RX_HSIOM
#define CYBSP_J2_2_HSIOM CYBSP_DEBUG_UART_RX_HSIOM
#define CYBSP_DEBUG_UART_RX_IRQ ioss_interrupts_gpio_9_IRQn
#define CYBSP_A8_IRQ CYBSP_DEBUG_UART_RX_IRQ
#define CYBSP_J2_2_IRQ CYBSP_DEBUG_UART_RX_IRQ

#if defined (CY_USING_HAL)
#define CYBSP_DEBUG_UART_RX_HAL_PORT_PIN P9_0
#define CYBSP_A8_HAL_PORT_PIN CYBSP_DEBUG_UART_RX_HAL_PORT_PIN
#define CYBSP_J2_2_HAL_PORT_PIN CYBSP_DEBUG_UART_RX_HAL_PORT_PIN
#define CYBSP_DEBUG_UART_RX P9_0
#define CYBSP_A8 CYBSP_DEBUG_UART_RX
#define CYBSP_J2_2 CYBSP_DEBUG_UART_RX
#define CYBSP_DEBUG_UART_RX_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_A8_HAL_IRQ CYBSP_DEBUG_UART_RX_HAL_IRQ
#define CYBSP_J2_2_HAL_IRQ CYBSP_DEBUG_UART_RX_HAL_IRQ
#define CYBSP_DEBUG_UART_RX_HAL_DIR CYHAL_GPIO_DIR_INPUT 
#define CYBSP_A8_HAL_DIR CYBSP_DEBUG_UART_RX_HAL_DIR
#define CYBSP_J2_2_HAL_DIR CYBSP_DEBUG_UART_RX_HAL_DIR
#define CYBSP_DEBUG_UART_RX_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_NONE
#define CYBSP_A8_HAL_DRIVEMODE CYBSP_DEBUG_UART_RX_HAL_DRIVEMODE
#define CYBSP_J2_2_HAL_DRIVEMODE CYBSP_DEBUG_UART_RX_HAL_DRIVEMODE
#endif /* defined (CY_USING_HAL) */

#define CYBSP_DEBUG_UART_TX_ENABLED 1U
#define CYBSP_A9_ENABLED CYBSP_DEBUG_UART_TX_ENABLED
#define CYBSP_J2_4_ENABLED CYBSP_DEBUG_UART_TX_ENABLED
#define CYBSP_DEBUG_UART_TX_PORT GPIO_PRT9
#define CYBSP_A9_PORT CYBSP_DEBUG_UART_TX_PORT
#define CYBSP_J2_4_PORT CYBSP_DEBUG_UART_TX_PORT
#define CYBSP_DEBUG_UART_TX_PORT_NUM 9U
#define CYBSP_A9_PORT_NUM CYBSP_DEBUG_UART_TX_PORT_NUM
#define CYBSP_J2_4_PORT_NUM CYBSP_DEBUG_UART_TX_PORT_NUM
#define CYBSP_DEBUG_UART_TX_PIN 1U
#define CYBSP_A9_PIN CYBSP_DEBUG_UART_TX_PIN
#define CYBSP_J2_4_PIN CYBSP_DEBUG_UART_TX_PIN
#define CYBSP_DEBUG_UART_TX_NUM 1U
#define CYBSP_A9_NUM CYBSP_DEBUG_UART_TX_NUM
#define CYBSP_J2_4_NUM CYBSP_DEBUG_UART_TX_NUM
#define CYBSP_DEBUG_UART_TX_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define CYBSP_A9_DRIVEMODE CYBSP_DEBUG_UART_TX_DRIVEMODE
#define CYBSP_J2_4_DRIVEMODE CYBSP_DEBUG_UART_TX_DRIVEMODE
#define CYBSP_DEBUG_UART_TX_INIT_DRIVESTATE 1
#define CYBSP_A9_INIT_DRIVESTATE CYBSP_DEBUG_UART_TX_INIT_DRIVESTATE
#define CYBSP_J2_4_INIT_DRIVESTATE CYBSP_DEBUG_UART_TX_INIT_DRIVESTATE
#ifndef ioss_0_port_9_pin_1_HSIOM
    #define ioss_0_port_9_pin_1_HSIOM HSIOM_SEL_GPIO
#endif
#define CYBSP_DEBUG_UART_TX_HSIOM ioss_0_port_9_pin_1_HSIOM
#define CYBSP_A9_HSIOM CYBSP_DEBUG_UART_TX_HSIOM
#define CYBSP_J2_4_HSIOM CYBSP_DEBUG_UART_TX_HSIOM
#define CYBSP_DEBUG_UART_TX_IRQ ioss_interrupts_gpio_9_IRQn
#define CYBSP_A9_IRQ CYBSP_DEBUG_UART_TX_IRQ
#define CYBSP_J2_4_IRQ CYBSP_DEBUG_UART_TX_IRQ

#if defined (CY_USING_HAL)
#define CYBSP_DEBUG_UART_TX_HAL_PORT_PIN P9_1
#define CYBSP_A9_HAL_PORT_PIN CYBSP_DEBUG_UART_TX_HAL_PORT_PIN
#define CYBSP_J2_4_HAL_PORT_PIN CYBSP_DEBUG_UART_TX_HAL_PORT_PIN
#define CYBSP_DEBUG_UART_TX P9_1
#define CYBSP_A9 CYBSP_DEBUG_UART_TX
#define CYBSP_J2_4 CYBSP_DEBUG_UART_TX
#define CYBSP_DEBUG_UART_TX_HAL_IRQ CYHAL_GPIO_IRQ_NONE
#define CYBSP_A9_HAL_IRQ CYBSP_DEBUG_UART_TX_HAL_IRQ
#define CYBSP_J2_4_HAL_IRQ CYBSP_DEBUG_UART_TX_HAL_IRQ
#define CYBSP_DEBUG_UART_TX_HAL_DIR CYHAL_GPIO_DIR_OUTPUT 
#define CYBSP_A9_HAL_DIR CYBSP_DEBUG_UART_TX_HAL_DIR
#define CYBSP_J2_4_HAL_DIR CYBSP_DEBUG_UART_TX_HAL_DIR
#define CYBSP_DEBUG_UART_TX_HAL_DRIVEMODE CYHAL_GPIO_DRIVE_STRONG
#define CYBSP_A9_HAL_DRIVEMODE CYBSP_DEBUG_UART_TX_HAL_DRIVEMODE
#define CYBSP_J2_4_HAL_DRIVEMODE CYBSP_DEBUG_UART_TX_HAL_DRIVEMODE
#define CYBSP_A10 (P9_2)
#define CYBSP_J2_6 CYBSP_A10
#define CYBSP_A11 (P9_3)
#define CYBSP_J2_8 CYBSP_A11
#define CYBSP_A12 (P9_4)
#define CYBSP_J2_10 CYBSP_A12
#define CYBSP_A13 (P9_5)
#define CYBSP_J2_12 CYBSP_A13
#define CYBSP_A15 (P9_6)
#define CYBSP_J2_16 CYBSP_A15
#define CYBSP_J2_18 (P9_7)
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_SWDIO_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_SWDIO_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_SWDCK_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_SWDCK_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_SPI_MOSI_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_SPI_MOSI_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_SPI_MISO_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_SPI_MISO_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_SPI_SCK_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_SPI_SCK_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_SPI_CSN_config;

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_SPI_CSN_obj;
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_DEBUG_UART_RX_config;

#define CYBSP_A8_config CYBSP_DEBUG_UART_RX_config
#define CYBSP_J2_2_config CYBSP_DEBUG_UART_RX_config

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_DEBUG_UART_RX_obj;
#define CYBSP_A8_obj CYBSP_DEBUG_UART_RX_obj
#define CYBSP_J2_2_obj CYBSP_DEBUG_UART_RX_obj
#endif /* defined (CY_USING_HAL) */

extern const cy_stc_gpio_pin_config_t CYBSP_DEBUG_UART_TX_config;

#define CYBSP_A9_config CYBSP_DEBUG_UART_TX_config
#define CYBSP_J2_4_config CYBSP_DEBUG_UART_TX_config

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_DEBUG_UART_TX_obj;
#define CYBSP_A9_obj CYBSP_DEBUG_UART_TX_obj
#define CYBSP_J2_4_obj CYBSP_DEBUG_UART_TX_obj
#endif /* defined (CY_USING_HAL) */

void init_cycfg_pins(void);
void reserve_cycfg_pins(void);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CYCFG_PINS_H */
