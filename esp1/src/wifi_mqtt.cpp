#include "wifi_mqtt.h"

#include "app_state.h"
#include "door_control.h"
#include "led_control.h"

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

    if ((strcmp(topic, cmdOutsideTopic) == 0 || strcmp(topic, stateOutsideTopic) == 0) && length > 0)
    {
        applyLedState(OUT_LED_PIN, payload, length, "OUTSIDE");
    }

    if ((strcmp(topic, cmdInsideTopic) == 0 || strcmp(topic, stateInsideTopic) == 0) && length > 0)
    {
        applyLedState(IN_LED_PIN, payload, length, "INSIDE");
    }

    // Only command topic should actuate the servo. Handling state topic here can
    // create a feedback loop when retained/state updates are echoed by the broker.
    if (strcmp(topic, cmdDoorTopic) == 0 && length > 0)
    {
        handleDoorCommand(payload, length);
    }

    if (strcmp(topic, cmdDoorLockTopic) == 0 && length > 0)
    {
        handleDoorLockCommand(payload, length);
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
    String clientId = "esp8266-" + String(ESP.getChipId(), HEX);

    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print(" as ");
    Serial.println(clientId);

    //   bool ok = client.connect(clientId.c_str());
    bool ok = client.connect(
        clientId.c_str(),           // client ID
        "status/esp8266/heartbeat", // LWT topic
        1,                          // LWT QoS
        true,                       // LWT retain
        "dead"                      // LWT message (published by broker when ESP disconnects)
    );

    if (ok)
    {
        Serial.println("MQTT connected!");

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

        client.publish(reqOutsideStateTopic, "get");
        client.publish(reqInsideStateTopic, "get");
        client.publish(stateDoorTopic, doorOpenState ? "1" : "0", true);
        client.publish(stateDoorLockTopic, doorLockedState ? "1" : "0", true);
        client.publish(pubTopic, "1", true);
    }
    else
    {
        Serial.print("MQTT connect failed, state=");
        Serial.println(client.state());
    }
}
