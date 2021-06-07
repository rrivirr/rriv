#include "atlas_rgb.h"

// Constructor for RGB
AtlasRGB::AtlasRGB() {
  // rx = recv;
  // tx = trans;
  setupSerial();
  strcpy(inputString, "");
  strcpy(sensorString, "");
  inputStringComplete = false;
  sensorStringComplete = false;
}

// Setting up serial
void AtlasRGB::setupSerial() {
  //SoftwareSerial rgbSerial(this->rx, this->tx);
  Serial3.begin(9600);
}

// Communicating with sensor
void AtlasRGB::sendMessage() {
  Serial3.print(inputString);
  Serial3.print('\r'); // Appending a <CR>
  strcpy(inputString, "");
}

// Receiving a response
char * AtlasRGB::receiveResponse() {
  // Getting string from sensor
  if (Serial3.available() > 0) {
    char inputChar = (char)Serial3.read(); // Get the char
    strcat(sensorString, &inputChar);
    if (inputChar == '\r') {
      sensorStringComplete = true;
    }
  }

  // Printing string if complete
  if (sensorStringComplete) {
    if (isdigit(sensorString[0])) {
      return printRGBData();
    }
    strcpy(sensorString, "");
    sensorStringComplete = false;
  }

  return (char *) "";
}

// Print RGB Data
char * AtlasRGB::printRGBData() {                       
  char *red;                                          
  char *grn;                                          
  char *blu;                                                                                

  red = strtok(sensorString, ",");              
  grn = strtok(NULL, ",");                            
  blu = strtok(NULL, ","); 

  int int_red= atoi(red);                                 
  int int_grn= atoi(grn);                                 
  int int_blu= atoi(blu);

  char response[50];                         
  
  sprintf(response, "RED: %d", int_red);
  sprintf(response, "GREEN: %d", int_grn);
  sprintf(response, "BLUE: %d", int_blu);

  return response;
}

char * AtlasRGB::run() {
  if (strcmp(inputString, "")) {
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


char * toArray(int number)
{
    int n = log10(number) + 1;
    int i;
    char *numberArray = (char *) calloc(n, sizeof(char));
    for (i = n-1; i >= 0; --i, number /= 10)
    {
        numberArray[i] = (number % 10) + '0';
    }
    return numberArray;
} 

// Setting LED Brightness
int AtlasRGB::setLEDBrightness(int value, bool powerSaving) {
  if (value < 0) {
    strcpy(inputString, "L,");
    strcat(inputString, "?");
    return 0;
  }
  if (value >= 0 && value <= 100) {
    if (powerSaving) {
      sprintf(inputString, "L,%d,T", value);
      return 0;
    }
    sprintf(inputString, "L,%d", value);
  }
  return -1;
}

// Setting Indicator LED on/off
void AtlasRGB::setIndicatorLED(bool status, bool power) {
  strcpy(inputString, "iL,");
  if (!status) {
    strcat(inputString, "?");
  }
  else {
    if (power) {
      strcat(inputString, "1");
    }
    else {
      strcat(inputString, "0");
    }
  }
}













