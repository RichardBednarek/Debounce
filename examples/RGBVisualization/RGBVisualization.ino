// ****************************************************************************
// Title        : RGB Visualization Example for Debounce Library
// File Name    : 'RGBVisualization.ino'
// Target MCU   : Espressif ESP32 (Doit DevKit Version 1)
//
// Revision History:
// When         Who         Description of change
// -----------  ----------- -----------------------
// 18-APR-2022  brooks      program start
// 27-MAR-2025  brooks      added RGB LED control with all debounce functions
// ****************************************************************************

// Include Files
// ****************************************************************************
#include <Arduino.h>
#include <Debounce.h>                      // Debouncing library
#include <FastLED.h>                       // FastLED library

// Globals
// ****************************************************************************
const uint8_t LED = 15;                    // Pin number connected to LED
const uint8_t DATA_PIN = 16;               // Pin number for RGB LED
const uint8_t NUM_LEDS = 1;                // Number of RGB LEDs
const uint8_t BUTTON = 17;                 // Pin number for external BUTTON
const uint16_t BLINK_INTERVAL = 1000;      // Blink on/off time in milliseconds

bool ledState = false;                     // State false=LOW, true=HIGH
uint8_t ledBlueState = LOW;                // State of BUILTIN_LED
unsigned long ledBlinkTime = 0;            // Time of last LED heartbeat blink

CRGB leds[NUM_LEDS];                       // Define the array of leds

hw_timer_t * timer0 = NULL;                // Create timer
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool flagTimer0 = false;          // Flag if Interrupt triggered

// Define colors for different button states
const uint8_t BRIGHTNESS = 100;            // LED brightness level
const CRGB COLOR_OFF = CRGB::Black;        // LED off
const CRGB COLOR_PRESSED = CRGB::Red;      // Color when button is pressed
const CRGB COLOR_RELEASED = CRGB::Blue;    // Color when button is released
const CRGB COLOR_DOWN = CRGB::Green;       // Color when button is held down
const CRGB COLOR_UP = CRGB::Purple;        // Color when button is up

// Setup button debouncing with active HIGH logic
const uint8_t logicLevel = HIGH;           // Use HIGH for active HIGH buttons
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
    Serial.println("Debounce Library Example with RGB LED");
    Serial.println("=====================================");
    Serial.println("- RED: Button pressed event");
    Serial.println("- BLUE: Button released event");
    Serial.println("- GREEN: Button held down state");
    Serial.println("- PURPLE: Button up state");
    Serial.println("- BLUE LED: Heartbeat (blinks every second)");
    Serial.println("=====================================");
    
    pinMode(LED, OUTPUT);                  // Declare pin as digital output
    digitalWrite(LED, LOW);                // Start with LED off
    
    // Configure button pin based on logic level
    if (logicLevel == LOW) {
        pinMode(BUTTON, INPUT_PULLUP);     // Use internal pullup for active LOW button
    } else {
        pinMode(BUTTON, INPUT);            // Regular input for active HIGH button
    }
    
    pinMode(LED_BUILTIN, OUTPUT);          // LED digital pin as an output

    // Initialize FastLED library
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, COLOR_UP);  // Initial state - up
    FastLED.show();

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
        ledState = !ledState;              // Toggle LED state
        digitalWrite(LED, ledState);       // Set the LED state
        ledBlueState = !ledBlueState;      // Toggle LED on/off
        digitalWrite(LED_BUILTIN, ledBlueState);
    }
    
    // Process debounce updates from timer interrupt
    if (flagTimer0)                        // Process timer interrupt
    {
        portENTER_CRITICAL(&mux);
        flagTimer0 = false;
        portEXIT_CRITICAL(&mux);
        myButton.update();                 // Check button every 1ms
    }

    // Check button states and update RGB LED
    if (myButton.isPressed())
    {
        fill_solid(leds, NUM_LEDS, COLOR_PRESSED);
        FastLED.show();
        Serial.println("Button PRESSED - LED Red");
    }
    else if (myButton.isReleased())
    {
        fill_solid(leds, NUM_LEDS, COLOR_RELEASED);
        FastLED.show();
        Serial.println("Button RELEASED - LED Blue");
    }
    else if (myButton.stateChanged())
    {
        if (myButton.isDown())
        {
            fill_solid(leds, NUM_LEDS, COLOR_DOWN);
            FastLED.show();
            Serial.println("Button DOWN - LED Green");
        }
        else if (myButton.isUp())
        {
            fill_solid(leds, NUM_LEDS, COLOR_UP);
            FastLED.show();
            Serial.println("Button UP - LED Purple");
        }
    }
}