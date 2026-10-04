#include "config.h"
#include "app.h"

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================
// GLOBAL HARDWARE OBJECTS
// ============================================================

HardwareSerial RS485(1);

Adafruit_SSD1306 display(
    OLED_WIDTH,
    OLED_HEIGHT,
    &Wire,
    -1
);

// ============================================================
// FREERTOS OBJECTS
// ============================================================

SemaphoreHandle_t sensorDataMutex = nullptr;
SemaphoreHandle_t rs485Mutex = nullptr;

QueueHandle_t uiEventQueue = nullptr;
QueueHandle_t setpointQueue = nullptr;

// ============================================================
// SHARED SENSOR DATA
// ============================================================

SensorData sharedSensorData =
{
    0.0f,   // CO2
    0.0f,   // temperature
    0.0f,   // humidity
    0.0f,   // pressure
    0,      // fan pulses
    0,      // fan speed
    0,      // timestamp
    false   // valid
};

// ============================================================
// INTERNAL STATE
// ============================================================

static Preferences preferences;

static uint16_t currentSetpoint = DEFAULT_SETPOINT;

static bool controllerSafetyMode = false;

static bool valveOpen = false;

static unsigned long valveOpenedAt = 0;

static unsigned long lastInjectionTime = 0;

static uint8_t consecutiveZeroFanReads = 0;

// ============================================================
// MODBUS REGISTER DEFINITIONS
// ============================================================
//
// IMPORTANT:
// These register addresses must be verified against the
// course datasheets.
//
// Do NOT change these values unless they match your
// provided device documentation.
// ============================================================

// -------- Produal MIO --------

// These are placeholders until confirmed from the MIO
// datasheet/course material.

#define MIO_FAN_SPEED_REGISTER   0x0000
#define MIO_FAN_PULSE_REGISTER   0x0003

// -------- Vaisala GMP252 --------

#define GMP252_CO2_REGISTER      0x0000

// -------- Vaisala HMP60 --------

#define HMP60_TEMPERATURE_REGISTER  0x0001
#define HMP60_HUMIDITY_REGISTER     0x0002

// ============================================================
// MODBUS CRC16
// ============================================================

static uint16_t modbusCRC(
    const uint8_t *data,
    uint16_t length
)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

// ============================================================
// RS485 DIRECTION
// ============================================================

static void setRS485Transmit(bool transmit)
{
    digitalWrite(
        RS485_DE_RE_PIN,
        transmit ? HIGH : LOW
    );
}

// ============================================================
// MODBUS READ REGISTERS
// ============================================================

static bool modbusReadRegisters(
    uint8_t address,
    uint16_t startRegister,
    uint16_t registerCount,
    uint16_t *result
)
{
    if (result == nullptr)
    {
        return false;
    }

    if (rs485Mutex == nullptr)
    {
        return false;
    }

    if (xSemaphoreTake(
            rs485Mutex,
            pdMS_TO_TICKS(200)) != pdTRUE)
    {
        return false;
    }

    uint8_t request[8];

    request[0] = address;
    request[1] = 0x03;

    request[2] = (startRegister >> 8) & 0xFF;
    request[3] = startRegister & 0xFF;

    request[4] = (registerCount >> 8) & 0xFF;
    request[5] = registerCount & 0xFF;

    uint16_t crc = modbusCRC(
        request,
        6
    );

    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;

    while (RS485.available())
    {
        RS485.read();
    }

    setRS485Transmit(true);

    delayMicroseconds(100);

    RS485.write(
        request,
        sizeof(request)
    );

    RS485.flush();

    delayMicroseconds(100);

    setRS485Transmit(false);

    const uint16_t expectedLength =
        5 + registerCount * 2;

    uint8_t response[64];

    if (expectedLength > sizeof(response))
    {
        xSemaphoreGive(rs485Mutex);
        return false;
    }

    uint16_t received = 0;

    unsigned long startTime = millis();

    while (
        received < expectedLength &&
        millis() - startTime < 300
    )
    {
        if (RS485.available())
        {
            response[received++] =
                RS485.read();
        }
        else
        {
            delay(1);
        }
    }

    xSemaphoreGive(rs485Mutex);

    if (received != expectedLength)
    {
        return false;
    }

    if (response[0] != address)
    {
        return false;
    }

    if (response[1] != 0x03)
    {
        return false;
    }

    if (response[2] != registerCount * 2)
    {
        return false;
    }

    uint16_t receivedCRC =
        response[received - 2] |
        (response[received - 1] << 8);

    uint16_t calculatedCRC =
        modbusCRC(
            response,
            received - 2
        );

    if (receivedCRC != calculatedCRC)
    {
        return false;
    }

    for (uint16_t i = 0; i < registerCount; i++)
    {
        result[i] =
            ((uint16_t)response[3 + i * 2] << 8) |
            response[4 + i * 2];
    }

    return true;
}

