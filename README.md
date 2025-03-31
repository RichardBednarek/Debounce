# Arduino Debounce Library

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/debounce/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.1.0-green.svg)](https://github.com/brooksbUWO/Debounce)

A high-performance Arduino library for button debouncing using a 16-bit history approach for superior noise immunity.

## Overview

This library implements a sophisticated button debouncing system built on the 16-bit history pattern approach. It provides stable and responsive button readings even in electrically noisy environments, and supports a wide range of button interaction types:

- **Button press events** (rising edge detection)
- **Button release events** (falling edge detection)
- **Button held down state** (continuous pressed state)
- **Button up state** (continuous released state)
- **State change detection** (transitions between states)
- **Double-press detection** (two presses within a configurable time window)
- **Long-press detection** (press held for configurable duration)
- **Multiple-button management** (handle many buttons simultaneously)

## Why Use This Library?

- **Superior noise immunity** with 16-bit pattern matching instead of simple timing delays
- **Low memory footprint** - optimized implementation uses minimal resources
- **No blocking delays** - non-blocking implementation keeps your code responsive
- **Complete state detection** - detects press, release, held, and up states in a single library
- **Advanced event recognition** - detect double-press and long-press events
- **Event-based programming** with callbacks for all button events
- **Multiple button support** with consistent API
- **Configurable for any button type** - works with both active HIGH and active LOW buttons

## Key Features

- **16-bit pattern recognition** for reliable button state detection
- **Configurable for active HIGH or active LOW** button configurations
- **Efficient bit manipulation techniques** for rapid response time
- **State change detection** for implementing advanced button interfaces
- **Double-press detection** with configurable timing window
- **Long-press detection** with configurable duration
- **Event callback system** for reactive programming
- **Multiple button support** for complex interfaces
- **Both polling and interrupt-based implementations**
- **Standardized LED pattern system** for visual feedback
- **Compatible with all Arduino boards**
- **Well-documented code** with extensive comments

## Installation

### Arduino Library Manager (Recommended)
1. Open the Arduino IDE
2. Navigate to **Sketch > Include Library > Manage Libraries...**
3. Search for "Debounce"
4. Click "Install"

### Manual Installation
1. Download this repository as a ZIP file
2. In the Arduino IDE, navigate to **Sketch > Include Library > Add .ZIP Library...**
3. Select the downloaded ZIP file
4. Restart the Arduino IDE

## Basic Usage

```cpp
#include <Debounce.h>

// Pin number for button
const int PIN_BUTTON = 2;

// Create debounce instance (default constructor uses active HIGH)
Debounce myButton(PIN_BUTTON);

// Alternative with explicit logic level:
// const bool logicLevel = HIGH;  // Use HIGH for active HIGH, LOW for active LOW
// Debounce myButton(PIN_BUTTON, logicLevel);

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BUTTON, INPUT);
}

void loop() {
  // Must call update() regularly
  myButton.update();
  
  if (myButton.isPressed()) {
    Serial.println("Button pressed event detected!");
  }
}
```

## Implementation Methods

### Polling-based Implementation

The standard implementation uses regular calls to `update()` in your main loop:

```cpp
void loop() {
  // Update button state at regular intervals
  currentMillis = millis();
  if (currentMillis - timeDebounce >= INTERVAL_DEBOUNCE) {
    timeDebounce = currentMillis;
    myButton.update();
  }
  
  // Rest of your code
}
```

This approach is:
- Simple to implement
- Doesn't require timer interrupts
- Good for most applications
- Examples: DebounceSimple, DoublePressDetection, LongPressDetection, etc.

### Interrupt-driven Implementation

For more precise timing, you can use timer interrupts:

```cpp
// Timer interrupt service routine
void IRAM_ATTR onTimer() {
  myButton.update();  // Update button state at precise intervals
}

void setup() {
  // Set up timer interrupt
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);  // 1ms interval
  timerAlarmEnable(timer);
}
```

This approach is:
- More precise timing
- Consistent update rate regardless of main loop timing
- Ideal for critical applications
- Examples: DebounceSimpleInterrupt, DoublePressDetectionInterrupt, etc.

## Advanced Features

### Double-Press Detection

Detect double-presses with configurable timing window:

```cpp
// Enable double press detection with default window (300ms)
myButton.enableDoublePressDetection();

// Or with custom timing window (in milliseconds)
myButton.enableDoublePressDetection(true);
myButton.setDoublePressWindow(400);  // 400ms window

// In loop
if (myButton.isDoublePressed()) {
  // Handle double press event
}
```

### Long-Press Detection

Detect long-presses with configurable duration:

```cpp
// Set long press time (default is 1000ms)
myButton.setLongPressTime(1500);  // 1.5 second long press

// In loop
if (myButton.isLongPressed()) {
  // Handle long press event
}
```

### Event Callbacks

Register callbacks for event-driven programming:

```cpp
void setup() {
  // Register callbacks
  myButton.onPress(handlePress);
  myButton.onRelease(handleRelease);
  myButton.onDoublePress(handleDoublePress);
  myButton.onLongPressStart(handleLongPressStart);
  myButton.onLongPressEnd(handleLongPressEnd);
}

// Callback functions
void handlePress() {
  Serial.println("Button pressed!");
}

void handleDoublePress() {
  Serial.println("Double press detected!");
}
```

### Multiple Button Support

Handle multiple buttons efficiently:

```cpp
const int NUM_BUTTONS = 3;
const int PIN_BUTTONS[NUM_BUTTONS] = {2, 3, 4};
Debounce buttons[NUM_BUTTONS] = {
  Debounce(PIN_BUTTONS[0], HIGH),
  Debounce(PIN_BUTTONS[1], HIGH),
  Debounce(PIN_BUTTONS[2], HIGH)
};

void loop() {
  // Update all buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
    
    if (buttons[i].isPressed()) {
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" pressed!");
    }
  }
}
```

## LED Pattern System

The example sketches demonstrate various button events using a standardized LED pattern system:

- **PATTERN_HEARTBEAT**: Regular 1-second interval blinking during idle time
- **PRESS_SINGLE**: One quick blink for single press detection
- **PRESS_DOUBLE**: Two quick blinks for double press detection
- **PRESS_SHORT**: One quick blink for short press detection
- **PRESS_LONG**: One longer blink for long press detection

This system uses only the built-in LED (LED_BUILTIN) with different blink patterns to indicate button events, making the examples more accessible without requiring external components.

## Complete Examples

The library includes the following examples:

- **DebounceSimple** - Basic button debounce with toggle (polling-based)
- **DebounceSimpleInterrupt** - Basic button debounce with toggle (interrupt-based)
- **DoublePressDetection** - Implements double press detection (polling-based)
- **DoublePressDetectionInterrupt** - Implements double press detection (interrupt-based)
- **LongPressDetection** - Implements long press detection (polling-based)
- **LongPressDetectionInterrupt** - Implements long press detection (interrupt-based)
- **MultipleButtons** - Handles multiple buttons simultaneously (polling-based)
- **MultipleButtonsInterrupt** - Handles multiple buttons simultaneously (interrupt-based)
- **RGBVisualization** - Visualizes button states using RGB LED (polling-based)
- **RGBVisualizationInterrupt** - Visualizes button states using RGB LED (interrupt-based)
- **AdvancedButtonEvents** - Demonstrates callback functionality (polling-based)
- **AdvancedButtonEventsInterrupt** - Demonstrates callback functionality (interrupt-based)

## How It Works

Rather than using simple delay-based debouncing, this library uses a sophisticated bit-pattern approach:

1. Each time `update()` is called, the 16-bit history register is shifted left by one bit
2. The current button state is added to the least significant bit of the history
3. Specific bit patterns are detected to identify different button events:
   - Press pattern: `0b0000000000111111`
   - Release pattern: `0b1111000000000000`
   - Down state: `0b1111111111111111`
   - Up state: `0b0000000000000000`

This approach provides superior noise immunity and can detect both immediate events (press/release) and continuous states (down/up).

For advanced features like double-press detection, the library implements state machines that track timing between events while maintaining the core 16-bit pattern approach for button state detection.

## Performance Considerations

For optimal performance:
- Call `update()` as frequently as possible
- For timing-critical applications, consider calling `update()` from a timer interrupt
- The library uses minimal CPU resources, so it can be used with multiple buttons simultaneously
- When using multiple buttons, consider the memory usage (each button instance uses approximately 12 bytes)
- For double-press detection, ensure the timing window is appropriate for your application

## License

This library is released under the MIT License. See the LICENSE file for details.

## Credits

- Based on Jack Ganssle's foundational work on debouncing:
  - ["A Guide to Debouncing"](https://www.ganssle.com/debouncing.pdf)
  - ["A Guide to Debouncing - Part 2"](https://www.ganssle.com/debouncing-pt2.htm)

- Implemented based on Elliot Williams' "Ultimate Debouncer" approach:
  - ["Embed with Elliot: Debounce Your Noisy Buttons, Part I"](https://hackaday.com/2015/12/09/embed-with-elliot-debounce-your-noisy-buttons-part-i/)
  - ["Embed with Elliot: Debounce Your Noisy Buttons, Part II"](https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/)