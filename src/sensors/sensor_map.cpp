#include "sensor_map.h"
#include "generic_analog.h"

map_type sensorTypeMap;

void buildDriverSensorMap(){
  sensorTypeMap[0x0000] = &createInstance<GenericAnalog>;
}

SensorDriver * driverForSensorType(short type){
  return sensorTypeMap[type]();
}