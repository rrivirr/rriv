#include <Arduino.h>
#include <libmaple/libmaple.h>
#include <libmaple/pwr.h> // necessary?
#include <string.h>

#include "datalogger.h"
#include "scratch/dbgmcu.h"
#include "system/watchdog.h"
#include "utilities/qos.h"

// Setup and Loop
Datalogger * datalogger;

// void setupSensors(){

//   // read sensors types from EEPROM
//   // malloc configuration structs
//   // read configuration structs from EEPROM for each sensor type
//   // run setup for each sensor

  
//   // Setup RGB Sensor
//   AtlasRGB::instance()->setup(&WireTwo);

// }

void copyBytesToRegister(byte * registerPtr, byte msb, byte lsb){
  memcpy( registerPtr + 1, &msb, 1);
  memcpy( registerPtr, &lsb, 1);
}


void setup(void)
{
  startSerial2();
  Monitor::instance()->debugToSerial=true;

  startCustomWatchDog();
  printWatchDogStatus();


  // disable unused components and hardware pins //
  componentsAlwaysOff();
  //hardwarePinsAlwaysOff(); // TODO are we turning off I2C pins still, which is wrong

  setupSwitchedPower();

  enableSwitchedPower();

  setupHardwarePins();

  //Monitor::instance()->writeDebugMessage(atlasRGBSensor.get_name());
  // digitalWrite(PA4, LOW); // turn on the battery measurement

  //blinkTest();

  // Set up the internal RTC
  RCC_BASE->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC_BASE->APB1ENR |= RCC_APB1ENR_BKPEN;
  PWR_BASE->CR |= PWR_CR_DBP; // Disable backup domain write protection, so we can write

  allocateMeasurementValuesMemory();

  setupManualWakeInterrupts();

  powerUpSwitchableComponents();
  debug("ok");

  // Don't respond to interrupts during setup
  disableManualWakeInterrupt();
  clearManualWakeInterrupt();

  // Clear the alarms so they don't go off during setup
  clearAllAlarms();
  debug("okok");

  // initializeFilesystem();

  //initBLE();

  readUniqueId(uuid);
  uuidString[2 * UUID_LENGTH] = '\0';
  for (short i = 0; i < UUID_LENGTH; i++)
  {
    sprintf(&uuidString[2 * i], "%02X", (byte)uuid[i]);
  }
  Serial2.println(uuidString);

  setNotBursting(); // prevents bursting during first loop

  // TODO: read datalogger settings struct from EEPROM
  datalogger_settings_type * dataloggerSettings = (datalogger_settings_type *) malloc(sizeof(datalogger_settings_type));
  dataloggerSettings->interval = 15;
  datalogger = new Datalogger(dataloggerSettings);
  Monitor::instance()->writeDebugMessage("created datalogger");
  datalogger->setup();
  
  /* We're ready to go! */
  Monitor::instance()->writeDebugMessage(F("done with setup"));

/*
  setupSensors();
  Monitor::instance()->writeDebugMessage(F("done with sensor setup"));
  Serial2.flush();
*/

  print_debug_status();

  disableCustomWatchDog();
  print_debug_status(); // delays for 10s with user message, don't want watchdog to trigger
  startCustomWatchDog();
  Monitor::instance()->debugToSerial=false;
  Monitor::instance()->writeSerialMessage("Entering main run loop");
  Monitor::instance()->writeSerialMessage("Press return to access CLI");

}



/* main run loop order of operation: */
void loop(void)
{
  startCustomWatchDog();
  printWatchDogStatus();
  checkMemory();

  datalogger->loop();
  return; // skip the old loop below

  // Get reading from RGB Sensor
  // char * data = AtlasRGB::instance()->mallocDataMemory();
  // AtlasRGB::instance()->takeMeasurement(data);
  // free(data);
 

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
  datalogger->processCLI();

  if (CommandInterface::ready(Serial2))
  {
    handleControlCommand();
    return;
  }

  if (CommandInterface::ready(getBLE()))
  {
    Monitor::instance()->writeDebugMessage(F("BLE Input Ready"));
    awakeTime = timestamp(); // Push awake time forward
    // WaterBear_Control::processControlCommands(getBLE(), );
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
