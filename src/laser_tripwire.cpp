#include "laser_tripwire.h"

#include "app_state.h"

// Laser brightness (PWM value)
const int laserBrightness = 500;

/**
 * Initialize laser tripwire hardware:
 * - Laser pin as OUTPUT
 * - Sensor pin as INPUT
 * - Power on the laser
 */
void initLaserTripwire()
{
    Serial.println("Initializing laser tripwire...");

    pinMode(laserPin, OUTPUT);
    pinMode(sensorPin, INPUT);

    // Turn on the laser with PWM
    analogWrite(laserPin, laserBrightness);

    // Wait for laser to stabilize
    delay(500);

    Serial.println("Laser tripwire initialized successfully");
}

/**
 * Check if the laser beam has been broken
 * This function should be called regularly from the main loop
 */
void checkLaserBeam()
{
    static unsigned long lastTriggerTime = 0;
    unsigned long now = millis();

    // Check at regular intervals
    if (now - lastLaserCheck < LASER_CHECK_INTERVAL_MS)
    {
        return;
    }

    lastLaserCheck = now;

    // Debug output (every 1 second)
    static unsigned long lastDebugOutput = 0;
    if (now - lastDebugOutput > 1000)
    {
        lastDebugOutput = now;
        Serial.print("Laser sensor: ");
        Serial.println(digitalRead(sensorPin) ? "HIGH (beam broken)" : "LOW (beam intact)");
    }

    // Check if beam is broken (sensor reads HIGH when beam is blocked)
    if (digitalRead(sensorPin) == HIGH)
    {
        // Prevent multiple triggers within 5 seconds
        if (now - lastTriggerTime > 5000)
        {
            Serial.println("ALARM: Laser beam broken!");

            // Publish alarm event via MQTT
            if (client.connected())
            {
                client.publish(laserAlarmTopic, "TRIGGERED");
                Serial.println("Published laser alarm to MQTT");
            }
            else
            {
                Serial.println("MQTT not connected, alarm not published");
            }

            lastTriggerTime = now;
        }
    }
}

/**
 * Calibrate the laser sensor (not needed for digital sensor)
 * This function is kept for compatibility
 */
void calibrateLaserSensor()
{
    Serial.println("Calibration not needed for digital sensor");
}
