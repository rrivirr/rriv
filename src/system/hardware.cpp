#include "hardware.h"
#include "Arduino.h"
#include "configuration.h"
#include "system/monitor.h"
#include <libmaple/pwr.h> 

void startSerial2()
{
  // Start up Serial2
  // TODO: Need to do an if(Serial2) after an amount of time, just disable it
  Serial2.begin(SERIAL_BAUD);
  while (!Serial2)
  {
    delay(100);
  }
  Monitor::instance()->writeSerialMessage(F("Hello world: serial2"));
  Monitor::instance()->writeSerialMessage(F("Begin Setup"));
}

void setUpInternalRTC(){
  // Set up the internal RTC
  RCC_BASE->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC_BASE->APB1ENR |= RCC_APB1ENR_BKPEN;
  PWR_BASE->CR |= PWR_CR_DBP; // Disable backup domain write protection, so we can write
}