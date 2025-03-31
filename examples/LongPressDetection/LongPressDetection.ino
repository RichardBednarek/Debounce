// ****************************************************************************
// Title        : Long Press Detection Example
// Filename     : 'LongPressDetection.ino'
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
const uint16_t INTERVAL_LONG_BLINK = 300;  // Long blink interval (300ms)
const uint8_t INTERVAL_DEBOUNCE = 1;       // Update button state every 1ms
const uint16_t INTERVAL_LONG_PRESS = 500;  // Long press threshold in milliseconds

// LED pattern state machine
enum LedPatternState 
{
    PATTERN_HEARTBEAT,    // Regular heartbeat pattern
    PRESS_SHORT,          // Short press pattern (one quick blink)
    PRESS_LONG            // Long press pattern (one longer blink)
};

LedPatternState ledPattern = PATTERN_HEARTBEAT;  // Current state LED pattern
uint8_t stateLed = LOW;                          // Current LED state
uint32_t timeLedBlink = 0;                       // Time of last LED state change
uint32_t timeDebounce = 0;                       // Time of last debounce update
uint32_t timeButtonDown = 0;                     // Time when button was pressed
uint8_t blinkCount = 0;                          // Counter for blink sequence
bool stateButtonPrevious = false;                // Previous button state
bool longPressDetected = false;                  // Flag for long press detection

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
            
        case PRESS_SHORT:
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
                Serial.println("Short press event complete");
            }
            break;
            
        case PRESS_LONG:
            // One longer blink pattern
            if (blinkCount == 0) 
            {
                // First blink - turn on
                stateLed = HIGH;
                digitalWrite(PIN_LED, stateLed);
                timeLedBlink = currentMillis;
                blinkCount++;
            }
            else if (blinkCount == 1 && currentMillis - timeLedBlink >= INTERVAL_LONG_BLINK) 
            {
                // Turn off after longer interval
                stateLed = LOW;
                digitalWrite(PIN_LED, stateLed);
                timeLedBlink = currentMillis;
                blinkCount++;
            }
            else if (blinkCount == 2 && currentMillis - timeLedBlink >= INTERVAL_LONG_BLINK) 
            {
                // Pattern complete, return to heartbeat
                ledPattern = PATTERN_HEARTBEAT;
                timeLedBlink = currentMillis;
                blinkCount = 0;
                Serial.println("Long press event complete");
            }
            break;
    }
}

// Setup Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example - Long Press Detection");
    Serial.println("=====================================================");
    Serial.println("- LED: Shows different blink patterns for different events");
    Serial.println("- Short press: One quick blink");
    Serial.println("- Long press: One longer blink (press > 500ms)");
    Serial.println("- Heartbeat: Regular 1-second interval blinking");
    Serial.println("=====================================================");
    
    pinMode(PIN_LED, OUTPUT);              // Declare LED pin as digital output
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
    if (currentMillis - timeDebounce >= INTERVAL_DEBOUNCE)
    {
        timeDebounce = currentMillis;
        myButton.update();                 // Update button state every 1ms
    }
    
    // Long press detection logic
    if (myButton.isDown()) 
    {
        // If button just went down, record the time
        if (!stateButtonPrevious) 
        {
            timeButtonDown = currentMillis;
            stateButtonPrevious = true;
            longPressDetected = false;
            Serial.println("Button pressed, waiting to see if it's a long press...");
        }
        
        // Check for long press threshold while button is still down
        if (!longPressDetected && (currentMillis - timeButtonDown >= INTERVAL_LONG_PRESS)) 
        {
            longPressDetected = true;      // Set flag to prevent multiple triggers
            Serial.println("LONG PRESS detected - Showing long blink pattern");
            ledPattern = PRESS_LONG;
            blinkCount = 0;
        }
    }
    else 
    {
        // Button is up
        if (stateButtonPrevious) 
        {
            // Button was just released
            stateButtonPrevious = false;
            
            // If it wasn't a long press, then it was a short press
            if (!longPressDetected) 
            {
                Serial.println("SHORT PRESS detected - Showing short blink pattern");
                ledPattern = PRESS_SHORT;
                blinkCount = 0;
            }
        }
    }
    
    // Always update the LED based on current pattern
    updateLED();
}