#ifndef WATERBEAR_SENSOR_MAP
#define WATERBEAR_SENSOR_MAP

#include "sensor.h"
#include <map>
#include <string>


template<typename T> SensorDriver * createInstance() { return new T; }

typedef std::map<short, SensorDriver*(*)()> map_type;

void buildDriverSensorMap();
SensorDriver * driverForSensorType(short type);

#define GENERIC_ANALOG_SENSOR 0x0000
<<<<<<< HEAD
#define TEMPERATURE_SENSOR 0x0001
=======


#define NO_SENSOR 0xFFFF
>>>>>>> f2f7388328dd76b5c05d526422a9d5f48fce5811

#endif