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

#include "monitor.h"
#include "utilities/utilities.h"




Monitor * monitor = new Monitor();

// get default logger
Monitor *Monitor::instance()
{
  return monitor;
}

Monitor::Monitor() {}

void Monitor::writeSerialMessage(const char *message)
{
  Serial2.println(message);
  Serial2.flush();
}

void Monitor::writeSerialMessage(const __FlashStringHelper *message)
{
  Serial2.println(message);
  Serial2.flush();
}

void Monitor::writeDebugMessage(const char *message)
{
  if (this->debugToSerial)
  {
    this->writeSerialMessage(message);
  }

  if (this->debugToFile && this->filesystem != NULL)
  {
    this->filesystem->writeDebugMessage(message);
  }
}

void Monitor::writeDebugMessage(const int number)
{
  char message[10];
  sprintf(message, "%d", number);
  this->writeDebugMessage(message);
}

void Monitor::writeDebugMessage(int number, int base)
{
  unsigned char buf[CHAR_BIT * sizeof(long long)];
  unsigned long i = 0;
  if (number == 0)
  {
    this->writeDebugMessage('0');
    return;
  }

  while (number > 0)
  {
    buf[i++] = number % base;
    number /= base;
  }

  for (; i > 0; i--)
  {
    this->writeDebugMessage((char)(buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
  }
}

void Monitor::writeDebugMessage(const __FlashStringHelper *message)
{
  if (this->debugToSerial)
  {
    this->writeSerialMessage(message);
  }

  if (this->debugToFile && this->filesystem != NULL)
  {
    this->filesystem->writeDebugMessage(reinterpret_cast<const char *>(message));
  }
}

// A small helper
void Monitor::error(const __FlashStringHelper *err)
{
  writeDebugMessage(F("Error:"));
  writeDebugMessage(err);
  while (1)
    ;
}