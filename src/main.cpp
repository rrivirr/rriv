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

void testSwitchedPower();
void testVinMeasure();
void testExADCMOSFET();
void testEnableAVDD();
void ledTest();
void testIntADC();
void testExADC();


void workspace()
{
  // return;
    Serial2.println("tess");
    // while(1){};

  // ledTest();

  testSwitchedPower();
  // testVinMeasure();
  // testExADCMOSFET();
  // testEnableAVDD();
  // testIntADC();
  // testExADC();
}

#define RGB1 PA8
#define RGB2 PA9
#define RGB3 PA10

#include "system/measurement_components.h"

void testExADC(){
    enableI2C1();

  bool externalADCInstalled =  scanIC2(&Wire, 0x2f); // use datalogger setting once method is moved to instance method
  if (externalADCInstalled)
  {
      Serial2.println("reset exADC");
  // Reset external ADC (if it's installed)
  delay(1); // delay > 50ns before applying ADC reset
  digitalWrite(EXADC_RESET,LOW); // reset is active low
  delay(1); // delay > 10ns after starting ADC reset
  digitalWrite(EXADC_RESET,HIGH);
  delay(100); // Wait for ADC to start up

    Serial2.println(F("Set up extADC"));
    externalADC = new AD7091R();
    externalADC->configure();
    externalADC->enableChannel(0);
    externalADC->enableChannel(1);
    externalADC->enableChannel(2);
    externalADC->enableChannel(3);
  }
  else
  {
    Serial2.println(F("extADC not installed"));
  }
  while(1){
    externalADC->convertEnabledChannels();
    Serial2.println(externalADC->channel0Value());
    Serial2.println(externalADC->channel1Value());
    Serial2.println(externalADC->channel2Value());
    Serial2.println(externalADC->channel3Value());
    delay(1000);

  }
}

void testIntADC(){
  pinMode(ANALOG_INPUT_1_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_2_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_3_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_4_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_5_PIN, INPUT_ANALOG);
  while(1){
    Serial2.println(analogRead(ANALOG_INPUT_1_PIN));
    Serial2.println(analogRead(ANALOG_INPUT_2_PIN));
    Serial2.println(analogRead(ANALOG_INPUT_3_PIN));
    Serial2.println(analogRead(ANALOG_INPUT_4_PIN));
    Serial2.println(analogRead(ANALOG_INPUT_5_PIN));
    delay(4000);
  }
}

void ledTest() {
  pinMode(RGB1, OUTPUT_OPEN_DRAIN);
  pinMode(RGB2, OUTPUT_OPEN_DRAIN);
  pinMode(RGB3, OUTPUT_OPEN_DRAIN);

  int blue = 0;
  int red = 100;
  int green = 200;
  int factor = 2;
  while(1){
    // Serial2.println(blue);
    analogWrite(RGB1, red);
    analogWrite(RGB2, blue);
    analogWrite(RGB3, green);
    blue = (blue + 1*factor) % 256;
    red = (red + 2*factor) % 256;
    green = (green + 3*factor) % 256;
    delay(50);
  }
}

void testExADCMOSFET(){
  Serial2.println("test exADC MOSFET");
  pinMode(ENABLE_EX_ADC, OUTPUT_OPEN_DRAIN);
  pinMode(ENABLE_5V, OUTPUT);

  digitalWrite(ENABLE_5V, true);
    // digitalWrite(ENABLE_5V, true);
  Serial2.println("5V high");
  while(1){
      Serial2.println("5V high");
      delay(2000);
  }

  int delayTime = 10000;
  while(1){
  // digitalWrite(ENABLE_5V, false);
  // Serial2.println("5V low");
  // delay(delay  Time);


  // delay(delayTime);

  digitalWrite(ENABLE_EX_ADC, false);
  Serial2.println(false);
  delay(delayTime);
  // enableI2C1();
  
  digitalWrite(ENABLE_EX_ADC, true);
  Serial2.println(true);
  delay(delayTime);
  // enableI2C1();
  }
}

