#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "hardware/gpio.h"
#include "hardware/uart.h"

namespace Config
{
constexpr uint16_t DEFAULT_CO2_SETPOINT_PPM = 1000;
constexpr uint16_t MAX_CO2_SETPOINT_PPM = 1500;
constexpr uint16_t SAFETY_CO2_PPM = 2000;


constexpr uint32_t CO2_VALVE_ON_MS = 1000;
constexpr uint32_t CO2_VALVE_MIN_OFF_MS = 30000;

constexpr uint CO2_VALVE_PIN = 27;

constexpr uint UI_BUTTON_PIN = 9;

constexpr uart_inst_t *MODBUS_UART = uart1;

constexpr uint MODBUS_TX_PIN = 4;
constexpr uint MODBUS_RX_PIN = 5;
constexpr uint RS485_DE_PIN = 6;

constexpr uint32_t MODBUS_BAUD = 9600;

constexpr uint8_t PRODUAL_ADDRESS = 1;
constexpr uint8_t GMP252_ADDRESS = 240;
constexpr uint8_t HMP60_ADDRESS = 241;

constexpr uint8_t SDP610_I2C_ADDRESS = 0x40;

constexpr uint SDP610_SDA_PIN = 14;
constexpr uint SDP610_SCL_PIN = 15;

constexpr uint32_t SENSOR_PERIOD_MS = 1000;
constexpr uint32_t CONTROLLER_PERIOD_MS = 250;
constexpr uint32_t UI_PERIOD_MS = 100;
constexpr uint32_t NETWORK_PERIOD_MS = 10000;

constexpr uint32_t SETTINGS_MAGIC = 0x434F3243;


}

#endif
