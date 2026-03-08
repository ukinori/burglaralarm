/*
 * ============================================================
 *  Burglar Alarm System  -  Arduino Mega 2560
 * ============================================================
 *  Components:
 *   D2  RS Pro Reed Switch (door)        INPUT_PULLUP
 *   D3  RS Pro Reed Switch (window)      INPUT_PULLUP
 *   D4  TruSens HC-SR505 PIR OUT         INPUT
 *   D5  APEM PHAP3301 exit button        INPUT_PULLUP
 *   D8  BD911 NPN base via 220 ohm       OUTPUT
 *   D9  RS PRO piezo buzzer 78dB         OUTPUT
 *   D10 Red LED via 220 ohm              OUTPUT
 *   D11 Green LED via 220 ohm            OUTPUT
 *
 *  No LCD. No physical keypad.
 *  All user I/O via USB Serial to PC running main.py
 * ============================================================
 */

#include "AlarmSystem.h"

AlarmSystem alarm;

void setup() {
    alarm.begin();
}

void loop() {
    alarm.update();
}
