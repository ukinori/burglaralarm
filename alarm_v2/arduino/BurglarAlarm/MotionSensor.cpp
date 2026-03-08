#include "MotionSensor.h"

MotionSensor::MotionSensor(const char* id, const char* loc,
                            uint8_t p, int sensitivity,
                            unsigned long holdoffMs)
    : Sensor(id, loc, p),
      sensitivityLevel(sensitivity),
      pirLatched(false),
      pirLatchMs(0),
      pirHoldoffMs(holdoffMs)
{
    // HC-SR505 uses INPUT, not INPUT_PULLUP
    pinMode(pin, INPUT);
}

void MotionSensor::checkStatus() {
    bool pir = (digitalRead(pin) == HIGH);

    // Detect rising edge only (latch prevents re-trigger during 8s HIGH)
    if (pir && !pirLatched) {
        pirLatched = true;
        pirLatchMs = millis();
        trigger();
    }

    handleHoldoff();
}

void MotionSensor::handleHoldoff() {
    if (pirLatched && (millis() - pirLatchMs >= pirHoldoffMs)) {
        pirLatched = false;
        reset();
    }
}

void MotionSensor::setSensitivity(int level) {
    if (level >= 1 && level <= 10) sensitivityLevel = level;
}

int MotionSensor::getSensitivity() const { return sensitivityLevel; }
