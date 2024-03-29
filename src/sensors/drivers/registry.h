#ifndef WATERBEAR_SENSOR_REGISTRY
#define WATERBEAR_SENSOR_REGISTRY

#include "generic_analog.h"
#include "atlas_ec.h"
#include "driver_template.h"
#include "adafruit_dht22.h"
#include "generic_actuator.h"
#include "air_pump.h"
#include "BME280_driver.h"
#include "adafruit_ahtx0.h"
#include "atlas_co2_driver.h"
#include "rgb_driver.h"


#define MAX_SENSOR_TYPE 0xFFFE

void buildDriverSensorMap();

#endif