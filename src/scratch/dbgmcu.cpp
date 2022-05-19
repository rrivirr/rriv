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

#include "Arduino.h"
#include "system/logs.h"
#include "dbgmcu.h" 

bool checkDebugSystemDisabled()
{
  return DBGMCU_BASE->CR == 0;
}

void notifyDebugStatus()
{
  if(DBGMCU_BASE->CR != 0){
    notify("**** WARNING *****");
    notify("**** A MANUAL POWER CYCLE IS REQUIRED TO ENABLE LOW POWER MODES *****");
  }
}

void printMCUDebugStatus()
{
  notify("Setup:DBGMCU_BASE->CR=");
  notify(DBGMCU_BASE->CR);

  if(DBGMCU_BASE->CR != 0){
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      delay(2000);
  } else {
    notify("DEBUG MODE OFF - OK!");
  }
}