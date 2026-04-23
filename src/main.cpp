/*  ESP8266 + PubSubClient robust MQTT example
    - Unique client ID (prevents client-id collisions)
    - Reconnect with retry timer
    - Re-subscribe after every reconnect
    - Always runs client.loop()
    - Prints useful debug including client.state()
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// servo senzor
const int servo_pin = 9;
// gas senzor
const int MQ2_ANALOG_PIN = A0;
const int MQ2_DIH_ITAL_PIN = 14;

// ==== WiFi ====
const char* ssid = "MAJCHROCIK";
const char* pass = "dazdnik1";

// ==== MQTT ====
const char* mqtt_server = "10.195.236.205";
const uint16_t mqtt_port = 1883;

// Command topics (incoming desired state)
const char* subCmdTopic = "cmd/#";

// State topics (broker/device reports current state)
const char* subStateTopic = "state/#";

// Explicit state request topics (published on connect)
const char* reqOutsideStateTopic = "cmd/ledOUTSIDE/POWER/get";
const char* reqInsideStateTopic = "cmd/ledINSIDE/POWER/get";

const char* cmdOutsideTopic = "cmd/ledOUTSIDE/POWER";
const char* cmdInsideTopic = "cmd/ledINSIDE/POWER";
const char* stateOutsideTopic = "stat/ledOUTSIDE/POWER";
const char* stateInsideTopic = "stat/ledINSIDE/POWER";

const char* cmdDoorTopic = "cmd/door/OPEN";
const char* stateDoorTopic = "stat/door/OPEN";

// (Optional) publish a heartbeat so you can see the ESP is alive
const char* pubTopic = "status/esp8266/online";

// GPIO pins
const int OUT_LED_PIN = 5;  // D1 on NodeMCU (GPIO5)
const int IN_LED_PIN = 4;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMqttAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

unsigned long lastDebug = 0;
unsigned long lastHeartbeat = 0;

void setup_wifi();
void ensureMqttConnected();
void callback(char* topic, byte* payload, unsigned int length);
bool payloadIsOn(const byte* payload, unsigned int length);
void applyLedState(int pin, const byte* payload, unsigned int length, const char* label);

void setup() {
  Serial.begin(115200);
  delay(50);

  // Initialize GPIO pins
  pinMode(OUT_LED_PIN, OUTPUT);
  pinMode(IN_LED_PIN, OUTPUT);
  digitalWrite(IN_LED_PIN, LOW);
  pinMode(MQ2_DIH_ITAL_PIN, INPUT);
  Serial.println("Mq2 starting up, waiting 30 seconds");
  delay(30000);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // PubSubClient timeouts are in SECONDS (not ms)
  client.setSocketTimeout(10);  // seconds
  client.setKeepAlive(15);      // seconds
}

void setup_wifi() {
  Serial.println();
  Serial.println("Starting WiFi connection...");

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pass);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(250);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("\nWiFi failed to connect (will keep trying in loop)");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // payload is NOT null-terminated
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Accept both command topics and reported state topics.
  // This allows restoring outputs after boot from retained state responses.
  if ((strcmp(topic, cmdOutsideTopic) == 0 || strcmp(topic, stateOutsideTopic) == 0) && length > 0) {
    applyLedState(OUT_LED_PIN, payload, length, "OUTSIDE");
  }

  if ((strcmp(topic, cmdInsideTopic) == 0 || strcmp(topic, stateInsideTopic) == 0) && length > 0) {
    applyLedState(IN_LED_PIN, payload, length, "INSIDE");
  }
  
  //door servo
  if ((strcmp(topic, cmdDoorTopic) == 0 || strcmp(topic, stateDoorTopic) == 0) && length > 0) {
    bool on = payloadIsOn(payload, length);
    digitalWrite(servo_pin, on ? HIGH : LOW);
  }
}

bool payloadIsOn(const byte* payload, unsigned int length) {
  if (length == 0) return false;
  return payload[0] == '1' || payload[0] == 'o' || payload[0] == 'O' || payload[0] == 't' || payload[0] == 'T';
}

void applyLedState(int pin, const byte* payload, unsigned int length, const char* label) {
  bool on = payloadIsOn(payload, length);
  digitalWrite(pin, on ? HIGH : LOW);

  Serial.print("LED ");
  Serial.print(label);
  Serial.print(" => ");
  Serial.println(on ? "ON" : "OFF");
}

void ensureMqttConnected() {
  if (client.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttAttempt < MQTT_RETRY_INTERVAL_MS) return;
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

  if (ok) {
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
  } else {
    Serial.print("MQTT connect failed, state=");
    Serial.println(client.state());
    // Common states:
    // -2 = network failed, -4 = timeout, 5 = not authorized
  }
}
void doorOpen(){

}


void gasResponse(){

  int rawValue = analogRead(MQ2_ANALOG_PIN);
  int digitalAlert = digitalRead(MQ2_DIH_ITAL_PIN);

  int mapped = map(rawValue, 0,1023,0,100);
  Serial.print("Raw gas level: ");
  Serial.print(rawValue);
  Serial.print(" | Alert: ");
  Serial.println(digitalAlert == LOW ? "GAS DETECTED" : "Clear");

  if (client.connected()) {

    char buf[8];
    itoa(mapped, buf, 10); // convert int to string

    bool gasSent   = client.publish("stat/humidity/VALUE", buf);

    Serial.print("  >> stat/humidity/VALUE sent: ");
    Serial.print(gasSent ? "ok" : "failed");
    Serial.print(" | value: ");
    Serial.println(buf);
  } else {
    Serial.println("shit MQTT not working bro, skipping publish");
  }
}

void loop() {
  // Periodic debug
  if (millis() - lastDebug > 2000) {
    lastDebug = millis();
    Serial.print("WiFi=");
    Serial.print(WiFi.status());           // 3 = connected
    Serial.print(" MQTTconnected=");
    Serial.print(client.connected());      // 1 = connected
    Serial.print(" state=");
    Serial.print(client.state());          // 0 when connected
    Serial.print(" IP=");
    Serial.println(WiFi.localIP());
  }

  // Keep WiFi alive (ESP8266 core usually handles it, but this is fine)
  if (WiFi.status() != WL_CONNECTED) {
    // Try to recover WiFi if it dropped
    static unsigned long lastWifiRetry = 0;
    if (millis() - lastWifiRetry > 5000) {
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
  if (client.connected() && millis() - lastHeartbeat > 30000) {
    lastHeartbeat = millis();
    client.publish("status/esp8266/heartbeat", "alive");
  }

  delay(10);


  static unsigned long lastRead = 0;
  if(millis() - lastRead >= 5000){
    lastRead = millis();
    gasResponse();
  }
}