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
  //SoftwareSerial rgbSerial(this->rx, this->tx);
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
  int i = 0;
  while (Serial1.available() > 0) {
    char inputChar = (char)Serial1.read(); // Get the char
    Serial2.print(inputChar);
    this->sensorString[i++] = inputChar;
    if (inputChar == '\r') {
      this->sensorStringComplete = true;
      break;
    }
  }

  // Printing string if complete
  if (this->sensorStringComplete) {
    if (isdigit(this->sensorString[0])) {
      Serial2.println(this->sensorString);
      strcpy(this->sensorString, "");
      this->sensorStringComplete = false;
      memset(this->sensorString, 0, sizeof(this->sensorString));
      return this->sensorString;
    }
    else {
      Serial2.println(this->sensorString);
      strcpy(this->sensorString, "");
      this->sensorStringComplete = false;
      memset(this->sensorString, 0, sizeof(this->sensorString));
      return this->sensorString;
    }
  }

  return (char *) "";
}

// Print Sensor output in RGB format
char * AtlasRGB::printRGBData() {                       
  char *red;                                          
  char *grn;                                          
  char *blu;                                                                                

  red = strtok(this->sensorString, ",");              
  grn = strtok(NULL, ",");                            
  blu = strtok(NULL, ","); 

  Serial2.println(red);
  Serial2.println(grn);
  Serial2.println(blu);

  int int_red= atoi(red);                                 
  int int_grn= atoi(grn);                                 
  int int_blu= atoi(blu);

  char response[50];                         
  
  sprintf(response, "RED: %d", int_red);
  sprintf(response, "GREEN: %d", int_grn);
  sprintf(response, "BLUE: %d", int_blu);

  return response;
}

// Sends most recent command and receives latest response from sensor
char * AtlasRGB::run() {
  if (strcmp(this->inputString, "")) {
    sendMessage();
  }
  
  char * res = (char *) malloc(50);
  res = receiveResponse();
  
  return res;
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