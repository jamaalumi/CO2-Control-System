#include <Arduino.h>

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
    Serial.println(" Greenhouse CO2 Controller");
    Serial.println("================================");

    // --------------------------------------------------------
    // RS485
    // --------------------------------------------------------

    RS485.begin(
        RS485_BAUDRATE,
        SERIAL_8N1,
        RS485_RX_PIN,
        RS485_TX_PIN
    );

    pinMode(RS485_DE_RE_PIN, OUTPUT);

    digitalWrite(
        RS485_DE_RE_PIN,
        LOW
    );

    Serial.println("RS485 initialized.");

    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    Wire.begin(
        OLED_SDA_PIN,
        OLED_SCL_PIN
    );

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDRESS))
    {
        Serial.println("OLED initialization failed.");
    }
    else
    {
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        display.setCursor(0, 0);
        display.println("CO2 Controller");

        display.setCursor(0, 16);
        display.println("Starting...");

        display.display();
    }

    // --------------------------------------------------------
    // CO2 VALVE
    // --------------------------------------------------------

    pinMode(
        CO2_VALVE_PIN,
        OUTPUT
    );

    // Valve must always start OFF.
    setCO2Valve(false);

    // --------------------------------------------------------
    // ROTARY ENCODER
    // --------------------------------------------------------

    pinMode(
        ENCODER_A_PIN,
        INPUT_PULLUP
    );

    pinMode(
        ENCODER_B_PIN,
        INPUT_PULLUP
    );

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

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
    // LOAD SAVED SETPOINT
    // --------------------------------------------------------

    uint16_t setpoint = loadSetpoint();

    Serial.print("CO2 setpoint: ");
    Serial.print(setpoint);
    Serial.println(" ppm");

    // Send initial setpoint to controller queue.
    sendSetpointToController(setpoint);

    // --------------------------------------------------------
    // CREATE FREE RTOS OBJECTS
    // --------------------------------------------------------

    sensorDataMutex = xSemaphoreCreateMutex();

    rs485Mutex = xSemaphoreCreateMutex();

    uiEventQueue = xQueueCreate(
        UI_QUEUE_LENGTH,
        sizeof(UIEvent)
    );

    setpointQueue = xQueueCreate(
        SETPOINT_QUEUE_LENGTH,
        sizeof(SetpointMessage)
    );

    if (sensorDataMutex == nullptr ||
        rs485Mutex == nullptr ||
        uiEventQueue == nullptr ||
        setpointQueue == nullptr)
    {
        Serial.println("ERROR: FreeRTOS object creation failed.");

        while (true)
        {
            delay(1000);
        }
    }

    // --------------------------------------------------------
    // INITIAL SHARED SENSOR DATA
    // --------------------------------------------------------

    sharedSensorData.co2Ppm = 0.0f;
    sharedSensorData.temperature = 0.0f;
    sharedSensorData.humidity = 0.0f;
    sharedSensorData.pressurePa = 0.0f;

    sharedSensorData.fanPulses = 0;
    sharedSensorData.fanSpeed = 0;

    sharedSensorData.timestamp = millis();

    sharedSensorData.valid = false;

    // --------------------------------------------------------
    // CREATE TASKS
    // --------------------------------------------------------

    BaseType_t result;

    result = xTaskCreate(
        SensorTask,
        "SensorTask",
        4096,
        nullptr,
        2,
        nullptr
    );

    if (result != pdPASS)
    {
        Serial.println("ERROR: SensorTask creation failed.");
    }

    result = xTaskCreate(
        ControllerTask,
        "ControllerTask",
        4096,
        nullptr,
        3,
        nullptr
    );

    if (result != pdPASS)
    {
        Serial.println("ERROR: ControllerTask creation failed.");
    }

    result = xTaskCreate(
        UITask,
        "UITask",
        4096,
        nullptr,
        1,
        nullptr
    );

    if (result != pdPASS)
    {
        Serial.println("ERROR: UITask creation failed.");
    }

    result = xTaskCreate(
        NetworkTask,
        "NetworkTask",
        6144,
        nullptr,
        1,
        nullptr
    );

    if (result != pdPASS)
    {
        Serial.println("ERROR: NetworkTask creation failed.");
    }

    Serial.println("All tasks started.");

    // --------------------------------------------------------
    // START FAN OFF
    // --------------------------------------------------------

    setFanSpeed(FAN_OFF);

    Serial.println("System initialized.");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // All application work is handled by FreeRTOS tasks.
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
