// ****************************************************************************
// Title        : Advanced Button Events Example with Interrupt
// Filename     : 'AdvancedButtonEventsInterrupt.ino'
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
const uint8_t PIN_LED_PRESS = 15;          // LED for regular press events
const uint8_t PIN_LED_DOUBLE = 2;          // LED for double press events
const uint8_t PIN_LED_LONG = 4;            // LED for long press events
const uint8_t PIN_BUTTON = 17;             // Pin number for BUTTON
const uint16_t INTERVAL_HEARTBEAT = 1000;  // Heartbeat blink time in milliseconds
const uint16_t INTERVAL_DOUBLE_PRESS = 300; // Double press window in milliseconds
const uint16_t INTERVAL_LONG_PRESS = 1000; // Long press threshold in milliseconds

bool stateLedPress = false;                // State of regular press LED
bool stateLedDouble = false;               // State of double press LED
bool stateLedLong = false;                 // State of long press LED
uint8_t stateLedBlue = LOW;                // State of BUILTIN_LED
uint32_t timeLedBlink = 0;                 // Time of last LED heartbeat blink
int eventCounter = 0;                      // Counter for monitoring events

hw_timer_t * timer0 = NULL;                // Create timer
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool flagTimer0 = false;          // Flag if Interrupt triggered

// Setup button debouncing
const bool logicLevel = HIGH;              // Default logic is active HIGH
Debounce myButton(PIN_BUTTON, logicLevel); // Instantiate debouncing object

// Function Prototypes
// ****************************************************************************
void onButtonPress();
void onButtonRelease();
void onDoublePress();
void onLongPressStart();
void onLongPressEnd();

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
// Regular button press event
void onButtonPress()
{
    eventCounter++;
    Serial.print("Button press detected (Event #");
    Serial.print(eventCounter);
    Serial.println(")");
}

// Regular button release event
void onButtonRelease()
{
    Serial.println("Button released");
    
    // Only toggle on release if this wasn't part of a double press
    // and wasn't a long press
    if (myButton._dpState == DP_IDLE)
    {
        stateLedPress = !stateLedPress;
        digitalWrite(PIN_LED_PRESS, stateLedPress);
        
        if (stateLedPress)
            Serial.println("Regular press confirmed - LED is ON");
        else
            Serial.println("Regular press confirmed - LED is OFF");
    }
}

// Double press event
void onDoublePress()
{
    stateLedDouble = !stateLedDouble;      // Toggle double press LED
    digitalWrite(PIN_LED_DOUBLE, stateLedDouble);
    
    if (stateLedDouble)
        Serial.println("DOUBLE PRESS detected - Double press LED is ON");
    else
        Serial.println("DOUBLE PRESS detected - Double press LED is OFF");
}

// Long press start event
void onLongPressStart()
{
    stateLedLong = !stateLedLong;          // Toggle long press LED
    digitalWrite(PIN_LED_LONG, stateLedLong);
    
    if (stateLedLong)
        Serial.println("LONG PRESS started - Long press LED is ON");
    else
        Serial.println("LONG PRESS started - Long press LED is OFF");
}

// Long press end event
void onLongPressEnd()
{
    Serial.println("LONG PRESS ended");
}

// Setup Code
// ****************************************************************************
void setup()
{
    Serial.begin(115200);                  // Starts serial monitor
    Serial.println("Debounce Library Example - Advanced Button Events with Interrupt");
    Serial.println("=============================================================");
    Serial.println("- Regular press: Toggle first LED");
    Serial.println("- Double press: Toggle second LED (two presses < 300ms)");
    Serial.println("- Long press: Toggle third LED (press > 1000ms)");
    Serial.println("- BLUE LED: Heartbeat (blinks every second)");
    Serial.println("=============================================================");
    
    pinMode(PIN_LED_PRESS, OUTPUT);        // Declare regular press LED pin as output
    digitalWrite(PIN_LED_PRESS, LOW);      // Start with LED off
    
    pinMode(PIN_LED_DOUBLE, OUTPUT);       // Declare double press LED pin as output
    digitalWrite(PIN_LED_DOUBLE, LOW);     // Start with LED off
    
    pinMode(PIN_LED_LONG, OUTPUT);         // Declare long press LED pin as output
    digitalWrite(PIN_LED_LONG, LOW);       // Start with LED off
    
    // Configure button pin based on logic level
    if (logicLevel == LOW) 
    {
        pinMode(PIN_BUTTON, INPUT_PULLUP); // Use internal pullup for active LOW button
    } 
    else 
    {
        pinMode(PIN_BUTTON, INPUT);        // Regular input for active HIGH button
    }
    
    pinMode(LED_BUILTIN, OUTPUT);          // LED digital pin as an output
    
    // Configure button event detection
    myButton.enableDoublePressDetection(true);
    myButton.setDoublePressWindow(INTERVAL_DOUBLE_PRESS);
    myButton.setLongPressTime(INTERVAL_LONG_PRESS);
    
    // Register callback functions
    myButton.onPress(onButtonPress);
    myButton.onRelease(onButtonRelease);
    myButton.onDoublePress(onDoublePress);
    myButton.onLongPressStart(onLongPressStart);
    myButton.onLongPressEnd(onLongPressEnd);
    
    // Configure timer interrupt
    timer0 = timerBegin(0, 80, true);      // 80MHz/80 prescaler=1 MHz
    timerAttachInterrupt(timer0, &timerISR0, true);
    timerAlarmWrite(timer0, 10000, true);  // ISR 1ms
    timerAlarmEnable(timer0);              // Enable ISR
    
    // Print configuration summary
    Serial.println("Button Configuration:");
    Serial.print("  Logic Level: "); 
    Serial.println(logicLevel ? "HIGH" : "LOW");
    Serial.print("  Double-Press Window: "); 
    Serial.print(INTERVAL_DOUBLE_PRESS);
    Serial.println(" ms");
    Serial.print("  Long-Press Time: "); 
    Serial.print(INTERVAL_LONG_PRESS);
    Serial.println(" ms");
    
    Serial.println("Ready! Press the button to test different events.");
}

// Main program
// ****************************************************************************
void loop()
{
    // Heartbeat LED blinking
    uint32_t currentMillis = millis();
    if (currentMillis - timeLedBlink > INTERVAL_HEARTBEAT)
    {
        timeLedBlink = currentMillis;      // Update last LED blink time
        stateLedBlue = !stateLedBlue;      // Toggle the LED state
        digitalWrite(LED_BUILTIN, stateLedBlue); // Set the LED state
    }
    
    // Process debounce updates from timer interrupt
    if (flagTimer0)
    {
        portENTER_CRITICAL(&mux);
        flagTimer0 = false;
        portEXIT_CRITICAL(&mux);
        myButton.update();                 // Check button every 1ms
    }
    
    // Note: The actual button event handling is done through callbacks
    // This loop just handles timing and housekeeping
}