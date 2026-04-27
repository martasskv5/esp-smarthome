#include "led_control.h"

#include <EEPROM.h>

#include "app_state.h"
#include "utils.h"

static void restoreLedStatesFromEeprom();
static void persistLedStatesToEeprom();

void initLedsAndEeprom() {
  pinMode(OUT_LED_PIN, OUTPUT);
  pinMode(IN_LED_PIN, OUTPUT);
  EEPROM.begin(8);
  restoreLedStatesFromEeprom();
}

void applyLedState(int pin, const byte *payload, unsigned int length, const char *label) {
  bool on = payloadIsOn(payload, length);
  bool changed = false;

  if (pin == OUT_LED_PIN) {
    changed = (outsideLedOn != on);
    outsideLedOn = on;
  } else if (pin == IN_LED_PIN) {
    changed = (insideLedOn != on);
    insideLedOn = on;
  }

  digitalWrite(pin, on ? HIGH : LOW);

  if (changed) {
    persistLedStatesToEeprom();
  }

  Serial.print("LED ");
  Serial.print(label);
  Serial.print(" => ");
  Serial.println(on ? "ON" : "OFF");
}

static void restoreLedStatesFromEeprom() {
  if (EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC) {
    outsideLedOn = EEPROM.read(EEPROM_ADDR_OUTSIDE) == 1;
    insideLedOn = EEPROM.read(EEPROM_ADDR_INSIDE) == 1;
  } else {
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

static void persistLedStatesToEeprom() {
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.write(EEPROM_ADDR_OUTSIDE, outsideLedOn ? 1 : 0);
  EEPROM.write(EEPROM_ADDR_INSIDE, insideLedOn ? 1 : 0);
  EEPROM.commit();
}
