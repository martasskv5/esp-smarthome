/*  ESP8266 + PubSubClient robust MQTT example
    - Unique client ID (prevents client-id collisions)
    - Reconnect with retry timer
    - Re-subscribe after every reconnect
    - Always runs client.loop()
    - Prints useful debug including client.state()
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <EEPROM.h>

// servo senzor
const int servo_pin = 13;
Servo servo;

// door motion sensor
const int motionTriggerPin = 14;
const int motionEchoPin = 12;

// gas senzor
const int MQ2_ANALOG_PIN = A0;

// ==== WiFi ====
const char *ssid = "MAJCHROCIK";
const char *pass = "dazdnik1";

// ==== MQTT ====
const char *mqtt_server = "10.195.236.205";
const uint16_t mqtt_port = 1883;

// Command topics (incoming desired state)
const char *subCmdTopic = "cmd/#";

// State topics (broker/device reports current state)
const char *subStateTopic = "stat/#";

// Explicit state request topics (published on connect)
const char *reqOutsideStateTopic = "cmd/ledOUTSIDE/POWER/get";
const char *reqInsideStateTopic = "cmd/ledINSIDE/POWER/get";

const char *cmdOutsideTopic = "cmd/ledOUTSIDE/POWER";
const char *cmdInsideTopic = "cmd/ledINSIDE/POWER";
const char *stateOutsideTopic = "stat/ledOUTSIDE/POWER";
const char *stateInsideTopic = "stat/ledINSIDE/POWER";

const char *cmdDoorTopic = "cmd/door/OPEN";
const char *stateDoorTopic = "stat/door/OPEN";

// (Optional) publish a heartbeat so you can see the ESP is alive
const char *pubTopic = "status/esp8266/online";

// GPIO pins
const int OUT_LED_PIN = 5; // D1 on NodeMCU (GPIO5)
const int IN_LED_PIN = 4;

const uint8_t EEPROM_MAGIC = 0xA5;
const int EEPROM_ADDR_MAGIC = 0;
const int EEPROM_ADDR_OUTSIDE = 1;
const int EEPROM_ADDR_INSIDE = 2;
const int EEPROM_ADDR_DOOR = 3;

bool outsideLedOn = false;
bool insideLedOn = false;
bool doorOpenState = false;
bool doorLockedState = false;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMqttAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

unsigned long lastDebug = 0;
unsigned long lastHeartbeat = 0;

void setup_wifi();
void ensureMqttConnected();
void callback(char *topic, byte *payload, unsigned int length);
bool payloadIsOn(const byte *payload, unsigned int length);
void applyLedState(int pin, const byte *payload, unsigned int length, const char *label);
void restoreLedStatesFromEeprom();
void persistLedStatesToEeprom();
void restoreDoorStatesFromEeprom();
void persistDoorStateToEeprom();

void setup()
{
    Serial.begin(115200);
    delay(50);

    // Initialize GPIO pins
    pinMode(OUT_LED_PIN, OUTPUT);
    pinMode(IN_LED_PIN, OUTPUT);
    EEPROM.begin(8);
    restoreLedStatesFromEeprom();
    pinMode(motionTriggerPin, OUTPUT);
    pinMode(motionEchoPin, INPUT);
    pinMode(servo_pin, OUTPUT);
    servo.attach(servo_pin);
    //   servo.write(-2); // Start with door closed
    restoreDoorStatesFromEeprom();
    Serial.println("Mq2 starting up, waiting 30 seconds");
    delay(30000);

    setup_wifi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    // PubSubClient timeouts are in SECONDS (not ms)
    client.setSocketTimeout(10); // seconds
    client.setKeepAlive(15);     // seconds
}

// Forward declarations
long measureDistance();

void setup_wifi()
{
    Serial.println();
    Serial.println("Starting WiFi connection...");

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, pass);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40)
    {
        delay(250);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
    }
    else
    {
        Serial.println("\nWiFi failed to connect (will keep trying in loop)");
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    // payload is NOT null-terminated
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Accept both command topics and reported state topics.
    // This allows restoring outputs after boot from retained state responses.
    if ((strcmp(topic, cmdOutsideTopic) == 0 || strcmp(topic, stateOutsideTopic) == 0) && length > 0)
    {
        applyLedState(OUT_LED_PIN, payload, length, "OUTSIDE");
    }

    if ((strcmp(topic, cmdInsideTopic) == 0 || strcmp(topic, stateInsideTopic) == 0) && length > 0)
    {
        applyLedState(IN_LED_PIN, payload, length, "INSIDE");
    }

    // door servo
    if ((strcmp(topic, cmdDoorTopic) == 0 || strcmp(topic, stateDoorTopic) == 0) && length > 0)
    {
        bool on = payloadIsOn(payload, length);
        if (on)
        {
            // Move from 0 to 135 degrees
            for (int pos = 0; pos <= 135; pos += 1)
            {
                yield();
                servo.write(pos); // Set position
                yield();
                delay(5); // Wait 15ms for the servo to move
            }
        }
        else
        {
            // Move back from 135 to 0 degrees
            for (int pos = 135; pos >= 0; pos -= 1)
            {
                yield();
                servo.write(pos);
                yield();
                delay(5);
            }
        }
    }
}

bool payloadIsOn(const byte *payload, unsigned int length)
{
    if (length == 0)
        return false;
    return payload[0] == '1' || payload[0] == 'o' || payload[0] == 'O' || payload[0] == 't' || payload[0] == 'T';
}

void applyLedState(int pin, const byte *payload, unsigned int length, const char *label)
{
    bool on = payloadIsOn(payload, length);
    bool changed = false;

    if (pin == OUT_LED_PIN)
    {
        changed = (outsideLedOn != on);
        outsideLedOn = on;
    }
    else if (pin == IN_LED_PIN)
    {
        changed = (insideLedOn != on);
        insideLedOn = on;
    }

    digitalWrite(pin, on ? HIGH : LOW);

    if (changed)
    {
        persistLedStatesToEeprom();
    }

    Serial.print("LED ");
    Serial.print(label);
    Serial.print(" => ");
    Serial.println(on ? "ON" : "OFF");
}

void restoreLedStatesFromEeprom()
{
    if (EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC)
    {
        outsideLedOn = EEPROM.read(EEPROM_ADDR_OUTSIDE) == 1;
        insideLedOn = EEPROM.read(EEPROM_ADDR_INSIDE) == 1;
    }
    else
    {
        outsideLedOn = false;
        insideLedOn = false;
        persistLedStatesToEeprom();
    }

    digitalWrite(OUT_LED_PIN, outsideLedOn ? HIGH : LOW);
    digitalWrite(IN_LED_PIN, insideLedOn ? HIGH : LOW);

    Serial.print("Restored OUTSIDE LED: ");
    Serial.println(outsideLedOn ? "ON" : "OFF");
    Serial.print("Restored INSIDE LED: ");
    Serial.println(insideLedOn ? "ON" : "OFF");
}

void persistLedStatesToEeprom()
{
    EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
    EEPROM.write(EEPROM_ADDR_OUTSIDE, outsideLedOn ? 1 : 0);
    EEPROM.write(EEPROM_ADDR_INSIDE, insideLedOn ? 1 : 0);
    EEPROM.commit();
}

void restoreDoorStatesFromEeprom()
{
    if (EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC)
    {
        doorOpenState = EEPROM.read(EEPROM_ADDR_DOOR) == 1;
    }
    else
    {
        doorOpenState = false;
        persistDoorStateToEeprom();
    }

    if (doorOpenState)
    {
        // Move from 0 to 135 degrees
        for (int pos = 0; pos <= 135; pos += 1)
        {
            yield();
            servo.write(pos); // Set position
            yield();
            delay(15); // Wait 15ms for the servo to move
        }
    }
    else
    {
        // Move back from 135 to 0 degrees
        for (int pos = 135; pos >= 0; pos -= 1)
        {
            yield();
            servo.write(pos);
            yield();
            delay(15);
        }
    }

    Serial.print("Restored DOOR: ");
    Serial.println(doorOpenState ? "OPEN" : "CLOSED");
}

void persistDoorStateToEeprom()
{
    EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
    EEPROM.write(EEPROM_ADDR_DOOR, doorOpenState ? 1 : 0);
    EEPROM.commit();
}

void ensureMqttConnected()
{
    if (client.connected())
        return;

    unsigned long now = millis();
    if (now - lastMqttAttempt < MQTT_RETRY_INTERVAL_MS)
        return;
    lastMqttAttempt = now;

    // Unique client id prevents disconnect loops when multiple devices exist
    String clientId = "esp8266-" + String(ESP.getChipId(), HEX);

    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print(" as ");
    Serial.println(clientId);

    // If your broker needs auth, use:
    // client.connect(clientId.c_str(), "username", "password");
    bool ok = client.connect(clientId.c_str());

    if (ok)
    {
        Serial.println("MQTT connected!");

        // Subscribe (do this every reconnect)
        bool subOk = client.subscribe(subCmdTopic);
        Serial.print("Subscribe to ");
        Serial.print(subCmdTopic);
        Serial.print(" => ");
        Serial.println(subOk ? "OK" : "FAILED (client-side)");

        bool stateSubOk = client.subscribe(subStateTopic);
        Serial.print("Subscribe to ");
        Serial.print(subStateTopic);
        Serial.print(" => ");
        Serial.println(stateSubOk ? "OK" : "FAILED (client-side)");

        // Ask your MQTT automation/broker for current LED states after boot/reconnect.
        // Expected response: retained or immediate publish to state/led.../POWER topics.
        client.publish(reqOutsideStateTopic, "get");
        client.publish(reqInsideStateTopic, "get");

        // Optional: publish a status message
        client.publish(pubTopic, "1", true); // retained
    }
    else
    {
        Serial.print("MQTT connect failed, state=");
        Serial.println(client.state());
        // Common states:
        // -2 = network failed, -4 = timeout, 5 = not authorized
    }
}
// Measure distance from ultrasonic sensor (HC-SR04)
long measureDistance()
{
    // Send 10 microsecond pulse to trigger pin
    digitalWrite(motionTriggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(motionTriggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(motionTriggerPin, LOW);

    // Measure echo pulse duration
    long duration = pulseIn(motionEchoPin, HIGH, 30000); // 30ms timeout

    // Convert duration to distance in centimeters
    // distance = (duration / 2) / 29.1
    long distance = (duration / 2) / 29;

    return distance;
}

void doorOpen()
{
    // Move from 0 to 135 degrees
    for (int pos = 0; pos <= 135; pos += 1)
    {
        yield();
        servo.write(pos); // Set position
        yield();
        delay(5); // Wait 5ms for the servo to move
    }
    doorOpenState = true;
    persistDoorStateToEeprom();
    Serial.println("Door opened by motion sensor");
}

void gasResponse()
{
    int rawValue = analogRead(MQ2_ANALOG_PIN);

    int mapped = map(rawValue, 0, 1023, 0, 100);
    Serial.print("Raw gas level: ");
    Serial.print(rawValue);
    if (client.connected())
    {
        char buf[8];
        itoa(mapped, buf, 10); // convert int to string

        bool gasSent = client.publish("stat/humidity/VALUE", buf);

        Serial.print("  >> stat/humidity/VALUE sent: ");
        Serial.print(gasSent ? "ok" : "failed");
        Serial.print(" | value: ");
        Serial.println(buf);
    }
    else
    {
        Serial.println("shit MQTT not working bro, skipping publish");
    }
}

void loop()
{
    // Periodic debug
    if (millis() - lastDebug > 2000)
    {
        lastDebug = millis();
        Serial.print("WiFi=");
        Serial.print(WiFi.status()); // 3 = connected
        Serial.print(" MQTTconnected=");
        Serial.print(client.connected()); // 1 = connected
        Serial.print(" state=");
        Serial.print(client.state()); // 0 when connected
        Serial.print(" IP=");
        Serial.println(WiFi.localIP());
    }

    // Keep WiFi alive (ESP8266 core usually handles it, but this is fine)
    if (WiFi.status() != WL_CONNECTED)
    {
        // Try to recover WiFi if it dropped
        static unsigned long lastWifiRetry = 0;
        if (millis() - lastWifiRetry > 5000)
        {
            lastWifiRetry = millis();
            Serial.println("WiFi not connected, retrying WiFi.begin()");
            WiFi.disconnect();
            WiFi.begin(ssid, pass);
        }
        delay(10);
        return;
    }

    // Maintain MQTT connection
    ensureMqttConnected();

    // Must be called very frequently
    client.loop();

    // Optional heartbeat publish every 30s when connected
    if (client.connected() && millis() - lastHeartbeat > 30000)
    {
        lastHeartbeat = millis();
        client.publish("status/esp8266/heartbeat", "alive");
    }

    delay(10);

    // Check motion sensor (ultrasonic)
    static unsigned long lastMotionCheck = 0;
    if (millis() - lastMotionCheck >= 1000) // Check every 1 second
    {
        lastMotionCheck = millis();
        long distance = measureDistance();
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");

        // Trigger door if object is within 3cm and door is not already open
        if (distance > 0 && distance < 3 && !doorOpenState)
        {
            Serial.println("Motion/object detected! Opening door...");
            doorOpen();
        }
    }

    static unsigned long lastRead = 0;
    if (millis() - lastRead >= 5000)
    {
        lastRead = millis();
        gasResponse();
    }
}