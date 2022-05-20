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

sensor_type_map_type sensorTypeMap;
std::map<std::string, short> sensorTypeCodeMap;

// hacking this for getting sensor working
#define ATLAS_CO2_SENSOR 0x0003

void setupSensorMapsLegacy()
{
  sensorTypeMap[ATLAS_CO2_SENSOR] = &createInstance<AtlasCO2Driver>;

  std::string dht22String( reinterpret_cast<const char *> (ATLAS_CO2_DRIVER_TYPE_STRING) );

  sensorTypeCodeMap[dht22String] = ATLAS_CO2_SENSOR;
}

template <typename T>
void setupSensorMaps(short sensorCode, const __FlashStringHelper * sensorTypeString )
{
  Serial2.println("create sensor map"); Serial2.flush();
  sensorTypeMap[sensorCode] = &createInstance<T>;
    Serial2.println("create sensor map"); Serial2.flush();
  

  std::string dht22String( reinterpret_cast<const char *> (sensorTypeString) );
    Serial2.println("create sensor map"); Serial2.flush();

  sensorTypeCodeMap[dht22String] = sensorCode;
    Serial2.println("create sensor map"); Serial2.flush();

}

short typeCodeForSensorTypeString(const char * type)
{
  return sensorTypeCodeMap[std::string(type)];
}

bool sensorTypeCodeExists(short type)
{
  return sensorTypeMap.count(type) > 0;
}

SensorDriver * driverForSensorTypeCode(short type)
{
  if(sensorTypeMap[type] != NULL)
  {
    Serial2.println("not null");
    return sensorTypeMap[type]();
  } else {
    return NULL;
  }
}
