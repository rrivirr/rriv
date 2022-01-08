#include "sensor_map.h"
#include "sensor_types.h"

map_type sensorTypeMap;

void buildDriverSensorMap(){
  sensorTypeMap[GENERIC_ANALOG_SENSOR] = &createInstance<GenericAnalog>;
  sensorTypeMap[GENERIC_ATLAS_SENSOR] = &createInstance<GenericAtlas>;
}

SensorDriver * driverForSensorType(short type){
  return sensorTypeMap[type]();
}