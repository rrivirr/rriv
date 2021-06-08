#ifndef WATERBEAR_ATLAS_RGB
#define WATERBEAR_ATLAS_RGB

#include <string.h>
#include <Arduino.h>

void setupSerial();
char * printRGBData(); 


// Basic Functionality
//AtlasRGB();

void start();
void sendMessage();
char * receiveResponse(); 
char * run();


// Command generation
int setLEDBrightness(int value, bool powerSaving);
void setIndicatorLED(bool status, bool power);
void findSensor();
void continuousMode(int value);
void singleMode();
void calibrateSensor();
void colorMatching(int value);
void deviceInformation();
void sleepSensor();
void factoryReset();


#endif