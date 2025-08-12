/***************************************************************************//**
* \file cyhal_ipc_impl.h
*
* \brief
* CAT1 specific implementation for IPC API.
*
********************************************************************************
* \copyright
* Copyright 2019-2021 Cypress Semiconductor Corporation (an Infineon company) or
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

#pragma once

#if (CYHAL_DRIVER_AVAILABLE_IPC)


#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
 * \addtogroup group_hal_impl_ipc IPC
 * \ingroup group_hal_impl
 * \{
 * Implementation specific interface for using the IPC driver.
 *
 */

/**
* \cond INTERNAL
*/

#if defined(COMPONENT_CAT1A)
    /* On CAT1A devices CY_IPC_CHANNELS is not available at compile time */
    /* Last channel is used by PDL Semaphores
     * Next to last channel is used by HAL IPC Driver
     */
    #define _CYHAL_IPC_DRV_CHANNELS     ( (CPUSS_IPC_IPC_NR - 1) - 1)
#elif defined(COMPONENT_CAT1D)
    /* Last channel is used by PDL Semaphores
     * Next to last channel is used by HAL IPC Driver
     */
    #define _CYHAL_IPC_DRV_CHANNELS     ( (CY_IPC_IP1_CH - 1) - 1)
#else
    /* Last channel is used by PDL Semaphores
     * Next to last channel is used by HAL IPC Driver
     */
    #define _CYHAL_IPC_DRV_CHANNELS     ( (CPUSS_IPC_IPC_NR - 1) - 1)
#endif /* defined(COMPONENT_CAT1A) or defined(COMPONENT_CAT1D) or other */

/** Definition of _CYHAL_IPC_CHAN_USER which stands for first user-available IPC channel index */
#if defined(CY_IPC_CHAN_USER)
#if defined(COMPONENT_CAT1D)
    /* On CAT1D device, there are two IPC instances and second one (IPC1) is intended for user application
    (CM33 <-> CM55 communication). PDL driver handles instances by having one channel pool - from 0 to 31,
    where first 16 belong to IPC0 and second 16 channels - for IPC1. */
    #define _CYHAL_IPC_CHAN_USER        (CY_IPC_CHAN_USER - IPC0_IPC_NR)
#else
    #define _CYHAL_IPC_CHAN_USER        (CY_IPC_CHAN_USER)
#endif
#else
    #if defined(COMPONENT_CAT1A)
        #define _CYHAL_IPC_CHAN_USER    (CY_IPC_CHAN_DDFT + 1)
    #elif defined(COMPONENT_CAT1B)
        #define _CYHAL_IPC_CHAN_USER    (0u)
    #else
        #error "Unhandled device"
    #endif /* defined(COMPONENT_CAT1A) or defined(COMPONENT_CAT1B) or other */
#endif /* !defined(CY_IPC_CHAN_USER) */

/** Number of HAL IPC Channels available to Applications */
#define CYHAL_IPC_USR_CHANNELS                 ((_CYHAL_IPC_DRV_CHANNELS) - _CYHAL_IPC_CHAN_USER)

/** Number of available for IPC HAL user channels */
#define CYHAL_IPC_USR_CHANNELS                 ((_CYHAL_IPC_DRV_CHANNELS) - _CYHAL_IPC_CHAN_USER)

#if (CYHAL_IPC_USR_CHANNELS > 0)
    /** User IPC channel 0 */
    #define CYHAL_IPC_CHAN_0    (_CYHAL_IPC_CHAN_USER)
#endif /* CYHAL_IPC_USR_CHANNELS > 0 */
#if (CYHAL_IPC_USR_CHANNELS > 1)
    /** User IPC channel 1 */
    #define CYHAL_IPC_CHAN_1    (_CYHAL_IPC_CHAN_USER + 1)
#endif /* CYHAL_IPC_USR_CHANNELS > 1 */
#if (CYHAL_IPC_USR_CHANNELS > 2)
    /** User IPC channel 2 */
    #define CYHAL_IPC_CHAN_2    (_CYHAL_IPC_CHAN_USER + 2)
#endif /* CYHAL_IPC_USR_CHANNELS > 2 */
#if (CYHAL_IPC_USR_CHANNELS > 3)
    /** User IPC channel 3 */
    #define CYHAL_IPC_CHAN_3    (_CYHAL_IPC_CHAN_USER + 3)
#endif /* CYHAL_IPC_USR_CHANNELS > 3 */
#if (CYHAL_IPC_USR_CHANNELS > 4)
    /** User IPC channel 4 */
    #define CYHAL_IPC_CHAN_4    (_CYHAL_IPC_CHAN_USER + 4)
