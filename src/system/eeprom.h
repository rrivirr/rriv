#ifndef WATERBEAR_EEPROM
#define WATERBEAR_EEPROM

#include <Arduino.h>
#include <Wire_slave.h> // Communicate with I2C/TWI devices

/*
 * Partition scheme (24LC01B 1Kbit)
 * 000-099 Waterbear Device Info
 * 100-199
 * 200-249
 * 250-299 Waterbear Device Calibration / Significant Values
 * 300-399
 * 400-499
 * 500-549 Sensor 1 Calibration
 * 550-599 Sensor 2 Calibration
 * 600-649 Sensor 3 Calibration
 * 650-699 Sensor 4 Calibration
 * 700-749 Sensor 5 Calibration
 * 750-799 Sensor 6 Calibration
 * 800-849 Sensor 7 Calibration
 * 850-899 Sensor 8 Calibration
 * 900-949 Sensor 9 Calibration
 * 950-999 Sensor 10 Calibration
*/

#define EEPROM_I2C_ADDRESS 0x50 // this is weird

#define EEPROM_RESET_VALUE 255 // max value of a byte

#define EEPROM_UUID_ADDRESS_START 0
#define EEPROM_UUID_ADDRESS_END 15
#define UUID_LENGTH 12 // STM32 has a 12 byte UUID, leave extra space for the future 16

#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START 16
#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_END 43
#define DEPLOYMENT_IDENTIFIER_LENGTH 25 // out of 28

#define DEVICE_SERIAL_NUMBER_ADDRESS_START 44
#define DEVICE_SERIAL_NUMBER_ADDRESS_END 63
#define DEVICE_SERIAL_NUMBER_LENGTH 16 // out of 20

// Waterbear Device Calibration / Significant Values
#define BURST_INTERVAL_ADDRESS 250
#define BURST_LENGTH_ADDRESS 251
#define USER_WAKE_TIMEOUT_ADDRESS 252 // timeout after wakeup from user interaction, seconds->min?
#define SD_FIELDCOUNT_ADDRESS 253

// Debug settings bit register map {outdated with various serial command modes?}
/*
 * 0 measurements: enable log messages related to measurements & bursts
 * 1 loop: don't sleep
 * 2 short sleep: sleep for a hard coded short amount of time
 * 3 to file: also send debug messages to the output file
 * 4 to serial: send debug messages to the serial interface
*/

#define DEBUG_SETTINGS_ADDRESS 404 // 8 booleans {measurements, loop, short sleep, to file, to serial}

/*
 * Sensor General Register Map:
 * 0 sensor type
 * 1 ADC/Digital sensor type
 * 2 calibrated boolean (1 yes, 0 no) could be a bit?
 * 3-29 calibration variables:
 * Pin if ADC
 * i2c address?
*/
#define SENSOR_ADDRESS_LENGTH 50
#define SENSOR_1_ADDRESS_START 500
#define SENSOR_2_ADDRESS_START 550
#define SENSOR_3_ADDRESS_START 600
#define SENSOR_4_ADDRESS_START 650
#define SENSOR_5_ADDRESS_START 700
#define SENSOR_6_ADDRESS_START 750
#define SENSOR_7_ADDRESS_START 800
#define SENSOR_8_ADDRESS_START 850
#define SENSOR_9_ADDRESS_START 900
#define SENSOR_10_ADDRESS_START 950

// Sensor Types
#define NO_SENSOR 0; // or 255?
#define SENSOR_TYPE_THERMISTOR 1


#define SENSOR_CALIBRATED 1

/*
 * Thermistor Register Map
 * 0 sensor type = 1
 * 1 ADC
 * 2 calibrated boolean (1 yes, 0 no)
 * 3 sensor pin
 * 4-5 c1
 * 6-7 v1
 * 8-9 c2
 * 10-11 v2
 * 12-13 m
 * 14-17 b
 * 18-21 calibration timestamp
*/

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

void clearEEPROMAddress(byte address, uint8_t length);

#endif