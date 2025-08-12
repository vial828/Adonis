/**
  ******************************************************************************
  * @file    app_bt_char.c
  * @author  vincent.he@metextech.com 
  * @date    2024/08/12
  * @version V0.01
  * @brief   Brief description.
  *
  * Description: This file consists of the bt char functions that will help
  *              debugging and developing the applications easier with much
  *              more meaningful information.
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
  * 2024-08-12      V0.01      vincent.he@metextech.com     the first version
  * 
  ******************************************************************************
**/

/******************************************************************************
 *                                INCLUDES
 ******************************************************************************/
#include "app_bt_char_adapter.h"

/* Header file includes */
#include <string.h>
#include "stdlib.h"
#include <inttypes.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_log.h"
#include "cybt_platform_trace.h"
#include "cycfg_gatt_db.h"
#include "cycfg_bt_settings.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_uuid.h"
#include "wiced_memory.h"
#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "cyhal_gpio.h"
#include "cyhal_wdt.h"
#include "wiced_bt_l2c.h"
#include "cyabs_rtos.h"
#include "sm_log.h"
#include "stdio.h"
#include "ota.h"

#include "app_bt_utils.h"
#include "app_bt_char.h"

#include "system_interaction_logic.h"
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

#include "ota_context.h"
#if (OTA_SERVICE_SUPPORT == 1)
/* OTA related header files */
#include "cy_ota_api.h"
#endif //#if (OTA_SERVICE_SUPPORT == 1)


#ifdef OTA_USE_EXTERNAL_FLASH
#include "ota_serial_flash.h"
#endif

//platform
#include "platform_io.h"
//middleware
#include "protocol_usb.h"
#include "data_base_info.h"
//service
#include "system_status.h"
#include "task_system_service.h"

#include "driver_mcu_eeprom.h"
#include "public_typedef.h"
#include "driver_amoled.h"
#include "task_ui_service.h"
#include "session_data_record.h"
#include "event_data_record.h"
#include "err_code.h"
#include "task_charge_service.h"

//mbedtls
#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
// #include "mbedtls/timing.h"
// #include "mbedtls/config.h"
// #include "mbedtls_alt_config.h"
// #include "timing_alt.h"
#include "mbedtls/platform.h"

#include "usr_ecc.h"


//nanopb decode
#include "uimage_decode.h"
#include "uimage_data_record.h"

/* PUBLIC macro -------------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
// 蓝牙广播名称存储于EEP的结构体
typedef struct DevName_t
{
	uint32_t	cal;
	uint8_t		len;
	uint8_t		data[DEV_NAME_LEN_MAX];
} DevName_t;

typedef struct Param_t				// 数据体结构待完善，目前预留128字节，可用字节为124
{
	uint32_t	cal;
	uint8_t		dat[8];		//	uint8_t		dat[16];
} Param_t;

typedef struct Param_Rtc_t
{
    uint32_t magic;
    uint16_t len;
	uint32_t	cal;
	uint32_t	timeStamp;
	uint8_t		resetCnt;
    uint16_t crc;
} Param_Rtc_t;
/* Private Consts ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static bool gbLockMode = 0;
static bool gbEosPrompt, gbCleanPrompt;
static uint8_t gRtcTrusted;			// 复位次数，大于等于2次复位，不可信任
static Ble_Flash_Data_t g_tBleFlashData;
static volatile uint32_t dummy_session_record_number =0;
static volatile uint32_t dummy_event_log_number =0;

/* Private function prototypes -----------------------------------------------*/


/* Private functions --------------------------------------------------------*/
__WEAK bool app_param_read(uint8_t index, uint8_t *pData, uint16_t len){}
__WEAK bool app_param_write(uint8_t index, uint8_t *pData, uint16_t len){}

Ble_Flash_Data_t* get_ble_flash_data_handle(void)
{
	return &g_tBleFlashData;
}



/*
 * Function Name: get_devSN
 * 
 *
 * Function Description: get device SN
 * @brief  The function is
 *
 * @param 
 *
 * @return 
 */
static wiced_bt_gatt_status_t get_devSN(uint8_t *pBuf)
{
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    app_param_read(INDEX_B,g_fdb_b_u.res,sizeof(FDB_are_b1_Info_t));
	memcpy(pBuf,g_fdb_b_u.fdb_b1_t.snNumber,16);
	return WICED_BT_GATT_SUCCESS;
}
/*
 * Function Name: set_devName
 * 
 *
 * Function Description:
 * @brief  The function is set device name
 *
 * @param 
 *
 * @return 
 */
static wiced_bt_gatt_status_t set_devName(const uint8_t *pBuf, uint16_t len)
{
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	DevName_t * devName = (DevName_t *)pBleData->bt_dev_name;

	devName->cal = 0xA55A;
	devName->len = len;
	memcpy(devName->data, pBuf, len);

//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);
	return WICED_BT_GATT_SUCCESS;
}
/*
 * Function Name: get_devName
 * 
 *
 * Function Description:
 * @brief  The function is get device name
 *
 * @param 
 *
 * @return 
 */
static uint16_t get_devName(uint8_t *pBuf)
{
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
	
    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	DevName_t * devName = (DevName_t *)pBleData->bt_dev_name;

	if ((0xA55A == devName->cal) && (devName->len <= DEV_NAME_LEN_MAX) && (devName->len > 0))		// 增加长度检查
	{
		memcpy(pBuf, devName->data, devName->len);
		return devName->len;
	}
	else
	{
		memcpy(pBuf, "glo Hilo ", sizeof("glo Hilo "));
		pBuf += (sizeof("glo Hilo ") - 1);				// 字符串结束符需要--

		uint8_t sn_temp[DEV_NAME_LEN_MAX];				// SN最大长度16字节
		get_devSN(sn_temp);
		memcpy(pBuf, sn_temp+12, 4);					// SN为16位，暂时取最后4位

		return (sizeof("glo Hilo ") - 1 + 4);
	}
}

//将公钥对(X , Y),合并成65字节的公钥
static void convert_ecdsa_key(uint8_t* public_key, uint8_t* key_x, uint8_t* key_y) 
{
    public_key[0] = 0x04; // Prepend 0x04 to indicate compressed format
    memcpy(public_key + 1, key_x, 32);
    memcpy(public_key + 33, key_y, 32);
}

//将65字节的公钥，拆分为公钥对(X, Y)
static void split_ecdsa_pubkey(uint8_t* public_key, uint8_t* key_x, uint8_t* key_y) 
{
    // 复制前32字节到X
    memcpy(key_x, public_key + 1, 32);
    // 复制剩余字节到Y
    memcpy(key_y, public_key + 33, 32);
}

static void dump_pubkey( const char *title, mbedtls_ecdsa_context *key)
{
    unsigned char buf[300];
    size_t len;

    if( mbedtls_ecp_point_write_binary( &key->grp, &key->Q,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf ) != 0 )
    {
        sm_log(SM_LOG_DEBUG, "internal error\n");
        return;
    }

	sm_log(SM_LOG_DEBUG, "\n  + %s: ", title);
    print_array(&buf[0], len);
}

static void dump_pubkey_Getpubkey( const char *title, mbedtls_ecdsa_context *key, uint8_t* public_key, size_t *public_key_len)
{
    unsigned char buf[300];
    size_t len;

    if( mbedtls_ecp_point_write_binary( &key->grp, &key->Q,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf ) != 0 )
    {
        sm_log(SM_LOG_DEBUG, "internal error\n");
        return;
    }
    
	if (public_key == NULL)
	{
		return;
	}

	memcpy(public_key, &buf[0], len);

	sm_log(SM_LOG_DEBUG, "\n  + %s: ", title);
    print_array(&buf[0], len);
}

/* Exported functions --------------------------------------------------------*/
uint32_t u32_endian_swap(uint32_t *pData)
{
	uint8_t *pU8;
	uint8_t u8Data;

	pU8 = (uint8_t *)pData;

	u8Data = *pU8;
	*pU8 = *(pU8 + 3);
	*(pU8 + 3) = u8Data;

	u8Data = *(pU8 + 1);
	*(pU8 + 1) = *(pU8 + 2);
	*(pU8 + 2) = u8Data;
	return *pData;
}

uint16_t u16_endian_swap(uint16_t *pData)
{
	uint8_t *pU8;
	uint8_t u8Data;

	pU8 = (uint8_t *)pData;

	u8Data = *pU8;
	*pU8 = *(pU8 + 1);
	*(pU8 + 1) = u8Data;
	return *pData;
}

/*
 * Function Name: bt_adapter_md5_calculate
 * 
 *
 * Function Description:
 * @brief  
 * calculate the md5 value upon one continous memory/flash aera, there is no restriction on how many bytes the data are.
 * The message digest algorithm method is a standard and the value can be calculated by mbedtls library.
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_md5_calculate(const unsigned char *pBuf, uint32_t memory_addr, size_t len, uint16_t repeat_cnt, unsigned char hash_value[])
{
    int ret = 0;

    sm_log(SM_LOG_DEBUG, "%s ,pBuf:%p ,memory_addr:%ld ,len:%ld \n", __FUNCTION__, pBuf, memory_addr, len);

    // unsigned char *buf;
    mbedtls_md5_context md5_ctx;

    // buf = mbedtls_calloc( 1024, sizeof(unsigned char) );

	if (len < 1)//len verify
	{
		return WICED_BT_GATT_ERROR;
	}

    /**
     * md5 initialize
     */
    mbedtls_md5_init( &md5_ctx );

    /* MD5 Start*/
    if( ( ret = mbedtls_md5_starts_ret( &md5_ctx ) ) != 0 )
    {
        mbedtls_md5_free( &md5_ctx );
        // mbedtls_free( buf );

		return WICED_BT_GATT_ERROR;
    }

	if (NULL != pBuf)//calcu ram buf data
    {
        for (uint16_t i=0; i<repeat_cnt; i++)
        {
            ret = mbedtls_md5_update_ret( &md5_ctx, pBuf, len);//update buf data
            if( ret != 0 )
            {
                mbedtls_md5_free( &md5_ctx );
                // mbedtls_free( buf );

                return WICED_BT_GATT_ERROR;
            }
        }
    }
    else if (memory_addr) //calcu ext flash buf data ,还需添加地址范围限制判断！
    {
        //to be continue
		if ((UIMAGE_END_ADDR <= memory_addr)||(memory_addr < UIMAGE_START_ADDR))
		{
			mbedtls_md5_free( &md5_ctx );
			// mbedtls_free( buf );

            return WICED_BT_GATT_ERROR;
		}

		uint8_t r_buf[1024]={0};
		uint16_t r_len = 0;
		uint32_t offset = 0;

		ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
		Qspiflash_t ptQspiflash;

		do
		{
			if (1024 <= (len - offset))
			{
				r_len = 1024;
			}
			else
			{
				r_len = len - offset;
			}

			ptQspiflash.addr = memory_addr+offset;//UI IMAGE SAVED ADDR
			ptQspiflash.data = r_buf;
			ptQspiflash.len = r_len;
			qspiflashDev->read( (uint8_t*)&ptQspiflash, r_len);

			sm_log(SM_LOG_DEBUG, "uimage_readAddr:0x%08x r_len(%d)\n", ptQspiflash.addr, r_len);

			ret = mbedtls_md5_update_ret( &md5_ctx, r_buf, r_len);//update buf data
			if( ret != 0 )
			{
				mbedtls_md5_free( &md5_ctx );
				// mbedtls_free( buf );

				return WICED_BT_GATT_ERROR;
			}

			offset += r_len;
			memset(r_buf, 0, sizeof(r_buf));
        	sys_task_security_ms_delay(10, TASK_NONE); // 防止大量数据操作，造成堵塞
		}while(offset < len);
    }
    else
    {
        mbedtls_md5_free( &md5_ctx );
        // mbedtls_free( buf );

		return WICED_BT_GATT_ERROR;
    }

    if( ( ret = mbedtls_md5_finish_ret( &md5_ctx, hash_value ) ) != 0 )
    {
        mbedtls_md5_free( &md5_ctx );
        // mbedtls_free( buf );

		return WICED_BT_GATT_ERROR;
    }

    mbedtls_md5_free( &md5_ctx );
    // mbedtls_free( buf );

    return( ret );
}

/*
 * Function Name: bt_adapter_sha256_calculate
 * 
 *
 * Function Description:
 * @brief  
 * calculate the sha256 value upon one continous memory/flash aera, there is no restriction on how many bytes the data are.
 * The SHA-256 method is a standard and the value can be calculated by mbedtls library.
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_sha256_calculate(const unsigned char *pBuf, uint32_t memory_addr, size_t len, uint16_t repeat_cnt, unsigned char hash_value[])
{
    int ret = 0;

    sm_log(SM_LOG_DEBUG, "%s ,pBuf:%p ,memory_addr:%ld ,len:%ld \n", __FUNCTION__, pBuf, memory_addr, len);

    // unsigned char *buf;
    mbedtls_sha256_context sha256_ctx;

    // buf = mbedtls_calloc( 1024, sizeof(unsigned char) );

	if ((len < 1)||(hash_value == NULL))//len verify
	{
		return WICED_BT_GATT_ERROR;
	}

    /**
     * sha256 initialize
     */
    mbedtls_sha256_init( &sha256_ctx );

    /* SHA-256 */
    if( ( ret = mbedtls_sha256_starts_ret( &sha256_ctx, 0 ) ) != 0 )
    {
        mbedtls_sha256_free( &sha256_ctx );
        // mbedtls_free( buf );

		return WICED_BT_GATT_ERROR;
    }

	if (NULL != pBuf)//calcu ram buf data
    {
        for (uint16_t i=0; i<repeat_cnt; i++)
        {
            ret = mbedtls_sha256_update_ret( &sha256_ctx, pBuf, len);//update buf data
            if( ret != 0 )
            {
                mbedtls_sha256_free( &sha256_ctx );
                // mbedtls_free( buf );

                return WICED_BT_GATT_ERROR;
            }
		}
    }
    else if (memory_addr) //calcu ext flash buf data ,还需添加地址范围限制判断！
    {
        //to be continue
		if ((UIMAGE_END_ADDR <= memory_addr)||(memory_addr < UIMAGE_START_ADDR))
		{
			mbedtls_sha256_free( &sha256_ctx );
			// mbedtls_free( buf );

            return WICED_BT_GATT_ERROR;
		}

		uint8_t r_buf[1024]={0};
		uint16_t r_len = 0;
		uint32_t offset = 0;
		uint16_t pkg_num = 0;

		ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
		Qspiflash_t ptQspiflash;

		do
		{
			if (1024 <= (len - offset))
			{
				r_len = 1024;
			}
			else
			{
				r_len = len - offset;
			}

			ptQspiflash.addr = memory_addr+offset;//UI IMAGE SAVED ADDR
			ptQspiflash.data = r_buf;
			ptQspiflash.len = r_len;
			qspiflashDev->read( (uint8_t*)&ptQspiflash, r_len);

			sm_log(SM_LOG_DEBUG, "uimage_readAddr:0x%08x r_len(%d) pkg_num(%d)\n", ptQspiflash.addr, r_len, ++pkg_num);

			#if 0
			sm_log(SM_LOG_DEBUG, "r_buf(%ld): \n", r_len);
			for(uint16_t i=0; i<r_len; i++)
			{
				sm_log(SM_LOG_DEBUG, " %02x", r_buf[i]);
			}
			sm_log(SM_LOG_DEBUG, "\n");
			#endif

			ret = mbedtls_sha256_update_ret( &sha256_ctx, r_buf, r_len);//update buf data
			if( ret != 0 )
			{
				mbedtls_sha256_free( &sha256_ctx );
				// mbedtls_free( buf );

				return WICED_BT_GATT_ERROR;
			}

			offset += r_len;
			memset(r_buf, 0, sizeof(r_buf));
        	sys_task_security_ms_delay(10, TASK_NONE); // 防止大量数据操作，造成堵塞
		}while(offset < len);
    }
    else
    {
        mbedtls_sha256_free( &sha256_ctx );
        // mbedtls_free( buf );

		return WICED_BT_GATT_ERROR;
    }

    if( ( ret = mbedtls_sha256_finish_ret( &sha256_ctx, hash_value ) ) != 0 )
    {
        mbedtls_sha256_free( &sha256_ctx );
        // mbedtls_free( buf );

		return WICED_BT_GATT_ERROR;
    }

    mbedtls_sha256_free( &sha256_ctx );
    // mbedtls_free( buf );

    return( ret );
}

