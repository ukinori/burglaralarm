#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H
#include "Sensor.h"

/**
 * MotionSensor  –  TruSens HC-SR505 Mini PIR
 *
 * The HC-SR505 holds its OUT pin HIGH for ~8 seconds after
 * detecting movement. This class latches on the rising edge
 * and applies a holdoff so the sustained HIGH does not
 * cause repeated triggers.
 *
 * Wiring:
 *   Pin 1  VCC  → 5V
 *   Pin 2  OUT  → Arduino digital pin (INPUT)
 *   Pin 3  GND  → GND
 */
class MotionSensor : public Sensor {
private:
    int           sensitivityLevel; // 1–10 (informational; not hardware-adjustable)
    bool          pirLatched;
    unsigned long pirLatchMs;
    unsigned long pirHoldoffMs;     // must be > sensor hold time (~8s); default 9000

public:
    MotionSensor(const char* id, const char* loc,
                 uint8_t pin, int sensitivity = 5,
                 unsigned long holdoffMs = 9000UL);

    void checkStatus() override;    // detect rising edge; handles holdoff
    void handleHoldoff();           // clears latch after holdoff expires
    void setSensitivity(int level);
    int  getSensitivity() const;
};

#endif
