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
#include <string.h>

#include "datalogger.h"
#include "system/watchdog.h"
#include "system/hardware.h"
#include "utilities/i2c.h"
#include "utilities/qos.h"
#include "scratch/dbgmcu.h"
#include "version.h"
#include "system/eeprom.h"
#include "system/logs.h"

#include "libmaple/usb.h"
#include <USBComposite.h>
USBCompositeSerial USBSerial;

// Setup and Loop
Datalogger *datalogger;
void printWelcomeMessage(datalogger_settings_type *dataloggerSettings);
void workspace();

void setup(void)
{
  startSerial2();
  Monitor::instance()->debugToSerial = true;

 
  workspace();

  startCustomWatchDog();
  printWatchDogStatus();

  // disable unused components and hardware pins
  componentsAlwaysOff();
  //hardwarePinsAlwaysOff(); // TODO are we turning off I2C pins still, which is wrong

  setupInternalRTC();

  // turn on switched power to read from EEPROM
  setupSwitchedPower();
  cycleSwitchablePower();
  enableI2C1();
  delay(500);

  debug("creating datalogger");
  datalogger_settings_type *dataloggerSettings = (datalogger_settings_type *)malloc(sizeof(datalogger_settings_type));
  Datalogger::readConfiguration(dataloggerSettings);
  datalogger = new Datalogger(dataloggerSettings);
  debug("created datalogger");
  datalogger->setup();

  /* We're ready to go! */
  debug(F("done with setup"));
  notifyDebugStatus();

  startCustomWatchDog(); // printMCUDebugStatus delays with user message, don't want watchdog to trigger

  Monitor::instance()->debugToSerial = false;

  printWelcomeMessage(dataloggerSettings);

  if (datalogger->inMode(logging))
  {
    // notify("Device will enter logging mode in 5 seconds");
    notify("Type 'i' to exit to interactive mode");
    Serial2.print("CMD >> ");
    int start = timestamp();
    int now = start;
    while (now < start + 5)
    {
      startCustomWatchDog();
      datalogger->processCLI();
      now = timestamp();
    }
  }
  else
  {
    Serial2.print("CMD >> ");
    Serial2.flush();
  }

}

void loop(void)
{
  startCustomWatchDog();
  printWatchDogStatus();
  checkMemory();

  datalogger->loop();
}

void printWelcomeMessage(datalogger_settings_type *dataloggerSettings)
{
  // Welcome message

   const __FlashStringHelper *welcomeMessage = F("Welcome"); //F(R"RRIV(
// ____/\\\\\\\\\________/\\\\\\\\\______/\\\\\\\\\\\__/\\\________/\\\_        
//  __/\\\///////\\\____/\\\///////\\\___\/////\\\///__\/\\\_______\/\\\_       
//   _\/\\\_____\/\\\___\/\\\_____\/\\\_______\/\\\_____\//\\\______/\\\__      
//    _\/\\\\\\\\\\\/____\/\\\\\\\\\\\/________\/\\\______\//\\\____/\\\___     
//     _\/\\\//////\\\____\/\\\//////\\\________\/\\\_______\//\\\__/\\\____    
//      _\/\\\____\//\\\___\/\\\____\//\\\_______\/\\\________\//\\\/\\\_____   
//       _\/\\\_____\//\\\__\/\\\_____\//\\\______\/\\\_________\//\\\\\______  
//        _\/\\\______\//\\\_\/\\\______\//\\\__/\\\\\\\\\\\______\//\\\_______ 
//         _\///________\///__\///________\///__\///////////________\///________


// River Restoration Intelligence and Verification  https://rriv.org
// Copyright (C) 2020  Zaven Arra  zaven.arra@gmail.com
// This program comes with ABSOLUTELY NO WARRANTY; for details type `show-warranty'.
// This is free software, and you are welcome to redistribute it
// under certain conditions; type `show-conditions' for details.
// )RRIV");

  // You are connected to Little Peep
  // Site name: TRAY
  // Hardware version: WB2.1
  // Software version: v1.3.2

  // The river is at the center.
  // Type 'help' for command list.
  // CMD >>
  // )RRIV";

  notify(welcomeMessage);
  char buffer[100];
  sprintf(buffer, "Site name: %s", dataloggerSettings->siteName);
  notify(buffer);
  sprintf(buffer, "Software version: %s", WATERBEAR_FIRMWARE_VERSION);
  notify(buffer);

  const __FlashStringHelper *handoff = F(R"RRIV(
Type 'help' for command list.
)RRIV");
  notify(handoff);
}

// space to work our development details


void workspace()
{
  Serial2.println(RCC_BASE->CFGR, BIN);
  Serial2.println(RCC_BASE->APB1ENR, BIN);
  // 1000 0010 0000 0000 0000 0111 //APB1ENR
  //
  // exit(0);

  Serial2.println("hello");

  // USBComposite.clear(); // clear any plugins previously registered
  // USBSerial.registerComponent(); 
  // USBComposite.begin();
  // while(!USBComposite)
  // {
  //   Serial2.println("waiting");
  //   delay(1000);
  // }

  // usb_resume_init();
  USBSerial.begin();
  while(!USBSerial)
  {
    Serial2.println("waiting");
    delay(1000);
  }
  Serial2.println("Begin USBSerial");
  
  while(1)
  {
    USBSerial.write("hello");
    delay(1000);
  }

  exit(0);
  //0000 0000 0001 1101 1000 0100 0000 1010
  // PLL is divided by 1.5 for USBCLK
  // PLLSRC is HSE
  // PLLMUL is 9
  // this gives a 48MHz USBCLK
  // so long as USE_HSI and cpu speed is turned off in build params


  //0000 0000 0011 1000 1000 0100 0000 1010
  //
  //0000 
  //0000 
  // 0 
  // 0 - PLL divided by 1.5 for USB
  // 1110 PLL x16
  // 0 HSE not divided
  // 0  HSI / 2 as PLL input
  //1000 0100 0000 10
  // 10 PLL is SYSCLK
  RCC_BASE->CFGR &= 0x1F; // use HSE as PLLSRC
  Serial2.println(RCC_BASE->CFGR, BIN);
  RCC_BASE->CFGR &= 0b01001111111111; //0100 is input clock x6 PLLMUL
  Serial2.println(RCC_BASE->CFGR, BIN);

  while(!rcc_is_clk_ready(RCC_CLK_PLL));
  rcc_switch_sysclk(RCC_CLKSRC_PLL);
  Serial2.println("done");

  exit(0);
}