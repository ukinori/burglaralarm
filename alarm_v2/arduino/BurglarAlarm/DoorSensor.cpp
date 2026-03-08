#include "DoorSensor.h"

DoorSensor::DoorSensor(const char* id, const char* loc, uint8_t p)
    : Sensor(id, loc, p), isDoorOpen(false), lastState(false) {}

void DoorSensor::checkStatus() {
    bool current = (digitalRead(pin) == HIGH);  // HIGH = open
    if (current && !lastState) {
        isDoorOpen = true;
        trigger();
        onOpen();
    }
    if (!current) {
        isDoorOpen = false;
        reset();
    }
    lastState = current;
}

bool DoorSensor::getDoorOpen() const { return isDoorOpen; }

void DoorSensor::onOpen() {
    // Callback: AlarmSystem polls isTriggered() rather than using this
}
