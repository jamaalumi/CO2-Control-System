#pragma once

#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

// ============================================================
// UART / MODBUS CONFIGURATION
// ============================================================

#define MODBUS_UART uart1

#define MODBUS_BAUD_RATE 9600

#define MODBUS_TX_PIN 4
#define MODBUS_RX_PIN 5

// ============================================================
// SENSOR CONFIGURATION
// ============================================================

#define SENSOR_TASK_PERIOD_MS 1000

// ============================================================
// CONTROLLER CONFIGURATION
// ============================================================

#define CONTROLLER_TASK_PERIOD_MS 100

// CO2 limits in ppm
#define CO2_MIN_PPM 800
#define CO2_MAX_PPM 1200

// Safety limit
#define CO2_SAFETY_LIMIT_PPM 1500

// ============================================================
// UI CONFIGURATION
// ============================================================

#define UI_TASK_PERIOD_MS 100

// ============================================================
// QUEUE CONFIGURATION
// ============================================================

#define SETPOINT_QUEUE_LENGTH 5
#define UI_EVENT_QUEUE_LENGTH 10

// ============================================================
// GPIO CONFIGURATION
// ============================================================

// Ventilation fan
#define FAN_PIN 15

// CO2 injection valve
#define CO2_VALVE_PIN 14

// Optional status LED
#define LED_PIN 25

// ============================================================
// STACK / PRIORITY CONFIGURATION
// ============================================================

#define SENSOR_TASK_STACK_SIZE 512
#define CONTROLLER_TASK_STACK_SIZE 512
#define UI_TASK_STACK_SIZE 512
#define MODBUS_TASK_STACK_SIZE 512

#define SENSOR_TASK_PRIORITY 2
#define CONTROLLER_TASK_PRIORITY 3
#define UI_TASK_PRIORITY 1
#define MODBUS_TASK_PRIORITY 2