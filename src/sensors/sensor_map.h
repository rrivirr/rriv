#ifndef WATERBEAR_SENSOR_MAP
#define WATERBEAR_SENSOR_MAP

#include "sensor.h"
#include <map>
#include <string>


template<typename T> SensorDriver * createInstance() { return new T; }

typedef std::map<short, SensorDriver*(*)()> map_type;

void buildDriverSensorMap();
SensorDriver * driverForSensorType(short type);

#define GENERIC_ANALOG_SENSOR 0x00

#endif