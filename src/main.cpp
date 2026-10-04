#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "app.h"

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
    // CO2 VALVE
    // --------------------------------------------------------

    pinMode(CO2_VALVE_PIN, OUTPUT);

    digitalWrite(
        CO2_VALVE_PIN,
        LOW
    );

    // --------------------------------------------------------
    // ENCODER
    // --------------------------------------------------------

    pinMode(
        ENCODER_A_PIN,
        INPUT_PULLUP
    );

    pinMode(
        ENCODER_B_PIN,
        INPUT_PULLUP
    );

    // --------------------------------------------------------
    // BUTTON
    // --------------------------------------------------------

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    // --------------------------------------------------------
    // RS485 DIRECTION
    // --------------------------------------------------------

    pinMode(
        RS485_DE_RE_PIN,
        OUTPUT
    );

    digitalWrite(
        RS485_DE_RE_PIN,
        LOW
    );

    // --------------------------------------------------------
    // RS485
    // --------------------------------------------------------

    RS485.begin(
        RS485_BAUDRATE,
        SERIAL_8N1,
        RS485_RX_PIN,
        RS485_TX_PIN
    );

    Serial.println(
        "RS485 initialized."
    );

    // --------------------------------------------------------
    // I2C
    // --------------------------------------------------------

    Wire.begin(
        OLED_SDA_PIN,
        OLED_SCL_PIN
    );

    Serial.println(
        "I2C initialized."
    );

    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS))
    {
        Serial.println(
            "OLED initialization failed!"
        );
    }
    else
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

        display.println(
            "CO2 Controller"
        );

        display.println();

        display.println(
            "Starting..."
        );

        display.display();

        Serial.println(
            "OLED initialized."
        );
    }

    // --------------------------------------------------------
    // MUTEXES
    // --------------------------------------------------------

    sensorDataMutex =
        xSemaphoreCreateMutex();

    rs485Mutex =
        xSemaphoreCreateMutex();

    if (
        sensorDataMutex == nullptr ||
        rs485Mutex == nullptr
    )
    {
        Serial.println(
            "ERROR: Mutex creation failed!"
        );

        while (true)
        {
            delay(1000);
        }
    }

    // --------------------------------------------------------
    // QUEUES
    // --------------------------------------------------------

    uiEventQueue =
        xQueueCreate(
            UI_QUEUE_LENGTH,
            sizeof(UIEvent)
        );

    setpointQueue =
        xQueueCreate(
            SETPOINT_QUEUE_LENGTH,
            sizeof(SetpointMessage)
        );

    if (
        uiEventQueue == nullptr ||
        setpointQueue == nullptr
    )
    {
        Serial.println(
            "ERROR: Queue creation failed!"
        );

        while (true)
        {
            delay(1000);
        }
    }

    // --------------------------------------------------------
    // INTERRUPTS
    // --------------------------------------------------------

    attachInterrupt(
        digitalPinToInterrupt(
            ENCODER_A_PIN
        ),
        encoderISR,
        CHANGE
    );

    attachInterrupt(
        digitalPinToInterrupt(
            BUTTON_PIN
        ),
        buttonISR,
        FALLING
    );

    // --------------------------------------------------------
    // SENSOR TASK
    // --------------------------------------------------------

    BaseType_t sensorTaskResult =
        xTaskCreate(
            SensorTask,
            "Sensor Task",
            4096,
            nullptr,
            2,
            nullptr
        );

    // --------------------------------------------------------
    // CONTROLLER TASK
    // --------------------------------------------------------

    BaseType_t controllerTaskResult =
        xTaskCreate(
            ControllerTask,
            "Controller Task",
            4096,
            nullptr,
            3,
            nullptr
        );

    // --------------------------------------------------------
    // UI TASK
    // --------------------------------------------------------

    BaseType_t uiTaskResult =
        xTaskCreate(
            UITask,
            "UI Task",
            4096,
            nullptr,
            1,
            nullptr
        );

    // --------------------------------------------------------
    // NETWORK TASK
    // --------------------------------------------------------

    BaseType_t networkTaskResult =
        xTaskCreate(
            NetworkTask,
            "Network Task",
            8192,
            nullptr,
            1,
            nullptr
        );

    // --------------------------------------------------------
    // CHECK TASK CREATION
    // --------------------------------------------------------

    if (
        sensorTaskResult != pdPASS ||
        controllerTaskResult != pdPASS ||
        uiTaskResult != pdPASS ||
        networkTaskResult != pdPASS
    )
    {
        Serial.println(
            "ERROR: One or more tasks failed!"
        );

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println(
        "All FreeRTOS tasks started."
    );

    Serial.println(
        "CO2 controller ready."
    );

    Serial.println(
        "================================"
    );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}