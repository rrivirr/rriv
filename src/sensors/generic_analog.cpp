#include "generic_analog.h"

GenericAnalog::GenericAnalog(){}

void GenericAnalog::stop(){}

bool GenericAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable

  return true;
}

char * GenericAnalog::getDataString(){
  return dataString;
}

char * GenericAnalog::getCSVColumns(){

}

protocol_type GenericAnalog::getProtocol(){
  return analog;
}