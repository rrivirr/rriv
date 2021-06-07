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
  
  public:
    AtlasRGB(int recv, int trans);
    void setupSerial();
    void serialEvent();
    void printRGBData();     
}

#endif