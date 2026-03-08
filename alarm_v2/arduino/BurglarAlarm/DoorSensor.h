#ifndef DOOR_SENSOR_H
#define DOOR_SENSOR_H
#include "Sensor.h"

/**
 * DoorSensor  –  RS Pro Reed Switch
 * INPUT_PULLUP: LOW = door closed (secure)
 *               HIGH = door open  (breach)
 */
class DoorSensor : public Sensor {
private:
    bool isDoorOpen;
    bool lastState;

public:
    DoorSensor(const char* id, const char* loc, uint8_t pin);

    void checkStatus() override;  // reads pin, sets trigger on opening edge
    bool getDoorOpen() const;
    void onOpen();
};

#endif
