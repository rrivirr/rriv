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

#include <Arduino.h>
#include <RTClock.h>
#include "monitor.h"
#include "system/logs.h"

bool awakenedByUser = false;
void clearManualWakeInterrupt()
{
  EXTI_BASE->PR = 0x00000080; // this clear the interrupt on exti line
  NVIC_BASE->ICPR[0] = 1 << NVIC_EXTI_9_5;
}

void disableManualWakeInterrupt()
{
  NVIC_BASE->ICER[0] = 1 << NVIC_EXTI_9_5;
}

void enableManualWakeInterrupt()
{
  NVIC_BASE->ISER[0] = 1 << NVIC_EXTI_9_5;
}

void handleManualWakeInterrupt()
{
  disableManualWakeInterrupt();
  clearManualWakeInterrupt();
  awakenedByUser = true;
}


void setupManualWakeInterrupts()
{

  // debug("setup manual wake int");

  // Set up interrupts
  awakenedByUser = false;

  exti_attach_interrupt(EXTI7, EXTI_PC, handleManualWakeInterrupt, EXTI_FALLING);

}



void enableRTCAlarmInterrupt(){
  debug("wait RTC finished");
  rtc_wait_finished();
  
  debug("setting up EXTI");
  *bb_perip(&EXTI_BASE->IMR, EXTI_RTC_ALARM_BIT) = 1;
	*bb_perip(&EXTI_BASE->RTSR, EXTI_RTC_ALARM_BIT) = 1;

  debug("setting up NVIC");
  nvic_irq_enable(NVIC_RTCALARM);  // ISER
}

void clearRTCAlarmInterrupt(){
  // this clear method looks pretty wack to me
  *bb_perip(&EXTI_BASE->PR, EXTI_RTC_ALARM_BIT) = 1; // this clears the interrupt on exti line
  *bb_perip(&NVIC_BASE->ICPR, EXTI_RTC_ALARM_BIT) = 1;// FIX: wrong bit define here
}

void disableRTCAlarmInterrup(){
  nvic_irq_disable(NVIC_RTCALARM); // ICER
}

// void enableUserInterrupt()
// {
//   NVIC_BASE->ISER[1] = 1 << (NVIC_EXTI_15_10 - 32);
// }

// void clearUserInterrupt()
// {
//   EXTI_BASE->PR = 0x00000400; // this clears the interrupt on exti line
//   NVIC_BASE->ICPR[1] = 1 << (NVIC_EXTI_15_10 - 32);
// }

// void disableUserInterrupt()
// {
//   NVIC_BASE->ICER[1] = 1 << (NVIC_EXTI_15_10 - 32); // it's on EXTI 10
// }



void clearAllInterrupts()
{
  // only enable the timer and user interrupts
  NVIC_BASE->ICER[0] = NVIC_BASE->ISER[0];
  NVIC_BASE->ICER[1] = NVIC_BASE->ISER[1];
  NVIC_BASE->ICER[2] = NVIC_BASE->ISER[2];
}

void clearAllPendingInterrupts()
{
  // clear any pending interrupts
  NVIC_BASE->ICPR[0] = NVIC_BASE->ISPR[0];
  NVIC_BASE->ICPR[1] = NVIC_BASE->ISPR[1];
  NVIC_BASE->ICPR[2] = NVIC_BASE->ISPR[2];
}

void storeAllInterrupts(int &iser1, int &iser2, int &iser3)
{
  iser1 = NVIC_BASE->ISER[0];
  iser2 = NVIC_BASE->ISER[1];
  iser3 = NVIC_BASE->ISER[2];
}

void reenableAllInterrupts(int iser1, int iser2, int iser3)
{
  NVIC_BASE->ISER[0] = iser1;
  NVIC_BASE->ISER[1] = iser2;
  NVIC_BASE->ISER[2] = iser3;
}