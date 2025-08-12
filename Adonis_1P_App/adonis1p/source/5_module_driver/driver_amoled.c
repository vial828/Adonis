/**
  ******************************************************************************
  * @file    driver_amoled.c
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
#include <stdlib.h>
#include "driver_amoled.h"

#define ENABLE_DRIVER_AMOLED

#include "cyhal.h"
#include "sm_log.h"
#include "cycfg.h"

#define DRV_HOR_RES      90
#define DRV_VER_RES      282
#define DRV_COLOR_DEPTH  3   // 2:565  3: 888
#define DRV_IMAGE_SEZE  (DRV_HOR_RES * DRV_VER_RES * DRV_COLOR_DEPTH)
#define MAX_XLOOP       256
#define MAX_YLOOP       256
//#define CYBSP_SPI_MOSI  (P7_0)
//#define CYBSP_SPI_MISO  (P7_1)
//#define CYBSP_SPI_CLK   (P7_2)
//#define CYBSP_SPI_CS    (P7_3)
#define HW_AMOLED_DCX   (P7_4)
#define HW_AMOLED_RST   (P8_2)


#define HW_AMOLED_VCCEN     (P7_5)
#define HW_AMOLED_IOVCCEN   (P8_1)
#define HW_AMOLED_TE   (P7_7)


//#define BITS_PER_FRAME             (8)

//#define SPI_FREQ_HZ                (25000000UL)

//cyhal_spi_t mSPI;

typedef struct DrvAmoledArea_t{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} DrvAmoledArea_t;

typedef struct DrvAmoledInfo_t{
    DrvAmoledArea_t area;
    uint8_t *data;
    uint32_t len;
} DrvAmoledInfo_t;

typedef struct Dma2DInfo_t {
    uint32_t xloop;
    uint32_t yloop;
    uint32_t yincrement;
} Dma2DInfo_t;

volatile bool tx_dma_done = true;
uint8_t g_send_one_data = 0; // 发单独一个命令或者数据
static inline void write_data(uint8_t *data, uint32_t len);

#ifdef ENABLE_DRIVER_AMOLED

#endif

#if defined(BLE_PROFILES_PROTOCOL_SUPPORT)
static uint8_t	g_oLedBrigth = 100;

/**
  * @brief  获取亮度值
  * @param  None
  * @return 亮度值
  * @note   None
  */
uint8_t amoled_brigth_get(void)
{
    return g_oLedBrigth;
}

/**
  * @brief  设置亮度值
  * @param  None
  * @return None
  * @note   None
  */
void amoled_brigth_set(uint8_t dat)
{
    g_oLedBrigth = dat;
}
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

/*******************************************************************************
 * Sets value of the display Reset pin.
 *******************************************************************************/
void driver_rm69600_write_reset_pin(bool value)
{
    cyhal_gpio_write(HW_AMOLED_RST, value);
}

/*******************************************************************************
 * Writes one byte of data to the software i8080 interface with the LCD_DC pin
 *******************************************************************************/
void driver_rm69600_write_command(uint8_t data)
{
#ifdef ENABLE_DRIVER_AMOLED
    while(tx_dma_done==false);
    tx_dma_done= false;
    while (true == Cy_SCB_SPI_IsBusBusy(CYBSP_AMOLED_SPI_HW));
    g_send_one_data = data;
    cyhal_gpio_write(HW_AMOLED_DCX, 0u);
    write_data(&g_send_one_data, 1);
#endif
}

/*******************************************************************************
 * Writes one byte of data to the software i8080 interface with the LCD_DC pin
 *******************************************************************************/
void driver_rm69600_write_data(uint8_t data)
{
#ifdef ENABLE_DRIVER_AMOLED
    uint8_t sendData = data;
    while(tx_dma_done==false);
    tx_dma_done= false;
    while (true == Cy_SCB_SPI_IsBusBusy(CYBSP_AMOLED_SPI_HW));
    g_send_one_data = data;
    cyhal_gpio_write(HW_AMOLED_DCX, 1u);
    write_data(&g_send_one_data, 1);
#endif
}

