/***************************************************************************//**
* \file cyhal_ipc.c
*
* \brief
* Provides a high level interface for interacting with the Infineon Inter Processor Communication.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low level functions
* can be used directly.
*
********************************************************************************
* \copyright
* Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company) or
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
*/

/*
* \section section_hal_impl_ipc_number Number of IPC HAL channels, that are available
* Number of IPC HAL channels (CYHAL_IPC_CHAN_0 ... CYHAL_IPC_CHAN_N) can be accessed via CYHAL_IPC_USR_CHANNELS define.
* Here is the list of currently supported devices and their IPC HAL available channels:
* - CAT1A : 8, valid range is from CYHAL_IPC_CHAN_0 to (including) CYHAL_IPC_CHAN_7
* - CAT1C : 3, valid range is from CYHAL_IPC_CHAN_0 to (including) CYHAL_IPC_CHAN_2
* - CAT1D : 15, valid range is from CYHAL_IPC_CHAN_0 to (including) CYHAL_IPC_CHAN_14
*
*/

/** \} group_hal_ipc */

#include <string.h>
#include "cyhal_utils.h"
#include "cy_syslib.h"
#include "cyhal_ipc.h"
#include "cyhal_system.h"
#include "cyhal_irq_impl.h"

#if (CYHAL_DRIVER_AVAILABLE_IPC)

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
/* RTOS_AWARE is for semaphore usage so that in a multi-threaded system, we can allow other
 * threads to run if the current thread is blocked by a HAL/PDL IPC Semaphore.
 */
#include "cyabs_rtos.h"
#endif /* (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0) */

#if defined(ENABLE_CYHAL_IPC_DEBUG_PRINT)
#include <stdio.h>
#if defined(CYHAL_IPC_INIT_CORE)
#warning DEBUG INFO: Building for CYHAL_IPC_INIT_CORE
#endif
#if defined(CORE_NAME_CM0P_0)
#warning DEBUG INFO: Building for CORE_NAME_CM0P_0
#endif
#if defined(CORE_NAME_CM4_0)
#warning DEBUG INFO: Building for CORE_NAME_CM4_0
#endif
#if defined(CORE_NAME_CM33)
#warning DEBUG INFO: Building for CORE_NAME_CM33
#endif
#if defined(CORE_NAME_CM55)
#warning DEBUG INFO: Building for CORE_NAME_CM55
#endif
#if defined(CORE_NAME_CM7_0)
#warning DEBUG INFO: Building for CORE_NAME_CM7_0
#endif
#if defined(CORE_NAME_CM7_1)
#warning DEBUG INFO: Building for CORE_NAME_CM7_1
#endif
#endif // ENABLE_CYHAL_IPC_DEBUG_PRINT

