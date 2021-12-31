#include "hardware.h"

void GPIOpinOff(uint8 pin)
{
    digitalWrite(pin, LOW);
}

void GPIOpinOn(uint8 pin)
{
    digitalWrite(pin, HIGH);
}