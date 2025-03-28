// ****************************************************************************
// Title        : Simple Toggle Example for Debounce Library
// File Name    : 'SimpleToggle.ino'
// Target MCU   : Espressif ESP32 (Doit DevKit Version 1)
//
// Revision History:
// When         Who         Description of change
// -----------  ----------- -----------------------
// 18-APR-2022  brooks      program start
// 27-MAR-2025  brooks      simplified example
// ****************************************************************************

// Include Files
// ****************************************************************************
#include <Arduino.h>
#include <Debounce.h>                      // Debouncing library

// Globals
// ****************************************************************************
const uint8_t LED = 15;                    // Pin number connected to LED
const uint8_t BUTTON = 17;                 // Pin number for external BUTTON
const uint16_t BLINK_INTERVAL = 1000;      // Blink on/off time in milliseconds

bool ledState = false;                     // State false=LOW, true=HIGH
uint8_t ledBlueState = LOW;                // State of BUILTIN_LED
unsigned long ledBlinkTime = 0;            // Time of last LED heartbeat blink

hw_timer_t * timer0 = NULL;                // Create timer
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool flagTimer0 = false;          // Flag if Interrupt triggered

// Setup button debouncing
const bool logicLevel = HIGH;              // Default logic is active HIGH
Debounce myButton(BUTTON, logicLevel);     // Instantiate debouncing object

// Interrupt Service Routine (ISR)
// ****************************************************************************
void IRAM_ATTR timerISR0()
{
   portENTER_CRITICAL_ISR(&mux);
   flagTimer0 = true;                      // Set flag variable
   portEXIT_CRITICAL_ISR(&mux);
}

// Begin Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example");
    Serial.println("========================");
    Serial.println("- External LED: Toggles when button is pressed");
    Serial.println("- BLUE LED: Heartbeat (blinks every second)");
    Serial.println("========================");
    
    pinMode(LED, OUTPUT);                  // Declare pin as digital output
    digitalWrite(LED, LOW);                // Start with LED off
    
    // Configure button pin based on logic level
    if (logicLevel == LOW) {
        pinMode(BUTTON, INPUT_PULLUP);     // Use internal pullup for active LOW button
    } else {
        pinMode(BUTTON, INPUT);            // Regular input for active HIGH button
    }
    
    pinMode(LED_BUILTIN, OUTPUT);          // LED digital pin as an output

    timer0 = timerBegin(0, 80, true);      // 80MHz/80 prescaler=1 MHz
    timerAttachInterrupt(timer0, &timerISR0, true);
    timerAlarmWrite(timer0, 10000, true);  // ISR 1ms
    timerAlarmEnable(timer0);              // Enable ISR
}

// Main program
// ****************************************************************************
void loop()
{
    // Heartbeat LED blinking
    if (millis() - ledBlinkTime > BLINK_INTERVAL)
    {
        ledBlinkTime = millis();           // Update last LED blink time
        ledBlueState = !ledBlueState;      // Toggle the LED state
        digitalWrite(LED_BUILTIN, ledBlueState); // Set the LED state
    }
    
    // Process debounce updates from timer interrupt
    if (flagTimer0)
    {
        portENTER_CRITICAL(&mux);
        flagTimer0 = false;
        portEXIT_CRITICAL(&mux);
        myButton.update();                 // Check button every 1ms
    }

    // Check if button is pressed and toggle external LED
    if (myButton.isPressed())
    {
        ledState = !ledState;              // Toggle LED state
        digitalWrite(LED, ledState);       // Set the LED state
        
        if (ledState)
            Serial.println("External LED is ON");
        else
            Serial.println("External LED is OFF");
    }
}