// 写一个数据块
void driver_rm69600_write_data_block(uint8_t *data, uint32_t len)
{
#ifdef ENABLE_DRIVER_AMOLED
    while(tx_dma_done==false);
    tx_dma_done= false;
    while (true == Cy_SCB_SPI_IsBusBusy(CYBSP_AMOLED_SPI_HW));
    cyhal_gpio_write(HW_AMOLED_DCX, 1u);
    write_data(data, len);
#endif
}

void driver_amoled_dcx(uint8_t cmd_data)
{
    if(cmd_data==0)cyhal_gpio_write(HW_AMOLED_DCX, 0);
    else cyhal_gpio_write(HW_AMOLED_DCX, 1);
}


void driver_amoled_rst(uint8_t onoff)
{
    if(onoff==0)cyhal_gpio_write(HW_AMOLED_RST, 0);
    else cyhal_gpio_write(HW_AMOLED_RST, 1);
}

extern uint32_t get_ms_tick(void);

void driver_amoled_power_up(uint8_t onoff)
{
    uint32_t ticks;
    if(onoff==1) {
//        driver_rm69600_write_reset_pin(0);
        ticks = get_ms_tick();
        while (get_ms_tick() -ticks < 2);
        cyhal_gpio_write(HW_AMOLED_IOVCCEN, 1);
//        ticks = get_ms_tick();
//        while (get_ms_tick() -ticks < 20);
        cyhal_gpio_write(HW_AMOLED_VCCEN, 1);
        ticks = get_ms_tick();
        while (get_ms_tick() -ticks < 20);
//        driver_rm69600_write_reset_pin(1);
    }
    else {
        driver_rm69600_write_reset_pin(0);
        ticks = get_ms_tick();
        while (get_ms_tick() -ticks < 2);
        cyhal_gpio_write(HW_AMOLED_VCCEN, 0);
        ticks = get_ms_tick();
        while (get_ms_tick() -ticks < 20);
        cyhal_gpio_write(HW_AMOLED_IOVCCEN, 0);
    }
}

void handle_error(void)          // 异常处理后续需优化
{
     /* Disable all interrupts. */
    __disable_irq();

    /* Infinite loop. */
    while(1u) {}
}

/*******************************************************************************
 * Writes one byte of data to the software i8080 interface.
 *******************************************************************************/
