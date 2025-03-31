// ****************************************************************************
// Title        : Simple Debounce Example with Interrupt
// Filename     : 'DebounceSimpleInterrupt.ino'
// Target MCU   : Espressif ESP32 (Doit DevKit Version 1)
//
// Revision History:
// When         Who         Description of change
// -----------  ----------- -----------------------
// 18-APR-2022  brooks      program start
// ****************************************************************************

// Include Files
// ****************************************************************************
#include <Arduino.h>
#include <Debounce.h>                      // Debouncing library

// Globals
// ****************************************************************************
const uint8_t PIN_LED = LED_BUILTIN;       // Using built-in LED
const uint8_t PIN_BUTTON = 17;             // Pin number for button
const uint16_t INTERVAL_HEARTBEAT = 1000;  // Heartbeat blink interval (1 second)
const uint16_t INTERVAL_QUICK_BLINK = 100; // Quick blink interval (100ms)

// LED pattern state machine
enum LedPatternState 
{
    PATTERN_HEARTBEAT,    // Regular heartbeat pattern
    PRESS_SINGLE          // Single press pattern (one quick blink)
};

LedPatternState ledPattern = PATTERN_HEARTBEAT;  // Current LED pattern
uint8_t stateLed = LOW;                          // Current LED state
uint32_t timeLedBlink = 0;                       // Time of last LED state change
uint8_t blinkCount = 0;                          // Counter for blink sequence

hw_timer_t * timer0 = NULL;                      // Create timer
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool flagTimer0 = false;                // Flag if Interrupt triggered

// Setup button debouncing
const bool logicLevel = HIGH;                    // Default logic is active HIGH
Debounce myButton(PIN_BUTTON, logicLevel);       // Instantiate debouncing object

// Interrupt Service Routine (ISR)
// ****************************************************************************
void IRAM_ATTR timerISR0()
{
   portENTER_CRITICAL_ISR(&mux);
   flagTimer0 = true;                     // Set flag variable
   portEXIT_CRITICAL_ISR(&mux);
}

// Function Implementations
// ****************************************************************************
// Function to update LED based on current pattern
void updateLED() 
{
    uint32_t currentMillis = millis();
    
    switch (ledPattern) 
    {
        case PATTERN_HEARTBEAT:
            // Regular heartbeat pattern (1 second on, 1 second off)
            if (currentMillis - timeLedBlink >= INTERVAL_HEARTBEAT) 
            {
                timeLedBlink = currentMillis;
                stateLed = !stateLed;
                digitalWrite(PIN_LED, stateLed);
            }
            break;
            
        case PRESS_SINGLE:
            // One quick blink pattern
            if (blinkCount == 0) 
            {
                // First blink - turn on
                stateLed = HIGH;
                digitalWrite(PIN_LED, stateLed);
                timeLedBlink = currentMillis;
                blinkCount++;
            }
            else if (blinkCount == 1 && currentMillis - timeLedBlink >= INTERVAL_QUICK_BLINK) 
            {
                // Turn off after interval
                stateLed = LOW;
                digitalWrite(PIN_LED, stateLed);
                timeLedBlink = currentMillis;
                blinkCount++;
            }
            else if (blinkCount == 2 && currentMillis - timeLedBlink >= INTERVAL_QUICK_BLINK) 
            {
                // Pattern complete, return to heartbeat
                ledPattern = PATTERN_HEARTBEAT;
                timeLedBlink = currentMillis;
                blinkCount = 0;
                Serial.println("Button press event complete");
            }
            break;
    }
}

// Setup Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example - Interrupt Version");
    Serial.println("=========================================");
    Serial.println("- LED: Shows heartbeat pattern");
    Serial.println("- Button press: Shows quick blink pattern");
    Serial.println("=========================================");
    
    pinMode(PIN_LED, OUTPUT);              // Declare pin as digital output
    digitalWrite(PIN_LED, LOW);            // Start with LED off
    
    // Configure button pin based on logic level
    if (logicLevel == LOW) 
    {
        pinMode(PIN_BUTTON, INPUT_PULLUP); // Use internal pullup for active LOW button
    } 
    else 
    {
        pinMode(PIN_BUTTON, INPUT);        // Regular input for active HIGH button
    }

    timer0 = timerBegin(0, 80, true);      // 80MHz/80 prescaler=1 MHz
    timerAttachInterrupt(timer0, &timerISR0, true);
    timerAlarmWrite(timer0, 10000, true);  // ISR 1ms
    timerAlarmEnable(timer0);              // Enable ISR
}

// Main program
// ****************************************************************************
void loop()
{
    // Process debounce updates from timer interrupt
    if (flagTimer0)
    {
        portENTER_CRITICAL(&mux);
        flagTimer0 = false;
        portEXIT_CRITICAL(&mux);
        myButton.update();                 // Check button every 1ms
    }

    // Check if button is pressed
    if (myButton.isPressed())
    {
        Serial.println("Button pressed - Showing quick blink pattern");
        ledPattern = PRESS_SINGLE;
        blinkCount = 0;
    }
    
    // Always update the LED based on current pattern
    updateLED();
}