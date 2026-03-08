#include "PINManager.h"
#include <string.h>

PINManager::PINManager()
    : attemptCount(0), lockedOut(false), lockoutStartMs(0)
{
    memset(pin, 0, sizeof(pin));
}

void PINManager::begin() {
    loadFromEEPROM();
}

void PINManager::loadFromEEPROM() {
    for (int i = 0; i < PIN_LENGTH; i++) {
        char c = (char)EEPROM.read(EEPROM_PIN_ADDR + i);
        pin[i] = (c >= '0' && c <= '9') ? c : '1';
    }
    pin[PIN_LENGTH] = '\0';
    // Default PIN on blank EEPROM
    if ((uint8_t)pin[0] == 0xFF || pin[0] == '\0') {
        strncpy(pin, "123456", PIN_LENGTH);
        saveToEEPROM();
    }
}

void PINManager::saveToEEPROM() {
    for (int i = 0; i < PIN_LENGTH; i++)
        EEPROM.update(EEPROM_PIN_ADDR + i, (uint8_t)pin[i]);
}

bool PINManager::checkFormat(const char* p) const {
    if (!p || strlen(p) != PIN_LENGTH) return false;
    for (int i = 0; i < PIN_LENGTH; i++)
        if (p[i] < '0' || p[i] > '9') return false;
    return true;
}

bool PINManager::validatePIN(const char* input) {
    // Auto-clear lockout after 5 minutes
    if (lockedOut) {
        if (millis() - lockoutStartMs >= PIN_LOCKOUT_MS) {
            lockedOut    = false;
            attemptCount = 0;
        } else {
            return false;  // still locked
        }
    }
    if (!checkFormat(input)) return false;
    if (strncmp(pin, input, PIN_LENGTH) == 0) {
        attemptCount = 0;
        return true;
    }
    attemptCount++;
    if (attemptCount >= MAX_PIN_ATTEMPTS) {
        lockedOut      = true;
        lockoutStartMs = millis();
    }
    return false;
}

bool PINManager::changePIN(const char* oldPIN, const char* newPIN) {
    if (!validatePIN(oldPIN))    return false;
    if (!checkFormat(newPIN))    return false;
    if (strncmp(pin, newPIN, PIN_LENGTH) == 0) return false;  // same PIN
    strncpy(pin, newPIN, PIN_LENGTH);
    pin[PIN_LENGTH] = '\0';
    saveToEEPROM();
    return true;
}

void PINManager::resetAttempts()      { attemptCount = 0; lockedOut = false; }
bool PINManager::isLocked()     const { return lockedOut; }
int  PINManager::getAttemptCount() const { return attemptCount; }
