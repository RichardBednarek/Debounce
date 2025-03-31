// ****************************************************************************
// Title        : Long Press Detection Example - Interrupt Version
// Filename     : 'LongPressDetectionInterrupt.ino'
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
const uint16_t INTERVAL_LONG_PRESS = 500;  // Long press threshold in milliseconds

// LED pattern state machine
enum LedPatternState 
{
    PATTERN_HEARTBEAT,    // Regular heartbeat pattern
    PRESS_SHORT,          // Short press pattern (one quick blink)
    PRESS_LONG            // Long press pattern (one longer blink)
};

LedPatternState ledPattern = PATTERN_HEARTBEAT;  // Current LED pattern
uint8_t stateLed = LOW;                          // Current LED state
uint32_t timeLedBlink = 0;                       // Time of last LED state change
uint32_t timeButtonDown = 0;                     // Time when button was pressed
uint8_t blinkCount = 0;                          // Counter for blink sequence
bool stateButtonPrevious = false;                // Previous button state
bool longPressDetected = false;                  // Flag for long press detection

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
   flagTimer0 = true;                      // Set flag variable
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
    Serial.println("Debounce Library Example - Long Press Detection (Interrupt Version)");
    Serial.println("==================================================================");
    Serial.println("- LED: Shows different blink patterns for different events");
    Serial.println("- Short press: One quick blink");
    Serial.println("- Long press: One longer blink (press > 500ms)");
    Serial.println("- Heartbeat: Regular 1-second interval blinking");
    Serial.println("==================================================================");
    
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

    // Configure and start the timer interrupt
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
    
    // Long press detection logic
    if (myButton.isDown()) 
    {
        // If button just went down, record the time
        if (!stateButtonPrevious) 
        {
            timeButtonDown = millis();
            stateButtonPrevious = true;
            longPressDetected = false;
            Serial.println("Button pressed, waiting to see if it's a long press...");
        }
        
        // Check for long press threshold while button is still down
        if (!longPressDetected && (millis() - timeButtonDown >= INTERVAL_LONG_PRESS)) 
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