// ============================================================
// MODBUS WRITE SINGLE REGISTER
// ============================================================

static bool modbusWriteRegister(
    uint8_t address,
    uint16_t registerAddress,
    uint16_t value
)
{
    if (rs485Mutex == nullptr)
    {
        return false;
    }

    if (xSemaphoreTake(
            rs485Mutex,
            pdMS_TO_TICKS(200)) != pdTRUE)
    {
        return false;
    }

    uint8_t request[8];

    request[0] = address;
    request[1] = 0x06;

    request[2] =
        (registerAddress >> 8) & 0xFF;

    request[3] =
        registerAddress & 0xFF;

    request[4] =
        (value >> 8) & 0xFF;

    request[5] =
        value & 0xFF;

    uint16_t crc =
        modbusCRC(
            request,
            6
        );

    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;

    while (RS485.available())
    {
        RS485.read();
    }

    setRS485Transmit(true);

    delayMicroseconds(100);

    RS485.write(
        request,
        sizeof(request)
    );

    RS485.flush();

    delayMicroseconds(100);

    setRS485Transmit(false);

    uint8_t response[8];

    uint16_t received = 0;

    unsigned long startTime = millis();

    while (
        received < 8 &&
        millis() - startTime < 300
    )
    {
        if (RS485.available())
        {
            response[received++] =
                RS485.read();
        }
        else
        {
            delay(1);
        }
    }

    xSemaphoreGive(rs485Mutex);

    if (received != 8)
    {
        return false;
    }

    uint16_t receivedCRC =
        response[6] |
        (response[7] << 8);

    uint16_t calculatedCRC =
        modbusCRC(
            response,
            6
        );

    if (receivedCRC != calculatedCRC)
    {
        return false;
    }

    return true;
}

// ============================================================
// READ MIO
// ============================================================

static bool readMioSensors(
    uint16_t &fanPulses
)
{
    uint16_t registers[1];

    if (!modbusReadRegisters(
            MIO_MODBUS_ADDRESS,
            MIO_FAN_PULSE_REGISTER,
            1,
            registers))
    {
        return false;
    }

    fanPulses = registers[0];

    return true;
}

// ============================================================
// READ GMP252
// ============================================================

static bool readGMP252(
    float &co2Ppm
)
{
    uint16_t registers[1];

    if (!modbusReadRegisters(
            GMP252_MODBUS_ADDRESS,
            GMP252_CO2_REGISTER,
            1,
            registers))
    {
        return false;
    }

    co2Ppm =
        static_cast<float>(
            registers[0]
        );

    return true;
}

// ============================================================
// READ HMP60
// ============================================================

static bool readHMP60(
    float &temperature,
    float &humidity
)
{
    uint16_t registers[2];

    if (!modbusReadRegisters(
            HMP60_MODBUS_ADDRESS,
            HMP60_TEMPERATURE_REGISTER,
            2,
            registers))
    {
        return false;
    }

    temperature =
        static_cast<float>(
            static_cast<int16_t>(
                registers[0]
            )
        ) * 0.1f;

    humidity =
        static_cast<float>(
            registers[1]
        ) * 0.1f;

    return true;
}

// ============================================================
// READ SDP610
// ============================================================

