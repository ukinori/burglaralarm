#ifndef BUZZER_H
#define BUZZER_H
#include <Arduino.h>

/**
 * Buzzer  –  RS PRO 78dB Continuous-Tone Piezo (1–30V)
 * Direct digital drive from an Arduino digital pin.
 * No transistor needed; the buzzer draws < 40mA at 5V.
 */
class Buzzer {
private:
    uint8_t pin;
    bool    activeState;

public:
    explicit Buzzer(uint8_t pin);

    void activate();
    void deactivate();
    bool isActive() const;
};

#endif