#endif /* CYHAL_IPC_USR_CHANNELS > 4 */
#if (CYHAL_IPC_USR_CHANNELS > 5)
    /** User IPC channel 5 */
    #define CYHAL_IPC_CHAN_5    (_CYHAL_IPC_CHAN_USER + 5)
#endif /* CYHAL_IPC_USR_CHANNELS > 5 */
#if (CYHAL_IPC_USR_CHANNELS > 6)
    /** User IPC channel 6 */
    #define CYHAL_IPC_CHAN_6    (_CYHAL_IPC_CHAN_USER + 6)
#endif /* CYHAL_IPC_USR_CHANNELS > 6 */
#if (CYHAL_IPC_USR_CHANNELS > 7)
    /** User IPC channel 7 */
    #define CYHAL_IPC_CHAN_7    (_CYHAL_IPC_CHAN_USER + 7)
#endif /* CYHAL_IPC_USR_CHANNELS > 7 */
#if (CYHAL_IPC_USR_CHANNELS > 8)
    /** User IPC channel 8 */
    #define CYHAL_IPC_CHAN_8    (_CYHAL_IPC_CHAN_USER + 8)
#endif /* CYHAL_IPC_USR_CHANNELS > 8 */
#if (CYHAL_IPC_USR_CHANNELS > 9)
    /** User IPC channel 9 */
    #define CYHAL_IPC_CHAN_9    (_CYHAL_IPC_CHAN_USER + 9)
#endif /* CYHAL_IPC_USR_CHANNELS > 9 */
#if (CYHAL_IPC_USR_CHANNELS > 10)
    /** User IPC channel 10 */
    #define CYHAL_IPC_CHAN_10    (_CYHAL_IPC_CHAN_USER + 10)
#endif /* CYHAL_IPC_USR_CHANNELS > 10 */
#if (CYHAL_IPC_USR_CHANNELS > 11)
    /** User IPC channel 11 */
    #define CYHAL_IPC_CHAN_11    (_CYHAL_IPC_CHAN_USER + 11)
#endif /* CYHAL_IPC_USR_CHANNELS > 11 */
#if (CYHAL_IPC_USR_CHANNELS > 12)
    /** User IPC channel 12 */
    #define CYHAL_IPC_CHAN_12    (_CYHAL_IPC_CHAN_USER + 12)
#endif /* CYHAL_IPC_USR_CHANNELS > 12 */
#if (CYHAL_IPC_USR_CHANNELS > 13)
    /** User IPC channel 13 */
    #define CYHAL_IPC_CHAN_13    (_CYHAL_IPC_CHAN_USER + 13)
#endif /* CYHAL_IPC_USR_CHANNELS > 13 */
#if (CYHAL_IPC_USR_CHANNELS > 14)
    /** User IPC channel 14 */
    #define CYHAL_IPC_CHAN_14    (_CYHAL_IPC_CHAN_USER + 14)
#endif /* CYHAL_IPC_USR_CHANNELS > 14 */
#if (CYHAL_IPC_USR_CHANNELS > 15)
    /** User IPC channel 15 */
    #define CYHAL_IPC_CHAN_15    (_CYHAL_IPC_CHAN_USER + 15)
#endif /* CYHAL_IPC_USR_CHANNELS > 15 */
#if (CYHAL_IPC_USR_CHANNELS > 16)
    #error "Unhandled number of free IPC channels"
#endif /* CYHAL_IPC_USR_CHANNELS > 16 */

/** Definition of _CYHAL_IPC_INTR_USER which stands for first user-available IPC interrupt structure index */
#if defined(CY_IPC_INTR_USER)
#if defined(COMPONENT_CAT1D)
#define _CYHAL_IPC_INTR_USER            (CY_IPC_INTR_USER - IPC0_IPC_IRQ_NR)
#else
#define _CYHAL_IPC_INTR_USER            (CY_IPC_INTR_USER)
#endif
#elif defined(CY_IPC_INTR_SPARE)
#define _CYHAL_IPC_INTR_USER            (CY_IPC_INTR_SPARE)
#else
#define _CYHAL_IPC_INTR_USER            (CYHAL_IPC_CHAN_0)
#endif