static bool readSDP610(
    float &pressurePa
)
{
    Wire.beginTransmission(
        SDP610_ADDRESS
    );

    // SDP610 measurement command.
    // Exact command sequence should be verified
    // against the course datasheet.
    uint8_t command[] =
    {
        0x36,
        0x2F
    };

    Wire.write(
        command,
        sizeof(command)
    );

    if (Wire.endTransmission() != 0)
    {
        return false;
    }

    delay(5);

    if (Wire.requestFrom(
            SDP610_ADDRESS,
            3) != 3)
    {
        return false;
    }

    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t crc = Wire.read();

    (void)crc;

    int16_t raw =
        static_cast<int16_t>(
            ((uint16_t)msb << 8) |
            lsb
        );

    // Placeholder scale.
    // Must be replaced with the scale factor
    // from the SDP610 course datasheet.
    pressurePa =
        static_cast<float>(raw);

    return true;
}

// ============================================================
// FAN SPEED
// ============================================================

bool setFanSpeed(uint8_t speed)
{
    if (speed > FAN_MAX)
    {
        speed = FAN_MAX;
    }

    // Convert 0-100% to 0-1000 representing
    // 0.0-10.0 V if the MIO register uses
    // tenths of a volt.
    //
    // The exact register/scaling MUST be verified
    // against the MIO datasheet.

    uint16_t outputValue =
        static_cast<uint16_t>(
            speed * 10
        );

    bool success =
        modbusWriteRegister(
            MIO_MODBUS_ADDRESS,
            MIO_FAN_SPEED_REGISTER,
            outputValue
        );

    if (success)
    {
        if (sensorDataMutex != nullptr &&
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(50)) == pdTRUE)
        {
            sharedSensorData.fanSpeed =
                speed;

            xSemaphoreGive(
                sensorDataMutex
            );
        }
    }

    return success;
}

// ============================================================
// CO2 VALVE
// ============================================================

void setCO2Valve(bool enabled)
{
    valveOpen = enabled;

    digitalWrite(
        CO2_VALVE_PIN,
        enabled ? HIGH : LOW
    );

    if (enabled)
    {
        valveOpenedAt = millis();

        Serial.println(
            "CO2 valve OPEN"
        );
    }
    else
    {
        Serial.println(
            "CO2 valve CLOSED"
        );
    }
}

// ============================================================
// START CO2 INJECTION
// ============================================================

static void startCO2Injection()
{
    if (controllerSafetyMode)
    {
        return;
    }

    unsigned long now = millis();

    if (
        lastInjectionTime != 0 &&
        now - lastInjectionTime <
            CO2_INJECTION_WAIT_MS
    )
    {
        return;
    }

    lastInjectionTime = now;

    setCO2Valve(true);
}

// ============================================================
// HANDLE CO2 VALVE TIMER
// ============================================================

static void updateCO2Valve()
{
    if (!valveOpen)
    {
        return;
    }

    if (
        millis() - valveOpenedAt >=
        CO2_VALVE_OPEN_TIME_MS
    )
    {
        setCO2Valve(false);
    }
}

// ============================================================
// SENSOR TASK
// ============================================================

void SensorTask(void *parameter)
{
    (void)parameter;

    Serial.println(
        "SensorTask started."
    );

    for (;;)
    {
        float co2 = 0.0f;
        float temperature = 0.0f;
        float humidity = 0.0f;
        float pressure = 0.0f;

        uint16_t fanPulses = 0;

        bool co2OK =
            readGMP252(co2);

        bool hmpOK =
            readHMP60(
                temperature,
                humidity
            );

        bool pressureOK =
            readSDP610(
                pressure
            );

        bool fanOK =
            readMioSensors(
                fanPulses
            );

        bool valid =
            co2OK &&
            hmpOK &&
            pressureOK &&
            fanOK;

        // ----------------------------------------------------
        // Fan pulse monitoring
        // ----------------------------------------------------

        if (fanOK)
        {
            if (fanPulses == 0)
            {
                if (
                    consecutiveZeroFanReads <
                    FAN_ZERO_READINGS_REQUIRED
                )
                {
                    consecutiveZeroFanReads++;
                }
            }
            else
            {
                consecutiveZeroFanReads = 0;
            }
        }

        // ----------------------------------------------------
        // Update shared data
        // ----------------------------------------------------

        if (
            sensorDataMutex != nullptr &&
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(100)) == pdTRUE
        )
        {
            if (co2OK)
            {
                sharedSensorData.co2Ppm =
                    co2;
            }

            if (hmpOK)
            {
                sharedSensorData.temperature =
                    temperature;

                sharedSensorData.humidity =
                    humidity;
            }

            if (pressureOK)
            {
                sharedSensorData.pressurePa =
                    pressure;
            }

            if (fanOK)
            {
                sharedSensorData.fanPulses =
                    fanPulses;
            }

            sharedSensorData.timestamp =
                millis();

            sharedSensorData.valid =
                valid;

            xSemaphoreGive(
                sensorDataMutex
            );
        }

        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_PERIOD_MS
            )
        );
    }
}

