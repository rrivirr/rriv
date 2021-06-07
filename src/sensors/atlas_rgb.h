#ifndef WATERBEAR_ATLAS_RGB
#define WATERBEAR_ATLAS_RGB

#include <Wire.h> // Communicate with I2C/TWI devices
#include <EC_OEM.h>
#include <string.h>

std::string inputString;
std::string sensorString;
bool inputStringComplete;
bool sensorStringComplete;

void setupRGB(int rx, int tx); // Receiving and transmitting pins
void serialEvent();
void printRGBData();     

#endif