#ifndef WATERBEAR_SENSOR_MAP
#define WATERBEAR_SENSOR_MAP

#include "sensor.h"
#include <map>
#include <string>
#include "drivers/generic_analog.h"


template<typename T> SensorDriver * createInstance() { return new T; }

typedef std::map<short, SensorDriver*(*)()> map_type;

void buildDriverSensorMap();
SensorDriver * driverForSensorType(short type);



#endif