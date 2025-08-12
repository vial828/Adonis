/***************************************************************************//**
* \file cyip_sflash.h
*
* \brief
* SFLASH IP definitions
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
*
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
*******************************************************************************/

#ifndef _CYIP_SFLASH_H_
#define _CYIP_SFLASH_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    SFLASH
*******************************************************************************/

#define SFLASH_SECTION_SIZE                     0x00008000UL

/**
  * \brief FLASH Supervisory Region (SFLASH)
  */
typedef struct {
   __IM uint8_t  RESERVED;
  __IOM uint8_t  SI_REVISION_ID;                /*!< 0x00000001 Indicates Silicon Revision ID of the device */
  __IOM uint16_t SILICON_ID;                    /*!< 0x00000002 Indicates Silicon ID of the device */
  __IOM uint16_t FAMILY_ID;                     /*!< 0x00000004 Indicates Family ID of the device */
   __IM uint16_t RESERVED1[9];
  __IOM uint32_t SFLASH_SVN;                    /*!< 0x00000018 SFLASH Subversion */
   __IM uint32_t RESERVED2[377];
  __IOM uint8_t  DIE_LOT[3];                    /*!< 0x00000600 Lot Number (3 bytes) */
  __IOM uint8_t  DIE_WAFER;                     /*!< 0x00000603 Wafer Number */
  __IOM uint8_t  DIE_X;                         /*!< 0x00000604 X Position on Wafer, CRI Pass/Fail Bin */
  __IOM uint8_t  DIE_Y;                         /*!< 0x00000605 Y Position on Wafer, CHI Pass/Fail Bin */
  __IOM uint8_t  DIE_SORT;                      /*!< 0x00000606 Sort1/2/3 Pass/Fail Bin */
  __IOM uint8_t  DIE_MINOR;                     /*!< 0x00000607 Minor Revision Number */
  __IOM uint8_t  DIE_DAY;                       /*!< 0x00000608 Day number */
  __IOM uint8_t  DIE_MONTH;                     /*!< 0x00000609 Month number */
  __IOM uint8_t  DIE_YEAR;                      /*!< 0x0000060A Year number */
   __IM uint8_t  RESERVED3[61];
  __IOM uint16_t SAR_TEMP_MULTIPLIER;           /*!< 0x00000648 SAR Temperature Sensor Multiplication Factor */
  __IOM uint16_t SAR_TEMP_OFFSET;               /*!< 0x0000064A SAR Temperature Sensor Offset */
   __IM uint32_t RESERVED4[8];
  __IOM uint32_t CSP_PANEL_ID;                  /*!< 0x0000066C CSP Panel Id to record panel ID of CSP die */
   __IM uint32_t RESERVED5[52];
  __IOM uint8_t  LDO_0P9V_TRIM;                 /*!< 0x00000740 LDO_0P9V_TRIM */
  __IOM uint8_t  LDO_1P1V_TRIM;                 /*!< 0x00000741 LDO_1P1V_TRIM */
   __IM uint16_t RESERVED6[95];
  __IOM uint32_t IFX_POLICY_SIZE;               /*!< 0x00000800 Size of Infineon Policy */
  __IOM uint8_t  ICV_ROT_KEY_REVOCATION;        /*!< 0x00000804 IFX Root Of Trust Key Revocation to allow for using the second
                                                                key if the first key is revoked due to compromised. The pattern
                                                                of 1-byte state that indicate the key has been revoked can be
                                                                any numbers but not all zeros or all ones */
  __IOM uint8_t  PROT_FW_ROT_KEY_REVOCATION;    /*!< 0x00000805 Protected FW Root Of Trust Key Revocation to allow for using
                                                                the second key if the first key is revoked due to compromised.
                                                                The pattern of 1-byte state that indicate the key has been
                                                                revoked can be any numbers but not all zeros or all ones */
  __IOM uint8_t  OEM_ROT_KEY_REVOCATION;        /*!< 0x00000806 OEM Root Of Trust Key Revocation to allow for using the second
                                                                key if the first key is revoked due to compromised. The pattern
                                                                of 1-byte state that indicate the key has been revoked can be
                                                                any numbers but not all zeros or all ones */
   __IM uint8_t  RESERVED7;
  __IOM uint8_t  NV_COUNTER_RAMAPP;             /*!< 0x00000808 8-bit COUNTER Limits unsuccessful RAMAPP attempts */
  __IOM uint8_t  NV_COUNTER_L1;                 /*!< 0x00000809 8-bit ANTI_ROLLBACK_COUNTER is used to prevent rollback of OEM
                                                                application update. */
  __IOM uint8_t  NV_COUNTER_L2;                 /*!< 0x0000080A 8-bit ANTI_ROLLBACK_COUNTER is used to prevent rollback of OEM
                                                                application update. */
   __IM uint8_t  RESERVED8;
  __IOM uint8_t  L1_HASH[16];                   /*!< 0x0000080C SHA-256 hash of L1 image. Updated after signature check
                                                                succeeds. */
  __IOM uint8_t  L2_HASH[16];                   /*!< 0x0000081C SHA-256 hash of L2 image. Updated after signature check
                                                                succeeds. */
  __IOM uint32_t BOOTROW_CRC;                   /*!< 0x0000082C CRC of the BOOTROW -- See Boot SAS 12.1 'Security Requirements'
                                                                item #5 */
  __IOM uint32_t PROV_OEM_COMPLETE;             /*!< 0x00000830 OEM assets are finalized in SFLASH, no further update is
                                                                allowed vs. further updates are allowed */
  __IOM uint8_t  FB_FLAGS[4];                   /*!< 0x00000834 For internal use only, a default value of 0 should be used at
                                                                production. Allows togging GPIO pin at the start and end of
                                                                Flash boot to perform the boot time measurements. */
  __IOM uint8_t  MPN_PROT_FW_KEY[16];           /*!< 0x00000838 This key is programmed by IFX (cyapp_prov_icv) in SORT or
                                                                PROVISIONED LCS. This key is programmed only for MPNs which
                                                                support PROT_FW, for the other MPN's this key is 0 (all zeros).
                                                                This key is used for decrypting the input parameters by
                                                                cyapp_prot_fw_policy. Key size is 16 bytes. Included in
                                                                FACTORY_HASH. */
   __IM uint32_t RESERVED9[6];
  __IOM uint32_t PROT_FW_ENABLED;               /*!< 0x00000860 Protected FW enabled.  0 if not enabled */
  __IOM uint8_t  PROT_FW_ROT_KEY_0_HASH[16];    /*!< 0x00000864 Truncated SHA-256 hash of Protected FW public key 0 */
  __IOM uint8_t  PROT_FW_ROT_KEY_1_HASH[16];    /*!< 0x00000874 Truncated SHA-256 hash of Protected FW public key 1 */
  __IOM uint8_t  PROT_FW_ENCRYPTION_KEY[16];    /*!< 0x00000884 AES-128 encryption key for protected FW */
  __IOM uint32_t PROT_FW_ADDR;                  /*!< 0x00000894 Start address of protected FW */
  __IOM uint32_t PROT_FW_SIZE;                  /*!< 0x00000898 Size of protected FW */
  __IOM uint32_t PROT_FW_RW_ADDR;               /*!< 0x0000089C Start of protected FW RW area in flash */
  __IOM uint32_t PROT_FW_RW_SIZE;               /*!< 0x000008A0 Size of protected FW RW area in flash */
  __IOM uint32_t PROT_FW_RAM_ADDR;              /*!< 0x000008A4 Start of protected FW RAM */
  __IOM uint32_t PROT_FW_RAM_SIZE;              /*!< 0x000008A8 Size of protected FW RAM */
   __IM uint32_t RESERVED10[981];
  __IOM uint16_t PILO_FREQ_STEP;                /*!< 0x00001800 Resolution step for PILO at class in BCD format */
   __IM uint16_t RESERVED11;
  __IOM uint32_t CSDV2_CSD0_ADC_VREF0;          /*!< 0x00001804 CSD 1p2 & 1p6 voltage levels for accuracy */
  __IOM uint32_t CSDV2_CSD0_ADC_VREF1;          /*!< 0x00001808 CSD 2p1 & 0p8 voltage levels for accuracy */
  __IOM uint32_t CSDV2_CSD0_ADC_VREF2;          /*!< 0x0000180C CSD calibration spare voltage level for accuracy */
  __IOM uint32_t PWR_TRIM_WAKE_CTL;             /*!< 0x00001810 Wakeup delay */
   __IM uint16_t RESERVED12;
  __IOM uint16_t RADIO_LDO_TRIMS;               /*!< 0x00001816 Radio LDO Trims */
  __IOM uint32_t CPUSS_TRIM_ROM_CTL_ULP;        /*!< 0x00001818 CPUSS TRIM ROM CTL ULP value */
  __IOM uint32_t CPUSS_TRIM_RAM_CTL_ULP;        /*!< 0x0000181C CPUSS TRIM RAM CTL ULP value */
  __IOM uint32_t CPUSS_TRIM_ROM_CTL_LP;         /*!< 0x00001820 CPUSS TRIM ROM CTL LP value */
  __IOM uint32_t CPUSS_TRIM_RAM_CTL_LP;         /*!< 0x00001824 CPUSS TRIM RAM CTL LP value */
   __IM uint32_t RESERVED13[7];
  __IOM uint32_t CPUSS_TRIM_ROM_CTL_HALF_ULP;   /*!< 0x00001844 CPUSS TRIM ROM CTL HALF ULP value */
  __IOM uint32_t CPUSS_TRIM_RAM_CTL_HALF_ULP;   /*!< 0x00001848 CPUSS TRIM RAM CTL HALF ULP value */
  __IOM uint32_t CPUSS_TRIM_ROM_CTL_HALF_LP;    /*!< 0x0000184C CPUSS TRIM ROM CTL HALF LP value */
  __IOM uint32_t CPUSS_TRIM_RAM_CTL_HALF_LP;    /*!< 0x00001850 CPUSS TRIM RAM CTL HALF LP value */
   __IM uint32_t RESERVED14[363];
  __IOM uint32_t DEVICE_ID_PRIVATE_KEY;         /*!< 0x00001E00 Device ID private key */
   __IM uint32_t RESERVED15[127];
  __IOM uint32_t FLASH_BOOT_OBJECT_SIZE;        /*!< 0x00002000 Flash Boot - Object Size */
  __IOM uint32_t FLASH_BOOT_APP_ID;             /*!< 0x00002004 Flash Boot - Application ID/Version */
  __IOM uint32_t FLASH_BOOT_ATTRIBUTE;          /*!< 0x00002008 N/A */
  __IOM uint32_t FLASH_BOOT_N_CORES;            /*!< 0x0000200C Flash Boot - Number of Cores(N) */
  __IOM uint32_t FLASH_BOOT_VT_OFFSET;          /*!< 0x00002010 Flash Boot - Core Vector Table offset */
  __IOM uint32_t FLASH_BOOT_CORE_CPUID;         /*!< 0x00002014 Flash Boot - Core CPU ID/Core Index */
  __IOM uint8_t  FLASH_BOOT_CODE[24552];        /*!< 0x00002018 Flash Boot - Code and Data */
} SFLASH_Type;                                  /*!< Size = 32768 (0x8000) */


