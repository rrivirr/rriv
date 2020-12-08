#ifndef WATERBEAR_EEPROM
#define WATERBEAR_EEPROM

#include <Arduino.h>
#include <Wire.h> // Communicate with I2C/TWI devices


#define EEPROM_I2C_ADDRESS 0x50

#define EEPROM_UUID_ADDRESS_START 0
#define EEPROM_UUID_ADDRESS_END 15
#define UUID_LENGTH 12 // STM32 has a 12 byte UUID, leave extra space for the future

#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START 16
#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_END   43
#define DEPLOYMENT_IDENTIFIER_LENGTH 25


void writeEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress, byte data );
byte readEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress );

void readDeploymentIdentifier(char * deploymentIdentifier);
void writeDeploymentIdentifier(char * deploymentIdentifier);

void readUniqueId(unsigned char * uuid); // uuid must point to char[UUID_LENGTH]

#endif