#ifndef WATERBEAR_ATLAS_RGB
#define WATERBEAR_ATLAS_RGB

#include <Wire.h> // Communicate with I2C/TWI devices
#include <EC_OEM.h>
#include <string.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

class AtlasRGB
{
  private:
    int rx;
    int tx;
    SoftwareSerial rgbSerial;
    std::string inputString;
    std::string sensorString;
    bool inputStringComplete;
    bool sensorStringComplete;

    void setupSerial();
    std::string printRGBData(); 
  
  public:
    // Basic Functionality
    AtlasRGB(int recv, int trans);
  
    void sendMessage();
    std::string receiveResponse(); 
    std::string run();

    // Command generation
    int setLEDBrightness(int value, bool powerSaving);
    void setIndicatorLED(bool status, bool power);

}

#endif