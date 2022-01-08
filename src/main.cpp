#include <Arduino.h>
#include <string.h>

#include "datalogger.h"
#include "system/watchdog.h"
#include "system/hardware.h"
#include "utilities/i2c.h"
#include "utilities/qos.h"
#include "scratch/dbgmcu.h"

// Setup and Loop
Datalogger * datalogger;

void setup(void)
{
  startSerial2();
  Monitor::instance()->debugToSerial=true;

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
  datalogger_settings_type * dataloggerSettings = (datalogger_settings_type *) malloc(sizeof(datalogger_settings_type));
  Datalogger::readConfiguration(dataloggerSettings);
  datalogger = new Datalogger(dataloggerSettings);
  debug("created datalogger");
  datalogger->setup();
  
  /* We're ready to go! */
  debug(F("done with setup"));

  startCustomWatchDog(); // printDebugStatus delays with user message, don't want watchdog to trigger
  printDebugStatus(); 
  
  Monitor::instance()->debugToSerial=false;
  notify("Entering main run loop");
  notify("Press return to access CLI");
  int start = timestamp();
  int now = start;
  while(now < start + 10) // wait x seconds
  {
    startCustomWatchDog();
    datalogger->processCLI();
    now = timestamp();
  }
}

void loop(void)
{
  startCustomWatchDog();
  printWatchDogStatus();
  checkMemory();

  datalogger->loop();
}
