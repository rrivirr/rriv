#ifndef WATERBEAR_SENSOR_MAP
#define WATERBEAR_SENSOR_MAP

#include "sensor.h"
#include <map>
#include <string>
#include "drivers/generic_analog.h"
#include "drivers/atlas_ec.h"

template<typename T> SensorDriver * createInstance() { return new T; }

typedef std::map<short, SensorDriver*(*)()> sensor_type_map_type;
typedef std::map<std::string, SensorDriver*(*)()> sensor_string_map_type;


void buildDriverSensorMap();
SensorDriver * driverForSensorType(short type);
SensorDriver * driverForSensorTypeString(char * type);


#endif