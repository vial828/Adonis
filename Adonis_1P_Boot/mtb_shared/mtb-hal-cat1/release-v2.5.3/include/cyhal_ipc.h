/***************************************************************************//**
* \file cyhal_ipc.h
*
* \brief
* Provides a high level interface for interacting with the Infineon IPC.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low level functions
* can be used directly.
*
********************************************************************************
* \copyright
* Copyright 2022 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation
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

/**
* \addtogroup group_hal_ipc IPC (Inter-Processor Communication)
* \ingroup group_hal
* \{
* High level interface for communicating between processors on a multi-core device.
*
* The IPC driver allows communication between multiple CPUs or between multiple tasks
* operating in different domains within a single CPU. It supports binary semaphores
* and message queues, similar to how they are used for task interactions in an RTOS envrionment.
*
* \section subsection_ipc_features Features
* * Binary semaphore for resource access control and general signalling
* * Message queue for passing data between tasks/cores
*
* \section subsection_ipc_quickstart Quick Start
* For binary semaphores, only one task/CPU may initialize a semaphore. Other tasks/CPUs
* then get the handle of the created semaphore. Then take/give the semaphore.
*
* For queues, only one task/CPU may initialize a queue. Other tasks/CPUs then get the handle
* of the created queue. Use the get/put functions to take out or put in items to the queue.
*
* \section section_hal_impl_ipc_semaphore_idx_valid_range Valid range for IPC semaphore number
* cyhal_ipc_semaphore_init function semaphore_num parameter valid range is from 0 to (including) CY_IPC_SEMA_COUNT - 2
*
* \section section_hal_impl_ipc_queue_pool_memory Queue pool memory allocation
* Queue pool memory allocation can be done with help of CYHAL_IPC_QUEUE_POOL_ALLOC macro, but using it is not mandatory,
* so user can allocate memory for the queue pool in any convenient for them way until such allocation satisfies next parameters:
* - Memory should be shared (e.g. CY_SECTION_SHAREDMEM macro can be used)
* - Memory should be able to store NUM_ITEMS x ITEM_SIZE bytes, where NUM_ITEMS - number of max. elements, that can queue store,
* ITEM_SIZE - size of each queue item in bytes
* - If CM7 core used with Data Cache enabled, memory should be aligned to __SCB_DCACHE_LINE_SIZE (e.g. CY_ALIGN(__SCB_DCACHE_LINE_SIZE) can be used)
*
* \section section_hal_impl_ipc_queue_handle_memory Queue handle memory allocation
* Queue handle memory allocation can be done with help of CYHAL_IPC_QUEUE_HANDLE_ALLOC macro, but using it is not mandatory,
* so user can allocate memory for the queue handle in any convenient for them way until such allocation satisfies next parameters:
* - Memory should be shared (e.g. CY_SECTION_SHAREDMEM macro can be used)
* - Memory should be able to store cyhal_ipc_queue_t type struct
* - If CM7 core used with Data Cache enabled, memory should be aligned to __SCB_DCACHE_LINE_SIZE (e.g. CY_ALIGN(__SCB_DCACHE_LINE_SIZE) can be used)
*
* \section section_hal_impl_ipc_preemtable_sema Preemtable Semaphore parameter
* If preemptable parameter is enabled (true) for semaphore, the user must ensure that there are no deadlocks in the
* system, which can be caused by an interrupt that occurs after the IPC channel is locked. Unless the user is ready to
* handle IPC channel locks at the application level, set preemptable to false.
*
* \section section_hal_impl_ipc_interrupts_priorities IPC interrupts implementation and priorities
* In current HAL IPC implementation, each core has its "own" IPC INTR structure, which services all possible HAL IPC
* channels. Due to that, callbacks (interrupts) priorities are not flexible in configuration, which means that
* priority, set by \ref cyhal_ipc_queue_enable_event function, will only be applied, if it is lower,
* than the one, which is currently applied for the source (IPC INTR structure), that services interrupts for current
* core. Priority is being applied core-wide, for all channels and queue numbers.
*
* \section section_hal_impl_ipc_queue_operation_isr IPC queues operations in ISR context
* IPC queue put/get operations cannot be performed in callbacks with a timeout != 0 (ISR context).
* Such operations will result in \ref CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT.
*
* \section section_hal_impl_ipc_last_sema_occupied Last available IPC semaphore is occupied by HAL IPC
* Last available IPC semaphore (CYHAL_IPC_SEMA_COUNT - 1) is occupied by multi-core interrupt synchronization mechanism
* and is not available for user. Semaphores for internal use by queues start by using semaphores from the
* last available IPC semaphore and work their way down. Applications can start with semaphore 0 and work their way
* up.
*
* \section section_hal_impl_ipc_semaphores_initialization IPC Semaphores initialization
* On some devices (currently, CAT1C and CAT1D devices), startup code does not initialize IPC PDL driver semaphores,
* so it is done by cyhal_ipc_init(), which is called from all cores. The INIT_CORE will do the initialization
* and set up a pointer using an IPC channel so that all Cores can read and use the same variables.
* For backward compatibility, calling cyhal_ipc_semaphore_init() or cyhal_ipc_queue_init() will call cyhal_ipc_init()
* if it has not already been done on that core.
* By default, for CAT1C, CM7_0 should call this function before other cores. On CAT1D, use CM33 core.
* The initialization core can be changed by user by defining `CYHAL_IPC_INIT_CORE` with required core name. For example,
* definition of CYHAL_IPC_INIT_CORE to be equal CORE_NAME_CM7_1 will make cyhal_ipc_init() on CAT1C device
* initialize IPC PDL semaphores on CM7_1 core. Note: Defines `CORE_NAME_*` are being generated for each device by
* corresponding recipe-make-* asset.
*
* \section section_hal_impl_ipc_get_queue_handle_before_init Calling cyhal_ipc_queue_get_handle() before cyhal_ipc_queue_init()
* will result in \ref CYHAL_IPC_RSLT_ERR_QUEUE_NOT_FOUND. The correct way of using cyhal_ipc_queue_get_handle()
* is to call cyhal_ipc_queue_init() before using cyhal_ipc_queue_get_handle(), and pass the same channel and queue number that
* was used for the queue initialization. Queues can be initialized on any core and accessed from any core.
*
* \section section_ipc_snippets Code Snippets
*
* \subsection subsection_ipc_snippet1 Snippet 1: Binary semaphore example
* \snippet hal_ipc.c snippet_cyhal_ipc_semaphore
*
* \subsection subsection_ipc_snippet2 Snippet 2: Message queue example
* \snippet hal_ipc.c snippet_cyhal_ipc_queue
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cy_result.h"
#include "cyhal_hw_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** \addtogroup group_hal_results_ipc IPC HAL Results
 *  IPC specific return codes
 *  \ingroup group_hal_results
 *  \{ *//**
 */

