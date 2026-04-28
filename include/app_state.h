#ifndef APP_STATE_H
#define APP_STATE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// Servo and door sensor pins
extern const int servo_pin;
extern const int motionTriggerPin;
extern const int motionEchoPin;

// Gas sensor pins
extern const int MQ2_ANALOG_PIN;

// WiFi credentials
extern const char *ssid;
extern const char *pass;

// MQTT server config
extern const char *mqtt_server;
extern const uint16_t mqtt_port;

// MQTT topics
extern const char *subCmdTopic;
extern const char *subStateTopic;

extern const char *reqOutsideStateTopic;
extern const char *reqInsideStateTopic;

extern const char *cmdOutsideTopic;
extern const char *stateOutsideTopic;

extern const char *cmdInsideTopic;
extern const char *stateInsideTopic;

extern const char *cmdDoorTopic;
extern const char *stateDoorTopic;

extern const char *cmdDoorLockTopic;
extern const char *stateDoorLockTopic;

extern const char *pubTopic;

// LED pins
extern const int OUT_LED_PIN;
extern const int IN_LED_PIN;

// EEPROM addresses and markers
extern const uint8_t EEPROM_MAGIC;
extern const int EEPROM_ADDR_MAGIC;
extern const int EEPROM_ADDR_OUTSIDE;
extern const int EEPROM_ADDR_INSIDE;
extern const int EEPROM_ADDR_DOOR;
extern const int EEPROM_ADDR_DOOR_LOCK;

// Runtime state
extern bool outsideLedOn;
extern bool insideLedOn;
extern bool doorOpenState;
extern bool doorLockedState;

extern WiFiClient espClient;
extern PubSubClient client;
extern Servo servo;

extern unsigned long lastMqttAttempt;
extern const unsigned long MQTT_RETRY_INTERVAL_MS;
extern unsigned long lastDebug;
extern unsigned long lastHeartbeat;
extern unsigned long doorAutoCloseTimer;
extern bool doorAutoCloseArmed;

#endif