// ============================================================
// CONTROLLER TASK
// ============================================================

void ControllerTask(void *parameter)
{
    (void)parameter;

    Serial.println(
        "ControllerTask started."
    );

    for (;;)
    {
        // ----------------------------------------------------
        // Receive new setpoint
        // ----------------------------------------------------

        SetpointMessage message;

        if (
            setpointQueue != nullptr &&
            xQueueReceive(
                setpointQueue,
                &message,
                0) == pdTRUE
        )
        {
            currentSetpoint =
                constrain(
                    message.co2Setpoint,
                    CO2_USER_MIN,
                    CO2_USER_MAX
                );

            Serial.print(
                "New CO2 setpoint: "
            );

            Serial.print(
                currentSetpoint
            );

            Serial.println(
                " ppm"
            );
        }

        // ----------------------------------------------------
        // Update valve timer
        // ----------------------------------------------------

        updateCO2Valve();

        // ----------------------------------------------------
        // Get current sensor data
        // ----------------------------------------------------

        SensorData data;

        if (
            sensorDataMutex == nullptr ||
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(100)) != pdTRUE
        )
        {
            vTaskDelay(
                pdMS_TO_TICKS(
                    CONTROLLER_PERIOD_MS
                )
            );

            continue;
        }

        data =
            sharedSensorData;

        xSemaphoreGive(
            sensorDataMutex
        );

        // ----------------------------------------------------
        // Sensor invalid
        // ----------------------------------------------------

        if (!data.valid)
        {
            // Fail-safe:
            // No CO2 injection when sensor data is invalid.

            setCO2Valve(false);

            controllerSafetyMode = true;

            setFanSpeed(FAN_HIGH);

            vTaskDelay(
                pdMS_TO_TICKS(
                    CONTROLLER_PERIOD_MS
                )
            );

            continue;
        }

        // ----------------------------------------------------
        // SAFETY LIMIT
        // ----------------------------------------------------

        if (
            data.co2Ppm >=
            CO2_SAFETY_LIMIT
        )
        {
            controllerSafetyMode = true;

            // Never inject CO2 during safety mode.
            setCO2Valve(false);

            // Increase ventilation.
            setFanSpeed(FAN_MAX);
        }
        else
        {
            controllerSafetyMode = false;

            // ------------------------------------------------
            // CO2 below requested level
            // ------------------------------------------------

            if (
                data.co2Ppm <
                currentSetpoint
            )
            {
                // Do not ventilate unnecessarily.
                setFanSpeed(FAN_OFF);

                // Valve is controlled by the injection
                // timer and 30-second lockout.
                if (!valveOpen)
                {
                    startCO2Injection();
                }
            }

            // ------------------------------------------------
            // CO2 at or above requested level
            // ------------------------------------------------

            else
            {
                // Plants consume CO2 naturally.
                // No additional ventilation is required
                // below the safety limit.

                setCO2Valve(false);

                setFanSpeed(FAN_OFF);
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(
                CONTROLLER_PERIOD_MS
            )
        );
    }
}

// ============================================================
// UI TASK
// ============================================================

