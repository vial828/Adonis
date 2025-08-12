//-----------------------------------------------------------------------------------------------------------------------
//---------------------------for new--------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------


#include "driver_uart_usb.h"
#include "cy_pdl.h"
#include "cycfg.h"


uint8_t usb_rx_buf[UART_USB_RX_LEN_MAX];	//
uint8_t usb_tx_buf[UART_USB_LEN_MAX];	//

#define MAX_DMA_LOOP	256u

typedef struct
{
	bool isInit;
	
	uint16_t rxTimeout;        
	uint16_t rxLen; 
	uint16_t rxLenMask;
	uint8_t *pRxBuf;


	bool txIdleFlag;
	uint16_t txTimeout;        
	uint16_t txLen; 
	uint16_t txLenMask;
	uint8_t *pTxBuf;
	SemaphoreHandle_t xSemaphore_tx;
}usbObj_t;

static usbObj_t gs_usb;


static cy_stc_sysint_t UART_USB_INT_cfg =
{
	.intrSrc	  = UART_USB_IRQ,
	.intrPriority = 7u,
};

static cy_stc_sysint_t USB_TX_DMA_INT_cfg =
{
	.intrSrc	  = (IRQn_Type)TxDmaUsb_IRQ,
	.intrPriority = 7u,
};


cy_stc_scb_uart_context_t UART_USB_context;

__WEAK TaskHandle_t get_task_usb(void)
{
    return NULL;
}


void uart_usb_1ms_isr(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(gs_usb.rxTimeout<=0)
	{
		return ;
	}
	
	gs_usb.rxTimeout--;//通信超时计数
	
	if(gs_usb.rxTimeout == 0)
	{
		if(gs_usb.rxLen < UART_USB_LEN_MIN)
		{
			gs_usb.rxLen = 0;// 如果长度不够，认为是干扰帧，则重置
			return ;
		}

		/*send message to TASK*/
		if(get_task_usb() != NULL)
		{
            xTaskNotifyFromISR(get_task_usb(),USB_RX,eSetBits, &xHigherPriorityTaskWoken );//USB的接收
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );/* 如果为pdTRUE，则进行一次上下文切换 */
		}
	}

	
}



/*******************************************************************************
* Function Name: uart_pen_isr
********************************************************************************
*
* Summary:
* Handles UART Rx underflow and overflow conditions. This conditions must never
* occur.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void uart_usb_isr(void)
{
    uint32 intrSrcRx;
    uint32 intrSrcTx;
    uint32_t bytenum;

    /* Get RX interrupt sources */
    intrSrcRx = Cy_SCB_UART_GetRxFifoStatus(UART_USB_HW);

    if((intrSrcRx & CY_SCB_UART_RX_NOT_EMPTY) != 0)
	{
        /* Clear UART "RX fifo not empty interrupt" */
		bytenum = Cy_SCB_UART_GetNumInRxFifo(UART_USB_HW);
        /* Get the character from terminal */
		if((bytenum+gs_usb.rxLen) <= UART_USB_RX_LEN_MAX)
		{
			Cy_SCB_UART_GetArray(UART_USB_HW, &gs_usb.pRxBuf[gs_usb.rxLen], bytenum);
		}
//		gs_usb.rxLen = ((gs_usb.rxLen + bytenum)&gs_usb.rxLenMask);
		gs_usb.rxLen = (gs_usb.rxLen + bytenum) % (UART_USB_RX_LEN_MAX + 1);
		gs_usb.rxTimeout = UART_USB_TIMEOUT;
	}
	
    Cy_SCB_UART_ClearRxFifoStatus(UART_USB_HW, intrSrcRx);

    /* Get TX interrupt sources */
    intrSrcTx = Cy_SCB_UART_GetTxFifoStatus(UART_USB_HW);
    Cy_SCB_UART_ClearTxFifoStatus(UART_USB_HW, intrSrcTx);

    /* RX overflow or RX underflow or RX overflow occured */
    //uart_error = 1;
}


