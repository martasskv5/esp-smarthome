#include <Arduino.h>
#include "app_state.h"
#include "wifi_mqtt.h"
#include "laser_tripwire.h"

void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("\n\nStarting ESP8266 Laser Alarm System...");

    // Initialize laser tripwire alarm
    initLaserTripwire();

    // Initialize WiFi and MQTT
    setup_wifi();
    initMqtt();
}

void loop()
{
    // Maintain WiFi connection
    if (WiFi.status() != WL_CONNECTED)
    {
        setup_wifi();
    }

    // Maintain MQTT connection and process messages
    ensureMqttConnected();
    client.loop();

    // Check laser beam status
    checkLaserBeam();
}
