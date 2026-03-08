#ifndef WINDOW_SENSOR_H
#define WINDOW_SENSOR_H
#include "Sensor.h"

/**
 * WindowSensor  –  RS Pro Reed Switch (identical wiring to DoorSensor)
 * INPUT_PULLUP: LOW = window closed, HIGH = window open (breach)
 */
class WindowSensor : public Sensor {
private:
    bool isWindowOpen;
    bool lastState;

public:
    WindowSensor(const char* id, const char* loc, uint8_t pin);

    void checkStatus() override;
    bool getWindowOpen() const;
    void onOpen();
};

#endif
