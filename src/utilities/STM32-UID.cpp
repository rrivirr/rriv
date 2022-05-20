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

#include "STM32-UID.h"
#include "Arduino.h"

/*
void foobar()
{
  uint32_t idPart1 = STM32_UUID[0];
  uint32_t idPart2 = STM32_UUID[1];
  uint32_t idPart3 = STM32_UUID[2];
  //do something with the overall 96 bits
}
*/

void getSTM32UUID(unsigned char * uuid)
{
  uint32_t * stmUUID = (uint32_t *) malloc(sizeof(uint32_t) * 3);

  stmUUID[0] = STM32_UUID_F103[0];
  stmUUID[1] = STM32_UUID_F103[1];
  stmUUID[2] = STM32_UUID_F103[2];

  memcpy((void *)uuid, &stmUUID[0], 12);

  free(stmUUID);
}

void decodeUniqueId(unsigned char * uuid, char * uuidString, int uuidLength)
{
  uuidString[2 * uuidLength] = '\0';
  for (short i = 0; i < uuidLength; i++)
  {
    sprintf(&uuidString[2 * i], "%02X", (byte)uuid[i]);
  }
  Serial2.println(uuidString);
  Serial2.flush();
}