void testEnableAVDD() {
  Serial2.println("test avdd  ");
  pinMode(ENABLE_F103_AVDD, OUTPUT_OPEN_DRAIN);

  int delayTime = 10000;
  while(1){
  // digitalWrite(ENABLE_5V, false);
  // Serial2.println("5V low");
  // delay(delayTime);

  // digitalWrite(ENABLE_5V, true);
  // Serial2.println("5V high");
  // delay(delayTime);

  digitalWrite(ENABLE_F103_AVDD, false);
    Serial2.println(false);
  delay(delayTime);
  // enableI2C1();
  
  digitalWrite(ENABLE_F103_AVDD, true);
    Serial2.println(true);
  delay(delayTime);
  // enableI2C1();
  }
}

void testVinMeasure() {
  Serial2.println("test vin");
  pinMode(ENABLE_VIN_MEASURE, OUTPUT_OPEN_DRAIN);
  pinMode(PB0, INPUT_ANALOG);


  int delayTime = 10000;
  while(1){
  // digitalWrite(ENABLE_5V, false);
  // Serial2.println("5V low");
  // delay(delayTime);

  // digitalWrite(ENABLE_5V, true);
  // Serial2.println("5V high");
  // delay(delayTime);

  digitalWrite(ENABLE_VIN_MEASURE, false);
  Serial2.println(false);
  for(int i=0; i<10; i=i+1){
    Serial2.println(getBatteryValue());
    delay(1000);
  }

  
  digitalWrite(ENABLE_VIN_MEASURE, true);
  Serial2.println(true);
  for(int i=0; i<10; i=i+1){
    Serial2.println(getBatteryValue());
        delay(1000);

  }
  }

  bool enabled = false;
  while(1) {
    Serial2.println(true);
    digitalWrite(ENABLE_VIN_MEASURE, true);
    Serial2.println(getBatteryValue());
    delay(2000);
    
    Serial2.println(true);
    digitalWrite(ENABLE_VIN_MEASURE, true);
    Serial2.println(getBatteryValue());
    delay(2000);

    Serial2.println(true);
    digitalWrite(ENABLE_VIN_MEASURE, true);
    Serial2.println(getBatteryValue());
    delay(2000);

      Serial2.println(true);
    digitalWrite(ENABLE_VIN_MEASURE, true);
    Serial2.println(getBatteryValue());
    delay(2000);

      Serial2.println(true);
    digitalWrite(ENABLE_VIN_MEASURE, true);
    Serial2.println(getBatteryValue());
    delay(2000);

      Serial2.println(true);
    digitalWrite(ENABLE_VIN_MEASURE, true);
    Serial2.println(getBatteryValue());
    delay(2000);

    Serial2.println(false);
    digitalWrite(ENABLE_VIN_MEASURE, false);
    Serial2.println(getBatteryValue());
    delay(2000);

     Serial2.println(false);
    digitalWrite(ENABLE_VIN_MEASURE, false);
    Serial2.println(getBatteryValue());
    delay(2000);

     Serial2.println(false);
    digitalWrite(ENABLE_VIN_MEASURE, false);
    Serial2.println(getBatteryValue());
    delay(2000);

     Serial2.println(false);
    digitalWrite(ENABLE_VIN_MEASURE, false);
    Serial2.println(getBatteryValue());
    delay(2000);
  }
}

void testSwitchedPower() {

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
  // while(1){
  //   delay(1000);
  // }

while(1){
  delay(1000);
  Serial2.println("Set low");
  pinMode(SWITCHED_POWER_ENABLE, OUTPUT);
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);

  delay(1000);

  for(int i=0; i<6; i++){
    Serial2.println(i);
    delay(1000);
  }

  Serial2.println("Set high");
  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
      delay(1000);
  for(int i=0; i<6; i++){
    Serial2.println(i);
    delay(1000);
  }
  continue;

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