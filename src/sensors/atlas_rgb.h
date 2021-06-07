#ifndef WATERBEAR_ATLAS_RGB
#define WATERBEAR_ATLAS_RGB

#include <string.h>
#include <Arduino.h>

class AtlasRGB
{
  private:
    // int rx;
    // int tx;
    //HardwareSerial rgbSerial;
    char inputString[30];
    char sensorString[30];
    bool inputStringComplete;
    bool sensorStringComplete;

    void setupSerial();
    char * printRGBData(); 
  
  public:
    // Basic Functionality
    AtlasRGB();
  
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

};

#endif