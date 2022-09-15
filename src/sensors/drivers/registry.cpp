#include "registry.h"
#include "sensors/sensor_map.h"
#include "system/logs.h"
//
// Follow steps to add a new sensor driver
//

// Step 1: Include the header for your driver in registry.h

// Step 2: Add a #define for the next available integer code
#define GENERIC_ANALOG_SENSOR 0x0000
#define ATLAS_EC_OEM_SENSOR 0x0001
#define ADAFRUIT_DHT22_SENSOR 0x0002
#define ATLAS_CO2_SENSOR 0x0003
#define ATLAS_RGB_SENSOR 0x0004


#define DRIVER_TEMPLATE 0xFFFE
#define NO_SENSOR 0xFFFF



void buildDriverSensorMap()
{  
// Step 3: call setupSensorMaps with the class name, code, and type string for your sensor

  // setupSensorMaps<GenericAnalogDriver>(GENERIC_ANALOG_SENSOR, F(GENERIC_ANALOG_DRIVER_TYPE_STRING));

  // setupSensorMaps<AtlasECDriver>(ATLAS_EC_OEM_SNSOR, F(ATLAS_EC_OEM_TYPE_STRING));

  // setupSensorMaps<DriverTemplate>(DRIVER_TEMPLATE, F(DRIVER_TEMPLATE_TYPE_STRING));
 
  // setupSensorMaps<AdaDHT22>(ADAFRUIT_DHT22_SENSOR, F(ADAFRUIT_DHT22_TYPE_STRING));
  setupSensorMaps<rgbDriver>(ATLAS_RGB_SENSOR, F(RGB_DRIVER_TYPE_STRING));

  // setupSensorMaps<AtlasCO2Driver>(ATLAS_CO2_SENSOR, F(ATLAS_CO2_DRIVER_TYPE_STRING));

  
  // setupSensorMaps<$CLASS_NAME>($SENSOR_CODE, F($SENSOR_STRING_NAME));
  // $CLASS_NAME is the C++ class of the sensor
  // $SENSOR_CODE is the define added in step 2 above for this sensor
  // $SENSOR_STRING_NAME is the define for human readable sensor name found in the .h for this sensor driver


}
