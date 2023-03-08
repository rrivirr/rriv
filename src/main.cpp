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

// Setup and Loop
Datalogger *datalogger;
void printWelcomeMessage(datalogger_settings_type *dataloggerSettings);
void workspace();

void setup(void)
{
  // startSerial2();
  // Monitor::instance()->debugToSerial = true;

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

  // debug("creating datalogger");
  datalogger_settings_type *dataloggerSettings = (datalogger_settings_type *)malloc(sizeof(datalogger_settings_type));
  Datalogger::readConfiguration(dataloggerSettings);
  datalogger = new Datalogger(dataloggerSettings);
  // debug("created datalogger");
  datalogger->setup();

  /* We're ready to go! */
  debug(F("done with setup"));
  notifyDebugStatus();

  startCustomWatchDog(); // printMCUDebugStatus delays with user message, don't want watchdog to trigger

  Monitor::instance()->debugToSerial = false;

  printWelcomeMessage(dataloggerSettings);

  if (datalogger->inMode(logging))
  {
    notify("Device will enter logging mode in 5 seconds");
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

   const __FlashStringHelper *welcomeMessage = F("Hi"); //F(R"RRIV(
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
  char humanTimeString[25];
  t_t2ts(timestamp(), 0, humanTimeString); // don't have an offset to count millis correctly
  sprintf(buffer, "Sys time: %i, %s", time, humanTimeString);
  notify(buffer);

  const __FlashStringHelper *handoff = F(R"RRIV(
Type 'help' for command list.
)RRIV");
  notify(handoff);
}

// space to work our development details


void workspace()
{
  // delay(5000);

  pinMode(PA0, INPUT); // PA0-WKUP/USART2_CTS/ADC12_IN0/TIM2_CH1_ETR
  pinMode(PA1, INPUT); // INPUT_ANALOG was suggested or INPUT_PULLDOWN?
  pinMode(PA2, INPUT);
  pinMode(PA3, INPUT);
  pinMode(PA4, INPUT);
  pinMode(PA5, INPUT);
  pinMode(PA6, INPUT);
  pinMode(PA7, INPUT);
  pinMode(PA8, INPUT);
  pinMode(PA9, INPUT);
  pinMode(PA10, INPUT);
  pinMode(PA11, INPUT);
  pinMode(PA12, INPUT);
  pinMode(PA13, INPUT);
  pinMode(PA14, INPUT);
  pinMode(PA15, INPUT);

  pinMode(PB0, INPUT); // PB0 ADC12_IN8/TIM3_CH3
  pinMode(PB1, INPUT);
  pinMode(PB2, INPUT);
  pinMode(PB3, INPUT);
  pinMode(PB4, INPUT);
  pinMode(PB5, INPUT);
  pinMode(PB6, INPUT);
  pinMode(PB7, INPUT);
  pinMode(PB8, INPUT);
  pinMode(PB9, INPUT);
  pinMode(PB10, INPUT);
  pinMode(PB11, INPUT);
  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);
  pinMode(PB14, INPUT);
  pinMode(PB15, INPUT);

 
  pinMode(PC0, INPUT);
  pinMode(PC1, INPUT);
  pinMode(PC2, INPUT);
  pinMode(PC3, INPUT);
  pinMode(PC4, INPUT);
  pinMode(PC5, INPUT); // external ADC reset
  pinMode(PC6, INPUT); // this is the switch power pin
  pinMode(PC7, INPUT);
  pinMode(PC8, INPUT);
  pinMode(PC9, INPUT);
  pinMode(PC10, INPUT);
  pinMode(PC11, INPUT);
  pinMode(PC12, INPUT);
  pinMode(PC13, INPUT);
  pinMode(PC14, INPUT);
  pinMode(PC15, INPUT);  
  
  // pinMode(PD0, INPUT);
  // pinMode(PD1, INPUT);


  // disable unused peripherals
  ADC1->regs->CR2 &= ~ADC_CR2_TSVREFE;  
  adc_disable(ADC1); 

  // disable 5V
  pinMode(PB12, OUTPUT);
  digitalWrite(PB12, LOW);


  Serial2.begin(SERIAL_BAUD);
  while (!Serial2)
  {
    delay(100);
  }
  Serial2.println("Serial 2 begin");

while(1){
      delay(1000);
  pinMode(SWITCHED_POWER_ENABLE, OUTPUT);
        delay(1000);

  Serial2.println("Set high");
      delay(1000);

  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
      delay(1000);

  Serial2.println(F("OK"));
      delay(1000);

  for(int i=0; i<5; i++){
    Serial2.println(i);
    delay(1000);
  }

  Serial2.println(F("Set low"));
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
  delay(1000);
  Serial2.println(F("OK"));
  for(int i=0; i<5; i++){
    Serial2.println(i);
    delay(1000);
  }

  Serial2.println("Set high again");
  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
  Serial2.println(F("OK again"));
  for(int i=0; i<10; i++){
    Serial2.println(i);
    delay(1000);
  }


  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
  delay(2000);
   Serial2.println(F("C"));

}
  // setupSwitchedPower();
  // cycleSwitchablePower();
  while(1){}
  return;
  // notify(sizeof(long long));
  // notify(sizeof(sone));
  // exit(0);
  setupSwitchedPower();
  enableSwitchedPower();
  pinMode(PA8, OUTPUT_OPEN_DRAIN);
  bool pin = HIGH;
  while(true){
    Serial2.println("tick");
    Serial2.println(pin);
    digitalWrite(PA8, pin);
    delay(5000);
    pin = !pin;
  }
}