#if defined(__cplusplus)
extern "C" {
#endif

#if (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE)
/** This define can be used for aligning address of the data structures with enabled Data Cache */
#define _CYHAL_IPC_DATA_ALIGN       CY_ALIGN(__SCB_DCACHE_LINE_SIZE)
#else
/** This define can be used for aligning address of the data structures with enabled Data Cache */
#define _CYHAL_IPC_DATA_ALIGN
#endif /* (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE) */


/* Shared memory is non-cachable memory. Currently we invalidate/clean all memory as if it is cached.
 *In Phase 2 we need to reduce the scope of these calls in order to speed up processing.
 *
 * Before Reading non-Shared Memory:
 *     SCB_InvalidateDCache_by_Addr(addr,size)  for invalidating the D-Cache (to Read RAM, updating the cache)
 * After Writing to non-Shared Memory:
 *     SCB_CleanDCache_by_Addr(addr,size) for cleaning the D-Cache (writing the cache through to RAM)
 */
#if ( ( (CY_CPU_CORTEX_M7) && defined (ENABLE_CM7_DATA_CACHE) ) || ( (CY_CPU_CORTEX_M55) && defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U) ) )
/* NOTES:
 *  D-Cache is invalidated starting from a 32 byte aligned address in 32 byte granularity.
 *  D-Cache memory blocks which are part of given address + given size are invalidated.
 */
#define INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(addr,size)      \
    SCB_InvalidateDCache_by_Addr( (volatile void *)( (uint32_t)addr & ~(__SCB_DCACHE_LINE_SIZE - 1) ), ( (size + (__SCB_DCACHE_LINE_SIZE - 1) ) & ~(__SCB_DCACHE_LINE_SIZE - 1) ) )

#define CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(addr,size)              \
         SCB_CleanDCache_by_Addr( (volatile void *)( (uint32_t)addr & ~(__SCB_DCACHE_LINE_SIZE - 1) ), ( (size + (__SCB_DCACHE_LINE_SIZE - 1) ) & ~(__SCB_DCACHE_LINE_SIZE - 1) ) )
#else
#define INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(addr,size)
#define CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(addr,size)
#endif

/* This is the define to be used when we add support for multiple processes per queue
 * in Phase 2 work.
 */
#if defined(CYHAL_IPC_MULTI_PROCESS_PER_QUEUE)
    #error "Code needed for Multiple Processes per Queue"
#endif

/* Debugging: Enable to check that HAL Semaphore "take" status is the same as the PDL "set" state.
 * This test happens before & after all HAL Semaphore operations if enabled.
 */
#if defined(CYHAL_IPC_INIT_CORE)
//#define ENABLE_CYHAL_IPC_PDL_SEMAPHORE_CHECK    1
#endif

/* Enable this for printing on NON-INIT Core
 */
#if defined(ENABLE_CYHAL_IPC_DEBUG_PRINT)
#if !defined(CYHAL_IPC_INIT_CORE)
#define _CYHAL_NON_INIT_CORE_PRINTF(arg)     printf arg
#else
#define _CYHAL_NON_INIT_CORE_PRINTF(arg)
#endif
#else
#define _CYHAL_NON_INIT_CORE_PRINTF(arg)
#endif

#if defined(ENABLE_CYHAL_IPC_DEBUG_PRINT)
#if defined(CYHAL_IPC_INIT_CORE)
#define _CYHAL_INIT_CORE_PRINTF(arg)     printf arg
#else
#define _CYHAL_INIT_CORE_PRINTF(arg)
#endif
#else
#define _CYHAL_INIT_CORE_PRINTF(arg)
#endif

/* IPC semaphore timeout when performing internal IPC service tasks NON_ISR */
#define _CYHAL_IPC_SERVICE_SEMA_TIMEOUT_US      (2000)

/* HAL IPC Driver uses the last available semaphore
 * Queue Semaphores and Queue Notification semaphores count down from there.
 * Application Semaphores count up from !st semaphore.
 */
#define _CYHAL_IPC_DRIVER_SEMA_NUM              (CYHAL_IPC_SEMA_COUNT - 1)
#define _CYHAL_LAST_USER_SEMA_NUM               (_CYHAL_IPC_DRIVER_SEMA_NUM)

#define _CYHAL_SEMA_USAGE_BITS_ARRAY_SIZE       (CYHAL_IPC_SEMA_COUNT / CY_IPC_SEMA_PER_WORD)

#define _CYHAL_IPC_CURRENT_CORE_IPC_INTR_SRC        ((_cyhal_system_irq_t)(_CYHAL_IPC_IPC_INTR_SRC_BASE + _CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN))

/* for determiining if we are executing in an ISR at runtime */
#define _CYHAL_EXECUTING_IN_ISR()               ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0)

/* Use this macro for checking & returning if timeout is != 0 in an ISR */
#define _CYHAL_CHECK_TIMEOUT_AND_RETURN_IF_IN_ISR(timeout)      \
        if (  _CYHAL_EXECUTING_IN_ISR() && ( (timeout) != 0) )    \
        {                                                                           \
                return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;            \
        }

/* Use this macro for checking that timeout in an ISR is 0 */
#define _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout)      \
        if ( _CYHAL_EXECUTING_IN_ISR() && ( (timeout) != 0) )    \
        {                               \
                (timeout) = 0;            \
        }



/** HAL IPC interrupts enabled for this Core */
static bool _cyhal_ipc_interrupts_initialized = false;

/** Local knowledge that semas have been initialized */
static bool _cyhal_ipc_PDL_semas_initialized = false;

/** Local indicator that HAL IPC has been initialized. Local indicator is updated when _cyhal_ipc_driver_init()
 * for "this" Process is called. Multiple Processes will have a local copy of this variable and must co-ordinate
 * initialization so that ONLY ONE Process does the HAL IPC Driver Shared Memory "initialization".
 * The subsequent Processes do not. See PDL function Cy_IPC_Sema_Init for more info.
 * The first time "this" Process calls cyhal_ipc_queue_init(), this value will
 * be 0x00, as it is cleared during system power-up in the .bss section of RAM variables.
 * This indicates that "this" Process must check to see if the HAL IPC Driver Shared Memory has been initialized.
 * If the value is non-zero, it means that "this" Process already checked that the
 * HAL IPC Driver Shared Memory variables have been initialized. */
static bool _cyhal_ipc_local_initialized = false;

#define _CYHAL_IPC_UNUSED_SEMAPHORE                 (0xFFFF)
#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
#define _CYHAL_IPC_SEMA_INTR_STR_NUM                (_CYHAL_IPC_INTR_USER + _CYHAL_IPC_CORE_NUM)

typedef struct
{
    cy_semaphore_t      rtos_semaphore;
    uint32_t            sema_num;
} _cyhal_ipc_rtos_sema_t;
#endif /* (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0) */

/** This structure holds the shared variables between all cores.
 *  The address of the structure is passed through an IPC channel from the INIT Core (Boot) to the other Cores.
 *  see _cyhal_ipc_register_interrupts_for_this_core().
 */
typedef struct
{
    /** Local pointer to the IPC Driver Semaphore for changing any HAL IPC Driver variables
     * Declared as this:
     * CY_SECTION_SHAREDMEM static cyhal_ipc_t _cyhal_ipc_semaphore_base _CYHAL_IPC_DATA_ALIGN;
     */
    cyhal_ipc_t         *ipc_driver_semaphore;

    /** Local HAL IPC Semaphore bit field usage for Semaphore indexes in use
     *  Declared as this:
     *  CY_SECTION_SHAREDMEM static uint32_t _cyhal_ipc_sema_usage_bits_base[ CYHAL_IPC_SEMA_COUNT / CY_IPC_SEMA_PER_WORD ] _CYHAL_IPC_DATA_ALIGN;
     */
    uint32_t            *ipc_semaphore_usage_bits; // _cyhal_ipc_sema_usage_bits

    /** HAL IPC Semaphore linked list for lookup of already initialized semaphores
     *  Declared as this:
     *  CY_SECTION_SHAREDMEM static cyhal_ipc_t         *_cyhal_ipc_sema_linked_list_base _CYHAL_IPC_DATA_ALIGN;
     */
    cyhal_ipc_t         *ipc_semaphore_list;    // _cyhal_ipc_sema_linked_list_base

    /** List of initialized array of HAL IPC Queue pointers -- protect with _cyhal_ipc_semaphore
     * Arranged by Channel to efficiently scan through list when ISR occurs on a channel.
     * When accessing this array, adjust channel by _CYHAL_IPC_FIX_CHAN_NUM(channel) to account for different device channel support.
     * Some devices have first User Channel > 0.
     *  Declared as this:
     *  CY_SECTION_SHAREDMEM static cyhal_ipc_queue_t *_cyhal_ipc_queue_array_base[ CYHAL_IPC_USR_CHANNELS ][ _CYHAL_IPC_QUEUES_PER_CHANNEL ] _CYHAL_IPC_DATA_ALIGN;
     *  To access, use the function _cyhal_ipc_queue_array_get_pointer(channel_num, queue_num);
     */
    cyhal_ipc_queue_t   **ipc_queue_array;  // _cyhal_ipc_queue_array_base

} _cyhal_ipc_shared_t;


#if defined(CYHAL_IPC_INIT_CORE)
/** The struct for storing the required HAL IPC shared variables.
 *  The pointer to this structure is shared with the other Cores so all Cores work with same variables.
 */
CY_SECTION_SHAREDMEM static _cyhal_ipc_shared_t _cyhal_ipc_shared_vars_base _CYHAL_IPC_DATA_ALIGN;
_cyhal_ipc_shared_t *_cyhal_ipc_shared_vars = &_cyhal_ipc_shared_vars_base;
#else
/* For non-init Cores, we send the ptr over an IPC channel so all the cores share the same variables. */
_cyhal_ipc_shared_t *_cyhal_ipc_shared_vars = NULL;
#endif

/********************************************* FORWARD DECLARATIONS ****************************************************/

static void _cyhal_ipc_irq_handler(void);
static uint32_t  _cyhal_ipc_sema_usage_bit_get_status(uint32_t bit_index);

/* sizes of "_base" shared memory variables */
#define SIZE_OF_SEMAPHORE_BITS_ARRAY    ( _CYHAL_SEMA_USAGE_BITS_ARRAY_SIZE * sizeof(uint32_t) )
#define SIZE_OF_QUEUE_ARRAY             ( (CYHAL_IPC_USR_CHANNELS * _CYHAL_IPC_QUEUES_PER_CHANNEL) * sizeof (cyhal_ipc_queue_t *) )
#define SIZE_OF_RTOS_SEMAPHORE_ARRAY    (_CYHAL_IPC_CORE_NUM * CYHAL_IPC_RTOS_SEMA_NUM * sizeof(_cyhal_ipc_rtos_sema_t) )

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
    static _cyhal_ipc_rtos_sema_t _cyhal_ipc_rtos_semaphore_base[CYHAL_IPC_RTOS_SEMA_NUM];
#endif // (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)

/********************************************* SEMAPHORE Debug helper *******************************************************/
/* Enable above for testing HAL & PDL "taken" status */
#if defined(ENABLE_CYHAL_IPC_PDL_SEMAPHORE_CHECK)

/* debugging */
void _cyhal_ipc_semaphore_validate_HAL_with_PDL(uint32_t line, const char *func_name)
{
    /* go through the semaphore list and check that the PDL has the same state as we do */
    cyhal_ipc_t         *walker = _cyhal_ipc_shared_vars->ipc_semaphore_list;
    /* step through the linked list */
    while (walker != NULL)
    {
        uint32_t sema_num;
        bool HAL_taken;
        bool PDL_taken;
        cy_en_ipcsema_status_t pdl_stat;

        /* check if the semaphore is taken */
        do
        {
            uint32_t interruptState = cyhal_system_critical_section_enter();     // For Thread safety, PDL handles Core safety
            sema_num  = walker->sema_number;
            HAL_taken = (walker->sema_taken == 0) ? false : true;
            PDL_taken = false;
            bool HAL_bit_allocated  = (_cyhal_ipc_sema_usage_bit_get_status(sema_num) == 0) ? false : true;
            pdl_stat = Cy_IPC_Sema_Status(sema_num);
            cyhal_system_critical_section_exit(interruptState);

            switch(pdl_stat)
            {
            case CY_IPC_SEMA_STATUS_LOCKED:
                PDL_taken  = true;
                break;
            case CY_IPC_SEMA_STATUS_UNLOCKED:
                PDL_taken  = false;
                break;
            default:
                PDL_taken  = false;
                break;
            }

            cyhal_system_delay_us(CYHAL_IPC_POLLING_INTERVAL_uS);

        } while (pdl_stat == CY_IPC_SEMA_NOT_ACQUIRED);

        if (HAL_bit_allocated == true)
        {
            if (HAL_taken != PDL_taken)
            {
                _CYHAL_INIT_CORE_PRINTF(("%ld %s() Walker %p : sema: %ld HAL_taken: %d PDL_taken %d\n", line, func_name, walker, sema_num, HAL_taken, PDL_taken));
            }
        }
        else
        {
            _CYHAL_INIT_CORE_PRINTF(("%ld %s() Walker %p : sema: %ld NOT Allocated? %d     HAL_in_use: %d PDL_in_use %d\n", line, func_name, walker, sema_num, HAL_bit_allocated, HAL_taken, PDL_taken));
        }
        walker = walker->next_sema;
    }
}

#else
    #define _cyhal_ipc_semaphore_validate_HAL_with_PDL(line, func)
#endif
/********************************************* SEMAPHORE HELPERS *******************************************************/


/* Get HAL-level IPC Driver Semaphore allocation usage bit. 0 = not allocated, 1 = allocated
 * Lock the HAL IPC Driver Semaphore before calling this function:  cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us)
 */
static uint32_t  _cyhal_ipc_sema_usage_bit_get_status(uint32_t bit_index)
{
    CY_ASSERT(bit_index <  CYHAL_IPC_SEMA_COUNT);
    uint32_t value;
    value = _cyhal_ipc_shared_vars->ipc_semaphore_usage_bits[ bit_index / CY_IPC_SEMA_PER_WORD ] & ((uint32_t)1 << (bit_index % CY_IPC_SEMA_PER_WORD) );
    return value;
}

/* Mark a HAL-level IPC Driver Semaphore allocation usage bit. 0 = not allocated, 1 = allocated
 * Lock the HAL IPC Driver Semaphore before calling this function:  cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us)
 */
static void _cyhal_ipc_sema_usage_bit_mark_used(uint32_t bit_index)
{
    CY_ASSERT(bit_index <  CYHAL_IPC_SEMA_COUNT );
    _cyhal_ipc_shared_vars->ipc_semaphore_usage_bits[ (bit_index / CY_IPC_SEMA_PER_WORD) ] |= ((uint32_t)1 << (bit_index % CY_IPC_SEMA_PER_WORD) );
}

/* Clear a HAL-level IPC Driver Semaphore allocation usage bit. 0 = not allocated, 1 = allocated
 * Lock the HAL IPC Driver Semaphore before calling this function:  cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us)
 */
static void _cyhal_ipc_sema_usage_bit_mark_free(uint32_t bit_index)
{
    CY_ASSERT(bit_index <  CYHAL_IPC_SEMA_COUNT );
    _cyhal_ipc_shared_vars->ipc_semaphore_usage_bits[ (bit_index / CY_IPC_SEMA_PER_WORD) ] &= ~((uint32_t)1 << (bit_index % CY_IPC_SEMA_PER_WORD) );
}

/* Find a free a HAL-level IPC Driver Semaphore allocation usage bit. 0 = not allocated, 1 = allocated
 * Lock the HAL IPC Driver Semaphore before calling this function:  cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us)
 * HAL IPC internal uses semaphores from the highest number downward. Start looking from the end (internal use is from last to first to allow Apps to start with 0).
 * Apps use from 0 up.
 *
 * @param[out]  sema_number - If successful, returns available Semaphore number
 *
 * returns  CY_RSLT_SUCCESS
 *          CYHAL_IPC_RSLT_ERR_NO_SEMA_AVAILABLE - No semaphores are available, look to reduce usage or increase CYHAL_IPC_SEMA_COUNT
 */
static cy_rslt_t _cyhal_ipc_sema_usage_find_free(uint32_t *sema_number)
{
    CY_ASSERT(sema_number != NULL);

    uint32_t i;
    for (i = _CYHAL_IPC_DRIVER_SEMA_NUM; i > 0; i--)
    {
        if ( _cyhal_ipc_sema_usage_bit_get_status(i) == 0)
        {
            *sema_number = i;
            return CY_RSLT_SUCCESS;
        }
    }
    return CYHAL_IPC_RSLT_ERR_NO_SEMA_AVAILABLE;
}

/*
 * Lock the HAL IPC Driver Semaphore before calling this function:
 *  cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us)
 *
 * obj and semaphore_num must be checked before calling this functions
 */
static cy_rslt_t _cyhal_ipc_sema_inner_init(cyhal_ipc_t *obj, uint32_t semaphore_num, bool preemptable)
{
    cy_rslt_t   result = CY_RSLT_SUCCESS;

    /* The PDL IPC Semaphores are not "allocated" on an individual basis.
     * Just make sure the HAL IPC Semaphore has not been already taken.
     */

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    /* Check if this sema has been allocated */
    if (_cyhal_ipc_sema_usage_bit_get_status(semaphore_num) != 0)
    {
        /* Semaphore is already allocated (inited) */
        result = CYHAL_IPC_RSLT_ERR_SEMA_NUM_IN_USE;
    }
    else
    {
        _cyhal_ipc_sema_usage_bit_mark_used(semaphore_num);

        obj->sema_preemptable = preemptable;
        obj->sema_number = semaphore_num;
        obj->sema_taken = 0;
        obj->queue_obj = NULL;              /* Not associated with a Queue yet */

        /* Add to our list of semaphores */
        obj->next_sema = _cyhal_ipc_shared_vars->ipc_semaphore_list;
        _cyhal_ipc_shared_vars->ipc_semaphore_list = obj;

        CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(obj, sizeof(cyhal_ipc_t) );
    }

    return result;
}

cy_en_ipcsema_status_t _cyhal_ipc_sema_PDL_set(cyhal_ipc_t *obj)
{
    IPC_STRUCT_Type *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(_CYHAL_IPC_INTERNAL_USE_CHAN));
    cy_en_ipcsema_status_t pdl_ipc_sema_result = CY_IPC_SEMA_LOCKED;

    uint32_t interruptState = cyhal_system_critical_section_enter();
    if(CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire(ipc_base))
    {
        pdl_ipc_sema_result = Cy_IPC_Sema_Set(obj->sema_number, obj->sema_preemptable);
        if (pdl_ipc_sema_result == CY_IPC_SEMA_SUCCESS)
        {
            obj->sema_taken++;      /* HAL keep track of Sema state */
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(obj, sizeof(cyhal_ipc_t) );
        }
        (void) Cy_IPC_Drv_LockRelease (ipc_base, CY_IPC_NO_NOTIFICATION);
    }
    cyhal_system_critical_section_exit(interruptState);

    return pdl_ipc_sema_result;
}

