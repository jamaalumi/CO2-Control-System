#include "ui.h"

#include "app.h"
#include "config.h"
#include "storage.h"

#include "hardware/gpio.h"

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include <iostream>

static void gpio_callback(
uint gpio,
uint32_t events
)
{
(void)gpio;
(void)events;


BaseType_t higher_priority_task_woken =
    pdFALSE;

UiEvent event;

event.type =
    UiEventType::BUTTON_PRESSED;

xQueueSendFromISR(
    g_ui_event_queue,
    &event,
    &higher_priority_task_woken
);

portYIELD_FROM_ISR(
    higher_priority_task_woken
);


}

void ui_init()
{
gpio_init(
Config::UI_BUTTON_PIN
);


gpio_set_dir(
    Config::UI_BUTTON_PIN,
    GPIO_IN
);

gpio_set_pulls(
    Config::UI_BUTTON_PIN,
    true,
    false
);

gpio_set_irq_enabled_with_callback(
    Config::UI_BUTTON_PIN,
    GPIO_IRQ_EDGE_FALL,
    true,
    &gpio_callback
);


}

void ui_task(
void *parameter
)
{
(void)parameter;


std::cout
    << "UI task started.\n";

std::cout
    << "Current CO2 setpoint: "
    << g_settings.co2_setpoint_ppm
    << " ppm\n";

while (true)
{
    UiEvent event;

    if (
        xQueueReceive(
            g_ui_event_queue,
            &event,
            pdMS_TO_TICKS(
                Config::UI_PERIOD_MS
            )
        ) == pdTRUE
    )
    {
        if (
            event.type ==
            UiEventType::BUTTON_PRESSED
        )
        {
            uint16_t new_setpoint =
                g_settings.co2_setpoint_ppm
                +100;

            if (
                new_setpoint >
                Config::MAX_CO2_SETPOINT_PPM
            )
            {
                new_setpoint =
                    Config::DEFAULT_CO2_SETPOINT_PPM;
            }

            g_settings.co2_setpoint_ppm =
                new_setpoint;

            storage_save_settings(
                g_settings
            );

            xQueueSend(
                g_setpoint_queue,
                &new_setpoint,
                pdMS_TO_TICKS(20)
            );

            std::cout
                << "UI: new CO2 setpoint = "
                << new_setpoint
                << " ppm\n";
        }
    }

    SensorData copy = {};

    if (
        xSemaphoreTake(
            g_sensor_mutex,
            pdMS_TO_TICKS(10)
        ) == pdTRUE
    )
    {
        copy =
            g_sensor_data;

        xSemaphoreGive(
            g_sensor_mutex
        );
    }

    std::cout
        << "CO2="
        << copy.co2_ppm
        << " ppm, Temp="
        << copy.temperature_c
        << " C, RH="
        << copy.humidity_percent
        << " %, Pressure="
        << copy.pressure_pa
        << " Pa, FanPulses="
        << copy.fan_pulse_count
        << "\n";
}


}
