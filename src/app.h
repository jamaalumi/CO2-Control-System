#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================
// GLOBAL HARDWARE OBJECTS
// ============================================================

extern HardwareSerial RS485;

extern Adafruit_SSD1306 display;

// ============================================================
// SENSOR DATA
// ============================================================

struct SensorData
{
    float co2Ppm;
    float temperature;
    float humidity;

    uint16_t fanPulses;
    uint16_t fanSpeed;

    unsigned long timestamp;

    bool valid;
};

// ============================================================
// UI EVENTS
// ============================================================

enum class UIEventType
{
    NONE = 0,
    ENCODER_CW,
    ENCODER_CCW,
    BUTTON_PRESS
};

struct UIEvent
{
    UIEventType type;
};

// ============================================================
// SETPOINT MESSAGE
// ============================================================

struct SetpointMessage
{
    uint16_t co2Setpoint;
};

// ============================================================
// FREERTOS OBJECTS
// ============================================================

extern SemaphoreHandle_t sensorDataMutex;
extern SemaphoreHandle_t rs485Mutex;

extern QueueHandle_t uiEventQueue;
extern QueueHandle_t setpointQueue;

extern SensorData sharedSensorData;

// ============================================================
// TASKS
// ============================================================

void SensorTask(void *parameter);

void ControllerTask(void *parameter);

void UITask(void *parameter);

void NetworkTask(void *parameter);

// ============================================================
// RS485 / FAN / VALVE
// ============================================================

bool setFanSpeed(uint8_t speed);

void setCO2Valve(bool enabled);

// ============================================================
// SETPOINT
// ============================================================

uint16_t loadSetpoint();

void saveSetpoint(uint16_t value);

void sendSetpointToController(uint16_t setpoint);

// ============================================================
// INTERRUPTS
// ============================================================

void encoderISR();

void buttonISR();

// ============================================================
// DISPLAY
// ============================================================

void updateDisplay(
    const SensorData& data,
    uint16_t setpoint,
    bool safetyMode
);

// ============================================================
// WIFI
// ============================================================

bool connectWiFi();

#endif