cy_en_ipcsema_status_t _cyhal_ipc_sema_PDL_clear(cyhal_ipc_t *obj)
{
    IPC_STRUCT_Type *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(_CYHAL_IPC_INTERNAL_USE_CHAN));
    cy_en_ipcsema_status_t pdl_ipc_sema_result = CY_IPC_SEMA_LOCKED;

    uint32_t interruptState = cyhal_system_critical_section_enter();
    if(CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire(ipc_base))
    {
        pdl_ipc_sema_result = Cy_IPC_Sema_Clear(obj->sema_number, obj->sema_preemptable);
        if (pdl_ipc_sema_result == CY_IPC_SEMA_SUCCESS)
        {
            obj->sema_taken--;
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(obj, sizeof(cyhal_ipc_t) );
        }
        else if (pdl_ipc_sema_result == CY_IPC_SEMA_NOT_ACQUIRED)
        {
            /* The PDL return value SEMA_NOT_ACQUIRED means that the semaphore was already cleared (given).
             * We allow for semaphore give multiple times, like in an RTOS, so this is a success.
             */
            obj->sema_taken = 0;
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(obj, sizeof(cyhal_ipc_t) );
        }
        (void) Cy_IPC_Drv_LockRelease (ipc_base, CY_IPC_NO_NOTIFICATION);
    }
    cyhal_system_critical_section_exit(interruptState);

    return pdl_ipc_sema_result;
}
/********************************************** RTOS SEMAPHORE HELPERS ********************************************************/

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)

/* We do this once per core at startup so there is always a callback registered */
static void _cyhal_ipc_register_irq_callback(void)
{
    /* Enable all possible interrupt bits for sema interrupt */
    _cyhal_irq_register((_cyhal_system_irq_t)(_CYHAL_IPC_IPC_INTR_SRC_BASE + _CYHAL_IPC_SEMA_INTR_STR_NUM), CYHAL_ISR_PRIORITY_DEFAULT, _cyhal_ipc_irq_handler);
}

static void _cyhal_ipc_initialize_rtos_semaphores(void)
{
    cy_rslt_t result;

    /* Initialize the RTOS semaphores for the pre-allocated pool */
    for (uint32_t rtos_sema_idx = 0; rtos_sema_idx < CYHAL_IPC_RTOS_SEMA_NUM; ++rtos_sema_idx)
    {
        result = cy_rtos_init_semaphore(&_cyhal_ipc_rtos_semaphore_base[rtos_sema_idx].rtos_semaphore, 1, 0);
        if (CY_RSLT_SUCCESS == result)
        {
            _cyhal_ipc_rtos_semaphore_base[rtos_sema_idx].sema_num = _CYHAL_IPC_UNUSED_SEMAPHORE;
        }
    }
    CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(_cyhal_ipc_rtos_semaphore_base, sizeof(_cyhal_ipc_rtos_semaphore_base) );

}
#endif /* (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0) */


/********************************* HAL IPC Queue Helpers ********************************************/

cyhal_ipc_queue_t *_cyhal_ipc_queue_array_get_pointer(uint32_t channel_num, uint32_t queue_index)
{
    cyhal_ipc_queue_t *queue = NULL;

    if ( (channel_num < CYHAL_IPC_USR_CHANNELS) && (queue_index < _CYHAL_IPC_QUEUES_PER_CHANNEL) )
    {
        /* Macro to access individual items in the queue arrays */
        uint32_t offset = (_CYHAL_IPC_FIX_CHAN_NUM(channel_num) * _CYHAL_IPC_QUEUES_PER_CHANNEL) + queue_index;
        queue = _cyhal_ipc_shared_vars->ipc_queue_array[ offset ];
    }
    return queue;
}

cy_rslt_t _cyhal_ipc_queue_array_set_pointer(uint32_t channel_num, uint32_t queue_index, cyhal_ipc_queue_t *queue )
{
    cy_rslt_t result = CYHAL_IPC_RSLT_ERR_QUEUE_NOT_FOUND;
    if ( (channel_num < CYHAL_IPC_USR_CHANNELS) && (queue_index < _CYHAL_IPC_QUEUES_PER_CHANNEL) )
    {
        uint32_t offset = (_CYHAL_IPC_FIX_CHAN_NUM(channel_num) * _CYHAL_IPC_QUEUES_PER_CHANNEL) + queue_index;
        _cyhal_ipc_shared_vars->ipc_queue_array[ offset ] = queue;
        result = CY_RSLT_SUCCESS;
    }
    return result;
}

/********************************* HAL IPC INTERRUPT HANDLER ********************************************/

static void _cyhal_ipc_irq_handler(void)
{
    _cyhal_system_irq_t irqn = _cyhal_irq_get_active();
    uint32_t isr_channel = irqn - _CYHAL_IPC_IPC_INTR_SRC_BASE;
    IPC_INTR_STRUCT_Type *ipc_intr_base = Cy_IPC_Drv_GetIntrBaseAddr(_CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(isr_channel));

    /* We are interested only in Release events */
    uint32_t interrupt_status_masked_original = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASKED_RELEASE, Cy_IPC_Drv_GetInterruptStatusMasked(ipc_intr_base));
    uint32_t interrupt_status_masked = interrupt_status_masked_original;

    /* Debugging - enable at top of this file */
    _CYHAL_NON_INIT_CORE_PRINTF(("%ld %s() caller:%p, isr_channel(%lu), interrupt_status_masked(0x%lx)\n",
                                __LINE__, __func__, __builtin_return_address(0), isr_channel, interrupt_status_masked));

    Cy_IPC_Drv_ClearInterrupt(ipc_intr_base, interrupt_status_masked_original, 0);

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
    if (_CYHAL_IPC_SEMA_INTR_STR_NUM == isr_channel)
    {
        if (interrupt_status_masked != 0)
        {
            for (size_t sema_idx = 0; sema_idx < CYHAL_IPC_RTOS_SEMA_NUM; sema_idx++)
            {
                uint32_t current_sema_mask = 1 << sema_idx;
                if (interrupt_status_masked & current_sema_mask)
                {
                    if (_cyhal_ipc_rtos_semaphore_base[sema_idx].sema_num != _CYHAL_IPC_UNUSED_SEMAPHORE)
                    {
                        (void)cy_rtos_set_semaphore(&_cyhal_ipc_rtos_semaphore_base[sema_idx].rtos_semaphore, true);
                        _cyhal_ipc_rtos_semaphore_base[sema_idx].sema_num = _CYHAL_IPC_UNUSED_SEMAPHORE;
                    }
                }
            }
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(_cyhal_ipc_rtos_semaphore_base, sizeof(_cyhal_ipc_rtos_semaphore_base) );
        }
    }
    else
#endif /* (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0) */
#if (_CYHAL_IPC_INTR_USER > 0)
        if (isr_channel >= _CYHAL_IPC_INTR_USER)
#endif /* (_CYHAL_IPC_INTR_USER > 0) */
    {
        uint32_t channel = CYHAL_IPC_CHAN_0;
        while (interrupt_status_masked != 0)
        {
            uint32_t channel_mask = 1UL << channel;
            if ((interrupt_status_masked & channel_mask) != 0)
            {
                interrupt_status_masked &= ~channel_mask;

                /* This channel got an interrupt on this core
                 * Check all queues in the channel to see if we have any callbacks to execute
                 */
                uint32_t i;
                for (i = 0; i < _CYHAL_IPC_QUEUES_PER_CHANNEL; i++ )
                {
                    cyhal_ipc_queue_t *queue = _cyhal_ipc_queue_array_get_pointer(channel, i);
                    if (queue != NULL)
                    {
                        cyhal_ipc_event_callback_t  callback = NULL;
                        void                        *callback_arg = NULL;
                        cyhal_ipc_event_t           callback_flags = CYHAL_IPC_NO_INTR;

                        IPC_STRUCT_Type *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(channel));
                        if (CY_RSLT_SUCCESS == Cy_IPC_Drv_LockAcquire(ipc_base) )
                        {
                            INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue, sizeof(cyhal_ipc_queue_t));

                            if ( (queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].cb != NULL) &&
                                 (queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].flags != CYHAL_IPC_NO_INTR) )
                            {
                                callback = (cyhal_ipc_event_callback_t)queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].cb;
                                callback_arg = queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].arg;
                                callback_flags = (cyhal_ipc_event_t)queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].flags;

                                /* clear flags */
                                queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].flags = CYHAL_IPC_NO_INTR;
                                CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(&queue->notifications[_CYHAL_IPC_CUR_CORE_IDX].flags, sizeof(uint32_t) );
                            }
                            (void)Cy_IPC_Drv_LockRelease(ipc_base, CY_IPC_NO_NOTIFICATION);  /* Do not trigger an interrupt */
                        }

                        if (callback != NULL)
                        {
                            callback(callback_arg, callback_flags);
                        }
                    }
                }
            }
            channel++;   /* For multiple channel interrupts on this core */
            if (_CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE(channel) == false)
            {
                break;
            }
        }
    }
}

