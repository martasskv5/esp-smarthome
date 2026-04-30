#include "laser_tripwire.h"
#include "app_state.h"
#include "utils.h"

// Laser brightness (PWM value, 0-255)
const int laserBrightness = 255;

// Buzzer state tracking
static unsigned long buzzerEndTime = 0;
static bool buzzerActive = false;

/**
 * Initialize laser tripwire hardware:
 * - Laser pin as OUTPUT
 * - Sensor pin as INPUT
 * - Buzzer pin as OUTPUT
 * - Power on the laser
 */
void initLaserTripwire()
{
    Serial.println("Initializing laser tripwire alarm system...");

    pinMode(laserPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    // A0 is analog input, no need to set pinMode

    // Turn off the laser
    digitalWrite(laserPin, LOW);

    // Buzzer off initially
    digitalWrite(buzzerPin, HIGH);

    // Wait for laser to stabilize
    delay(500);

    long sum = 0;
    const int samples = 20;
    for (int i = 0; i < samples; i++)
    {
        sum += analogRead(sensorPin);
        delay(10);
    }
    sensorBaseline = sum / samples;

    Serial.println("Laser tripwire alarm system initialized successfully");
    Serial.print("  Laser pin: ");
    Serial.println(laserPin);
    Serial.print("  Sensor pin (A0): ");
    Serial.println(sensorPin);
    Serial.print("  Buzzer pin: ");
    Serial.println(buzzerPin);
    Serial.print("  Sensor baseline: ");
    Serial.println(sensorBaseline);
    Serial.print("  Trigger delta: ");
    Serial.println(SENSOR_THRESHOLD_DELTA);

    // Debug: Check laser pin state and sensor reading
    Serial.print("  Laser GPIO state: ");
    Serial.println(digitalRead(laserPin) ? "HIGH (ON)" : "LOW (OFF)");
    delay(100);
    int initialReading = analogRead(sensorPin);
    Serial.print("  Initial sensor reading: ");
    Serial.println(initialReading);

    // Hardware diagnostic: Read A0 multiple times to verify connection
    Serial.println("\n[HARDWARE CHECK] Reading A0 10 times:");
    for (int i = 0; i < 10; i++)
    {
        int reading = analogRead(sensorPin);
        Serial.print("  Read ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(reading);
        delay(50);
    }
    Serial.println("[END HARDWARE CHECK]\n");
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
    if (now - lastLaserCheck < LASER_CHECK_INTERVAL_MS || !LockedState)
    {
        return;
    }

    lastLaserCheck = now;

    // Debug output (every 1 second)
    static unsigned long lastDebugOutput = 0;
    if (now - lastDebugOutput > 1000)
    {
        lastDebugOutput = now;
        int sensorValue = analogRead(sensorPin);
        Serial.print("[SENSOR DEBUG] Raw value: ");
        Serial.print(sensorValue);
        Serial.print(" | Baseline: ");
        Serial.print(sensorBaseline);
        Serial.print(" | Delta: ");
        Serial.print(abs(sensorValue - sensorBaseline));
        Serial.print(" | Trigger delta: ");
        Serial.print(SENSOR_THRESHOLD_DELTA);
        Serial.print(" | Trigger level: ");
        Serial.print(TRIGGER_LEVEL);
        Serial.print(" | Status: ");
        Serial.println(sensorValue > TRIGGER_LEVEL ? "BEAM BROKEN (above trigger)" : "BEAM OK");
    }

    // Primary check: absolute trigger level (user requested behavior)
    int sensorValue = analogRead(sensorPin);
    if (sensorValue > TRIGGER_LEVEL)
    {
        // Prevent multiple triggers within 5 seconds
        if (now - lastTriggerTime > 1000)
        {
            Serial.println("ALARM: Laser beam broken!");

            // Sound the buzzer for 2 seconds
            soundBuzzer(2000);

            // Publish alarm event via MQTT
            if (client.connected())
            {
                client.publish(laserAlarmTopic, "1");
                Serial.println("Published laser alarm to MQTT");
            }
            else
            {
                Serial.println("MQTT not connected, alarm not published");
            }

            lastTriggerTime = now;
        }
    }

    // Handle buzzer timeout
    if (buzzerActive && now >= buzzerEndTime)
    {
        stopBuzzer();
        // Publish alarm event via MQTT
        if (client.connected())
        {
            client.publish(laserAlarmTopic, "0");
            Serial.println("Published laser alarm to MQTT");
        }
        else
        {
            Serial.println("MQTT not connected, alarm not published");
        }
    }
}

/**
 * Sound the buzzer for the specified duration
 */
void soundBuzzer(unsigned long durationMs)
{
    Serial.print("Sounding buzzer for ");
    Serial.print(durationMs);
    Serial.println("ms");

    digitalWrite(buzzerPin, LOW);
    buzzerActive = true;
    buzzerEndTime = millis() + durationMs;
}

/**
 * Stop the buzzer
 */
void stopBuzzer()
{
    if (buzzerActive)
    {
        Serial.println("Stopping buzzer");
        digitalWrite(buzzerPin, HIGH);
        buzzerActive = false;
    }
}

void handleLockCommand(byte *payload, unsigned int length)
{
    Serial.println("Received lock command via MQTT");
    bool on = payloadIsOn(payload, length);
    if (on)
    {
        Serial.println("Locking...");
        LockedState = true;
        digitalWrite(laserPin, HIGH); // Turn off laser when locked
    }
    else
    {
        Serial.println("Unlocking...");
        LockedState = false;
        digitalWrite(laserPin, LOW); // Turn on laser when unlocked
    }
}
