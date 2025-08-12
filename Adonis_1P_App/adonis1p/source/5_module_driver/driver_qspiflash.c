/**
  ******************************************************************************
  * @file    driver_qspiflash.c
  * @author  xuhua.huang@metextech.com
  * @date    2024/03/013
  * @version V0.01
  * @brief   Brief description.
  *
  *   Detailed description starts here.
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
  * 2024-03-013     V0.01      xuhua.huang@metextech.com    the first version
  *
  ******************************************************************************
  */

#include "stdio.h"
#include "driver_qspiflash.h"

#define ENABLE_DRIVER_QSPI

#include "cyhal.h"
#include "cybsp.h"
#include "cy_serial_flash_qspi.h"
//#include "bsp_spi_flash.h"
#include "cycfg_qspi_memslot.h"

#include "cy_serial_flash_qspi.h"
#include "cyhal_qspi.h"
#include "cy_trigmux.h"
#include "cy_utils.h"

#define CYBSP_QSPI_D3 (P11_3)
#define CYBSP_QSPI_D2 (P11_4)
#define CYBSP_QSPI_D1 (P11_5)
#define CYBSP_QSPI_D0 (P11_6)
#define CYBSP_QSPI_SCK (P11_7)
#define CYBSP_QSPI_SS (P11_2)

#define QSPI_BUS_FREQUENCY_HZ   (50000000lu)

#ifdef ENABLE_DRIVER_QSPI

#endif

typedef struct DrvQspiflash_t
{
    uint32_t addr;
    uint8_t *data;
    uint32_t len;
} DrvQspiflash_t;
/**
  * @brief  初始化qspiflash硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_qspiflash_init(void)
{
#ifdef ENABLE_DRIVER_QSPI
    cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0,
                  CYBSP_QSPI_D1, CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
                  CYBSP_QSPI_SCK, CYBSP_QSPI_SS, QSPI_BUS_FREQUENCY_HZ);
#endif
    return 0;
}

/**
  * @brief  去初始化qspiflash硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_qspiflash_deinit(void)
{
#ifdef ENABLE_DRIVER_QSPI
    cy_serial_flash_qspi_deinit();
    Cy_GPIO_Pin_FastInit(GPIO_PRT11, 2, CY_GPIO_DM_STRONG, 1UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT11, 3, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT11, 4, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT11, 5, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT11, 6, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT11, 7, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
#endif
return 0;
}

/**
  * @brief  向qspif flash写数据
  * @param  pBuf:   要写入的数据
            len:    要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_qspiflash_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_QSPI
    if(len==0)
    {
        return -1;
    }
    if(pBuf == NULL)
    {
        return -1;
    }
    DrvQspiflash_t *ptdrvQspiflash;
    ptdrvQspiflash = (DrvQspiflash_t *)pBuf;
    size_t sectorSize = cy_serial_flash_qspi_get_erase_size(ptdrvQspiflash->addr); // 4KB sector,64KB block
    uint32_t res1 = cy_serial_flash_qspi_erase(ptdrvQspiflash->addr, sectorSize);
    if (ptdrvQspiflash->len < sectorSize) {
        sectorSize = ptdrvQspiflash->len;
    }
    uint32_t res2 = cy_serial_flash_qspi_write(ptdrvQspiflash->addr, sectorSize, ptdrvQspiflash->data);
#endif
    if (res1 != 0 || res2 != 0) {
        return -1;
    }
    return 0;
}

/**
  * @brief  读qspi flash
  * @param  buf:要读入的数据
            len:要读入数据的长度
  * @return 0：成功，-1：失败
  * @note   
  */
int driver_qspiflash_read(uint8_t *pBuf, uint16_t len)
{
    // len要大于0且不能小于按键属性结构体的大小且是其整数倍
    if(len==0)
    {
        return -1;
    }
    if(pBuf == NULL)
    {
        return -1;
    }
    DrvQspiflash_t *ptdrvQspiflash;
    ptdrvQspiflash = (DrvQspiflash_t *)pBuf;
    uint32_t res = cy_serial_flash_qspi_read(ptdrvQspiflash->addr, ptdrvQspiflash->len, ptdrvQspiflash->data);
    if (res != 0) {
        return -1;
    }
    return 0;
}

/* Read Manufacturer and Device ID */
cy_en_smif_status_t qspi_read_flash_chip_id(uint8_t *id, uint16_t length)
{
	cy_en_smif_status_t status;
	cy_stc_smif_mem_config_t *memCfg;
	SMIF_Type *QSPIPort;
	cy_stc_smif_context_t *QSPI_context;
	uint16_t dummyCycles = 64u;

	memCfg = qspi_get_memory_config(0);
	QSPIPort = qspi_get_device();
	QSPI_context = qspi_get_context();

	status = Cy_SMIF_TransmitCommand(QSPIPort,
									(0x9FU), /* Read manufacturer and device identification */
									CY_SMIF_WIDTH_SINGLE,
									NULL,
									0u,
									CY_SMIF_WIDTH_SINGLE,
									memCfg->slaveSelect,
									CY_SMIF_TX_NOT_LAST_BYTE,
									QSPI_context);
	if(CY_SMIF_SUCCESS == status) {
		status = Cy_SMIF_ReceiveDataBlocking(QSPIPort, id, length,
									CY_SMIF_WIDTH_SINGLE,
									QSPI_context);
	}

	return status;
}

