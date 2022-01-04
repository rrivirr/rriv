#ifndef WATERBEAR_HARDWARE
#define WATERBEAR_HARDWARE

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

//#define GPIO_PIN_1 PC15 // doesn't work, not sure why
//#define GPIO_PIN_2 PA13 // doesn't work, not sure why
#define GPIO_PIN_3 PC12 // works, CN7 3

#define EXT_MOSFET_ENABLE NULL // for switched_power.cpp

// Bluefruit on SPI
#define BLUEFRUIT_SPI_SCK   PB13
#define BLUEFRUIT_SPI_MISO  PB14
#define BLUEFRUIT_SPI_MOSI  PB15
#define BLUEFRUIT_SPI_CS    PB8  // Pullup
#define BLUEFRUIT_SPI_IRQ   PB9
#define BLUEFRUIT_SPI_RST   PC4

void GPIOpinOff(uint8 pin);
void GPIOpinOn(uint8 pin);
void startSerial2();
void setupInternalRTC();
void setupHardwarePins();

#endif