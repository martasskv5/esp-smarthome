#include "gas_sensor.h"

#include "app_state.h"

void initGasSensor() {
  Serial.println("Mq2 starting up, waiting 30 seconds");
  delay(30000);
}

void gasResponse() {
  int rawValue = analogRead(MQ2_ANALOG_PIN);
  int mapped = map(rawValue, 0, 1023, 0, 100);

  Serial.print("Raw gas level: ");
  Serial.print(rawValue);

  if (client.connected()) {
    char buf[8];
    itoa(mapped, buf, 10);

    bool gasSent = client.publish("stat/humidity/VALUE", buf);

    Serial.print("  >> stat/humidity/VALUE sent: ");
    Serial.print(gasSent ? "ok" : "failed");
    Serial.print(" | value: ");
    Serial.println(buf);
  } else {
    Serial.println("shit MQTT not working bro, skipping publish");
  }
}
