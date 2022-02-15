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

#include "utilities.h"
#include "system/monitor.h"
#include "system/clock.h"

// For F103RM convienience in this file
#define Serial Serial2



// void printInterruptStatus(HardwareSerial &serial)
// {
//   serial.println(F("NVIC->ISER:"));
//   serial.println(NVIC_BASE->ISER[0], BIN);
//   serial.println(NVIC_BASE->ISER[1], BIN);
//   serial.println(NVIC_BASE->ISER[2], BIN);
//   serial.println(EXTI_BASE->PR, BIN);
//   serial.flush();
// }

void printDateTime(HardwareSerial &serial, DateTime now)
{
  serial.println(now.unixtime());

  serial.print(now.year(), DEC);
  serial.print('/');
  serial.print(now.month(), DEC);
  serial.print('/');
  serial.print(now.day(), DEC);
  // Serial.print(" (");
  // Serial.print(now.dayOfTheWeek());
  // Serial.print(") ");
  serial.print("  ");
  serial.print(now.hour(), DEC);
  serial.print(':');
  serial.print(now.minute(), DEC);
  serial.print(':');
  serial.print(now.second(), DEC);
  serial.println();
  serial.flush();
}

void blink(int times, int duration)
{
  pinMode(PA5, OUTPUT);
  for (int i = times; i > 0; i--)
  {
    digitalWrite(PA5, HIGH);
    delay(duration);
    digitalWrite(PA5, LOW);
    delay(duration);
  }
}

void printDS3231Time()
{
  char testTime[11]; // timestamp responses
  sprintf(testTime, "TS: %lld", timestamp()); // convert time_t value into string
  Monitor::instance()->writeDebugMessage(testTime);
}

// void printNVICStatus()
// {
//   char  message[100];
//   sprintf(message, "1: NVIC_BASE->ISPR\n%" PRIu32"\n%" PRIu32"\n%" PRIu32, NVIC_BASE->ISPR[0], NVIC_BASE->ISPR[1], NVIC_BASE->ISPR[2]);
//   Monitor::instance()->writeSerialMessage(F(message));
// }

const char * reinterpretCharPtr(const __FlashStringHelper *string)
{
  return reinterpret_cast<const char *>(string);
}

void blinkTest()
{
  debug(F("blink test:"));
  blink(10,250);
}