static inline void write_data(uint8_t *data, uint32_t len)
{
#ifdef ENABLE_DRIVER_AMOLED
    uint32_t len0 = 0;
    uint32_t len1 = 0;
    uint32_t len2 = 0;
    static Dma2DInfo_t dma2DInfoDesc0;
    static Dma2DInfo_t dma2DInfoDesc1;
    static Dma2DInfo_t dma2DInfoDesc2;
    if (len == 0 || data == NULL ) {
        return;
    } if (len <= MAX_XLOOP) {
        dma2DInfoDesc0.xloop = len;
        dma2DInfoDesc0.yloop = 1;
        dma2DInfoDesc0.yincrement = len;

        Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.xloop);
        Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,1);
        
        Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.yloop);
        Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.yincrement);
        
        Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (uint8_t *)data);
        Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

        Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, NULL);

        Cy_DMA_Channel_SetDescriptor(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0);

        Cy_DMA_Channel_Enable(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL);

    } else if (len <= (MAX_XLOOP * MAX_YLOOP)) {
        // 设置第一个描述符
        dma2DInfoDesc0.xloop = MAX_XLOOP;
        dma2DInfoDesc0.yloop = len / MAX_XLOOP;
//        if (dma2DInfoDesc0.yloop > MAX_YLOOP) {
//            dma2DInfoDesc0.yloop = MAX_YLOOP;
//        }
        dma2DInfoDesc0.yincrement = MAX_XLOOP;
//        sm_log(SM_LOG_INFO,"x0=%d, y0=%d, inc0=%d \r\n",dma2DInfoDesc0.xloop, dma2DInfoDesc0.yloop, dma2DInfoDesc0.yincrement);

        Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.xloop);
        
        Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,1);
        
        Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.yloop);
        Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.yincrement);
        
        Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (uint8_t *)data);
        Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

        // 设置第二个描述符，如果有
        if (len % MAX_XLOOP != 0) {
            len0 = dma2DInfoDesc0.xloop * dma2DInfoDesc0.yloop;
            len1 = len - len0;
            dma2DInfoDesc1.xloop = len1;
            dma2DInfoDesc1.yloop = 1;
            dma2DInfoDesc1.yincrement = len1;
//            sm_log(SM_LOG_INFO,"x1=%d, y1=%d, inc1=%d \r\n",dma2DInfoDesc1.xloop, dma2DInfoDesc1.yloop, dma2DInfoDesc1.yincrement);

            Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1);

            Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.xloop);
            
            Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,1);
            
            Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yloop);
           // Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yincrement);
            
            Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (uint8_t *)&data[len0]);
            Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

            //Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,NULL);
        } else {
            //Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0);
        }
        Cy_DMA_Channel_SetDescriptor(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0);
        Cy_DMA_Channel_Enable(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL);
    } else { // 需要三个描述符情况
        // 设置第一个描述符
        dma2DInfoDesc0.xloop = MAX_XLOOP;
        dma2DInfoDesc0.yloop = MAX_YLOOP;
        //        if (dma2DInfoDesc0.yloop > MAX_YLOOP) {
        //            dma2DInfoDesc0.yloop = MAX_YLOOP;
        //        }
        dma2DInfoDesc0.yincrement = MAX_XLOOP;
//        sm_log(SM_LOG_INFO,"x0=%d, y0=%d, inc0=%d \r\n",dma2DInfoDesc0.xloop, dma2DInfoDesc0.yloop, dma2DInfoDesc0.yincrement);

        Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.xloop);

        Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,1);

        Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.yloop);
        Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,dma2DInfoDesc0.yincrement);

        Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (uint8_t *)data);
        Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);
        len0 = (MAX_XLOOP * MAX_YLOOP);
        // 设置第二个描述符
        len1 = len - (MAX_XLOOP * MAX_YLOOP);
        if (len1 <= MAX_XLOOP) {
            dma2DInfoDesc1.xloop = len1;
            dma2DInfoDesc1.yloop = 1;
            dma2DInfoDesc1.yincrement = len1;
            Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1);
            Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.xloop);
            Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,1);
            
            Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yloop);
            Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yincrement);
            
            Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (uint8_t *)&data[len0]);
            Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);
            
            Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, NULL);
            
            Cy_DMA_Channel_SetDescriptor(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1);
            
            Cy_DMA_Channel_Enable(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL);

        } else if (len1 % MAX_XLOOP == 0) { // 设置第二个描述符
        dma2DInfoDesc1.xloop = MAX_XLOOP;
        dma2DInfoDesc1.yloop =  len1/ MAX_XLOOP;
        dma2DInfoDesc1.yincrement = MAX_XLOOP;
//        sm_log(SM_LOG_INFO,"x0=%d, y0=%d, inc0=%d \r\n",dma2DInfoDesc1.xloop, dma2DInfoDesc1.yloop, dma2DInfoDesc1.yincrement);

        Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1);

        Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.xloop);
        
        Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,1);
        
        Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yloop);
        Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yincrement);
        
        Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (uint8_t *)&data[len0]);
        Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);
        } else {
        dma2DInfoDesc1.xloop = MAX_XLOOP;
        dma2DInfoDesc1.yloop =  len1/ MAX_XLOOP;
        dma2DInfoDesc1.yincrement = MAX_XLOOP;
//        sm_log(SM_LOG_INFO,"x0=%d, y0=%d, inc0=%d \r\n",dma2DInfoDesc1.xloop, dma2DInfoDesc1.yloop, dma2DInfoDesc1.yincrement);

        Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1);

        Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.xloop);
        
        Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,1);
        
        Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yloop);
        Cy_DMA_Descriptor_SetYloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,dma2DInfoDesc1.yincrement);
        
        Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (uint8_t *)&data[len0]);
        Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

        // 设置第三个描述符
        len1 = dma2DInfoDesc1.xloop * dma2DInfoDesc1.yloop;
        len2 = len - len0 - len1;
        dma2DInfoDesc2.xloop = len2;
        dma2DInfoDesc2.yloop = 1;
        dma2DInfoDesc2.yincrement = len2;