/*******************************************************************************
* Function Name: tx_dma_complete
********************************************************************************
*
* Summary:
*  Handles Tx Dma descriptor completion interrupt source: only used for
*  indication.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void usb_tx_dma_complete(void)
{	
    Cy_DMA_Channel_ClearInterrupt(TxDmaUsb_HW, TxDmaUsb_CHANNEL);
	
	if(gs_usb.xSemaphore_tx != NULL)
	{
		xSemaphoreGiveFromISR(gs_usb.xSemaphore_tx, NULL);
	}
}



/*******************************************************************************
* Function Name: configure_tx_dma
********************************************************************************
*
* Summary:
* Configures DMA Tx channel for operation.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void configure_usb_tx_dma(uint8_t* buffer_a, cy_stc_sysint_t* int_config)
{
    cy_en_dma_status_t dma_init_status;

    /* Init descriptor */
    dma_init_status = Cy_DMA_Descriptor_Init(&TxDmaUsb_Descriptor_0, &TxDmaUsb_Descriptor_0_config);
    if (dma_init_status!=CY_DMA_SUCCESS)
    {
        CY_ASSERT(0);
    }

    dma_init_status = Cy_DMA_Descriptor_Init(&TxDmaUsb_Descriptor_1, &TxDmaUsb_Descriptor_1_config);
    if (dma_init_status!=CY_DMA_SUCCESS)
    {
        CY_ASSERT(0);
    }

	
    dma_init_status = Cy_DMA_Channel_Init(TxDmaUsb_HW, TxDmaUsb_CHANNEL, &TxDmaUsb_channelConfig);
    if (dma_init_status!=CY_DMA_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Set source and destination for descriptor 1 */
    Cy_DMA_Descriptor_SetSrcAddress(&TxDmaUsb_Descriptor_0, (uint32_t *) buffer_a);
    Cy_DMA_Descriptor_SetDstAddress(&TxDmaUsb_Descriptor_0, (uint32_t *) &UART_USB_HW->TX_FIFO_WR);

    /* Set source and destination for descriptor 1 */
    Cy_DMA_Descriptor_SetSrcAddress(&TxDmaUsb_Descriptor_1, (uint32_t *) &buffer_a[MAX_DMA_LOOP]);
    Cy_DMA_Descriptor_SetDstAddress(&TxDmaUsb_Descriptor_1, (uint32_t *) &UART_USB_HW->TX_FIFO_WR);


    /* Set next descriptor to NULL to stop the chain execution after descriptor 1
    *  is completed.
    */
    
    Cy_DMA_Channel_SetDescriptor(TxDmaUsb_HW, TxDmaUsb_CHANNEL, &TxDmaUsb_Descriptor_0);
	Cy_DMA_Descriptor_SetNextDescriptor(&TxDmaUsb_Descriptor_0, NULL);

    /* Initialize and enable the interrupt from TxDma */
    Cy_SysInt_Init  (int_config, &usb_tx_dma_complete);
    NVIC_EnableIRQ(int_config->intrSrc);

    /* Enable DMA interrupt source */
    Cy_DMA_Channel_SetInterruptMask(TxDmaUsb_HW, TxDmaUsb_CHANNEL, CY_DMA_INTR_MASK);

    /* Enable Data Write block but keep channel disabled to not trigger
    *  descriptor execution because TX FIFO is empty and SCB keeps active level
    *  for DMA.
    */
    Cy_DMA_Enable(TxDmaUsb_HW);
}



//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

