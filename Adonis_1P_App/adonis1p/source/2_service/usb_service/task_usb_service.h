#ifndef __TASK_USB_SERVICE_H
#define __TASK_USB_SERVICE_H

#include "stdint.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include "semphr.h"

TaskHandle_t* get_task_usb_handle(void);
void task_usb_service(void *pvParam);
#endif