/* Channel & Interrupt Use on PSE84 (CAT1D)
 *  These defines are in cyhal_ipc_impl.h
 *
 * PDL IPC Semaphore uses Channel 15 to protect Semaphore operations
 *  _CYHAL_IPC_PDL_SEMA_CHAN
 *
 * HAL IPC Driver uses Channel 14 to protect HAL IPC Sema & Queue operations
 *  _CYHAL_IPC_INTERNAL_USE_CHAN
 *   When using this define to access PDL and DRV APIs, we need to use the macro
 *      _CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(_CYHAL_IPC_INTERNAL_USE_CHAN)
 *
 * HAL IPC Driver uses IRQ 15 for interrupt stuff
 *   _CYHAL_IPC_INTERNAL_USE_IRQ
 *   When using this define to access PDL and DRV APIs, we need to use the macro
 *      _CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(_CYHAL_IPC_INTERNAL_USE_IRQ)
 */
#if defined(COMPONENT_CAT1D)
#define _CYHAL_IPC_FIX_CHAN_NUM(channel)                    (channel)
/* On CAT1D device, there are two IPC instances and second one (IPC1) is intended for user application
* (CM33 <-> CM55 communication). PDL driver handles instances by having one channel pool - from 0 to 31,
* where first 16 belong to IPC0 and second 16 channels - for IPC1. */
#define _CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(channel)        ((channel) + IPC0_IPC_NR)
#define _CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(channel)        ((channel) + IPC0_IPC_IRQ_NR)
#else
/* TODO:  Review _CYHAL_IPC_FIX_CHAN_NUM define for CAT1A: in the new implementation
 *  we are operating with this define with an internal HAL objects.
 */
// #define _CYHAL_IPC_FIX_CHAN_NUM(channel)                    ((channel) + CYHAL_IPC_CHAN_0)
#define _CYHAL_IPC_FIX_CHAN_NUM(channel)                    ((channel))
#define _CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(channel)        ((channel))
#define _CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(channel)        ((channel))
#endif

/* TODO:  Review _CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE define for CAT1A: in the new implementation
 *  we are operating with this define with an internal HAL objects.
 */
//#if (CYHAL_IPC_CHAN_0 > 0)
//#define _CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE(channel)    (((channel) >= CYHAL_IPC_CHAN_0) && ((channel) < (CYHAL_IPC_CHAN_0 + CYHAL_IPC_USR_CHANNELS)))
//#else
#define _CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE(channel)    ((channel) < (CYHAL_IPC_CHAN_0 + CYHAL_IPC_USR_CHANNELS))
//#endif /* (CYHAL_IPC_CHAN_0 > 0) or other */

#define _CYHAL_IPC_RELEASE_INTR_BITS            (16u)

#ifdef CY_IPC_CHAN_SEMA
#define _CYHAL_IPC_PDL_SEMA_CHAN                (CY_IPC_CHAN_SEMA)
#elif defined(COMPONENT_CAT1D)
/* Last available channel is used for PDL semaphores */
#define _CYHAL_IPC_PDL_SEMA_CHAN                (CY_IPC_IP1_CH - 1)
#endif /* ifdef CY_IPC_CHAN_SEMA or other */

/* Definition of the Channel used by the HAL IPC Driver for interal use */
#define _CYHAL_IPC_INTERNAL_USE_CHAN            (_CYHAL_IPC_PDL_SEMA_CHAN - 1)

/** \endcond */

/** Number of semaphores, that can be used */
#ifdef CY_IPC_SEMA_COUNT
#define CYHAL_IPC_SEMA_COUNT               (CY_IPC_SEMA_COUNT)
#else
#define CYHAL_IPC_SEMA_COUNT               (128u)
#endif /* ifdef CY_IPC_SEMA_COUNT or other */

/** Macro for Queue pool shared memory allocation. Allocation of queue pool should be done for each queue being initialized,
 * but not mandatory with help of this macro. This macro can be used only in function scope.
 * Please refer to implementation specific documentation for the requirements for memory allocation if macro is not used.
 * Params:
 * queue_pool - void pointer to point to the shared memory
 * NUM_ITEMS - number of items, that are expected to fit into the queue
 * ITEMSIZE - size of one queue item (in bytes)
 */
#if ( (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE) ) || ( (CY_CPU_CORTEX_M55) && defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U) )
#define CYHAL_IPC_QUEUE_POOL_ALLOC(queue_pool, NUM_ITEMS, ITEMSIZE) \
    do { CY_SECTION_SHAREDMEM static uint8_t _cyhal_ipc_queue_pool[ITEMSIZE * NUM_ITEMS] CY_ALIGN(__SCB_DCACHE_LINE_SIZE); queue_pool = (void*)&_cyhal_ipc_queue_pool; } while (0)
#else
#define CYHAL_IPC_QUEUE_POOL_ALLOC(queue_pool, NUM_ITEMS, ITEMSIZE) \
    do { CY_SECTION_SHAREDMEM static uint8_t _cyhal_ipc_queue_pool[ITEMSIZE * NUM_ITEMS]; queue_pool = (void*)&_cyhal_ipc_queue_pool; } while (0)
