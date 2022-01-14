#include "sensor_map.h"
#include "sensor_types.h"

sensor_type_map_type sensorTypeMap;
sensor_string_map_type sensorStringTypeMap;

void buildDriverSensorMap()
{
  sensorTypeMap[GENERIC_ANALOG_SENSOR] = &createInstance<GenericAnalog>;
  sensorTypeMap[GENERIC_ATLAS_SENSOR] = &createInstance<AtlasEC>;

  std::string genericAnalogString(reinterpret_cast<const char *> F("generic_analog"));
  std::string atlasECString(reinterpret_cast<const char *> F("atlas_ec"));
  sensorStringTypeMap[genericAnalogString] = &createInstance<GenericAnalog>;
  sensorStringTypeMap[atlasECString] = &createInstance<AtlasEC>;

  Serial2.println(sensorStringTypeMap.count("atlas_ec") );
  Serial2.println(sensorStringTypeMap.count("generic_analog") );
  Serial2.println("done");
}

SensorDriver * driverForSensorType(short type)
{
  return sensorTypeMap[type]();
}

SensorDriver * driverForSensorTypeString(char * type)
{
  std::string typeString(type);
  return sensorStringTypeMap[typeString]();
}