/********************************* HAL IPC INTERRUPT STUFF ********************************************/

void _cyhal_ipc_register_interrupts_for_this_core(void)
{
    /* Initialize interrupts for channel based on Core */
    if (false == _cyhal_ipc_interrupts_initialized)
    {
        _cyhal_irq_register(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_SRC, CYHAL_ISR_PRIORITY_DEFAULT, _cyhal_ipc_irq_handler);
        _cyhal_irq_enable(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_SRC);                                                /* Enable IRQs for this Core */
#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
        _cyhal_irq_enable((_cyhal_system_irq_t)(_CYHAL_IPC_IPC_INTR_SRC_BASE + _CYHAL_IPC_SEMA_INTR_STR_NUM));  /* Enable IRQs for RTOS Semaphores */
#endif
        _cyhal_ipc_interrupts_initialized = true;
    }
}

/* This is only called from cyhal_ipc_init() upon failure to initialize */
void _cyhal_ipc_unregister_interrupts_for_this_core(void)
{
    /* Un-initialize interrupts for channel based on Core */
    if (true == _cyhal_ipc_interrupts_initialized)
    {
        _cyhal_irq_disable(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_SRC);                                               /* Disable IRQs for this core */
#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
        _cyhal_irq_disable((_cyhal_system_irq_t)(_CYHAL_IPC_IPC_INTR_SRC_BASE + _CYHAL_IPC_SEMA_INTR_STR_NUM)); /* Disable IRQs for RTOS Semaphores */
#endif
        _cyhal_ipc_interrupts_initialized = false;
    }
}

static cy_rslt_t _cyhal_ipc_enable_interrupt(uint32_t channel, uint32_t flags, bool enable)
{
    cy_rslt_t result;
    CY_UNUSED_PARAMETER(flags); // FUTURE: adjust interrupts based on flags

    IPC_STRUCT_Type *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(channel));
    IPC_INTR_STRUCT_Type *ipc_intr_base = Cy_IPC_Drv_GetIntrBaseAddr(_CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN));
    uint32_t channel_intr_mask = (1UL << channel);

    /* Debugging - enable at top of this file */
    _CYHAL_NON_INIT_CORE_PRINTF(("%d %s  chan:%ld flags:0x%lx enable:%d\n", __LINE__, __func__, channel, flags, enable));

    _cyhal_ipc_register_interrupts_for_this_core();

    /* We cannot allow interrupt to happen between _cyhal_ipc_acquire_core_sync_sema and successful
    *   _cyhal_ipc_give_core_sync_sema, as interrupt will also attempt to acquire semaphore (which
    *   will obviously fail) */
    uint32_t intr_status = cyhal_system_critical_section_enter();
    result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, CYHAL_IPC_NEVER_TIMEOUT);
    if (CY_RSLT_SUCCESS == result)
    {
        result = (cy_rslt_t)Cy_IPC_Drv_LockAcquire(ipc_base);
        if (CY_RSLT_SUCCESS == result)
        {
            uint32_t current_ipc_interrupt_mask = Cy_IPC_Drv_GetInterruptMask(ipc_intr_base);
            if (enable)
            {
                Cy_IPC_Drv_ClearInterrupt(ipc_intr_base, channel_intr_mask, 0);
                Cy_IPC_Drv_SetInterruptMask(ipc_intr_base, current_ipc_interrupt_mask | channel_intr_mask, 0);
            }
            else
            {
                Cy_IPC_Drv_SetInterruptMask(ipc_intr_base, current_ipc_interrupt_mask & ~channel_intr_mask, 0);
            }

            /* No reason to generate interrupt if no events were triggered by performed operation */
            (void)Cy_IPC_Drv_LockRelease(ipc_base, CY_IPC_NO_NOTIFICATION);
        }

        cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);
    }
    cyhal_system_critical_section_exit(intr_status);

    return result;
}

void _cyhal_ipc_trigger_interrupts_for_channel(uint32_t channel)
{
    cy_rslt_t isr_result;
    IPC_STRUCT_Type *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(channel));

    _cyhal_ipc_register_interrupts_for_this_core();

    isr_result = (cy_rslt_t)Cy_IPC_Drv_LockAcquire(ipc_base);
    if (CY_RSLT_SUCCESS == isr_result)
    {
        (void)Cy_IPC_Drv_LockRelease(ipc_base, _CYHAL_IPC_TRIGGER_ISR_MASK);
    }

}

static cy_rslt_t _cyhal_ipc_queue_put_get(cyhal_ipc_t *obj, void *msg, uint32_t timeout_us, bool put)
{
    CY_ASSERT(obj != NULL);
    CY_ASSERT(obj->queue_obj != NULL);
    CY_ASSERT(msg != NULL);

    uint32_t i;
    cy_rslt_t   result = CY_RSLT_SUCCESS;
    uint32_t    triggered_events = CYHAL_IPC_NO_INTR;
    uint32_t channel;

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    cyhal_ipc_queue_t *queue = obj->queue_obj;
    result = cyhal_ipc_semaphore_take(&queue->queue_semaphore, timeout_us);
    if ( CY_RSLT_SUCCESS == result )
    {
        INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue, sizeof(cyhal_ipc_queue_t) );
        channel = queue->channel_num;
#if defined(CYHAL_IPC_MULTI_PROCESS_PER_QUEUE)
#error Need to handle multiple processes per core here
#endif
        if (true == put)
        {
            result = CYHAL_IPC_RSLT_ERR_QUEUE_FULL;
            /* Put the message into the queue */
            if (queue->curr_items < queue->num_items)
            {
                uint32_t item_index  = (queue->first_item + queue->curr_items) % queue->num_items;
                uint8_t  *queue_item = (uint8_t *)((uint32_t)queue->queue_pool + (item_index * queue->item_size));
                memcpy((void *)queue_item, msg, queue->item_size);

                queue->curr_items++;

                CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_item, queue->item_size);
                CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue, sizeof(cyhal_ipc_queue_t) );

                triggered_events |= CYHAL_IPC_QUEUE_WRITE;
                for (i = 0; i < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; i++ )
                    queue->notifications[i].flags |= queue->notifications[i].mask & CYHAL_IPC_QUEUE_WRITE;
                result = CY_RSLT_SUCCESS;

                /* Trigger interrupt when we are full, but not every time we call if it is already full */
                if (queue->curr_items == queue->num_items)
                {
                    triggered_events |= CYHAL_IPC_QUEUE_FULL;
                    for (i = 0; i < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; i++ )
                        queue->notifications[i].flags |= queue->notifications[i].mask & CYHAL_IPC_QUEUE_FULL;
                }
            }
        }
        else
        {
            /* Try to GET the message from the queue */
            result = CYHAL_IPC_RSLT_ERR_QUEUE_EMPTY;    /* assume failure */

            /* Get the message into the queue */
            if (queue->curr_items > 0)
            {
                uint8_t *queue_item = (uint8_t *)((uint32_t)queue->queue_pool + (queue->first_item * queue->item_size));
                uint32_t next_item = (queue->first_item + 1) % queue->num_items;
                memcpy(msg, (void *)queue_item, queue->item_size);

                queue->first_item = next_item;
                queue->curr_items--;

                CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(msg, queue->item_size);
                CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue, sizeof(cyhal_ipc_queue_t) );

                triggered_events |= CYHAL_IPC_QUEUE_READ;
                for (i = 0; i < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; i++ )
                    queue->notifications[i].flags |= queue->notifications[i].mask & CYHAL_IPC_QUEUE_READ;
                result = CY_RSLT_SUCCESS;

                /* Trigger interrupt when we empty, but not every time we call if it is already empty */
                if (queue->curr_items == 0)
                {
                    triggered_events |= CYHAL_IPC_QUEUE_EMPTY;
                    for (i = 0; i < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; i++ )
                        queue->notifications[i].flags |= queue->notifications[i].mask & CYHAL_IPC_QUEUE_EMPTY;
                }
            }
        }
        cyhal_ipc_semaphore_give(&queue->queue_semaphore);
    } // semaphore take

    /* We send events even if the put/get fails for FULL or EMPTY */
    if (triggered_events != CYHAL_IPC_NO_INTR)
    {
        _cyhal_ipc_trigger_interrupts_for_channel(channel);
    }

    return result;

}

/****************************************** INIT PUBLIC FUNCTIONS *****************************************************/
/*
 * cyhal_ipc_init(uint32_t timeout_ms)
 *
 * Initialize the HAL IPC Driver
 */
