#ifndef WATERBEAR_EEPROM
#define WATERBEAR_EEPROM

#include <Arduino.h>
#include <Wire_slave.h> // Communicate with I2C/TWI devices

/*
 * EEPROM Partition scheme (24LC01B 1Kbit)
 * 000-359 Waterbear Device Info, Calibration, Significant Values
 * 360-423 Sensor Calibration Block 1
 * 424-487 Sensor Calibration Block 2
 * 488-551 Sensor Calibration Block 3
 * 552-615 Sensor Calibration Block 4
 * 616-679 Sensor Calibration Block 5
 * 680-743 Sensor Calibration Block 6
 * 744-807 Sensor Calibration Block 7
 * 808-873 Sensor Calibration Block 8
 * 872-936 Sensor Calibration Block 9
 * 936-999 Sensor Calibration Block 10
*/

#define EEPROM_I2C_ADDRESS 0x50

#define EEPROM_RESET_VALUE 255 // max value of a byte

#define EEPROM_UUID_ADDRESS_START 0
#define EEPROM_UUID_ADDRESS_END 15
#define UUID_LENGTH 12 // STM32 has a 12 byte UUID, leave extra space for the future 16

#define EEPROM_DATALOGGER_CONFIGURATION_START 16
#define EEPROM_DATALOGGER_CONFIGURATION_SIZE 64
#define EEPROM_DATALOGGER_SENSORS_START 80
#define EEPROM_DATALOGGER_SENSOR_SIZE 64

#define USER_WAKE_TIMEOUT_ADDRESS 252 // timeout after wakeup from user interaction, seconds->min?

// Debug settings bit register map {outdated with various serial command modes?}
/*
 * 0 measurements: enable log messages related to measurements & bursts
 * 1 loop: don't sleep
 * 2 short sleep: sleep for a hard coded short amount of time
 * 3 to file: also send debug messages to the output file
 * 4 to serial: send debug messages to the serial interface
*/

//Sensor slot addresses (64bytes each, 360-999)
// #define SENSOR_SLOT_1_ADDRESS 360
// #define SENSOR_SLOT_2_ADDRESS 424
// #define SENSOR_SLOT_3_ADDRESS 488
// #define SENSOR_SLOT_4_ADDRESS 552
// #define SENSOR_SLOT_5_ADDRESS 616
// #define SENSOR_SLOT_6_ADDRESS 680
// #define SENSOR_SLOT_7_ADDRESS 744
// #define SENSOR_SLOT_8_ADDRESS 808
// #define SENSOR_SLOT_9_ADDRESS 872
// #define SENSOR_SLOT_10_ADDRESS 936

//Thermistor Calibration Block {outdated with addition of generic analog driver}
// #define TEMPERATURE_C1_ADDRESS_START 64
// #define TEMPERATURE_C1_ADDRESS_END 65
// #define TEMPERATURE_C1_ADDRESS_LENGTH 2

// #define TEMPERATURE_V1_ADDRESS_START 66
// #define TEMPERATURE_V1_ADDRESS_END 67
// #define TEMPERATURE_V1_ADDRESS_LENGTH 2

// #define TEMPERATURE_C2_ADDRESS_START 68
// #define TEMPERATURE_C2_ADDRESS_END 69
// #define TEMPERATURE_C2_ADDRESS_LENGTH 2

// #define TEMPERATURE_V2_ADDRESS_START 70
// #define TEMPERATURE_V2_ADDRESS_END 71
// #define TEMPERATURE_V2_ADDRESS_LENGTH 2

// #define TEMPERATURE_TIMESTAMP_ADDRESS_START 72 // epoch timestamp
// #define TEMPERATURE_TIMESTAMP_ADDRESS_END 75
// #define TEMPERATURE_TIMESTAMP_ADDRESS_LENGTH 4

// #define TEMPERATURE_M_ADDRESS_START 76 // float xxx.xx =*100> unsigned short xxxxx
// #define TEMPERATURE_M_ADDRESS_END 77
// #define TEMPERATURE_M_ADDRESS_LENGTH 2

// #define TEMPERATURE_B_ADDRESS_START 78
// #define TEMPERATURE_B_ADDRESS_END 81
// #define TEMPERATURE_B_ADDRESS_LENGTH 4

//#define TEMPERATURE_SCALER 100 // applies to C1 C2 M B values for storage
//#define TEMPERATURE_BLOCK_LENGTH 18//for resetting 64-81

void writeEEPROM(TwoWire * wire, int deviceaddress, short eeaddress, byte data );
byte readEEPROM(TwoWire * wire, int deviceaddress, short eeaddress );

// void readDeploymentIdentifier(char * deploymentIdentifier);
// void writeDeploymentIdentifier(char * deploymentIdentifier);

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