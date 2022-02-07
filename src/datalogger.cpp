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

#include "datalogger.h"
#include <Cmd.h>
#include "system/measurement_components.h"
#include "system/monitor.h"
#include "system/watchdog.h"
#include "system/command.h"
#include "sensors/sensor.h"
#include "sensors/sensor_map.h"
#include "sensors/sensor_types.h"
#include "utilities/i2c.h"
#include "utilities/qos.h"
#include "utilities/STM32-UID.h"
#include "scratch/dbgmcu.h"

// static method to read configuration from EEPROM
void Datalogger::readConfiguration(datalogger_settings_type *settings)
{
  byte *buffer = (byte *)malloc(sizeof(byte) * sizeof(datalogger_settings_type));
  for (unsigned short i = 0; i < sizeof(datalogger_settings_type); i++)
  {
    short address = EEPROM_DATALOGGER_CONFIGURATION_START + i;
    buffer[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
  }

  memcpy(settings, buffer, sizeof(datalogger_settings_type));

  // apply defaults
  if (settings->burstNumber == 0 || settings->burstNumber > 20)
  {
    settings->burstNumber = 1;
  }
  if (settings->interBurstDelay > 300)
  {
    settings->interBurstDelay = 0;
  }

  settings->debug_values = true;
}

Datalogger::Datalogger(datalogger_settings_type *settings)
{
  powerCycle = true;
  debug("creating datalogger");
  debug("got mode");
  debug(settings->mode);

  // defaults
  if (settings->interval < 1)
  {
    debug("Setting interval to 1 by default");
    settings->interval = 1;
  }

  memcpy(&this->settings, settings, sizeof(datalogger_settings_type));

  switch (settings->mode)
  {
  case 'i':
    changeMode(interactive);
    strcpy(loggingFolder, reinterpret_cast<const char *> F("INTERACTIVE"));
    break;
  case 'l':
    changeMode(logging);
    strcpy(loggingFolder, settings->siteName);
    break;
  default:
    changeMode(interactive);
    strcpy(loggingFolder, reinterpret_cast<const char *> F("NOT_DEPLOYED"));
    break;
  }
}

void Datalogger::setup()
{
  startCustomWatchDog();

  setupHardwarePins();
  setupSwitchedPower();
  powerUpSwitchableComponents();

  bool externalADCInstalled = scanIC2(&Wire, 0x2f);
  settings.externalADCEnabled = externalADCInstalled;

  setupManualWakeInterrupts();
  disableManualWakeInterrupt(); // don't respond to interrupt during setup
  clearManualWakeInterrupt();

  clearAllAlarms(); // don't respond to alarms during setup

  //initBLE();

  unsigned char uuid[UUID_LENGTH];
  readUniqueId(uuid);
  decodeUniqueId(uuid, uuidString, UUID_LENGTH);

  buildDriverSensorMap();
  loadSensorConfigurations();
  initializeFilesystem();
  setUpCLI();
}

void Datalogger::loop()
{

  if (inMode(deploy_on_trigger))
  {
    deploy(); // if deploy returns false here, the trigger setup has a fatal coding defect not detecting invalid conditions for deployment
    goto SLEEP;
    return;
  }

  if (inMode(logging))
  {
    if (powerCycle)
    {
      debug("Powercycle");
      bool deployed = deploy();
      if (!deployed)
      {
        // what should we do here?
        // if we are in the field and the manual power cycle got skipped, the battery will quickly drain
        // perhaps just shut down the unit?
        powerDownSwitchableComponents();
        while (1)
          ;
      }

      goto SLEEP;
    }

    if (shouldExitLoggingMode())
    {
      notify("Should exit logging mode");
      changeMode(interactive);
      return;
    }

    if (shouldContinueBursting())
    {
      measureSensorValues();
      writeMeasurementToLogFile();
    }
    else
    {
      completedBursts++;
      if (completedBursts < settings.burstNumber)
      {
        debug(F("do another burst"));
        debug(settings.burstNumber);
        debug(completedBursts);
        
        notify(F("Waiting for burst delay"));
        extendCustomWatchdog(settings.interBurstDelay*60); // convert minutes to seconds
        
        /*
        pauseCustomWatchDog();
        notify(F("Waiting for burst delay"));
        delay(settings.interBurstDelay * 1000 * 60); //convert minutes to milliseconds
        resumeCustomWatchDog();
        */

        initializeBurst();
        return;
      }

      // go to sleep
      fileSystemWriteCache->flushCache();
    SLEEP:
      stopAndAwaitTrigger();
      initializeMeasurementCycle();
      return;
    }
    return;
  }

  processCLI();

  // not currently used
  // TODO: do we want to cache dirty config to reduce writes to EEPROM?
  // if (configurationIsDirty())
  // {
  //   debug("Configuration is dirty");
  //   storeConfiguration();
  //   stopLogging();
  // }

  if (inMode(logging) || inMode(deploy_on_trigger))
  {
    // processCLI may have moved logger into a deployed mode
    goto SLEEP;
  }
  else if (inMode(interactive))
  {
    if (interactiveModeLogging)
    {
      if (timestamp() > lastInteractiveLogTime + 1)
      {
        // notify(F("interactive log"));
        measureSensorValues(false);
        if(interactiveModeLogging)
        {
          Serial2.print("\n");
          for (unsigned int i = 0; i < sensorCount; i++)
          {
            Serial2.print(drivers[i]->getCSVColumnNames());
            if (i < sensorCount - 1)
            {
              Serial2.print(F(","));
            }
            else
            {
              Serial2.println("");
            }
          }

          for (unsigned int i = 0; i < sensorCount; i++)
          {
            Serial2.print(drivers[i]->getDataString());
            if (i < sensorCount - 1)
            {
              Serial2.print(F(","));
            }
            else
            {
              Serial2.println("");
            }
          }
          Serial2.print("CMD >> ");
        }
        writeMeasurementToLogFile();
        lastInteractiveLogTime = timestamp();
      }
    }
  }
  else if (inMode(debugging))
  {
    measureSensorValues(false);
    writeMeasurementToLogFile();
    delay(5000); // this value could be configurable, also a step / read from CLI is possible
  }
  else
  {
    // invalid mode!
    notify(F("Invalid Mode!"));
    notify(mode);
    mode = interactive;
    delay(1000);
  }

  powerCycle = false;
}

void Datalogger::loadSensorConfigurations()
{

  // load sensor configurations from EEPROM and count them
  sensorCount = 0;
  generic_config *configs[EEPROM_TOTAL_SENSOR_SLOTS];
  for (int i = 0; i < EEPROM_TOTAL_SENSOR_SLOTS; i++)
  {
    notify("reading slot");
    generic_config *sensorConfig = (generic_config *)malloc(sizeof(generic_config));

    readSensorConfigurationFromEEPROM(i, sensorConfig);

    notify(sensorConfig->common.sensor_type);
    if (sensorConfig->common.sensor_type <= MAX_SENSOR_TYPE)
    {
      notify("found configured sensor");
      sensorCount++;
    }
    sensorConfig->common.slot = i;
    configs[i] = sensorConfig;
  }
  if (sensorCount == 0)
  {
    notify("no sensor configurations found");
  }
  notify("FREE MEM");
  printFreeMemory();

  // construct the drivers
  notify("construct drivers");
  drivers = (SensorDriver **)malloc(sizeof(SensorDriver *) * sensorCount);
  int j = 0;
  for (int i = 0; i < EEPROM_TOTAL_SENSOR_SLOTS; i++)
  {
    notify("FREE MEM");
    printFreeMemory();
    if (configs[i]->common.sensor_type > MAX_SENSOR_TYPE)
    {
      notify("no sensor");
      continue;
    }

    debug("getting driver for sensor type");
    debug(configs[i]->common.sensor_type);
    SensorDriver *driver = driverForSensorType(configs[i]->common.sensor_type);
    debug("got sensor driver");
    checkMemory();

    drivers[j] = driver;
    j++;

    if (driver->getProtocol() == i2c)
    {
      debug("got i2c sensor");
      ((I2CSensorDriver *)driver)->setWire(&WireTwo);
      debug("set wire");
    }
    debug("do setup");
    driver->setup();

    debug("configure sensor driver");
    driver->configure(*configs[i]); //pass configuration struct to the driver
    debug("configured sensor driver");
  }

  for (int i = 0; i < EEPROM_TOTAL_SENSOR_SLOTS; i++)
  {
    free(configs[i]);
  }

  // set up bookkeeping for dirty configurations
  // if (dirtyConfigurations != NULL)
  // {
  //   free(dirtyConfigurations);
  // }
  // dirtyConfigurations = (bool *)malloc(sizeof(bool) * (sensorCount + 1));
}

void Datalogger::reloadSensorConfigurations() // for dev & debug
{
  // calling this function does not deal with memory fragmentation
  // so it's not part of the main system, only for dev & debug
  notify("FREE MEM reload");
  printFreeMemory();
  // free sensor configs
  for(int i=0; i<sensorCount; i++)
  {
    delete(drivers[i]);
  }
  free(drivers);
  notify("FREE MEM reload");
  printFreeMemory();
  loadSensorConfigurations();
}

void Datalogger::startLogging()
{
  initializeMeasurementCycle();
  interactiveModeLogging = true;
}

void Datalogger::stopLogging()
{
  interactiveModeLogging = false;
}

bool Datalogger::shouldExitLoggingMode()
{
  if (Serial2.peek() != -1)
  {
    //attempt to process the command line
    for (int i = 0; i < 10; i++)
    {
      processCLI();
    }
    if (inMode(interactive))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return false;
}

bool Datalogger::shouldContinueBursting()
{
  for (int i = 0; i < sensorCount; i++)
  {
    notify(F("check sensor burst"));
    notify(i);
    if (!drivers[i]->burstCompleted())
    {
      return true;
    }
  }
  return false;
}

void Datalogger::initializeBurst()
{
  for (unsigned int i = 0; i < sensorCount; i++)
  {
    drivers[i]->initializeBurst();
  }
}

void Datalogger::initializeMeasurementCycle()
{
  notify(F("setting base time"));
  currentEpoch = timestamp();
  offsetMillis = millis();

  initializeBurst();

  completedBursts = 0;

  // extendCustomWatchdog(settings.startUpDelay*60);

  /*
  pauseCustomWatchDog();
  notify(F("Waiting for start up delay"));
  delay(settings.startUpDelay * 1000 * 60);
  resumeCustomWatchDog();
  */

  notify(F("Waiting for start up delay"));
  delay(settings.startUpDelay); // can be zero

  bool sensorsWarmedUp = false;
  while(sensorsWarmedUp == false)
  {
    sensorsWarmedUp = true;
    for (int i = 0; i < sensorCount; i++)
    {
      notify("check isWarmed");
      if (!drivers[i]->isWarmedUp())
      {
        // TODO: enhancement, ask the sensor driver if we should sleep MCU for a while
        // mcuSleep(drivers[i]->warmUpSleep());
        sensorsWarmedUp = false;
      }
    }
  }

}

void Datalogger::measureSensorValues(bool performingBurst)
{
  if (settings.externalADCEnabled)
  {
    // get readings from the external ADC
    debug("converting enabled channels call");
    externalADC->convertEnabledChannels();
    debug("converted enabled channels");
  }

  for (unsigned int i = 0; i < sensorCount; i++)
  {
    if (drivers[i]->takeMeasurement())
    {
      if (performingBurst)
      {
        drivers[i]->incrementBurst(); // burst bookkeeping
      }
    }
  }
}

void Datalogger::writeStatusFieldsToLogFile()
{
  debug(F("Write status fields"));

  // Fetch and Log time from DS3231 RTC as epoch and human readable timestamps
  uint32 currentMillis = millis();
  double currentTime = (double)currentEpoch + (((double)currentMillis - offsetMillis) / 1000);

  char currentTimeString[20];
  char humanTimeString[20];
  sprintf(currentTimeString, "%10.3f", currentTime);                  // convert double value into string
  t_t2ts(currentTime, currentMillis - offsetMillis, humanTimeString); // convert time_t value to human readable timestamp

  char buffer[100];

  fileSystemWriteCache->writeString(settings.siteName);
  fileSystemWriteCache->writeString((char *)",");
  sprintf(buffer, "%s-%s-%s", settings.siteName, settings.deploymentTimestamp, uuidString ); // deployment identifier
  fileSystemWriteCache->writeString(buffer);
  fileSystemWriteCache->writeString((char *)",");
  sprintf(buffer, "%ld,", settings.deploymentTimestamp);
  fileSystemWriteCache->writeString(buffer);
  fileSystemWriteCache->writeString(uuidString);
  fileSystemWriteCache->writeString((char *)",");
  fileSystemWriteCache->writeString(currentTimeString);
  fileSystemWriteCache->writeString((char *)",");
  fileSystemWriteCache->writeString(humanTimeString);
  fileSystemWriteCache->writeString((char *)",");
}

bool Datalogger::writeMeasurementToLogFile()
{
  writeStatusFieldsToLogFile();

  // write out the raw battery reading
  int batteryValue = analogRead(PB0);
  char buffer[100];
  sprintf(buffer, "%d,", batteryValue);
  fileSystemWriteCache->writeString(buffer);

  // and write out the sensor data
  debug(F("Write out sensor data"));
  for (unsigned int i = 0; i < sensorCount; i++)
  {
    // get values from the sensor
    char *dataString = drivers[i]->getDataString();
    fileSystemWriteCache->writeString(dataString);
    if (i < sensorCount - 1)
    {
      fileSystemWriteCache->writeString((char *)reinterpretCharPtr(F(",")));
    }
  }
  sprintf(buffer, ",%s,", userNote);
  fileSystemWriteCache->writeString(buffer);
  if (userValue != INT_MIN)
  {
    sprintf(buffer, "%d", userValue);
    fileSystemWriteCache->writeString(buffer);
  }
  fileSystemWriteCache->endOfLine();
  return true;
}

void Datalogger::setUpCLI()
{
  cli = CommandInterface::create(Serial2, this);
  cli->setup();
}

void Datalogger::processCLI()
{
  cli->poll();
}

// not currently used
// bool Datalogger::configurationIsDirty()
// {
//   for(int i=0; i<sensorCount+1; i++)
//   {
//     if(dirtyConfigurations[i])
//     {
//       return true;
//     }
//   }

//   return false;
// }

// not curretnly used
// void Datalogger::storeConfiguration()
// {
//   for(int i=0; i<sensorCount+1; i++)
//   {
//     if(dirtyConfigurations[i]){
//       //store this config block to EEPROM
//     }
//   }
// }

void Datalogger::getConfiguration(datalogger_settings_type *dataloggerSettings)
{
  memcpy(dataloggerSettings, &settings, sizeof(datalogger_settings_type));
}

void Datalogger::setSensorConfiguration(char *type, cJSON *json)
{

  SensorDriver *driver = NULL;
  notify("get driver");
  driver = driverForSensorTypeString(type);
  notify("got driver");

  if (driver != NULL)
  {
    notify("configure from json");
    driver->configureFromJSON(json);
    notify("configured from json");
    if (driver->getProtocol() == i2c)
    {
      notify("got i2c sensor");
      ((I2CSensorDriver *)driver)->setWire(&WireTwo);
      notify("set wire");
    }
    notify("do setup");
    driver->setup();
    notify("done setup");
    generic_config configuration = driver->getConfiguration();
    storeSensorConfiguration(&configuration);

    notify(F("updating slots"));
    bool slotReplacement = false;
    notify(sensorCount);
    for (int i = 0; i < sensorCount; i++)
    {
      if (drivers[i]->getConfiguration().common.slot == driver->getConfiguration().common.slot)
      {
        slotReplacement = true;
        notify(F("slot replacement"));
        notify(i);
        SensorDriver *replacedDriver = drivers[i];
        drivers[i] = driver;
        notify("deleting");
        delete (replacedDriver);
        notify(F("OK"));
        break;
      }
    }
    if (!slotReplacement)
    {
      sensorCount = sensorCount + 1;
      SensorDriver **updatedDrivers = (SensorDriver **)malloc(sizeof(SensorDriver *) * sensorCount);
      int i = 0;
      // printFreeMemory();
      // if(sensorCount > 1)
      // {
      //   notify(drivers[i]->getConfiguration().common.slot);
      //   notify(driver->getConfiguration().common.slot);
      // }
      for (; i < sensorCount-1 && drivers[i]->getConfiguration().common.slot < driver->getConfiguration().common.slot ; i++)
      {
        updatedDrivers[i] = drivers[i];
      }

      updatedDrivers[i] = driver;
      i++;
      for (; i < sensorCount; i++)
      {
        updatedDrivers[i] = drivers[i-1];
      }
      free(drivers);
      drivers = updatedDrivers;
    }
    notify(F("OK"));
  }
}

void Datalogger::clearSlot(unsigned short slot)
{
  bool slotConfigured = false;
  for (int i = 0; i < sensorCount; i++)
  {
    if (drivers[i]->getConfiguration().common.slot == slot)
    {
      slotConfigured = true;
      break;
    }
  }
  if (!slotConfigured)
  {
    notify("Slot not configured");
    return;
  }

  byte empty[SENSOR_CONFIGURATION_SIZE];
  for (int i = 0; i < SENSOR_CONFIGURATION_SIZE; i++)
  {
    empty[i] = 0xFF;
  }
  writeSensorConfigurationToEEPROM(slot, empty);
  sensorCount--;

  SensorDriver **updatedDrivers = (SensorDriver **)malloc(sizeof(SensorDriver *) * sensorCount);
  int j = 0;
  for (int i = 0; i < sensorCount + 1; i++)
  {
    generic_config configuration = this->drivers[i]->getConfiguration();
    if (configuration.common.slot != slot)
    {
      updatedDrivers[j] = this->drivers[i];
      j++;
    } 
    else 
    {
      delete(this->drivers[i]);
    }
  }
  free(this->drivers);
  this->drivers = updatedDrivers;
}

cJSON *Datalogger::getSensorConfiguration(short index) // returns unprotected **
{
  return drivers[index]->getConfigurationJSON();
}

void Datalogger::setInterval(int interval)
{
  settings.interval = interval;
  storeDataloggerConfiguration();
}

void Datalogger::setBurstNumber(int number)
{
  settings.burstNumber = number;
  storeDataloggerConfiguration();
}

void Datalogger::setStartUpDelay(int delay)
{
  settings.startUpDelay = delay;
  storeDataloggerConfiguration();
}

void Datalogger::setIntraBurstDelay(int delay)
{
  settings.interBurstDelay = delay;
  storeDataloggerConfiguration();
}

void Datalogger::setExternalADCEnabled(bool enabled)
{
  settings.externalADCEnabled = enabled;
}

void Datalogger::setUserNote(char *note)
{
  strcpy(userNote, note);
}

void Datalogger::setUserValue(int value)
{
  userValue = value;
}

void Datalogger::toggleTraceValues()
{
  settings.debug_values = !settings.debug_values;
  storeConfiguration();
  Serial2.println(bool(settings.debug_values));
}

SensorDriver *Datalogger::getDriver(unsigned short slot)
{
  for (int i = 0; i < sensorCount; i++)
  {
    if (this->drivers[i]->getConfiguration().common.slot == slot)
    {
      return this->drivers[i];
    }
  }
  return NULL;
}

void Datalogger::calibrate(unsigned short slot, char *subcommand, int arg_cnt, char **args)
{
  SensorDriver *driver = getDriver(slot);
  if (strcmp(subcommand, "init") == 0)
  {
    driver->initCalibration();
  }
  else
  {
    notify(args[0]);
    driver->calibrationStep(subcommand, atoi(args[0]));
  }
}

void Datalogger::storeMode(mode_type mode)
{
  char modeStorage = 'i';
  switch (mode)
  {
  case logging:
    modeStorage = 'l';
    break;
  case deploy_on_trigger:
    modeStorage = 't';
    break;
  default:
    modeStorage = 'i';
    break;
  }
  settings.mode = modeStorage;
  storeDataloggerConfiguration();
}

void Datalogger::changeMode(mode_type mode)
{
  char message[50];
  sprintf(message, reinterpret_cast<const char *> F("Moving to mode %d"), mode);
  notify(message);
  this->mode = mode;
}

bool Datalogger::inMode(mode_type mode)
{
  return this->mode == mode;
}

bool Datalogger::deploy()
{
  notify(F("Deploying now!"));
  notifyDebugStatus();
  if (checkDebugSystemDisabled() == false)
  {
    notify("**** ABORTING DEPLOYMENT *****");
    notify("**** PLEASE POWER CYCLE THIS UNIT AND TRY AGAIN *****");
    return false;
  }

  setDeploymentTimestamp(timestamp());  // TODO: deployment should span across power cycles
  strcpy(loggingFolder, settings.siteName);
  fileSystem->closeFileSystem();
  initializeFilesystem();
  changeMode(logging);
  storeMode(logging);
  powerCycle = false; // not a powercycle loop
  return true;
}

void Datalogger::initializeFilesystem()
{
  SdFile::dateTimeCallback(dateTime);

  fileSystem = new WaterBear_FileSystem(loggingFolder, SD_ENABLE_PIN);
  Monitor::instance()->filesystem = fileSystem;
  debug(F("Filesystem started OK"));

  time_t setupTime = timestamp();
  char setupTS[21];
  sprintf(setupTS, "unixtime: %lld", setupTime);
  notify(setupTS);

  char header[200];
  const char *statusFields = "site,deployment,deployed_at,uuid,time.s,time.h,battery.V";
  strcpy(header, statusFields);
  debug(header);
  for (int i = 0; i < sensorCount; i++)
  {
    debug(i);
    debug(drivers[i]->getCSVColumnNames());
    strcat(header, ",");
    strcat(header, drivers[i]->getCSVColumnNames());
  }
  strcat(header, ",user_note,user_value");

  fileSystem->setNewDataFile(setupTime, header); // name file via epoch timestamps

  if (fileSystemWriteCache != NULL)
  {
    delete (fileSystemWriteCache);
  }
  debug("make a new write cache");
  fileSystemWriteCache = new WriteCache(fileSystem);
}

void Datalogger::powerUpSwitchableComponents()
{
  cycleSwitchablePower();

  // turn on 5v booster for exADC reference voltage, needs the delay
  // might be possible to turn off after exADC discovered, not certain.
  gpioPinOn(GPIO_PIN_4);
  
  delay(500);
  enableI2C1();
  enableI2C2();

  debug("resetting for exADC");
  // Reset external ADC (if it's installed)
  delay(1); // delay > 50ns before applying ADC reset
  digitalWrite(EXADC_RESET,LOW); // reset is active low
  delay(1); // delay > 10ns after starting ADC reset
  digitalWrite(EXADC_RESET,HIGH);
  delay(100); // Wait for ADC to start up

  bool externalADCInstalled = scanIC2(&Wire, 0x2f); // use datalogger setting once method is moved to instance method
  if (externalADCInstalled)
  {
    debug(F("Set up external ADC"));
    externalADC = new AD7091R();
    externalADC->configure();
    externalADC->enableChannel(0);
    externalADC->enableChannel(1);
    externalADC->enableChannel(2);
    externalADC->enableChannel(3);
  }
  else
  {
    debug(F("external ADC not installed"));
  }

  debug(F("Switchable components powered up"));
}

void Datalogger::powerDownSwitchableComponents() // called in stopAndAwaitTrigger
{
  //TODO: hook for sensors that need to be powered down?
  gpioPinOff(GPIO_PIN_3); //not in use currently
  gpioPinOff(GPIO_PIN_4); //turn off 5v booster
  i2c_disable(I2C2);
  debug(F("Switchable components powered down"));
}

void Datalogger::prepareForUserInteraction()
{
  char humanTime[26];
  time_t awakenedTime = timestamp();

  t_t2ts(awakenedTime, millis(), humanTime);
  debug(F("Awakened by user"));
  debug(F(humanTime));

  awakenedByUser = false;
  awakeTime = awakenedTime;
}

// bool Datalogger::checkAwakeForUserInteraction(bool debugLoop)
// {
//   // Are we awake for user interaction?
//   bool awakeForUserInteraction = false;
//   if (timestamp() < awakeTime + USER_WAKE_TIMEOUT)
//   {
//     debug(F("Awake for user interaction"));
//     awakeForUserInteraction = true;
//   }
//   else
//   {
//     if (!debugLoop)
//     {
//       debug(F("Not awake for user interaction"));
//     }
//   }
//   if (!awakeForUserInteraction)
//   {
//     awakeForUserInteraction = debugLoop;
//   }
//   return awakeForUserInteraction;
// }

// bool checkTakeMeasurement(bool bursting, bool awakeForUserInteraction)
// {
//   // See if we should send a measurement to an interactive user
//   // or take a bursting measurement
//   bool takeMeasurement = false;
//   if (bursting)
//   {
//     takeMeasurement = true;
//   }
//   else if (awakeForUserInteraction)
//   {
//     unsigned long currentMillis = millis();
//     unsigned int interactiveMeasurementDelay = 1000;
//     if (currentMillis - lastMillis >= interactiveMeasurementDelay)
//     {
//       lastMillis = currentMillis;
//       takeMeasurement = true;
//     }
//   }
//   return takeMeasurement;
// }

void Datalogger::stopAndAwaitTrigger()
{
  debug(F("Await measurement trigger"));

  if (Clock.checkIfAlarm(1))
  {
    debug(F("Alarm 1"));
  }

  // printInterruptStatus(Serial2);
  debug(F("Going to sleep"));

  // save enabled interrupts
  int iser1, iser2, iser3;
  storeAllInterrupts(iser1, iser2, iser3);

  clearManualWakeInterrupt();
  setNextAlarmInternalRTC(settings.interval);

  powerDownSwitchableComponents();
  fileSystem->closeFileSystem(); // close file, filesystem
  disableSwitchedPower();

  awakenedByUser = false; // Don't go into sleep mode with any interrupt state

  componentsStopMode();

  disableCustomWatchDog();
  debug(F("disabled watchdog"));
  disableSerialLog();     // TODO
  hardwarePinsStopMode(); // switch to input mode

  clearAllInterrupts();
  clearAllPendingInterrupts();

  enableManualWakeInterrupt();    // The button, which is not powered during stop mode on v0.2 hardware
  nvic_irq_enable(NVIC_RTCALARM); // enable our RTC alarm interrupt

  enterStopMode();
  //enterSleepMode();

  reenableAllInterrupts(iser1, iser2, iser3);
  disableManualWakeInterrupt();
  nvic_irq_disable(NVIC_RTCALARM);

  enableSerialLog();
  enableSwitchedPower();
  setupHardwarePins(); // used from setup steps in datalogger

  debug(F("Awakened by interrupt"));

  startCustomWatchDog(); // could go earlier once working reliably
  // delay( (WATCHDOG_TIMEOUT_SECONDS + 5) * 1000); // to test the watchdog

  if (awakenedByUser == true)
  {
    notify(F("USER TRIGGERED INTERRUPT"));
  }

  // We have woken from the interrupt
  // printInterruptStatus(Serial2);

  powerUpSwitchableComponents();
  // turn components back on
  componentsBurstMode();
  fileSystem->reopenFileSystem();

  if (awakenedByUser == true)
  {
    awakeTime = timestamp();
  }

  // We need to check on which interrupt was triggered
  if (awakenedByUser)
  {
    prepareForUserInteraction();
  }
}

void Datalogger::storeDataloggerConfiguration()
{
  writeDataloggerSettingsToEEPROM(&this->settings);
}

void Datalogger::storeSensorConfiguration(generic_config *configuration)
{
  notify(F("Storing sensor configuration"));
  // notify(configuration->common.slot);
  // notify(configuration->common.sensor_type);
  writeSensorConfigurationToEEPROM(configuration->common.slot, configuration);
}

void Datalogger::setSiteName(char *siteName)
{
  strcpy(this->settings.siteName, siteName);
  storeDataloggerConfiguration();
}

void Datalogger::setDeploymentTimestamp(int timestamp)
{
  this->settings.deploymentTimestamp = timestamp;
}

const char *Datalogger::getUUIDString()
{
  return uuidString;
}
