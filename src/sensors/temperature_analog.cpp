#include "temperature_analog.h"

TemperatureAnalog::TemperatureAnalog(){}

void TemperatureAnalog::stop(){}

bool TemperatureAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable
  //int value = analogRead(sensorPins[i]);
  int measurement = 1200;
  this->TempMV = measurement;
  return true;
}

char * TemperatureAnalog::getDataString(){
  

  // when settings file is complete add option to check for new calibration parameters to over-ride hard-coded values.
  TempC = ((CalM/CalMMult) * TempMV + (CalB/CalBMult)) * TempCMult;
  return dataString;
}

const char * TemperatureAnalog::getCSVColumns(){
return CSVColumns;

}

protocol_type TemperatureAnalog::getProtocol(){
  return analog;
}




