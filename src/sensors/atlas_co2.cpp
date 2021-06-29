/**
 * @file   atlas_co2.cpp
 * @author Shayne Marques marques.shayne24@gmail.com
 * @brief  Class declaration to interface with AtlasScientific EZO-CO2 Embedded NDIR Carbon Dioxide Sensor in i2C mode
 * https://atlas-scientific.com/probes/co2-sensor/
 */

#include "atlas_co2.h"
#include <Wire_slave.h>

// Reference object
AtlasCO2 * co2Sensor = new AtlasCO2();


// Returns instance
AtlasCO2 * AtlasCO2::instance() {
  return co2Sensor;
}

// Dummy constructor
AtlasCO2::AtlasCO2(){}

// Constructor/Starter
void AtlasCO2::start(TwoWire * wire) {
  Serial2.println("In CO2 constructor");
  strcpy(this->inputString, "");
  strcpy(this->sensorString, "");
  this->wire = wire;
  this->code = 0;
  this->data = 0;
  this->address = 105; // Default address for CO2 sensor 0x69
  this->time = 300; // Response delay time in ms
  Serial2.println("CO2 Constructor");
}

void AtlasCO2::sendCommand() {
  Serial2.print("In send command: ");
  Serial2.println(this->inputString);
  this->wire->beginTransmission(this->address);
  this->wire->write(this->inputString);
  this->wire->endTransmission();
  strcpy(this->inputString, "");
}

char * AtlasCO2::receiveResponse() {
  static int i = 0;
  if (strcmp(this->inputString, "sleep") != 0) {
    
    // Delay for response
    delay(this->time); 

    // Request for 52 bytes from the circuit
    this->wire->requestFrom(this->address, 52);
    
    // Get response code           
    this->code = this->wire->read();                 
    
    switch (this->code) {                  
      case 1:              
        Serial2.println("Success");
        break;                         
      case 2:              
        Serial2.println("Failed");             
        break;                         
      case 254:              
        Serial2.println("Pending");            
        break;                         
      case 255:              
        Serial2.println("No Data");
        break;                         
    }

    // Constructing response array
    while (this->wire->available()) {
      char res = this->wire->read();
      Serial2.println(res);
      this->sensorString[i++] = res;   
      if (res == 0) {                
        i = 0;            
        break;       
      }
    }
    return this->sensorString;
  }
  return (char *) "";
}

// Sends most recent command and receives latest response from sensor
char * AtlasCO2::run() {
  if (strcmp(this->inputString, "")) {
    sendCommand();
  }
  receiveResponse();
  return this->sensorString;
}

/**************************************************************************************** 
 Add individual sensor commands below
 Negative or false first argument - status query
 Return 0 on success, -1 on failure
*****************************************************************************************/

// Setting LED Brightness
int AtlasCO2::setLEDBrightness(int value, bool powerSaving) {
  if (value < 0) {
    strcpy(this->inputString, "L,");
    strcat(this->inputString, "?");
    return 0;
  }
  if (value >= 0 && value <= 100) {
    if (powerSaving) {
      sprintf(this->inputString, "L,%d,T", value);
      return 0;
    }
    sprintf(this->inputString, "L,%d", value);
    return 0;
  }
  return -1;
}

// Setting Indicator LED on/off
void AtlasCO2::setIndicatorLED(bool status, bool power) {
  strcpy(this->inputString, "iL,");
  if (status) {
    strcat(this->inputString, "?");
  }
  else {
    if (power) {
      strcat(this->inputString, "1");
    }
    else {
      strcat(this->inputString, "0");
    }
  }
}

// Switches back to UART Mode
// Sets baud rate: 300, 1200, 2400, 9600, 19200, 38400, 57600, 115200
int AtlasCO2::setBaudRate(int value) {
  if (value < 0) {
    strcpy(this->inputString, "Baud,?");
    return 0;
  }
  else {
    switch(value) {
      case 300: case 1200: case 2400: case 9600: case 19200: case 38400: case 57600: case 115200: 
        sprintf(this->inputString, "Baud,%d", value);
        return 0;
      default:
        return -1;
    }
  }
}

// Sets gamma correction: 0.01 - 4.99
int AtlasCO2::gammaCorrection(float value) {
  if (value < 0) {
    strcpy(this->inputString, "G,?");
    return 0;
  }
  if (value >= 0.01 && value <= 4.99) {
    sprintf(this->inputString, "G,%f", value);
    return 0;
  }
  return -1;
}

// Sets custom name for sensor
void AtlasCO2::nameDevice(char * value) {
  if (!strcmp(value, "")) {
    strcpy(this->inputString, "Name,?");
  }
  else {
    sprintf(this->inputString, "Name,%s", value);
  }
}

// Returns device information 
void AtlasCO2::deviceInformation() {
  strcpy(this->inputString, "i");
}

// Enters sleep mode
void AtlasCO2::sleepSensor() {
  strcpy(this->inputString, "Sleep");
}

// Performs a factory reset
void AtlasCO2::factoryReset() {
  strcpy(this->inputString, "Factory");
}

// Takes a single reading
void AtlasCO2::singleMode() {
  strcpy(this->inputString, "R");
}

// Calibrates sensor
void AtlasCO2::calibrateSensor() {
  strcpy(this->inputString, "Cal");
}

// Flashes LED to find sensor
void AtlasCO2::findSensor() {
  strcpy(this->inputString, "Find");
}

// Returns device status
void AtlasCO2::getStatus() {
  strcpy(this->inputString, "Status");
}
