# Arduino Debounce Library

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/reference/en/libraries/debounce/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)](https://github.com/brooksbUWO/Debounce)

A high-performance Arduino library for button debouncing using a 16-bit history approach for superior noise immunity.

## Overview

This library implements Elliot Williams' "Ultimate Debouncer" approach, providing stable and responsive button readings even in electrically noisy environments. Using a sophisticated 16-bit history pattern, it reliably detects various button states:

- **Button press events** (rising edge detection)
- **Button release events** (falling edge detection)
- **Button held down state** (continuous pressed state)
- **Button up state** (continuous released state)
- **State change detection** (transitions between states)

## Why Use This Library?

- **Superior noise immunity** with 16-bit pattern matching instead of simple timing delays
- **Low memory footprint** - optimized implementation uses minimal resources
- **No blocking delays** - non-blocking implementation keeps your code responsive
- **Complete state detection** - detects press, release, held, and up states in a single library
- **Configurable for any button type** - works with both active HIGH and active LOW buttons

## Key Features

- **16-bit pattern recognition** for reliable button state detection
- **Configurable for active HIGH or active LOW** button configurations
- **Efficient bit manipulation techniques** for rapid response time
- **State change detection** for implementing advanced button interfaces
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

## Usage

### Basic Usage

```cpp
#include <Debounce.h>

// Pin number for button
const int BUTTON_PIN = 2;

// Create debounce instance (default constructor uses active HIGH)
Debounce button(BUTTON_PIN);

// Alternative with explicit logic level:
// const int logicLevel = HIGH;  // Use HIGH for active HIGH, LOW for active LOW
// Debounce button(BUTTON_PIN, logicLevel);

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  // Must call update() regularly
  button.update();
  
  if (button.isPressed()) {
    Serial.println("Button pressed event detected!");
  }
}
```

### Complete Example

```cpp
#include <Debounce.h>

const int BUTTON_PIN = 2;
const int LED_PIN = 13;

// Define the button logic level
const int logicLevel = LOW;  // Use LOW for active LOW, HIGH for active HIGH

// Create debounce instance with specified logic level
Debounce button(BUTTON_PIN, logicLevel);

void setup() {
  Serial.begin(9600);
  
  // Configure pin mode based on logic level
  if (logicLevel == LOW) {
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use internal pullup for active LOW button
  } else {
    pinMode(BUTTON_PIN, INPUT);  // Regular input for active HIGH button
  }
  
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Update debounce state - call this as frequently as possible
  button.update();
  
  // Check for button press event (happens once at press moment)
  if (button.isPressed()) {
    Serial.println("Button PRESSED!");
    digitalWrite(LED_PIN, HIGH);
  }
  
  // Check for button release event (happens once at release moment)
  if (button.isReleased()) {
    Serial.println("Button RELEASED!");
    digitalWrite(LED_PIN, LOW);
  }
  
  // Check for continuous button states
  if (button.isDown()) {
    // Button is currently held down
    // This will be true for entire duration of press
  }
  
  if (button.isUp()) {
    // Button is currently up/released
    // This will be true for entire duration of released state
  }
  
  // Detect any state change (press or release)
  if (button.stateChanged()) {
    Serial.println("Button state changed!");
  }
}
```

## Examples Included

The library comes with the following example sketches:

1. **SimpleToggle** - Basic example that toggles an LED when a button is pressed
2. **RGBVisualization** - Advanced example using RGB LEDs to visualize different button states and transitions

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

## Performance Considerations

For optimal performance:
- Call `update()` as frequently as possible
- For timing-critical applications, consider calling `update()` from a timer interrupt
- The library uses minimal CPU resources, so it can be used with multiple buttons simultaneously

## License

This library is released under the MIT License. See the LICENSE file for details.

## Credits

- Based on the "Ultimate Debouncer" approach by Elliot Williams:
  https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/
- Maintained by [brooksbUWO](https://github.com/brooksbUWO)

## Contributing

Contributions to improve the library are welcome! Please feel free to submit a pull request or open an issue on GitHub.