// ****************************************************************************
// Title        : Double Press Detection Example with Interrupt
// Filename     : 'DoublePressDetectionInterrupt.ino'
// Target MCU   : Espressif ESP32 (Doit DevKit Version 1)
//
// Revision History:
// When         Who         Description of change
// -----------  ----------- -----------------------
// 30-MAR-2025  brooks      program start
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
const uint16_t INTERVAL_DOUBLE_PRESS = 300; // Double press detection window

// LED pattern state machine
enum LedPatternState 
{
    PATTERN_HEARTBEAT,    // Regular heartbeat pattern
    PRESS_SINGLE,         // Single press pattern (one quick blink)
    PRESS_DOUBLE          // Double press pattern (two quick blinks)
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
                Serial.println("Single press event complete");
            }
            break;
            
        case PRESS_DOUBLE:
            // Two quick blinks pattern
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
                // Second blink - turn on
                stateLed = HIGH;
                digitalWrite(PIN_LED, stateLed);
                timeLedBlink = currentMillis;
                blinkCount++;
            }
            else if (blinkCount == 3 && currentMillis - timeLedBlink >= INTERVAL_QUICK_BLINK) 
            {
                // Turn off after interval
                stateLed = LOW;
                digitalWrite(PIN_LED, stateLed);
                timeLedBlink = currentMillis;
                blinkCount++;
            }
            else if (blinkCount == 4 && currentMillis - timeLedBlink >= INTERVAL_QUICK_BLINK) 
            {
                // Pattern complete, return to heartbeat
                ledPattern = PATTERN_HEARTBEAT;
                timeLedBlink = currentMillis;
                blinkCount = 0;
                Serial.println("Double press event complete");
            }
            break;
    }
}

// Setup Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example - Double Press Detection with Interrupt");
    Serial.println("============================================================");
    Serial.println("- LED: Shows different blink patterns for different events");
    Serial.println("- Single press: One quick blink");
    Serial.println("- Double press: Two quick blinks");
    Serial.println("- Heartbeat: Regular 1-second interval blinking");
    Serial.println("============================================================");
    
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
    
    // Initialize and configure double press detection
    myButton.enableDoublePressDetection(true);
    myButton.setDoublePressWindow(INTERVAL_DOUBLE_PRESS);
    
    // Configure timer interrupt
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
    
    // Handle single press (non-double press)
    if (myButton.isPressed())
    {
        // If double press detection is enabled, a normal press 
        // is only meaningful if it's not part of a double press sequence
        Serial.println("Button pressed - waiting to see if it's part of a double press...");
    }
    
    // Handle double press
    if (myButton.isDoublePressed())
    {
        Serial.println("DOUBLE PRESS detected - Showing double blink pattern");
        ledPattern = PRESS_DOUBLE;
        blinkCount = 0;
    }
    
    // Once a button press is confirmed to be a single press (not part of double press)
    // We show the single press blink pattern
    static bool firstPress = true;
    if (myButton.isReleased() && myButton._dpState == DP_IDLE && firstPress)
    {
        firstPress = false;
        Serial.println("SINGLE PRESS confirmed - Showing single blink pattern");
        ledPattern = PRESS_SINGLE;
        blinkCount = 0;
    }
    else if (myButton.isReleased() && myButton._dpState == DP_IDLE)
    {
        firstPress = true;
    }
    
    // Always update the LED based on current pattern
    updateLED();
}