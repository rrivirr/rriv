#include <Arduino.h>
#include <Wire_slave.h> // Communicate with I2C/TWI devices

void i2cSendTransmission(byte i2cAddress, byte registerAddress, const void * data, int numBytes);
void i2cError(int transmissionCode);
void scanIC2(TwoWire *wire);
void enableI2C1();
void enableI2C2();

