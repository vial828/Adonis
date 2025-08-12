#include "driver_i2c.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* I2C bus frequency */
#define I2C_FREQ_400K                (400000UL)
#define I2C_FREQ_300K                (300000UL)
#define I2C_FREQ_200K                (200000UL)
#define I2C_FREQ_100K                (100000UL)

#define CYBSP_2V8_EN_PIN 		(P5_6)  // V03

const cyhal_gpio_t i2c_pin_sda[I2C_BUS_NUM] = 
{
	CYBSP_I2C_BUS0_SDA,
	CYBSP_I2C_BUS1_SDA,	
};

const cyhal_gpio_t i2c_pin_scl[I2C_BUS_NUM] = 
{
	CYBSP_I2C_BUS0_SCL,
	CYBSP_I2C_BUS1_SCL,	
};


const cyhal_i2c_cfg_t i2c_cfg[I2C_BUS_NUM] = 
{
	{
		.is_slave =false,
		.address = 0,
		.frequencyhal_hz = I2C_FREQ_300K,
	},
	{
		.is_slave =false,
		.address = 0,
		.frequencyhal_hz = I2C_FREQ_300K,
	},
};

typedef struct
{
    cyhal_i2c_t mI2cBus[I2C_BUS_NUM];
	SemaphoreHandle_t xSemaphore_I2C[I2C_BUS_NUM];
}I2cObject_t;

static I2cObject_t gs_I2c;

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

static void i2x_mDelay(uint16_t ms)
{
	vTaskDelay(ms);
}

/**
  * @brief  I2C module initialize
  * @param  none
  * @return None
  * @note   None
  */
void i2c_initialize(I2cBus_e bus)
{
	cy_rslt_t result;

	CY_ASSERT(bus < I2C_BUS_NUM);


	/* Init I2C master */
	result = cyhal_i2c_init(&gs_I2c.mI2cBus[bus], i2c_pin_sda[bus], i2c_pin_scl[bus], NULL);
	/* I2C master init failed. Stop program execution */
	CY_ASSERT(result == CY_RSLT_SUCCESS);

	/* Configure I2C Master */
	result = cyhal_i2c_configure(&gs_I2c.mI2cBus[bus], &i2c_cfg[bus]);
	/* I2C master configuration failed. Stop program execution */
	CY_ASSERT(result == CY_RSLT_SUCCESS);

	// Create I2C Mutex
	gs_I2c.xSemaphore_I2C[bus] = xSemaphoreCreateMutex();
    
	if(gs_I2c.xSemaphore_I2C[bus] == NULL)
	{
		sm_log(SM_LOG_ERR,"gs_I2c.xSemaphore_I2C %d creat failure! \r\n",bus);
	}else{
		xSemaphoreGive(gs_I2c.xSemaphore_I2C[bus]); 
		sm_log(SM_LOG_INFO,"gs_I2c.xSemaphore_I2C %d creat success! \r\n",bus);
	}
}

/**
  * @brief  I2C module deinitialize
  * @param  I2cBus_e  I2cBus NO.
  * @return None
  * @note   None
  */
void i2c_deinitialize(I2cBus_e bus)
{
	cy_rslt_t result;

	CY_ASSERT(bus < I2C_BUS_NUM);
    cyhal_i2c_free(&gs_I2c.mI2cBus[bus]);
    vSemaphoreDelete(gs_I2c.xSemaphore_I2C[bus]);
}