cy_rslt_t cyhal_ipc_init(uint32_t timeout_ms)
{
    cy_rslt_t                   result = CY_RSLT_SUCCESS;
    IPC_STRUCT_Type             *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(_CYHAL_IPC_INTERNAL_USE_CHAN));

    (void)timeout_ms;   /* satisfy IAR/ARM compilers, as CYHAL_IPC_INIT_CORE does not use the argument */

    /* INIT_CORE:
     *     Set _CYHAL_IPC_INTERNAL_USE_CHAN data to 0x00
     *     This lets other cores we are initializing, please wait.
     * OTHER CORE(s) will use the value to wait until INIT_CORE is done with initialization
     */
    if (false == _cyhal_ipc_local_initialized)
    {
#if defined(CYHAL_IPC_INIT_CORE)
        if(CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire (ipc_base))
        {
            /* Write 0x00 to start */
            Cy_IPC_Drv_WriteDataValue(ipc_base, 0);

            /* Release, but do not trigger a release event */
            (void) Cy_IPC_Drv_LockRelease (ipc_base, CY_IPC_NO_NOTIFICATION);
        }
#endif
    }

    /* Initialize Semaphores if it was not done before.
     * CAT1A devices, the startup code inits the PDL IPC Semaphore list (see system_psoc6_cm0plus.c)
     * On CAT1C and CAT1D devices, startup code does not initialize IPC PDL semaphore and
     *  does not allocate shared memory for them.
     */
    if (false == _cyhal_ipc_PDL_semas_initialized)
    {
#if defined(CYHAL_IPC_INIT_CORE)
        /* This code is executing on the INIT (Boot) Core */
#if defined(COMPONENT_CAT1C) || defined(COMPONENT_CAT1D) && !defined(CY_IPC_DEFAULT_CFG_DISABLE)
        /* We need to initialize the PDL semaphores. This is the array passed to the PDL
         * only 1 instance in the whole App, on the Boot Core */
        CY_SECTION_SHAREDMEM static uint32_t ipcSemaArray[ CYHAL_IPC_SEMA_COUNT / CY_IPC_SEMA_PER_WORD ] _CYHAL_IPC_DATA_ALIGN;
        result = (cy_rslt_t)Cy_IPC_Sema_Init(_CYHAL_IPC_PDL_SEMA_CHAN, CYHAL_IPC_SEMA_COUNT, ipcSemaArray);
#endif
#else // INIT Core

        /* This code is executing on a NON-INIT (Boot) Core */
        result = (cy_rslt_t)Cy_IPC_Sema_Init(_CYHAL_IPC_PDL_SEMA_CHAN, 0, NULL);
#endif  /* Boot vs. non-Boot core */
        if (CY_RSLT_SUCCESS == result)
        {
            _cyhal_ipc_PDL_semas_initialized = true;
        }
    }

    /* PDL-level Semaphores initialized, now we define and initialize the HAL-level IPC variables */
    if ( (CY_RSLT_SUCCESS == result) && (false == _cyhal_ipc_local_initialized) )
    {
#if defined(CYHAL_IPC_INIT_CORE)
        /* These variables are the only instance allowed. Pointers to the variables are passed to the other Cores
         * during initialization so that all Cores reference the same memory addresses.
         */
        /** IPC Driver Semaphore for changing any HAL IPC Driver variables */
        CY_SECTION_SHAREDMEM static cyhal_ipc_t _cyhal_ipc_semaphore_base _CYHAL_IPC_DATA_ALIGN;
        memset(&_cyhal_ipc_semaphore_base, 0x00, sizeof(cyhal_ipc_t) );

        /** HAL IPC Semaphore bit field usage for Semaphore indexes in use */
        CY_SECTION_SHAREDMEM static uint32_t _cyhal_ipc_sema_usage_bits_base[ _CYHAL_SEMA_USAGE_BITS_ARRAY_SIZE ] _CYHAL_IPC_DATA_ALIGN;
        memset(_cyhal_ipc_sema_usage_bits_base, 0x00, sizeof(_cyhal_ipc_sema_usage_bits_base) );

        /** HAL IPC Initialized semaphore singly-linked list */
        CY_SECTION_SHAREDMEM static cyhal_ipc_t         *_cyhal_ipc_sema_linked_list_base _CYHAL_IPC_DATA_ALIGN;
        _cyhal_ipc_sema_linked_list_base = NULL;

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
        memset(_cyhal_ipc_rtos_semaphore_base, 0x00, sizeof(_cyhal_ipc_rtos_semaphore_base));
        _cyhal_ipc_initialize_rtos_semaphores();
#endif // (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)

        /** List of initialized HAL IPC Queue pointers -- protect with _cyhal_ipc_shared_vars->ipc_driver_semaphore
         * Arranged by Channel to efficiently scan through list when ISR occurs on a channel.
         * When accessing this array, adjust channel by _CYHAL_IPC_FIX_CHAN_NUM(channel) to account for different device channel support.
         * Some devices have first User Channel > 0. */
        CY_SECTION_SHAREDMEM static cyhal_ipc_queue_t *_cyhal_ipc_queue_array_base[ CYHAL_IPC_USR_CHANNELS * _CYHAL_IPC_QUEUES_PER_CHANNEL ] _CYHAL_IPC_DATA_ALIGN;
        memset(_cyhal_ipc_queue_array_base, 0x00, SIZE_OF_QUEUE_ARRAY );

        /* Internal initialize and send info to other Cores */
        _cyhal_ipc_shared_vars->ipc_driver_semaphore     = &_cyhal_ipc_semaphore_base;
        _cyhal_ipc_shared_vars->ipc_semaphore_usage_bits = _cyhal_ipc_sema_usage_bits_base;
        _cyhal_ipc_shared_vars->ipc_semaphore_list       = _cyhal_ipc_sema_linked_list_base;
        _cyhal_ipc_shared_vars->ipc_queue_array          = (cyhal_ipc_queue_t **)&_cyhal_ipc_queue_array_base;

        /* Init the HAL IPC Driver Semaphore */
        if (CY_RSLT_SUCCESS == result)
        {
            result = _cyhal_ipc_sema_inner_init(_cyhal_ipc_shared_vars->ipc_driver_semaphore, _CYHAL_IPC_DRIVER_SEMA_NUM, false);
            if (CY_RSLT_SUCCESS != result)
            {
                result = CYHAL_IPC_RSLT_ERR_NO_SEMA_AVAILABLE;
            }
        }

        if (CY_RSLT_SUCCESS == result)
        {
            if(CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire(ipc_base))
            {
                /* Set the actual shared variables value */
                Cy_IPC_Drv_WriteDataValue(ipc_base, (uint32_t)_cyhal_ipc_shared_vars);

                /* Release, but do not trigger a release event */
                (void) Cy_IPC_Drv_LockRelease(ipc_base, CY_IPC_NO_NOTIFICATION);
            }

        }
#else // defined(CYHAL_IPC_INIT_CORE)
        /* Other Core(s) wait for the INIT_CORE to set the Cy_IPC_Drv_ReadDataValue(ipc_base) to be non-0x00 */
        uint32_t init_state = 0;
        do
        {
            if(CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire(ipc_base))
            {
                /* read the _cyhal_ipc_shared_vars value from the IPC */
                init_state = Cy_IPC_Drv_ReadDataValue(ipc_base);

                /* Release, but do not trigger a release event */
                (void) Cy_IPC_Drv_LockRelease(ipc_base, CY_IPC_NO_NOTIFICATION);
            }
            cyhal_system_delay_us(1000);

            if (timeout_ms != CYHAL_IPC_NEVER_TIMEOUT)
            {
                timeout_ms--;
            }

        } while ( (init_state == 0) && (timeout_ms > 0) );

        if (timeout_ms == 0)
        {
            result = CYHAL_IPC_RSLT_ERR_TIMEOUT;
        }

        if (CY_RSLT_SUCCESS == result)
        {
            /* Getting shared memory memory pointer to the first element of queues linked list */
            _cyhal_ipc_shared_vars = (_cyhal_ipc_shared_t *)Cy_IPC_Drv_ReadDataValue(ipc_base);

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
            /* Each Core must have it's own RTOS semaphores */
            memset(_cyhal_ipc_rtos_semaphore_base, 0x00, sizeof(_cyhal_ipc_rtos_semaphore_base));
            _cyhal_ipc_initialize_rtos_semaphores();
#endif // (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
        }
#endif // defined(CYHAL_IPC_INIT_CORE)

        if (CY_RSLT_SUCCESS == result)
        {
#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
            /* We do this once per core at startup so there is always a callback registered */
            _cyhal_ipc_register_irq_callback();
#endif  // (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)

            /* initialize interrupts for this Core */
            _cyhal_ipc_register_interrupts_for_this_core();

            /* Set local flag indicating we have initialized the HAL IPC Driver Shared Memory
             * This flag is checked in queue_init() to see if the Shared Memory needs to be initialized.
             */
            _cyhal_ipc_local_initialized = true;
        }
    }

    if (CY_RSLT_SUCCESS != result)
    {
        /* Un-init things as we failed */
        _cyhal_ipc_local_initialized = false;
        _cyhal_ipc_shared_vars = NULL;
        _cyhal_ipc_unregister_interrupts_for_this_core();
    }

    return result;
}

/**************************************** SEMAPHORE PUBLIC FUNCTIONS **************************************************/

/*
 * In the PDL IPC, all the semaphores allocated have been 'initialized' at startup in the "Boot Core" file (or equivalent):
 *  CY8CPROTO-062-4343W system_psoc6_cm0plus.c SystemInit()  Cy_IPC_Sema_Init().
In HAL IPC, we only call Cy_IPC_Sema_Set() or Cy_IPC_Sema_Clear() for the Semaphores.
 * Here we can only check that HAL-level doesn't have a record of it being allocated, and that it isn't currently Set.
 *
 * To find a free HAL-level semaphore number, call _cyhal_ipc_sema_usage_find_free(uint32_t *sema_number)
 */
cy_rslt_t cyhal_ipc_semaphore_init(cyhal_ipc_t *obj, uint32_t semaphore_num, bool preemptable)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Since Shared Memory is NOT cleared at power-up/system reset we
     * need to determine if the HAL IPC Driver has been initialized.
     *
     * - Check if local variable is set, indicating we have done our part of initialization
     *   on this CPU Core.
     */
    if (_cyhal_ipc_local_initialized == false)
    {
        cyhal_ipc_init(CYHAL_IPC_NEVER_TIMEOUT);
    }

    /* Last semaphore is used for internal IPC queues functionality */
    if ( (obj == NULL) || (semaphore_num > _CYHAL_LAST_USER_SEMA_NUM) )
    {
        /* Semaphore index exceeds the number of allowed Semaphores */
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    /* Init the object passed in */
    memset(obj, 0x00, sizeof(cyhal_ipc_t) );

    /* See if Semaphore is already inited */
    if (_cyhal_ipc_sema_usage_bit_get_status(semaphore_num) != 0)
    {
        return CYHAL_IPC_RSLT_ERR_SEMA_NUM_IN_USE;
    }

    /* Grab the HAL PDL Driver Semaphore */
    result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
    if (CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_ipc_sema_inner_init(obj, semaphore_num, preemptable);

        cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);
    }

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);

    return result;
}

