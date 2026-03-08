#include "Buzzer.h"

Buzzer::Buzzer(uint8_t p) : pin(p), activeState(false) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Buzzer::activate() {
    activeState = true;
    digitalWrite(pin, HIGH);
}

void Buzzer::deactivate() {
    activeState = false;
    digitalWrite(pin, LOW);
}

bool Buzzer::isActive() const { return activeState; }
