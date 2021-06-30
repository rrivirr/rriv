#include "temp_analog.h"

TempAnalog::TempAnalog(){}

void TempAnalog::stop(){}

bool TempAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable
  //int value = analogRead(sensorPins[i]);
  int measurement = 1200;
  this->TempMV = measurement;
  return true;
}

char * TempAnalog::getDataString(){
  

  // when settings file is complete add option to check for new calibration parameters to over-ride hard-coded values.
  TempC = ((CalM/CalMMult) * TempMV + (CalB/CalBMult)) * TempCMult;
  return dataString;
}

const char * TempAnalog::getCSVColumns(){
return CSVColumns;

}

protocol_type TempAnalog::getProtocol(){
  return analog;
}




