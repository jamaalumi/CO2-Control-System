#ifndef CONFIG_H

#define CONFIG_H

#define RS485_RX_PIN        16
#define RS485_TX_PIN        17
#define RS485_DE_RE_PIN     4
#define RS485_BAUDRATE      9600
#define MIO_MODBUS_ADDRESS  1

#define OLED_SDA_PIN        21
#define OLED_SCL_PIN        22
#define OLED_ADDRESS        0x3C
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

#define ENCODER_A_PIN       32
#define ENCODER_B_PIN       33
#define BUTTON_PIN          25

#define CO2_VALVE_PIN       26

#define REG_CO2_PPM         0x0000
#define REG_TEMPERATURE     0x0001
#define REG_HUMIDITY        0x0002
#define REG_FAN_PULSES      0x0003
#define REG_FAN_SPEED       0x0004

#define CO2_SCALE           1.0f
#define TEMPERATURE_SCALE   0.1f
#define HUMIDITY_SCALE      0.1f

#define CO2_USER_MAX        1500
#define CO2_USER_MIN        500
#define CO2_SAFETY_LIMIT    2000

#define FAN_OFF             0
#define FAN_LOW             30
#define FAN_NORMAL          60
#define FAN_HIGH            80
#define FAN_MAX             100

#define SENSOR_PERIOD_MS        1000
#define CONTROLLER_PERIOD_MS    500
#define UI_PERIOD_MS            100
#define NETWORK_PERIOD_MS       15000

#define UI_QUEUE_LENGTH         10
#define SETPOINT_QUEUE_LENGTH   10

#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

#define THINGSPEAK_API_KEY      "YOUR_WRITE_API_KEY"
#define THINGSPEAK_CHANNEL_ID   "YOUR_CHANNEL_ID"
#define THINGSPEAK_READ_API_KEY "YOUR_READ_API_KEY"

#define THINGSPEAK_INTERVAL_MS  15000

#endif