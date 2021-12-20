#include "atlas_rgb.h"

// Constructor for RGB
char inputString[30];
char sensorString[30];
bool inputStringComplete;
bool sensorStringComplete;

void start() {
  setupSerial();
  strcpy(inputString, "");
  strcpy(sensorString, "");
  inputStringComplete = false;
  sensorStringComplete = false;
  Serial2.println("RGB Constructor");
}

// Setting up serial
void setupSerial() {
  //SoftwareSerial rgbSerial(this->rx, this->tx);
  Serial1.begin(9600);
  Serial2.println("Setup Serial1");
}

// Communicating with sensor
void sendMessage() {
  Serial1.print(inputString);
  Serial1.print('\r'); // Appending a <CR>
  Serial2.print("Input command: ");
  Serial2.println(inputString);
  strcpy(inputString, "");
  memset(inputString, 0, sizeof(inputString));
}

// Receiving a response
char * receiveResponse() {
  // Getting string from sensor
  int i = 0;
  while (Serial1.available() > 0) {
    char inputChar = (char)Serial1.read(); // Get the char
    Serial2.print(inputChar);
    sensorString[i++] = inputChar;
    if (inputChar == '\r') {
      sensorStringComplete = true;
      break;
    }
  }

  // Printing string if complete
  if (sensorStringComplete) {
    if (isdigit(sensorString[0])) {
      Serial2.println(sensorString);
      strcpy(sensorString, "");
      sensorStringComplete = false;
      memset(sensorString, 0, sizeof(sensorString));
      return sensorString;
    }
    else {
      Serial2.println(sensorString);
      strcpy(sensorString, "");
      sensorStringComplete = false;
      memset(sensorString, 0, sizeof(sensorString));
      return sensorString;
    }
  }

  return (char *) "";
}

// Print RGB Data
char * printRGBData() {                       
  char *red;                                          
  char *grn;                                          
  char *blu;                                                                                

  red = strtok(sensorString, ",");              
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

char * run() {
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
int setLEDBrightness(int value, bool powerSaving) {
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
    return 0;
  }
  return -1;
}

// Setting Indicator LED on/off
void setIndicatorLED(bool status, bool power) {
  strcpy(inputString, "iL,");
  if (status) {
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

// Flashes LED to find sensor
void findSensor() {
  strcpy(inputString, "Find");
}

// Setting continuous mode
void continuousMode(int value) {
  if (value < 0) {
    strcpy(inputString, "C,?");
    return;
  }
  sprintf(inputString, "C,%d", value);
}

// Takes a single reading
void singleMode() {
  strcpy(inputString, "R");
}

// Calibrates sensor
void calibrateSensor() {
  strcpy(inputString, "Cal");
}

// Toggling color matching
void colorMatching(int value) {
  if (value < 0) {
    strcpy(inputString, "M,?");
    return;
  }
  if (value) {
    strcpy(inputString, "M,1");
  }
  else {
    strcpy(inputString, "M,0");
  }
  return;
}

// Returns device information 
void deviceInformation() {
  strcpy(inputString, "i");
}

// Enters sleep mode
void sleepSensor() {
  strcpy(inputString, "Sleep");
}

// Performs a factory reset
void factoryReset() {
  strcpy(inputString, "Factory");
}