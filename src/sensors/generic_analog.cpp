#include "generic_analog.h"

GenericAnalog::GenericAnalog(){}

void GenericAnalog::setup(){}

void GenericAnalog::stop(){}

bool GenericAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable

  return true;
}

char * GenericAnalog::getDataString(){
  return dataString;
}

char * GenericAnalog::getCSVColumns(){
   return csvColumnHeaders;
}

protocol_type GenericAnalog::getProtocol(){
  return analog;
}