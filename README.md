# CO₂ Control and Ventilation System

## Overview

This project is an ESP32-based CO₂ monitoring and ventilation control system.

The system monitors CO₂ concentration and environmental sensor data through a Produal MIO device using Modbus/RS-485. The ESP32 runs multiple FreeRTOS tasks to separate sensor reading, control logic, user interface, and network communication.

The system can control a ventilation fan and a CO₂ injection valve based on the measured CO₂ concentration.

## System Architecture

The application consists of four main FreeRTOS tasks:

### 1. Sensor Task

The Sensor Task:

- Reads CO₂, temperature and humidity from the Produal MIO.
- Reads the fan pulse count from the MIO.
- Updates the shared sensor data structure.
- Uses a mutex to protect shared sensor data.
- Uses the RS-485 mutex when accessing the Modbus bus.

The Produal MIO handles fan pulse counting in hardware. The ESP32 only reads the pulse count through Modbus.

### 2. Controller Task

The Controller Task:

- Runs the main control loop.
- Reads the latest sensor values.
- Controls the ventilation fan.
- Controls the CO₂ injection valve.
- Receives CO₂ setpoints through a FreeRTOS queue.
- Enforces the CO₂ safety limit.

The hard safety limit is:

**2000 ppm CO₂**

When the CO₂ concentration reaches or exceeds 2000 ppm:

- CO₂ injection is disabled.
- Ventilation is set to maximum.

The normal user-adjustable CO₂ setpoint has a maximum of:

**1500 ppm**

### 3. UI Task

The UI Task:

- Displays sensor measurements on the OLED display.
- Processes rotary encoder input.
- Processes the encoder/button input.
- Allows the user to change the CO₂ setpoint.
- Saves the setpoint to non-volatile storage.
- Sends updated setpoints to the Controller Task.

The UI cannot set the normal CO₂ limit above 1500 ppm.

### 4. Network Task

The Network Task:

- Connects the ESP32 to Wi-Fi.
- Sends sensor data to ThingSpeak.
- Receives a possible CO₂ setpoint from the cloud.
- Sends cloud setpoints to the Controller Task through the same setpoint queue.

## FreeRTOS Communication

### Shared Sensor Data

Sensor readings are stored in a shared structure:

cpp
struct SensorData
{
    float co2Ppm;
    float temperature;
    float humidity;
    uint32_t fanPulses;
    uint32_t timestamp;
    bool valid;
};