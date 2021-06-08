#include "atlas_rgb.h"

// Reference object
AtlasRGB * rgbSensor = new AtlasRGB();


// Returns instance
AtlasRGB * AtlasRGB::instance() {
  return rgbSensor;
}

// Dummy constructor
AtlasRGB::AtlasRGB(){}

// Constructor/Starter
void AtlasRGB::start() {
  setupSerial();
  strcpy(this->inputString, "");
  strcpy(this->sensorString, "");
  this->inputStringComplete = false;
  this->sensorStringComplete = false;
  Serial2.println("RGB Constructor");
}

// Setting up serial
void AtlasRGB::setupSerial() {
  Serial1.begin(9600);
  Serial2.println("Setup Serial1");
}

// Communicating with sensor
void AtlasRGB::sendMessage() {
  Serial1.print(this->inputString);
  Serial1.print('\r'); // Appending a <CR>
  Serial2.print("Input command: ");
  Serial2.println(this->inputString);
  strcpy(this->inputString, "");
  memset(this->inputString, 0, sizeof(this->inputString));
}

// Receiving a response
char * AtlasRGB::receiveResponse() {
  // Getting string from sensor
  static int i = 0;
  if (Serial1.available() > 0) {
    char inputChar = (char)Serial1.read(); // Get the char
    this->sensorString[i++] = inputChar;
    if (inputChar == '\r') {
      this->sensorStringComplete = true;
      i = 0;
    }
  }

  // Printing string if complete
  if (this->sensorStringComplete) {
    char * copy = (char *) malloc(strlen(this->sensorString) + 1);
    strcpy(copy, this->sensorString);
    strcpy(this->sensorString, "");
    this->sensorStringComplete = false;
    memset(this->sensorString, 0, sizeof(this->sensorString));
    if (isdigit(copy[0])) {
      Serial2.println(printRGBData(copy));
      return copy;
    }
    else {
      Serial2.println(copy);
      return copy;
    }
  }
}

// Print input string in RGB format
char * AtlasRGB::printRGBData(char * input) {                       
  int red= atoi(strtok(input, ","));                                 
  int grn= atoi(strtok(NULL, ","));                                 
  int blu= atoi(strtok(NULL, ","));

  char * response = (char *) malloc(50);                         
  
  sprintf(response, "RED: %d GREEN: %d BLUE: %d", red, grn, blu);

  return response;
}

// Sends most recent command and receives latest response from sensor
char * AtlasRGB::run() {
  if (strcmp(this->inputString, "")) {
    sendMessage();
  }
  return receiveResponse();
}

/* 
 Add individual sensor commands below
 Negative or false first argument - status query
 Return 0 on success, -1 on failure
*/

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

// Flashes LED to find sensor
void AtlasRGB::findSensor() {
  strcpy(this->inputString, "Find");
}

// Setting continuous mode
void AtlasRGB::continuousMode(int value) {
  if (value < 0) {
    strcpy(this->inputString, "C,?");
    return;
  }
  sprintf(this->inputString, "C,%d", value);
}

// Takes a single reading
void AtlasRGB::singleMode() {
  strcpy(this->inputString, "R");
}

// Calibrates sensor
void AtlasRGB::calibrateSensor() {
  strcpy(this->inputString, "Cal");
}

// Toggling color matching
void AtlasRGB::colorMatching(int value) {
  if (value < 0) {
    strcpy(this->inputString, "M,?");
    return;
  }
  if (value) {
    strcpy(this->inputString, "M,1");
  }
  else {
    strcpy(this->inputString, "M,0");
  }
  return;
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