/** HAL IPC Driver not initialized */
#define CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED           \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 0))
/** Invalid parameter error */
#define CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER           \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 1))
/** Timeout Occurred */
#define CYHAL_IPC_RSLT_ERR_TIMEOUT           \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 2))
/** Provided semaphore number already used */
#define CYHAL_IPC_RSLT_ERR_SEMA_NUM_IN_USE            \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 3))
/** Provided semaphore number already taken */
#define CYHAL_IPC_RSLT_ERR_SEMA_TAKEN            \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 4))
/** Failed to take the semaphore */
#define CYHAL_IPC_RSLT_ERR_SEMA_FAIL            \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 5))
/** Can't allocate the semaphore, possibly bad number */
#define CYHAL_IPC_RSLT_ERR_NO_SEMA_AVAILABLE           \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 6))
/** Provided queue number already used */
#define CYHAL_IPC_RSLT_ERR_QUEUE_NUM_IN_USE            \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 7))
/** Queue is full */
#define CYHAL_IPC_RSLT_ERR_QUEUE_FULL                  \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 8))
/** Queue is empty */
#define CYHAL_IPC_RSLT_ERR_QUEUE_EMPTY                 \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 9))
/** Queue object is not found */
#define CYHAL_IPC_RSLT_ERR_QUEUE_NOT_FOUND             \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 10))
/** IPC interrupt was enabled for one of the cores, but was not (yet) handled */
#define CYHAL_IPC_RSLT_ERR_ISR_WAS_NOT_HANDLED         \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 11))
/** Operation can't be performed in ISR context */
#define CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR         \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 12))
/** Operation can't be performed in ISR context with timeout != 0 */
#define CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT         \
    (CY_RSLT_CREATE_EX(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_ABSTRACTION_HAL, CYHAL_RSLT_MODULE_IPC, 13))

/**
 * \}
 */

/** The maximum number of Queues per Channel.
 * Device or Application can override.
 */
#ifdef CYHAL_IPC_QUEUES_PER_CHANNEL
#define _CYHAL_IPC_QUEUES_PER_CHANNEL               (CYHAL_IPC_QUEUES_PER_CHANNEL)
#else
#define _CYHAL_IPC_QUEUES_PER_CHANNEL               (8)
#endif /* _CYHAL_IPC_QUEUES_PER_CHANNEL */