//        sm_log(SM_LOG_INFO,"x1=%d, y1=%d, inc1=%d \r\n",dma2DInfoDesc2.xloop, dma2DInfoDesc2.yloop, dma2DInfoDesc2.yincrement);

        Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2);

        Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2,dma2DInfoDesc2.xloop);

        Cy_DMA_Descriptor_SetXloopSrcIncrement(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2,1);

        Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2,dma2DInfoDesc2.yloop);

        Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2, (uint8_t *)&data[len0 + len1]);
        Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

        }
        Cy_DMA_Channel_SetDescriptor(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL,&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0);
        Cy_DMA_Channel_Enable(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL);
    }
#endif
}

/******************************************************************************
* Function Name: tx_dma_complete
*******************************************************************************
*
* Summary:      This function check the tx DMA status
*
* Parameters:   None
*
* Return:       None
*
******************************************************************************/
void tx_dma_complete(void)
{
#ifdef ENABLE_DRIVER_AMOLED

     /* Check tx DMA status */
     if ((CY_DMA_INTR_CAUSE_COMPLETION    != Cy_DMA_Channel_GetStatus(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL)) &&
         (CY_DMA_INTR_CAUSE_CURR_PTR_NULL != Cy_DMA_Channel_GetStatus(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL)))
     {
         /* DMA error occurred while TX operations */
         handle_error();
     }

     tx_dma_done = true;
     /* Clear tx DMA interrupt */
     Cy_DMA_Channel_ClearInterrupt(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL);
#endif
}

void sleep_display(void)
{
    uint32_t ticks = 0;
    driver_rm69600_write_command(0x28);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 100);
    driver_rm69600_write_data(0x10);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 150);
}

