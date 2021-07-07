#include <Arduino.h>
#include <libmaple/libmaple.h>
#include <libmaple/pwr.h> // necessary?
#include <string.h>
#include <Ezo_i2c.h>

#include "datalogger.h"
#include "scratch/dbgmcu.h"
#include "system/watchdog.h"
#include "utilities/qos.h"

// Setup and Loop
Datalogger * datalogger;

void setup(void)
{
  startSerial2();

  startCustomWatchDog();

  // TODO: read datalogger settings struct from EEPROM
  /*
  datalogger_settings_type * dataloggerSettings = (datalogger_settings_type *) malloc(sizeof(datalogger_settings_type));
  dataloggerSettings->interval = 15;
  datalogger = new Datalogger(dataloggerSettings);
  datalogger->setup();
  */
  
  // disable unused components and hardware pins //
  componentsAlwaysOff();
  //hardwarePinsAlwaysOff(); // TODO are we turning off I2C pins still, which is wrong

  setupSwitchedPower();

  enableSwitchedPower();

  setupHardwarePins();

  //Serial2.println(atlasRGBSensor.get_name());
  // digitalWrite(PA4, LOW); // turn on the battery measurement

  //blinkTest();

  // Set up the internal RTC
  RCC_BASE->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC_BASE->APB1ENR |= RCC_APB1ENR_BKPEN;
  PWR_BASE->CR |= PWR_CR_DBP; // Disable backup domain write protection, so we can write
  
  // delay(20000);

  allocateMeasurementValuesMemory();

  setupManualWakeInterrupts();

  powerUpSwitchableComponents();
  delay(2000); // what is this delay for?

   // Don't respond to interrupts during setup
  disableManualWakeInterrupt();
  clearManualWakeInterrupt();

  // Clear the alarms so they don't go off during setup
  clearAllAlarms();

  initializeFilesystem();

  //initBLE();

  readUniqueId(uuid);

  setNotBursting(); // prevents bursting during first loop

  /* We're ready to go! */
  Monitor::instance()->writeDebugMessage(F("done with setup"));
  Serial2.flush();

/*
  setupSensors();
  Monitor::instance()->writeDebugMessage(F("done with sensor setup"));
  Serial2.flush();
*/

  print_debug_status();
}




/* main run loop order of operation: */
void loop(void)
{
  Serial2.println("STARTING LOOP");
  startCustomWatchDog();
  printWatchDogStatus();

  checkMemory();

/*
  datalogger->loop();
  return; // skip the old loop below
*/

  bool bursting = shouldContinueBursting();
  bool debugLoop = checkDebugLoop();
  bool awakeForUserInteraction = checkAwakeForUserInteraction(debugLoop);
  bool takeMeasurement = checkTakeMeasurement(bursting, awakeForUserInteraction);

  // Should we sleep until a measurement is triggered?
  bool awaitMeasurementTrigger = false;
  if (!bursting && !awakeForUserInteraction)
  {
    Monitor::instance()->writeDebugMessage(F("Not bursting or awake"));
    awaitMeasurementTrigger = true;
  }

  if (awaitMeasurementTrigger) // Go to sleep
  {
    stopAndAwaitTrigger();
    return; // Go to top of loop
  }

  if (WaterBear_Control::ready(Serial2))
  {
    handleControlCommand();
    return;
  }

  if (WaterBear_Control::ready(getBLE()))
  {
    Monitor::instance()->writeDebugMessage(F("BLE Input Ready"));
    awakeTime = timestamp(); // Push awake time forward
    WaterBear_Control::processControlCommands(getBLE());
    return;
  }

  if (takeMeasurement)
  {
    takeNewMeasurement();
    trackBurst(bursting);
    if (DEBUG_MEASUREMENTS)
    {
      monitorValues();
    }
  }

  if (configurationMode)
  {
    monitorConfiguration();
  }

  if (debugValuesMode)
  {
    if (burstCount == burstLength) // will cause loop() to continue until mode turned off
    {
      prepareForTriggeredMeasurement();
    }
    monitorValues();
  }

  if (tempCalMode)
  {
    monitorTemperature();
  }
}
