#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H
#include <Arduino.h>
#include <EEPROM.h>

#define PIN_LENGTH          6
#define MAX_PIN_ATTEMPTS    5
#define PIN_LOCKOUT_MS      300000UL  // 5 minutes
#define EEPROM_PIN_ADDR     0         // 6 bytes

/**
 * PINManager
 * Stores the 6-digit PIN in EEPROM.
 * Enforces a 5-attempt lockout for 5 minutes on repeated failures.
 */
class PINManager {
private:
    char          pin[PIN_LENGTH + 1];
    int           attemptCount;
    bool          lockedOut;
    unsigned long lockoutStartMs;

    void loadFromEEPROM();
    void saveToEEPROM();

public:
    PINManager();

    void begin();                                      // load PIN on startup
    bool validatePIN(const char* input);               // true = correct PIN
    bool changePIN(const char* oldPIN, const char* newPIN);
    bool checkFormat(const char* p) const;             // 6 digits, no spaces
    void resetAttempts();
    bool isLocked()       const;
    int  getAttemptCount() const;
};

#endif
