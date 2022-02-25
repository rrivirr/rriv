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

//#include "system/monitor.h" // for debug() and notify()

sensor_type_map_type sensorTypeMap;
sensor_string_map_type sensorStringTypeMap;

void buildDriverSensorMap()
{
  // Serial2.println("building driver sensor map");
  sensorTypeMap[GENERIC_ANALOG_SENSOR] = &createInstance<GenericAnalog>;
  std::string genericAnalogString(reinterpret_cast<const char *> F("generic_analog"));
  sensorStringTypeMap[genericAnalogString] = &createInstance<GenericAnalog>;

  // sensorTypeMap[GENERIC_ATLAS_SENSOR] = &createInstance<AtlasEC>;
  // std::string atlasECString(reinterpret_cast<const char *> F("atlas_ec"));
  // sensorStringTypeMap[atlasECString] = &createInstance<AtlasEC>;

  sensorTypeMap[DRIVER_TEMPLATE] = &createInstance<DriverTemplate>;
  std::string driverTemplateString(reinterpret_cast<const char *> F("driver_template"));
  sensorStringTypeMap[driverTemplateString] = &createInstance<DriverTemplate>;

  sensorTypeMap[ADAFRUIT_DHT22_SENSOR] = &createInstance<AdaDHT22>;
  std::string dht22String(reinterpret_cast<const char *> F("dht22"));
  sensorStringTypeMap[dht22String] = &createInstance<AdaDHT22>;
  
  // Serial2.println(sensorStringTypeMap.count("atlas_ec") );
  // Serial2.print("generic analog:");
  // Serial2.println(sensorStringTypeMap.count("generic_analog") );
  // Serial2.flush();
  // Serial2.print("driver_template:");
  // Serial2.println(sensorStringTypeMap.count("driver_template") );
  // Serial2.flush();
  // Serial2.println("done");
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