#ifndef WATERBEAR_UTILITIES
#define WATERBEAR_UTILITIES

// #include <Arduino.h>
#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include "DS3231.h"
#include "configuration.h"
#include "system/control.h"

void scanIC2(TwoWire * wire);

void printInterruptStatus(HardwareSerial &serial);
void printDateTime(HardwareSerial &serial, DateTime now);

void blink(int times, int duration);
void printDS3231Time();

#endif
