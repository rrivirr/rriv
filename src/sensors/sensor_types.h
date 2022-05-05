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

#ifndef WATERBEAR_SENSOR_TYPES
#define WATERBEAR_SENSOR_TYPES

#define GENERIC_ANALOG_SENSOR 0x0000
#define GENERIC_ATLAS_SENSOR 0x0001
#define ADAFRUIT_DHT22_SENSOR 0x0002

#define DRIVER_TEMPLATE 0xFFFE
#define NO_SENSOR 0xFFFF

#define MAX_SENSOR_TYPE 0xFFFE // max value for number of sensor types, change if causing issues on v0.2 blue boards

#endif