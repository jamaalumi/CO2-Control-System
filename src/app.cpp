#include "app.h"

#include "config.h"
#include "controller.h"
#include "network.h"
#include "sensors.h"
#include "storage.h"
#include "ui.h"

#include <iostream>

SensorData g_sensor_data = {};

ControllerSettings g_settings = {
Config::DEFAULT_CO2_SETPOINT_PPM
};

SemaphoreHandle_t g_sensor_mutex = nullptr;

SemaphoreHandle_t g_modbus_mutex = nullptr;

QueueHandle_t g_setpoint_queue = nullptr;

QueueHandle_t g_ui_event_queue = nullptr;

static void task_creation_error(
const char *name
)
{
std::cout
<< "ERROR: failed to create task: "
<< name
<< "\n";
}

void app_init()
{
g_sensor_mutex =
xSemaphoreCreateMutex();


g_modbus_mutex =
    xSemaphoreCreateMutex();

g_setpoint_queue =
    xQueueCreate(
        8,
        sizeof(uint16_t)
    );

g_ui_event_queue =
    xQueueCreate(
        8,
        sizeof(UiEvent)
    );

if (!g_sensor_mutex ||
    !g_modbus_mutex ||
    !g_setpoint_queue ||
    !g_ui_event_queue)
{
    std::cout
        << "ERROR: FreeRTOS object creation failed.\n";

    taskDISABLE_INTERRUPTS();

    while (true)
    {
    }
}

if (!storage_load_settings(g_settings))
{
    g_settings.co2_setpoint_ppm =
        Config::DEFAULT_CO2_SETPOINT_PPM;

    storage_save_settings(g_settings);
}

ui_init();


}

void app_start_tasks()
{
BaseType_t result;


result = xTaskCreate(
    sensors_task,
    "SENSORS",
    512,
    nullptr,
    tskIDLE_PRIORITY + 2,
    nullptr
);

if (result != pdPASS)
    task_creation_error("SENSORS");

result = xTaskCreate(
    controller_task,
    "CONTROL",
    512,
    nullptr,
    tskIDLE_PRIORITY + 3,
    nullptr
);

if (result != pdPASS)
    task_creation_error("CONTROL");

result = xTaskCreate(
    ui_task,
    "UI",
    512,
    nullptr,
    tskIDLE_PRIORITY + 1,
    nullptr
);

if (result != pdPASS)
    task_creation_error("UI");

result = xTaskCreate(
    network_task,
    "NETWORK",
    768,
    nullptr,
    tskIDLE_PRIORITY + 1,
    nullptr
);

if (result != pdPASS)
    task_creation_error("NETWORK");

}

extern "C" void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    char *pcTaskName
)
{
    (void)xTask;
    (void)pcTaskName;

    while (true)
    {
        tight_loop_contents();
    }
}