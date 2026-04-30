#ifndef LASER_TRIPWIRE_H
#define LASER_TRIPWIRE_H

#include <Arduino.h>

/**
 * Initialize laser tripwire alarm system:
 * - Laser pin as OUTPUT
 * - Sensor pin as INPUT
 * - Buzzer pin as OUTPUT
 * - Power on the laser
 */
void initLaserTripwire();

/**
 * Check if the laser beam has been broken
 * - Reads sensor value
 * - Triggers buzzer and MQTT alarm if beam is broken
 * - Should be called regularly from the main loop
 */
void checkLaserBeam();

/**
 * Sound the buzzer for alarm
 */
void soundBuzzer(unsigned long durationMs);

/**
 * Stop the buzzer
 */
void stopBuzzer();

void handleLockCommand(byte *payload, unsigned int length);

#endif
