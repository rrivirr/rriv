#include <Arduino.h>
#include <Wire.h> // Communicate with I2C/TWI devices
#include "DS3231.h"
#include "WaterBear_FileSystem.h"

void scanIC2(TwoWire * wire);

void printInterruptStatus(HardwareSerial &serial);
void printDateTime(HardwareSerial &serial, DateTime now);


void writeEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress, byte data );
byte readEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress );


void blink(int times, int duration);
void printDS3231Time();


WaterBear_FileSystem * debugFilesystemHandle;
void writeSerialMessage(const char * message);
void writeSerialMessage(const __FlashStringHelper * message);
void writeDebugMessage(const char * message);
void writeDebugMessage(const __FlashStringHelper * message);
void error(const __FlashStringHelper*err);