/****************************************************************************************
* @brief   driver i2c init
* @param   none
* @return 	none
* @note    
****************************************************************************************
**/
int driver_i2c_init(void)
{
	cyhal_gpio_init(CYBSP_2V8_EN_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
	cyhal_gpio_write(CYBSP_2V8_EN_PIN, 0);  // 2.8V 使能
	sm_log(SM_LOG_INFO,"Enable 2.8V control! \r\n");

    sm_log(SM_LOG_INFO,"I2C_BUS0 init start! \r\n");
    cyhal_gpio_free(P6_0);
    cyhal_gpio_free(P6_1);

	i2c_initialize(I2C_BUS0);
	
    sm_log(SM_LOG_INFO,"I2C_BUS0 init success! \r\n");

    sm_log(SM_LOG_INFO,"I2C_BUS1 init start! \r\n");

    cyhal_gpio_free(P6_4);
    cyhal_gpio_free(P6_5);

    i2c_initialize(I2C_BUS1);
	
    sm_log(SM_LOG_INFO,"I2C_BUS1 init success! \r\n");

	return RET_SUCCESS;
}



/****************************************************************************************
* @brief   driver i2c init
* @param   none
* @return 	none
* @note    
****************************************************************************************
**/

int driver_i2c_deinit(void)
{
	/* power low config here */
    i2c_deinitialize(I2C_BUS0);
    i2c_deinitialize(I2C_BUS1);
    i2x_mDelay(11); // POA 版本，显示屏VDDI要比VDD晚10ms下电， 显示屏先去初始化VDD先下电
    cyhal_gpio_init(CYBSP_2V8_EN_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_write(CYBSP_2V8_EN_PIN, 1);//2V8EN PMOS关闭
    cyhal_gpio_init(P6_0, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(P6_1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(P6_4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    cyhal_gpio_init(P6_5, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
	return RET_SUCCESS;

}


/****************************************************************************************
* @brief   Write data by i2c
* @param   pBuf 		is the data to be written
		   len 			no use
* @return 0 on success 
* @note    
****************************************************************************************
**/ 
int driver_i2c_write(uint8_t *pBuf, uint16_t len)
{
	I2cDataType_t* pTemp = NULL;
	cy_rslt_t relt = CY_RSLT_SUCCESS;

    if (pBuf == NULL) {
        return RET_FAILURE;
    }

	pTemp = (I2cDataType_t*)pBuf;
	xSemaphoreTake(gs_I2c.xSemaphore_I2C[pTemp->busN],portMAX_DELAY);
	
	relt = cyhal_i2c_master_write(&gs_I2c.mI2cBus[pTemp->busN], pTemp->devAddr,pTemp->wDat, pTemp->wLen, I2C_WR_TIMEROUT, true);
    if(relt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_ERR,"I2C write error device_address = 0x%x! \r\n",pTemp->devAddr);
    }

	xSemaphoreGive(gs_I2c.xSemaphore_I2C[pTemp->busN]);
	return (relt == CY_RSLT_SUCCESS)?RET_SUCCESS:RET_FAILURE;

}


/****************************************************************************************
* @brief   	Read data by i2c
* @param 	pBuf 		is the data to be readed to
			len 		no use
* @return 0 on success 
* @note    
****************************************************************************************
**/ 
int driver_i2c_read(uint8_t *pBuf, uint16_t len)
{
	I2cDataType_t* pTemp = NULL;
	cy_rslt_t relt = CY_RSLT_SUCCESS;

	if (pBuf == NULL) {
        return RET_FAILURE;
    }
	pTemp = (I2cDataType_t*)pBuf;
	xSemaphoreTake(gs_I2c.xSemaphore_I2C[pTemp->busN],portMAX_DELAY);
	
	if(pTemp->wLen)
	{
		relt = cyhal_i2c_master_write(&gs_I2c.mI2cBus[pTemp->busN], pTemp->devAddr,pTemp->wDat, pTemp->wLen, I2C_WR_TIMEROUT, true);
	}
    
    if(relt != CY_RSLT_SUCCESS)
    {
        sm_log(SM_LOG_ERR,"I2C write error device_address = 0x%x! \r\n",pTemp->devAddr);
    }

	if((relt == CY_RSLT_SUCCESS) || (pTemp->wLen == 0))
	{
		relt = cyhal_i2c_master_read(&gs_I2c.mI2cBus[pTemp->busN], pTemp->devAddr,pTemp->rDat,pTemp->rLen, I2C_WR_TIMEROUT, true);
        if(relt != CY_RSLT_SUCCESS)
        {
            sm_log(SM_LOG_ERR,"I2C read error device_address = 0x%x! \r\n",pTemp->devAddr);
        }
	}
	
	xSemaphoreGive(gs_I2c.xSemaphore_I2C[pTemp->busN]);
	return (relt == CY_RSLT_SUCCESS)?RET_SUCCESS:RET_FAILURE;//sucess 0,failure -1

}


