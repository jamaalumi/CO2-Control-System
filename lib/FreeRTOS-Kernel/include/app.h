#ifndef APP_H
#define APP_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include <stdint.h>

struct SensorData
{
uint16_t co2_ppm;


float temperature_c;
float humidity_percent;
float pressure_pa;

uint32_t fan_pulse_count;

bool fan_running;

uint32_t timestamp_ms;


};

struct ControllerSettings
{
uint16_t co2_setpoint_ppm;
};

extern SensorData g_sensor_data;

extern ControllerSettings g_settings;

extern SemaphoreHandle_t g_sensor_mutex;

extern SemaphoreHandle_t g_modbus_mutex;

extern QueueHandle_t g_setpoint_queue;

extern QueueHandle_t g_ui_event_queue;

void app_init();

void app_start_tasks();

#endif