void cyhal_ipc_semaphore_free(cyhal_ipc_t *obj)
{
    cy_rslt_t result;

    if (false == _cyhal_ipc_local_initialized)
    {
        return; // CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    if (obj == NULL)
    {
        return; // CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *  return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);

    /* Grab the HAL IPC Driver Semaphore when changing HAL IPC list of semaphores */
    result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
    if (CY_RSLT_SUCCESS == result)
    {
        /* Free our usage bit */
        if (obj->sema_number != _CYHAL_IPC_UNUSED_SEMAPHORE)
        {
            _cyhal_ipc_sema_usage_bit_mark_free(obj->sema_number);
        }

        /* remove from the linked list */
        cyhal_ipc_t *last = NULL;
        cyhal_ipc_t *walker = _cyhal_ipc_shared_vars->ipc_semaphore_list;
        while(walker != NULL)
        {
            if (walker->sema_number == obj->sema_number)
            {
                if (walker == _cyhal_ipc_shared_vars->ipc_semaphore_list)
                {
                    /* obj is the first semaphore in the list
                     * point list to the next one.
                     */
                    _cyhal_ipc_shared_vars->ipc_semaphore_list = _cyhal_ipc_shared_vars->ipc_semaphore_list->next_sema;
                }
                else
                {
                    last->next_sema = walker->next_sema;
                }
                obj->next_sema = NULL;
                walker = NULL;
            }
            else
            {
                last = walker;
                walker = walker->next_sema;
            }
        }

        /* We are done with the HAL IPC Driver Semaphore */
        cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);
    }

    /* outside of gate for ipc_driver_semaphore
     * If the semaphore is taken, give it first.
     */
    if (obj->sema_taken > 0)
    {
        (void)cyhal_ipc_semaphore_give(obj);
    }

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);

    return; // CY_RSLT_SUCCESS;
}

cy_rslt_t cyhal_ipc_semaphore_get_handle(cyhal_ipc_t **obj, uint32_t semaphore_num, uint32_t timeout_us)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    if (obj == NULL)
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     */
    _CYHAL_CHECK_TIMEOUT_AND_RETURN_IF_IN_ISR(timeout_us);

    result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
    if (CY_RSLT_SUCCESS == result)
    {
        /* Walk the linked list looking for the semaphore number */
        cyhal_ipc_t *walker = _cyhal_ipc_shared_vars->ipc_semaphore_list;
        while(walker != NULL)
        {
            if (walker->sema_number == semaphore_num)
            {
                result = CY_RSLT_SUCCESS;
                *obj = walker;
                break;
            }
            walker = walker->next_sema;
        }
        cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);
    }

    return result;
}

cy_rslt_t cyhal_ipc_semaphore_take(cyhal_ipc_t *obj, uint32_t timeout_us)
{
    cy_rslt_t               result = CYHAL_IPC_RSLT_ERR_SEMA_FAIL;
    cy_en_ipcsema_status_t  pdl_ipc_sema_result = CY_IPC_SEMA_LOCKED;

    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);

    /* Check if we are executing during an ISR, timeout must be 0x00 */
    _CYHAL_CHECK_TIMEOUT_AND_RETURN_IF_IN_ISR(timeout_us);

    if ( (obj == NULL) || (obj->sema_number > _CYHAL_LAST_USER_SEMA_NUM) )
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    /* Debugging - enable at top of this file */
    _CYHAL_NON_INIT_CORE_PRINTF(("%ld %s() start, caller:%p, obj->sema_taken(%lu), timeout_us(0x%lx)\n",
                                __LINE__, __func__, __builtin_return_address(0), obj->sema_taken, timeout_us));

    /* Try to take the PDL semaphore once
     * If we get it, then we don't need to try the RTOS semaphore.
     */

    pdl_ipc_sema_result = _cyhal_ipc_sema_PDL_set(obj);
    if (pdl_ipc_sema_result == CY_IPC_SEMA_SUCCESS)
    {
         result = CY_RSLT_SUCCESS;
        /* debugging */
        _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);
        return result;
    }

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
    IPC_STRUCT_Type *ipc_base = Cy_IPC_Drv_GetIpcBaseAddress(_CYHAL_IPC_PDL_CHAN_ACCESS_FIX_CHAN(_CYHAL_IPC_INTERNAL_USE_CHAN));
    IPC_INTR_STRUCT_Type *ipc_intr_base = Cy_IPC_Drv_GetIntrBaseAddr(_CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(_CYHAL_IPC_SEMA_INTR_STR_NUM));
    uint32_t intr_status_masked = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASKED_RELEASE, Cy_IPC_Drv_GetInterruptStatusMasked(ipc_intr_base));
    uint32_t current_sema_intr_mask = 0;
    cy_rslt_t rtos_sema_result = CY_RSLT_SUCCESS;
    int32_t rtos_sema_idx = -1;     /* -1 means we did not find an rtos_semaphore */
    uint32_t interruptState;

    /* Only try using rtos sema if we have a chance of getting the PDL semaphore.
     * Since the minimum timeout for rtos semaphore is 1ms, don't use rtos semaphore unless the timeout is > 1000 uSec
     * And don't use if we are in an ISR.
     */
    if ( (timeout_us >= 1000) && ( (pdl_ipc_sema_result == CY_IPC_SEMA_LOCKED) || (pdl_ipc_sema_result == CY_IPC_SEMA_NOT_ACQUIRED) ) )
    {
        /* Try to use an RTOS semaphore to allow other threads to run */
        interruptState = cyhal_system_critical_section_enter();
        if(CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire(ipc_base))  // Internal use channel
        {
            INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(_cyhal_ipc_rtos_semaphore_base, sizeof(_cyhal_ipc_rtos_semaphore_base) );

            /* find an unused rtos semaphore */
            uint32_t index;
            for (index = 0; index < CYHAL_IPC_RTOS_SEMA_NUM; ++index)
            {
                if (_cyhal_ipc_rtos_semaphore_base[index].sema_num == _CYHAL_IPC_UNUSED_SEMAPHORE)
                {
                    _cyhal_ipc_rtos_semaphore_base[index].sema_num = obj->sema_number;
                    rtos_sema_idx = index;
                    current_sema_intr_mask = 1 << rtos_sema_idx;

                    CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(&_cyhal_ipc_rtos_semaphore_base[index], sizeof(_cyhal_ipc_rtos_sema_t) );
                    break;
                }
            }
            (void) Cy_IPC_Drv_LockRelease (ipc_base, CY_IPC_NO_NOTIFICATION);
        }
        cyhal_system_critical_section_exit(interruptState);

    }
#endif // (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)

    uint32_t timeout_us_left = timeout_us;
    bool     never_timeout = (timeout_us == CYHAL_IPC_NEVER_TIMEOUT) ? true : false;
    do
    {
#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
        if (rtos_sema_idx >= 0)
        {
            /* Enable all possible interrupt bits for rtos sema interrupt.
             * We set all of the possible bits so that we don't accidentally clear other needed bits.
             */
            Cy_IPC_Drv_SetInterruptMask(ipc_intr_base, (1 << CYHAL_IPC_RTOS_SEMA_NUM) - 1, 0);

            /* cyhal_system_critical_section_enter(); Critial section here stops the rtos semaphore interrupt from working! */

            /* Use minimal rtos call timeout of 1 ms, loop for longer times */
            rtos_sema_result = cy_rtos_get_semaphore( &_cyhal_ipc_rtos_semaphore_base[rtos_sema_idx].rtos_semaphore, 1, false);
        }
#else
        cyhal_system_delay_us(CYHAL_IPC_POLLING_INTERVAL_uS); /* always step the same amount for non-rtos usage */
#endif
        pdl_ipc_sema_result = _cyhal_ipc_sema_PDL_set(obj);
        switch (pdl_ipc_sema_result)
        {
        case CY_IPC_SEMA_SUCCESS:
            result = CY_RSLT_SUCCESS;
            break;

        case CY_IPC_SEMA_LOCKED:        /* IPC channel busy or locked, wait a bit and try again */
            result = CYHAL_IPC_RSLT_ERR_SEMA_FAIL;
            break;

        case CY_IPC_SEMA_NOT_ACQUIRED:      /* PDL IPC channel not initialized (we init well before getting here)
                                            * -or- Semaphore is already taken */
            result = CYHAL_IPC_RSLT_ERR_SEMA_TAKEN;
            break;

        case CY_IPC_SEMA_OUT_OF_RANGE:
        default:
            result = CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
            break;
        }

        if (never_timeout == false)
        {
            uint32_t step_us = CYHAL_IPC_POLLING_INTERVAL_uS;
#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
            step_us = _CYHAL_UTILS_US_PER_MS;
#endif
            if (timeout_us_left  >= step_us)
            {
                timeout_us_left -= step_us;
            }
            else
            {
                timeout_us_left = 0;
            }
        }

    } while ( (timeout_us_left > 0) && ( (result != CY_RSLT_SUCCESS) && (result != CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER) ) );

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
    if ( (rtos_sema_idx >= 0) && (pdl_ipc_sema_result == CY_IPC_SEMA_SUCCESS) )
    {
        /* cyhal_system_critical_section_enter(); Not needed as this is a set, not a test & set */

        /* We don't need the rtos semaphore, un-associate it with the semaphore */
        _cyhal_ipc_rtos_semaphore_base[rtos_sema_idx].sema_num = _CYHAL_IPC_UNUSED_SEMAPHORE;
        CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(&_cyhal_ipc_rtos_semaphore_base[rtos_sema_idx], sizeof(_cyhal_ipc_rtos_sema_t) );
        rtos_sema_idx = -1;
    }

    /* update the interrupt status masked, clear if used */
    intr_status_masked = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASKED_RELEASE, Cy_IPC_Drv_GetInterruptStatusMasked(ipc_intr_base));
    if ((pdl_ipc_sema_result == CY_IPC_SEMA_SUCCESS) && (intr_status_masked & current_sema_intr_mask))
    {
        /* If semaphore get was successful and interrupt was not cleared by IRQ handler (e.g. interrupts are disabled),
        *   clear pending interrupt, that is related to this semaphore number */
        Cy_IPC_Drv_ClearInterrupt(ipc_intr_base, current_sema_intr_mask, 0);
    }
    /* If IPC semaphore was not successfully got and unexpected result was returned by cy_rtos_get_semaphore,
    *  forward the RTOS result to the user. */
    if ((CY_RSLT_SUCCESS != result) && ((CY_RSLT_SUCCESS != rtos_sema_result) && (CY_RTOS_TIMEOUT != rtos_sema_result)))
    {
        result = rtos_sema_result;
    }
#endif /* (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0) */

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);
    _CYHAL_NON_INIT_CORE_PRINTF(("%ld %s() end, caller:%p, obj->sema_taken(%lu)\n",
                                __LINE__, __func__, __builtin_return_address(0), obj->sema_taken));
    return result;
}

