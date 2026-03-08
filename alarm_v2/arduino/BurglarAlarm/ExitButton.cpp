#include "ExitButton.h"

ExitButton::ExitButton(uint8_t p)
    : pin(p), pressedState(false), lastState(false)
{
    pinMode(pin, INPUT_PULLUP);
}

void ExitButton::checkStatus() {
    bool current = (digitalRead(pin) == LOW);   // LOW = pressed (active-low)
    pressedState = (current && !lastState);      // rising edge in logical sense
    lastState    = current;
}

bool ExitButton::isPressed() const { return pressedState; }

void ExitButton::onPress() {}  // AlarmSystem polls isPressed() instead