/*
 * Function Name: bt_adapter_ecdsa_sign
 * 
 *
 * Function Description:
 * @brief  
 * Provide signature data, generate public key pair (X,Y), generate signature [R, S]
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_ecdsa_sign(uint8_t* hash, size_t hlen,
    				uint8_t* key_x, uint8_t* key_y,
    				uint8_t* sign_r, uint8_t* sign_s)
{
    int ret = 1;

    mbedtls_ecdsa_context ctx_sign;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_mpi r, s;

    unsigned char public_key_buffer[65];

    size_t public_key_size;
    size_t rlen, slen;

    mbedtls_ecdsa_init(&ctx_sign);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    /*
     * Generate a key pair for signing
     */
    sm_log(SM_LOG_DEBUG, "\n  . Seeding the random number generator...");
    

    mbedtls_entropy_init(&entropy);
    // if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
    //     NULL, 0)) != 0)
    // {
    //     sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
    //     goto exit;
    // }

    sm_log(SM_LOG_DEBUG, " ok\n  . Generating key pair...");
    

    if ((ret = mbedtls_ecdsa_genkey(&ctx_sign, MBEDTLS_ECP_DP_SECP256R1,
        bt_adapter_mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_ecdsa_genkey returned %d\n", ret);
        goto exit;
    }

    sm_log(SM_LOG_DEBUG, " ok (key size: %d bits)\n", (int)ctx_sign.grp.pbits);

    dump_pubkey_Getpubkey("  + Public key: ", &ctx_sign, public_key_buffer, &public_key_size);

    split_ecdsa_pubkey(public_key_buffer, key_x, key_y);

    /*
     * Sign message hash
     */
    sm_log(SM_LOG_DEBUG, "  . Signing message hash...");
    

    if ((ret = mbedtls_ecdsa_sign(&ctx_sign.grp, &r, &s, &ctx_sign.d,
        hash, hlen,
        bt_adapter_mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_ecdsa_sign returned %d\n", ret);
        goto exit;
    }

    rlen = mbedtls_mpi_size(&r);
    slen = mbedtls_mpi_size(&s);

    sm_log(SM_LOG_DEBUG, " rlen: %d slen: %d\n", rlen, slen);

    mbedtls_mpi_write_binary(&r, sign_r, rlen);
    mbedtls_mpi_write_binary(&s, sign_s, slen);

    sm_log(SM_LOG_DEBUG, " ok \n");

    return ret;
exit:
    mbedtls_ecdsa_free(&ctx_sign);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return WICED_BT_GATT_ERROR;
}

/*
 * Function Name: bt_adapter_ecdsa_sign_verify
 * 
 *
 * Function Description:
 * @brief  
 * Provide Hash data, and public key pair (X,Y), and signature [R, S]
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_ecdsa_sign_verify(uint8_t* hash, size_t hlen,
                        uint8_t* key_x, uint8_t* key_y,
                        uint8_t* sign_r, uint8_t* sign_s)
{
    int ret = 1;

    mbedtls_ecdsa_context  ctx_verify;
    mbedtls_mpi r, s;

    size_t public_key_size;

    unsigned char public_key_buffer[65]; // 公钥的最大大小，65字节（包括结束字节）

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    convert_ecdsa_key(public_key_buffer, key_x, key_y);

    mbedtls_mpi_read_binary(&r, sign_r, 32);
    mbedtls_mpi_read_binary(&s, sign_s, 32);

    /*
     * Transfer public information to verifying context
     *
     * We could use the same context for verification and signatures, but we
     * chose to use a new one in order to make it clear that the verifying
     * context only needs the public key (Q), and not the private key (d).
     */
    sm_log(SM_LOG_DEBUG, "  . Preparing verification context...");
    

    mbedtls_ecdsa_init(&ctx_verify);
    mbedtls_ecp_group_load(&ctx_verify.grp, MBEDTLS_ECP_DP_SECP256R1);

    public_key_size = sizeof(public_key_buffer);

    // 给ECDSA上下文写入公钥
    if ((ret = mbedtls_ecp_point_read_binary(&ctx_verify.grp, &ctx_verify.Q, public_key_buffer, public_key_size)) != 0)
    {
        sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_ecp_point_read_binary returned %d\n", ret);
        goto exit;
    }

    dump_pubkey(" \n + Verifying Public key: ", &ctx_verify);

    /*
     * Verify signature
     */
    sm_log(SM_LOG_DEBUG, " ok\n  . Verifying signature...");
    

    if ((ret = mbedtls_ecdsa_verify(&ctx_verify.grp,
        hash, hlen, &ctx_verify.Q, &r, &s)) != 0)
    {
        sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_ecdsa_verify returned %d\n", ret);
        goto exit;
    }
    sm_log(SM_LOG_DEBUG, " ok\n");

    return ret;
exit:
    mbedtls_ecdsa_free(&ctx_verify);
    return WICED_BT_GATT_ERROR;
}

/*
 * Function Name: bt_adapter_ecdsa_message_sign_verify
 * 
 *
 * Function Description:
 * @brief  
 * Provide message data, and public key pair (X,Y), and signature [R, S]
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_ecdsa_message_sign_verify(uint8_t *message, size_t message_size, 
								uint8_t* public_key, uint8_t* signature)
{
    int ret = 0;
    unsigned char hash[32];
    unsigned char key_X[32], key_Y[32];
    unsigned char sign_R[32], sign_S[32];

    size_t hlen;

    /*
     * Compute message hash
     */
    sm_log(SM_LOG_DEBUG, "%s  . Computing message hash...", __FUNCTION__);
    

    if ((ret = mbedtls_sha256_ret(message, message_size, hash, 0)) != 0)
    {
        sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_sha256_ret returned %d\n", ret);
    }
    hlen = sizeof(hash);
    sm_log(SM_LOG_DEBUG, " ok\n");
    
    sm_log(SM_LOG_DEBUG, "  + message Hash value: ");
    print_array(&hash[0], 32);

    // 复制前32字节到X
    memcpy(key_X, public_key + 0, 32);
    // 复制剩余字节到Y
    memcpy(key_Y, public_key + 32, 32);
	
    // 复制前32字节到X
    memcpy(sign_R, signature + 0, 32);
    // 复制剩余字节到Y
    memcpy(sign_S, signature + 32, 32);

    sm_log(SM_LOG_DEBUG, "\n  + hash: ");
    print_array(&hash[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + key_X: ");
    print_array(&key_X[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + key_Y: ");
    print_array(&key_Y[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + sign_R: ");
    print_array(&sign_R[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + sign_S: ");
    print_array(&sign_S[0], 32);
    
    sm_log(SM_LOG_DEBUG, "  \n. Sign_Verify Start ...\n");
    if ((ret = bt_adapter_ecdsa_sign_verify(hash, hlen, key_X, key_Y, sign_R, sign_S)) == 0)
    {
        sm_log(SM_LOG_DEBUG, " bt_adapter_ecdsa_sign_verify successful!\n");
    }

    return ret;
}

int bt_adapter_ecdsa_sign_verify_test(void)
{
    int ret = 0;
    unsigned char message[] = "Hello world!";
    unsigned char hash[32];
    unsigned char key_X[32], key_Y[32];
    unsigned char sign_R[32], sign_S[32];

    size_t hlen;

    /*
     * Compute message hash
     */
    sm_log(SM_LOG_DEBUG, "%s  . Computing message hash...", __FUNCTION__);
    

    if ((ret = mbedtls_sha256_ret(message, (sizeof(message)-1), hash, 0)) != 0)
    {
        sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_sha256_ret returned %d\n", ret);
    }
    hlen = sizeof(hash);
    sm_log(SM_LOG_DEBUG, " ok\n");
    
    sm_log(SM_LOG_DEBUG, "  + default Hash value: ");
    print_array(&hash[0], 32);

    sm_log(SM_LOG_DEBUG, "  . Sign_Test Start ...\n");
    if ((ret = bt_adapter_ecdsa_sign(hash, hlen, key_X, key_Y, sign_R, sign_S)) == 0)
    {
        sm_log(SM_LOG_DEBUG, " bt_adapter_ecdsa_sign successful!\n");
    }
    sm_log(SM_LOG_DEBUG, "  . Sign_Test end ...\n");
    sm_log(SM_LOG_DEBUG, "\n  + hash: ");
    print_array(&hash[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + key_X: ");
    print_array(&key_X[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + key_Y: ");
    print_array(&key_Y[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + sign_R: ");
    print_array(&sign_R[0], 32);
    sm_log(SM_LOG_DEBUG, "\n  + sign_S: ");
    print_array(&sign_S[0], 32);
    
    sm_log(SM_LOG_DEBUG, "  \n. Sign_Verify Start ...\n");
    if ((ret = bt_adapter_ecdsa_sign_verify(hash, hlen, key_X, key_Y, sign_R, sign_S)) == 0)
    {
        sm_log(SM_LOG_DEBUG, " bt_adapter_ecdsa_sign_verify successful!\n");
    }

    return ret;
}

/*
 * Function Name: bt_adapter_uECC_message_sign_verify
 * 
 *
 * Function Description:
 * @brief  
 * Provide message data, and public key pair (X,Y), and signature [R, S]
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_uECC_message_sign_verify(uint8_t *message, size_t message_size, 
								uint8_t* public_key, uint8_t* signature)
{
	const struct uECC_Curve_t* curves;
    int ret = 0;
    unsigned char hash[32];

    size_t hlen;

    /*
     * Compute message hash
     */
    sm_log(SM_LOG_DEBUG, "\n\n%s  . Computing message(%d) hash...", __FUNCTION__, message_size);
    
    hlen = sizeof(hash);

    struct sha256 s;
    sha256_init(&s);
    sha256_update(&s, message, message_size);
    sha256_final(&s, hash);
    sm_log(SM_LOG_DEBUG, "  + sha256_init Hash value: ");
    print_array(&hash[0], hlen);
    sm_log(SM_LOG_DEBUG, " ok\n");

    // if ((ret = mbedtls_sha256_ret(message, message_size, hash, 0)) != 0)
    // {
    //     sm_log(SM_LOG_DEBUG, " failed\n  ! mbedtls_sha256_ret returned %d\n", ret);
    // }
    // sm_log(SM_LOG_DEBUG, "  + mbedtls_sha256_ret Hash value: ");
    // print_array(&hash[0], hlen);
    // sm_log(SM_LOG_DEBUG, " ok\n");

    sm_log(SM_LOG_DEBUG, "\n  + public_key: ");
    print_array(&public_key[0], 64);

    sm_log(SM_LOG_DEBUG, "\n  + signature: ");
    print_array(&signature[0], 64);
    
    sm_log(SM_LOG_DEBUG, "  \n. Sign_Verify Start ...\n");

    curves = uECC_secp256r1();
    if ((ret = uECC_verify(public_key, hash, hlen, signature, curves)) == 1)
    {
        sm_log(SM_LOG_DEBUG, "\n %s successful!\n", __FUNCTION__);
    }
	else
	{
        sm_log(SM_LOG_DEBUG, "\n %s failure: %d\n", __FUNCTION__, ret);
	}
    
    return ret;
}

/*
 * Function Name: bt_adapter_uECC_sha256_sign_verify
 * 
 *
 * Function Description:
 * @brief  
 * Provide message data, and public key pair (X,Y), and signature [R, S]
 *
 * @param  None
 *
 * @return ret (0 or WICED_BT_GATT_ERROR)
 *
*/
int bt_adapter_uECC_sha256_sign_verify(uint8_t *hash, size_t hlen, 
								uint8_t* public_key, uint8_t* signature)
{
	const struct uECC_Curve_t* curves;
    int ret = 0;


    sm_log(SM_LOG_DEBUG, "\n\n %s \n", __FUNCTION__);

    sm_log(SM_LOG_DEBUG, "  + mbedtls_sha256_ret Hash value: ");
    print_array(&hash[0], hlen);
    sm_log(SM_LOG_DEBUG, " ok\n");

    sm_log(SM_LOG_DEBUG, "\n  + public_key: ");
    print_array(&public_key[0], 64);

    sm_log(SM_LOG_DEBUG, "\n  + signature: ");
    print_array(&signature[0], 64);
    
    sm_log(SM_LOG_DEBUG, "  \n. Sign_Verify Start ...\n");

    curves = uECC_secp256r1();
    if ((ret = uECC_verify(public_key, hash, hlen, signature, curves)) == 1)
    {
        sm_log(SM_LOG_DEBUG, " %s successful!\n", __FUNCTION__);
    }
	else
	{
        sm_log(SM_LOG_DEBUG, " %s failure: %d\n", __FUNCTION__, ret);
	}
    
    return ret;
}

/*
 * Function Name: bt_adapter_device_information_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_device_information_get(session_service_device_information_char_read_t *device_information)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    uint8_t pIapBuf[32] = {0};
	char str[2] = {0};

//    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
	if (NULL == device_information)
    {
        return WICED_BT_GATT_ERROR;
    }

    //update device information
	/* ----------------------------------------------------------APP---------------------------------------------------------*/
	// memcpy(pIapBuf, PROTOCOL_APP_VERSION, strlen(PROTOCOL_APP_VERSION));
	// str[0] = pIapBuf[0];
	// int app_ver_num = atoi(str); // 字符串转换为整数
	// sm_log(SM_LOG_INFO, "FW_VERSION_MAJOR: %ld\n", app_ver_num);
    device_information->firmware_major_version = APP_VERSION_MAJOR;//app_ver_num;
	
	// str[0] = pIapBuf[2];
	// app_ver_num = atoi(str);    // 字符串转换为整数
	// sm_log(SM_LOG_INFO, "FW_VERSION_MIN: %ld\n", app_ver_num);
    device_information->firmware_minor_version = APP_VERSION_MINOR;//app_ver_num;
	
	// str[0] = pIapBuf[4];
	// str[1] = pIapBuf[5];
	// app_ver_num = atoi(str);    // 字符串转换为整数
	// sm_log(SM_LOG_INFO, "SW_REVISION_PATCH: %ld\n", app_ver_num);
    device_information->sw_revision_patch = APP_VERSION_BUILD;//app_ver_num;
    u16_endian_swap(&device_information[0].sw_revision_patch);

	/* ---------------------------------------------------------HARDWARE----------------------------------------------------------*/
	memset(pIapBuf, 0, sizeof(pIapBuf));
	memset(str, 0, sizeof(str));
	memcpy(pIapBuf, PROTOCOL_HARDWARE_VERSION, strlen(PROTOCOL_HARDWARE_VERSION));
	str[0] = pIapBuf[1];
	int hardware_ver_num = atoi(str); // 字符串转换为整数
	sm_log(SM_LOG_INFO, "BOARD_VERSION: %ld str: %s pIapBuf: %s\n", hardware_ver_num, str, pIapBuf);
    device_information->board_classification = hardware_ver_num; //(HW Version)
    u16_endian_swap(&device_information[0].board_classification);

	/* ---------------------------------------------------------BOOTLOADER----------------------------------------------------------*/
	memset(pIapBuf, 0, sizeof(pIapBuf));
    uint32_t addr = MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - 1*MCU_FLASH_ROW_SIZE;
#ifdef DEF_BLE_APP_GROUP_EN
        addr = MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - 3*MCU_FLASH_ROW_SIZE; // BUG 1925137
#else
        addr = MCU_FLASH_START_ADDR + MCU_FLASH_TOTAL_SIZE - 1*MCU_FLASH_ROW_SIZE; // BUG 1925137
#endif
    char ver[] = "NA\n";
    
	ptIoDev mcuFlashDev = io_dev_get_dev(DEV_MCU_FLASH);
    McuFlashDataType_t  t_temp;
    t_temp.addr = addr;
    t_temp.pData = pIapBuf;
    t_temp.size = sizeof(pIapBuf);
    mcuFlashDev->read( (uint8_t*)&t_temp, t_temp.size);

    sm_log(SM_LOG_INFO, "Boot Version: %s\n", pIapBuf);

	if ((strlen(pIapBuf)-1) == 5) //ones
    {
		// char str[1] = {0};
		// str[0] = pIapBuf[4];
		str[0] = pIapBuf[0];          // 获取最高字节，比如2.0.0， 那么取 “2”. bug1968366
    	int boot_ver_num = atoi(str); // 字符串转换为整数
    	sm_log(SM_LOG_INFO, "boot_ver_num: %ld\n", boot_ver_num);

		device_information->bootloader_version = boot_ver_num;
    }
    else if ((strlen(pIapBuf)-1) == 6) //tens
	{
		// char str[2] = {0};
		str[0] = pIapBuf[4];
		str[1] = pIapBuf[5];

    	int boot_ver_num = atoi(str); // 字符串转换为整数
    	sm_log(SM_LOG_INFO, "boot_ver_num: %ld\n", boot_ver_num);

		device_information->bootloader_version = boot_ver_num;
	}
	else
	{
        device_information->bootloader_version = 0xFF;
	}

	/* ---------------------------------------------------------SN GET----------------------------------------------------------*/
	uint8_t sn_temp[DEV_NAME_LEN_MAX];							// SN最大长度16字节
	memset(sn_temp, 0, DEV_NAME_LEN_MAX);
	
	status = get_devSN(sn_temp);
    memcpy(device_information->serial_number, sn_temp, DEV_NAME_LEN_MAX);	// SN为16Bytes
//	sm_log(SM_LOG_INFO, "%s\n", (sn_temp));

    return status; 
}

/*
 * Function Name: bt_adapter_device_information_set_adv_name
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_device_information_set_adv_name(const uint8_t *pBuf, uint16_t len)
{
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
	if ((NULL == pBuf) || len > DEV_NAME_LEN_MAX || len < 1)
	{
		return WICED_BT_GATT_ERROR;
	}
    else
    {
		return set_devName(pBuf, len);
	}
}

/*
 * Function Name: bt_adapter_device_information_get_adv_name
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_device_information_get_adv_name(uint8_t *pBuf, uint16_t *len)
{
	sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
	*len = get_devName(pBuf);

	memset(app_gap_device_name, 0, sizeof(app_gap_device_name));
	memcpy(app_gap_device_name, pBuf, *len);

    return WICED_BT_GATT_SUCCESS; 
}

/*
 * Function Name: bt_adapter_time_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_time_set(const uint8_t *pBuf, uint16_t len)
{
	uint32_t timestamp;
	uint32_t *pData;
	sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (NULL == pBuf || len != 4)
    {
        return WICED_BT_GATT_ERROR;
    }

    #if 0
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	pData = (uint32_t *)pBuf;
	timestamp = *pData;
	u32_endian_swap(&timestamp);

	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->write((uint8_t *)&timestamp, len);
	
//	gRtcTrusted = 0;					// 时间可信任; 先清除，后写入
	gRtcTrusted = 2;					// 复位后为零，不可信任
	set_time_stamp_to_flash();

    return WICED_BT_GATT_SUCCESS; 
}

/*
 * Function Name: bt_adapter_time_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_time_get(session_service_time_char_write_read_t *timestamp)
{
	uint32_t ts;
	sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
    if (NULL == timestamp) 
    {
        return WICED_BT_GATT_ERROR;
    }

	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->read((uint8_t *)&ts, 4);

	u32_endian_swap(&ts);

	timestamp->time_value = ts;

    return WICED_BT_GATT_SUCCESS; 
}

uint8_t Get_Rtc_Trusted(void)
{
//	if (gRtcTrusted > 1)	return 1;
//	else					return 0;

	if ((gRtcTrusted == 1) || (gRtcTrusted == 2))	return 0;
	else											return 1;
}

/*
 * Function Name: bt_adapter_battery_power_remaining_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_battery_power_remaining_get(session_service_battery_power_remaining_char_read_ntf_t *battery_power_remaining)
{
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (battery_power_remaining == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

//	sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	// 获取电池电量
	MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();

//	sm_log(SM_LOG_ERR, "battery_power_value: %d chg.state:%d chg.partab:%d sessions_remaining:%d\n", p_tMontorDataInfo->bat.soc, p_tMontorDataInfo->chg.state, p_tMontorDataInfo->chg.partab, p_tMontorDataInfo->bat.soc/5);
	sm_log(SM_LOG_ERR, "battery_power_value: %d chg.state:%d chg.partab:%d sessions_remaining:%d\n", p_tMontorDataInfo->bat.soc, p_tMontorDataInfo->state, p_tMontorDataInfo->partab, p_tMontorDataInfo->bat.soc/5);
	battery_power_remaining->battery_power_value = p_tMontorDataInfo->bat.remap_soc;//update ext battery variables
//	if (p_tMontorDataInfo->chg.state == 1)
//	{
//		battery_power_remaining->charging_state = p_tMontorDataInfo->chg.state;
//	}
//	else
//	{
//		battery_power_remaining->charging_state = 0;//unplug
//	}
//	battery_power_remaining->charging_state = p_tMontorDataInfo->chg.state;
	if (p_tMontorDataInfo->state == CHG_STATE_CHARGE || p_tMontorDataInfo->state == CHG_STATE_FULL)
	{
		battery_power_remaining->charging_state = 1;		// normal Charging
	}
	else
	{
		battery_power_remaining->charging_state = 0;		// not Charging
	}

// #define SOC_SESSIONS_5	(23) // 23~27%
// #define SOC_SESSIONS_4	(19) // 19~22%
// #define SOC_SESSIONS_3	(15) // 15~18%
// #define SOC_SESSIONS_2	(10) // 10~14%
// #define SOC_SESSIONS_1	(5)  // 05~09%

//#define SESSION_CNT_5 99
//#define SESSION_CNT_4 80
//#define SESSION_CNT_3 60
//#define SESSION_CNT_2 40
//#define SESSION_CNT_1 20
//#define SESSION_CNT_0 0

	if (get_battery_eol_status())		// EOL电量修正	Bug 2032660
	{
		uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;
		uint8_t sessionCnt = 0;
		if (batteryLevel > 99) {
			sessionCnt = 5;
		} else if (batteryLevel > 80) {
			sessionCnt = 4;
		} else if (batteryLevel > 60) {
			sessionCnt = 3;
		} else if (batteryLevel > 40) {
			sessionCnt = 2;
		} else if (batteryLevel > 20) {
			sessionCnt = 1;
		} else {
			sessionCnt = 0;
		}
		battery_power_remaining->sessions_remaining = sessionCnt;
	}
	else
	{
		if (p_tMontorDataInfo->bat.remap_soc >= 28)
		{
			battery_power_remaining->sessions_remaining = p_tMontorDataInfo->bat.remap_soc/5;
		}
		else
		{
			if (p_tMontorDataInfo->bat.remap_soc >= 23) {
				battery_power_remaining->sessions_remaining = 5;
			} else if (p_tMontorDataInfo->bat.remap_soc >= 19) {
				battery_power_remaining->sessions_remaining = 4;
			} else if (p_tMontorDataInfo->bat.remap_soc >= 15) {
				battery_power_remaining->sessions_remaining = 3;
			} else if (p_tMontorDataInfo->bat.remap_soc >= 10) {
				battery_power_remaining->sessions_remaining = 2;
			} else if (p_tMontorDataInfo->bat.remap_soc >= 5) {
				battery_power_remaining->sessions_remaining = 1;
			} else {
				battery_power_remaining->sessions_remaining = 0;
			}
		}
	}

	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_battery_power_remaining_change_happened
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return STATUS  (true or false)
 */
bool bt_adapter_battery_power_remaining_change_happened(session_service_battery_power_remaining_char_read_ntf_t *battery_power_remaining)
{
	static session_service_battery_power_remaining_char_read_ntf_t sta_buf = {0};
	session_service_battery_power_remaining_char_read_ntf_t sta_now;

	// 获取电池电量
	MonitorDataInfo_t *p_tMontorDataInfo = get_monitor_data_info_handle();
	
	// sm_log(SM_LOG_ERR, "battery_power_value: %d chg.state:%d chg.partab:%d sessions_remaining:%d\n", p_tMontorDataInfo->bat.soc, p_tMontorDataInfo->chg.state, p_tMontorDataInfo->chg.partab, p_tMontorDataInfo->bat.soc/5);
		
	sta_now.battery_power_value = p_tMontorDataInfo->bat.remap_soc;//update ext battery variables
//	sta_now.charging_state = p_tMontorDataInfo->chg.state;
//	sta_now.charging_state = p_tMontorDataInfo->state;
	if (p_tMontorDataInfo->state == CHG_STATE_CHARGE || p_tMontorDataInfo->state == CHG_STATE_FULL)
	{
		sta_now.charging_state = 1;		// normal Charging
	}
	else
	{
		sta_now.charging_state = 0;		// not Charging
	}

// #define SOC_SESSIONS_5	(23) // 23~27%
// #define SOC_SESSIONS_4	(19) // 19~22%
// #define SOC_SESSIONS_3	(15) // 15~18%
// #define SOC_SESSIONS_2	(10) // 10~14%
// #define SOC_SESSIONS_1	(5)  // 05~09%

	if (get_battery_eol_status())		// EOL电量修正	Bug 2032660
	{
		uint8_t batteryLevel = (uint8_t)p_tMontorDataInfo->bat.remap_soc;
		uint8_t sessionCnt = 0;
		if (batteryLevel > 99) {
			sessionCnt = 5;
		} else if (batteryLevel > 80) {
			sessionCnt = 4;
		} else if (batteryLevel > 60) {
			sessionCnt = 3;
		} else if (batteryLevel > 40) {
			sessionCnt = 2;
		} else if (batteryLevel > 20) {
			sessionCnt = 1;
		} else {
			sessionCnt = 0;
		}
		sta_now.sessions_remaining = sessionCnt;
	}
	else if (p_tMontorDataInfo->bat.remap_soc >= 28)
	{
		sta_now.sessions_remaining  = p_tMontorDataInfo->bat.remap_soc/5;
	}
	else
	{
		if (p_tMontorDataInfo->bat.remap_soc >= 23) {
			sta_now.sessions_remaining  = 5;
		} else if (p_tMontorDataInfo->bat.remap_soc >= 19) {
			sta_now.sessions_remaining  = 4;
		} else if (p_tMontorDataInfo->bat.remap_soc >= 15) {
			sta_now.sessions_remaining  = 3;
		} else if (p_tMontorDataInfo->bat.remap_soc >= 10) {
			sta_now.sessions_remaining  = 2;
		} else if (p_tMontorDataInfo->bat.remap_soc >= 5) {
			sta_now.sessions_remaining  = 1;
		} else {
			sta_now.sessions_remaining  = 0;
		}
	}
	// sta_now.sessions_remaining = p_tMontorDataInfo->bat.remap_soc/5; //temp value

    if (battery_power_remaining == NULL)
    {
        return false;
    }
	else
	{
//		memcpy(battery_power_remaining, &sta_now, sizeof(session_service_battery_power_remaining_char_read_ntf_t));
		if (0 == memcmp(&sta_buf, &sta_now, sizeof(session_service_battery_power_remaining_char_read_ntf_t)))
		{
			return false;
		}
		else
		{			
			memcpy(battery_power_remaining, &sta_now, sizeof(session_service_battery_power_remaining_char_read_ntf_t));
			memcpy(&sta_buf, &sta_now, sizeof(session_service_battery_power_remaining_char_read_ntf_t));
			return true;
		}
	}    
}

static uint8_t	s_findmy_running_state = false;

/*
 * Function Name: xxx
 * 
 *
 * Function Description:
 * @brief  s_findmy_running_state management.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
bool bt_adapter_findmy_running_state_get(void)
{
    return s_findmy_running_state;
}

void bt_adapter_findmy_running_state_restore(void)
{
    beep_stop();
    s_findmy_running_state = false;
	set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
}

static void bt_adapter_findmy_running_state_set(void)
{
    s_findmy_running_state = true;
}

/*
 * Function Name: bt_adapter_findmy_device_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_findmy_device_get(session_service_findmy_device_char_t *findmy_device)
{
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (findmy_device == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	FindMe_t * sFind = (FindMe_t *)pBleData->bt_find_me;
	
	if (0xA55A == sFind->cal)
	{
		findmy_device->device_alert = sFind->dat[0];
		findmy_device->alert_duration = sFind->dat[1];
		return WICED_BT_GATT_SUCCESS;
	}
	else
	{
 		findmy_device->device_alert = 0;
 		findmy_device->alert_duration = 0;
		return WICED_BT_GATT_ERROR;
	} 
//    return status; 
}

/*
 * Function Name: bt_adapter_findmy_device_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_findmy_device_set(const uint8_t *pBuf, uint16_t len)
{

    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL || len != 2 || (*pBuf) > 1) 
    {
        return WICED_BT_GATT_ERROR;
    }

    #if 0
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	FindMe_t * sFind = (FindMe_t *)pBleData->bt_find_me;

	sFind->cal = 0xA55A;
	sFind->dat[0] = *pBuf++;							// 不用大小端转换
	sFind->dat[1] = *pBuf;
//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);

	if (1 == sFind->dat[0])
	{
		bt_adapter_findmy_running_state_set();
//		if (get_beep_status_idle())
		if (sFind->dat[1])
		{
//			beep_set((sFind->dat[1]*1000), 0, 1);
			beep_set_tune(1000 * sFind->dat[1]);
			set_system_external_even(EXT_EVENT_FIND_MY_GLO);
		}
		else
		{
//			beep_stop();
//			set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
			bt_adapter_findmy_running_state_restore();				// bug 2020143			
			sFind->dat[0] = 0;										// bug 2031575	若时间为0，则Enable = 0
		}
	}
	else if (0 == sFind->dat[0])
	{
//		beep_stop();
//		set_system_external_even(EXT_EVENT_FIND_MY_GLO_CANCLE);
		bt_adapter_findmy_running_state_restore();		
	}
//    else
//    {   
//	    return WICED_BT_GATT_ERROR;
//    }
	return WICED_BT_GATT_SUCCESS;
}

void bt_adapter_findmy_device_clr(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	FindMe_t * sFind = (FindMe_t *)pBleData->bt_find_me;
    uint16_t rslt = WICED_BT_GATT_SUCCESS;

	if (1 == sFind->dat[0])
	{
		sFind->dat[0] = 0;
//		app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
		set_update_BleData_status(1);

		beep_stop();
    	s_findmy_running_state = false;

		bt_adapter_findmy_device_get(app_session_service_ext_findmy_device_char);
        app_session_service_ext_findmy_device_char->device_alert = 0x00;

        if (app_server_context.bt_conn_id && (app_session_service_findmy_device_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION))
        {
            rslt = wiced_bt_gatt_server_send_notification(app_server_context.bt_conn_id,
                                HDLC_SESSION_SERVICE_FINDMY_DEVICE_VALUE,
                                app_session_service_ext_findmy_device_char_len,
                                (uint8_t *)app_session_service_ext_findmy_device_char,NULL);
            if (rslt == WICED_BT_GATT_SUCCESS)
            {
                sm_log(SM_LOG_DEBUG, "Sending Successful\n");
            }
        }
        else
        {
            sm_log(SM_LOG_DEBUG, "Notification Disable or Disconnected.\n");
        }
	}
}
/*
 * Function Name: get_oled_brightness_from_eeprom
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  None
 *
 * @return
 */
void get_oled_brightness_from_eeprom(void)
{
	uint8_t bright;

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->led_brightness;

	if (0xA55A == sParam->cal)				// 目标值为0-100
	{
		if (sParam->dat[0] > 100)
		{
			bright = 100;
		}
//		else if (sParam->dat[0] == 0)		// 不做截断处理
//		{
//			bright = 0;
//		}
//		else if (sParam->dat[0] < 10)
//		{
//			bright = 10;
//		}
		else
		{
			bright = sParam->dat[0];
		}
	}
	else
	{
		bright = 100;
	}

	amoled_brigth_set(bright);
}

/*
 * Function Name: bt_adapter_screen_led_brightness_control_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_screen_led_brightness_control_get(session_service_screen_led_control_char_t *screen_led_brightness_control)
{
	
//	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (screen_led_brightness_control == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

//	screen_led_brightness_control->brightness = amoled_brigth_get();
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->led_brightness;
	screen_led_brightness_control->brightness = sParam->dat[0];

	uint8_t temp = amoled_brigth_get();//bug1984162
    sm_log(SM_LOG_DEBUG, "amoled_brigth_get:%d Bledata:%d\n", temp, screen_led_brightness_control->brightness);

	if (sParam->dat[0] != temp) 
	{
		screen_led_brightness_control->brightness = temp;

		sParam->cal = 0xA55A;
		sParam->dat[0] = temp;
		set_update_BleData_status(1);
	}

	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_screen_led_brightness_control_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_screen_led_brightness_control_set(const uint8_t *pBuf, uint16_t len)
{

    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL || len != 1 || (*pBuf > 100)) 			// bug 1869317
    {
        return WICED_BT_GATT_ERROR;
    }

    #if 0
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->led_brightness;

	sParam->cal = 0xA55A;
	sParam->dat[0] = *pBuf;

//	if (sParam->dat[0] > 100)
//	{
//		sParam->dat[0] = 100;
//	}
//	else if (sParam->dat[0] == 0)		// 不做截断处理
//	{
//		sParam->dat[0] = 0;
//	}
//	else if (sParam->dat[0] < 10)
//	{
//		sParam->dat[0] = 10;
//	}

//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);

	if (SUBSTATUS_FIND_ME != get_subSystem_status())	// 若正在查找，不更新亮度，待查找完成后再更新 bug 1964122
	{
		get_oled_brightness_from_eeprom();

		uint32_t temp;
		temp = amoled_brigth_get();
		temp *= 255;
		temp /= 100;
		driver_rm69600_write_command(0x51);
		driver_rm69600_write_data((uint8_t)temp);
	}
	
    return status; 
}

/*
 * Function Name: get_motor_strength_from_eeprom
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  None
 *
 * @return
 */
void get_motor_strength_from_eeprom(void)
{
	uint8_t strength;
	
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->haptic_strength;

	if (0xA55A == sParam->cal)				// 目标值为0-0xff，需要转换成0-100
	{
		if (sParam->dat[0] > 100)
		{
			strength = 100;
		}
//		else if (sParam->dat[0] < 30)		// 不做截断处理
//		{
//			strength = 30;
//		}
		else
		{
			strength = sParam->dat[0];
		}
	}
	else
	{
		strength = 100;
	}
	motor_strength_set(strength);
}

/*
 * Function Name: bt_adapter_haptic_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_haptic_get(session_service_haptic_set_char_t *haptic_set_char)
{
    if (haptic_set_char == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

//	haptic_set_char->haptic_strength = motor_strength_get();
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->haptic_strength;
	haptic_set_char->haptic_strength = sParam->dat[0];

	uint8_t temp = motor_strength_get();//bug1984162
    sm_log(SM_LOG_DEBUG, "motor_strength_get:%d Bledata:%d\n", temp, haptic_set_char->haptic_strength);

	if (sParam->dat[0] != temp) 
	{
		haptic_set_char->haptic_strength = temp;

		sParam->cal = 0xA55A;
		sParam->dat[0] = temp;
		set_update_BleData_status(1);
	}

	return WICED_BT_GATT_SUCCESS;
}



/*
 * Function Name: bt_adapter_haptic_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_haptic_set(const uint8_t *pBuf, uint16_t len)
{

    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL || len != 1 || (*pBuf > 100)) 
    {
        return WICED_BT_GATT_ERROR;
    }

    #if 0
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->haptic_strength;

	sParam->cal = 0xA55A;
	sParam->dat[0] = *pBuf;

//	if (sParam->dat[0] > 100)
//	{
//		sParam->dat[0] = 100;
//	}
//	else if (sParam->dat[0] < 30)		// 不做截断处理
//	{
//		sParam->dat[0] = 30;
//	}

//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);

	if (SUBSTATUS_FIND_ME != get_subSystem_status())	// 若正在查找，不更新，待查找完成后再更新 bug 1964122
	{
		get_motor_strength_from_eeprom();
	}
    return status; 
}

/*
 * Function Name: get_beep_mode_from_eeprom
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  None
 *
 * @return
 */
void get_beep_mode_from_eeprom(void)
{
	uint8_t mode;

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->buz_loudness;

	if (0xA55A == sParam->cal)				// 目标值为0-2
	{
		if (sParam->dat[0] > 2)
		{
			mode = 0;
		}
		else
		{
			mode = sParam->dat[0];
		}
	}
	else
	{
		mode = 0;
	}
	beep_mode_set(mode);
}

/*
 * Function Name: bt_adapter_buzzer_speaker_loudness_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_buzzer_speaker_loudness_get(session_service_buzzer_speaker_set_char_t *buzzer_speaker_set_char)
{
    if (buzzer_speaker_set_char == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	buzzer_speaker_set_char->loudness = beep_mode_get();
	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_buzzer_speaker_loudness_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_buzzer_speaker_loudness_set(const uint8_t *pBuf, uint16_t len)
{
	wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

	sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (pBuf == NULL || len != 1 || (*pBuf > 2)) 			// bug 1885699
	{
		return WICED_BT_GATT_ERROR;
	}

#if 0
	for(uint8_t i=0; i<len; i++)
	{
		sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
	}
	sm_log(SM_LOG_DEBUG, "\n");
#endif

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->buz_loudness;

	sParam->cal = 0xA55A;
	sParam->dat[0] = *pBuf;

//	if (sParam->dat[0] > 2)
//	{
//		sParam->dat[0] = 2;
//	}

//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);

	get_beep_mode_from_eeprom();
	return status; 
}

/*
 * Function Name: get_lock_mode
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
bool get_lock_mode(void)
{
    return gbLockMode;
}

void set_lock_mode(bool dat)
{
    gbLockMode = dat;
}


/*
 * Function Name: get_lock_mode_from_eeprom
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
bool get_lock_mode_from_eeprom(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->lock_mode;

	if (0xA55A == sParam->cal)
	{
		if (sParam->dat[0] == 0)
		{
			gbLockMode = false;
		}
		else
		{
			gbLockMode = true;
		}
	}
	else
	{
		gbLockMode = false;//BAT indicate default unlock!
	}
	return gbLockMode;
}


/*
 * Function Name: bt_adapter_device_lock_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_device_lock_get(session_service_device_lock_char_t *device_lock)
{
    if (device_lock == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	device_lock->device_lock_state = get_lock_mode();
	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_device_lock_status_change_happened
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_handler.
 *
 * @param  None
 *
 * @return STATUS  (true or false)
 */
bool bt_adapter_device_lock_status_change_happened(session_service_device_lock_char_t *device_lock)
{
	static uint8_t status_buf = 0;
	uint8_t status_new;
    if (device_lock == NULL)
    {
        return false;
    }

    // sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	status_new = get_lock_mode();
	if (status_new == status_buf)
	{
		return false;
	}
	else
	{
		status_buf = status_new;
		device_lock->device_lock_state = get_lock_mode();
		return true;
	}
}

bool g_updateLockStatus = false;
bool get_update_lock_status(void)
{
    return g_updateLockStatus;
}
void set_update_lock_status(bool status)
{
    g_updateLockStatus = status;
}

void lockStatus_update_to_eeprom(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->lock_mode;

	sParam->cal = 0xA55A;
	sParam->dat[0] = get_lock_mode()? 1:0;

//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);
}


/*
 * Function Name: bt_adapter_device_lock_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_write_handler.
 *
 * @param  
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_device_lock_set(const uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL || len != 1 || (*pBuf > 1)) 
    {
        return WICED_BT_GATT_ERROR;
    }

    #if 0
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	bool bLock;
	bLock = *pBuf;
	if (bLock)
	{
		set_system_external_even(EXT_EVENT_LOCK);
	}
	else
	{
		set_system_external_even(EXT_EVENT_UNLOCK);
	}

	set_lock_mode(bLock);
	set_update_lock_status(1);

    return status; 
}

/*-------------------------------------------*/
bool get_eos_prompt(void)
{
    return gbEosPrompt;
}

/*-------------------------------------------*/
bool get_eos_prompt_from_eeprom(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->Eos_prompt;

	if (0xA55A == sParam->cal)
	{
		if (sParam->dat[0] == 0)
		{
			gbEosPrompt = false;
		}
		else
		{
			gbEosPrompt = true;
		}
	}
	else
	{
		gbEosPrompt = false;//BAT indicate default unlock!
	}
	return gbEosPrompt;
}

/*-------------------------------------------*/
wiced_bt_gatt_status_t bt_adapter_eos_prompt_get(uint8_t *dat)
{
    if (dat == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	*dat = get_eos_prompt();
	return WICED_BT_GATT_SUCCESS;
}

/*-------------------------------------------*/
wiced_bt_gatt_status_t bt_adapter_eos_prompt_set(const uint8_t *pBuf, uint16_t len)
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

//    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL || len != 1 || (*pBuf > 1)) 
    {
        return WICED_BT_GATT_ERROR;
    }

	gbEosPrompt = (*pBuf)? 1:0;

    sm_log(SM_LOG_DEBUG, "%s, gbEosPrompt:%d\n", __FUNCTION__, gbEosPrompt);

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->Eos_prompt;
	
	sParam->dat[0] = gbEosPrompt;
	sParam->cal = 0xA55A;						// 需要写Magic字段
	set_update_BleData_status(1);

    return status; 
}

/*-------------------------------------------*/
bool get_clean_prompt(void)
{
#ifdef DEF_DRY_CLEAN_EN
    return gbCleanPrompt;
#else
	return 0;
#endif
}
/*-------------------------------------------*/
bool get_clean_prompt_from_eeprom(void)
{
#ifdef DEF_DRY_CLEAN_EN
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->Clean_prompt;

	if (0xA55A == sParam->cal)
	{
		if (sParam->dat[0] == 0)
		{
			gbCleanPrompt = false;
		}
		else
		{
			gbCleanPrompt = true;
		}
	}
	else
	{
		gbCleanPrompt = false;//BAT indicate default unlock!
	}
	return gbCleanPrompt;
#else
	return 0;
#endif
}
/*-------------------------------------------*/
wiced_bt_gatt_status_t bt_adapter_clean_prompt_get(uint8_t *dat)
{
    if (dat == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	*dat = get_clean_prompt();
	return WICED_BT_GATT_SUCCESS;
}
/*-------------------------------------------*/
wiced_bt_gatt_status_t bt_adapter_clean_prompt_set(const uint8_t *pBuf, uint16_t len)
{	
#ifdef DEF_DRY_CLEAN_EN
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

//    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL || len != 1 || (*pBuf > 1)) 
    {
        return WICED_BT_GATT_ERROR;
    }

	gbCleanPrompt = (*pBuf)? 1:0;

    sm_log(SM_LOG_DEBUG, "%s, gbCleanPrompt:%d\n", __FUNCTION__, gbCleanPrompt);

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->Clean_prompt;
	
	sParam->dat[0] = gbCleanPrompt;
	sParam->cal = 0xA55A;						// 需要写Magic字段
	set_update_BleData_status(1);

    return status; 
#else
	return WICED_BT_GATT_ERROR;
#endif
}

/*
 * Function Name: get_time_stamp_from_flash
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
void get_time_stamp_from_flash(void)
{
#if 0
	ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
	E2PDataType_t e2pTemp;
	Param_Rtc_t sParamRtc;

	e2pTemp.addr = E2P_ADDR_OF_TIME_STAMP;
	e2pTemp.pDate = &sParamRtc;
	e2pTemp.size = sizeof(sParamRtc);
	e2pDev->read((uint8_t*)&e2pTemp,sizeof(e2pTemp));

	if (0xA55A == sParamRtc.cal)
	{
		gRtcTrusted = sParamRtc.resetCnt;
		if (gRtcTrusted < 0x0F)	
		{
			gRtcTrusted++;
		}
	}
	else
	{
//		sParamRtc.timeStamp = 1723075688;
		sParamRtc.timeStamp = 0;				// 客户要求默认值为0
		gRtcTrusted = 0x0F;
	}
//	sm_log(SM_LOG_DEBUG, "Set RTC TimeStamp = %d\r\n", timeStamp);
#endif
	Param_Rtc_t sParamRtc;
    bool res = app_param_read(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));
    if (res == false || sParamRtc.cal != 0xA55A) 
	{
		sParamRtc.timeStamp = 0;                // 客户要求默认值为0
		gRtcTrusted = 0;
    }
	else
	{
		gRtcTrusted = sParamRtc.resetCnt;
// 逻辑修改，长按复位前若为2，则写入0x51，复位后判断
		if (gRtcTrusted == 0x51)				// 同步时钟后，长按复位1次
		{
			gRtcTrusted = 1;
			sParamRtc.resetCnt = gRtcTrusted;
			app_param_write(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));
		}
		else if (BleCongested_Read())			// 若为静默重启；则保持原有状态
		{
			if (gRtcTrusted > 2)
			{
				gRtcTrusted = 0;
			}
		}
		else
		{
			gRtcTrusted = 0;
		}
    }

	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->write((uint8_t *)&(sParamRtc.timeStamp), 4);
}


/*
 * Function Name: set_time_stamp_to_flash
 * 
 *
 * Function Description:
 * @brief  
 *
 * @param  
 *
 * @return
 */
void set_time_stamp_to_flash()
{
//	ptIoDev e2pDev = io_dev_get_dev(DEV_MCU_EEPROM);//
//	E2PDataType_t e2pTemp;
	Param_Rtc_t sParamRtc;

	sParamRtc.cal = 0xA55A;
	sParamRtc.resetCnt = gRtcTrusted;

	ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
	rtcDev->read((uint8_t *)&(sParamRtc.timeStamp), 4);
//	sm_log(SM_LOG_DEBUG, "Set EEPROM TimeStamp = %d\r\n", timeStamp);
    
    app_param_write(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));

    #if 0
	e2pTemp.addr = E2P_ADDR_OF_TIME_STAMP;
	e2pTemp.pDate = &sParamRtc;
	e2pTemp.size = sizeof(sParamRtc);
	e2pDev->write((uint8_t*)&e2pTemp,sizeof(e2pTemp));
    #endif
}

//void clr_time_stamp_trush()
//{
//	if (gRtcTrusted)
//	{
//		gRtcTrusted = 0;					// 复位后为零，不可信任
//
//		Param_Rtc_t sParamRtc;
//
//		sParamRtc.cal = 0xA55A;
//		sParamRtc.resetCnt = gRtcTrusted;
//
//		ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
//		rtcDev->read((uint8_t *)&(sParamRtc.timeStamp), 4);
//	    
//	    app_param_write(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));
//	}
//}

void BleCongested_Write(bool sta)
{
	uint8_t res;
	
	res = (sta)? 1:0;

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->ble_Congested;
	
	sParam->dat[0] = res;
	sParam->cal = 0xA55A;						// 需要写Magic字段
	set_update_BleData_status(1);
}

bool BleCongested_Read(void)
{
	bool res;
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->ble_Congested;

	if (0xA55A == sParam->cal)
	{
		if (sParam->dat[0] == 0)
		{
			res = false;
		}
		else
		{
			res = true;
		}
	}
	else
	{
		res = false;
	}
	return res;
}

void handle_time_stamp_trush_Long_Press_Reset(void)
{
	if (gRtcTrusted == 2)
	{
//		gRtcTrusted = 0x51;					// 复位后为零，不可信任
		Param_Rtc_t sParamRtc;

		sParamRtc.cal = 0xA55A;
		sParamRtc.resetCnt = 0x51;

		ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
		rtcDev->read((uint8_t *)&(sParamRtc.timeStamp), 4);
	    
	    app_param_write(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));
	}
}

/*
 * Function Name: bt_adapter_session_records_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_session_records_get(session_service_records_char_t *session_records_char)
{
    if ((session_records_char == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (session_record_get(session_records_char))
	{
		return WICED_BT_GATT_ERROR;
	}
	else
	{
		return WICED_BT_GATT_SUCCESS;
	}
}

/*
 * Function Name: bt_adapter_session_records_get_next
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_session_records_get_next(session_service_records_char_t *session_records_char_next)
{
    if ((session_records_char_next == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    // sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (session_record_get_next(session_records_char_next))
	{
		return WICED_BT_GATT_ERROR;
	}
	else
	{
		return WICED_BT_GATT_SUCCESS;
	}
}


/*
 * Function Name: bt_adapter_session_records_number_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return the numbers of session records
 */
uint32_t bt_adapter_session_records_number_get(void)
{
    // sm_log(SM_LOG_DEBUG, "%s, num: %d\n", __FUNCTION__, session_record_total_get());

	return session_record_total_get();	
}


/*
 * Function Name: bt_adapter_session_records_clear_all
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_session_records_clear_all(void)
{
    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (session_record_erase_all())
	{
		return WICED_BT_GATT_ERROR;
	}
	else
	{
		return WICED_BT_GATT_SUCCESS;
	}
}

/*
 * Function Name: bt_adapter_session_records_dummy_debug_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_session_records_dummy_debug_set(session_service_records_char_t *session_records_char, uint32_t number)
{

    if ((session_records_char == NULL) || number > 4096)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	dummy_session_record_number = dummy_session_record_number + number;

	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_session_records_dummy_debug_set_proc
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_session_records_dummy_debug_set_proc(uint32_t* remain_number)
{

	session_service_records_char_t session_records_char;
	
    // sm_log(SM_LOG_DEBUG, "Dummy_session_record_number, remain: %ld\n", dummy_session_record_number);

    if (remain_number == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

//	session_records_char->count             = 0x17B;         //Counter that increments each time a session is started on the device.
	session_records_char.time_stamp        = 0xFFFFFFFF;    //Epoch time of start of session.
	session_records_char.duration          = 0xFFFF;        //Duration of the session in seconds.
	session_records_char.session_exit_code = 0xFFFF;        //Exit Codes from a session.
	session_records_char.mode              = 0xFF;          //0 = Base Session1 = Session was run in “Boost Mode”.
	session_records_char.heatingProfile	= 0xFF;
	session_records_char.z1_max_temp       = 0xFFFF;        //Max temperature that Z1 reached during the session.
//	session_records_char->z2_max_temp       = 0xFFFF;        //Max temperature that Z2 reached during the session.
	session_records_char.battery_max_temp  = 0xFFFF;        //Max temperature that the battery reached during the session
	session_records_char.trusted           = 0x01;//0xFF;          //0 = Trusted 1 = Untrusted

	HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
	SessionTimeSum_v2_t *p_tSessionTimeSum = get_session_time_sum_info_handle();

	if (dummy_session_record_number > 0)
	{
		dummy_session_record_number--;
		*remain_number = dummy_session_record_number;
		p_tHeatParamInfo->smokeSuctionSum++; // 总加热次数 
		p_tSessionTimeSum->suctionSum = p_tHeatParamInfo->smokeSuctionSum;
		set_update_session_time_sum_status(true);

		session_records_char.count = p_tHeatParamInfo->smokeSuctionSum;
		if (session_record_insert(&session_records_char))
		{
			return WICED_BT_GATT_ERROR;
		}
	}
	else
	{
		*remain_number = 0;
	}
	cy_rtos_delay_milliseconds(10);
	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_session_status_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_session_status_get(session_service_status_char_t *session_status)
{
    if (session_status == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	session_status_get(session_status);

	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_session_status_change_happened
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_handler.
 *
 * @param  None
 *
 * @return STATUS  (true or false)
 */
bool bt_adapter_session_status_change_happened(session_service_status_char_t *session_status)
{
	static session_service_status_char_t status_buf;
	
	session_service_status_char_t status_new;
    if (session_status == NULL)
    {
        return false;
    }

    // sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	session_status_get(&status_new);
	if(memcmp(&status_buf,&status_new,sizeof(session_service_status_char_t)) == 0)
	{
		return false;
	}
	else
	{
		memcpy(&status_buf,&status_new,sizeof(session_service_status_char_t));
		memcpy(session_status,&status_new,sizeof(session_service_status_char_t));
		return true;
	}
}


void bt_adapter_factory_reset_all(void)
{
	delete_all_error_even();
// 清除Flash数据记录
	session_record_erase_all();
	event_record_erase_all(1);
	errcode_clr_to_flash();
	lifecycle_clr_to_flash();


// 清除蓝牙增加部分EEPROM
// 清除RTC
	Param_Rtc_t sParamRtc = {0};
    app_param_write(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));

// 清除Flash最后2Row写Heat部分
//	HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//	ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
//	memset(p_tHeatParamInfo, 0xff, sizeof(HeatParamInfo_t));
//	paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));

// 清除蓝牙邦定数据部分	// Gison 20241026
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
//	memset((uint8_t *)pBleData, 0, sizeof(Ble_Flash_Data_t));	// 若读出数据无效，全清零
	memset((uint8_t *)(pBleData->bt_dev_name), 0, (sizeof(Ble_Flash_Data_t) - 2054));	// 清除除Profile外数据
	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(0);

// 清除烟支记录Counter
	HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
	SessionTimeSum_v2_t *p_tSessionTimeSum = get_session_time_sum_info_handle();
		
	p_tHeatParamInfo->smokeSuctionSum = 0; // 总加热次数 
	p_tSessionTimeSum->suctionSum = 0;
	p_tSessionTimeSum->baseTimeSumClean = 0;
	p_tSessionTimeSum->boostTimeSumClean = 0; //自清洁烟支计算清零
//	p_tSessionTimeSum->baseTimeSum = 0;		使用该2字段计算EOL
//	p_tSessionTimeSum->boostTimeSum = 0;
			
	set_update_session_time_sum_status(false);
	session_time_sum_update_to_flash();

	uimage_record_erase_all();									// 需要清除MyName
}


/*
 * Function Name: bt_adapter_factory_reset_ble
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_write_handler.
 *
 * @param  
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_factory_reset_ble(const uint8_t *pBuf, uint16_t len)
{
    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

    if (pBuf == NULL) 
    {
        return WICED_BT_GATT_ERROR;
    }

    #if 0
    for(uint8_t i=0; i<len; i++)
    {
        sm_log(SM_LOG_DEBUG, " %02x", pBuf[i]);
    }
    sm_log(SM_LOG_DEBUG, "\n");
    #endif

	 if (pBuf[0] == 1)
	 {
			delete_all_error_even();
		// 清除Flash数据记录
			session_record_erase_all();		//bug. 1935399
			event_record_erase_all(1); //0>1, 通过蓝牙发送Factory Reset的时候清除Log的Cnt
			errcode_clr_to_flash();
			lifecycle_clr_to_flash();


		// 清除蓝牙增加部分EEPROM
		// 清除RTC
			Param_Rtc_t sParamRtc = {0};
			app_param_write(INDEX_D_5, (uint8_t *)&sParamRtc, sizeof(sParamRtc));

		// 清除Flash最后2Row写Heat部分
//			HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
//			ptIoDev paramDev = io_dev_get_dev(DEV_PARAM);
//			memset(p_tHeatParamInfo, 0xff, sizeof(HeatParamInfo_t));
//			paramDev->write( (uint8_t*)&(p_tHeatParamInfo->validCheck), sizeof(HeatParamInfo_t));

		// 清除蓝牙邦定数据部分	// Gison 20241026
			Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
//			memset((uint8_t *)pBleData, 0, sizeof(Ble_Flash_Data_t));	// 若读出数据无效，全清零
			memset((uint8_t *)(pBleData->bt_dev_name), 0, (sizeof(Ble_Flash_Data_t) - 2054));	// 清除除Profile外数据
			app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
			set_update_BleData_status(0);

// 清除烟支记录Counter	烟支记录不删除，此处也不能清除										//bug. 1935399
			HeatParamInfo_t* p_tHeatParamInfo = get_heat_param_info_handle();
			SessionTimeSum_v2_t *p_tSessionTimeSum = get_session_time_sum_info_handle();
				
			p_tHeatParamInfo->smokeSuctionSum = 0; // 总加热次数 
			p_tSessionTimeSum->suctionSum = 0;
			p_tSessionTimeSum->baseTimeSumClean = 0;
			p_tSessionTimeSum->boostTimeSumClean = 0; //自清洁烟支计算清零
		//	p_tSessionTimeSum->baseTimeSum = 0; 	使用该2字段计算EOL
		//	p_tSessionTimeSum->boostTimeSum = 0;
					
			set_update_session_time_sum_status(false);
			session_time_sum_update_to_flash();
		
			uimage_record_erase_all();									// 需要清除MyName
//			set_shipping_mode(1);		Factory Reset 不进入船运：Bug. 1938731
			cy_rtos_delay_milliseconds(500); //bug1984589
			NVIC_SystemReset(); 		// 复位系统
	 }
	 else if (pBuf[0] == 2)//uimage reset
	 {
		uimage_record_erase_all();
	 }
    return WICED_BT_GATT_SUCCESS; 
}


// 判断是否选择默认加热曲线
bool IsCustomHeatingProfile(void)				//选择曲线0时返回0，曲线1和2返回1；用于显示定制化
{
	HEATER *heaterInfo = get_heat_manager_info_handle();
	return heaterInfo->heatingProfile_base == 0? false:true;
}

// 判断是否选择默认加热曲线
bool IsCustomHeatingProfileBoost(void)				//选择曲线0时返回0，曲线1和2返回1；用于显示定制化
{
	HEATER *heaterInfo = get_heat_manager_info_handle();
	return heaterInfo->heatingProfile_boost == 0x10? false:true;
}
/*
 * Function Name: bt_adapter_heating_prof_sel_index_number_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  base_profile_index / boost_profile_index
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_index_number_get(uint8_t *base_profile_index, uint8_t *boost_profile_index)
{
    if ((base_profile_index == NULL) || (boost_profile_index == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	HEATER *heaterInfo = get_heat_manager_info_handle();
	*base_profile_index = heaterInfo->heatingProfile_base;
	*boost_profile_index = heaterInfo->heatingProfile_boost;	// 不接受指定，固定为0x10(初始化时已经设置)

	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_heating_prof_sel_profiles_installed_index_array_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  pBuf/len, array_buf/array_len
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_profiles_installed_index_array_get(uint8_t *pBuf, uint16_t *len)
{
    if ((pBuf == NULL) || (len == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t *	heatProfile = (heatProfile_t *)pBleData->heatProfile;

	uint8_t i,number;
	uint8_t dataTemp;
	uint16_t length;
	uint8_t *pBufTemp = pBuf;
	number = 0;
	
	for (i = 0; i < MAX_NUM_HEATING_PROFILE; i++)				// 枚举 MAX_NUM_HEATING_PROFILE 条曲线
	{
		if (heatProfile->magic == DEFAULT_HEATING_PROFILE_MAGIC_CONFIG)
		{
			number++;
			memcpy(pBufTemp, &(heatProfile->tHeatPower[0]), HEATING_PROFILE_LEN);
		}
		else
		{
			memset(pBufTemp, 0, HEATING_PROFILE_LEN);
		}
		pBufTemp += HEATING_PROFILE_LEN;
		heatProfile++;						// 地址指向下一条
	}
	
	if (number)	
	{
		pBufTemp = pBuf;
// 大小端转换
		for (length = 0; length < (HEATING_PROFILE_LEN * MAX_NUM_HEATING_PROFILE); length += 2)
		{
			dataTemp =	*pBufTemp;
			*pBufTemp = *(pBufTemp+1);
			pBufTemp++;
			*pBufTemp++ = dataTemp;
		}
		*len = HEATING_PROFILE_LEN * MAX_NUM_HEATING_PROFILE;
		return WICED_BT_GATT_SUCCESS;
	}
	else		
	{
		*len = 0;
		return WICED_BT_GATT_ERROR;
	}
}


/*
 * Function Name: bt_adapter_heating_prof_sel_profiles_installed_index_array_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  pBuf/len, array_buf/array_len
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_profiles_installed_index_array_set(uint8_t *pBuf, uint16_t len)
{
    if ((pBuf == NULL) || (len != (HEATING_PROFILE_LEN * MAX_NUM_HEATING_PROFILE)))
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	heatProfile_t	heatProfile[MAX_NUM_HEATING_PROFILE];

	uint8_t i,dataTemp,length;
	uint8_t *pBufTemp;
	uint8_t *pHeatProfile;

	pHeatProfile = &(heatProfile[0].tHeatPower);

	for (i = 0; i < MAX_NUM_HEATING_PROFILE; i++)								// 枚举 MAX_NUM_HEATING_PROFILE 条曲线
	{
		heatProfile[i].magic = DEFAULT_HEATING_PROFILE_MAGIC_CONFIG;
		memcpy(pHeatProfile, pBuf, HEATING_PROFILE_LEN);

		pBufTemp = pHeatProfile;
// 大小端转换
		for (length = 0; length < (HEATING_PROFILE_LEN); length += 2)
		{
			dataTemp =	*pBufTemp;
			*pBufTemp = *(pBufTemp+1);
			pBufTemp++;
			*pBufTemp++ = dataTemp;
		}
// 相应地址++
		pHeatProfile += sizeof(heatProfile_t);
		pBuf += HEATING_PROFILE_LEN;
	}
// 写入
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	memcpy((uint8_t*)pBleData->heatProfile, (uint8_t*)heatProfile, sizeof(heatProfile_t) * MAX_NUM_HEATING_PROFILE);
//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);
	
	return WICED_BT_GATT_SUCCESS;
}


/*
 * Function Name: bt_adapter_heating_prof_sel_profiles_installed_index_number_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  mode
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
uint8_t bt_adapter_heating_prof_sel_profiles_installed_index_number_get(uint8_t *pBuf)
{
	if ((pBuf == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }
    uint8_t number = 0;
	
    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
	
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t *	heatProfile = (heatProfile_t *)pBleData->heatProfile;

	for (uint8_t i = 0; i < (MAX_NUM_HEATING_PROFILE - 1); i++)				// 3条Base
	{
		if (heatProfile->magic == DEFAULT_HEATING_PROFILE_MAGIC_CONFIG)
		{
			number++;
			*pBuf++ = i;
		}
		heatProfile++;		// 指向下一地址
	}

//	for (uint8_t i = 0; i < MAX_NUM_HEATING_PROFILE; i++)					// 1条Boost
	{
		if (heatProfile->magic == DEFAULT_HEATING_PROFILE_MAGIC_CONFIG)
		{
			number++;
			*pBuf++ = 0x10;
		}
//		heatProfile++;		// 指向下一地址
	}

	*pBuf = 0xFF;

	return number;
}

/*
 * Function Name: bt_adapter_heating_prof_sel_index_payload_get_next
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  pBuf/len
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_index_payload_get(uint8_t *pBuf, uint8_t *len, uint8_t restart_index)
{
	if (restart_index == 3)
	{
        return WICED_BT_GATT_ERROR;
	}
	else if (restart_index == 0x10)	// boost曲线的序号为0x10
	{
		restart_index = 3;
	}
	
    if ((pBuf == NULL) || (len == NULL) || restart_index >= MAX_NUM_HEATING_PROFILE)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	heatProfile_t *	heatProfile = (heatProfile_t *)pBleData->heatProfile;

	heatProfile += restart_index;

	if (heatProfile->magic == DEFAULT_HEATING_PROFILE_MAGIC_CONFIG)
	{
		memcpy(pBuf, &(heatProfile->tHeatPower[0]), HEATING_PROFILE_LEN);
		*len = HEATING_PROFILE_LEN;
// 大小端转换
		uint8_t length,dataTemp;
		uint8_t *pBufTemp = pBuf;

		for (length = 0; length < HEATING_PROFILE_LEN; length += 2)
		{
			dataTemp =	*pBufTemp;
			*pBufTemp = *(pBufTemp+1);
			pBufTemp++;
			*pBufTemp++ = dataTemp;
		}
		return WICED_BT_GATT_SUCCESS;
	}
	else
	{
		*len = 0;
		return WICED_BT_GATT_ERROR;
	}
}

/*
 * Function Name: bt_adapter_heating_prof_sel_index_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_write_handler.
 *
 * @param  
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_heating_prof_sel_index_set(uint8_t mode, uint8_t profile_index)
{
//    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (mode > 1)
	{
		return WICED_BT_GATT_ERROR;
	}

	if (mode == 1)					// boost只有一条曲线，指定其它为错误，指定特定直接返回，不操作
	{
		if (profile_index == 0x10)	
		{		
//			if (get_current_ui_detail_status() < UI_HEATTING || get_current_ui_detail_status() == UI_ERR_BAT_LOW)
//			{
//				set_current_ui_detail_status(UI_SET_PROFILE_BOOST);	// 由UI判断是否显示Doc
//			}
//			motor_set2(HAPTIC_1);
			set_system_external_even(EXT_EVENT_HEATING_PROFILES_BOOST);
			return WICED_BT_GATT_SUCCESS;
		}
		else						
		{
			return WICED_BT_GATT_ERROR;
		}
	}
	else	// base
	{
		if (profile_index > 2)
		{
			return WICED_BT_GATT_ERROR;
		}
	// 先确定Index内容是否存在
		Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
		heatProfile_t *	heatProfile = (heatProfile_t *)pBleData->heatProfile;

		heatProfile += profile_index;

		if (heatProfile->magic != DEFAULT_HEATING_PROFILE_MAGIC_CONFIG)
		{
	    	sm_log(SM_LOG_DEBUG, "heatProfile.magic err!\n");
			return WICED_BT_GATT_ERROR;
		}
	// 先读取Sel_Index
		heatProfileSel_t * heatProfileSel = (heatProfileSel_t *)pBleData->heatProfileSel;

	// 更新后回写Sel_Index
		if (heatProfileSel->magic != DEFAULT_HEATING_PROFILE_SEL_MAGIC_CONFIG)		// v1版本0xA55A，由于更新了Profile,更新Magic
		{
			heatProfileSel->index_base_used = 0;
			heatProfileSel->index_boost_used = 3;	// 必须指定3位置，只有一条曲线
			heatProfileSel->magic = DEFAULT_HEATING_PROFILE_SEL_MAGIC_CONFIG;
		}
//		if (mode == 0)
//		{
			heatProfileSel->index_base_used = profile_index;		// 指向新Index
//		}
//		else if (mode == 1)
//		{
//			heatProfileSel->index_boost_used = profile_index;	// 指向新Index
//		}
	//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
		set_update_BleData_status(1);

		heat_param_info_update();

//		if (get_current_ui_detail_status() < UI_HEATTING || get_current_ui_detail_status() == UI_ERR_BAT_LOW)
//		{
//			set_current_ui_detail_status(UI_SET_PROFILE);	// 由UI判断是否显示Doc
//		}
//		motor_set2(HAPTIC_1);
		set_system_external_even(EXT_EVENT_HEATING_PROFILES);

	    return WICED_BT_GATT_SUCCESS; 
	}
}

/*
 * Function Name: bt_adapter_event_log_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_event_log_get(debug_service_event_log_char_t *event_log_char)
{
    if ((event_log_char == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (event_record_get(event_log_char))
	{
		return WICED_BT_GATT_ERROR;
	}
	else
	{
		return WICED_BT_GATT_SUCCESS;
	}
}

/*
 * Function Name: bt_adapter_event_log_get_next
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_event_log_get_next(debug_service_event_log_char_t *event_log_char_next)
{
    if ((event_log_char_next == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    // sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (event_record_get_next(event_log_char_next))
	{
		return WICED_BT_GATT_ERROR;
	}
	else
	{
		return WICED_BT_GATT_SUCCESS;
	}
}


/*
 * Function Name: bt_adapter_event_log_number_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return the numbers of session records
 */
uint32_t bt_adapter_event_log_number_get(void)
{
    // sm_log(SM_LOG_DEBUG, "%s, num: %d\n", __FUNCTION__, event_record_total_get());

	return event_record_total_get();	
}

/*
 * Function Name: bt_adapter_event_log_clear_all
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_event_log_clear_all(void)
{
    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	if (event_record_erase_all(0))
	{
		return WICED_BT_GATT_ERROR;
	}
	else
	{
		return WICED_BT_GATT_SUCCESS;
	}
}

/*
 * Function Name: bt_adapter_event_log_dummy_debug_set
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_event_log_dummy_debug_set(debug_service_event_log_char_t *event_log_char, uint32_t number)
{

    if ((event_log_char == NULL) || number > 4096)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	dummy_event_log_number = dummy_event_log_number + number;

	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_event_log_dummy_debug_set_proc
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_event_log_dummy_debug_set_proc(uint32_t* remain_number)
{

	debug_service_event_log_char_t event_log_char;

    // sm_log(SM_LOG_DEBUG, "Dummy_event_log_number, remain: %ld\n", dummy_event_log_number);

    if (remain_number == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

	uint8_t i;
	event_log_char.count      = 0;								// Flash存储管理该ID         
	event_log_char.timestamp  = 0xFFFFFFFF;    
	event_log_char.event_code = 0xFF;
	for (i = 0; i < 15; i++)
	{
		event_log_char.event_data[i] = 0xff;
	}

	if (dummy_event_log_number > 0)
	{
		dummy_event_log_number--;
		*remain_number = dummy_event_log_number;
		if (event_record_insert(&event_log_char))
		{
			return WICED_BT_GATT_ERROR;
		}
	}
	else
	{
		*remain_number = 0;
	}
	cy_rtos_delay_milliseconds(10);
	return WICED_BT_GATT_SUCCESS;
	
}

/*
 * Function Name: bt_adapter_lifecycle_data_individual_field_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return value
 */
uint32_t bt_adapter_lifecycle_data_individual_field_get(debug_service_lifecycle_data_opcode01_type_e opcode01_type)
{

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	LifeCycle_t * pLifeCycle = get_life_cycle_handle();

	switch (opcode01_type)
	{
		case MIN_BAT_TEMP:				return (uint32_t)pLifeCycle->batTempMin;
		case MAX_BAT_TEMP:				return (uint32_t)pLifeCycle->batTempMax;
		case MIN_BAT_VOL:				return (uint32_t)pLifeCycle->batVolMin;
		case MAX_BAT_VOL:				return (uint32_t)pLifeCycle->batVolMax;
		case MAX_BAT_CHARGE_CURRENT:	return (uint32_t)pLifeCycle->batChargeCurrentMax;
		case MAX_BAT_DISCHARGE_CURRENT:	return (uint32_t)pLifeCycle->batDischargeCurrentMax;
		case TOTAL_CHARGE_TIME:			return (uint32_t)pLifeCycle->batChargeTimeTotal;
		case FULLY_CHARGED_COUNT:		return (uint32_t)pLifeCycle->batFullChargeCnt;
		case COMPLETE_SESSION:			return (uint32_t)pLifeCycle->sessionCompleteCnt;
		case INCOMPLETE_SESSION:		return (uint32_t)pLifeCycle->sessionIncompleteCnt;
		case MIN_ZONE1_TEMP:			return (uint32_t)pLifeCycle->tempZ1Min;
		case MAX_ZONE1_TEMP:			return (uint32_t)pLifeCycle->tempZ1Max;
//		case MIN_ZONE2_TEMP:			return (uint32_t)pLifeCycle->tempZ2Min;
//		case MAX_ZONE2_TEMP:			return (uint32_t)pLifeCycle->tempZ2Max;
		default:						return 0;
	}
}

/*
 * Function Name: bt_adapter_lifecycle_data_all_field_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_lifecycle_data_all_field_get(debug_service_lifecycle_data_opcode01_rsp_t *lifecycle_data)
{

    if ((lifecycle_data == NULL))
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	LifeCycle_t * pLifeCycle = get_life_cycle_handle();

	memcpy((uint8_t*)lifecycle_data, (uint8_t*)(&(pLifeCycle->batTempMin)), sizeof(debug_service_lifecycle_data_opcode01_rsp_t));
	
	return WICED_BT_GATT_SUCCESS;
	
}

/*
 * Function Name: bt_adapter_lifecycle_data_clear_all
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_lifecycle_data_clear_all(void)
{
    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	lifecycle_clr_to_flash();
	return WICED_BT_GATT_SUCCESS;
}


/*
 * Function Name: bt_adapter_random_data_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_write_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
wiced_bt_gatt_status_t bt_adapter_random_data_get(uint8_t *pBuf, const uint8_t length)
{
    if ((pBuf == NULL) || length > 32)
    {
        return WICED_BT_GATT_ERROR;
    }

//    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	uint32_t aData[32];
	ptIoDev trngDev = io_dev_get_dev(DEV_TRNG);
	trngDev->read(aData, 32);			// 底层驱动限定32长度

	memcpy(pBuf, aData, length);
	
	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_mbedtls_ctr_drbg_random
 * 
 *
 * Function Description:
 * @brief  imitation mbedtls_ctr_drbg_random
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */
int bt_adapter_mbedtls_ctr_drbg_random( void *p_rng, unsigned char *output,
                             size_t output_len )
{
    if ((output == NULL) || output_len > 1024)
    {
        return WICED_BT_GATT_ERROR;
    }

	if ((output_len > 0)&&(output_len <= 32))
	{
		bt_adapter_random_data_get(output, output_len);
	}
	else if (output_len > 32)
	{
		size_t offset = 0;
		do
		{
			if (32 < (output_len-offset))
			{
				bt_adapter_random_data_get(output+offset, 32);
				offset = offset +32;
			}
			else 
			{
				bt_adapter_random_data_get(output+offset, (output_len-offset));
				offset = output_len;
			}
		} while (offset < output_len);
	}
	
	return 0;
}

/*
 * Function Name: bt_adapter_last_error_get
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */ 
//extern const uint16_t cErrCodeKeyNum[ERRCODE_MAX_LEN];
wiced_bt_gatt_status_t bt_adapter_last_error_get(debug_service_last_error_char_t *last_error_char)
{
    if (last_error_char == NULL)
    {
        return WICED_BT_GATT_ERROR;
    }

    sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);

	debug_service_last_error_char_t *pErrCode = get_errCode_info_handle();

	if (pErrCode->error_code == 0)			// 若无错误，必须更新时间戳
	{
		ptIoDev rtcDev = io_dev_get_dev(DEV_RTC);
		rtcDev->read((uint8_t *)&(pErrCode->timestamp), 4);
	}

	memcpy(last_error_char, pErrCode, sizeof(debug_service_last_error_char_t));	
	return WICED_BT_GATT_SUCCESS;
}

/*
 * Function Name: bt_adapter_last_error_change_happened
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_handler.
 *
 * @param  None
 *
 * @return STATUS  (true or false)
 */
bool bt_adapter_last_error_change_happened(debug_service_last_error_char_t *last_error_char)
{
    if (last_error_char == NULL)
    {
        return false;
    }

    // sm_log(SM_LOG_DEBUG, "%s\n", __FUNCTION__);
	
	debug_service_last_error_char_t *pErrCode = get_errCode_info_handle();

	if (get_last_error_status_change() == false)	// 无新错误
	{
		return false;
	}
	else
	{
		memcpy(last_error_char, pErrCode, sizeof(debug_service_last_error_char_t)); 
//		sm_log(SM_LOG_DEBUG, "err Code = %d!\n\n\n", err_code);
		return true;
	}
}



// 增加离线读写Last Error
bool g_updateLastError = false;

bool get_update_last_error(void)
{
    return g_updateLastError;
}

void set_update_last_error(bool status)
{
    g_updateLastError = status;
}

void get_last_error_from_eeprom(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->last_error;

	if (0xA55A != sParam->cal)
	{
		memset(sParam->dat, 0, sizeof(sParam->dat));
	}

//	debug_service_last_error_char_t *ptr = (debug_service_last_error_char_t *)(&sParam->dat);
//	sm_log(SM_LOG_DEBUG, "err Code = %d\t%d!\n\n\n", ptr->error_code, ptr->timestamp);
}

void last_error_update_to_eeprom(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	Param_t * sParam = (Param_t *)pBleData->last_error;

	sParam->cal = 0xA55A;

//	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
	set_update_BleData_status(1);
}


/*
 * Function Name: bt_adapter_uimage_payload_upgrade
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status (WICED_BT_GATT_SUCCESS or WICED_BT_GATT_ERROR)
 */ 
wiced_bt_gatt_status_t bt_adapter_uimage_payload_upgrade(uint32_t uimage_writeAddr, uint8_t* pBuf, size_t len)
{
    if ((uimage_writeAddr < UIMAGE_START_ADDR)||(uimage_writeAddr >= UIMAGE_END_ADDR)||(pBuf == NULL))//addr range judge
    {
        return WICED_BT_GATT_ERROR;
    }

    //判断数据是否被写满(4K)
    if (uimage_writeAddr % 4096 == 0)
    {
        uimage_record_erase_sector(uimage_writeAddr,4096);//擦除 4 K, 将要写入的新扇区
    }

    sm_log(SM_LOG_DEBUG, "%s uimage_writeAddr:0x%08x len:%ld\n", __FUNCTION__, uimage_writeAddr, len);

	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	ptQspiflash.addr = uimage_writeAddr;//UI IMAGE SAVED ADDR
	ptQspiflash.data = pBuf;
	ptQspiflash.len = len;
	qspiflashDev->write( (uint8_t*)&ptQspiflash, len);
	
	return WICED_BT_GATT_SUCCESS;
}

static uint32_t uimage_Intro_len = 0;
static uint32_t uimage_Greeting_len = 0;
static uint32_t uimage_Outro_len = 0;
/*
 * Function Name: bt_adapter_uimage_Intro Greeting Outro Manage
 * 
 *
 * Function Description:
 * @brief  The function is invoked by xx_read_handler.
 *
 * @param  None
 *
 * @return None
 */ 
void bt_adapter_user_uimage_Intro_length_set(uint32_t length)
{
   uimage_Intro_len= length;
}
uint32_t bt_adapter_user_uimage_Intro_length_get(void)
{
	return uimage_Intro_len;
}

void bt_adapter_user_uimage_Greeting_length_set(uint32_t length)
{
   uimage_Greeting_len= length;
}
uint32_t bt_adapter_user_uimage_Greeting_length_get(void)
{
	return uimage_Greeting_len;
}

void bt_adapter_user_uimage_Outro_length_set(uint32_t length)
{
   uimage_Outro_len= length;
}
uint32_t bt_adapter_user_uimage_Outro_length_get(void)
{
	return uimage_Outro_len;
}

bool MyName_uImage_Intro_isExisted(void)
{
	uint8_t protobuf[64]={0};
	uint16_t protobufLen =0;
	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	/********************************************* uimage intro******************************************** */
	ptQspiflash.addr = USER_UIMAGE_INTRO_START_ADDR;
	ptQspiflash.data = protobuf;
	ptQspiflash.len = sizeof(protobuf);
	qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(protobuf));

	// sm_log(SM_LOG_INFO, "\n protobuf(%d): \r\n",sizeof(protobuf));
	// print_array(&protobuf[0], sizeof(protobuf));

    protobufLen = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(protobuf[0], protobuf[1]);

	bool status;

	status = uimage_screen_update_nanopb_decode(&protobuf[2], protobufLen, NULL);
    sm_log(SM_LOG_DEBUG, "%s uImageIntroDecode: %d\r\n", __FUNCTION__, status);

	return status;
}

bool MyName_uImage_Greeting_isExisted(void)
{
	uint8_t protobuf[64]={0};
	uint16_t protobufLen =0;
	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	/********************************************* uimage greeting******************************************** */
	ptQspiflash.addr = USER_UIMAGE_GREETING_START_ADDR;
	ptQspiflash.data = protobuf;
	ptQspiflash.len = sizeof(protobuf);
	qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(protobuf));

	// sm_log(SM_LOG_INFO, "\n protobuf(%d): \r\n",sizeof(protobuf));
	// print_array(&protobuf[0], sizeof(protobuf));

    protobufLen = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(protobuf[0], protobuf[1]);

	bool status;

	status = uimage_screen_update_nanopb_decode(&protobuf[2], protobufLen, NULL);
    sm_log(SM_LOG_DEBUG, "%s uImageIntroDecode: %d\r\n", __FUNCTION__, status);

	return status;
}

bool MyName_uImage_Outro_isExisted(void)
{
	uint8_t protobuf[64]={0};
	uint16_t protobufLen =0;
	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	/********************************************* uimage outro*********************************************** */
	ptQspiflash.addr = USER_UIMAGE_OUTRO_START_ADDR;
	ptQspiflash.data = protobuf;
	ptQspiflash.len = sizeof(protobuf);
	qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(protobuf));

	// sm_log(SM_LOG_INFO, "\n protobuf(%d): \r\n",sizeof(protobuf));
	// print_array(&protobuf[0], sizeof(protobuf));

    protobufLen = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(protobuf[0], protobuf[1]);

	bool status;

	status = uimage_screen_update_nanopb_decode(&protobuf[2], protobufLen, NULL);
    sm_log(SM_LOG_DEBUG, "%s uImageIntroDecode: %d\r\n", __FUNCTION__, status);

	return status;
}

static uint8_t g_updateUimage = 0xFF;
void MyName_uImage_isUpdating_Set(uint32_t uimage_addr)
{
	if (USER_UIMAGE_INTRO_START_ADDR == uimage_addr)
	{
		g_updateUimage = 0;
	}
	else if (USER_UIMAGE_GREETING_START_ADDR == uimage_addr)
	{
		g_updateUimage = 1;
	}
	else if (USER_UIMAGE_OUTRO_START_ADDR == uimage_addr)
	{
		g_updateUimage = 2;
	}
	else
	{
		g_updateUimage = 0xFF;
	}
}

uint8_t MyName_uImage_isUpdating_Get(void)
{
	return g_updateUimage;
}

uint32_t MyName_uImage_Intro_Addr_Get(void)
{
	uint8_t protobuf[32]={0};
	uint32_t protobufLen =0;
	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	ptQspiflash.addr = USER_UIMAGE_INTRO_START_ADDR;
	ptQspiflash.data = protobuf;
	ptQspiflash.len = sizeof(protobuf);
	qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(protobuf));

	// sm_log(SM_LOG_INFO, "\n protobuf(%d): \r\n",sizeof(protobuf));
	// print_array(&protobuf[0], sizeof(protobuf));

    protobufLen = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(protobuf[0], protobuf[1]);

    sm_log(SM_LOG_DEBUG, "%s Intro_Addr: 0x%08X\r\n", __FUNCTION__, (USER_UIMAGE_INTRO_START_ADDR+protobufLen+2));

	return (USER_UIMAGE_INTRO_START_ADDR+protobufLen+2);
}

uint32_t MyName_uImage_Greeting_Addr_Get(void)
{
	uint8_t protobuf[32]={0};
	uint32_t protobufLen =0;
	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	ptQspiflash.addr = USER_UIMAGE_GREETING_START_ADDR;
	ptQspiflash.data = protobuf;
	ptQspiflash.len = sizeof(protobuf);
	qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(protobuf));

	// sm_log(SM_LOG_INFO, "\n protobuf(%d): \r\n",sizeof(protobuf));
	// print_array(&protobuf[0], sizeof(protobuf));

    protobufLen = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(protobuf[0], protobuf[1]);

    sm_log(SM_LOG_DEBUG, "%s Greeting_Addr: 0x%08X\r\n", __FUNCTION__, (USER_UIMAGE_GREETING_START_ADDR+protobufLen+2));

	return (USER_UIMAGE_GREETING_START_ADDR+protobufLen+2);
}

uint32_t MyName_uImage_Outro_Addr_Get(void)
{
	uint8_t protobuf[32]={0};
	uint32_t protobufLen =0;
	ptIoDev qspiflashDev = io_dev_get_dev(DEV_QSPIFLASH);
	Qspiflash_t ptQspiflash;

	ptQspiflash.addr = USER_UIMAGE_OUTRO_START_ADDR;
	ptQspiflash.data = protobuf;
	ptQspiflash.len = sizeof(protobuf);
	qspiflashDev->read( (uint8_t*)&ptQspiflash, sizeof(protobuf));

	// sm_log(SM_LOG_INFO, "\n protobuf(%d): \r\n",sizeof(protobuf));
	// print_array(&protobuf[0], sizeof(protobuf));

    protobufLen = APP_BT_MERGE_2BYTES_BIG_ENDDIAN(protobuf[0], protobuf[1]);

    sm_log(SM_LOG_DEBUG, "%s Outro_Addr: 0x%08X\r\n", __FUNCTION__, (USER_UIMAGE_OUTRO_START_ADDR+protobufLen+2));

	return (USER_UIMAGE_OUTRO_START_ADDR+protobufLen+2);
}

static bool g_updateBleData = false;
bool get_update_BleData_status(void)
{
    return g_updateBleData;
}
void set_update_BleData_status(bool status)
{
    g_updateBleData = status;
}

void BleData_update_to_flash(void)
{
	Ble_Flash_Data_t * pBleData = get_ble_flash_data_handle();
	app_param_write(FLASH_DATA_AREA_BLE, (uint8_t *)pBleData, sizeof(Ble_Flash_Data_t));
}


void flash_data_save_change(bool bNoTimeStamp)
{
// error code
	if (true == get_update_errcode_status()) {
		errcode_update_to_flash();
		set_update_errcode_status(false);
	}
// session time
	if (true == get_update_session_time_sum_status()) {
		set_update_session_time_sum_status(false);
		session_time_sum_update_to_flash();
	}
// time stamp
	if (bNoTimeStamp == 0)
		{set_time_stamp_to_flash();}
// lifecycle
	if (true == get_update_lifecycle_status()) {
		lifecycle_update_to_flash();
		set_update_lifecycle_status(false);
	}
// 以下为蓝牙相关数据
	if (true == get_update_lock_status()) {
		lockStatus_update_to_eeprom();
		set_update_lock_status(false);
	}
	if (true == get_update_last_error()) {
		last_error_update_to_eeprom();
		set_update_last_error(false);
	}
    if (true == get_update_BleData_status()) {		// 此语句需要最后执行，前面Lock和Last_Err并不真正写入Flash，只设置标志
    	BleData_update_to_flash();
    	set_update_BleData_status(false);
    }
}
/* [] END OF FILE */
