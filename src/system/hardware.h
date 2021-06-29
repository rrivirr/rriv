#include <Arduino.h>
#include <Wire_slave.h> // Communicate with I2C/TWI devices

// For F103RB
#define Serial Serial2

// extern TwoWire Wire;
#define WireOne Wire 

// extern TwoWire WireTwo;
#define WireTwo Wire1

#define SWITCHED_POWER_ENABLE PC6
#define SD_ENABLE_PIN PC8

#define BLE_COMMAND_MODE_PIN PB5
#define INTERRUPT_LINE_7_PIN PC7
//pinMode(PB10, INPUT_PULLDOWN); // This WAS interrupt line 10, user interrupt. Needs to be reassigned.

#define ANALOG_INPUT_1_PIN PB1 // A2
#define ANALOG_INPUT_2_PIN PC0 // A3
#define ANALOG_INPUT_3_PIN PC1 // A4
#define ANALOG_INPUT_4_PIN PC2 // A5
#define ANALOG_INPUT_5_PIN PC3 // A6

#define ONBOARD_LED_PIN PA5

