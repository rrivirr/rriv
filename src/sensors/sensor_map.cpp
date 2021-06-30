#include "sensor_map.h"
#include "generic_analog.h"
#include "temp_analog.h"

map_type sensorTypeMap;

void buildDriverSensorMap(){
  sensorTypeMap[0x0000] = &createInstance<GenericAnalog>;
  sensorTypeMap[0x0001] = &createInstance<TempAnalog>;
}

SensorDriver * driverForSensorType(short type){
  return sensorTypeMap[type]();
}