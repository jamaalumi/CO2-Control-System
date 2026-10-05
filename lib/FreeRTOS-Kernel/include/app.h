#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


// ============================================================
// SHARED SENSOR DATA
// ============================================================

struct SensorData
{
    float co2_ppm;

    float temperature_c;

    float humidity_percent;

    float pressure_pa;

    uint32_t fan_pulse_count;

    bool fan_running;

    uint32_t timestamp_ms;
};


// ============================================================
// CONTROLLER SETTINGS
// ============================================================

struct ControllerSettings
{
    uint16_t co2_setpoint_ppm;
};


// ============================================================
// GLOBAL SHARED DATA
// ============================================================

extern SensorData g_sensor_data;

extern ControllerSettings g_settings;


// ============================================================
// FREERTOS SYNCHRONIZATION OBJECTS
// ============================================================

extern SemaphoreHandle_t g_sensor_mutex;

extern SemaphoreHandle_t g_modbus_mutex;


// ============================================================
// FREERTOS QUEUES
// ============================================================

extern QueueHandle_t g_setpoint_queue;

extern QueueHandle_t g_ui_event_queue;


// ============================================================
// TASK FUNCTIONS
// ============================================================

void sensorTask(void *pvParameters);

void controllerTask(void *pvParameters);

void uiTask(void *pvParameters);

void modbusTask(void *pvParameters);

void networkTask(void *pvParameters);


// ============================================================
// SENSOR FUNCTIONS
// ============================================================

bool readSensors(SensorData &data);


// ============================================================
// CONTROLLER FUNCTIONS
// ============================================================

void updateController(const SensorData &data);


// ============================================================
// UI EVENTS
// ============================================================

enum class UIEvent : uint8_t
{
    NONE = 0,

    BUTTON_UP,

    BUTTON_DOWN,

    BUTTON_SELECT,

    BUTTON_BACK
};