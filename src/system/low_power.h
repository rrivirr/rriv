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

#ifndef WATERBEAR_LOW_POWER
#define WATERBEAR_LOW_POWER

#include "datalogger.h"

// Port everyting into C++ classes and don't be lazy
// class WB_LowPower {

// public:
  void enterStopMode();
  void enterSleepMode();
  void componentsAlwaysOff(); // turn off unused components during setup
  void hardwarePinsAlwaysOff(); // disable unused hardware pins during setup
  void componentsStopMode(); // for stop/sleep mode
  void hardwarePinsStopMode(); // for stop/sleep mode
  void restorePinDefaults(); // not necessary?
  void componentsBurstMode(); // wake up components
// }

void disableSerialLog();
void enableSerialLog();

#endif