#endif /* (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE) */

/** Macro for Queue handle shared memory allocation. It is not mandatory to use it for handle allocation, user can allocate
 * the memory in the more convenient for them way. Please refer to implementation specific documentation for the
 * requirements for memory allocation if macro is not used.
 *
 * Can be used only in function scope.
 * Params:
 * queue_handle - pointer to cyhal_ipc_queue_t data type, which will point to the shared memory
 */
#if ( (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE) ) || ( (CY_CPU_CORTEX_M55) && defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U) )
#define CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue_handle) \
    do { CY_SECTION_SHAREDMEM static cyhal_ipc_queue_t _cyhal_ipc_queue_handle CY_ALIGN(__SCB_DCACHE_LINE_SIZE); queue_handle = &_cyhal_ipc_queue_handle; } while (0)
#else
#define CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue_handle) \
    do { CY_SECTION_SHAREDMEM static cyhal_ipc_queue_t _cyhal_ipc_queue_handle; queue_handle = &_cyhal_ipc_queue_handle; } while (0)
#endif /* (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE) */

/** Polling interval, that will be used in blocking cyhal_ipc_* functions.
 * It is recommended to use value either below 1000 (e.g. 750) or multiple of 1000 (e.g. 1000, 2000, 3000, etc.)
 */
#ifndef CYHAL_IPC_POLLING_INTERVAL_uS
#define CYHAL_IPC_POLLING_INTERVAL_uS   (1000u)
#endif /* #ifndef CYHAL_IPC_POLLING_INTERVAL_uS */

/** This macro defines what device's core will initialize IPC PDL semaphores in scope of cyhal_ipc_semaphore_init call.
 * Please refer to implementation specific documentation for details.
 */
#if !defined(CYHAL_IPC_INIT_CORE)
#if defined(COMPONENT_CAT1A) && defined(CORE_NAME_CM0P_0)
#define CYHAL_IPC_INIT_CORE             1
#elif defined(COMPONENT_CAT1C) && defined(CORE_NAME_CM7_0)
#define CYHAL_IPC_INIT_CORE             1
#elif defined(COMPONENT_CAT1D) && defined(CORE_NAME_CM33_0)
#define CYHAL_IPC_INIT_CORE             1
#endif /* defined(COMPONENT_CAT1C) or defined(COMPONENT_CAT1D) */
#endif /* not defined (CYHAL_IPC_INIT_CORE) */

/** Number of RTOS semaphores, that will be allocated and used by driver in RTOS environment
 * (CY_RTOS_AWARE or COMPONENT_RTOS_AWARE should be defined). Usage of RTOS semaphores in IPC semaphores implementation
 * helps to utilize waiting for semaphores times in RTOS environment more effectively by allowing other threads to run
 * while waiting for a semaphore. To achieve most effectiveness,
 * it is recommended to define CYHAL_IPC_RTOS_SEMA_NUM value to be greater-equal to the number of IPC semaphores, that
 * are planned to be used. Only semaphores with `semaphore_num`, that is less than _CYHAL_IPC_RELEASE_INTR_BITS can
 * benefit from this feature.
 * Value of this define can be 0. In this case, IPC HAL semaphores will not use RTOS semaphores.*/
#ifndef CYHAL_IPC_RTOS_SEMA_NUM
#define CYHAL_IPC_RTOS_SEMA_NUM         (4u)
#endif /* #ifndef CYHAL_IPC_RTOS_SEMA_NUM */

#if (CYHAL_IPC_RTOS_SEMA_NUM > _CYHAL_IPC_RELEASE_INTR_BITS)
#error "Cannot handle selected amount of RTOS semaphores. Please fix CYHAL_IPC_RTOS_SEMA_NUM value"
#endif /* CYHAL_IPC_RTOS_SEMA_NUM > _CYHAL_IPC_RELEASE_INTR_BITS */

#if defined(COMPONENT_CAT1A) || defined(COMPONENT_CAT1D)
/* Number of cores being serviced by the driver */
#define _CYHAL_IPC_CORE_NUM                         (2)

#if defined(COMPONENT_CAT1A)
#define _CYHAL_IPC_CM0P_IDX                         (0)     //!< Boot Core
#define _CYHAL_IPC_CM4_IDX                          (1)
#else /* defined(COMPONENT_CAT1D) */
#define _CYHAL_IPC_CM33_IDX                         (0)     //!< Boot Core
#define _CYHAL_IPC_CM55_IDX                         (1)
#endif

