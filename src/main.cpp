/*  ESP8266 + PubSubClient robust MQTT example
    - Unique client ID (prevents client-id collisions)
    - Reconnect with retry timer
    - Re-subscribe after every reconnect
    - Always runs client.loop()
    - Prints useful debug including client.state()
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ==== WiFi ====
const char* ssid = "MAJCHROCIK";
const char* pass = "dazdnik1";

// ==== MQTT ====
const char* mqtt_server = "10.195.236.205";
const uint16_t mqtt_port = 1883;

// Topic you want to receive
const char* subTopic = "cmd/#";

// (Optional) publish a heartbeat so you can see the ESP is alive
const char* pubTopic = "status/esp8266/online";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMqttAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

unsigned long lastDebug = 0;
unsigned long lastHeartbeat = 0;

void setup_wifi();
void ensureMqttConnected();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  delay(50);

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

  // Example: if topic == cmd/ledOUTSIDE/POWER and payload is '1' / '0'
  // You can parse like this (optional):
  // if (strcmp(topic, subTopic) == 0 && length > 0) {
  //   if (payload[0] == '1') { ... }
  //   else if (payload[0] == '0') { ... }
  // }
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
    bool subOk = client.subscribe(subTopic);
    Serial.print("Subscribe to ");
    Serial.print(subTopic);
    Serial.print(" => ");
    Serial.println(subOk ? "OK" : "FAILED (client-side)");

    // Optional: publish a status message
    client.publish(pubTopic, "1", true); // retained
  } else {
    Serial.print("MQTT connect failed, state=");
    Serial.println(client.state());
    // Common states:
    // -2 = network failed, -4 = timeout, 5 = not authorized
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
}