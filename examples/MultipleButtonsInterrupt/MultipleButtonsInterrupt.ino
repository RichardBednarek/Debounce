// ****************************************************************************
// Title        : Multiple Button Handling Example - Interrupt Version
// Filename     : 'MultipleButtonsInterrupt.ino'
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
uint8_t stateLedBlue = LOW;                // State of BUILTIN_LED
uint32_t timeLedBlink = 0;                 // Time of last LED heartbeat blink

// Array of BUTTON pins
const uint8_t BUTTON_PINS[] = {16, 17, 18, 19}; // Four buttons
const uint8_t NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

// Array of LED pins (one per button)
const uint8_t LED_PINS[] = {21, 22, 23, 25}; // Four LEDs
const uint8_t NUM_LEDS = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

// Button and LED states
Debounce* buttons[NUM_BUTTONS];            // Array of button objects
bool stateLeds[NUM_LEDS] = {false};        // Current LED states

hw_timer_t * timer0 = NULL;                // Create timer
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool flagTimer0 = false;          // Flag if Interrupt triggered

// Interrupt Service Routine (ISR)
// ****************************************************************************
void IRAM_ATTR timerISR0()
{
   portENTER_CRITICAL_ISR(&mux);
   flagTimer0 = true;                      // Set flag variable
   portEXIT_CRITICAL_ISR(&mux);
}

// Setup Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example - Multiple Buttons (Interrupt Version)");
    Serial.println("=========================================================");
    Serial.println("- Each button toggles its corresponding LED");
    Serial.println("- BLUE LED: Heartbeat (blinks every second)");
    Serial.println("=========================================================");
    
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
    // Heartbeat LED blinking
    if (millis() - timeLedBlink > INTERVAL_HEARTBEAT)
    {
        timeLedBlink = millis();           // Update last LED blink time
        stateLedBlue = !stateLedBlue;      // Toggle the LED state
        digitalWrite(LED_BUILTIN, stateLedBlue); // Set the LED state
    }
    
    // Process debounce updates from timer interrupt
    if (flagTimer0)
    {
        portENTER_CRITICAL(&mux);
        flagTimer0 = false;
        portEXIT_CRITICAL(&mux);
        
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