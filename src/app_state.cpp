#include "app_state.h"

// Servo and door sensor pins
const int servo_pin = 13;
const int motionTriggerPin = 14;
const int motionEchoPin = 12;

// Gas sensor pins
const int MQ2_ANALOG_PIN = A0;

// WiFi credentials
const char *ssid = "MAJCHROCIK";
const char *pass = "dazdnik1";

// MQTT server config
const char *mqtt_server = "10.195.236.205";
const uint16_t mqtt_port = 1883;

// MQTT topics
const char *subCmdTopic = "cmd/#";
const char *subStateTopic = "stat/#";

const char *reqOutsideStateTopic = "cmd/ledOUTSIDE/POWER/get";
const char *reqInsideStateTopic = "cmd/ledINSIDE/POWER/get";

const char *cmdOutsideTopic = "cmd/ledOUTSIDE/POWER";
const char *cmdInsideTopic = "cmd/ledINSIDE/POWER";

const char *stateOutsideTopic = "stat/ledOUTSIDE/POWER";
const char *stateInsideTopic = "stat/ledINSIDE/POWER";

// this sends message -> door open || door close
const char *cmdDoorTopic = "cmd/door/OPEN";
const char *stateDoorTopic = "stat/door/OPEN";


// door lock -> message to openHabian
const char *cmdDoorLockTopic = "cmd/door/LOCK";
const char *stateDoorLockTopic = "stat/door/LOCK";

// this shit communicates with mqtt aka openHabian, it sends shit 
// and signals so it knows when its on or off type shit
const char *cmdTripwireTopic = "cmd/alarm/ON";
const char *stateTripwireTopic = "stat/alarm/ON";

// what the fuck is this
const char *pubTopic = "status/esp8266/online";

// LED pins
const int OUT_LED_PIN = 5;
const int IN_LED_PIN = 4;

//TRIPWIRE pins
const int OUT_LASER = 10;
const int IN_FOTORES = 9;

// EEPROM addresses and markers | what-fucking-ever this means
const uint8_t EEPROM_MAGIC = 0xA5;
const int EEPROM_ADDR_MAGIC = 0;
const int EEPROM_ADDR_OUTSIDE = 1;
const int EEPROM_ADDR_INSIDE = 2;
const int EEPROM_ADDR_DOOR = 3;
const int EEPROM_ADDR_DOOR_LOCK = 4;

// Runtime state
bool outsideLedOn = false;
bool insideLedOn = false;
bool doorOpenState = false;
bool doorLockedState = false;

WiFiClient espClient;
PubSubClient client(espClient);
Servo servo;

unsigned long lastMqttAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;
unsigned long lastDebug = 0;
unsigned long lastHeartbeat = 0;
unsigned long doorAutoCloseTimer = 0;
bool doorAutoCloseArmed = false;
