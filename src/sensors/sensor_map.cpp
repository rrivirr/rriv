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
#include "Arduino.h"

sensor_type_map_type sensorTypeMap;
std::map<std::string, short> sensorTypeCodeMap;


short typeCodeForSensorTypeString(const char * type)
{
  return sensorTypeCodeMap[std::string(type)];
}

SensorDriver * driverForSensorTypeCode(short type)
{
  if(sensorTypeMap[type] == NULL)
  {
    return NULL;
  }
  return sensorTypeMap[type]();
}
