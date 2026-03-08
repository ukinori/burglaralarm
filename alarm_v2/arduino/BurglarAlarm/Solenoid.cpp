#include "Solenoid.h"

Solenoid::Solenoid(uint8_t p, int r)
    : driverPin(p), baseResistor(r), lockedState(false)
{
    pinMode(driverPin, OUTPUT);
    digitalWrite(driverPin, LOW);   // start unlocked
}

void Solenoid::lock() {
    lockedState = true;
    digitalWrite(driverPin, HIGH);  // saturate BD911 → solenoid ON
}

void Solenoid::unlock() {
    lockedState = false;
    digitalWrite(driverPin, LOW);   // cut off BD911 → solenoid OFF
}

bool Solenoid::isLocked() const { return lockedState; }
