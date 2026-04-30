#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include <Arduino.h>

/**
 * Initialize WiFi connection
 */
void setup_wifi();

/**
 * Initialize MQTT client and callbacks
 */
void initMqtt();

/**
 * Ensure MQTT is connected, attempt to reconnect if needed
 */
void ensureMqttConnected();

/**
 * MQTT message callback handler
 */
void mqttCallback(char *topic, byte *payload, unsigned int length);

#endif
