#ifndef DOOR_CONTROL_H
#define DOOR_CONTROL_H

#include <Arduino.h>

void initDoorHardware();
void handleDoorCommand(const byte *payload, unsigned int length);
void doorOpen();
void doorClose();
void checkMotionAutoClose();

#endif
