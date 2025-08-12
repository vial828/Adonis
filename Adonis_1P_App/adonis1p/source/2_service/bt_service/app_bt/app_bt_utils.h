/******************************************************************************
* File Name:   util_functions.h
*
* Description: This file consists of the utility functions that will help
*              debugging and developing the applications easier with much
*              more meaningful information.
*
* Related Document: See Readme.md
*
********************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef __APP_BT_UTILS_H__
#define __APP_BT_UTILS_H__

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"
#include <stdio.h>
#include <stdbool.h>

/******************************************************************************
 *                                Constants
 ******************************************************************************/
#define CASE_RETURN_STR(enum_val)          case enum_val: return #enum_val;

#define FROM_BIT16_TO_8(val)            ((uint8_t)((val) >> 8 ))

                                    //stream((the most significant octet)0x12 34 56 78) merge to 0x12345678  
#define APP_BT_MERGE_4BYTES_BIG_ENDDIAN(a, b, c, d)  (uint32_t)( \
                                                (((uint32_t)((uint32_t)(a) & 0xffu)) << 24u) | \
                                                (((uint32_t)((uint32_t)(b) & 0xffu)) << 16u) | \
                                                (((uint32_t)((uint32_t)(c) & 0xffu)) <<  8u) | \
                                                (((uint32_t)((uint32_t)(d) & 0xffu))) )

												            //stream((the most significant octet)0x12 34 56 78) merge to 0x78563412  
#define APP_BT_MERGE_4BYTES_LITTLE_ENDDIAN(a, b, c, d)  (uint32_t)( \
                                                (((uint32_t)((uint32_t)(d) & 0xffu)) << 24u) | \
                                                (((uint32_t)((uint32_t)(c) & 0xffu)) << 16u) | \
                                                (((uint32_t)((uint32_t)(b) & 0xffu)) <<  8u) | \
                                                (((uint32_t)((uint32_t)(a) & 0xffu))) )

                                    //stream((the most significant octet)0x12 34) merge to 0x1234
#define APP_BT_MERGE_2BYTES_BIG_ENDDIAN(a, b)  (uint16_t)( \
                                                (((uint16_t)((uint16_t)(a) & 0xffu)) <<  8u) | \
                                                (((uint16_t)((uint16_t)(b) & 0xffu))) )
                                                
												            //stream((the most significant octet)0x12 34) merge to 0x3412 
#define APP_BT_MERGE_2BYTES_LITTLE_ENDDIAN(a, b)  (uint16_t)( \
                                                (((uint16_t)((uint16_t)(b) & 0xffu)) <<  8u) | \
                                                (((uint16_t)((uint16_t)(a) & 0xffu))) )

/****************************************************************************
 *                              FUNCTION DECLARATIONS
 ***************************************************************************/
void print_bd_address(wiced_bt_device_address_t bdadr);
void print_array(void * to_print, uint16_t len);

const char *get_bt_event_name(wiced_bt_management_evt_t event);
const char *get_bt_advert_mode_name(wiced_bt_ble_advert_mode_t mode);
const char *get_bt_gatt_disconn_reason_name(wiced_bt_gatt_disconn_reason_t reason);
const char *get_bt_gatt_status_name(wiced_bt_gatt_status_t status);

void data_array_invert_endianness(void *inArrPtr, uint32_t byteSize);
bool data_array_is_equal(uint8_t *arr1, uint8_t *arr2, uint8_t size);

#endif      /*__APP_BT_UTILS_H__ */


/* [] END OF FILE */
