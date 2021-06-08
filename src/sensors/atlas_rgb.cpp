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
  this->red = 0;
  this->green = 0;
  this->blue = 0;
  Serial2.println("RGB Constructor");
}

// Setting up serial
void AtlasRGB::setupSerial() {
  Serial1.begin(9600);
  Serial2.println("Serial1 setup complete");
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
    // Deep copy of sensor response
    char * copy = (char *) malloc(strlen(this->sensorString) + 1);
    strcpy(copy, this->sensorString);
    
    // Clearing sensorString for next response
    strcpy(this->sensorString, "");
    this->sensorStringComplete = false;
    memset(this->sensorString, 0, sizeof(this->sensorString));
    
    if (isdigit(copy[0])) {
      // Prints RGB format output to Serial
      Serial2.println(printRGBData(copy));
      return copy;
    }
    else {
      // Prints commmand outputs to Serial
      Serial2.println(copy);
      return copy;
    }
  }

  return (char *) "";
}

// Print input string in RGB format
char * AtlasRGB::printRGBData(char * input) {                       
  this->red= atoi(strtok(input, ","));                                 
  this->green= atoi(strtok(NULL, ","));                                 
  this->blue= atoi(strtok(NULL, ","));

  char * response = (char *) malloc(50);                         
  
  sprintf(response, "RED: %d GREEN: %d BLUE: %d", this->red, this->green, this->blue);

  return response;
}

// Sends most recent command and receives latest response from sensor
char * AtlasRGB::run() {
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