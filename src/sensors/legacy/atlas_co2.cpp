/**
 * @file   atlas_co2.cpp
 * @author Shayne Marques marques.shayne24@gmail.com
 * @brief  Class declaration to interface with AtlasScientific EZO-CO2 Embedded NDIR Carbon Dioxide Sensor in i2C mode
 * https://atlas-scientific.com/probes/co2-sensor/
 */

#include "atlas_co2.h"
#include <Wire_slave.h>
#include "system/monitor.h"

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
  debug(F("In CO2 constructor"));
  strcpy(this->inputString, "");
  strcpy(this->sensorString, "");
  this->wire = wire;
  this->code = 0;
  this->data = 0;
  this->address = 105; // Default address for CO2 sensor 0x69
  this->time = 300; // Response delay time in ms
  debug(F("CO2 Constructor"));
}

void AtlasCO2::sendCommand() {
  debug("In send command: ");
  debug(this->inputString);
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
        debug(F("Success"));
        break;                         
      case 2:              
        debug(F("Failed"));             
        break;                         
      case 254:              
        debug(F("Pending"));            
        break;                         
      case 255:              
        debug(F("No Data"));
        break;                         
    }

    // Constructing response array
    while (this->wire->available()) {
      char res = this->wire->read();
      debug(res);
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
    strcpy(this->inputString, reinterpret_cast<const char *> F("G,?"));
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
    strcpy(this->inputString, reinterpret_cast<const char *> F("Name,?"));
  }
  else {
    sprintf(this->inputString, reinterpret_cast<const char *> F("Name,%s"), value);
  }
}

// Returns device information 
void AtlasCO2::deviceInformation() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("i"));
}

// Enters sleep mode
void AtlasCO2::sleepSensor() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("Sleep"));
}

// Performs a factory reset
void AtlasCO2::factoryReset() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("Factory"));
}

// Takes a single reading
void AtlasCO2::singleMode() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("R"));
}

// Calibrates sensor
void AtlasCO2::calibrateSensor() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("Cal"));
}

// Flashes LED to find sensor
void AtlasCO2::findSensor() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("Find"));
}

// Returns device status
void AtlasCO2::getStatus() {
  strcpy(this->inputString, reinterpret_cast<const char *> F("Status"));
}