int driver_usb_init(void)
{
    // Variable Declarations
	cy_en_scb_uart_status_t init_status;
    
    cyhal_wdt_t wdtObj;
    cyhal_wdt_init(&wdtObj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdtObj); // 从mcuboot跳转过来，先禁止看门狗定时器，初始化完成后再开启
    
	gs_usb.isInit=false;

	gs_usb.rxTimeout = 0;        
	gs_usb.rxLen = 0; 
	gs_usb.rxLenMask = UART_USB_RX_LEN_MAX-1;
	gs_usb.pRxBuf = usb_rx_buf;

	gs_usb.txTimeout = 0;        
	gs_usb.txLen = 0; 
	gs_usb.txLenMask = UART_USB_LEN_MAX-1;
	gs_usb.pTxBuf = usb_tx_buf;
	gs_usb.xSemaphore_tx = xSemaphoreCreateBinary();

	// MCU APP启动后先禁止SWD，防止充电DPDM影响MCU启动
    Cy_GPIO_SetHSIOM(GPIO_PRT6, 7UL, HSIOM_SEL_GPIO);
    Cy_GPIO_SetDrivemode(GPIO_PRT6, 7UL, CY_GPIO_DM_ANALOG);
    Cy_GPIO_SetHSIOM(GPIO_PRT6, 6UL, HSIOM_SEL_GPIO);
    Cy_GPIO_SetDrivemode(GPIO_PRT6, 6UL, CY_GPIO_DM_ANALOG);
    // GPOUT脉冲,至少200us的高电平
	cyhal_gpio_init(P9_3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
	cyhal_gpio_write(P9_3, 1);

    Cy_GPIO_Pin_Init(UART_RX_USB_PORT, UART_RX_USB_PIN, &UART_RX_USB_config);
    Cy_GPIO_Pin_Init(UART_TX_USB_PORT, UART_TX_USB_PIN, &UART_TX_USB_config);

	/* Configure DMA  Tx channels for operation */
	configure_usb_tx_dma(gs_usb.pTxBuf, &USB_TX_DMA_INT_cfg);

	/* Initialize and enable interrupt from UART. The UART interrupt sources
	*	are enabled in the Component GUI */
	Cy_SysInt_Init(&UART_USB_INT_cfg, &uart_usb_isr);
	NVIC_EnableIRQ(UART_USB_INT_cfg.intrSrc);

	/* Start UART operation */
	init_status = Cy_SCB_UART_Init(UART_USB_HW, &UART_USB_config, &UART_USB_context);
	if (init_status!=CY_SCB_UART_SUCCESS)
	{
		CY_ASSERT(0);
	}

	Cy_SCB_UART_Enable(UART_USB_HW);
    
	if(gs_usb.xSemaphore_tx != NULL)
	{
		xSemaphoreGive(gs_usb.xSemaphore_tx); 
	}

	gs_usb.isInit=true;
	return (init_status==CY_SCB_UART_SUCCESS)?RET_SUCCESS:RET_FAILURE;
}




int driver_usb_deinit(void)
{
	gs_usb.isInit=false;
    Cy_SCB_UART_Disable(UART_USB_HW, &UART_USB_context);
    Cy_SCB_UART_DeInit(UART_USB_HW);
    NVIC_DisableIRQ(UART_USB_INT_cfg.intrSrc);
    Cy_DMA_Channel_DeInit(TxDmaUsb_HW, TxDmaUsb_CHANNEL);
    Cy_DMA_Descriptor_DeInit(&TxDmaUsb_Descriptor_0);
    Cy_DMA_Descriptor_DeInit(&TxDmaUsb_Descriptor_1);

    Cy_GPIO_Pin_FastInit(GPIO_PRT9, 0, CY_GPIO_DM_PULLDOWN_IN_OFF, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT9, 1, CY_GPIO_DM_PULLDOWN_IN_OFF, 0UL, HSIOM_SEL_GPIO);

    vSemaphoreDelete(gs_usb.xSemaphore_tx);

    return RET_SUCCESS;
}


int driver_usb_read(uint8_t *pBuf, uint16_t len)
{
	uint16_t retlen = 0;


	if(pBuf == NULL)
	{
		return -1;
	}

	if(gs_usb.rxTimeout == 0 )
	{
		if(gs_usb.rxLen > 0)
		{
			memcpy(pBuf,gs_usb.pRxBuf,gs_usb.rxLen);
			retlen = gs_usb.rxLen;
			gs_usb.rxLen = 0;
		}
	}
	
	return (int)retlen;
}

int driver_usb_write(uint8_t *pBuf, uint16_t len)
{
	uint32_t xLoopLen = 0;
	uint32_t yLoopLen = 0;

	if((pBuf == NULL)||(len == 0)||(len > UART_USB_LEN_MAX))
	{
		return RET_FAILURE;
	}
	xSemaphoreTake(gs_usb.xSemaphore_tx,portMAX_DELAY);
	memcpy(gs_usb.pTxBuf,pBuf,len);
	gs_usb.txLen = len;
	
	xLoopLen = (gs_usb.txLen>MAX_DMA_LOOP)?MAX_DMA_LOOP:gs_usb.txLen;
	yLoopLen = gs_usb.txLen/MAX_DMA_LOOP;


	Cy_DMA_Descriptor_SetXloopDataCount(&TxDmaUsb_Descriptor_0,xLoopLen);
	Cy_DMA_Descriptor_SetXloopSrcIncrement(&TxDmaUsb_Descriptor_0,1);
	
	Cy_DMA_Descriptor_SetSrcAddress(&TxDmaUsb_Descriptor_0, (uint32_t *)gs_usb.pTxBuf);
	Cy_DMA_Descriptor_SetDstAddress(&TxDmaUsb_Descriptor_0, (uint32_t *)&UART_USB_HW->TX_FIFO_WR);

	Cy_DMA_Descriptor_SetYloopDataCount(&TxDmaUsb_Descriptor_0,1);
	Cy_DMA_Descriptor_SetYloopSrcIncrement(&TxDmaUsb_Descriptor_0,xLoopLen);

	if(gs_usb.txLen<=MAX_DMA_LOOP)
	{
  		Cy_DMA_Descriptor_SetNextDescriptor(&TxDmaUsb_Descriptor_0, NULL);

	}else{
		Cy_DMA_Descriptor_SetNextDescriptor(&TxDmaUsb_Descriptor_0,&TxDmaUsb_Descriptor_1);

		Cy_DMA_Descriptor_SetXloopDataCount(&TxDmaUsb_Descriptor_1,(gs_usb.txLen-xLoopLen));
		Cy_DMA_Descriptor_SetXloopSrcIncrement(&TxDmaUsb_Descriptor_1,1);

		Cy_DMA_Descriptor_SetYloopDataCount(&TxDmaUsb_Descriptor_1,1);
		Cy_DMA_Descriptor_SetYloopSrcIncrement(&TxDmaUsb_Descriptor_1,(gs_usb.txLen-xLoopLen));

		Cy_DMA_Descriptor_SetSrcAddress(&TxDmaUsb_Descriptor_1, (uint8_t *)&gs_usb.pTxBuf[MAX_DMA_LOOP]);
		Cy_DMA_Descriptor_SetDstAddress(&TxDmaUsb_Descriptor_1, (uint32_t *)&UART_USB_HW->TX_FIFO_WR);

		
	}

	Cy_DMA_Channel_SetDescriptor(TxDmaUsb_HW, TxDmaUsb_CHANNEL, &TxDmaUsb_Descriptor_0);
	Cy_DMA_Channel_Enable(TxDmaUsb_HW, TxDmaUsb_CHANNEL);


	return RET_SUCCESS;

}

/**
  * @brief  判断驱动是否已执行去初始化
  * @param  None:
  * @return 0：去初始化，1：初始化
  * @note   None
  */
bool get_driver_status(void)
{
    return gs_usb.isInit;
}






