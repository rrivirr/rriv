#include "ezo_rgb.h"

// Reference object
EzoRGB * rgbSensor = new EzoRGB();


// Returns instance
EzoRGB * EzoRGB::instance() {
  return rgbSensor;
}

// Dummy constructor
EzoRGB::EzoRGB(){}

// Constructor/Starter
void EzoRGB::start() {
  strcpy(this->inputString, "");
  strcpy(this->sensorString, "");
  this->inputStringComplete = false;
  this->sensorStringComplete = false;
  this->code = 0;
  this->red = 0;
  this->green = 0;
  this->blue = 0;
  this->address = 112;
  Serial2.println("RGB Constructor");
}

void sendCommand() {
  Wire2.beginTransmission(this->address);
  Wire2.write(this->inputString);
  Wire2.endTransmission();
}

byte * receiveResponse() {
  static int i = 0;
  if (strcmp(this->inputString, "sleep") != 0) {
    // Delay for response
    delay(this->time); 

    // Request for 52 bytes from the circuit
    Wire2.requestFrom(this->address, 52, 1);           
    this->code = Wire2.read();                 
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
    while (Wire2.available()) {
      sensorString[i] = Wire2.read();   
      if (sensorString[i++] == 0) {                
        i = 0;            
        break;       
      }
    }
    return sensorString;
  }
}

// Sends most recent command and receives latest response from sensor
byte * AtlasRGB::run() {
  if (strcmp(this->inputString, "")) {
    sendMessage();
  }
  return receiveResponse();
}

/**************************************************************************************** 
 Add individual sensor commands below
 Negative or false first argument - status query
 Return 0 on success, -1 on failure
*****************************************************************************************/

// Setting LED Brightness
int AtlasRGB::setLEDBrightness(int value, bool powerSaving) {
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
void AtlasRGB::setIndicatorLED(bool status, bool power) {
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

// Setting continuous mode
void AtlasRGB::continuousMode(int value) {
  if (value < 0) {
    strcpy(this->inputString, "C,?");
  }
  else {
    sprintf(this->inputString, "C,%d", value);
  }
}

// Toggling color matching
void AtlasRGB::colorMatching(int value) {
  if (value < 0) {
    strcpy(this->inputString, "M,?");
  }
  else if (value) {
    strcpy(this->inputString, "M,1");
  }
  else {
    strcpy(this->inputString, "M,0");
  }   
}

// Sets baud rate: 300, 1200, 2400, 9600, 19200, 38400, 57600, 115200
int AtlasRGB::setBaudRate(int value) {
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

// Setting proximity detection: 3 methods

// Sets custom distance: 250 - 1023
int AtlasRGB::proximityDetection(int value) {
  if (value < 0) {
    strcpy(this->inputString, "P,?");
    return 0;
  }
  if (value >= 250 && value <= 1023) {
    sprintf(this->inputString, "P,%d", value);
    return 0;
  }
  return -1;
}

// Sets predefined value: high (H), medium (M), low (L)
int AtlasRGB::proximityDetection(char value) {
  switch(value) {
    case 'H': case 'M': case 'L': 
      sprintf(this->inputString, "P,%c", value);
      return 0;
    default: 
      return -1;
  }
}

// Toggles proximity detection
void AtlasRGB::proximityDetection(bool power) {
  if (power) {
    strcpy(this->inputString, "P,1");
  }
  else {
    strcpy(this->inputString, "P,0");
  }
}

// Sets gamma correction: 0.01 - 4.99
int AtlasRGB::gammaCorrection(float value) {
  if (value < 0) {
    strcpy(this->inputString, "G,?");
    return 0;
  }
  if (value >= 0.01 && value <= 4.99) {
    sprintf(this->inputString, "P,%f", value);
    return 0;
  }
  return -1;
}

// Sets custom name for sensor
void AtlasRGB::nameDevice(char * value) {
  if (!strcmp(value, "")) {
    strcpy(this->inputString, "Name,?");
  }
  else {
    sprintf(this->inputString, "Name,%s", value);
  }
}

// Returns device information 
void AtlasRGB::deviceInformation() {
  strcpy(this->inputString, "i");
}

// Enters sleep mode
void AtlasRGB::sleepSensor() {
  strcpy(this->inputString, "Sleep");
}

// Performs a factory reset
void AtlasRGB::factoryReset() {
  strcpy(this->inputString, "Factory");
}

// Takes a single reading
void AtlasRGB::singleMode() {
  strcpy(this->inputString, "R");
}

// Calibrates sensor
void AtlasRGB::calibrateSensor() {
  strcpy(this->inputString, "Cal");
}

// Flashes LED to find sensor
void AtlasRGB::findSensor() {
  strcpy(this->inputString, "Find");
}

// Returns device status
void AtlasRGB::getStatus() {
  strcpy(this->inputString, "Status");
}