cy_rslt_t cyhal_ipc_semaphore_give(cyhal_ipc_t *obj)
{
    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    cy_rslt_t result = CYHAL_IPC_RSLT_ERR_SEMA_FAIL;
    cy_en_ipcsema_status_t pdl_ipc_sema_result = CY_IPC_SEMA_LOCKED;

    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);

    if ( (obj == NULL) || (obj->sema_number > _CYHAL_LAST_USER_SEMA_NUM) )
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    /* Check if we are executing during an ISR, timeout must be 0x00 */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    do
    {
        pdl_ipc_sema_result = _cyhal_ipc_sema_PDL_clear(obj);

        switch (pdl_ipc_sema_result)
        {
        case CY_IPC_SEMA_SUCCESS:
            result = CY_RSLT_SUCCESS;
            break;

        case CY_IPC_SEMA_LOCKED: /* IPC channel busy or locked, wait a bit and try again */
            result = CYHAL_IPC_RSLT_ERR_SEMA_FAIL;
            break;

        case CY_IPC_SEMA_NOT_ACQUIRED:
            result = CY_RSLT_SUCCESS;   /* We allow for semaphore give multiple times, like in an RTOS */
            break;

        case CY_IPC_SEMA_OUT_OF_RANGE:
            result = CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
            break;

        /* Other codes not returned by PDL IPC Semaphore Clear */
        default:
            result = CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
            break;
        }

        if (timeout_us > CYHAL_IPC_POLLING_INTERVAL_uS)
        {
            cyhal_system_delay_us(CYHAL_IPC_POLLING_INTERVAL_uS);
            timeout_us -= CYHAL_IPC_POLLING_INTERVAL_uS;
        }
        else
        {
            timeout_us  = 0;
        }
    } while  ( (result != CY_RSLT_SUCCESS) && (timeout_us > 0) );

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0)
    if (CY_IPC_SEMA_SUCCESS == pdl_ipc_sema_result)
    {
        /* we actually cleared the PDL semaphore, trigger IRQ */
        for (size_t sema_idx = 0; sema_idx < CYHAL_IPC_RTOS_SEMA_NUM; sema_idx++)
        {
            if (_cyhal_ipc_rtos_semaphore_base[sema_idx].sema_num == obj->sema_number)
            {
                IPC_INTR_STRUCT_Type *ipc_intr_base = Cy_IPC_Drv_GetIntrBaseAddr(_CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(_CYHAL_IPC_SEMA_INTR_STR_NUM));
                Cy_IPC_Drv_SetInterrupt(ipc_intr_base, 1 << sema_idx, 0);
                break;
            }
        }

    }
#endif /* (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && (CYHAL_IPC_RTOS_SEMA_NUM > 0) */

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);
    _CYHAL_NON_INIT_CORE_PRINTF(("%ld %s() end, caller:%p, obj->sema_taken(%lu)\n",
                                __LINE__, __func__, __builtin_return_address(0), obj->sema_taken));
    return result;
}

/***************************************** QUEUES PUBLIC FUNCTIONS ****************************************************/

cy_rslt_t cyhal_ipc_queue_init(cyhal_ipc_t *obj, cyhal_ipc_queue_t *queue_handle)
{
    uint32_t semaphore_number;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Validate args */
    if ( (obj == NULL) || (queue_handle == NULL) )
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *  return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    /* Since Shared Memory is NOT cleared at power-up/system reset we
     * need to determine if the HAL IPC Driver has been initialized.
     *
     * - Check if local variable is set, indicating we have done our part of initialization
     *   on this CPU Core.
     */
    if (_cyhal_ipc_local_initialized == false)
    {
        cyhal_ipc_init(CYHAL_IPC_NEVER_TIMEOUT);
    }

    /*
     * Validate user-filled fields in queue structure
     * * obj is in Shared Memory
     * * queue_handle is in Shared Memory
     * * queue_pool is in Shared Memory
     * * 0 <= channel_num < CYHAL_IPC_USR_CHANNELS
     * * 0 <= queue_num < _CYHAL_IPC_QUEUES_PER_CHANNEL
     * * _cyhal_ipc_shared_vars->ipc_queue_array[ _CYHAL_IPC_FIX_CHAN_NUM(channel_num) ][ queue_num ] == NULL
     * _cyhal_ipc_shared_vars->ipc_queue_array[channel_num][queue_num]
     *
     */
    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

    if ( (obj == NULL) || (queue_handle == NULL) || (queue_handle->queue_pool == NULL) ||
         (_CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE(queue_handle->channel_num) == false) ||
         (queue_handle->queue_num >= _CYHAL_IPC_QUEUES_PER_CHANNEL) ||
        (queue_handle->num_items == 0) ||
        (queue_handle->item_size == 0) )
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    /* Check if Queue index has already been used */
    cyhal_ipc_queue_t *queue = _cyhal_ipc_queue_array_get_pointer(queue_handle->channel_num, queue_handle->queue_num);
    if (queue != NULL)
    {
        return CYHAL_IPC_RSLT_ERR_QUEUE_NUM_IN_USE;
    }

    /* Initialize Caller's Object to store queue ptr
     * The caller always uses the obj ptr from now on.
     * They must not alter the data in queue_handle either!
     */
    memset (obj, 0x00, sizeof(cyhal_ipc_t) );

    /* initialize the Queue's Semaphore */
    result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
    if (CY_RSLT_SUCCESS == result)
    {
        result = _cyhal_ipc_sema_usage_find_free(&semaphore_number);
        if (CY_RSLT_SUCCESS == result)
        {
            memset(&queue_handle->queue_semaphore, 0x00, sizeof(cyhal_ipc_t) );
            result = _cyhal_ipc_sema_inner_init(&queue_handle->queue_semaphore, semaphore_number, false); // we already have the driver semaphore
            if (CY_RSLT_SUCCESS == result)
            {
                /* Initialize other Queue info */
                queue_handle->curr_items = 0;
                queue_handle->first_item = 0;
                memset(&queue_handle->notifications, 0x00, sizeof(queue_handle->notifications));
                memset(queue_handle->queue_pool, 0x00, (queue_handle->num_items * queue_handle->item_size) );
                queue_handle->queue_semaphore.queue_obj = queue_handle;

                obj->queue_obj = (void *)queue_handle;
                obj->sema_number = semaphore_number;

                _cyhal_ipc_queue_array_set_pointer(queue_handle->channel_num, queue_handle->queue_num, queue_handle);
            }
        }
        cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);
    }

    CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

    /* debugging */
    _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);

    return result;
}

