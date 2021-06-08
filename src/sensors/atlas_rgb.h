#ifndef WATERBEAR_ATLAS_RGB
#define WATERBEAR_ATLAS_RGB

#include <string.h>
#include <Arduino.h>

class AtlasRGB {
  private:
    // Variables
    char inputString[30];
    char sensorString[30];
    bool inputStringComplete;
    bool sensorStringComplete;

    // Sensor Data
    int red;
    int green;
    int blue;
    
    // Private Functions
    void setupSerial();
    char * printRGBData(char * input); 

  public: 
    // Instance
    static AtlasRGB * instance();

    // Constructor
    AtlasRGB();

    // Functionality
    void start();
    void sendMessage();
    char * receiveResponse(); 
    char * run();
  
    // Command generation
    int setLEDBrightness(int value, bool powerSaving);
    void setIndicatorLED(bool status, bool power);
    void continuousMode(int value);
    void colorMatching(int value);
    int setBaudRate(int value);
    int proximityDetection(int value);
    int proximityDetection(char value);
    void proximityDetection(bool power);
    int gammaCorrection(float value);
    void nameDevice(char * value);
    void deviceInformation();
    void sleepSensor();
    void factoryReset();
    void singleMode();
    void calibrateSensor();
    void findSensor();
    void getStatus();
};

#endif