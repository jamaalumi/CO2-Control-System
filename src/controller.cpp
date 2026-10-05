#include "controller.h"

#include "app.h"
#include "config.h"
#include "modbus.h"

#include "hardware/gpio.h"

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include <iostream>

static uint16_t get_co2_setpoint()
{
uint16_t setpoint =
Config::DEFAULT_CO2_SETPOINT_PPM;

```
if (
    xSemaphoreTake(
        g_sensor_mutex,
        pdMS_TO_TICKS(50)
    ) == pdTRUE
)
{
    setpoint =
        g_settings.co2_setpoint_ppm;

    xSemaphoreGive(
        g_sensor_mutex
    );
}

if (
    setpoint >
    Config::MAX_CO2_SETPOINT_PPM
)
{
    setpoint =
        Config::MAX_CO2_SETPOINT_PPM;
}

return setpoint;
```

}

static uint16_t get_co2_value()
{
uint16_t co2 = 0;

```
if (
    xSemaphoreTake(
        g_sensor_mutex,
        pdMS_TO_TICKS(50)
    ) == pdTRUE
)
{
    co2 =
        g_sensor_data.co2_ppm;

    xSemaphoreGive(
        g_sensor_mutex
    );
}

return co2;
```

}

static void set_fan(
uint8_t percent
)
{
if (
xSemaphoreTake(
g_modbus_mutex,
pdMS_TO_TICKS(200)
) == pdTRUE
)
{
produal_set_fan_percent(
percent
);

```
    xSemaphoreGive(
        g_modbus_mutex
    );
}
```

}

static void valve_on()
{
gpio_put(
Config::CO2_VALVE_PIN,
1
);
}

static void valve_off()
{
gpio_put(
Config::CO2_VALVE_PIN,
0
);
}

void controller_task(
void *parameter
)
{
(void)parameter;

```
gpio_init(
    Config::CO2_VALVE_PIN
);

gpio_set_dir(
    Config::CO2_VALVE_PIN,
    GPIO_OUT
);

valve_off();

uint32_t last_valve_open_ms = 0;

while (true)
{
    uint16_t queued_setpoint;

    while (
        xQueueReceive(
            g_setpoint_queue,
            &queued_setpoint,
            0
        ) == pdTRUE
    )
    {
        if (
            queued_setpoint >
            Config::MAX_CO2_SETPOINT_PPM
        )
        {
            queued_setpoint =
                Config::MAX_CO2_SETPOINT_PPM;
        }

        g_settings.co2_setpoint_ppm =
            queued_setpoint;
    }

    const uint16_t co2 =
        get_co2_value();

    const uint16_t setpoint =
        get_co2_setpoint();

    const uint32_t now =
        to_ms_since_boot(
            get_absolute_time()
        );

    /*
     * SAFETY:
     *
     * CO2 > 2000 ppm
     * -> maximum ventilation.
     */
    if (
        co2 >
        Config::SAFETY_CO2_PPM
    )
    {
        set_fan(100);
    }

    /*
     * Above requested level but below
     * safety level:
     *
     * No need to vent solely because
     * CO2 is above the setpoint.
     */
    else if (co2 > setpoint)
    {
        set_fan(0);
    }

    /*
     * Below requested level:
     * inject CO2 if the 30-second
     * lockout has expired.
     */
    else
    {
        set_fan(0);

        const bool enough_time_passed =
            (last_valve_open_ms == 0) ||
            (
                now -
                last_valve_open_ms
                >=
                Config::CO2_VALVE_MIN_OFF_MS
            );

        if (enough_time_passed)
        {
            valve_on();

            vTaskDelay(
                pdMS_TO_TICKS(
                    Config::CO2_VALVE_ON_MS
                )
            );

            valve_off();

            last_valve_open_ms =
                to_ms_since_boot(
                    get_absolute_time()
                );
        }
    }

    vTaskDelay(
        pdMS_TO_TICKS(
            Config::CONTROLLER_PERIOD_MS
        )
    );
}
```

}
