#ifndef SOLENOID_H
#define SOLENOID_H
#include <Arduino.h>

/**
 * Solenoid  –  Pimoroni COM2700 5V push-pull solenoid
 * Driven by a BD911 NPN BJT transistor.
 *
 * Circuit:
 *   Arduino driverPin → 220Ω (baseResistor) → BD911 Base
 *   BD911 Collector → Solenoid (−)
 *   Solenoid (+) → 5V
 *   BD911 Emitter → GND
 *   Flyback diode: Cathode→5V, Anode→Collector
 *
 * HIGH on driverPin = transistor saturated = solenoid LOCKED.
 * LOW  on driverPin = transistor off       = solenoid UNLOCKED.
 */
class Solenoid {
private:
    uint8_t driverPin;
    int     baseResistor;   // Ohms — 220Ω in this design
    bool    lockedState;

public:
    Solenoid(uint8_t driverPin, int baseResistor = 220);

    void lock();
    void unlock();
    bool isLocked() const;
};

#endif
