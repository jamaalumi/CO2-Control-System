#include "network.h"

#include "app.h"
#include "config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <iostream>

void network_task(
void *parameter
)
{
(void)parameter;

```
std::cout
    << "Network task started.\n";

std::cout
    << "ThingSpeak network integration "
    << "is not enabled yet.\n";

while (true)
{
    /*
     * Later this task will:
     *
     * 1. Read shared sensor data.
     *
     * 2. Connect to the network.
     *
     * 3. Send measurements to ThingSpeak.
     *
     * 4. Receive a remote setpoint.
     *
     * 5. Validate the setpoint.
     *
     * 6. Send the setpoint to
     *    g_setpoint_queue.
     *
     * The standard Raspberry Pi Pico has
     * no built-in Wi-Fi.
     */

    vTaskDelay(
        pdMS_TO_TICKS(
            Config::NETWORK_PERIOD_MS
        )
    );
}
```

}
