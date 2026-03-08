#include "Sensor.h"
#include <string.h>

Sensor::Sensor(const char* id, const char* loc, uint8_t p)
    : isTriggeredFlag(false), pin(p)
{
    strncpy(sensorId, id,  sizeof(sensorId)  - 1); sensorId[sizeof(sensorId)-1]  = '\0';
    strncpy(location, loc, sizeof(location)  - 1); location[sizeof(location)-1]  = '\0';
    pinMode(pin, INPUT_PULLUP);
}

void        Sensor::trigger()              { isTriggeredFlag = true; }
void        Sensor::reset()               { isTriggeredFlag = false; }
bool        Sensor::isTriggered()  const  { return isTriggeredFlag; }
const char* Sensor::getSensorId()  const  { return sensorId; }
const char* Sensor::getLocation()  const  { return location; }
uint8_t     Sensor::getPin()       const  { return pin; }