void UITask(void *parameter)
{
    (void)parameter;

    Serial.println(
        "UITask started."
    );

    uint16_t uiSetpoint =
        loadSetpoint();

    for (;;)
    {
        UIEvent event;

        while (
            uiEventQueue != nullptr &&
            xQueueReceive(
                uiEventQueue,
                &event,
                0) == pdTRUE
        )
        {
            if (
                event.type ==
                UIEventType::ENCODER_CW
            )
            {
                if (
                    uiSetpoint <
                    CO2_USER_MAX
                )
                {
                    uiSetpoint += 50;

                    if (
                        uiSetpoint >
                        CO2_USER_MAX
                    )
                    {
                        uiSetpoint =
                            CO2_USER_MAX;
                    }

                    saveSetpoint(
                        uiSetpoint
                    );

                    sendSetpointToController(
                        uiSetpoint
                    );
                }
            }
            else if (
                event.type ==
                UIEventType::ENCODER_CCW
            )
            {
                if (
                    uiSetpoint >
                    CO2_USER_MIN
                )
                {
                    if (uiSetpoint >= 50)
                    {
                        uiSetpoint -= 50;
                    }

                    if (
                        uiSetpoint <
                        CO2_USER_MIN
                    )
                    {
                        uiSetpoint =
                            CO2_USER_MIN;
                    }

                    saveSetpoint(
                        uiSetpoint
                    );

                    sendSetpointToController(
                        uiSetpoint
                    );
                }
            }
        }

        SensorData data;

        if (
            sensorDataMutex != nullptr &&
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(20)) == pdTRUE
        )
        {
            data =
                sharedSensorData;

            xSemaphoreGive(
                sensorDataMutex
            );
        }
        else
        {
            data =
                sharedSensorData;
        }

        updateDisplay(
            data,
            uiSetpoint,
            controllerSafetyMode
        );

        vTaskDelay(
            pdMS_TO_TICKS(
                UI_PERIOD_MS
            )
        );
    }
}

// ============================================================
// NETWORK TASK
// ============================================================

