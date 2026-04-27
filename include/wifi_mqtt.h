#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include <Arduino.h>

void setup_wifi();
void initMqtt();
void ensureMqttConnected();
void mqttCallback(char *topic, byte *payload, unsigned int length);

#endif
