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