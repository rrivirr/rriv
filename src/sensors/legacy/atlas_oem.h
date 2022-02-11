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

#ifndef WATERBEAR_ATLAS_OEM
#define WATERBEAR_ATLAS_OEM

#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include <EC_OEM.h>

void setupEC_OEM(TwoWire * wire);
void hibernateEC_OEM();
void clearECCalibrationData();
void setECDryPointCalibration();
void setECLowPointCalibration(float lowPoint);
void setECHighPointCalibration(float highPoint);
bool readECDataIfAvailable(float * value);

#endif