#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "hardware/uart.h"

namespace Config
{
    // ========================================================
    // CO2 limits
    // ========================================================

    constexpr uint16_t DEFAULT_CO2_SETPOINT_PPM = 1000;
    constexpr uint16_t MAX_CO2_SETPOINT_PPM = 1500;
    constexpr uint16_t SAFETY_CO2_PPM = 2000;


    // ========================================================
    // Task timing
    // ========================================================

    constexpr uint32_t SENSOR_PERIOD_MS = 1000;
    constexpr uint32_t CONTROLLER_PERIOD_MS = 500;
    constexpr uint32_t NETWORK_PERIOD_MS = 15000;
    constexpr uint32_t UI_PERIOD_MS = 100;


    // ========================================================
    // RS-485 / Modbus
    // ========================================================

    constexpr uint MODBUS_TX_PIN = 4;
    constexpr uint MODBUS_RX_PIN = 5;
    constexpr uint RS485_DE_PIN = 6;
    inline uart_inst_t* MODBUS_UART = uart1;
    constexpr uint32_t MODBUS_BAUD = 9600;


    // ========================================================
    // CO2 valve
    // ========================================================

    constexpr uint CO2_VALVE_PIN = 7;

    constexpr uint32_t CO2_VALVE_ON_MS = 1000;
    constexpr uint32_t CO2_VALVE_MIN_OFF_MS = 5000;


    // ========================================================
    // SDP610 pressure sensor
    // ========================================================

    constexpr uint SDP610_SDA_PIN = 14;
    constexpr uint SDP610_SCL_PIN = 15;


    // ========================================================
    // UI
    // ========================================================

    constexpr uint UI_BUTTON_PIN = 16;


    // ========================================================
    // Flash storage
    // ========================================================

    constexpr uint32_t SETTINGS_MAGIC = 0x434F3243UL;
}

#endif