/* IPC INTR of HAL IPC CHAN 0 is used to service interrupts on CM0p, IPC_CHAN 1 is used for CM4 (for CAT1A devices) and
* IPC CHAN 0 - for CM33, IPC CHAN 1 - for CM55 */

#define _CYHAL_IPC_TRIGGER_ISR_MASK                 ((1UL << _CYHAL_IPC_INTR_USER) | (1UL << (_CYHAL_IPC_INTR_USER + 1)))

#if defined(COMPONENT_CAT1A)
#if (CY_CPU_CORTEX_M0P)
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM0P_IDX)
#define _CYHAL_IPC_OTHER_CORE_IDX                   (_CYHAL_IPC_CM4_IDX)
#else /* CY_CPU_CORTEX_M4 */
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER + 1)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM4_IDX)
#define _CYHAL_IPC_OTHER_CORE_IDX                   (_CYHAL_IPC_CM0P_IDX)
#endif /* CY_CPU_CORTEX_M0P or CY_CPU_CORTEX_M4 */
#else
#if (CY_CPU_CORTEX_M33)
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM33_IDX)
#define _CYHAL_IPC_OTHER_CORE_IDX                   (_CYHAL_IPC_CM55_IDX)
#else /*CY_CPU_CORTEX_M55 */
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER + 1)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM55_IDX)
#define _CYHAL_IPC_OTHER_CORE_IDX                   (_CYHAL_IPC_CM33_IDX)
#endif /* CY_CPU_CORTEX_M33 or CY_CPU_CORTEX_M55 */
#endif /* defined(COMPONENT_CAT1A) or other (defined(COMPONENT_CAT1D)) */

#elif defined(COMPONENT_CAT1C)
/* Number of cores being serviced by the driver */
#define _CYHAL_IPC_CORE_NUM                         (3)

#if defined(COMPONENT_CAT1C)
#define _CYHAL_IPC_CM0P_IDX                         (0)      //!< Boot Core
#define _CYHAL_IPC_CM7_0_IDX                        (1)
#define _CYHAL_IPC_CM7_1_IDX                        (2)

#if (CY_CPU_CORTEX_M0P)
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM0P_IDX)
#define _CYHAL_IPC_OTHER_CORE_0_IDX                 (_CYHAL_IPC_CM7_0_IDX)
#define _CYHAL_IPC_OTHER_CORE_1_IDX                 (_CYHAL_IPC_CM7_1_IDX)
#elif (CY_CPU_CORTEX_M7)
#if (CORE_NAME_CM7_0)
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER + 1)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM7_0_IDX)
#define _CYHAL_IPC_OTHER_CORE_0_IDX                 (_CYHAL_IPC_CM0P_IDX)
#define _CYHAL_IPC_OTHER_CORE_1_IDX                 (_CYHAL_IPC_CM7_1_IDX)
#elif (CORE_NAME_CM7_1)
#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN       (_CYHAL_IPC_INTR_USER + 2)
#define _CYHAL_IPC_CUR_CORE_IDX                     (_CYHAL_IPC_CM7_1_IDX)
#define _CYHAL_IPC_OTHER_CORE_0_IDX                 (_CYHAL_IPC_CM0P_IDX)
#define _CYHAL_IPC_OTHER_CORE_1_IDX                 (_CYHAL_IPC_CM7_0_IDX)
#else
#error "Unable to determine CM7 core index"
#endif /* defined (ACTIVE_CORE_CM7_0) or defined (ACTIVE_CORE_CM7_0) or error */
#endif /* CY_CPU_COERTX_M0P or CY_CPU_CORTEX_M7 */
#endif /* defined(COMPONENT_CAT1C) */

#define _CYHAL_IPC_TRIGGER_ISR_MASK                 ((1UL << _CYHAL_IPC_INTR_USER) | (1UL << (_CYHAL_IPC_INTR_USER + 1)) | (1UL << (_CYHAL_IPC_INTR_USER + 2)))
#else /* not CAT1A / CAT1D / CAT1C */
    #error "Unhandled device"
#endif /* defined(COMPONENT_CAT1A) or defined(COMPONENT_CAT1D) or defined(COMPONENT_CAT1C) */

#if defined(COMPONENT_CAT1D)
#define _CYHAL_IPC_IPC_INTR_SRC_BASE                (m55appcpuss_interrupts_ipc_dpslp_0_IRQn)
#else
#define _CYHAL_IPC_IPC_INTR_SRC_BASE                (cpuss_interrupts_ipc_0_IRQn)
#endif

/** \} group_hal_impl_ipc */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* CYHAL_DRIVER_AVAILABLE_IPC */