/**
  * @brief  初始化amoled硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_amoled_init(void)
{
#ifdef ENABLE_DRIVER_AMOLED
    uint32_t ticks = 0;
    sm_log(SM_LOG_INFO,"SPI init start! \r\n");

    cy_rslt_t rslt;

    rslt = cyhal_gpio_init(HW_AMOLED_VCCEN, CYHAL_GPIO_DIR_OUTPUT,
                          CYHAL_GPIO_DRIVE_STRONG, false);

    rslt = cyhal_gpio_init(HW_AMOLED_IOVCCEN, CYHAL_GPIO_DIR_OUTPUT,
                          CYHAL_GPIO_DRIVE_STRONG, false);
    rslt = cyhal_gpio_init(HW_AMOLED_DCX, CYHAL_GPIO_DIR_OUTPUT,
                          CYHAL_GPIO_DRIVE_STRONG, false);
    rslt = cyhal_gpio_init(HW_AMOLED_RST, CYHAL_GPIO_DIR_OUTPUT,
                          CYHAL_GPIO_DRIVE_STRONG, false);
//    rslt = cyhal_gpio_init(HW_AMOLED_TE, CYHAL_GPIO_DIR_INPUT,
 //                         CYHAL_GPIO_DRIVE_PULLUP, true); // 会影响休眠功耗，如启用TE功能需要确认功耗

    cyhal_gpio_write(HW_AMOLED_DCX, 0u); // 先拉低， 否则会串电到IOVCC， 后续休眠节能也要拉低或者高阻态
    driver_rm69600_write_reset_pin(0);
    cyhal_gpio_write(HW_AMOLED_IOVCCEN, 0);
    cyhal_gpio_write(HW_AMOLED_VCCEN, 0);

    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 20);
//    driver_amoled_power_up(1); // 先初始化上电时序，以免初始化SPI时因为SPI的IO串电到IOVCC
    cyhal_gpio_write(HW_AMOLED_IOVCCEN, 1);
    cyhal_gpio_write(HW_AMOLED_VCCEN, 1);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 20);

    Cy_GPIO_Pin_Init(CYBSP_SPI_MOSI_PORT, CYBSP_SPI_MOSI_PIN, &CYBSP_SPI_MOSI_config);
    Cy_GPIO_Pin_Init(CYBSP_SPI_MISO_PORT, CYBSP_SPI_MISO_PIN, &CYBSP_SPI_MISO_config);
    Cy_GPIO_Pin_Init(CYBSP_SPI_SCK_PORT, CYBSP_SPI_SCK_PIN, &CYBSP_SPI_SCK_config);
    Cy_GPIO_Pin_Init(CYBSP_SPI_CSN_PORT, CYBSP_SPI_CSN_PIN, &CYBSP_SPI_CSN_config);


    cy_en_scb_spi_status_t init_status;

    /* Configure SPI block */
    init_status = Cy_SCB_SPI_Init(CYBSP_AMOLED_SPI_HW, &CYBSP_AMOLED_SPI_config, NULL);

    /* If the initialization fails, return failure status */
    if (init_status != CY_SCB_SPI_SUCCESS)
    {
      //  return(INIT_FAILURE);
    }

    /* Set active slave select to line 0 */
    Cy_SCB_SPI_SetActiveSlaveSelect(CYBSP_AMOLED_SPI_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable SPI master block. */
    Cy_SCB_SPI_Enable(CYBSP_AMOLED_SPI_HW);
    
    sm_log(SM_LOG_INFO,"SPI init succ! \r\n");

     cy_en_dma_status_t dma_init_status;
     const cy_stc_sysint_t intTxDma_cfg =
     {
         .intrSrc      = CYBSP_AMOLED_SPI_DMA_TX_IRQ,
         .intrPriority = 7u
     };
     /* Initialize descriptor */
     dma_init_status = Cy_DMA_Descriptor_Init(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, &CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0_config);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
        // return INIT_FAILURE;
     }

     /* Initialize descriptor */
     dma_init_status = Cy_DMA_Descriptor_Init(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, &CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1_config);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
        // return INIT_FAILURE;
     }

     /* Initialize descriptor */
     dma_init_status = Cy_DMA_Descriptor_Init(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2, &CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2_config);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
        // return INIT_FAILURE;
     }

     dma_init_status = Cy_DMA_Channel_Init(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL, &CYBSP_AMOLED_SPI_DMA_TX_channelConfig);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
       //  return INIT_FAILURE;
     }
     Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (uint8_t *)&g_send_one_data);
     Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);


     Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (uint8_t *)&g_send_one_data);
     Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

     Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2, (uint8_t *)&g_send_one_data);
     Cy_DMA_Descriptor_SetDstAddress(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2, (void *)&CYBSP_AMOLED_SPI_HW->TX_FIFO_WR);

    //     Cy_DMA_Descriptor_SetXloopDataCount(&g_SPI_DMA_TX_Descriptor_0,count);

     cyhal_system_set_isr(CYBSP_AMOLED_SPI_DMA_TX_IRQ, CYBSP_AMOLED_SPI_DMA_TX_IRQ, 7u, &tx_dma_complete);
     NVIC_EnableIRQ((IRQn_Type)intTxDma_cfg.intrSrc);

     Cy_DMA_Channel_SetInterruptMask(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL, CY_DMA_INTR_MASK);

     Cy_DMA_Enable(CYBSP_AMOLED_SPI_DMA_TX_HW);
     
     sm_log(SM_LOG_INFO,"DMA init succ! \r\n");
#if 0
    driver_rm69600_write_command(0xfe);
    driver_rm69600_write_data(0x00);

//    driver_rm69600_write_command(0x36);
//    driver_rm69600_write_data(0x08);

//    driver_rm69600_write_command(0x3a);
//    driver_rm69600_write_data(0x05); // rgb 565

    driver_rm69600_write_command(0xc4);
    driver_rm69600_write_data(0x80);

    driver_rm69600_write_command(0x2a);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x59);

    driver_rm69600_write_command(0x2b);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x19);

    driver_rm69600_write_command(0x31);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x58);

    driver_rm69600_write_command(0x30);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x18);

    driver_rm69600_write_command(0x12);
    
    driver_rm69600_write_command(0x35);
    driver_rm69600_write_data(0x00);

    driver_rm69600_write_command(0x51);
    driver_rm69600_write_data(0xff);

    driver_rm69600_write_command(0x11);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 61);
    driver_rm69600_write_command(0x29);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 2);


    sm_log(SM_LOG_INFO,"OLED init succ! \r\n");
