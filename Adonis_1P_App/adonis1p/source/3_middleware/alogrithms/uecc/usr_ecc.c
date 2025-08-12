/**
  ******************************************************************************
  * @file    usr_ecc.c
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

#include "usr_ecc.h"

static const uint8_t PUBLIC_KEY[64] = {
//    0xCF, 0xA7, 0x20, 0xD6, 0x4F, 0xDD, 0x03, 0x50,
//    0xF7, 0x4C, 0x22, 0x63, 0xE4, 0x52, 0xCA, 0x07,
//    0xA9, 0xD0, 0x97, 0xBF, 0xDB, 0xBC, 0x8B, 0x12,
//    0xF2, 0x93, 0x56, 0x92, 0x3E, 0xC7, 0x33, 0x96,
//    0xCE, 0xBD, 0x8A, 0x80, 0xE2, 0x92, 0x3A, 0xCE,
//    0x94, 0xE8, 0x6E, 0xD4, 0x30, 0x7F, 0x7F, 0x9B,
//    0x31, 0x6C, 0x19, 0xF5, 0x63, 0x46, 0x60, 0x32,
//    0x06, 0x2E, 0xEF, 0xF5, 0x9C, 0x56, 0x0E, 0x31
    
    0x74, 0xC6, 0x84, 0xF0, 0x09, 0x18, 0x38, 0x78,
    0x18, 0xEA, 0xA9, 0xB4, 0x38, 0x0B, 0xBB, 0xDC,
    0x74, 0xF9, 0x41, 0x3B, 0x78, 0x28, 0x75, 0xE4,
    0x7F, 0x95, 0x44, 0x1C, 0xB3, 0x27, 0x0B, 0xB9,
    0x94, 0x57, 0xE4, 0x27, 0x20, 0xDA, 0x6B, 0x94,
    0xF1, 0x52, 0xA5, 0xE1, 0x6C, 0x50, 0x62, 0x8E,
    0x84, 0x56, 0x61, 0xBD, 0xA7, 0xAF, 0x0D, 0x79,
    0x2B, 0x0D, 0x42, 0xB4, 0x82, 0x5C, 0x2D, 0x30

};

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
uint8_t ecc_verify(const uint8_t* random, const uint8_t* hash, const uint8_t* sign) {
    const struct uECC_Curve_t* curves;
    struct sha256 s;
    uint8_t calcHash[32];
    
    sha256_init(&s);
    sha256_update(&s, random, 32);
    sha256_final(&s, calcHash);
    
    for (uint8_t i=0; i<32; i++) {
      if (*(calcHash + i) != *(hash + i)) {
        return 0;
      }
    }
    
    curves = uECC_secp256k1();
    
    return uECC_verify(PUBLIC_KEY, hash, 32, sign, curves) ? 1 : 0;
}

/*******************************************************************************
** End Of File
*******************************************************************************/
