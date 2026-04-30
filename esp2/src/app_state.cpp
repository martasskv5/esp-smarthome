#include "app_state.h"

// Alarm hardware pins
const int laserPin = 5;         // GPIO5 - controls the laser
const int sensorPin = A0;       // A0 (GPIO17) - analog photoresistor sensor
const int buzzerPin = 14;       // GPIO14 - buzzer for alarm

// Sensor calibration values
const int SENSOR_THRESHOLD_DELTA = 80; // Trigger when light deviates from baseline by this amount
// Absolute trigger level: when sensor reading rises above this value, alarm triggers
const int TRIGGER_LEVEL = 300;

// WiFi credentials
const char *ssid = "MAJCHROCIK";
const char *pass = "dazdnik1";

// MQTT server config
const char *mqtt_server = "10.195.236.205";
const uint16_t mqtt_port = 1883;

// MQTT topics for alarm system
const char *laserAlarmTopic = "stat/alarm/ON";
const char *pubTopic = "status/esp8266-alarm/online";
const char *subCmdTopic = "cmd/#";

// MQTT client instances
WiFiClient espClient;
PubSubClient client(espClient);

// Timing state
unsigned long lastMqttAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;
unsigned long lastLaserCheck = 0;
const unsigned long LASER_CHECK_INTERVAL_MS = 50; // Check every 50ms
int sensorBaseline = 0;

const char *cmdDoorLockTopic = "cmd/door/LOCK";
const char *stateDoorLockTopic = "stat/door/LOCK";
bool LockedState = false;