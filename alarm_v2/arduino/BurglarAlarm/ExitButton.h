#ifndef EXIT_BUTTON_H
#define EXIT_BUTTON_H
#include <Arduino.h>

/**
 * ExitButton  –  APEM PHAP3301 Short-Travel Push Button
 *
 * On/off action. Wired active-low with INPUT_PULLUP.
 * Pressing the button pulls the pin to GND.
 * The class detects the falling edge (pressed event).
 *
 * Wiring:
 *   Pin 1 → Arduino digital pin (INPUT_PULLUP)
 *   Pin 2 → GND
 */
class ExitButton {
private:
    uint8_t pin;
    bool    pressedState;
    bool    lastState;

public:
    explicit ExitButton(uint8_t pin);

    void checkStatus();       // call every loop(); updates internal state
    bool isPressed() const;   // true on the press event (single-fire)
    void onPress();           // hook; AlarmSystem calls isPressed() each loop
};

#endif
