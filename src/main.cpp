#include <Arduino.h>

#include "app_state.h"
#include "door_control.h"
#include "gas_sensor.h"
#include "laser_tripwire.h"
#include "led_control.h"
#include "wifi_mqtt.h"

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP booting...");
  Serial.print("Reset reason: ");
  Serial.println(ESP.getResetReason());

  initLedsAndEeprom();
  initDoorHardware();
  initGasSensor();
  initLaserTripwire();

  setup_wifi();
  initMqtt();

  Serial.println("Setup complete");
}

void loop()
{
  if (millis() - lastDebug > 2000)
  {
    lastDebug = millis();
    Serial.print("WiFi=");
    Serial.print(WiFi.status());
    Serial.print(" MQTTconnected=");
    Serial.print(client.connected());
    Serial.print(" state=");
    Serial.print(client.state());
    Serial.print(" IP=");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() != WL_CONNECTED)
  {
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

  ensureMqttConnected();
  client.loop();

  if (client.connected() && millis() - lastHeartbeat > 30000)
  {
    lastHeartbeat = millis();
    client.publish("status/esp8266/heartbeat", "alive");
  }

  checkMotionAutoClose();
  // Check laser tripwire for beam interruption
  checkLaserBeam();

  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 5000)
  {
    lastRead = millis();
    gasResponse();
  }

  delay(10);
}