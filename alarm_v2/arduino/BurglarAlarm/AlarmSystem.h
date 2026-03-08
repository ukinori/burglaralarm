#ifndef ALARM_SYSTEM_H
#define ALARM_SYSTEM_H
#include <Arduino.h>
#include "DoorSensor.h"
#include "WindowSensor.h"
#include "MotionSensor.h"
#include "ExitButton.h"
#include "PINManager.h"
#include "Solenoid.h"
#include "Buzzer.h"
#include "IntrusionLog.h"
#include "AccessibilitySettings.h"

// ── Pin assignments ───────────────────────────────────────────
#define PIN_DOOR_SENSOR     2
#define PIN_WINDOW_SENSOR   3
#define PIN_PIR_SENSOR      4
#define PIN_EXIT_BUTTON     5
#define PIN_SOLENOID        8
#define PIN_BUZZER          9
#define PIN_LED_RED        10
#define PIN_LED_GREEN      11

// ── System mode ───────────────────────────────────────────────
enum class SystemMode : uint8_t {
    IDLE          = 0,
    EXIT_DELAY    = 1,
    ARMED_FULL    = 2,
    ARMED_HOME    = 3,
    ENTRY_DELAY   = 4,
    ALARM_ACTIVE  = 5,
    PIN_LOCKED    = 6,
    AWAITING_FACE = 7
};

/**
 * AlarmSystem  –  top-level controller
 *
 * Owns all hardware objects (composition).
 * Uses IntrusionLog, NotificationService, AccessibilitySettings (dependency).
 * All user I/O is over Serial to the PC; no LCD or physical keypad.
 */
class AlarmSystem {
private:
    // ── Composed objects (owned by AlarmSystem) ────────────
    DoorSensor            doorSensor;
    WindowSensor          windowSensor;
    MotionSensor          motionSensor;
    ExitButton            exitButton;
    PINManager            pinManager;
    Solenoid              solenoid;
    Buzzer                buzzer;
    IntrusionLog          intrusionLog;
    AccessibilitySettings accessibility;

    // ── State ───────────────────────────────────────────────
    SystemMode    currentMode;
    unsigned long modeStartMs;

    // ── LED flash ────────────────────────────────────────────
    unsigned long lastFlashMs;
    bool          flashState;

    // ── Serial input buffer ──────────────────────────────────
    char    serialBuf[64];
    uint8_t serialLen;

    // ── Private methods ──────────────────────────────────────
    void handleSensors();
    void handleSerial();
    void handleTimers();
    void handleFlash();

    void enterMode(SystemMode newMode);
    void processCommand(const char* cmd);

    void armFull();
    void armHome();
    void disarm(const char* enteredPin);
    void triggerAlarm(const char* reason);
    void cancelAlarm(const char* enteredPin);

    void setLed(uint8_t ledPin, bool on);
    void tx(const char* msg);

public:
    AlarmSystem();
    void begin();
    void update();
};

#endif
