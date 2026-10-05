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
    float temperature;
    float humidity;
};

// ============================================================
// GLOBAL SHARED DATA
// ============================================================

extern SensorData g_sensor_data;

// ============================================================
// FREERTOS SYNCHRONIZATION OBJECTS
// ============================================================

// Protects g_sensor_data
extern SemaphoreHandle_t g_sensor_mutex;

// Protects Modbus communication
extern SemaphoreHandle_t g_modbus_mutex;

// ============================================================
// FREERTOS QUEUES
// ============================================================

// CO2 setpoint queue
extern QueueHandle_t g_setpoint_queue;

// UI event queue
extern QueueHandle_t g_ui_event_queue;

// ============================================================
// TASK FUNCTIONS
// ============================================================

void sensorTask(void *pvParameters);

void controllerTask(void *pvParameters);

void uiTask(void *pvParameters);

void modbusTask(void *pvParameters);

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