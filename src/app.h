#ifndef APP_H
#define APP_H

#include <Arduino.h>

struct SensorData
{
    float co2Ppm;
    float temperature;
    float humidity;

    uint32_t fanPulses;

    uint32_t timestamp;

    bool valid;
};

enum class UIEventType
{
    ENCODER_CW,
    ENCODER_CCW,
    BUTTON_PRESS
};

struct UIEvent
{
    UIEventType type;
};

struct SetpointMessage
{
    uint16_t co2Setpoint;
};

extern SemaphoreHandle_t sensorDataMutex;
extern SemaphoreHandle_t rs485Mutex;
extern QueueHandle_t uiEventQueue;
extern QueueHandle_t setpointQueue;

extern SensorData sharedSensorData;

void SensorTask(void *parameter);

void ControllerTask(void *parameter);

void UITask(void *parameter);

void NetworkTask(void *parameter);

void IRAM_ATTR encoderISR();

void IRAM_ATTR buttonISR();

bool modbusReadHoldingRegisters(
    uint8_t slaveAddress,
    uint16_t startRegister,
    uint16_t quantity,
    uint16_t *buffer
);

bool modbusWriteRegister(
    uint8_t slaveAddress,
    uint16_t registerAddress,
    uint16_t value
);

bool readMioSensors(
    SensorData &data
);

bool setFanSpeed(
    uint8_t percentage
);

void setCO2Valve(
    bool enabled
);

uint16_t loadSetpoint();

void saveSetpoint(
    uint16_t setpoint
);

void sendSetpointToController(
    uint16_t setpoint
);

void updateDisplay(
    const SensorData &data,
    uint16_t setpoint,
    bool safetyMode
);

#endif