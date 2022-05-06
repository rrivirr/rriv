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
  
  setupSensorMaps<GenericAnalogDriver>(GENERIC_ANALOG_SENSOR, F(GENERIC_ANALOG_DRIVER_TYPE_STRING));
  // setupSensorMaps<AtlasEC>(GENERIC_ATLAS_SENSOR, F(ATLAS_EC_TYPE_STRING));
  setupSensorMaps<DriverTemplate>(DRIVER_TEMPLATE, F(DRIVER_TEMPLATE_TYPE_STRING));
  setupSensorMaps<AdaDHT22>(ADAFRUIT_DHT22_SENSOR, F(ADAFRUIT_DHT22_TYPE_STRING));
}

template <class T>
void setupSensorMaps(short sensorCode, const __FlashStringHelper * sensorTypeString )
{
  sensorTypeMap[sensorCode] = &createInstance<T>;
  std::string dht22String( reinterpret_cast<const char *> (sensorTypeString) );
  sensorTypeCodeMap[dht22String] = sensorCode;
}

short typeCodeForSensorTypeString(const char * type)
{
  return sensorTypeCodeMap[std::string(type)];
}

SensorDriver * driverForSensorType(short type)
{
  return sensorTypeMap[type]();
}
