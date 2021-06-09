#ifndef WATERBEAR_ATLAS_CO2
#define WATERBEAR_ATLAS_CO2

#include <string.h>
#include <Arduino.h>

class AtlasCO2 {
  private:
    // Variables
    char inputString[30];
    char sensorString[30];
    bool inputStringComplete;
    bool sensorStringComplete;

    // Sensor Data
    int data;
    
    // Private Functions
    void setupSerial();

  public: 
    // Instance
    static AtlasCO2 * instance();

    // Constructor
    AtlasCO2();

    // Functionality
    void start();
    void sendMessage();
    char * receiveResponse(); 
    char * run();
  
    // Command generation
    void setIndicatorLED(bool status, bool power);
    int setAlarm(char * msg, int value);
    int calibrateSensor(bool clear, int value);
    void continuousMode(int value);
    int setBaudRate(int value);
    int enableTemperature(int value);
    void nameDevice(char * value);
    void exportCalibration(bool status);
    void deviceInformation();
    void sleepSensor();
    void factoryReset();
    void singleMode();
    void findSensor();
    void getStatus();
};

#endif