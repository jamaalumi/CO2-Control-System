#pragma once

// ============================================================
// FreeRTOSConfig.h
// Raspberry Pi Pico / RP2040
// ============================================================

#include <stdint.h>

// ------------------------------------------------------------
// Scheduler
// ------------------------------------------------------------

#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

#define configCPU_CLOCK_HZ                     125000000UL
#define configTICK_RATE_HZ                     1000

#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 16

#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

// ------------------------------------------------------------
// Memory
// ------------------------------------------------------------

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configTOTAL_HEAP_SIZE                  (32 * 1024)

// ------------------------------------------------------------
// Synchronization
// ------------------------------------------------------------

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1

#define configUSE_COUNTING_SEMAPHORES            1

// ------------------------------------------------------------
// Queues
// ------------------------------------------------------------

#define configQUEUE_REGISTRY_SIZE               10

// ------------------------------------------------------------
// Software timers
// ------------------------------------------------------------

#define configUSE_TIMERS                        0

// ------------------------------------------------------------
// Task notifications
// ------------------------------------------------------------

#define configUSE_TASK_NOTIFICATIONS             1

// ------------------------------------------------------------
// Hook functions
// ------------------------------------------------------------

#define configUSE_MALLOC_FAILED_HOOK             0
#define configUSE_DAEMON_TASK_STARTUP_HOOK       0
#define configCHECK_FOR_STACK_OVERFLOW           0

// ------------------------------------------------------------
// Co-routines
// ------------------------------------------------------------

#define configUSE_CO_ROUTINES                    0

// ------------------------------------------------------------
// Optional API functions
// ------------------------------------------------------------

#define INCLUDE_vTaskDelay                       1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskPrioritySet                 1

// ------------------------------------------------------------
// Cortex-M0+ interrupt configuration
// ------------------------------------------------------------

#define configPRIO_BITS                          2

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY  3
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

// ------------------------------------------------------------
// Assert
// ------------------------------------------------------------

#define configASSERT(x)                          \
    if ((x) == 0)                                \
    {                                            \
        taskDISABLE_INTERRUPTS();               \
        for (;;) {}                              \
    }

// ------------------------------------------------------------
// Optional features
// ------------------------------------------------------------

#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configGENERATE_RUN_TIME_STATS           0

// ------------------------------------------------------------
// Newlib
// ------------------------------------------------------------

#define configUSE_NEWLIB_REENTRANT               0