/** The maximum number of Processes per Queue.
* Device or Application can override.
*/
#ifdef CYHAL_IPC_MAX_PROCESSES_PER_QUEUE
#define _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE          (CYHAL_IPC_MAX_PROCESSES_PER_QUEUE)
#else
#define _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE          (2)
#endif /* _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE */

/** Define the HAL IPC communication if not already defined
 */
#ifdef CYHAL_IPC_COMM_CHANNEL
#define _CYHAL_IPC_COMM_CHANNEL                 (CYHAL_IPC_COMM_CHANNEL)
#else
#define _CYHAL_IPC_COMM_CHANNEL                 (CM0_IPC0_CH_NUM)
#endif /* _CYHAL_IPC_COMM_CHANNEL */

/** This define can be used as timeout argument for the IPC HAL driver functions, that take timeout
 * as input parameter, in order to make function never time out (wait forever) */
#define CYHAL_IPC_NEVER_TIMEOUT (0xFFFFFFFFUL)

/** Flags enum of IPC events. Multiple events can be enabled via \ref cyhal_ipc_queue_enable_event and
 * the callback from \ref cyhal_ipc_queue_register_callback will be run to notify. */
typedef enum
{
    CYHAL_IPC_NO_INTR           = 0,      //!< No interrupt
    CYHAL_IPC_QUEUE_WRITE       = 1 << 0, //!< New item was written to the queue
    CYHAL_IPC_QUEUE_READ        = 1 << 1, //!< New item was read from the queue
    CYHAL_IPC_QUEUE_FULL        = 1 << 2, //!< Queue is full
    CYHAL_IPC_QUEUE_EMPTY       = 1 << 3, //!< Queue is empty
    CYHAL_IPC_QUEUE_RESET       = 1 << 4, //!< Queue was reset
} cyhal_ipc_event_t;

/** Aggregate of all the Queue events */
#define CYHAL_IPC_ALL_QUEUE_EVENTS ( CYHAL_IPC_QUEUE_WRITE | CYHAL_IPC_QUEUE_READ | CYHAL_IPC_QUEUE_FULL | CYHAL_IPC_QUEUE_EMPTY | CYHAL_IPC_QUEUE_RESET )

/** Event handler for IPC interrupts */
typedef void (*cyhal_ipc_event_callback_t)(void *callback_arg, cyhal_ipc_event_t event);

/** IPC queue structure element */
typedef struct cyhal_ipc_queue_s
{
    /* Items to be populated by caller before initialization
     * Combination of channel_num and queue_num uniquely defines a specific Queue.
     */
    uint32_t    channel_num;        //!< IPC channel number (e.g. CYHAL_IPC_CHAN_0) Please refer to implementation specific documentation for number of available IPC channels for particular device.
    uint32_t    queue_num;          //!< Queue number, which must be unique for each queue in scope of one IPC channel.
    uint32_t    num_items;          //!< Maximum number of items (packets) allowed in the queue
    uint32_t    item_size;          //!< Size of each packet (item) in the Queue
    void        *queue_pool;        //!< Pointer to the queue packets in shared memory. This memory will be cleared by HAL IPC Queue Initialization.
                                    //!< Packet list is an array of packets of num_items, It is used as a circular array, first_item is an index to first item stored.

    /* Items populated by HAL when calling cyhal_ipc_queue_init() */
    uint32_t    curr_items;         //!< Current number of items (packets) in the queue. Set to 0x00.
    uint32_t    first_item;         //!< Index of the first item in the queue array, INVALID if curr_items is 0x00

    cyhal_ipc_t queue_semaphore;    //!< IPC Semaphore - covers all items in the queue EXCEPT for notifications, which have their own Semaphores

    /* Items populated by HAL IPC when calling \ref _cyhal_ipc_queue_register_callback. */
    _cyhal_ipc_queue_process_info_t notifications[ _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE ];  //!< Notification structure per Process
                                                                                          //!< Changed when user calls  \ref cyhal_ipc_queue_enable_event and \ref cyhal_ipc_queue_register_callback.
                                                                                          //!< Modified by HAL IPC during execution (not expected to be modified by user directly).
} cyhal_ipc_queue_t;

