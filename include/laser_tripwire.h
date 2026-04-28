#ifndef LASER_TRIPWIRE_H
#define LASER_TRIPWIRE_H

#include <Arduino.h>

/**
 * Initialize laser tripwire hardware
 * - Sets up laser output pin
 * - Initializes sensor input reading
 * - Powers on the laser
 */
void initLaserTripwire();

/**
 * Check laser beam status
 * - Reads sensor value
 * - Detects if beam is broken (value drops below threshold)
 * - Publishes MQTT alarm message if beam is broken
 * - Should be called regularly from the main loop
 */
void checkLaserBeam();

/**
 * Calibrate the laser sensor baseline
 * Call this function when you want to set the current light level as the "beam intact" baseline
 */
void calibrateLaserSensor();

#endif