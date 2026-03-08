#ifndef SENSOR_H
#define SENSOR_H
#include <Arduino.h>

/**
 * Sensor  (abstract base class)
 * All physical sensors inherit from this class.
 * Every subclass MUST implement checkStatus().
 */
class Sensor {
protected:
    char          sensorId[8];
    bool          isTriggeredFlag;
    char          location[24];
    uint8_t       pin;

public:
    Sensor(const char* id, const char* loc, uint8_t pin);

    // Pure virtual – must be implemented by every subclass
    virtual void  checkStatus() = 0;

    void          trigger();
    void          reset();
    bool          isTriggered()  const;
    const char*   getSensorId()  const;
    const char*   getLocation()  const;
    uint8_t       getPin()       const;
};

#endif
