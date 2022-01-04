#ifndef WATERBEAR_EEPROM
#define WATERBEAR_EEPROM

#include <Arduino.h>
#include <Wire_slave.h> // Communicate with I2C/TWI devices

#define EEPROM_I2C_ADDRESS 0x50

#define EEPROM_RESET_VALUE 255 // max value of a byte

#define EEPROM_UUID_ADDRESS_START 0
#define EEPROM_UUID_ADDRESS_END 15
#define UUID_LENGTH 12 // STM32 has a 12 byte UUID, leave extra space for the future 16

#define EEPROM_DATALOGGER_CONFIGURATION_START 16
#define EEPROM_DATALOGGER_CONFIGURATION_SIZE 64
#define EEPROM_DATALOGGER_SENSORS_START 80
#define EEPROM_DATALOGGER_SENSOR_SIZE 64
#define EEPROM_TOTAL_SENSOR_SLOTS 1

void writeEEPROM(TwoWire * wire, int deviceaddress, short eeaddress, byte data );
byte readEEPROM(TwoWire * wire, int deviceaddress, short eeaddress );

void readUniqueId(unsigned char * uuid); // uuid must point to char[UUID_LENGTH]

void writeEEPROMBytes(short address, unsigned char * data, uint8_t size);
void readEEPROMBytes(short address, unsigned char * data, uint8_t size);

void writeDataloggerSettingsToEEPROM(void * dataloggerSettings);
void writeSensorConfigurationToEEPROM(short slot, void * configuration);

void readEEPROMBytesMem(short address, void * destination, uint8_t size); // Little Endian
void writeEEPROMBytesMem(short address, void * source, uint8_t size);

void clearEEPROMAddress(short address, uint8_t length);

void readEEPROMObject(short address, void * data, uint8_t size); // Little Endian

#endif