#include "atlas_rgb.h"

// Constructor for RGB
AtlasRGB::AtlasRGB(int recv, int trans) {
  rx = recv;
  tx = trans;
  setupSerial();
  inputString = "";
  sensorString = "";
  inputStringComplete = false;
  sensorStringComplete = false;
}

// Setting up serial
AtlasRGB::setupSerial() {
  rgbSerial = SoftwareSerial(this->rx, this->tx);
  rgbSerial.begin(9600);
}

// Communicating with sensor
AtlasRGB::sendMessage() {
  rgbSerial.print(inputString);
  rgbSerial.print('\r'); // Appending a <CR>
  inputString = "";
}

// Receiving a response
AtlasRBG::receiveResponse() {
  // Getting string from sensor
  if (rgbSerial.available > 0) {
    char inputChar = (char)rgbSerial.read(); // Get the char
    sensorString += inputChar;
    if (inputChar == '\r') {
      sensorStringComplete = true;
    }
  }

  // Printing string if complete
  if (sensorStringComplete) {
    if (isdigit(sensorString[0])) {
      return printRGBData();
    }
    sensorString = "";
    sensorStringComplete = false;
  }

  return "";
}

// Print RGB Data
AtlasRGB::printRGBData() {
  std::string response = "";
  char sensorArray[30];                        
  char *red;                                          
  char *grn;                                          
  char *blu;                                          
  int intRed;                                        
  int intGrn;                                       
  int intBlu;                                        

  for (int i = 0; i < arrayLength; i++) {
      sensorArray[i] = sensorString[i];
  }

  red = strtok(sensorArray, ",");              
  grn = strtok(NULL, ",");                            
  blu = strtok(NULL, ",");                            
  
  repsonse += "RED:"; response += red;
  response += "\nGREEN:"; response += grn;
  response += "\nBLUE:"; response += blu;

  return response;
}

AtlasRGB::run() {
  if (inputString.compare("")) {
    sendMessage();
  }
  
  std::string res = receiveResponse();
  
  return res;
}

/* 
 Add individual sensor commands below
 Negative or false first argument - status query
 Return 0 on success, -1 on failure
*/

// Setting LED Brightness
AtlasRGB::setLEDBrightness(int value, bool powerSaving) {
  inputString = "L,";
  if (value < 0) {
    inputString += "?";
    return 0;
  }
  if (value >= 0 && value <= 100) {
    repsonseString += std::to_string(value);
    if (powerSaving) {
      inputString += ",T";
      return 0;
    }
  }
  return -1;
}

// Setting Indicator LED on/off
AtlasRGB::setIndicatorLED(bool status, bool power) {
  inputString = "iL,";
  if (!status) {
    inputString += "?";
  }
  else {
    if (power) {
      inputString += "1";
    }
    else {
      inputString += "0";
    }
  }
}













