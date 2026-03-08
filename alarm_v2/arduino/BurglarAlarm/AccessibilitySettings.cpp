#include "AccessibilitySettings.h"

AccessibilitySettings::AccessibilitySettings()
    : timeoutSeconds(30), fontSize(2), highContrast(false), language(0) {}

void AccessibilitySettings::begin() {
    loadFromEEPROM();
}

void AccessibilitySettings::loadFromEEPROM() {
    uint8_t t = EEPROM.read(EEPROM_ACC_ADDR);
    if (t >= 5 && t <= 120) timeoutSeconds = t;

    uint8_t f = EEPROM.read(EEPROM_ACC_ADDR + 1);
    if (f >= 1 && f <= 3) fontSize = f;

    uint8_t h = EEPROM.read(EEPROM_ACC_ADDR + 2);
    if (h <= 1) highContrast = (bool)h;

    uint8_t l = EEPROM.read(EEPROM_ACC_ADDR + 3);
    if (l <= 9) language = l;
}

void AccessibilitySettings::saveToEEPROM() {
    EEPROM.update(EEPROM_ACC_ADDR,     timeoutSeconds);
    EEPROM.update(EEPROM_ACC_ADDR + 1, fontSize);
    EEPROM.update(EEPROM_ACC_ADDR + 2, (uint8_t)highContrast);
    EEPROM.update(EEPROM_ACC_ADDR + 3, language);
}

void AccessibilitySettings::adjustTimeout(uint8_t s) {
    if (s >= 5 && s <= 120) { timeoutSeconds = s; saveToEEPROM(); }
}
void AccessibilitySettings::adjustFontSize(uint8_t sz) {
    if (sz >= 1 && sz <= 3) { fontSize = sz; saveToEEPROM(); }
}
void AccessibilitySettings::toggleHighContrast() {
    highContrast = !highContrast; saveToEEPROM();
}
void AccessibilitySettings::setLanguage(uint8_t l) {
    language = l; saveToEEPROM();
}

uint8_t AccessibilitySettings::getTimeout()      const { return timeoutSeconds; }
uint8_t AccessibilitySettings::getFontSize()     const { return fontSize; }
bool    AccessibilitySettings::getHighContrast() const { return highContrast; }
uint8_t AccessibilitySettings::getLanguage()     const { return language; }
