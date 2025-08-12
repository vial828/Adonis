
#include "driver_mcu_flash.h"

#include "driver_param.h"
#include "cy_flash.h"

McuFlashDataType_t *g_ptFlash = NULL;


int driver_mcu_flash_init(void)
{
#if 0
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_INV_PROT                 %x\r\n", CY_FLASH_DRV_INV_PROT                );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_INVALID_FM_PL            %x\r\n", CY_FLASH_DRV_INVALID_FM_PL           );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_INVALID_FLASH_ADDR       %x\r\n", CY_FLASH_DRV_INVALID_FLASH_ADDR      );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_ROW_PROTECTED            %x\r\n", CY_FLASH_DRV_ROW_PROTECTED           );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_IPC_BUSY                 %x\r\n", CY_FLASH_DRV_IPC_BUSY                );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_INVALID_INPUT_PARAMETERS %x\r\n", CY_FLASH_DRV_INVALID_INPUT_PARAMETERS);
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_PL_ROW_COMP_FA           %x\r\n", CY_FLASH_DRV_PL_ROW_COMP_FA          );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_ERR_UNC                  %x\r\n", CY_FLASH_DRV_ERR_UNC                 );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_PROGRESS_NO_ERROR        %x\r\n", CY_FLASH_DRV_PROGRESS_NO_ERROR       );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_OPERATION_STARTED        %x\r\n", CY_FLASH_DRV_OPERATION_STARTED       );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_OPCODE_BUSY              %x\r\n", CY_FLASH_DRV_OPCODE_BUSY             );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_CHECKSUM_NON_ZERO        %x\r\n", CY_FLASH_DRV_CHECKSUM_NON_ZERO       );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_NO_ERASE_SUSPEND         %x\r\n", CY_FLASH_DRV_NO_ERASE_SUSPEND        );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_FLASH_NOT_ERASED         %x\r\n", CY_FLASH_DRV_FLASH_NOT_ERASED        );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_NO_ERASE_ONGOING         %x\r\n", CY_FLASH_DRV_NO_ERASE_ONGOING        );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_ACTIVE_ERASE             %x\r\n", CY_FLASH_DRV_ACTIVE_ERASE            );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_INVALID_DATA_WIDTH       %x\r\n", CY_FLASH_DRV_INVALID_DATA_WIDTH      );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_FLASH_SAFTEY_ENABLED     %x\r\n", CY_FLASH_DRV_FLASH_SAFTEY_ENABLED    );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_INVALID_SFLASH_ADDR      %x\r\n", CY_FLASH_DRV_INVALID_SFLASH_ADDR     );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_SFLASH_BACKUP_ERASED     %x\r\n", CY_FLASH_DRV_SFLASH_BACKUP_ERASED    );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_SECTOR_SUSPEND           %x\r\n", CY_FLASH_DRV_SECTOR_SUSPEND          );
    sm_log(SM_LOG_ERR, "CY_FLASH_DRV_SROM_API_TIMEOUT         %x\r\n", CY_FLASH_DRV_SROM_API_TIMEOUT        );
#endif
    return RET_SUCCESS;
}


int driver_mcu_flash_deinit(void)
{
    return RET_SUCCESS;
}

int driver_mcu_flash_write(uint8_t *pBuf, uint16_t len)
{
    uint8_t temBuf[MCU_FLASH_ROW_SIZE] = {0};
    const uint32_t * row_ptr = NULL;
    cy_en_flashdrv_status_t enStatus;
    g_ptFlash = (McuFlashDataType_t*)pBuf;
    if(g_ptFlash == NULL || (g_ptFlash->size) <= 0)
    {
        return RET_FAILURE;
    }
    int rowNum = 0;
    int j = 0;
    while (j < len) {
        row_ptr = (const uint32_t *) temBuf;
        for (int i = 0; i < MCU_FLASH_ROW_SIZE; i++) {
            temBuf[i] = g_ptFlash->pData[j++];
            if (j >= len) {
                break;
            }
        }
        // 擦除 API(底层写有擦除？)
    //  cy_en_flashdrv_status_t enStatus1 = Cy_Flash_EraseRow(g_ptFlash->addr);
        enStatus = Cy_Flash_WriteRow(g_ptFlash->addr + (rowNum * MCU_FLASH_ROW_SIZE), row_ptr);
        rowNum++;
        if (enStatus != 0) {
            break;
        }
    }
    return enStatus;
}

int driver_mcu_flash_read(uint8_t *pBuf, uint16_t len)
{
    uint32_t programAddr;
    uint32_t i = 0;
    g_ptFlash = (McuFlashDataType_t*)pBuf;

    if(g_ptFlash == NULL || (g_ptFlash->size) <= 0)
    {
        return RET_FAILURE;
    }
    programAddr = g_ptFlash->addr;
    // 此处添加写进FLASH API接口
    while (i < len)
    {
        uint32_t val = *(__IO uint32_t *)programAddr;
        if (i + 4 > len) { // 防止buf访问越界
          memcpy(&g_ptFlash->pData[i], (uint8_t*)&val, len - i);
		  break;							  // 20241015 防止pBuf溢出
        } else {
          memcpy(&g_ptFlash->pData[i], (uint8_t*)&val, sizeof(uint32_t));
        }
        i += 4;
        programAddr = programAddr + 4;
    }
    return RET_SUCCESS;
}


