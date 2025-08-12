/**
  ******************************************************************************
  * @file    driver_key.c
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
#include "driver_key.h"
#include "driver_uart_usb.h"

#define ENABLE_DRIVER_KEY

#ifdef ENABLE_DRIVER_KEY
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#define KEY1_PIN        P0_4 // V03 boost
#define KEY2_PIN        P8_4 // V03 standard
#define USB_PIN         P1_4 // V03

#define GPIO_INTERRUPT_PRIORITY (7u)

cyhal_gpio_callback_data_t key1_callback_data;
cyhal_gpio_callback_data_t key2_callback_data;
cyhal_gpio_callback_data_t usb_callback_data;

static void key1_interrupt_handler(void);
static void key2_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
static void usb_interrupt_handler(void);

/* This structure initializes the Port0 interrupt for the NVIC */
cy_stc_sysint_t intrKey1Cfg =
    {
        .intrSrc = ioss_interrupts_gpio_0_IRQn, /* Interrupt source is GPIO port 0 interrupt */
        .intrPriority = 2UL                     /* Interrupt priority is 2 */
};

cy_stc_sysint_t intrUsbCfg =
    {
        .intrSrc = ioss_interrupts_gpio_1_IRQn, /* Interrupt source is GPIO port 1 interrupt */
        .intrPriority = 2UL                     /* Interrupt priority is 2 */
};

#endif

extern TaskHandle_t* get_task_system_handle(void);
eTaskState taskState;      // 用于存储任务状态的变量
uint8_t wakeUpBuf[4];
// 获取唤醒源
void get_wake_up_source(uint8_t *buf)
{
    buf[0] = wakeUpBuf[0];
    buf[1] = wakeUpBuf[1];
    buf[2] = wakeUpBuf[2];
    buf[3] = wakeUpBuf[3]; // 按键同时按下标志，在唤醒后初始化第一时间判断
}

void clr_wake_up_source(void)
{
    wakeUpBuf[0] = 0;
    wakeUpBuf[1] = 0;
    wakeUpBuf[2] = 0;
    wakeUpBuf[3] = 0;
}
static void key1_interrupt_handler(void)
{
#ifdef ENABLE_DRIVER_KEY
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TaskHandle_t *temp_handle;
    temp_handle = get_task_system_handle();
    if (false == get_driver_status()) {
        wakeUpBuf[0] = 1;
        vTaskNotifyGiveFromISR(*temp_handle, &xHigherPriorityTaskWoken);
    }
    /* Clear pin interrupt logic. Required to detect next interrupt */
    Cy_GPIO_ClearInterrupt(CYHAL_GET_PORTADDR(KEY1_PIN), CYHAL_GET_PIN(KEY1_PIN));
#endif
}

static void key2_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
#ifdef ENABLE_DRIVER_KEY
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TaskHandle_t *temp_handle;
    temp_handle = get_task_system_handle();
    if (false == get_driver_status()) {
        wakeUpBuf[1] = 1;
        vTaskNotifyGiveFromISR(*temp_handle, &xHigherPriorityTaskWoken);
    }
    /* Clear pin interrupt logic. Required to detect next interrupt */
//    Cy_GPIO_ClearInterrupt(CYHAL_GET_PORTADDR(KEY2_PIN), CYHAL_GET_PIN(KEY2_PIN));
#endif
}

static void usb_interrupt_handler(void)
{
#ifdef ENABLE_DRIVER_KEY
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TaskHandle_t *temp_handle;
    temp_handle = get_task_system_handle();
    if (false == get_driver_status()) {
        wakeUpBuf[2] = 1;
        vTaskNotifyGiveFromISR(*temp_handle, &xHigherPriorityTaskWoken);
    }
    /* Clear pin interrupt logic. Required to detect next interrupt */
    Cy_GPIO_ClearInterrupt(CYHAL_GET_PORTADDR(USB_PIN), CYHAL_GET_PIN(USB_PIN));
#endif
}

uint8_t g_initKeyflag = 0;
uint32_t g_key1Mstick = 1400; //
uint32_t g_key2Mstick = 1400; // 算上开机按键0.8 + boot0.6时间
uint32_t g_usbMstick = 0;
uint32_t get_key1_mstick(void)
{
    return g_key1Mstick;
}

uint32_t get_key2_mstick(void)
{
    return g_key2Mstick;
}

uint32_t get_usb_mstick(void)
{
    return g_usbMstick;
}

void shipping_mode_out_press_key(void)
{
    if (1 == g_initKeyflag) {
        if (Cy_GPIO_Read(GPIO_PRT0, 4) == 0) {
            g_key1Mstick++;
        } else {
            g_key1Mstick = 0;
        }
        if (cyhal_gpio_read(KEY2_PIN) == 0) {
            g_key2Mstick++;
        } else {
            g_key2Mstick = 0;
        }
        if (Cy_GPIO_Read(GPIO_PRT1, 4) == 0) {
            if (g_usbMstick < 10000) {
                g_usbMstick++;
            }
        } else {
            if (g_usbMstick < 100) { // 100 MS防抖
                g_usbMstick = 0;
            }
        }
    }
}

