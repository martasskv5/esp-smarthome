#include <EEPROM.h>
#include "app_state.h"
#include "utils.h"

int x;
void tripwireSetup()
{
    Serial.print("Initialising laser...");
    pinMode(OUT_LASER, OUTPUT);
    pinMode(IN_FOTORES, INPUT);
}

void tripwire()
{
    x = analogRead(IN_FOTORES);
    Serial.print("Reading: ");
    Serial.println(x);
    if(x < 800)
    {
        Serial.println("BEAM BROKEN");
    }
}