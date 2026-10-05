#pragma once

#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

namespace Config
{
    // ========================================================
    // UART / MODBUS
    // ========================================================

    static uart_inst_t* const MODBUS_UART = uart1;

    constexpr uint32_t MODBUS_BAUD = 9600;
    constexpr uint32_t MODBUS_BAUD_RATE = 9600;

    constexpr uint8_t MODBUS_TX_PIN = 4;
    constexpr uint8_t MODBUS_RX_PIN = 5;

    constexpr uint8_t RS485_DE_PIN = 6;


    // ========================================================
    // SENSOR
    // ========================================================

    constexpr uint32_t SENSOR_PERIOD_MS = 1000;
    constexpr uint32_t SENSOR_TASK_PERIOD_MS = 1000;

    constexpr uint8_t SDP610_SDA_PIN = 8;
    constexpr uint8_t SDP610_SCL_PIN = 9;


    // ========================================================
    // CONTROLLER
    // ========================================================

    constexpr uint32_t CONTROLLER_PERIOD_MS = 100;
    constexpr uint32_t CONTROLLER_TASK_PERIOD_MS = 100;

    constexpr uint16_t DEFAULT_CO2_SETPOINT_PPM = 800;

    constexpr uint16_t CO2_MIN_PPM = 800;
    constexpr uint16_t CO2_MAX_PPM = 1200;

    constexpr uint16_t MAX_CO2_SETPOINT_PPM = 1200;

    constexpr uint16_t SAFETY_CO2_PPM = 1500;
    constexpr uint16_t CO2_SAFETY_LIMIT_PPM = 1500;


    // ========================================================
    // UI
    // ========================================================

    constexpr uint32_t UI_PERIOD_MS = 100;
    constexpr uint32_t UI_TASK_PERIOD_MS = 100;


    // ========================================================
    // NETWORK
    // ========================================================

    constexpr uint32_t NETWORK_PERIOD_MS = 1000;


    // ========================================================
    // GPIO
    // ========================================================

    constexpr uint8_t FAN_PIN = 15;

    constexpr uint8_t CO2_VALVE_PIN = 14;

    constexpr uint8_t LED_PIN = 25;


    // ========================================================
    // CO2 VALVE
    // ========================================================

    constexpr uint32_t CO2_VALVE_ON_MS = 500;
    constexpr uint32_t CO2_VALVE_MIN_OFF_MS = 5000;


    // ========================================================
    // QUEUES
    // ========================================================

    constexpr uint32_t SETPOINT_QUEUE_LENGTH = 5;
    constexpr uint32_t UI_EVENT_QUEUE_LENGTH = 10;


    // ========================================================
    // TASK STACK SIZES
    // ========================================================

    constexpr uint32_t SENSOR_TASK_STACK_SIZE = 512;
    constexpr uint32_t CONTROLLER_TASK_STACK_SIZE = 512;
    constexpr uint32_t UI_TASK_STACK_SIZE = 512;
    constexpr uint32_t NETWORK_TASK_STACK_SIZE = 512;
    constexpr uint32_t MODBUS_TASK_STACK_SIZE = 512;


    // ========================================================
    // TASK PRIORITIES
    // ========================================================

    constexpr uint32_t SENSOR_TASK_PRIORITY = 2;
    constexpr uint32_t CONTROLLER_TASK_PRIORITY = 3;
    constexpr uint32_t UI_TASK_PRIORITY = 1;
    constexpr uint32_t NETWORK_TASK_PRIORITY = 2;
    constexpr uint32_t MODBUS_TASK_PRIORITY = 2;
}