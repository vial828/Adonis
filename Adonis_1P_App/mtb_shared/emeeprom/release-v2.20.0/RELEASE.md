# Emulated EEPROM Middleware Library 2.20

### What's Included?
Please refer to the [README.md](./README.md) and the [API Reference Guide](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html) for a complete description of the Emulated EEPROM Middleware.
The revision history of the Emulated EEPROM Middleware is also available on the [API Reference Guide Changelog](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html#section_em_eeprom_changelog).

New in this release:
* Added support for XMC7xxx devices
* Added support for T2G-B-H devices
* Updated documentation
* Updated minor version

Release version v2.10:
* Added support for PSoC 4 devices
* Updated documentation
* Updated minor version

Release version v2.0:
* Updated major and minor version defines for consistency with other libraries
* Updated documentation for user experience improvement
* Added migration guide from PSoC Creator Em EEPROM component
* Added mechanism to restore corrupted redundant copy from the main data copy

### Defect Fixes
* Fixed MISRA Violation
* Fixed defect of the Cy_Em_EEPROM_Read() function when Emulated EEPROM data corruption in some cases caused infinite loop.
* Fixed defect of the Cy_Em_EEPROM_Read() function when the function returns incorrect data after restoring data from redundant copy.


### Supported Software and Tools
This version of the Emulated EEPROM Middleware was validated for the compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 3.0     |
| CAT1 Peripheral Driver Library (PDL)                    | 3.0.0   |
| CAT2 Peripheral Driver Library (PDL)                    | 1.2.0   |
| GCC Compiler                                            | 10.3.1  |
| IAR Compiler                                            | 8.42.2  |
| ARM Compiler 6                                          | 6.14    |

### More information
For more information, refer to the following documents:
* [Emulated EEPROM Middleware README.md](./README.md)
* [Emulated EEPROM Middleware API Reference Guide](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [CAT1 PDL API Reference](https://cypresssemiconductorco.github.io/mtb-pdl-cat1/pdl_api_reference_manual/html/index.html)
* [CAT2 PDL API Reference](https://cypresssemiconductorco.github.io/mtb-pdl-cat2/pdl_api_reference_manual/html/index.html)
* [AN219434 Importing PSoC Creator Code into an IDE for a PSoC 6 Project](https://www.cypress.com/an219434)
* [AN210781 Getting Started with PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity](http://www.cypress.com/an210781)
* [PSoC 6 Technical Reference Manual](https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference)
* [PSoC 63 with BLE Datasheet Programmable System-on-Chip datasheet](http://www.cypress.com/ds218787)
  
---
(c) 2021-2022, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
This software, associated documentation and materials ("Software") is owned by Cypress Semiconductor Corporation or one of its affiliates ("Cypress") and is protected by and subject to worldwide patent protection (United States and foreign), United States copyright laws and international treaty provisions. Therefore, you may use this Software only as provided in the license agreement accompanying the software package from which you obtained this Software ("EULA"). If no EULA applies, then any reproduction, modification, translation, compilation, or representation of this Software is prohibited without the express written permission of Cypress.
Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress reserves the right to make changes to the Software without notice. Cypress does not assume any liability arising out of the application or use of the Software or any product or circuit described in the Software. Cypress does not authorize its products for use in any products where a malfunction or failure of the Cypress product may reasonably be expected to result in significant property damage, injury or death ("High Risk Product"). By including Cypress's product in a High Risk Product, the manufacturer of such system or application assumes all risk of such use and in doing so agrees to indemnify Cypress against all liability.
