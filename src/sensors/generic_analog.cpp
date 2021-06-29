#include "generic_analog.h"

GenericAnalog::GenericAnalog(){}

void GenericAnalog::stop(){}

void GenericAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable
}

char * GenericAnalog::getDataString(){
  return dataString;
}

char * GenericAnalog::getCSVColumns(){

}

protocol_type GenericAnalog::getProtocol(){
  return analog;
}