// ****************************************************************************
// Title        : Multiple Button Handling Example
// Filename     : 'MultipleButtons.ino'
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
const uint16_t INTERVAL_HEARTBEAT = 1000;  // Heartbeat blink time in milliseconds
const uint8_t INTERVAL_DEBOUNCE = 1;       // Update button state every 1ms
uint8_t stateLedBlue = LOW;                // State of BUILTIN_LED
uint32_t timeLedBlink = 0;                 // Time of last LED heartbeat blink
uint32_t timeDebounce = 0;                 // Time of last debounce update

// Array of BUTTON pins
const uint8_t BUTTON_PINS[] = {16, 17, 18, 19}; // Four buttons
const uint8_t NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

// Array of LED pins (one per button)
const uint8_t LED_PINS[] = {21, 22, 23, 25}; // Four LEDs
const uint8_t NUM_LEDS = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

// Button and LED states
Debounce* buttons[NUM_BUTTONS];            // Array of button objects
bool stateLeds[NUM_LEDS] = {false};        // Current LED states

// Setup Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example - Multiple Buttons");
    Serial.println("=========================================");
    Serial.println("- Each button toggles its corresponding LED");
    Serial.println("- BLUE LED: Heartbeat (blinks every second)");
    Serial.println("=========================================");
    
    // Initialize buttons and LEDs
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        buttons[i] = new Debounce(BUTTON_PINS[i], HIGH); // Create button object
        pinMode(BUTTON_PINS[i], INPUT);    // Set button pin as input
        
        pinMode(LED_PINS[i], OUTPUT);      // Set LED pin as output
        digitalWrite(LED_PINS[i], LOW);    // Start with LED off
        
        Serial.print("Button ");
        Serial.print(i);
        Serial.print(" on pin ");
        Serial.print(BUTTON_PINS[i]);
        Serial.print(" controls LED on pin ");
        Serial.println(LED_PINS[i]);
    }
    
    pinMode(LED_BUILTIN, OUTPUT);          // LED digital pin as an output
}

// Main program
// ****************************************************************************
void loop()
{
    // Current time for non-blocking operations
    uint32_t currentMillis = millis();
    
    // Heartbeat LED blinking
    if (currentMillis - timeLedBlink > INTERVAL_HEARTBEAT)
    {
        timeLedBlink = currentMillis;      // Update last LED blink time
        stateLedBlue = !stateLedBlue;      // Toggle the LED state
        digitalWrite(LED_BUILTIN, stateLedBlue); // Set the LED state
    }
    
    // Non-interrupt approach: update button state at regular intervals
    if (currentMillis - timeDebounce >= INTERVAL_DEBOUNCE)
    {
        timeDebounce = currentMillis;
        
        // Update all buttons
        for (uint8_t i = 0; i < NUM_BUTTONS; i++)
        {
            buttons[i]->update();          // Update button state every 1ms
        }
    }
    
    // Check all buttons for events and update LEDs
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        if (buttons[i]->isPressed())
        {
            // Toggle corresponding LED
            stateLeds[i] = !stateLeds[i];
            digitalWrite(LED_PINS[i], stateLeds[i]);
            
            Serial.print("Button ");
            Serial.print(i);
            Serial.print(" pressed - LED ");
            Serial.print(i);
            Serial.println(stateLeds[i] ? " ON" : " OFF");
        }
    }
}