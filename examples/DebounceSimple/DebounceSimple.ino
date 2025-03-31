// ****************************************************************************
// Title        : Simple Debounce Example
// Filename     : 'DebounceSimple.ino'
// Target MCU   : Espressif ESP32 (Doit DevKit Version 1)
//
// Revision History:
// When         Who         Description of change
// -----------  ----------- -----------------------
// 28-MAR-2025  brooks      program start
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
const uint8_t INTERVAL_DEBOUNCE = 1;       // Update button state every 1ms

// LED pattern state machine
enum LedPatternState 
{
    PATTERN_HEARTBEAT,    // Regular heartbeat pattern
    PRESS_SINGLE          // Single press pattern (one quick blink)
};

LedPatternState ledPattern = PATTERN_HEARTBEAT;  // Current LED pattern
uint8_t stateLed = LOW;                          // Current LED state
uint32_t timeLedBlink = 0;                       // Time of last LED state change
uint32_t timeDebounce = 0;                       // Time of last debounce update
uint8_t blinkCount = 0;                          // Counter for blink sequence

// Setup button debouncing
const bool logicLevel = HIGH;                    // Default logic is active HIGH
Debounce myButton(PIN_BUTTON, logicLevel);       // Instantiate debouncing object

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
    Serial.println("Debounce Library Example - Non-Interrupt Version");
    Serial.println("==============================================");
    Serial.println("- LED: Shows heartbeat pattern");
    Serial.println("- Button press: Shows quick blink pattern");
    Serial.println("==============================================");
    
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
}

// Main program
// ****************************************************************************
void loop()
{
    // Current time for non-blocking operations
    uint32_t currentMillis = millis();
    
    // Non-interrupt approach: update button state at regular intervals
    // This replaces the timer interrupt approach in the other examples
    if (currentMillis - timeDebounce >= INTERVAL_DEBOUNCE)
    {
        timeDebounce = currentMillis;
        myButton.update();                 // Update button state every 1ms
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