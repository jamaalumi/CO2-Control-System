#include "app.h"
#include "config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

HardwareSerial RS485(1);

Preferences preferences;

Adafruit_SSD1306 display(
    OLED_WIDTH,
    OLED_HEIGHT,
    &Wire,
    -1
);

SemaphoreHandle_t sensorDataMutex = nullptr;

SemaphoreHandle_t rs485Mutex = nullptr;

QueueHandle_t uiEventQueue = nullptr;

QueueHandle_t setpointQueue = nullptr;

SensorData sharedSensorData =
{
    0.0f,
    0.0f,
    0.0f,
    0,
    0,
    false
};

static uint16_t currentSetpoint = 1500;

static bool controllerSafetyMode = false;

static void rs485TransmitMode()
{
    digitalWrite(
        RS485_DE_RE_PIN,
        HIGH
    );

    delayMicroseconds(100);
}

static void rs485ReceiveMode()
{
    delayMicroseconds(100);

    digitalWrite(
        RS485_DE_RE_PIN,
        LOW
    );
}

static uint16_t modbusCRC(
    const uint8_t *buffer,
    uint16_t length
)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t pos = 0; pos < length; pos++)
    {
        crc ^= buffer[pos];

        for (uint8_t i = 0; i < 8; i++)
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

bool modbusReadHoldingRegisters(
    uint8_t slaveAddress,
    uint16_t startRegister,
    uint16_t quantity,
    uint16_t *buffer
)
{
    if (quantity == 0)
    {
        return false;
    }

    if (quantity > 16)
    {
        return false;
    }

    uint8_t request[8];

    request[0] = slaveAddress;

    request[1] = 0x03;

    request[2] =
        (startRegister >> 8) & 0xFF;

    request[3] =
        startRegister & 0xFF;

    request[4] =
        (quantity >> 8) & 0xFF;

    request[5] =
        quantity & 0xFF;

    uint16_t crc =
        modbusCRC(
            request,
            6
        );

    request[6] =
        crc & 0xFF;

    request[7] =
        (crc >> 8) & 0xFF;

    while (RS485.available())
    {
        RS485.read();
    }

    rs485TransmitMode();

    RS485.write(
        request,
        sizeof(request)
    );

    RS485.flush();

    rs485ReceiveMode();

    const uint16_t expectedLength =
        5 + quantity * 2;

    uint8_t response[64];

    uint16_t index = 0;

    unsigned long startTime =
        millis();

    while (
        index < expectedLength &&
        millis() - startTime < 500
    )
    {
        if (RS485.available())
        {
            response[index++] =
                RS485.read();
        }
        else
        {
            delay(1);
        }
    }

    if (index != expectedLength)
    {
        return false;
    }

    if (response[0] != slaveAddress)
    {
        return false;
    }

    if (response[1] != 0x03)
    {
        return false;
    }

    if (
        response[2] !=
        quantity * 2
    )
    {
        return false;
    }

    uint16_t receivedCRC =
        response[expectedLength - 2] |
        (
            response[expectedLength - 1]
            << 8
        );

    uint16_t calculatedCRC =
        modbusCRC(
            response,
            expectedLength - 2
        );

    if (receivedCRC != calculatedCRC)
    {
        return false;
    }

    for (uint16_t i = 0; i < quantity; i++)
    {
        buffer[i] =
            (
                response[3 + i * 2]
                << 8
            )
            |
            response[4 + i * 2];
    }

    return true;
}

bool modbusWriteRegister(
    uint8_t slaveAddress,
    uint16_t registerAddress,
    uint16_t value
)
{
    uint8_t request[8];

    request[0] = slaveAddress;

    request[1] = 0x06;

    request[2] =
        registerAddress >> 8;

    request[3] =
        registerAddress & 0xFF;

    request[4] =
        value >> 8;

    request[5] =
        value & 0xFF;

    uint16_t crc =
        modbusCRC(
            request,
            6
        );

    request[6] =
        crc & 0xFF;

    request[7] =
        crc >> 8;

    while (RS485.available())
    {
        RS485.read();
    }

    rs485TransmitMode();

    RS485.write(
        request,
        sizeof(request)
    );

    RS485.flush();

    rs485ReceiveMode();

    uint8_t response[8];

    uint16_t index = 0;

    unsigned long startTime =
        millis();

    while (
        index < 8 &&
        millis() - startTime < 500
    )
    {
        if (RS485.available())
        {
            response[index++] =
                RS485.read();
        }
        else
        {
            delay(1);
        }
    }

    if (index != 8)
    {
        return false;
    }

    uint16_t receivedCRC =
        response[6] |
        (
            response[7] << 8
        );

    uint16_t calculatedCRC =
        modbusCRC(
            response,
            6
        );

    if (receivedCRC != calculatedCRC)
    {
        return false;
    }

    for (uint8_t i = 0; i < 6; i++)
    {
        if (response[i] != request[i])
        {
            return false;
        }
    }

    return true;
}

bool readMioSensors(
    SensorData &data
)
{
    uint16_t value;

    if (
        !modbusReadHoldingRegisters(
            MIO_MODBUS_ADDRESS,
            REG_CO2_PPM,
            1,
            &value
        )
    )
    {
        return false;
    }

    data.co2Ppm =
        value * CO2_SCALE;

    if (
        !modbusReadHoldingRegisters(
            MIO_MODBUS_ADDRESS,
            REG_TEMPERATURE,
            1,
            &value
        )
    )
    {
        return false;
    }

    data.temperature =
        value * TEMPERATURE_SCALE;

    if (
        !modbusReadHoldingRegisters(
            MIO_MODBUS_ADDRESS,
            REG_HUMIDITY,
            1,
            &value
        )
    )
    {
        return false;
    }

    data.humidity =
        value * HUMIDITY_SCALE;

    if (
        !modbusReadHoldingRegisters(
            MIO_MODBUS_ADDRESS,
            REG_FAN_PULSES,
            1,
            &value
        )
    )
    {
        return false;
    }

    data.fanPulses = value;

    data.timestamp = millis();

    data.valid = true;

    return true;
}

bool setFanSpeed(
    uint8_t percentage
)
{
    if (percentage > 100)
    {
        percentage = 100;
    }

    uint16_t value = percentage;

    bool result =
        modbusWriteRegister(
            MIO_MODBUS_ADDRESS,
            REG_FAN_SPEED,
            value
        );

    Serial.print(
        "Fan command: "
    );

    Serial.print(
        percentage
    );

    Serial.println("%");

    return result;
}

void setCO2Valve(
    bool enabled
)
{
    digitalWrite(
        CO2_VALVE_PIN,
        enabled
            ? HIGH
            : LOW
    );

    Serial.print(
        "CO2 valve: "
    );

    Serial.println(
        enabled
            ? "ON"
            : "OFF"
    );
}

void SensorTask(
    void *parameter
)
{
    (void)parameter;

    SensorData localData;

    while (true)
    {
        if (
            xSemaphoreTake(
                rs485Mutex,
                pdMS_TO_TICKS(300)
            )
            == pdTRUE
        )
        {
            bool success =
                readMioSensors(
                    localData
                );

            xSemaphoreGive(
                rs485Mutex
            );

            if (success)
            {
                if (
                    xSemaphoreTake(
                        sensorDataMutex,
                        pdMS_TO_TICKS(100)
                    )
                    == pdTRUE
                )
                {
                    sharedSensorData =
                        localData;

                    xSemaphoreGive(
                        sensorDataMutex
                    );
                }
            }
            else
            {
                Serial.println(
                    "Sensor Modbus read failed"
                );
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_PERIOD_MS
            )
        );
    }
}

void ControllerTask(
    void *parameter
)
{
    (void)parameter;

    SensorData localData;

    uint16_t setpoint =
        DEFAULT_SETPOINT;

    SetpointMessage message;

    while (true)
    {
        while (
            xQueueReceive(
                setpointQueue,
                &message,
                0
            )
            == pdTRUE
        )
        {
            uint16_t requested =
                message.co2Setpoint;

            if (
                requested >
                CO2_USER_MAX
            )
            {
                requested =
                    CO2_USER_MAX;
            }

            if (
                requested <
                CO2_USER_MIN
            )
            {
                requested =
                    CO2_USER_MIN;
            }

            setpoint =
                requested;

            currentSetpoint =
                setpoint;

            Serial.print(
                "Controller setpoint: "
            );

            Serial.println(
                setpoint
            );
        }

        if (
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(100)
            )
            == pdTRUE
        )
        {
            localData =
                sharedSensorData;

            xSemaphoreGive(
                sensorDataMutex
            );
        }

        if (!localData.valid)
        {
            Serial.println(
                "Invalid sensor data!"
            );

            controllerSafetyMode =
                true;

            setCO2Valve(false);

            if (
                xSemaphoreTake(
                    rs485Mutex,
                    pdMS_TO_TICKS(200)
                )
                == pdTRUE
            )
            {
                setFanSpeed(
                    FAN_MAX
                );

                xSemaphoreGive(
                    rs485Mutex
                );
            }

            vTaskDelay(
                pdMS_TO_TICKS(
                    CONTROLLER_PERIOD_MS
                )
            );

            continue;
        }

        float co2 =
            localData.co2Ppm;

        if (
            co2 >=
            CO2_SAFETY_LIMIT
        )
        {
            controllerSafetyMode =
                true;

            Serial.println(
                "!!! CO2 SAFETY LIMIT !!!"
            );

            setCO2Valve(false);

            if (
                xSemaphoreTake(
                    rs485Mutex,
                    pdMS_TO_TICKS(200)
                )
                == pdTRUE
            )
            {
                setFanSpeed(
                    FAN_MAX
                );

                xSemaphoreGive(
                    rs485Mutex
                );
            }
        }
        else
        {
            controllerSafetyMode =
                false;

            if (
                co2 >
                setpoint
            )
            {
                setCO2Valve(false);

                uint8_t fanSpeed =
                    FAN_HIGH;

                if (
                    xSemaphoreTake(
                        rs485Mutex,
                        pdMS_TO_TICKS(200)
                    )
                    == pdTRUE
                )
                {
                    setFanSpeed(
                        fanSpeed
                    );

                    xSemaphoreGive(
                        rs485Mutex
                    );
                }
            }
            else
            {
                setCO2Valve(true);

                if (
                    xSemaphoreTake(
                        rs485Mutex,
                        pdMS_TO_TICKS(200)
                    )
                    == pdTRUE
                )
                {
                    setFanSpeed(
                        FAN_LOW
                    );

                    xSemaphoreGive(
                        rs485Mutex
                    );
                }
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(
                CONTROLLER_PERIOD_MS
            )
        );
    }
}

uint16_t loadSetpoint()
{
    preferences.begin(
        "co2-system",
        true
    );

    uint16_t value =
        preferences.getUShort(
            "setpoint",
            DEFAULT_SETPOINT
        );

    preferences.end();

    if (
        value >
        CO2_USER_MAX
    )
    {
        value =
            CO2_USER_MAX;
    }

    if (
        value <
        CO2_USER_MIN
    )
    {
        value =
            CO2_USER_MIN;
    }

    return value;
}

void saveSetpoint(
    uint16_t setpoint
)
{
    if (
        setpoint >
        CO2_USER_MAX
    )
    {
        setpoint =
            CO2_USER_MAX;
    }

    if (
        setpoint <
        CO2_USER_MIN
    )
    {
        setpoint =
            CO2_USER_MIN;
    }

    preferences.begin(
        "co2-system",
        false
    );

    preferences.putUShort(
        "setpoint",
        setpoint
    );

    preferences.end();

    Serial.print(
        "Saved setpoint: "
    );

    Serial.println(
        setpoint
    );
}

void sendSetpointToController(
    uint16_t setpoint
)
{
    SetpointMessage message;

    message.co2Setpoint =
        setpoint;

    xQueueSend(
        setpointQueue,
        &message,
        pdMS_TO_TICKS(100)
    );
}

void IRAM_ATTR encoderISR()
{
    BaseType_t higherPriorityTaskWoken =
        pdFALSE;

    UIEvent event;

    bool a =
        digitalRead(
            ENCODER_A_PIN
        );

    bool b =
        digitalRead(
            ENCODER_B_PIN
        );

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

    xQueueSendFromISR(
        uiEventQueue,
        &event,
        &higherPriorityTaskWoken
    );

    if (
        higherPriorityTaskWoken
    )
    {
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR buttonISR()
{
    BaseType_t higherPriorityTaskWoken =
        pdFALSE;

    UIEvent event;

    event.type =
        UIEventType::BUTTON_PRESS;

    xQueueSendFromISR(
        uiEventQueue,
        &event,
        &higherPriorityTaskWoken
    );

    if (
        higherPriorityTaskWoken
    )
    {
        portYIELD_FROM_ISR();
    }
}

void updateDisplay(
    const SensorData &data,
    uint16_t setpoint,
    bool safetyMode
)
{
    display.clearDisplay();

    display.setTextSize(1);

    display.setTextColor(
        SSD1306_WHITE
    );

    display.setCursor(
        0,
        0
    );

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
        "Limit: "
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
        "Hum: "
    );

    display.print(
        data.humidity,
        1
    );

    display.println(
        " %"
    );

    display.print(
        "Pulses: "
    );

    display.println(
        data.fanPulses
    );

    if (safetyMode)
    {
        display.println();

        display.println(
            "!!! SAFETY !!!"
        );
    }

    display.display();
}

void UITask(
    void *parameter
)
{
    (void)parameter;

    uint16_t setpoint =
        loadSetpoint();

    currentSetpoint =
        setpoint;

    sendSetpointToController(
        setpoint
    );

    SensorData localData;

    UIEvent event;

    unsigned long lastButtonTime =
        0;

    while (true)
    {
        if (
            xQueueReceive(
                uiEventQueue,
                &event,
                pdMS_TO_TICKS(
                    UI_PERIOD_MS
                )
            )
            == pdTRUE
        )
        {
            if (
                event.type ==
                UIEventType::ENCODER_CW
            )
            {
                if (
                    setpoint <
                    CO2_USER_MAX
                )
                {
                    setpoint += 50;
                }

                if (
                    setpoint >
                    CO2_USER_MAX
                )
                {
                    setpoint =
                        CO2_USER_MAX;
                }

                Serial.print(
                    "UI setpoint: "
                );

                Serial.println(
                    setpoint
                );
            }

            else if (
                event.type ==
                UIEventType::ENCODER_CCW
            )
            {
                if (
                    setpoint >
                    CO2_USER_MIN
                )
                {
                    setpoint -= 50;
                }

                if (
                    setpoint <
                    CO2_USER_MIN
                )
                {
                    setpoint =
                        CO2_USER_MIN;
                }

                Serial.print(
                    "UI setpoint: "
                );

                Serial.println(
                    setpoint
                );
            }

            else if (
                event.type ==
                UIEventType::BUTTON_PRESS
            )
            {
                unsigned long now =
                    millis();

                if (
                    now -
                    lastButtonTime
                    >
                    300
                )
                {
                    lastButtonTime =
                        now;

                    Serial.println(
                        "UI button pressed"
                    );

                    saveSetpoint(
                        setpoint
                    );

                    sendSetpointToController(
                        setpoint
                    );
                }
            }
        }

        if (
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(50)
            )
            == pdTRUE
        )
        {
            localData =
                sharedSensorData;

            xSemaphoreGive(
                sensorDataMutex
            );
        }

        updateDisplay(
            localData,
            setpoint,
            controllerSafetyMode
        );
    }
}

static bool connectWiFi()
{
    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {
        return true;
    }

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    Serial.print(
        "Connecting WiFi"
    );

    unsigned long start =
        millis();

    while (
        WiFi.status() !=
        WL_CONNECTED
        &&
        millis() - start < 10000
    )
    {
        Serial.print(
            "."
        );

        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }

    Serial.println();

    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {
        Serial.println(
            "WiFi connected"
        );

        Serial.println(
            WiFi.localIP()
        );

        return true;
    }

    Serial.println(
        "WiFi connection failed"
    );

    return false;
}

void NetworkTask(
    void *parameter
)
{
    (void)parameter;

    SensorData localData;

    connectWiFi();

    while (true)
    {
        if (
            WiFi.status() !=
            WL_CONNECTED
        )
        {
            connectWiFi();
        }

        if (
            xSemaphoreTake(
                sensorDataMutex,
                pdMS_TO_TICKS(100)
            )
            == pdTRUE
        )
        {
            localData =
                sharedSensorData;

            xSemaphoreGive(
                sensorDataMutex
            );
        }

        if (
            WiFi.status() ==
            WL_CONNECTED
        )
        {
            HTTPClient http;

            String url =
                "http://api.thingspeak.com/update";

            String parameters =
                "?api_key=";

            parameters +=
                THINGSPEAK_API_KEY;

            parameters +=
                "&field1=";

            parameters +=
                String(
                    localData.co2Ppm,
                    1
                );

            parameters +=
                "&field2=";

            parameters +=
                String(
                    localData.temperature,
                    1
                );

            parameters +=
                "&field3=";

            parameters +=
                String(
                    localData.humidity,
                    1
                );

            parameters +=
                "&field4=";

            parameters +=
                String(
                    localData.fanPulses
                );

            String fullURL =
                url +
                parameters;

            http.begin(
                fullURL
            );

            int responseCode =
                http.GET();

            Serial.print(
                "ThingSpeak update: "
            );

            Serial.println(
                responseCode
            );

            http.end();

            HTTPClient readHttp;

            String readURL =
                "https://api.thingspeak.com/channels/" +
                String(THINGSPEAK_CHANNEL_ID) +
                "/fields/5/last.txt";

            if (
                strlen(
                    THINGSPEAK_READ_API_KEY
                )
                > 0
            )
            {
                readURL +=
                    "?api_key=";

                readURL +=
                    THINGSPEAK_READ_API_KEY;
            }

            readHttp.begin(
                readURL
            );

            int readCode =
                readHttp.GET();

            if (
                readCode == 200
            )
            {
                String response =
                    readHttp.getString();

                response.trim();

                if (
                    response.length()
                    > 0
                )
                {
                    int cloudSetpoint =
                        response.toInt();

                    if (
                        cloudSetpoint >=
                        CO2_USER_MIN
                        &&
                        cloudSetpoint <=
                        CO2_USER_MAX
                    )
                    {
                        SetpointMessage message;

                        message.co2Setpoint =
                            cloudSetpoint;

                        xQueueSend(
                            setpointQueue,
                            &message,
                            0
                        );

                        Serial.print(
                            "Cloud setpoint: "
                        );

                        Serial.println(
                            cloudSetpoint
                        );
                    }
                    else
                    {
                        Serial.println(
                            "Cloud setpoint rejected"
                        );
                    }
                }
            }

            readHttp.end();
        }

        vTaskDelay(
            pdMS_TO_TICKS(
                NETWORK_PERIOD_MS
            )
        );
    }
}