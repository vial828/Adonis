/***************************************************************************//**
* \file cyboot_flash_list.h
* \version 1.0.0
* Provides header file for a flash programming API.
********************************************************************************
* \copyright
* (c) 2023, Cypress Semiconductor Corporation (an Infineon company) or an
* affiliate of Cypress Semiconductor Corporation.  All rights reserved.
* This software, associated documentation and materials ("Software") is owned
* by Cypress Semiconductor Corporation or one of its affiliates ("Cypress") and
* is protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you obtained this
* Software ("EULA"). If no EULA applies, then any reproduction, modification,
* translation, compilation, or representation of this Software is prohibited
* without the express written permission of Cypress.
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice.
* Cypress does not assume any liability arising out of the application or use
* of the Software or any product or circuit described in the Software. Cypress
* does not authorize its products for use in any products where a malfunction
* or failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product").
* By including Cypress's product in a High Risk Product, the manufacturer of
* such system or application assumes all risk of such use and in doing so
* agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef CY_FB_FLASH_API_H
#define CY_FB_FLASH_API_H

#include <stdint.h>
#include <stdbool.h>
/** \cond INTERNAL */
typedef struct
{
    uint32_t state; /* A state machine - EMPTY, ERASE_0, PROGRAM_0, WRITE_0,1 */
    /*
    * [0] 0 -BLOCKING, 1-NON-BLOCKING
    * [1] RWW or STALL_READ, for BLOCKING
    * [3:2] 
    *   0- column 33 is filled with zeros, no refresh is supported.
    *   1- column 33 is updated by Flash API, data_addr buffer contains enough space.
    *   2- column 33 is updated by Flash API, a new buffer is created, data_addr size is 32 columns.
    *   3- column 33 is included in flash data (data_addr).
    */
    uint32_t flags;
    uint32_t flash_addr; /* Non-blocking WriteRow */
    uint32_t data_addr;  /* Non-blocking WriteRow */
    uint32_t hv_params_addr; /* 0 for SFLASH, anything else for a user defined HV_PARAMS */
} cyboot_flash_context_t;

typedef enum {
    CYBOOT_FLASH_SUCCESS = 0xA0000000,
    CYBOOT_FLASH_FAIL    = 0xF0000000,
} cyboot_flash_result_t;

#define CY_FLASH_ROW_SIZE           (512UL)
#define CY_FLASH_RWW_MODE           (1UL)
#define CY_FLASH_SYSTEM_MSB_MASK    (0xFFC00000UL)
#define CY_WRITE_ROW_SUCCESS        (1UL)
#define CY_WRITE_ROW_ERROR          (0UL)
#define CY_FLASH_PARAMETERS_ADDR    (0x13400400UL)
#define CY_FLASH_PARAMETERS_SIZE    (CY_FLASH_ROW_SIZE)

#define CY_FLASH_MODE_STALL_READ    (1UL << 1UL)
#define CY_FLASH_MODE_BLOCKING      (1UL << 0UL)

typedef void (*cyboot_flash_complete_t)(cyboot_flash_context_t *ctx);
typedef bool (*cyboot_is_flash_ready_t)(cyboot_flash_context_t *ctx);
typedef void (*cyboot_flash_irq_handler_t)(void);

/* Blocking functions */
typedef cyboot_flash_result_t (*cyboot_flash_erase_row_t)(uint32_t flash_address, cyboot_flash_context_t *ctx);
typedef cyboot_flash_result_t (*cyboot_flash_program_row_t)(uint32_t flash_address, const void *data, cyboot_flash_context_t *ctx);
typedef cyboot_flash_result_t (*cyboot_flash_write_row_t)(uint32_t flash_address, const void *data, cyboot_flash_context_t *ctx);

/* Non-blocking functions */
typedef cyboot_flash_result_t (*cyboot_flash_erase_row_start_t)(uint32_t flash_address, cyboot_flash_context_t *ctx);
typedef cyboot_flash_result_t (*cyboot_flash_program_row_start_t)(uint32_t flash_address, const void *data,
    cyboot_flash_context_t *ctx);
typedef cyboot_flash_result_t (*cyboot_flash_write_row_start_t)(uint32_t flash_address, const void *data,
    cyboot_flash_context_t *ctx);

typedef struct
{
    cyboot_flash_complete_t cyboot_flash_complete;
    cyboot_is_flash_ready_t cyboot_is_flash_ready;
    cyboot_flash_irq_handler_t cyboot_flash_irq_handler;
    
    cyboot_flash_erase_row_t cyboot_flash_erase_row;
    cyboot_flash_program_row_t cyboot_flash_program_row;
    cyboot_flash_write_row_t cyboot_flash_write_row;
    
    cyboot_flash_erase_row_start_t cyboot_flash_erase_row_start;
    cyboot_flash_program_row_start_t cyboot_flash_program_row_start;
    cyboot_flash_write_row_start_t cyboot_flash_write_row_start;
} rom_funct_list_t;

#define ROM_FUNC    ((rom_funct_list_t *)0x10800040)

/*
* Usage:
* - ROM_FUNC->cyboot_flash_erase(addr, ctx);
* - ROM_FUNC->cyboot_flash_program(addr, data, ctx);
* - etc.
*/
/** \endcond */
#endif
/* [] END OF FILE */