#endif
return rslt;

#endif
    return 0;
}

/**
  * @brief  初始化amoled硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_amoled_reinit(void)
{
#ifdef ENABLE_DRIVER_AMOLED
    uint32_t ticks = 0;
    sm_log(SM_LOG_INFO,"OLED init start! \r\n");

    driver_rm69600_write_reset_pin(0);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 20);
    driver_rm69600_write_reset_pin(1);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 20); // 等待复位就绪

    driver_rm69600_write_command(0xfe);
    driver_rm69600_write_data(0x00);

//    driver_rm69600_write_command(0x36);
//    driver_rm69600_write_data(0x08);

//    driver_rm69600_write_command(0x3a);
//    driver_rm69600_write_data(0x05); // rgb 565

    driver_rm69600_write_command(0xc4);
    driver_rm69600_write_data(0x80);

    driver_rm69600_write_command(0x2a);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x59);

    driver_rm69600_write_command(0x2b);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x19);

    driver_rm69600_write_command(0x31);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x58);

    driver_rm69600_write_command(0x30);
    driver_rm69600_write_data(0x00);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x01);
    driver_rm69600_write_data(0x18);

    driver_rm69600_write_command(0x12);
    
    driver_rm69600_write_command(0x35);
    driver_rm69600_write_data(0x00);

    driver_rm69600_write_command(0x51);

#if !defined(BLE_PROFILES_PROTOCOL_SUPPORT)
		driver_rm69600_write_data(0xff);
#else
		uint32_t temp;
		temp = amoled_brigth_get();
		temp *= 255;
		temp /= 100;
		driver_rm69600_write_data((uint8_t)temp);
#endif //defined(BLE_PROFILES_PROTOCOL_SUPPORT)

    driver_rm69600_write_command(0x11);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 61);
    driver_rm69600_write_command(0x29);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 2);
    sm_log(SM_LOG_INFO,"OLED init succ! \r\n");
    return 0;

#endif
    return 0;
}


/**
  * @brief  去初始化amoled硬件
  * @param  None
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_amoled_deinit(void)
{
#ifdef ENABLE_DRIVER_AMOLED
    uint32_t ticks = 0;
    // 增加下电时序
    // DISPLY OFF
    driver_rm69600_write_command(0x28);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 5);
    driver_rm69600_write_command(0x10);
    ticks = get_ms_tick();
    while (get_ms_tick() -ticks < 220);
    
    Cy_SCB_SPI_DeInit(CYBSP_AMOLED_SPI_HW);
    NVIC_DisableIRQ(CYBSP_AMOLED_SPI_DMA_TX_IRQ);
    Cy_DMA_Channel_DeInit(CYBSP_AMOLED_SPI_DMA_TX_HW, CYBSP_AMOLED_SPI_DMA_TX_CHANNEL);
    Cy_DMA_Descriptor_DeInit(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_0);
    Cy_DMA_Descriptor_DeInit(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_1);
    Cy_DMA_Descriptor_DeInit(&CYBSP_AMOLED_SPI_DMA_TX_Descriptor_2);
    Cy_GPIO_Pin_FastInit(GPIO_PRT7, 0, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT7, 1, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT7, 2, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(GPIO_PRT7, 3, CY_GPIO_DM_STRONG, 0UL, HSIOM_SEL_GPIO);
    cyhal_gpio_write(HW_AMOLED_DCX, 0u);
    driver_rm69600_write_reset_pin(0);
    cyhal_gpio_write(HW_AMOLED_VCCEN, 0);
    ticks = get_ms_tick(); // POA 版本，显示屏VDDI要比VDD晚10ms下电， 显示屏先去初始化VDD先下电
    while (get_ms_tick() -ticks < 12);
    cyhal_gpio_write(HW_AMOLED_IOVCCEN, 0);
//    cyhal_gpio_init(P7_7, CYHAL_GPIO_DIR_OUTPUT, CY_GPIO_DM_STRONG, 0); // 帧同步信号 PDL与HAL库宏不能混用CYHAL_GPIO_DRIVE_STRONG
    cyhal_gpio_init(P7_7, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0); // 帧同步信号 PDL与HAL库宏不能混用CYHAL_GPIO_DRIVE_STRONG
#endif
return 0;
}

/**
  * @brief  对显示屏读操作
  * @param  pBuf:   要读入的数据
            len:    要读入数据的长度
  * @return 0：成功，-1：失败
  * @note
  */
