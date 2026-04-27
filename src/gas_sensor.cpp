#include "gas_sensor.h"

#include "app_state.h"

namespace {
unsigned long mq2WarmupStartMs = 0;
bool mq2ReadyLogged = false;
const unsigned long MQ2_WARMUP_MS = 30000;
}

void initGasSensor() {
  mq2WarmupStartMs = millis();
  mq2ReadyLogged = false;
  Serial.println("Mq2 starting up, warmup is non-blocking (30 seconds)");
}

void gasResponse() {
  unsigned long elapsed = millis() - mq2WarmupStartMs;
  if (elapsed < MQ2_WARMUP_MS) {
    Serial.print("Mq2 warming up, remaining ms: ");
    Serial.println(MQ2_WARMUP_MS - elapsed);
    return;
  }

  if (!mq2ReadyLogged) {
    Serial.println("Mq2 warmup complete, starting gas publishes");
    mq2ReadyLogged = true;
  }

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
