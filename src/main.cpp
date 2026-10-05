#include <iostream>

#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app.h"

extern "C"
{
uint32_t read_runtime_ctr(void)
{
return timer_hw->timerawl;
}
}

int main()
{
stdio_init_all();

sleep_ms(2000);

std::cout << "\n========================================\n";
std::cout << " CO2 Control System\n";
std::cout << " Raspberry Pi Pico + FreeRTOS\n";
std::cout << "========================================\n";

app_init();

app_start_tasks();

std::cout << "Starting FreeRTOS scheduler...\n";

vTaskStartScheduler();

std::cout << "ERROR: Scheduler stopped.\n";

while (true)
{
    tight_loop_contents();
}


}