int driver_amoled_read(uint8_t *pBuf, uint16_t len)
{

    if(pBuf == NULL) {
 //       pBuf = get_dma_send_buff();
    }
//    pBuf[0] = cyhal_gpio_read(HW_AMOLED_TE); // 预留TE
    return 0;
}

/**
  * @brief  向屏写数据
  * @param  pBuf:   要写入的数据
            len:    要写入数据的长度
  * @return 0：成功，-1：失败
  * @note   None
  */
int driver_amoled_write(uint8_t *pBuf, uint16_t len)
{
#ifdef ENABLE_DRIVER_AMOLED
    DrvAmoledInfo_t *ptdrvAmoledInfo;
    ptdrvAmoledInfo = (DrvAmoledInfo_t *)pBuf;
//#if 0
    if(ptdrvAmoledInfo->area.x1 < 0) return -1; // Both XS[7:0] and (XE[7:0]-XS[7:0]+1) should be multiples of 2..
    if(ptdrvAmoledInfo->area.y1 < 0) return -1; // Both YS[9:0] and (YE[9:0]-YS[9:0]+1) should be multiples of 2.
    if(ptdrvAmoledInfo->area.x2 > DRV_HOR_RES - 1) return -1;
    if(ptdrvAmoledInfo->area.y2 > DRV_VER_RES - 1) return -1;

    driver_rm69600_write_command(0x2a);
    driver_rm69600_write_data(ptdrvAmoledInfo->area.x1 >> 8);  
    driver_rm69600_write_data(ptdrvAmoledInfo->area.x1 & 0xFF); 
    driver_rm69600_write_data(ptdrvAmoledInfo->area.x2 >> 8);
    driver_rm69600_write_data(ptdrvAmoledInfo->area.x2 & 0xFF);

    driver_rm69600_write_command(0x2b);
    driver_rm69600_write_data(ptdrvAmoledInfo->area.y1 >> 8); 
    driver_rm69600_write_data(ptdrvAmoledInfo->area.y1 & 0xFF); 
    driver_rm69600_write_data(ptdrvAmoledInfo->area.y2 >> 8);
    driver_rm69600_write_data(ptdrvAmoledInfo->area.y2 & 0xFF);

    driver_rm69600_write_command(0x2c);
    driver_rm69600_write_data_block(ptdrvAmoledInfo->data, ptdrvAmoledInfo->len);
//#endif
    #if 0
    if(ptdrvAmoledInfo->area.x1 < 0) return -1;
    if(ptdrvAmoledInfo->area.y1 < 0) return -1;
    if(ptdrvAmoledInfo->area.x2 > DRV_HOR_RES - 1) return -1;
    if(ptdrvAmoledInfo->area.y2 > DRV_VER_RES - 1) return -1;

    driver_rm69600_write_command(0x2a);
    driver_rm69600_write_data(0);  
    driver_rm69600_write_data(2); 
    driver_rm69600_write_data(0);
    driver_rm69600_write_data(59);

    driver_rm69600_write_command(0x2b);
    driver_rm69600_write_data(0); 
    driver_rm69600_write_data(2); 
    driver_rm69600_write_data(0);
    driver_rm69600_write_data(199);

    driver_rm69600_write_command(0x2c);
    driver_rm69600_write_data_block(ptdrvAmoledInfo->data, 22968);
    #endif
#endif
    return 0;
}


