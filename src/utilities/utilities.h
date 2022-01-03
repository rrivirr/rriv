#ifndef WATERBEAR_UTILITIES
#define WATERBEAR_UTILITIES

// #include <Arduino.h>
#include "DS3231.h"
#include "configuration.h"
#include "system/command.h"

void printInterruptStatus(HardwareSerial &serial);
void printDateTime(HardwareSerial &serial, DateTime now);

void blink(int times, int duration);
void printDS3231Time();

const char * reinterpretCharPtr(const __FlashStringHelper *string);

void blinkTest();

#endif
