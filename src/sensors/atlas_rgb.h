#ifndef WATERBEAR_EZO_RGB
#define WATERBEAR_EZO_RGB

#include <Arduino.h>
#include <Wire.h>

class AtlasRGB {
  private:
    TwoWire * wire;
    char inputString[30]; // Stores the next command to be sent to the device
    int code; // Stores the response code from the sensor
    char sensorString[53]; // Stores the response from the sensor
    int time; // Stores the delay time to receive the response from the sensor
    uint8_t address; 
    int red;
    int green;
    int blue;

  public: 
    // Instance
    static AtlasRGB * instance();

    // Constructor
    AtlasRGB();

    // Functionality
    void start(TwoWire * wire);
    void sendCommand();
    char * receiveResponse(); 
    char * run();
  
    // Command generation
    int setLEDBrightness(int value, bool powerSaving);
    void setIndicatorLED(bool status, bool power);
    void continuousMode(int value);
    void colorMatching(int value);
    int setBaudRate(int value);
    int proximityDetection(int value);
    void setI2C(int value);
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