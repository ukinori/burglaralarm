#ifndef ACCESSIBILITY_SETTINGS_H
#define ACCESSIBILITY_SETTINGS_H
#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_ACC_ADDR  8    // 4 bytes: timeout(1) | fontSize(1) | highContrast(1) | lang(1)

/**
 * AccessibilitySettings
 * User preferences stored in EEPROM so they persist after power-off.
 */
class AccessibilitySettings {
private:
    uint8_t timeoutSeconds;  // entry/exit delay (5–120 s), default 30
    uint8_t fontSize;        // 1=small 2=medium 3=large
    bool    highContrast;
    uint8_t language;        // 0=EN, 1=FR, 2=DE ...

    void loadFromEEPROM();
    void saveToEEPROM();

public:
    AccessibilitySettings();
    void begin();

    void    adjustTimeout(uint8_t seconds);
    void    adjustFontSize(uint8_t size);
    void    toggleHighContrast();
    void    setLanguage(uint8_t lang);

    uint8_t getTimeout()      const;
    uint8_t getFontSize()     const;
    bool    getHighContrast() const;
    uint8_t getLanguage()     const;
};

#endif