void NetworkTask(void *parameter)
{
    (void)parameter;

    Serial.println(
        "NetworkTask started."
    );

    unsigned long lastNetworkSend = 0;

    for (;;)
    {
        if (
            millis() - lastNetworkSend >=
            NETWORK_PERIOD_MS
        )
        {
            lastNetworkSend =
                millis();

            if (
                WiFi.status() !=
                WL_CONNECTED
            )
            {
                connectWiFi();
            }

            if (
                WiFi.status() ==
                WL_CONNECTED
            )
            {
                SensorData data;

                if (
                    sensorDataMutex != nullptr &&
                    xSemaphoreTake(
                        sensorDataMutex,
                        pdMS_TO_TICKS(100)) == pdTRUE
                )
                {
                    data =
                        sharedSensorData;

                    xSemaphoreGive(
                        sensorDataMutex
                    );
                }

                // ------------------------------------------------
                // ThingSpeak REST API
                // ------------------------------------------------

                if (data.valid)
                {
                    HTTPClient http;

                    String url =
                        "http://api.thingspeak.com/update?api_key=";

                    url +=
                        THINGSPEAK_API_KEY;

                    url +=
                        "&field1=";

                    url +=
                        String(
                            data.co2Ppm,
                            1
                        );

                    url +=
                        "&field2=";

                    url +=
                        String(
                            data.temperature,
                            1
                        );

                    url +=
                        "&field3=";

                    url +=
                        String(
                            data.humidity,
                            1
                        );

                    url +=
                        "&field4=";

                    url +=
                        String(
                            data.pressurePa,
                            1
                        );

                    http.begin(url);

                    int response =
                        http.GET();

                    Serial.print(
                        "Cloud response: "
                    );

                    Serial.println(
                        response
                    );

                    http.end();
                }
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}

// ============================================================
// SETPOINT STORAGE
// ============================================================

uint16_t loadSetpoint()
{
    preferences.begin(
        "co2-controller",
        true
    );

    uint16_t value =
        preferences.getUShort(
            "setpoint",
            DEFAULT_SETPOINT
        );

    preferences.end();

    if (
        value < CO2_USER_MIN ||
        value > CO2_USER_MAX
    )
    {
        value =
            DEFAULT_SETPOINT;
    }

    return value;
}

// ============================================================
// SAVE SETPOINT
// ============================================================

void saveSetpoint(
    uint16_t value
)
{
    value =
        constrain(
            value,
            CO2_USER_MIN,
            CO2_USER_MAX
        );

    preferences.begin(
        "co2-controller",
        false
    );

    preferences.putUShort(
        "setpoint",
        value
    );

    preferences.end();
}

// ============================================================
// SEND SETPOINT TO CONTROLLER
// ============================================================

void sendSetpointToController(
    uint16_t setpoint
)
{
    if (setpointQueue == nullptr)
    {
        currentSetpoint =
            setpoint;

        return;
    }

    SetpointMessage message;

    message.co2Setpoint =
        constrain(
            setpoint,
            CO2_USER_MIN,
            CO2_USER_MAX
        );

    xQueueSend(
        setpointQueue,
        &message,
        pdMS_TO_TICKS(50)
    );
}

// ============================================================
// ENCODER INTERRUPT
// ============================================================

void IRAM_ATTR encoderISR()
{
    static unsigned long lastInterrupt = 0;

    unsigned long now =
        millis();

    if (
        now - lastInterrupt < 5
    )
    {
        return;
    }

    lastInterrupt =
        now;

    bool a =
        digitalRead(
            ENCODER_A_PIN
        );

    bool b =
        digitalRead(
            ENCODER_B_PIN
        );

    UIEvent event;

    if (a == b)
    {
        event.type =
            UIEventType::ENCODER_CW;
    }
    else
    {
        event.type =
            UIEventType::ENCODER_CCW;
    }

    BaseType_t higherPriorityTaskWoken =
        pdFALSE;

    if (uiEventQueue != nullptr)
    {
        xQueueSendFromISR(
            uiEventQueue,
            &event,
            &higherPriorityTaskWoken
        );
    }

    if (higherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

// ============================================================
// BUTTON INTERRUPT
// ============================================================

void IRAM_ATTR buttonISR()
{
    static unsigned long lastInterrupt = 0;

    unsigned long now =
        millis();

    if (
        now - lastInterrupt < 200
    )
    {
        return;
    }

    lastInterrupt =
        now;

    UIEvent event;

    event.type =
        UIEventType::BUTTON_PRESS;

    BaseType_t higherPriorityTaskWoken =
        pdFALSE;

    if (uiEventQueue != nullptr)
    {
        xQueueSendFromISR(
            uiEventQueue,
            &event,
            &higherPriorityTaskWoken
        );
    }

    if (higherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

// ============================================================
// OLED DISPLAY
// ============================================================

void updateDisplay(
    const SensorData& data,
    uint16_t setpoint,
    bool safetyMode
)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(
        SSD1306_WHITE
    );

    display.setCursor(0, 0);

    display.print(
        "CO2: "
    );

    display.print(
        data.co2Ppm,
        0
    );

    display.println(
        " ppm"
    );

    display.print(
        "Set: "
    );

    display.print(
        setpoint
    );

    display.println(
        " ppm"
    );

    display.print(
        "Temp: "
    );

    display.print(
        data.temperature,
        1
    );

    display.println(
        " C"
    );

    display.print(
        "RH: "
    );

    display.print(
        data.humidity,
        1
    );

    display.println(
        " %"
    );

    display.print(
        "Press: "
    );

    display.print(
        data.pressurePa,
        1
    );

    display.println(
        " Pa"
    );

    display.print(
        "Fan: "
    );

    display.print(
        data.fanSpeed
    );

    display.println(
        "%"
    );

    if (safetyMode)
    {
        display.println(
            "SAFETY MODE!"
        );
    }
    else if (valveOpen)
    {
        display.println(
            "CO2 INJECTION"
        );
    }

    display.display();
}

// ============================================================
// WIFI
// ============================================================

bool connectWiFi()
{
    if (
        strlen(WIFI_SSID) == 0
    )
    {
        return false;
    }

    if (
        strcmp(
            WIFI_SSID,
            "YOUR_WIFI_NAME"
        ) == 0
    )
    {
        Serial.println(
            "WiFi credentials not configured."
        );

        return false;
    }

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    Serial.print(
        "Connecting to WiFi"
    );

    unsigned long start =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - start < 10000
    )
    {
        delay(250);

        Serial.print(".");
    }

    Serial.println();

    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {
        Serial.print(
            "WiFi connected. IP: "
        );

        Serial.println(
            WiFi.localIP()
        );

        return true;
    }

    Serial.println(
        "WiFi connection failed."
    );

    return false;
}

