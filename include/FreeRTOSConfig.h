#ifndef FREERTOS_H
#define FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * FreeRTOS basic configuration
 * ============================================================ */

#include <stdint.h>
#include <stddef.h>

/* Configuration */

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0

#define configCPU_CLOCK_HZ                     125000000UL
#define configTICK_RATE_HZ                     1000

#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256
#define configTOTAL_HEAP_SIZE                  (16 * 1024)

#define configMAX_TASK_NAME_LEN                 16

#define configUSE_16_BIT_TICKS                  0

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES            1

#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            0

#define configPRIO_BITS                         2

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 3
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* ============================================================
 * API compatibility
 * ============================================================ */

#define INCLUDE_vTaskDelay                     1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_vTaskDelete                    1
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_xTaskGetSchedulerState         1
#define INCLUDE_xTaskGetCurrentTaskHandle      1
#define INCLUDE_uxTaskPriorityGet              1
#define INCLUDE_vTaskPrioritySet               1

/* ============================================================
 * Assertions
 * ============================================================ */

#ifndef configASSERT
#define configASSERT(x) \
    do { \
        if ((x) == 0) { \
            taskDISABLE_INTERRUPTS(); \
            for (;;) {} \
        } \
    } while (0)
#endif

/* ============================================================
 * Interrupt configuration
 * ============================================================ */

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

/* ============================================================
 * Cortex-M configuration
 * ============================================================ */

#define configENABLE_FPU                         0
#define configENABLE_MPU                         0
#define configENABLE_TRUSTZONE                   0

/* ============================================================
 * FreeRTOS version
 * ============================================================ */

#define tskKERNEL_VERSION_MAJOR                  10
#define tskKERNEL_VERSION_MINOR                  5
#define tskKERNEL_VERSION_BUILD                  0

/* ============================================================
 * Include portable types
 * ============================================================ */

#include "portable/GCC/ARM_CM0/portmacro.h"

/* ============================================================
 * FreeRTOS types
 * ============================================================ */

typedef uint32_t TickType_t;
typedef int32_t BaseType_t;
typedef uint32_t UBaseType_t;

#define pdTRUE      ((BaseType_t)1)
#define pdFALSE     ((BaseType_t)0)

#define pdPASS      (pdTRUE)
#define pdFAIL      (pdFALSE)

#define portMAX_DELAY ((TickType_t)0xffffffffUL)

#define pdMS_TO_TICKS(x) \
    ((TickType_t)(((uint64_t)(x) * configTICK_RATE_HZ) / 1000ULL))

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_H */