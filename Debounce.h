// ****************************************************************************
// Title		: Debounce
// File Name	: 'Debounce.h'
// Target MCU	: Espressif ESP32 (Doit DevKit Version 1)
//
// Code based on
// https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/
//
// Revision History:
// When			Who			Description of change
// -----------	-----------	-----------------------
// 14-APR-2022	brooks		program start
// 27-MAR-2025	brooks		updated to 16-bit history per article
// 30-MAR-2025	brooks		added double press detection
// ****************************************************************************

// Include Files
// ****************************************************************************
#include <Arduino.h>

#ifndef DEBOUNCE_H
#define DEBOUNCE_H

// Double Press State Machine states
enum DoublePressState {
    DP_IDLE,               // No press detected yet
    DP_FIRST_DETECTED,     // First press detected
    DP_AWAITING_SECOND,    // Between presses, waiting for second
    DP_DETECTED,           // Double press has occurred
    DP_COOLDOWN            // Waiting for complete release
};

// Function pointer type for callbacks
typedef void (*ButtonCallback)();

class Debounce
{
	public:
		// Constructors
		Debounce(uint8_t buttonPin);		// Default active HIGH logic

		Debounce(uint8_t buttonPin, uint8_t activeLevel);

		void update(void);
		uint8_t isPressed(void);
		uint8_t isUp(void);
		uint8_t isDown(void);
		uint8_t isReleased(void);
		uint8_t stateChanged(void);
        
        // Double-press detection methods
        void enableDoublePressDetection(bool enable = true);
        void setDoublePressWindow(unsigned long windowMs);
        uint8_t isDoublePressed(void);
        
        // Event callback methods
        void onPress(ButtonCallback callback);
        void onRelease(ButtonCallback callback);
        void onDoublePress(ButtonCallback callback);
        void onLongPressStart(ButtonCallback callback);
        void onLongPressEnd(ButtonCallback callback);
        
        // Long press detection methods
        void setLongPressTime(unsigned long timeMs);

	protected:

	private:
		uint16_t _buttonHistory=0;     // Changed to 16-bit history
		uint8_t _button=0;
		uint8_t _active=0;
		uint8_t _prevState=0;          // Track previous button state
        
        // Double press detection variables
        bool _doublePressEnabled = false;
        unsigned long _doublePressWindow = 300;    // Default 300ms window
        DoublePressState _dpState = DP_IDLE;
        unsigned long _firstPressTime = 0;
        bool _isDoublePressed = false;
        
        // Long press detection variables
        unsigned long _longPressTime = 1000;      // Default 1000ms for long press
        bool _longPressDetected = false;
        
        // Callback function pointers
        ButtonCallback _pressCallback = NULL;
        ButtonCallback _releaseCallback = NULL;
        ButtonCallback _doublePressCallback = NULL;
        ButtonCallback _longPressStartCallback = NULL;
        ButtonCallback _longPressEndCallback = NULL;

		uint8_t readButton(void);
        void updateDoublePress(void);
};

#endif										// DEBOUNCE_H