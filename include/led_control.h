#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

void initLedsAndEeprom();
void applyLedState(int pin, const byte *payload, unsigned int length, const char *label);

#endif
