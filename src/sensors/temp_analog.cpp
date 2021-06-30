#include "temp_analog.h"

TempAnalog::TempAnalog(){}

void TempAnalog::stop(){}

bool TempAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable
  int measurement = 1200;
  
  return true;
}

char * TempAnalog::getDataString(){
  this->TempMV = measurement;
  // when settings file is complete add option to check for new calibration parameters to over-ride hard-coded values.
  TempF = temp.slope * TempMV + temp.int
  return dataString;
}

const char * TempAnalog::getCSVColumns(){
return CSVColumns;

}

protocol_type TempAnalog::getProtocol(){
  return analog;
}




