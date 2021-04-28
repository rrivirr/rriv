#ifndef WATERBEAR_EEPROM
#define WATERBEAR_EEPROM

#include <Arduino.h>
#include <Wire.h> // Communicate with I2C/TWI devices


#define EEPROM_I2C_ADDRESS 0x50

#define EEPROM_UUID_ADDRESS_START 0
#define EEPROM_UUID_ADDRESS_END 15
#define UUID_LENGTH 12 // STM32 has a 12 byte UUID, leave extra space for the future 16

#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START 16
#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_END 43
#define DEPLOYMENT_IDENTIFIER_LENGTH 25 // out of 28

#define DEVICE_SERIAL_NUMBER_ADDRESS_START 44
#define DEVICE_SERIAL_NUMBER_ADDRESS_END 63
#define DEVICE_SERIAL_NUMBER_LENGTH 16 // out of 20

//Thermistor Calibration Block
#define TEMPERATURE_C1_ADDRESS_START 64
#define TEMPERATURE_C1_ADDRESS_END 65
#define TEMPERATURE_C1_ADDRESS_LENGTH 2

#define TEMPERATURE_V1_ADDRESS_START 66
#define TEMPERATURE_V1_ADDRESS_END 67
#define TEMPERATURE_V1_ADDRESS_LENGTH 2

#define TEMPERATURE_C2_ADDRESS_START 68
#define TEMPERATURE_C2_ADDRESS_END 69
#define TEMPERATURE_C2_ADDRESS_LENGTH 2

#define TEMPERATURE_V2_ADDRESS_START 70
#define TEMPERATURE_V2_ADDRESS_END 71
#define TEMPERATURE_V2_ADDRESS_LENGTH 2

#define TEMPERATURE_TIMESTAMP_ADDRESS_START 72 // epoch timestamp
#define TEMPERATURE_TIMESTAMP_ADDRESS_END 75
#define TEMPERATURE_TIMESTAMP_ADDRESS_LENGTH 4

#define TEMPERATURE_M_ADDRESS_START 76 // float xxx.xx =*100> unsigned short xxxxx
#define TEMPERATURE_M_ADDRESS_END 77
#define TEMPERATURE_M_ADDRESS_LENGTH 2

#define TEMPERATURE_B_ADDRESS_START 78
#define TEMPERATURE_B_ADDRESS_END 81
#define TEMPERATURE_B_ADDRESS_LENGTH 4

#define TEMPERATURE_SCALER 100 // applies to C1 C2 M B values for storage
#define TEMPERATURE_BLOCK_LENGTH 18//for resetting 64-81

void writeEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress, byte data );
byte readEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress );

void readDeploymentIdentifier(char * deploymentIdentifier);
void writeDeploymentIdentifier(char * deploymentIdentifier);

void readUniqueId(unsigned char * uuid); // uuid must point to char[UUID_LENGTH]

void writeEEPROMBytes(byte address, unsigned char * data, uint8_t size);
void readEEPROMBytes(byte address, unsigned char * data, uint8_t size);

#endif