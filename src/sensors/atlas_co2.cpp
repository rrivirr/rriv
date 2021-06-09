#include "atlas_co2.h"

// Reference object
AtlasCO2 * rgbSensor = new AtlasCO2();


// Returns instance
AtlasCO2 * AtlasCO2::instance() {
  return rgbSensor;
}

// Dummy constructor
AtlasCO2::AtlasCO2(){}

// Constructor/Starter
void AtlasCO2::start() {
  setupSerial();
  strcpy(this->inputString, "");
  strcpy(this->sensorString, "");
  this->inputStringComplete = false;
  this->sensorStringComplete = false;
  this->data = 0;
  Serial2.println("RGB Constructor");
}

// Setting up serial
void AtlasCO2::setupSerial() {
  Serial1.begin(9600);
  Serial2.println("Serial1 setup complete");
}

// Communicating with sensor
void AtlasCO2::sendMessage() {
  Serial1.print(this->inputString);
  Serial1.print('\r'); // Appending a <CR>
  Serial2.print("Input command: ");
  Serial2.println(this->inputString);

  strcpy(this->inputString, "");
  memset(this->inputString, 0, sizeof(this->inputString));
}

// Receiving a response
char * AtlasCO2::receiveResponse() {
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
    
    // Prints commmand outputs to Serial
    Serial2.println(copy);
    return copy;
  }

  return (char *) "";
}

// Sends most recent command and receives latest response from sensor
char * AtlasCO2::run() {
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

// Setting Indicator LED on/off
void AtlasCO2::setIndicatorLED(bool status, bool power) {
  strcpy(this->inputString, "L,");
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
void AtlasCO2::continuousMode(int value) {
  if (value < 0) {
    strcpy(this->inputString, "C,?");
  }
  else {
    sprintf(this->inputString, "C,%d", value);
  }
}

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