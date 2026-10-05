#include "sensors.h"

#include "app.h"
#include "config.h"
#include "modbus.h"

#include "hardware/i2c.h"

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include <iostream>

static bool read_sdp610(
float &pressure_pa
)
{
/*
* SDP610 I2C sensor.
*
* Address: 0x40
* SDA: GPIO14
* SCL: GPIO15
*
* Exact command and scale conversion should be taken
* from the datasheet revision used by the project.
*/


pressure_pa = 0.0f;

return false;


}

static void update_sensor_data(
uint16_t co2,
float temperature,
float humidity,
float pressure,
uint32_t pulses
)
{
if (
xSemaphoreTake(
g_sensor_mutex,
pdMS_TO_TICKS(100)
) == pdTRUE
)
{
g_sensor_data.co2_ppm = co2;


    g_sensor_data.temperature_c =
        temperature;

    g_sensor_data.humidity_percent =
        humidity;

    g_sensor_data.pressure_pa =
        pressure;

    g_sensor_data.fan_pulse_count =
        pulses;

    g_sensor_data.fan_running =
        pulses > 0;

    g_sensor_data.timestamp_ms =
        to_ms_since_boot(
            get_absolute_time()
        );

    xSemaphoreGive(
        g_sensor_mutex
    );
}


}

void sensors_task(
void *parameter
)
{
(void)parameter;


modbus_init();

i2c_init(
    i2c1,
    100 * 1000
);

gpio_set_function(
    Config::SDP610_SDA_PIN,
    GPIO_FUNC_I2C
);

gpio_set_function(
    Config::SDP610_SCL_PIN,
    GPIO_FUNC_I2C
);

gpio_pull_up(
    Config::SDP610_SDA_PIN
);

gpio_pull_up(
    Config::SDP610_SCL_PIN
);

TickType_t last_wake =
    xTaskGetTickCount();

while (true)
{
    uint16_t co2 = 0;

    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;

    uint32_t pulses = 0;

    if (
        xSemaphoreTake(
            g_modbus_mutex,
            pdMS_TO_TICKS(200)
        ) == pdTRUE
    )
    {
        gmp252_read_co2(co2);

        hmp60_read_environment(
            temperature,
            humidity
        );

        produal_read_fan_pulse_counter(
            pulses
        );

        xSemaphoreGive(
            g_modbus_mutex
        );
    }

    read_sdp610(pressure);

    update_sensor_data(
        co2,
        temperature,
        humidity,
        pressure,
        pulses
    );

    vTaskDelayUntil(
        &last_wake,
        pdMS_TO_TICKS(
            Config::SENSOR_PERIOD_MS
        )
    );
}


}