/**
  * @brief  初始化按键硬件
  * @param  None
  * @return None
  * @note   None
  */
int driver_key_init(void)
{
#ifdef ENABLE_DRIVER_KEY
// 添加按键IO初始化，USB插入检测IO初始化
//    static uint8_t init_flag = 0;
    if (g_initKeyflag == 0) {
        g_initKeyflag = 1;

        cy_rslt_t result;
//#if 0
        /* Initialize the user button */
        Cy_GPIO_Pin_FastInit(GPIO_PRT0, 4, CY_GPIO_DM_PULLUP, 1UL, HSIOM_SEL_GPIO);

        /* Pin Interrupts */
        /* Configure GPIO pin to generate interrupts */
        Cy_GPIO_SetInterruptEdge(GPIO_PRT0, 4, CY_GPIO_INTR_FALLING);
        Cy_GPIO_SetInterruptMask(GPIO_PRT0, 4, CY_GPIO_INTR_EN_MASK);

        /* Configure CM4+ CPU GPIO interrupt vector for Port 0 */
        Cy_SysInt_Init(&intrKey1Cfg, key1_interrupt_handler);
        NVIC_ClearPendingIRQ(intrKey1Cfg.intrSrc);
        NVIC_EnableIRQ((IRQn_Type)intrKey1Cfg.intrSrc);

        Cy_GPIO_Pin_FastInit(GPIO_PRT1, 4, CY_GPIO_DM_PULLUP, 1UL, HSIOM_SEL_GPIO);

        /* Pin Interrupts */
        /* Configure GPIO pin to generate interrupts */
        Cy_GPIO_SetInterruptEdge(GPIO_PRT1, 4, CY_GPIO_INTR_FALLING);
        Cy_GPIO_SetInterruptMask(GPIO_PRT1, 4, CY_GPIO_INTR_EN_MASK);

        /* Configure CM4+ CPU GPIO interrupt vector for Port 1 */
        Cy_SysInt_Init(&intrUsbCfg, usb_interrupt_handler);
        NVIC_ClearPendingIRQ(intrUsbCfg.intrSrc);
        NVIC_EnableIRQ((IRQn_Type)intrUsbCfg.intrSrc);
//#endif
        result = cyhal_gpio_init(KEY2_PIN, CYHAL_GPIO_DIR_INPUT,
                     CYHAL_GPIO_DRIVE_PULLUP, true);
        if (result != CY_RSLT_SUCCESS)
        {
            CY_ASSERT(0);
        }
#if 0
        result = cyhal_gpio_init(USB_PIN, CYHAL_GPIO_DIR_INPUT,
                     CYHAL_GPIO_DRIVE_PULLUP, true);
        if (result != CY_RSLT_SUCCESS)
        {
            CY_ASSERT(0);
        }
#endif
        key2_callback_data.callback = key2_interrupt_handler;
        cyhal_gpio_register_callback(KEY2_PIN, 
                                     &key2_callback_data);
        cyhal_gpio_enable_event(KEY2_PIN, CYHAL_GPIO_IRQ_FALL, 
                                     GPIO_INTERRUPT_PRIORITY, true);
#if 0
        usb_callback_data.callback = usb_interrupt_handler;
        cyhal_gpio_register_callback(USB_PIN, 
                                     &usb_callback_data);
        cyhal_gpio_enable_event(USB_PIN, CYHAL_GPIO_IRQ_FALL, 
                                     GPIO_INTERRUPT_PRIORITY, true);
#endif
        /* Enable global interrupts */
        __enable_irq();
    }
#endif
    if (wakeUpBuf[0] == 1 && wakeUpBuf[1] == 1) {
        wakeUpBuf[3] = 1;
    }
    return 0;
}


/**
  * @brief  去初始化按键硬件
  * @param  None
  * @return None
  * @note   None
  */
int driver_key_deinit(void)
{
#ifdef ENABLE_DRIVER_KEY

//    cyhal_gpio_init(INT_PD_PIN, CYHAL_GPIO_DIR_INPUT,
//					CYHAL_GPIO_DRIVE_PULL_NONE, false);
#endif
    return 0;
}

/**
  * @brief  读按键硬件状态
  * @param  buf:要读入的数据
            len:要读入数据的长度
  * @return 0：成功，-1：失败
  * @note   USB接入检测管脚状态也放此处
  */
int driver_key_read(uint8_t *pBuf, uint16_t len)
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
    // pBuf[0] :standard_key,pBuf[1]:boost_key, pBuf[2]: usb
    pBuf[0] = !(cyhal_gpio_read(KEY2_PIN));
    pBuf[1] = !Cy_GPIO_Read(GPIO_PRT0, 4);
//    pBuf[0] = !(cyhal_gpio_read(KEY1_PIN));
//    pBuf[2] = !(cyhal_gpio_read(USB_PIN));
    pBuf[2] = !Cy_GPIO_Read(GPIO_PRT1, 4);

    return 0;
}

