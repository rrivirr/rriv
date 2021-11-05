#include "generic_analog.h"
#include "system/monitor.h"

GenericAnalog::GenericAnalog()
{
  Serial2.println("hello");
  Serial2.flush();
  // debug("allocation GenericAnalog");
}

void GenericAnalog::setup()
{
  debug("setup GenericAnalog");
}

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