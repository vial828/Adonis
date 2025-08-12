
#include "driver_mcu_eeprom.h"



 /* The size of data to store in EEPROM. Note flash size used will be
  * closest multiple of flash row size */
#define EM_EEPROM_DATA_SIZE                       (CY_EM_EEPROM_FLASH_SIZEOF_ROW*2u)
 
 /* The Simple Mode is turned off */
#define SIMPLE_MODE                     (0u)
 
 /* Increases the flash endurance twice */
#define WEAR_LEVELLING_FACTOR           (1u)
 
 /* The Redundant Copy is turned off */
#define REDUNDANT_COPY                  (0u)
 
 /* The Blocking Write is turned on */
#define BLOCKING_WRITE                  (1u)/* 阻塞写开启 */

 /* Set the macro FLASH_REGION_TO_USE to either USER_FLASH or
  * EMULATED_EEPROM_FLASH to specify the region of the flash used for
  * emulated EEPROM.
  */
#define USER_FLASH              (0u)
#define EMULATED_EEPROM_FLASH   (1u)
 
#if CY_EM_EEPROM_SIZE
 /* CY_EM_EEPROM_SIZE to determine whether the target has a dedicated EEPROM region or not */
#define FLASH_REGION_TO_USE     EMULATED_EEPROM_FLASH
#else
#define FLASH_REGION_TO_USE     USER_FLASH
#endif


/* EEPROM configuration and context structure. */
cy_stc_eeprom_config_t Em_EEPROM_config =
{
        .eepromSize = EEPROM_SIZE,
        .simpleMode = SIMPLE_MODE,
        .blockingWrite = BLOCKING_WRITE,
        .redundantCopy = REDUNDANT_COPY,
        .wearLevelingFactor = WEAR_LEVELLING_FACTOR,
};


cy_stc_eeprom_context_t Em_EEPROM_context;

static SemaphoreHandle_t xSemaphore_E2P;



#if (EMULATED_EEPROM_FLASH == FLASH_REGION_TO_USE)
CY_SECTION(".cy_em_eeprom")
#endif /* #if(FLASH_REGION_TO_USE) */
CY_ALIGN(CY_EM_EEPROM_FLASH_SIZEOF_ROW)

#if (defined(CY_DEVICE_SECURE) && (USER_FLASH == FLASH_REGION_TO_USE ))
/* When CY8CKIT-064B0S2-4343W is selected as the target and EEPROM array is
 * stored in user flash, the EEPROM array is placed in a fixed location in
 * memory. The adddress of the fixed location can be arrived at by determining
 * the amount of flash consumed by the application. In this case, the example
 * consumes approximately 104000 bytes for the above target using GCC_ARM 
 * compiler and Debug configuration. The start address specified in the linker
 * script is 0x10000000, providing an offset of approximately 32 KB, the EEPROM
 * array is placed at 0x10021000 in this example. Note that changing the
 * compiler and the build configuration will change the amount of flash
 * consumed. As a resut, you will need to modify the value accordingly. Among
 * the supported compilers and build configurations, the amount of flash
 * consumed is highest for GCC_ARM compiler and Debug build configuration.
 */
#define APP_DEFINED_EM_EEPROM_LOCATION_IN_FLASH  (0x10021000)
#else
/* EEPROM storage in user flash or emulated EEPROM flash. */
const uint8_t eeprom_storage[CY_EM_EEPROM_GET_PHYSICAL_SIZE(EEPROM_SIZE, SIMPLE_MODE, WEAR_LEVELLING_FACTOR, REDUNDANT_COPY)] = {0u};

#endif /* #if (defined(CY_DEVICE_SECURE)) */








int driver_mcu_eeprom_init(void)
{
    cy_en_em_eeprom_status_t result;


        /* Initialize the flash start address in EEPROM configuration structure. */
#if (defined(CY_DEVICE_SECURE) && (USER_FLASH == FLASH_REGION_TO_USE ))
        Em_EEPROM_config.userFlashStartAddr = (uint32_t) APP_DEFINED_EM_EEPROM_LOCATION_IN_FLASH;
#else
        Em_EEPROM_config.userFlashStartAddr = (uint32_t) eeprom_storage;
#endif

    


    
    result = Cy_Em_EEPROM_Init(&Em_EEPROM_config, &Em_EEPROM_context);
    
     if (CY_EM_EEPROM_SUCCESS != result)
     {
         sm_log(SM_LOG_ERR,"mcu eeprom initialize error! \r\n");
         return RET_FAILURE;
     } 

	// Create I2C Mutex
	xSemaphore_E2P = xSemaphoreCreateMutex();
    
	if(xSemaphore_E2P == NULL)
	{
		sm_log(SM_LOG_ERR,"xSemaphore_E2P creat failure! \r\n");
	}else{
		xSemaphoreGive(xSemaphore_E2P); 
		sm_log(SM_LOG_INFO,"xSemaphore_E2P  creat success! \r\n");
	}

    sm_log(SM_LOG_INFO,"mcu eeprom initialize success! \r\n");
    return RET_SUCCESS;
}


int driver_mcu_eeprom_deinit(void)
{
	xSemaphoreTake(xSemaphore_E2P,portMAX_DELAY);


	//xSemaphoreGive(xSemaphore_E2P);


	vSemaphoreDelete(xSemaphore_E2P);

    return RET_SUCCESS;
}

int driver_mcu_eeprom_write(uint8_t *pBuf, uint16_t len)
{

    cy_en_em_eeprom_status_t result;
    E2PDataType_t* pTemp = (E2PDataType_t*)pBuf;
    


	if(pTemp == NULL || (pTemp->addr+pTemp->size) >= EEPROM_SIZE)
	{
	    sm_log(SM_LOG_ERR,"mcu eeprom write param error! \r\n");
        return RET_FAILURE;
	}
    
	xSemaphoreTake(xSemaphore_E2P,portMAX_DELAY);

	result = Cy_Em_EEPROM_Write(pTemp->addr, pTemp->pDate, pTemp->size, &Em_EEPROM_context);
	if (CY_EM_EEPROM_SUCCESS != result)
	{			
        sm_log(SM_LOG_ERR,"mcu eeprom write error! 0x%x \r\n",result);
        xSemaphoreGive(xSemaphore_E2P);
        return RET_FAILURE;
	}
    
	xSemaphoreGive(xSemaphore_E2P);

    return RET_SUCCESS;
}

int driver_mcu_eeprom_read(uint8_t *pBuf, uint16_t len)
{

    cy_en_em_eeprom_status_t result;
    E2PDataType_t* pTemp = (E2PDataType_t*)pBuf;
	if(pTemp == NULL || (pTemp->addr+pTemp->size) >= EEPROM_SIZE)
	{
	    sm_log(SM_LOG_ERR,"mcu eeprom read param error! %d, %d\r\n", (pTemp->addr+pTemp->size), EEPROM_SIZE);
        return RET_FAILURE;
	}
	xSemaphoreTake(xSemaphore_E2P,portMAX_DELAY);

	result = Cy_Em_EEPROM_Read(pTemp->addr, pTemp->pDate, pTemp->size, &Em_EEPROM_context);
	if (CY_EM_EEPROM_SUCCESS != result)
	{
        sm_log(SM_LOG_ERR,"mcu eeprom read error! 0x%x \r\n",result);
        xSemaphoreGive(xSemaphore_E2P);
        return RET_FAILURE;
	}
	xSemaphoreGive(xSemaphore_E2P);

    return RET_SUCCESS;
}










