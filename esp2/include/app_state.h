#ifndef APP_STATE_H
#define APP_STATE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Alarm hardware pins
extern const int laserPin;         // GPIO5 - controls the laser
extern const int sensorPin;        // A0 (GPIO17) - analog photoresistor sensor
extern const int buzzerPin;        // GPIO14 - buzzer for alarm
extern const int SENSOR_THRESHOLD_DELTA; // Allowed deviation from baseline before alarm
extern int sensorBaseline;               // Startup-calibrated sensor value
extern const int TRIGGER_LEVEL;          // Absolute sensor value threshold to trigger alarm

// WiFi credentials
extern const char *ssid;
extern const char *pass;

// MQTT server config
extern const char *mqtt_server;
extern const uint16_t mqtt_port;

// MQTT topics
extern const char *laserAlarmTopic;      // Topic to publish laser alarm
extern const char *pubTopic;             // Online status topic
extern const char *subCmdTopic;          // Subscribe to commands

// MQTT client
extern WiFiClient espClient;
extern PubSubClient client;

// Timing constants
extern unsigned long lastMqttAttempt;
extern const unsigned long MQTT_RETRY_INTERVAL_MS;
extern unsigned long lastLaserCheck;
extern const unsigned long LASER_CHECK_INTERVAL_MS;

extern const char *cmdDoorLockTopic;
extern const char *stateDoorLockTopic;
extern bool LockedState;

#endif