/** Initialize HAL IPC Driver
 * Call from All Cores to initializes the system.
 *
 * This function initializes the Shared Memory Data structures for HAL IPC Semaphore and HAL IPC Queue usage.
 * Call before any other HAL IPC functions.
 * On non-BOOT Cores, they will waiting for timeout_ms for the BOOT Core to finish initialization
 *
 * @param  timeout_ms - time in milliseconds for non-Boot Cores to wait for Boot Core to complete inititalization
 *
 * @return CY_RSLT_SUCCESS
 *         CYHAL_IPC_RSLT_ERR_TIMEOUT
 */
cy_rslt_t cyhal_ipc_init(uint32_t timeout_ms);

/** Creates a single semaphore based on a given number.
 *  The semaphore number range starts at 0.
 *  the max number depends on a few factors max = CYHAL_IPC_SEMA_COUNT - 1 (IPC driver semaphore) - num_queues - (num_queues * num_processes_per_queue)
 *
 * This function must be called before accessing the semaphore.
 * @param[out] obj              Pointer to an IPC object. The caller must allocate the memory for this object but the
 * init function will initialize its contents.
 * @param[in] semaphore_num     The semaphore number to initialize. Please refer to implementation specific documentation for valid range for this parameter.
 * @param[in] preemptable       Allows whether the semaphore can be preempted by another task.
 * @return The status of the init request
 */
cy_rslt_t cyhal_ipc_semaphore_init(cyhal_ipc_t *obj, uint32_t semaphore_num, bool preemptable);

/** Finds an already initialized semaphore based on a given number.
 *
 * This function must be called before accessing the semaphore from a different Core than it was initialized on.
 * @param[out] obj              Handle to an IPC object pointer.
 * @param[in] semaphore_num     The semaphore number to get a pointer for
 * @param[in] timeout_us        Timeout (in uSec) to wait while trying to get the pointer
 * @return The status of the request
 */
cy_rslt_t cyhal_ipc_semaphore_get_handle(cyhal_ipc_t **obj, uint32_t semaphore_num, uint32_t timeout_us);

/** Frees the IPC semaphore.
 *
 * This function frees the resources associated with the semaphore.
 * @param[in, out] obj          The IPC object.
 */
void cyhal_ipc_semaphore_free(cyhal_ipc_t *obj);

/** Takes/acquires a semaphore.
 *
 * If the semaphore is available, it is acquired and this function and returns.
 * This function has a timeout argument (in microseconds). If the semaphore is not available, it blocks until it
 * times out or succeeds in acquiring it.
 * @param[in] obj               The IPC object.
 * @param[in] timeout_us        Timeout in microseconds. Value 0 can be used if no timeout needed while
 * \ref CYHAL_IPC_NEVER_TIMEOUT can be used to make function block until semaphore is successfully taken.
 * @return The status of the take request
 */
cy_rslt_t cyhal_ipc_semaphore_take(cyhal_ipc_t *obj, uint32_t timeout_us);

/** Gives/releases a semaphore.
 *
 * The semaphore is released allowing other tasks waiting on the semaphore to take it.
 * @param[in] obj               The IPC object.
 * @return The status of the give request
 */
cy_rslt_t cyhal_ipc_semaphore_give(cyhal_ipc_t *obj);

/** Creates a new queue for a given IPC channel based on the given queue number and other parameters.
 *
 * This function requires \ref cyhal_ipc_queue_t (queue handle) pointer to shared memory. Some elements of cyhal_ipc_queue_t
 * structure are expected to be filled by user. One key element of the structure to be filled by user is a pointer to
 * the queue pool allocated in the shared memory.
 * Queue handle is used by other tasks/CPUs to refer to the queue. Note that this function must be called only by one
 * of the tasks/CPUs for the same IPC channel. This CPU can call the function multiple times for the same IPC
 * channel, but with a different queue number.
 * \note CYHAL_IPC_QUEUE_HANDLE_ALLOC and CYHAL_IPC_QUEUE_POOL_ALLOC macro can (and not mandatory) be used in order to
 * allocate memory for (respectively) queue handle (cyhal_ipc_queue_t) and queue pool in shared section.
 * Please refer to implementation specific documentation for the requirements for memory allocation if macro is not used.
 * Please refer to \ref subsection_ipc_snippet2 for initialization guidance.
 * @param[out] obj              Pointer to an IPC object. The caller must allocate the memory for this object
 * but the init function will initialize its contents.
 * @param[in] queue_handle      Queue handle. Fields channel_num, queue_num, queue_pool, curr_items and item_size
 * are expected to be filled by user before initialization. Please refer to \ref subsection_ipc_snippet2 for
 * initialization guidance.
 * @return The status of the init request
 */
