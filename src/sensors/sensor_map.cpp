/* 
 *  RRIV - Open Source Environmental Data Logging Platform
 *  Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "sensor_map.h"
#include "sensor_types.h"

sensor_type_map_type sensorTypeMap;
std::map<std::string, short> sensorTypeCodeMap;

void buildDriverSensorMap()
{
  // debug("building driver sensor map");
  // sensorTypeMap[GENERIC_ANALOG_SENSOR] = &createInstance<GenericAnalogDriver>;
  // std::string genericAnalogString(reinterpret_cast<const char *> F(GENERIC_ANALOG_DRIVER_TYPE_STRING));
  // sensorTypeCodeMap[genericAnalogString] = GENERIC_ANALOG_SENSOR;

  // sensorTypeMap[GENERIC_ATLAS_SENSOR] = &createInstance<AtlasEC>;
  // std::string atlasECString(reinterpret_cast<const char *> F(ATLAS_EC_TYPE_STRING));
  // sensorTypeCodeMap[atlasECString] = GENERIC_ATLAS_SENSOR;

  // sensorTypeMap[DRIVER_TEMPLATE] = &createInstance<DriverTemplate>;
  // std::string driverTemplateString(reinterpret_cast<const char *> F(DRIVER_TEMPLATE_TYPE_STRING));
  // sensorTypeCodeMap[driverTemplateString] = DRIVER_TEMPLATE;

  // sensorTypeMap[ADAFRUIT_DHT22_SENSOR] = &createInstance<AdaDHT22>;
  // std::string dht22String(reinterpret_cast<const char *> F(ADAFRUIT_DHT22_TYPE_STRING));
  // sensorTypeCodeMap[dht22String] = ADAFRUIT_DHT22_SENSOR;

  sensorTypeMap[ATLAS_CO2_SENSOR] = &createInstance<AtlasCO2Driver>;
  std::string atlasco2string(reinterpret_cast<const char *> F(ATLAS_CO2_DRIVER_TYPE_STRING));
  sensorTypeCodeMap[atlasco2string] = ATLAS_CO2_SENSOR;
}

short typeCodeForSensorTypeString(const char * type)
{
  return sensorTypeCodeMap[std::string(type)];
}

SensorDriver * driverForSensorType(short type)
{
  return sensorTypeMap[type]();
}
