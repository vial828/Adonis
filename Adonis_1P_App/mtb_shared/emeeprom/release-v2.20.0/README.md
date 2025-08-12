# Emulated EEPROM Middleware Library

### Overview
The Emulated EEPROM middleware emulates an EEPROM device in the PSoC device flash memory. The EEPROM middleware operates on the top of the Flash driver included in the Peripheral Driver Library (mtb-pdl-cat1 or mtb-pdl-cat2).

Use the Emulated EEPROM to store nonvolatile data on a target device, with wear leveling and restoring corrupted data from a redundant copy.

### Features
* EEPROM-Like Non-Volatile Storage
* Easy to use Read and Write API
* Optional Wear Leveling
* Optional Redundant Data storage

### XMC7xxx and T2G-B-H specific differences
* Emulated EEPROM has to be used with XMC7xxx and T2G-B-H Work Flash, not with Code Flash.
* XMC7xxx and T2G-B-H provides Work Flash with Small Sector size or Large sector size. By default the library supports small sector work flash. To use large sector work flash, please use Emulated EEPROM personality or add "EEPROM_LARGE_SECTOR_WFLASH" to DEFINES= in Makefile. The middleware build verifies the selected Work flash region and mapped flash address passed to initialization function.

### Quick Start
Refer to the [API Reference Guide Quick Start Guide](https://cypresssemiconductorco.github.io/emeeprom/em_eeprom_api_reference_manual/html/index.html#section_em_eeprom_quick_start) section for step-by-step instruction how to enable the Emulated EEPROM Middleware Library.

### More information
For more information, refer to the following documents:
* [Emulated EEPROM Middleware RELEASE.md](./RELEASE.md)
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
