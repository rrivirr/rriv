#include <Arduino.h>
#include <Wire.h> // Communicate with I2C/TWI devices

void scanIC2(TwoWire * wire);

void printInterruptStatus();

void writeEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress, byte data );
byte readEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress );
