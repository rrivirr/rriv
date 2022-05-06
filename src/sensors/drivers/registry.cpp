#include "registry.h"
#include "../sensor_map.h"
//
// Follow steps to add a new sensor driver
//

#include "generic_analog.h"
#include "atlas_ec.h"
#include "driver_template.h"
#include "adafruit_dht22.h"
// Step 1: Include the header for your driver


#define GENERIC_ANALOG_SENSOR 0x0000
#define GENERIC_ATLAS_SENSOR 0x0001
#define ADAFRUIT_DHT22_SENSOR 0x0002
#define ATLAS_CO2_SENSOR 0x0003
// Step 2: Add a #define for the next available integer code

#define DRIVER_TEMPLATE 0xFFFE
#define NO_SENSOR 0xFFFF



void buildDriverSensorMap()
{  
  setupSensorMaps<GenericAnalogDriver>(GENERIC_ANALOG_SENSOR, F(GENERIC_ANALOG_DRIVER_TYPE_STRING));
  // setupSensorMaps<AtlasEC>(GENERIC_ATLAS_SENSOR, F(ATLAS_EC_TYPE_STRING));
  setupSensorMaps<DriverTemplate>(DRIVER_TEMPLATE, F(DRIVER_TEMPLATE_TYPE_STRING));
  setupSensorMaps<AdaDHT22>(ADAFRUIT_DHT22_SENSOR, F(ADAFRUIT_DHT22_TYPE_STRING));
  setupSensorMaps<AtlasCO2Driver>(ATLAS_CO2_SENSOR, F(ATLAS_CO2_DRIVER_TYPE_STRING));
  // Step 3: call setupSensorMaps with the class name, code, and type string for your sensor
}