cy_rslt_t cyhal_ipc_queue_init(cyhal_ipc_t *obj, cyhal_ipc_queue_t *queue_handle);

/** Frees the queue.
 *
 * This operation only removes the queue handle from the list of available queues. The queue pool and the queue
 * handle allocated in the shared memory needs to be freed (if dynamically allocated) by the application.
 * @param[in, out] obj          The IPC object
 *
 */
void cyhal_ipc_queue_free(cyhal_ipc_t *obj);

/** Gets a handle pointer for a given IPC channel and queue number.
 *
 * This function should be called by other tasks/CPUs that have not called the initialization function.
 * Unpredicted behavior can happen if this function is called before \ref cyhal_ipc_queue_init. Please refer
 * to implementation specific documentation for additional details.
 * @param[out] obj              The IPC object handle.
 * @param[in] channel_num       IPC channel to use for the queue messaging.
 * @param[in] queue_num         Queue number.
 * @return The status of the get handle request
 */
cy_rslt_t cyhal_ipc_queue_get_handle(cyhal_ipc_t *obj, uint32_t channel_num, uint32_t queue_num);

/** Registers a callback to be executed when certain events occur.
 *
 * @param[in] obj               The IPC object.
 * @param[in] callback          The callback handler which will be invoked when an event triggers.
 * @param[in] callback_arg      Generic argument that will be provided to the callback when called.
 */
void cyhal_ipc_queue_register_callback(cyhal_ipc_t *obj, cyhal_ipc_event_callback_t callback, void *callback_arg);

/** Enables which events trigger the callback execution.
 *
 * It can trigger when a new item is written to the queue, read from the queue, when the queue becomes full,
 * when the queue becomes empty or when there is a reset. Note that these events might execute callbacks
 * associated to all queues that belong to an IPC channel. When defining the ISR priority, the last call to
 * this function overwrites the priority for all queue callbacks registered to the same IPC channel.
 * @param[in] obj               The IPC object
 * @param[in] event             The IPC event type
 * @param[in] intr_priority     The priority for NVIC interrupt events
 * @param[in] enable            True to turn on specified events, False to turn off
 */
void cyhal_ipc_queue_enable_event(cyhal_ipc_t *obj, cyhal_ipc_event_t event, uint8_t intr_priority, bool enable);

/** Adds one item to the queue.
 *
 * This function can be called by any task/CPU. This function has a timeout argument (in microseconds).
 * If the queue is full, it stays there until it times out or the queue is no longer full.
 * This function can be blocking or non-blocking (timeout set to ZERO).
 * @param[in] obj               The IPC object
 * @param[in] msg               Location of message queue item
 * @param[in] timeout_us        Timeout in microseconds. Value 0 can be used if no timeout needed while
 * \ref CYHAL_IPC_NEVER_TIMEOUT can be used to make function block until element is successfully put into the queue.
 * @return The status of the put request
 */
cy_rslt_t cyhal_ipc_queue_put(cyhal_ipc_t *obj, void *msg, uint32_t timeout_us);

/** Removes one item from the queue.
 *
 * This function can be called by any task/CPU. This function has a timeout argument (in microseconds).
 * If the queue is empty, it stays there until it times out or the queue receives a new item.
 * This function can be blocking or non-blocking (timeout set to ZERO).
 * @param[in] obj               The IPC object
 * @param[out] msg              Location of message queue item
 * @param[in] timeout_us        Timeout in microseconds. Value 0 can be used if no timeout needed while
 * \ref CYHAL_IPC_NEVER_TIMEOUT can be used to make function block until element is successfully taken from the queue.
 * @return The status of the get request
 */
cy_rslt_t cyhal_ipc_queue_get(cyhal_ipc_t *obj, void *msg, uint32_t timeout_us);

/** Returns how many items are in the queue.
 *
 * This function can be called by any task/CPU.
 * @param[in] obj               The IPC object
 * @return Number of items in the queue
 */
uint32_t cyhal_ipc_queue_count(cyhal_ipc_t *obj);

/** Clear all the items in the queue.
 *
 * This function can be called by the any task/CPU.
 * @param[in] obj               The IPC object
 * @return The status of the reset request
 */
cy_rslt_t cyhal_ipc_queue_reset(cyhal_ipc_t *obj);

#if defined(__cplusplus)
}
#endif

#ifdef CYHAL_IPC_IMPL_HEADER
#include CYHAL_IPC_IMPL_HEADER
#endif /* CYHAL_IPC_IMPL_HEADER */

/** \} group_hal_ipc */
