#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "app.h"

// RS485 on ESP32 UART2
HardwareSerial RS485(2);

// OLED
Adafruit_SSD1306 display(
    128,
    64,
    &Wire,
    -1
);


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // Serial
    // --------------------------------------------------------

    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("CO2 CONTROL SYSTEM");
    Serial.println("Starting...");
    Serial.println("================================");


    // --------------------------------------------------------
    // CO2 valve
    // --------------------------------------------------------

    pinMode(CO2_VALVE_PIN, OUTPUT);
    digitalWrite(CO2_VALVE_PIN, LOW);


    // --------------------------------------------------------
    // Encoder + button
    // --------------------------------------------------------

    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PIN, INPUT_PULLUP);


    // --------------------------------------------------------
    // RS485
    // --------------------------------------------------------

    pinMode(RS485_DE_RE_PIN, OUTPUT);
    digitalWrite(RS485_DE_RE_PIN, LOW);

    RS485.begin(
        RS485_BAUDRATE,
        SERIAL_8N1,
        RS485_RX_PIN,
        RS485_TX_PIN
    );


    // --------------------------------------------------------
    // I2C / OLED
    // --------------------------------------------------------

    Wire.begin(
        OLED_SDA_PIN,
        OLED_SCL_PIN
    );

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS))
    {
        Serial.println("OLED initialization failed!");
    }
    else
    {
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);
        display.println("CO2 Controller");
        display.println();
        display.println("Starting...");

        display.display();
    }


    // --------------------------------------------------------
    // Mutexes
    // --------------------------------------------------------

    sensorDataMutex = xSemaphoreCreateMutex();
    rs485Mutex = xSemaphoreCreateMutex();

    if (sensorDataMutex == nullptr ||
        rs485Mutex == nullptr)
    {
        Serial.println("ERROR: Mutex creation failed");

        while (true)
        {
            delay(1000);
        }
    }


    // --------------------------------------------------------
    // Queues
    // --------------------------------------------------------

    uiEventQueue = xQueueCreate(
        UI_QUEUE_LENGTH,
        sizeof(UIEvent)
    );

    setpointQueue = xQueueCreate(
        SETPOINT_QUEUE_LENGTH,
        sizeof(SetpointMessage)
    );

    if (uiEventQueue == nullptr ||
        setpointQueue == nullptr)
    {
        Serial.println("ERROR: Queue creation failed");

        while (true)
        {
            delay(1000);
        }
    }


    // --------------------------------------------------------
    // Interrupts
    // --------------------------------------------------------

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_A_PIN),
        encoderISR,
        CHANGE
    );

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonISR,
        FALLING
    );


    // --------------------------------------------------------
    // FreeRTOS tasks
    // --------------------------------------------------------

    xTaskCreate(
        SensorTask,
        "Sensor Task",
        4096,
        nullptr,
        2,
        nullptr
    );

    xTaskCreate(
        ControllerTask,
        "Controller Task",
        4096,
        nullptr,
        3,
        nullptr
    );

    xTaskCreate(
        UITask,
        "UI Task",
        4096,
        nullptr,
        1,
        nullptr
    );

    xTaskCreate(
        NetworkTask,
        "Network Task",
        8192,
        nullptr,
        1,
        nullptr
    );


    Serial.println("All FreeRTOS tasks started.");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}