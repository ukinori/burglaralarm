#include "WindowSensor.h"

WindowSensor::WindowSensor(const char* id, const char* loc, uint8_t p)
    : Sensor(id, loc, p), isWindowOpen(false), lastState(false) {}

void WindowSensor::checkStatus() {
    bool current = (digitalRead(pin) == HIGH);
    if (current && !lastState) {
        isWindowOpen = true;
        trigger();
        onOpen();
    }
    if (!current) {
        isWindowOpen = false;
        reset();
    }
    lastState = current;
}

bool WindowSensor::getWindowOpen() const { return isWindowOpen; }

void WindowSensor::onOpen() {}