void cyhal_ipc_queue_free(cyhal_ipc_t *obj)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t old_events_to_remove = CYHAL_IPC_NO_INTR;

    if (false == _cyhal_ipc_local_initialized)
    {
        return; // CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* Validate args */
    if (obj == NULL)
    {
        return; // CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *   CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    if ( (obj == NULL) || (obj->queue_obj == NULL) )
    {
        return; // CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    cyhal_ipc_queue_t *queue_handle = (cyhal_ipc_queue_t *)(obj->queue_obj);

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

    uint32_t channel_num = queue_handle->channel_num;
    uint32_t queue_num = queue_handle->queue_num;
    if (_cyhal_ipc_queue_array_get_pointer(channel_num, queue_num) != queue_handle)
    {
        return; // CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
    if (CY_RSLT_SUCCESS == result)
    {
        _cyhal_ipc_queue_array_set_pointer(channel_num, queue_num, NULL);
        cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);

        result = cyhal_ipc_semaphore_take(&queue_handle->queue_semaphore, timeout_us);
        if (CY_RSLT_SUCCESS == result)
        {
            /* release any notifications in the queue */
            uint32_t i;
            for (i = 0; i < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; i++)
            {
                if ( (queue_handle->notifications[i].sema_valid == true) &&
                     (cyhal_ipc_semaphore_take(&queue_handle->notifications[i].notification_semaphore, timeout_us) == CY_RSLT_SUCCESS) )
                {
                    old_events_to_remove |= queue_handle->notifications[i].mask;

                    queue_handle->notifications[i].arg = NULL;
                    queue_handle->notifications[i].cb = NULL;
                    queue_handle->notifications[i].mask = CYHAL_IPC_NO_INTR;
                    queue_handle->notifications[i].flags = CYHAL_IPC_NO_INTR;

                    cyhal_ipc_semaphore_free(&queue_handle->notifications[i].notification_semaphore);
                    queue_handle->notifications[i].sema_valid = false;
                }
            }
            cyhal_ipc_semaphore_free(&queue_handle->queue_semaphore);
        }
    }

    /* Clear any interrupts if they are not longer needed */
    if (old_events_to_remove != CYHAL_IPC_NO_INTR)
    {
/*        Interrupt may be needed by another queue, let's leave it enabled.
 *        _cyhal_ipc_enable_interrupt(channel, old_events_to_remove, false);
 */
    }

    CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );
    CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(obj, sizeof(cyhal_ipc_t) );

    return; // result;
}

cy_rslt_t cyhal_ipc_queue_get_handle(cyhal_ipc_t *obj, uint32_t channel_num, uint32_t queue_num)
{
    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* Validate args */
    if (obj == NULL)
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    /* Queue IPC channel number check */
    if ( (_CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE(channel_num) == false) || (queue_num >= _CYHAL_IPC_QUEUES_PER_CHANNEL))
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    cyhal_ipc_queue_t *queue = _cyhal_ipc_queue_array_get_pointer(channel_num, queue_num);
    if ( (obj != NULL) && (queue != NULL) )
    {
        memset (obj, 0x00, sizeof(cyhal_ipc_t) );
        obj->queue_obj = queue;
        CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(obj, sizeof(cyhal_ipc_t) );
        return CY_RSLT_SUCCESS;
    }

    return CYHAL_IPC_RSLT_ERR_QUEUE_NOT_FOUND;
}

void cyhal_ipc_queue_register_callback(cyhal_ipc_t *obj, cyhal_ipc_event_callback_t callback, void *callback_arg)
{
    if (false == _cyhal_ipc_local_initialized)
    {
        return; // CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* Validate args */
    if (obj == NULL)
    {
        return; // CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    cy_rslt_t   result = CY_RSLT_SUCCESS;
    cyhal_ipc_queue_t *queue_handle = (cyhal_ipc_queue_t *)(obj->queue_obj);

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *  return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    uint32_t process_number = _CYHAL_IPC_CUR_CORE_IDX;

#if defined(CYHAL_IPC_MULTI_PROCESS_PER_QUEUE)
    #error "Code needed for Multiple Processes per Queue"
#endif

    if (queue_handle->notifications[process_number].sema_valid == false)
    {
        /* We need to create the semaphore for this process! */
        result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
        if (CY_RSLT_SUCCESS == result)
        {
            uint32_t semaphore_number;

            result = _cyhal_ipc_sema_usage_find_free(&semaphore_number);
            if (CY_RSLT_SUCCESS == result)
            {
                result = _cyhal_ipc_sema_inner_init(&queue_handle->notifications[process_number].notification_semaphore, semaphore_number, false);
                if (CY_RSLT_SUCCESS == result)
                {
                    queue_handle->notifications[process_number].sema_valid = true;
                }
            }
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

            cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);
        }

        /* debugging */
        _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);
    }

    if (CY_RSLT_SUCCESS == result)
    {
        result = cyhal_ipc_semaphore_take(&queue_handle->notifications[process_number].notification_semaphore, timeout_us);
        if (CY_RSLT_SUCCESS == result)
        {
            queue_handle->notifications[process_number].cb  = (void *)callback;
            queue_handle->notifications[process_number].arg = callback_arg;

            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

            cyhal_ipc_semaphore_give(&queue_handle->notifications[process_number].notification_semaphore);
        }
    }

    return; // result;
}

void cyhal_ipc_queue_enable_event(cyhal_ipc_t *obj, cyhal_ipc_event_t event, uint8_t intr_priority, bool enable)
{
    if (false == _cyhal_ipc_local_initialized)
    {
        return; // CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* Validate args */
    if (obj == NULL)
    {
        return; // CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }
    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    cy_rslt_t   result = CY_RSLT_SUCCESS;
    cyhal_ipc_queue_t *queue_handle = (cyhal_ipc_queue_t *)(obj->queue_obj);

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

    uint32_t timeout_us = CYHAL_IPC_NEVER_TIMEOUT;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *  return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    uint32_t channel_num = queue_handle->channel_num;
    uint32_t process_number = _CYHAL_IPC_CUR_CORE_IDX;
#if defined(CYHAL_IPC_MULTI_PROCESS_PER_QUEUE)
    #error "Code needed for Multiple Processes per Queue"
#endif


    if (queue_handle->notifications[process_number].sema_valid == false)
    {
        /* We need to create the semaphore for this process! */
        result = cyhal_ipc_semaphore_take(_cyhal_ipc_shared_vars->ipc_driver_semaphore, timeout_us);
        if (CY_RSLT_SUCCESS == result)
        {
            uint32_t semaphore_number;

            result = _cyhal_ipc_sema_usage_find_free(&semaphore_number);
            if (CY_RSLT_SUCCESS == result)
            {
                result = _cyhal_ipc_sema_inner_init(&queue_handle->notifications[process_number].notification_semaphore, semaphore_number, false);
                if (CY_RSLT_SUCCESS == result)
                {
                    queue_handle->notifications[process_number].sema_valid = true;
                }
            }
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

            cyhal_ipc_semaphore_give(_cyhal_ipc_shared_vars->ipc_driver_semaphore);

            /* debugging */
            _cyhal_ipc_semaphore_validate_HAL_with_PDL(__LINE__, __func__);
        }
    }

    if (CY_RSLT_SUCCESS == result)
    {
        result = cyhal_ipc_semaphore_take(&queue_handle->notifications[process_number].notification_semaphore, timeout_us);
        if (CY_RSLT_SUCCESS == result)
        {
            /* get channel for setting / clearing ISR below */
            if (enable == true)
            {
                queue_handle->notifications[process_number].mask |= event;
            }
            else
            {
                queue_handle->notifications[process_number].mask &= ~event;
            }
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

            cyhal_ipc_semaphore_give(&queue_handle->notifications[process_number].notification_semaphore);
        }
    }

    /* Go through the events for this channel and see if we have events to trigger the interrupt */
    uint32_t events_enabled = CYHAL_IPC_NO_INTR;
    uint32_t queue_idx, process_idx;

    for (queue_idx = 0 ; queue_idx < _CYHAL_IPC_QUEUES_PER_CHANNEL; queue_idx++)
    {
#if defined(CYHAL_IPC_MULTI_PROCESS_PER_QUEUE)
    #error "Validate this code for Multiple Processes per Queue"
#endif
        for (process_idx = 0; process_idx < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; process_idx++)
        {
            cyhal_ipc_queue_t *queue = _cyhal_ipc_queue_array_get_pointer(channel_num, queue_idx);
            if (queue != NULL)
            {
                INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue, sizeof(cyhal_ipc_queue_t) );
                events_enabled |= queue->notifications[process_idx].mask;
            }
        }
    }

    /* All channel-related queues have no events enabled, disabling interrupts */
    if ( (queue_handle->notifications[process_number].mask == CYHAL_IPC_NO_INTR) &&
         (events_enabled == CYHAL_IPC_NO_INTR) ) {
        _cyhal_ipc_enable_interrupt(channel_num, CYHAL_IPC_ALL_QUEUE_EVENTS, false);
    }
    else if ( (CY_RSLT_SUCCESS == result) && (_CYHAL_IPC_IS_CHAN_IDX_IN_USER_RANGE(channel_num) == true) && (events_enabled != CYHAL_IPC_NO_INTR) )
    {
        /* get interrupt base address */
        IPC_INTR_STRUCT_Type *ipc_intr_base = Cy_IPC_Drv_GetIntrBaseAddr(_CYHAL_IPC_PDL_INTR_ACCESS_FIX_CHAN(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_CHAN));

        /* As one IPC INTR structure service all IPC channels on certain core, we can't change interrupt priority
        *  for all channels as requested, we can only make it higher. */
        if (intr_priority < _cyhal_irq_get_priority(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_SRC))
        {
            _cyhal_irq_set_priority(_CYHAL_IPC_CURRENT_CORE_IPC_INTR_SRC, intr_priority);
        }

        uint32_t current_ipc_interrupt_mask = Cy_IPC_Drv_GetInterruptMask(ipc_intr_base);
        uint32_t channel_intr_mask = (1UL << channel_num);
        if ((current_ipc_interrupt_mask & channel_intr_mask) == 0)
        {
            /* This interrupt was not yet enabled before, enable it now */
            (void)_cyhal_ipc_enable_interrupt(channel_num, events_enabled, true);
        }
    }

    return; // result;
}

cy_rslt_t cyhal_ipc_queue_put(cyhal_ipc_t *obj, void *msg, uint32_t timeout_us)
{
    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* Validate args */
    if ( (obj == NULL) || (msg == NULL) )
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     */
    _CYHAL_CHECK_TIMEOUT_AND_RETURN_IF_IN_ISR(timeout_us);

    return _cyhal_ipc_queue_put_get(obj, msg, timeout_us, true);
}

cy_rslt_t cyhal_ipc_queue_get(cyhal_ipc_t *obj, void *msg, uint32_t timeout_us)
{
    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    /* Validate args */
    if ( (obj == NULL) || (msg == NULL) )
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     */
    _CYHAL_CHECK_TIMEOUT_AND_RETURN_IF_IN_ISR(timeout_us);

    return _cyhal_ipc_queue_put_get(obj, msg, timeout_us, false);
}

uint32_t cyhal_ipc_queue_count(cyhal_ipc_t *obj)
{
    uint32_t num_items = 0;

    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    if (obj == NULL)
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    uint32_t timeout_us = _CYHAL_IPC_SERVICE_SEMA_TIMEOUT_US;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *  return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );

    if (obj->queue_obj != NULL)
    {
        cyhal_ipc_queue_t *queue_handle = (cyhal_ipc_queue_t *)(obj->queue_obj);

        INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue_handle, sizeof(cyhal_ipc_queue_t) );

        num_items = queue_handle->curr_items;
    }
    return num_items;
}

cy_rslt_t cyhal_ipc_queue_reset(cyhal_ipc_t *obj)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (false == _cyhal_ipc_local_initialized)
    {
        return CYHAL_IPC_RSLT_ERR_NOT_INITIALIZED;
    }

    if (obj == NULL)
    {
        return CYHAL_IPC_RSLT_ERR_INVALID_PARAMETER;
    }

    uint32_t timeout_us = _CYHAL_IPC_SERVICE_SEMA_TIMEOUT_US;
    /* Check if we are executing during an ISR, timeout must be 0x00
     * If we add the timeout as an argument, this should also check for
     * timeout !=0 and should fail if in ISR context.
     *  return CYHAL_IPC_RSLT_ERR_CANT_OPERATE_IN_ISR_W_TIMEOUT;
     */
    _CYHAL_ZERO_TIMEOUT_IF_IN_ISR(timeout_us);

    INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(obj, sizeof(cyhal_ipc_t) );
    if (obj->queue_obj != NULL)
    {
        cyhal_ipc_queue_t *queue = (cyhal_ipc_queue_t *)(obj->queue_obj);

        INVALIDATE_DCACHE_BEFORE_READING_FROM_MEMORY(queue, sizeof(cyhal_ipc_queue_t) );

        result = cyhal_ipc_semaphore_take(&queue->queue_semaphore, timeout_us);
        if (CY_RSLT_SUCCESS == result)
        {
            uint32_t i;

            queue->first_item = 0;
            queue->curr_items = 0;

            for (i = 0; i < _CYHAL_IPC_MAX_PROCESSES_PER_QUEUE; i++)
            {
                if (cyhal_ipc_semaphore_take(&queue->notifications[i].notification_semaphore, timeout_us) == CY_RSLT_SUCCESS)
                {
                    // Trigger events CYHAL_IPC_QUEUE_RESET & CYHAL_IPC_QUEUE_EMPTY
                    queue->notifications[i].flags |= queue->notifications[i].mask & CYHAL_IPC_QUEUE_RESET;
                    queue->notifications[i].flags |= queue->notifications[i].mask & CYHAL_IPC_QUEUE_EMPTY;

                    cyhal_ipc_semaphore_give(&queue->notifications[i].notification_semaphore);
                }
            }
            CLEAR_DCACHE_AFTER_WRITING_TO_MEMORY(queue, sizeof(cyhal_ipc_queue_t) );

            _cyhal_ipc_trigger_interrupts_for_channel(queue->channel_num);

            cyhal_ipc_semaphore_give(&queue->queue_semaphore);
        }
    }

    return result;
}


#if defined(__cplusplus)
}
#endif

#endif /* CYHAL_DRIVER_AVAILABLE_IPC */
