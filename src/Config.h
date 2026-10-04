
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
// CO2 CONTROL
// ============================================================

constexpr uint16_t DEFAULT_SETPOINT = 1000;

#define CO2_USER_MIN       500
#define CO2_USER_MAX       1500
#define CO2_SAFETY_LIMIT   2000

// CO2 injection valve
#define CO2_VALVE_PIN              27
#define CO2_VALVE_OPEN_TIME_MS     1000
#define CO2_INJECTION_WAIT_MS      30000

// ============================================================
// RS485 / MODBUS
// ============================================================

#define RS485_RX_PIN       16
#define RS485_TX_PIN       17
#define RS485_DE_RE_PIN    4
#define RS485_BAUDRATE     9600

// Modbus device addresses
#define MIO_MODBUS_ADDRESS        1
#define GMP252_MODBUS_ADDRESS     240
#define HMP60_MODBUS_ADDRESS      241

// ============================================================
// SDP610 DIFFERENTIAL PRESSURE SENSOR
// ============================================================

#define SDP610_SDA_PIN     14
#define SDP610_SCL_PIN     15
#define SDP610_ADDRESS     0x40

// ============================================================
// OLED
// ============================================================

#define OLED_SDA_PIN       21
#define OLED_SCL_PIN       22
#define OLED_ADDRESS       0x3C

#define OLED_WIDTH         128
#define OLED_HEIGHT        64

// ============================================================
// ROTARY ENCODER
// ============================================================

#define ENCODER_A_PIN      32
#define ENCODER_B_PIN      33
#define BUTTON_PIN         25

// ============================================================
// FAN SPEED
// ============================================================

#define FAN_OFF             0
#define FAN_LOW             30
#define FAN_NORMAL          60
#define FAN_HIGH            80
#define FAN_MAX             100

// ============================================================
// FAN PULSE MONITORING
// ============================================================

// Two consecutive zero readings means fan is stopped.
#define FAN_ZERO_READINGS_REQUIRED 2

// ============================================================
// TASK TIMING
// ============================================================

#define SENSOR_PERIOD_MS       1000
#define CONTROLLER_PERIOD_MS   500
#define UI_PERIOD_MS           100
#define NETWORK_PERIOD_MS      15000

// ============================================================
// QUEUES
// ============================================================

#define UI_QUEUE_LENGTH         10
#define SETPOINT_QUEUE_LENGTH   10

// ============================================================
// WIFI
// ============================================================

#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// ============================================================
// CLOUD / THINGSPEAK
// ============================================================

#define THINGSPEAK_API_KEY       "YOUR_WRITE_API_KEY"
#define THINGSPEAK_CHANNEL_ID    "YOUR_CHANNEL_ID"
#define THINGSPEAK_READ_API_KEY  "YOUR_READ_API_KEY"

#define THINGSPEAK_INTERVAL_MS   15000

#endif