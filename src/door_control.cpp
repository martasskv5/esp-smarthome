#include "door_control.h"

#include <EEPROM.h>

#include "app_state.h"
#include "utils.h"

static void restoreDoorStatesFromEeprom();
static void persistDoorStateToEeprom();

void initDoorHardware() {
  pinMode(motionTriggerPin, OUTPUT);
  pinMode(motionEchoPin, INPUT);
  pinMode(servo_pin, OUTPUT);
  servo.attach(servo_pin);

  restoreDoorStatesFromEeprom();
}

void handleDoorCommand(const byte *payload, unsigned int length) {
  bool on = payloadIsOn(payload, length);
  if (on) {
    doorOpen();
  } else {
    doorClose();
  }
}

void doorOpen() {
  if (doorOpenState) {
    return;
  }

  for (int pos = 0; pos <= 135; pos += 1) {
    yield();
    servo.write(pos);
    yield();
    delay(3);
  }

  doorOpenState = true;
  doorAutoCloseArmed = false;
  persistDoorStateToEeprom();

  if (client.connected()) {
    client.publish(stateDoorTopic, "1", true);
    Serial.println("Published door state: OPEN");
  }
}

void doorClose() {
  if (!doorOpenState) {
    return;
  }

  for (int pos = 135; pos >= 0; pos -= 1) {
    yield();
    servo.write(pos);
    yield();
    delay(3);
  }

  doorOpenState = false;
  doorAutoCloseArmed = false;
  persistDoorStateToEeprom();

  if (client.connected()) {
    client.publish(stateDoorTopic, "0", true);
    Serial.println("Published door state: CLOSED");
  }
}

void checkMotionAutoClose() {
  static unsigned long lastMotionCheck = 0;
  static uint8_t consecutiveNearReadings = 0;

  if (millis() - lastMotionCheck < 1000) {
    return;
  }

  lastMotionCheck = millis();

  digitalWrite(motionTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(motionTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(motionTriggerPin, LOW);

  float duration = pulseIn(motionEchoPin, HIGH, 30000);
  float distance = (duration * 0.0343f) / 2.0f;

  Serial.print("distance: ");
  Serial.println(distance);

  bool validDistance = duration > 0.0f && distance > 1.0f && distance < 400.0f;
  bool isNear = validDistance && distance <= 4.0f;

  if (isNear) {
    if (consecutiveNearReadings < 3) {
      consecutiveNearReadings++;
    }
  } else {
    consecutiveNearReadings = 0;
  }

  if (consecutiveNearReadings >= 3 && !doorOpenState) {
    Serial.println("Motion detected! Opening door...");
    doorOpen();
    Serial.println("Door opened by motion sensor");
    doorAutoCloseTimer = millis();
    doorAutoCloseArmed = true;
    consecutiveNearReadings = 0;
  }

  if (doorOpenState && doorAutoCloseArmed && millis() - doorAutoCloseTimer >= 10000) {
    doorClose();
    Serial.println("Door closed by motion sensor after 10s");
  }
}

static void restoreDoorStatesFromEeprom() {
  if (EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC) {
    doorOpenState = EEPROM.read(EEPROM_ADDR_DOOR) == 1;
  } else {
    doorOpenState = false;
    persistDoorStateToEeprom();
  }

  if (doorOpenState) {
    for (int pos = 0; pos <= 135; pos += 1) {
      yield();
      servo.write(pos);
      yield();
      delay(3);
    }
  } else {
    for (int pos = 135; pos >= 0; pos -= 1) {
      yield();
      servo.write(pos);
      yield();
      delay(3);
    }
  }

  doorAutoCloseArmed = false;

  Serial.print("Restored DOOR: ");
  Serial.println(doorOpenState ? "OPEN" : "CLOSED");
}

static void persistDoorStateToEeprom() {
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.write(EEPROM_ADDR_DOOR, doorOpenState ? 1 : 0);
  EEPROM.commit();
}
