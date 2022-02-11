/* 
 *  RRIV - Open Source Environmental Data Logging Platform
 *  Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file   atlas_rgb.h
 * @author Shayne Marques marques.shayne24@gmail.com
 * @brief  Class declaration to interface with AtlasScientific EZO-RGB Color Sensor in i2C mode
 * https://atlas-scientific.com/probes/color-sensor/
 */


#ifndef WATERBEAR_EZO_RGB
#define WATERBEAR_EZO_RGB

#include <Arduino.h>
#include <Wire_slave.h>

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

    // interface
    int dataMemorySize = 20;

  public: 
    // Instance
    static AtlasRGB * instance();

    // Constructor
    AtlasRGB();

    // Interface
    void setup(TwoWire * wire);
    void stop();
    char * mallocDataMemory();
    void takeMeasurement(char * data);


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