/* SFLASH.SI_REVISION_ID */
#define SFLASH_SI_REVISION_ID_SI_REVISION_ID_Pos 0UL
#define SFLASH_SI_REVISION_ID_SI_REVISION_ID_Msk 0xFFUL
/* SFLASH.SILICON_ID */
#define SFLASH_SILICON_ID_ID_Pos                0UL
#define SFLASH_SILICON_ID_ID_Msk                0xFFFFUL
/* SFLASH.FAMILY_ID */
#define SFLASH_FAMILY_ID_FAMILY_ID_Pos          0UL
#define SFLASH_FAMILY_ID_FAMILY_ID_Msk          0xFFFFUL
/* SFLASH.SFLASH_SVN */
#define SFLASH_SFLASH_SVN_DATA32_Pos            0UL
#define SFLASH_SFLASH_SVN_DATA32_Msk            0xFFFFFFFFUL
/* SFLASH.DIE_LOT */
#define SFLASH_DIE_LOT_LOT_Pos                  0UL
#define SFLASH_DIE_LOT_LOT_Msk                  0xFFUL
/* SFLASH.DIE_WAFER */
#define SFLASH_DIE_WAFER_WAFER_Pos              0UL
#define SFLASH_DIE_WAFER_WAFER_Msk              0xFFUL
/* SFLASH.DIE_X */
#define SFLASH_DIE_X_X_Pos                      0UL
#define SFLASH_DIE_X_X_Msk                      0xFFUL
/* SFLASH.DIE_Y */
#define SFLASH_DIE_Y_Y_Pos                      0UL
#define SFLASH_DIE_Y_Y_Msk                      0xFFUL
/* SFLASH.DIE_SORT */
#define SFLASH_DIE_SORT_S1_PASS_Pos             0UL
#define SFLASH_DIE_SORT_S1_PASS_Msk             0x1UL
#define SFLASH_DIE_SORT_S2_PASS_Pos             1UL
#define SFLASH_DIE_SORT_S2_PASS_Msk             0x2UL
#define SFLASH_DIE_SORT_S3_PASS_Pos             2UL
#define SFLASH_DIE_SORT_S3_PASS_Msk             0x4UL
#define SFLASH_DIE_SORT_CRI_PASS_Pos            3UL
#define SFLASH_DIE_SORT_CRI_PASS_Msk            0x8UL
#define SFLASH_DIE_SORT_CHI_PASS_Pos            4UL
#define SFLASH_DIE_SORT_CHI_PASS_Msk            0x10UL
#define SFLASH_DIE_SORT_ENG_PASS_Pos            5UL
#define SFLASH_DIE_SORT_ENG_PASS_Msk            0x20UL
/* SFLASH.DIE_MINOR */
#define SFLASH_DIE_MINOR_MINOR_Pos              0UL
#define SFLASH_DIE_MINOR_MINOR_Msk              0xFFUL
/* SFLASH.DIE_DAY */
#define SFLASH_DIE_DAY_MINOR_Pos                0UL
#define SFLASH_DIE_DAY_MINOR_Msk                0xFFUL
/* SFLASH.DIE_MONTH */
#define SFLASH_DIE_MONTH_MINOR_Pos              0UL
#define SFLASH_DIE_MONTH_MINOR_Msk              0xFFUL
/* SFLASH.DIE_YEAR */
#define SFLASH_DIE_YEAR_MINOR_Pos               0UL
#define SFLASH_DIE_YEAR_MINOR_Msk               0xFFUL
/* SFLASH.SAR_TEMP_MULTIPLIER */
#define SFLASH_SAR_TEMP_MULTIPLIER_TEMP_MULTIPLIER_Pos 0UL
#define SFLASH_SAR_TEMP_MULTIPLIER_TEMP_MULTIPLIER_Msk 0xFFFFUL
/* SFLASH.SAR_TEMP_OFFSET */
#define SFLASH_SAR_TEMP_OFFSET_TEMP_OFFSET_Pos  0UL
#define SFLASH_SAR_TEMP_OFFSET_TEMP_OFFSET_Msk  0xFFFFUL
/* SFLASH.CSP_PANEL_ID */
#define SFLASH_CSP_PANEL_ID_DATA32_Pos          0UL
#define SFLASH_CSP_PANEL_ID_DATA32_Msk          0xFFFFFFFFUL
/* SFLASH.LDO_0P9V_TRIM */
#define SFLASH_LDO_0P9V_TRIM_DATA8_Pos          0UL
#define SFLASH_LDO_0P9V_TRIM_DATA8_Msk          0xFFUL
/* SFLASH.LDO_1P1V_TRIM */
#define SFLASH_LDO_1P1V_TRIM_DATA8_Pos          0UL
#define SFLASH_LDO_1P1V_TRIM_DATA8_Msk          0xFFUL
/* SFLASH.IFX_POLICY_SIZE */
#define SFLASH_IFX_POLICY_SIZE_UNUSED_Pos       0UL
#define SFLASH_IFX_POLICY_SIZE_UNUSED_Msk       0xFFFFFFFFUL
/* SFLASH.ICV_ROT_KEY_REVOCATION */
#define SFLASH_ICV_ROT_KEY_REVOCATION_DATA32_Pos 0UL
#define SFLASH_ICV_ROT_KEY_REVOCATION_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.PROT_FW_ROT_KEY_REVOCATION */
#define SFLASH_PROT_FW_ROT_KEY_REVOCATION_DATA32_Pos 0UL
#define SFLASH_PROT_FW_ROT_KEY_REVOCATION_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.OEM_ROT_KEY_REVOCATION */
#define SFLASH_OEM_ROT_KEY_REVOCATION_DATA32_Pos 0UL
#define SFLASH_OEM_ROT_KEY_REVOCATION_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.NV_COUNTER_RAMAPP */
#define SFLASH_NV_COUNTER_RAMAPP_DATA32_Pos     0UL
#define SFLASH_NV_COUNTER_RAMAPP_DATA32_Msk     0xFFFFFFFFUL
/* SFLASH.NV_COUNTER_L1 */
#define SFLASH_NV_COUNTER_L1_DATA8_Pos          0UL
#define SFLASH_NV_COUNTER_L1_DATA8_Msk          0xFFUL
/* SFLASH.NV_COUNTER_L2 */
#define SFLASH_NV_COUNTER_L2_DATA8_Pos          0UL
#define SFLASH_NV_COUNTER_L2_DATA8_Msk          0xFFUL
/* SFLASH.L1_HASH */
#define SFLASH_L1_HASH_DATA8_Pos                0UL
#define SFLASH_L1_HASH_DATA8_Msk                0xFFUL
/* SFLASH.L2_HASH */
#define SFLASH_L2_HASH_DATA8_Pos                0UL
#define SFLASH_L2_HASH_DATA8_Msk                0xFFUL
/* SFLASH.BOOTROW_CRC */
#define SFLASH_BOOTROW_CRC_DATA32_Pos           0UL
#define SFLASH_BOOTROW_CRC_DATA32_Msk           0xFFFFFFFFUL
/* SFLASH.PROV_OEM_COMPLETE */
#define SFLASH_PROV_OEM_COMPLETE_DATA32_Pos     0UL
#define SFLASH_PROV_OEM_COMPLETE_DATA32_Msk     0xFFFFFFFFUL
/* SFLASH.FB_FLAGS */
#define SFLASH_FB_FLAGS_DATA32_Pos              0UL
#define SFLASH_FB_FLAGS_DATA32_Msk              0xFFFFFFFFUL
/* SFLASH.MPN_PROT_FW_KEY */
#define SFLASH_MPN_PROT_FW_KEY_DATA8_Pos        0UL
#define SFLASH_MPN_PROT_FW_KEY_DATA8_Msk        0xFFUL
/* SFLASH.PROT_FW_ENABLED */
#define SFLASH_PROT_FW_ENABLED_DATA32_Pos       0UL
#define SFLASH_PROT_FW_ENABLED_DATA32_Msk       0xFFFFFFFFUL
/* SFLASH.PROT_FW_ROT_KEY_0_HASH */
#define SFLASH_PROT_FW_ROT_KEY_0_HASH_DATA8_Pos 0UL
#define SFLASH_PROT_FW_ROT_KEY_0_HASH_DATA8_Msk 0xFFUL
/* SFLASH.PROT_FW_ROT_KEY_1_HASH */
#define SFLASH_PROT_FW_ROT_KEY_1_HASH_DATA8_Pos 0UL
#define SFLASH_PROT_FW_ROT_KEY_1_HASH_DATA8_Msk 0xFFUL
/* SFLASH.PROT_FW_ENCRYPTION_KEY */
#define SFLASH_PROT_FW_ENCRYPTION_KEY_DATA8_Pos 0UL
#define SFLASH_PROT_FW_ENCRYPTION_KEY_DATA8_Msk 0xFFUL
/* SFLASH.PROT_FW_ADDR */
#define SFLASH_PROT_FW_ADDR_DATA32_Pos          0UL
#define SFLASH_PROT_FW_ADDR_DATA32_Msk          0xFFFFFFFFUL
/* SFLASH.PROT_FW_SIZE */
#define SFLASH_PROT_FW_SIZE_DATA32_Pos          0UL
#define SFLASH_PROT_FW_SIZE_DATA32_Msk          0xFFFFFFFFUL
/* SFLASH.PROT_FW_RW_ADDR */
#define SFLASH_PROT_FW_RW_ADDR_DATA32_Pos       0UL
#define SFLASH_PROT_FW_RW_ADDR_DATA32_Msk       0xFFFFFFFFUL
/* SFLASH.PROT_FW_RW_SIZE */
#define SFLASH_PROT_FW_RW_SIZE_DATA32_Pos       0UL
#define SFLASH_PROT_FW_RW_SIZE_DATA32_Msk       0xFFFFFFFFUL
/* SFLASH.PROT_FW_RAM_ADDR */
#define SFLASH_PROT_FW_RAM_ADDR_DATA32_Pos      0UL
#define SFLASH_PROT_FW_RAM_ADDR_DATA32_Msk      0xFFFFFFFFUL
/* SFLASH.PROT_FW_RAM_SIZE */
#define SFLASH_PROT_FW_RAM_SIZE_DATA32_Pos      0UL
#define SFLASH_PROT_FW_RAM_SIZE_DATA32_Msk      0xFFFFFFFFUL
/* SFLASH.PILO_FREQ_STEP */
#define SFLASH_PILO_FREQ_STEP_STEP_Pos          0UL
#define SFLASH_PILO_FREQ_STEP_STEP_Msk          0xFFFFUL
/* SFLASH.CSDV2_CSD0_ADC_VREF0 */
#define SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P2_Pos 0UL
#define SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P2_Msk 0xFFFFUL
#define SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P6_Pos 16UL
#define SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P6_Msk 0xFFFF0000UL
/* SFLASH.CSDV2_CSD0_ADC_VREF1 */
#define SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_2P1_Pos 0UL
#define SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_2P1_Msk 0xFFFFUL
#define SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_0P8_Pos 16UL
#define SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_0P8_Msk 0xFFFF0000UL
/* SFLASH.CSDV2_CSD0_ADC_VREF2 */
#define SFLASH_CSDV2_CSD0_ADC_VREF2_VREF_HI_LEVELS_2P6_Pos 0UL
#define SFLASH_CSDV2_CSD0_ADC_VREF2_VREF_HI_LEVELS_2P6_Msk 0xFFFFUL
/* SFLASH.PWR_TRIM_WAKE_CTL */
#define SFLASH_PWR_TRIM_WAKE_CTL_WAKE_DELAY_Pos 0UL
#define SFLASH_PWR_TRIM_WAKE_CTL_WAKE_DELAY_Msk 0xFFUL
/* SFLASH.RADIO_LDO_TRIMS */
#define SFLASH_RADIO_LDO_TRIMS_LDO_ACT_Pos      0UL
#define SFLASH_RADIO_LDO_TRIMS_LDO_ACT_Msk      0xFUL
#define SFLASH_RADIO_LDO_TRIMS_LDO_LNA_Pos      4UL
#define SFLASH_RADIO_LDO_TRIMS_LDO_LNA_Msk      0x30UL
#define SFLASH_RADIO_LDO_TRIMS_LDO_IF_Pos       6UL
#define SFLASH_RADIO_LDO_TRIMS_LDO_IF_Msk       0xC0UL
#define SFLASH_RADIO_LDO_TRIMS_LDO_DIG_Pos      8UL
#define SFLASH_RADIO_LDO_TRIMS_LDO_DIG_Msk      0x300UL
/* SFLASH.CPUSS_TRIM_ROM_CTL_ULP */
#define SFLASH_CPUSS_TRIM_ROM_CTL_ULP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_ROM_CTL_ULP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_RAM_CTL_ULP */
#define SFLASH_CPUSS_TRIM_RAM_CTL_ULP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_RAM_CTL_ULP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_ROM_CTL_LP */
#define SFLASH_CPUSS_TRIM_ROM_CTL_LP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_ROM_CTL_LP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_RAM_CTL_LP */
#define SFLASH_CPUSS_TRIM_RAM_CTL_LP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_RAM_CTL_LP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_ROM_CTL_HALF_ULP */
#define SFLASH_CPUSS_TRIM_ROM_CTL_HALF_ULP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_ROM_CTL_HALF_ULP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_RAM_CTL_HALF_ULP */
#define SFLASH_CPUSS_TRIM_RAM_CTL_HALF_ULP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_RAM_CTL_HALF_ULP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_ROM_CTL_HALF_LP */
#define SFLASH_CPUSS_TRIM_ROM_CTL_HALF_LP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_ROM_CTL_HALF_LP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.CPUSS_TRIM_RAM_CTL_HALF_LP */
#define SFLASH_CPUSS_TRIM_RAM_CTL_HALF_LP_DATA32_Pos 0UL
#define SFLASH_CPUSS_TRIM_RAM_CTL_HALF_LP_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.DEVICE_ID_PRIVATE_KEY */
#define SFLASH_DEVICE_ID_PRIVATE_KEY_DATA32_Pos 0UL
#define SFLASH_DEVICE_ID_PRIVATE_KEY_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.FLASH_BOOT_OBJECT_SIZE */
#define SFLASH_FLASH_BOOT_OBJECT_SIZE_DATA32_Pos 0UL
#define SFLASH_FLASH_BOOT_OBJECT_SIZE_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.FLASH_BOOT_APP_ID */
#define SFLASH_FLASH_BOOT_APP_ID_APP_ID_Pos     0UL
#define SFLASH_FLASH_BOOT_APP_ID_APP_ID_Msk     0xFFFFUL
#define SFLASH_FLASH_BOOT_APP_ID_MINOR_VERSION_Pos 16UL
#define SFLASH_FLASH_BOOT_APP_ID_MINOR_VERSION_Msk 0xFF0000UL
#define SFLASH_FLASH_BOOT_APP_ID_MAJOR_VERSION_Pos 24UL
#define SFLASH_FLASH_BOOT_APP_ID_MAJOR_VERSION_Msk 0xF000000UL
/* SFLASH.FLASH_BOOT_ATTRIBUTE */
#define SFLASH_FLASH_BOOT_ATTRIBUTE_DATA32_Pos  0UL
#define SFLASH_FLASH_BOOT_ATTRIBUTE_DATA32_Msk  0xFFFFFFFFUL
/* SFLASH.FLASH_BOOT_N_CORES */
#define SFLASH_FLASH_BOOT_N_CORES_DATA32_Pos    0UL
#define SFLASH_FLASH_BOOT_N_CORES_DATA32_Msk    0xFFFFFFFFUL
/* SFLASH.FLASH_BOOT_VT_OFFSET */
#define SFLASH_FLASH_BOOT_VT_OFFSET_DATA32_Pos  0UL
#define SFLASH_FLASH_BOOT_VT_OFFSET_DATA32_Msk  0xFFFFFFFFUL
/* SFLASH.FLASH_BOOT_CORE_CPUID */
#define SFLASH_FLASH_BOOT_CORE_CPUID_DATA32_Pos 0UL
#define SFLASH_FLASH_BOOT_CORE_CPUID_DATA32_Msk 0xFFFFFFFFUL
/* SFLASH.FLASH_BOOT_CODE */
#define SFLASH_FLASH_BOOT_CODE_DATA_Pos         0UL
#define SFLASH_FLASH_BOOT_CODE_DATA_Msk         0xFFUL


#endif /* _CYIP_SFLASH_H_ */


/* [] END OF FILE */
