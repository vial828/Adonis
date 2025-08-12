/**
  ******************************************************************************
  * @file    usr_ecc.h
  * @author  king.hu@smooretech.com
  * @date    2024/07/30
  * @version V1.00
  * @brief   Brief description.
  *
  *   This file is used for unlock logger.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 SMOORE TECHNOLOGY CO.,LTD.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  * Change Logs:
  * Date            Version    Author                       Notes
  * 2024-07-30      V1.00      king.hu@smooretech.com       the first version
  *
  ******************************************************************************
  */
  
#ifndef _USR_ECC_H
#define _USR_ECC_H

#include "uECC.h"
#include "sha256.h"

/**
********************************************************************************
* @brief   ECC verify.
* @param   random[in]  Input random data.
*          hash[in]    The hash of the signed data.
*          sign[in]    The signature value.
* @return  Operation state, it will be:
*            1: success
*            0: failed
* @note    None
********************************************************************************
**/
uint8_t ecc_verify(const uint8_t* random, const uint8_t* hash, const uint8_t* sign);

#endif

/*******************************************************************************
** End Of File
*******************************************************************************/
