#include "wifi_mqtt.h"
#include "app_state.h"
#include "laser_tripwire.h"

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

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Handle alarm commands if needed
    // Can add logic here to control alarm behavior via MQTT

    if (strcmp(topic, cmdDoorLockTopic) == 0 && length > 0)
    {
        handleLockCommand(payload, length);
    }
}

void initMqtt()
{
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);
    client.setSocketTimeout(10);
    client.setKeepAlive(15);
}

void ensureMqttConnected()
{
    if (client.connected())
    {
        return;
    }

    unsigned long now = millis();
    if (now - lastMqttAttempt < MQTT_RETRY_INTERVAL_MS)
    {
        return;
    }

    lastMqttAttempt = now;
    String clientId = "esp8266-alarm-" + String(ESP.getChipId(), HEX);

    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print(" as ");
    Serial.println(clientId);

    bool ok = client.connect(
        clientId.c_str(),           // client ID
        "status/esp8266-alarm/lwt", // LWT topic
        1,                          // LWT QoS
        true,                       // LWT retain
        "dead"                      // LWT message
    );

    if (ok)
    {
        Serial.println("MQTT connected!");

        bool subOk = client.subscribe(subCmdTopic);
        Serial.print("Subscribe to ");
        Serial.print(subCmdTopic);
        Serial.print(" => ");
        Serial.println(subOk ? "OK" : "FAILED (client-side)");

        client.publish(pubTopic, "1", true);
    }
    else
    {
        Serial.print("MQTT connect failed, state=");
        Serial.println(client.state());
    }
}
