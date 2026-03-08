#include "AlarmSystem.h"
#include <string.h>
#include <stdio.h>

AlarmSystem::AlarmSystem()
    : doorSensor  ("DS1", "Front Door",  PIN_DOOR_SENSOR),
      windowSensor("WS1", "Main Window", PIN_WINDOW_SENSOR),
      motionSensor("MS1", "Hallway",     PIN_PIR_SENSOR),
      exitButton  (PIN_EXIT_BUTTON),
      solenoid    (PIN_SOLENOID),
      buzzer      (PIN_BUZZER),
      currentMode (SystemMode::IDLE),
      modeStartMs (0),
      lastFlashMs (0),
      flashState  (false),
      serialLen   (0)
{
    memset(serialBuf, 0, sizeof(serialBuf));
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::begin() {
    Serial.begin(9600);

    pinManager.begin();
    accessibility.begin();

    pinMode(PIN_LED_RED,   OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);

    solenoid.lock();
    setLed(PIN_LED_GREEN, true);
    setLed(PIN_LED_RED,   false);

    tx("STATUS:READY");
    tx("INFO:Default PIN is 123456 - change immediately");
    tx("HELP:ARM_FULL|ARM_HOME|DISARM:<pin>|CANCEL_ALARM:<pin>|CHANGE_PIN:<old>:<new>|ENROL_FACE|SET_TIMEOUT:<s>");
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::update() {
    handleSerial();
    doorSensor.checkStatus();
    windowSensor.checkStatus();
    motionSensor.checkStatus();
    exitButton.checkStatus();
    handleSensors();
    handleTimers();
    handleFlash();
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::handleSensors() {
    // Exit button: request face verification during entry delay
    if (exitButton.isPressed()) {
        intrusionLog.logEvent("Exit button pressed");
        if (currentMode == SystemMode::ENTRY_DELAY ||
            currentMode == SystemMode::ALARM_ACTIVE) {
            tx("CMD:FACE_VERIFY");
            enterMode(SystemMode::AWAITING_FACE);
        }
    }

    if (currentMode != SystemMode::ARMED_FULL &&
        currentMode != SystemMode::ARMED_HOME) return;

    if (doorSensor.isTriggered()) {
        doorSensor.reset();
        intrusionLog.logEvent("Door sensor triggered");
        tx("EVENT:DOOR_OPEN");
        tx("CMD:NOTIFY:Door opened while armed");
        enterMode(SystemMode::ENTRY_DELAY);
    }

    if (windowSensor.isTriggered()) {
        windowSensor.reset();
        intrusionLog.logEvent("Window sensor triggered");
        tx("EVENT:WINDOW_OPEN");
        tx("CMD:NOTIFY:Window opened while armed");
        enterMode(SystemMode::ENTRY_DELAY);
    }

    // PIR only active in ARMED_FULL (not home mode)
    if (currentMode == SystemMode::ARMED_FULL &&
        motionSensor.isTriggered()) {
        motionSensor.reset();
        intrusionLog.logEvent("Motion detected");
        tx("EVENT:MOTION_DETECTED");
        tx("CMD:NOTIFY:Motion detected");
        triggerAlarm("MOTION");
    }
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::handleTimers() {
    unsigned long elapsed = millis() - modeStartMs;
    unsigned long delay_ms = (unsigned long)accessibility.getTimeout() * 1000UL;

    if (currentMode == SystemMode::ENTRY_DELAY ||
        currentMode == SystemMode::AWAITING_FACE) {
        long remaining = (long)(delay_ms - elapsed) / 1000L;
        if (remaining <= 0) {
            triggerAlarm("ENTRY_TIMEOUT");
        } else {
            static long lastPrint = -1;
            if (remaining != lastPrint) {
                char buf[24];
                snprintf(buf, sizeof(buf), "COUNTDOWN:%ld", remaining);
                tx(buf);
                lastPrint = remaining;
            }
        }
        return;
    }

    if (currentMode == SystemMode::EXIT_DELAY) {
        long remaining = (long)(delay_ms - elapsed) / 1000L;
        if (remaining <= 0) {
            bool doorOpen   = (digitalRead(PIN_DOOR_SENSOR)   == HIGH);
            bool windowOpen = (digitalRead(PIN_WINDOW_SENSOR) == HIGH);
            if (doorOpen || windowOpen) {
                tx("STATUS:ARM_FAILED:Door or window still open");
                enterMode(SystemMode::IDLE);
            } else {
                enterMode(SystemMode::ARMED_FULL);
                tx("STATUS:ARMED_FULL");
            }
        } else {
            static long lastPrint = -1;
            if (remaining != lastPrint) {
                char buf[28];
                snprintf(buf, sizeof(buf), "EXIT_COUNTDOWN:%ld", remaining);
                tx(buf);
                lastPrint = remaining;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::handleFlash() {
    if (currentMode != SystemMode::ALARM_ACTIVE &&
        currentMode != SystemMode::ENTRY_DELAY)  return;
    if (millis() - lastFlashMs >= 400) {
        flashState  = !flashState;
        lastFlashMs = millis();
        setLed(PIN_LED_RED, flashState);
    }
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::handleSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (serialLen > 0) {
                serialBuf[serialLen] = '\0';
                processCommand(serialBuf);
                serialLen = 0;
            }
        } else if (serialLen < (uint8_t)(sizeof(serialBuf) - 1)) {
            serialBuf[serialLen++] = c;
        }
    }
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::processCommand(const char* cmd) {
    if (strcmp(cmd, "ARM_FULL") == 0)  { armFull(); return; }
    if (strcmp(cmd, "ARM_HOME") == 0)  { armHome(); return; }

    if (strncmp(cmd, "DISARM:", 7) == 0) {
        disarm(cmd + 7); return;
    }
    if (strncmp(cmd, "CANCEL_ALARM:", 13) == 0) {
        cancelAlarm(cmd + 13); return;
    }
    if (strncmp(cmd, "CHANGE_PIN:", 11) == 0) {
        const char* rest = cmd + 11;
        const char* sep  = strchr(rest, ':');
        if (!sep) { tx("ERROR:Format is CHANGE_PIN:<old>:<new>"); return; }
        char oldP[PIN_LENGTH + 1], newP[PIN_LENGTH + 1];
        int  oldLen = sep - rest;
        strncpy(oldP, rest, oldLen); oldP[oldLen] = '\0';
        strncpy(newP, sep + 1, PIN_LENGTH); newP[PIN_LENGTH] = '\0';
        if (pinManager.changePIN(oldP, newP)) tx("STATUS:PIN_CHANGED");
        else                                   tx("STATUS:PIN_CHANGE_FAILED");
        return;
    }
    if (strcmp(cmd, "FACE:MATCH") == 0) {
        if (currentMode == SystemMode::AWAITING_FACE ||
            currentMode == SystemMode::ENTRY_DELAY ||
            currentMode == SystemMode::ALARM_ACTIVE) {
            buzzer.deactivate();
            solenoid.unlock();
            setLed(PIN_LED_RED,   false);
            setLed(PIN_LED_GREEN, true);
            enterMode(SystemMode::IDLE);
            intrusionLog.logEvent("Disarmed via face recognition");
            tx("STATUS:DISARMED:FACE_AUTH");
        }
        return;
    }
    if (strcmp(cmd, "FACE:NO_MATCH") == 0) {
        if (currentMode == SystemMode::AWAITING_FACE)
            enterMode(SystemMode::ENTRY_DELAY);
        tx("STATUS:FACE_FAILED");
        return;
    }
    if (strcmp(cmd, "FACE:ENROLLED") == 0) {
        tx("STATUS:FACE_ENROLLED"); return;
    }
    if (strcmp(cmd, "ENROL_FACE") == 0) {
        tx("CMD:FACE_ENROL"); return;
    }
    if (strncmp(cmd, "SET_TIMEOUT:", 12) == 0) {
        int t = atoi(cmd + 12);
        if (t >= 5 && t <= 120) {
            accessibility.adjustTimeout((uint8_t)t);
            tx("STATUS:TIMEOUT_SET");
        }
        return;
    }
    if (strcmp(cmd, "STATUS") == 0) {
        char buf[20];
        snprintf(buf, sizeof(buf), "MODE:%d", (uint8_t)currentMode);
        tx(buf);
        return;
    }
    tx("ERROR:Unknown command");
}

// ─────────────────────────────────────────────────────────────
void AlarmSystem::armFull() {
    if (currentMode != SystemMode::IDLE) { tx("STATUS:ARM_REJECTED:Not idle"); return; }
    if (digitalRead(PIN_DOOR_SENSOR)   == HIGH ||
        digitalRead(PIN_WINDOW_SENSOR) == HIGH) {
        tx("STATUS:ARM_REJECTED:Close door and window first"); return;
    }
    solenoid.lock();
    setLed(PIN_LED_GREEN, false);
    enterMode(SystemMode::EXIT_DELAY);
    tx("STATUS:EXIT_DELAY_STARTED");
}

void AlarmSystem::armHome() {
    if (currentMode != SystemMode::IDLE) { tx("STATUS:ARM_REJECTED:Not idle"); return; }
    solenoid.lock();
    setLed(PIN_LED_GREEN, true);
    enterMode(SystemMode::ARMED_HOME);
    tx("STATUS:ARMED_HOME");
}

void AlarmSystem::disarm(const char* entered) {
    if (pinManager.isLocked()) { tx("STATUS:PIN_LOCKED"); return; }
    if (pinManager.validatePIN(entered)) {
        buzzer.deactivate();
        solenoid.unlock();
        setLed(PIN_LED_RED,   false);
        setLed(PIN_LED_GREEN, true);
        enterMode(SystemMode::IDLE);
        intrusionLog.logEvent("Disarmed via PIN");
        tx("STATUS:DISARMED");
    } else {
        char buf[48];
        if (pinManager.isLocked()) {
            tx("STATUS:PIN_LOCKED:5 minute lockout started");
        } else {
            snprintf(buf, sizeof(buf), "STATUS:WRONG_PIN:Attempt %d of %d",
                     pinManager.getAttemptCount(), MAX_PIN_ATTEMPTS);
            tx(buf);
        }
    }
}

void AlarmSystem::triggerAlarm(const char* reason) {
    buzzer.activate();
    setLed(PIN_LED_RED,   true);
    setLed(PIN_LED_GREEN, false);
    enterMode(SystemMode::ALARM_ACTIVE);
    char buf[64];
    snprintf(buf, sizeof(buf), "STATUS:ALARM_TRIGGERED:%s", reason);
    tx(buf);
    snprintf(buf, sizeof(buf), "CMD:NOTIFY:ALARM - %s", reason);
    tx(buf);
    tx("CMD:CALL_EMERGENCY");
    intrusionLog.logEvent(reason);
}

void AlarmSystem::cancelAlarm(const char* entered) {
    if (currentMode != SystemMode::ALARM_ACTIVE) { tx("STATUS:NOT_IN_ALARM"); return; }
    if (pinManager.validatePIN(entered)) {
        buzzer.deactivate();
        solenoid.unlock();
        setLed(PIN_LED_RED,   false);
        setLed(PIN_LED_GREEN, true);
        enterMode(SystemMode::IDLE);
        intrusionLog.logEvent("Alarm cancelled by PIN");
        tx("STATUS:ALARM_CANCELLED");
    } else {
        tx("STATUS:WRONG_PIN");
    }
}

void AlarmSystem::enterMode(SystemMode m) { currentMode = m; modeStartMs = millis(); }
void AlarmSystem::setLed(uint8_t p, bool on) { digitalWrite(p, on ? HIGH : LOW); }
void AlarmSystem::tx(const char* msg)        { Serial.println(msg); }
