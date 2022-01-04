/**
 * @file   atlas_co2.h
 * @author Shayne Marques marques.shayne24@gmail.com
 * @brief  Class definition to interface with AtlasScientific EZO-CO2 Embedded NDIR Carbon Dioxide Sensor in i2C mode
 * https://atlas-scientific.com/probes/co2-sensor/
 */

#ifndef WATERBEAR_EZO_CO2
#define WATERBEAR_EZO_CO2

#include <Arduino.h>
#include <Wire_slave.h>

class AtlasCO2 {
  private:
    TwoWire * wire;
    char inputString[30]; // Stores the next command to be sent to the device
    int code; // Stores the response code from the sensor
    char sensorString[53]; // Stores the response from the sensor
    int time; // Stores the delay time to receive the response from the sensor
    uint8_t address; 
    int data;

  public: 
    // Instance
    static AtlasCO2 * instance();

    // Constructor
    AtlasCO2();

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