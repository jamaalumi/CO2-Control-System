#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0

#define configCPU_CLOCK_HZ                     125000000UL
#define configTICK_RATE_HZ                     1000

#define configMAX_PRIORITIES                    8
#define configMINIMAL_STACK_SIZE               128
#define configMAX_TASK_NAME_LEN                16

#define configUSE_16_BIT_TICKS                 0
#define configIDLE_SHOULD_YIELD                1

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES            1
#define configUSE_COUNTING_SEMAPHORES           1

#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE              8

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY              2
#define configTIMER_QUEUE_LENGTH               10
#define configTIMER_TASK_STACK_DEPTH            256

#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1

#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            0

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#define configKERNEL_INTERRUPT_PRIORITY         255

#define configPRIO_BITS                         2
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191
#define configMAX_API_CALL_INTERRUPT_PRIORITY   191

#define configUSE_DYNAMIC_EXCEPTION_HANDLERS    1

#define configASSERT(x)                         \
    do                                          \
    {                                           \
        if ((x) == 0)                           \
        {                                       \
            taskDISABLE_INTERRUPTS();           \
            for (;;)                            \
            {                                   \
            }                                   \
        }                                       \
    } while (0)

#endif