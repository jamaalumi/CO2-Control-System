
#include <iostream>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"


// ============================================================
// Runtime counter required by FreeRTOS/Pico
// ============================================================

extern "C"
{
    uint32_t read_runtime_ctr(void)
    {
        return timer_hw->timerawl;
    }
}


// ============================================================
// FreeRTOS test task
// ============================================================

void led_task(void *param)
{
    (void)param;

    const uint LED_PIN = 25;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true)
    {
        gpio_put(LED_PIN, 1);

        vTaskDelay(
            pdMS_TO_TICKS(500)
        );

        gpio_put(LED_PIN, 0);

        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}


// ============================================================
// Serial test task
// ============================================================

void serial_task(void *param)
{
    (void)param;

    while (true)
    {
        std::cout
            << "FreeRTOS task running..."
            << std::endl;

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}


// ============================================================
// Main
// ============================================================

int main()
{
    stdio_init_all();

    sleep_ms(2000);

    std::cout
        << "================================"
        << std::endl;

    std::cout
        << " Raspberry Pi Pico FreeRTOS"
        << std::endl;

    std::cout
        << " Test starting..."
        << std::endl;

    std::cout
        << "================================"
        << std::endl;


    BaseType_t result;


    // --------------------------------------------------------
    // Create LED task
    // --------------------------------------------------------

    result = xTaskCreate(
        led_task,
        "LED",
        256,
        nullptr,
        tskIDLE_PRIORITY + 1,
        nullptr
    );

    if (result != pdPASS)
    {
        std::cout
            << "ERROR: LED task creation failed!"
            << std::endl;
    }


    // --------------------------------------------------------
    // Create serial task
    // --------------------------------------------------------

    result = xTaskCreate(
        serial_task,
        "SERIAL",
        256,
        nullptr,
        tskIDLE_PRIORITY + 1,
        nullptr
    );

    if (result != pdPASS)
    {
        std::cout
            << "ERROR: Serial task creation failed!"
            << std::endl;
    }


    // --------------------------------------------------------
    // Start FreeRTOS
    // --------------------------------------------------------

    std::cout
        << "Starting FreeRTOS scheduler..."
        << std::endl;


    vTaskStartScheduler();


    // Scheduler should never return
    while (true)
    {
    }
}

