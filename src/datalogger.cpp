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
#include "sensors/sensor_map.h"
#include "sensors/drivers/registry.h"
#include "utilities/i2c.h"
#include "utilities/qos.h"
#include "utilities/STM32-UID.h"
#include "scratch/dbgmcu.h"
#include "system/logs.h"
#include "sensors/drivers/air_pump.h"
#include "sensors/drivers/generic_actuator.h"

// #include "sensors/drivers/air_pump.h"
// #include "sensors/drivers/generic_actuator.h"

void Datalogger::sleepMCU(uint32 milliseconds)
{
  if(milliseconds < 5)
  {
    delay(milliseconds);
    return;
  }

  setNextAlarmInternalRTCMilliseconds(milliseconds);

  int iser1, iser2, iser3;
  storeAllInterrupts(iser1, iser2, iser3);

  disableCustomWatchDog();
  disableSerialLog();

  clearAllInterrupts();
  clearAllPendingInterrupts();

  nvic_irq_enable(NVIC_RTCALARM); // enable our RTC alarm interrupt

  enterSleepMode();

  nvic_irq_disable(NVIC_RTCALARM);

  reenableAllInterrupts(iser1, iser2, iser3);

  offsetMillis -= milliseconds; // account for millisecond count while systick was off

  enableSerialLog();
  startCustomWatchDog();

  reloadCustomWatchdog();
}


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
  settings->log_raw_data = true;
}

Datalogger::Datalogger(datalogger_settings_type *settings)
{
  powerCycle = true;

  //defaults
  if (settings->interval < 1)
  {
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

  checkMemory();
  buildDriverSensorMap();
  debug("Built driver sensor map");
  loadSensorConfigurations();
  debug("Loaded sensor configurations");
  initializeFilesystem();
  setUpCLI();
}

/*
*
* return: continue processing readings cycle bool
*/
bool Datalogger::processReadingsCycle()
{

  measureSensorValues();
 
  if (settings.log_raw_data) // we are really talking about a burst summary
  {
    writeRawMeasurementToLogFile();
  }
  

  if (shouldContinueBursting())
  {
    // sleep for maximum time before next reading
    // ask all drivers for maximum time until next burst reading
    // ask all drivers for maximum time until next available reading
    // sleep for whichever is less

    notify(minMillisecondsUntilNextReading());
    sleepMCU(minMillisecondsUntilNextReading());
    return true;
  }

  // otherwise burse cycle completed,
  completedBursts++;

  // so output burst summary
  writeSummaryMeasurementToLogFile();

  if (completedBursts < settings.burstNumber)
  {
    // debug(F("do another burst"));

    if (settings.interBurstDelay > 0)
    {
      notify(F("burst delay"));
      int interBurstDelay = settings.interBurstDelay * 60; // convert to seconds
      // todo: we should sleep any sensors that can be slept without re-warming
      // this could be called 'standby' mode
      // placeSensorsInStandbyMode();
      sleepMCU(interBurstDelay * 1000); // convert seconds to milliseconds
      // wakeSensorsFromStandbyMode();
    }

    initializeBurst();
    return true;
  }

  return false;
}

void Datalogger::testMeasurementCycle()
{
  initializeMeasurementCycle();
  fileSystemWriteCache->setOutputToSerial(true); // another way to do this would be to set a special write cache
  while(processReadingsCycle()){
    fileSystemWriteCache->flushCache();
    outputLastMeasurement();
  }      
  notify(F("after while loop"));
  fileSystemWriteCache->flushCache();            // instead of using a boolean in this particular write cache
  fileSystemWriteCache->setOutputToSerial(false);// and then set it back to the original writecache here
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
    //AE actuate hook before measurement could go here

    if (powerCycle)
    {
      debug("Powercycle");
      bool deployed = enterFieldLoggingMode();
      if (!deployed)
      {
        // what should we do here?
        // if we are in the field and the manual power cycle got skipped, the battery will quickly drain
        // perhaps just shut down the unit?
        powerDownSwitchableComponents();
        while (1)
          ;
      }
      powerCycle = false; // handled powercycle loop
      goto SLEEP;
    }

    if (shouldExitLoggingMode())
    {
      notify("Should exit logging mode");
      changeMode(interactive);
      return;
    }

    if(processReadingsCycle() == true) // nothing changes if this true/false
    {
      return;
    }

    // otherwise go to sleep
    fileSystemWriteCache->flushCache();

    //AE actuator hook could go here: is this before or after measurement? 

  SLEEP:
    stopAndAwaitTrigger();
    initializeMeasurementCycle();
    return;
  }

  processCLI();

  storeSensorConfigurationIfNeedsSave();

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
          outputLastMeasurement();
          Serial2.print(F("CMD >> "));
        }
        writeRawMeasurementToLogFile();
        lastInteractiveLogTime = timestamp();
      }
    }
    
  }
  else if (inMode(debugging))
  {
    measureSensorValues(false);
    writeRawMeasurementToLogFile();
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
  configuration_bytes sensorConfigs[EEPROM_TOTAL_SENSOR_SLOTS];
  for (int i = 0; i < EEPROM_TOTAL_SENSOR_SLOTS; i++)
  {
    notify("reading slot");
    readSensorConfigurationFromEEPROM(i, &sensorConfigs[i]);

    common_sensor_driver_config * commonConfiguration = (common_sensor_driver_config *) &sensorConfigs[i].common;

    notify(commonConfiguration->sensor_type);
    if( sensorTypeCodeExists(commonConfiguration->sensor_type) )
    {
      notify("found configured sensor");
      sensorCount++;
    }
    commonConfiguration->slot = i;
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
    common_sensor_driver_config * commonConfiguration = (common_sensor_driver_config *) &sensorConfigs[i].common;

    if ( !sensorTypeCodeExists(commonConfiguration->sensor_type) )
    {
      notify("no sensor with that code");
      continue;
    }

    debug("getting driver for sensor type");
    debug(commonConfiguration->sensor_type);
    SensorDriver *driver = driverForSensorTypeCode(commonConfiguration->sensor_type);
    debug("got sensor driver");
    checkMemory();

    drivers[j] = driver;
    j++;

    if (driver->getProtocol() == i2c)
    {
      debug("got i2c sensor");
      ((I2CProtocolSensorDriver *)driver)->setWire(&WireTwo);
      debug("set wire");
    }
    debug("do setup");
    driver->setup();

    debug("configure sensor driver");
    driver->configureFromBytes(sensorConfigs[i]); //pass configuration struct to the driver
    debug("configured sensor driver");
  }

}

void Datalogger::reloadSensorConfigurations() // for dev & debug
{
  // calling this function does not deal with memory fragmentation
  // so it's not part of the main system, only for dev & debug
  notify("FREE MEM reload");
  printFreeMemory();
  // free sensor configs
  for(unsigned short i=0; i<sensorCount; i++)
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
  for (unsigned short i = 0; i < sensorCount; i++)
  {
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

// void delaySeconds(int seconds)
// {
//   delayMilliseconds(seconds * 60);
// }


void Datalogger::initializeMeasurementCycle()
{
  //AE actuate hook before measurement could go here

  // notify(F("setting base time"));
  currentEpoch = timestamp();
  offsetMillis = millis();

  initializeBurst();

  completedBursts = 0;

  if (settings.startUpDelay > 0)
  {
    // notify("current test");
    // disableCustomWatchDog();
    // delay(20000);
    // startCustomWatchDog();

    notify(F("wait for start up delay"));
    int startUpDelay = settings.startUpDelay*60; // convert to seconds and print
    // startUpDelay = 2;
    notify(startUpDelay);
    sleepMCU(startUpDelay * 1000); // convert seconds to milliseconds
    notify("sleep done");
  }

  bool sensorsWarmedUp = false;
  while(sensorsWarmedUp == false)
  {
    sensorsWarmedUp = true;
    for (unsigned short i = 0; i < sensorCount; i++)
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
  //AE actuator call could go here 
  

}

void Datalogger::measureSensorValues(bool performingBurst)
{
  notify(F("made it to measureSensorValue"));
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
  //AE temp for actuators 
// for (unsigned int i = 0; i < sensorCount; i++)
//   {
//     drivers[i]->stop();
//     delay(drivers[i]->millisecondsUntilNextRequestedReading);
//   }  


  //AE actuate hook after measurement could go here
}

void Datalogger::writeStatusFieldsToLogFile(const char * type)
{
  // debug(F("Write status fields"));

  fileSystemWriteCache->writeString(type);
  fileSystemWriteCache->writeString((char *)",");

  // Fetch and Log time from DS3231 RTC as epoch and human readable timestamps
  uint32 currentMillis = millis();

  double currentTime = (double) currentEpoch + ( (double) ( currentMillis - offsetMillis) ) / 1000;

  char currentTimeString[20];
  char humanTimeString[24]; // YYYY-MM-DD HH:MM:SS:sss
  sprintf(currentTimeString, "%10.3f", currentTime);                  // convert double value into string
  t_t2ts(currentTime, currentMillis - offsetMillis, humanTimeString); // convert time_t value to human readable timestamp

  fileSystemWriteCache->writeString(settings.siteName);
  fileSystemWriteCache->writeString((char *)",");
  fileSystemWriteCache->writeString(settings.loggerName);
  fileSystemWriteCache->writeString((char *)",");

  char buffer[100];
  if(settings.deploymentIdentifier[0] == 0xFF)
  {
    sprintf(buffer, "%s-%lu", uuidString, settings.deploymentTimestamp);
  }
  else
  {
    char deploymentIdentifier[16] = {0};
    strncpy(deploymentIdentifier, settings.deploymentIdentifier, 15);
    debug(deploymentIdentifier[0]);
    debug(deploymentIdentifier);
    debug(uuidString);
    debug(settings.deploymentTimestamp);
    sprintf(buffer, "%s-%s-%lu", deploymentIdentifier, uuidString, settings.deploymentTimestamp);
  }
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

  // write out the raw battery reading
  sprintf(buffer, "%d,", getBatteryValue());
  fileSystemWriteCache->writeString(buffer);
}

void Datalogger::writeUserFieldsToLogFile()
{
  char buffer[150];
  sprintf(buffer, ",%s,", userNote);
  fileSystemWriteCache->writeString(buffer);
  if (userValue != INT_MIN)
  {
    sprintf(buffer, "%d", userValue);
    fileSystemWriteCache->writeString(buffer);
  }
}

bool Datalogger::writeRawMeasurementToLogFile()
{
  writeStatusFieldsToLogFile("raw");

  // and write out the sensor data
  debug(F("Write sensor data"));
  for (unsigned short i = 0; i < sensorCount; i++)
  {
    // get values from the sensors
    const char *dataString = drivers[i]->getRawDataString();
    fileSystemWriteCache->writeString(dataString);
    if (i < sensorCount - 1)
    {
      fileSystemWriteCache->writeString((char *)reinterpretCharPtr(F(",")));
    }
  }

  writeUserFieldsToLogFile();
  fileSystemWriteCache->endOfLine();
  return true;
}

bool Datalogger::writeSummaryMeasurementToLogFile()
{
  writeStatusFieldsToLogFile("summary");

  // and write out the sensor data
  for (unsigned short i = 0; i < sensorCount; i++)
  {
    // get values from the sensors
    const char *dataString = drivers[i]->getSummaryDataString();
    fileSystemWriteCache->writeString(dataString);
    if (i < sensorCount - 1)
    {
      fileSystemWriteCache->writeString((char *)reinterpretCharPtr(F(",")));
    }
  }

  writeUserFieldsToLogFile();
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


void Datalogger::storeSensorConfigurationIfNeedsSave()
{
  for(unsigned short i=0; i<sensorCount; i++)
  {
    if(drivers[i]->getNeedsSave())
    {
      storeSensorConfiguration(drivers[i]);
    }

  }
}

void Datalogger::setConfiguration(cJSON *config)
{
  const cJSON* siteNameJSON = cJSON_GetObjectItemCaseSensitive(config, "siteName");
  if(siteNameJSON != NULL && cJSON_IsString(siteNameJSON) && strlen(siteNameJSON->valuestring) <= 7)
  {
    strcpy(settings.siteName, siteNameJSON->valuestring);
  } else {
    notify("Invalid site name");
  }

  const cJSON* loggerNameJSON = cJSON_GetObjectItemCaseSensitive(config, "loggerName");
  if(loggerNameJSON != NULL && cJSON_IsString(loggerNameJSON) && strlen(loggerNameJSON->valuestring) <= 7)
  {
    strcpy(settings.loggerName, loggerNameJSON->valuestring);
  } else {
    notify("Invalid logger name");
  }
  
  const cJSON* deploymentIdentifierJSON = cJSON_GetObjectItemCaseSensitive(config, "deploymentIdentifier");
  if(deploymentIdentifierJSON != NULL && cJSON_IsString(deploymentIdentifierJSON) && strlen(deploymentIdentifierJSON->valuestring) <= 15)
  {
    strcpy(settings.deploymentIdentifier, deploymentIdentifierJSON->valuestring);
  } else {
    notify("Invalid deployment identifier");
  }

  const cJSON * intervalJson = cJSON_GetObjectItemCaseSensitive(config, "interval");
  if(intervalJson != NULL && cJSON_IsNumber(intervalJson) && intervalJson->valueint > 0)
  {
    settings.interval = (byte) intervalJson->valueint;
  } else {
    notify("Invalid interval");
  }

  const cJSON * burstNumberJson = cJSON_GetObjectItemCaseSensitive(config, "burstNumber");
  if(burstNumberJson != NULL && cJSON_IsNumber(burstNumberJson) && burstNumberJson->valueint > 0)
  {
    settings.burstNumber = (byte) burstNumberJson->valueint;
  } else {
    notify("Invalid burst number");
  }

  const cJSON * startUpDelayJson = cJSON_GetObjectItemCaseSensitive(config, "startUpDelay");
  if(startUpDelayJson != NULL && cJSON_IsNumber(startUpDelayJson) && startUpDelayJson->valueint >= 0)
  {
    settings.startUpDelay = (byte) startUpDelayJson->valueint;
  } else {
    notify("Invalid start up delay");
  }

  const cJSON * interBurstDelayJson = cJSON_GetObjectItemCaseSensitive(config, "interBurstDelay");
  if(interBurstDelayJson != NULL && cJSON_IsNumber(interBurstDelayJson) && interBurstDelayJson->valueint >= 0)
  {
    settings.interBurstDelay = (byte) interBurstDelayJson->valueint;
  } else {
    notify("Invalid inter burst delay");
  }

  storeDataloggerConfiguration();
}

// TODO: can I modify this to return a JSON instead? It can then be used when writing the comments before the header in the csv
void Datalogger::getConfiguration(datalogger_settings_type *dataloggerSettings)
{
  memcpy(dataloggerSettings, &settings, sizeof(datalogger_settings_type));

  // cJSON* json = cJSON_CreateObject();
  // cJSON_AddStringToObject(json, reinterpretCharPtr(F("device_uuid")), this->datalogger->getUUIDString());
  // cJSON_AddStringToObject(json, reinterpretCharPtr(F("site_name")), dataloggerSettings.siteName);
  // cJSON_AddStringToObject(json, reinterpretCharPtr(F("logger_name")), dataloggerSettings.loggerName);
  // cJSON_AddStringToObject(json, reinterpretCharPtr(F("deployment_identifier")), dataloggerSettings.deploymentIdentifier);
  // cJSON_AddNumberToObject(json, reinterpretCharPtr(F("interval(min)")), dataloggerSettings.interval);
  // cJSON_AddNumberToObject(json, reinterpretCharPtr(F("burst_number")), dataloggerSettings.burstNumber);
  // cJSON_AddNumberToObject(json, reinterpretCharPtr(F("start_up_delay(min)")), dataloggerSettings.startUpDelay);
  // cJSON_AddNumberToObject(json, reinterpretCharPtr(F("burst_delay(min)")), dataloggerSettings.interBurstDelay);
  // return(json)
}

void Datalogger::setSensorConfiguration(char *type, cJSON *json)
{

  SensorDriver *driver = NULL;
  short typeCode = typeCodeForSensorTypeString(type);
  driver = driverForSensorTypeCode(typeCode);

  if (driver != NULL)
  {
    if (driver->configureFromJSON(json) == false)
    {
      return;
    }
    if (driver->getProtocol() == i2c)
    {
      ((I2CProtocolSensorDriver *)driver)->setWire(&WireTwo);
    }
    driver->setup();
    storeSensorConfiguration(driver);

    bool slotReplacement = false;
    for (unsigned short i = 0; i < sensorCount; i++)
    {
      if (drivers[i]->getCommonConfigurations()->slot == driver->getCommonConfigurations()->slot)
      {
        slotReplacement = true;
        SensorDriver *replacedDriver = drivers[i];
        drivers[i] = driver;
        delete (replacedDriver);
        break;
      }
    }
    if (!slotReplacement)
    {
      sensorCount = sensorCount + 1;
      SensorDriver **updatedDrivers = (SensorDriver **)malloc(sizeof(SensorDriver *) * sensorCount);
      unsigned short i = 0;
      // printFreeMemory();
      // if(sensorCount > 1)
      // {
      //   notify(drivers[i]->getCommonConfigurations()->slot);
      //   notify(driver->getCommonConfigurations()->slot);
      // }
      for (; i < sensorCount-1 && drivers[i]->getCommonConfigurations()->slot < driver->getCommonConfigurations()->slot ; i++)
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
  }
}

void Datalogger::clearSlot(unsigned short slot)
{
  bool slotConfigured = false;
  for (unsigned short i = 0; i < sensorCount; i++)
  {
    if (drivers[i]->getCommonConfigurations()->slot == slot)
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
  for (unsigned short i = 0; i < sensorCount + 1; i++)
  {
    if (this->drivers[i]->getCommonConfigurations()->slot != slot)
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
  for (unsigned short i = 0; i < sensorCount; i++)
  {
    if (this->drivers[i]->getCommonConfigurations()->slot == slot)
    {
      return this->drivers[i];
    }
  }
  return NULL;
}

void Datalogger::calibrate(unsigned short slot, char *subcommand, int arg_cnt, char **args)
{

  SensorDriver *driver = getDriver(slot);
  if(driver == NULL)
  {
    notify(F("No driver"));
    return;
  }

  if (strcmp(subcommand, "init") == 0)
  {
    notify("calling init");
    driver->initCalibration();
  }
  else
  {

    driver->calibrationStep(subcommand, arg_cnt, args);
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
  enterFieldLoggingMode();
  return true;
}

bool Datalogger::enterFieldLoggingMode()
{
  strcpy(loggingFolder, settings.siteName);
  fileSystem->closeFileSystem();
  initializeFilesystem();
  setSensorDebugModes(false);
  changeMode(logging);
  storeMode(logging);
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
  const char *statusFields = "type,site,logger,deployment,deployed_at,uuid,time.s,time.h,battery.V";
  strcpy(header, statusFields);
  debug(header);
  for (unsigned short i = 0; i < sensorCount; i++)
  {
    debug(i);
    debug(drivers[i]->getCSVColumnHeaders());
    strcat(header, ",");
    strcat(header, drivers[i]->getCSVColumnHeaders());
  }
  strcat(header, ",user_note,user_value");

  fileSystem->setNewDataFile(setupTime, header); // name file via epoch timestamps

  if (fileSystemWriteCache != NULL)
  {
    delete (fileSystemWriteCache);
  }
  fileSystemWriteCache = new WriteCache(fileSystem);
}

void Datalogger::powerUpSwitchableComponents()
{
  cycleSwitchablePower();

  // turn on 5v booster for exADC reference voltage, needs the delay
  // might be possible to turn off after exADC discovered, not certain.
  gpioPinOn(GPIO_PIN_3);
  
  delay(250);
  enableI2C1();
  enableI2C2();

  debug("reset exADC");
  // Reset external ADC (if it's installed)
  delay(1); // delay > 50ns before applying ADC reset
  digitalWrite(EXADC_RESET,LOW); // reset is active low
  delay(1); // delay > 10ns after starting ADC reset
  digitalWrite(EXADC_RESET,HIGH);
  delay(100); // Wait for ADC to start up

  bool externalADCInstalled = scanIC2(&Wire, 0x2f); // use datalogger setting once method is moved to instance method
  if (externalADCInstalled)
  {
    debug(F("Set up extADC"));
    externalADC = new AD7091R();
    externalADC->configure();
    externalADC->enableChannel(0);
    externalADC->enableChannel(1);
    externalADC->enableChannel(2);
    externalADC->enableChannel(3);
  }
  else
  {
    debug(F("extADC not installed"));
  }

  debug(F("Switchable components powered up"));
};

void Datalogger::powerDownSwitchableComponents() // called in stopAndAwaitTrigger
{
  //TODO: hook for sensors that need to be powered down? separate functions?
  //TODO: hook for actuators that need to be powered down?
  gpioPinOff(GPIO_PIN_3); //turn off 5v booster
  gpioPinOff(GPIO_PIN_6); //not in use currently
  i2c_disable(I2C2);
  digitalWrite(EXADC_RESET,LOW);
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

  // printInterruptStatus(Serial2);
  debug(F("Going to sleep"));

  // save enabled interrupts
  int iser1, iser2, iser3;
  storeAllInterrupts(iser1, iser2, iser3);

  clearManualWakeInterrupt();
  setNextAlarmInternalRTC(settings.interval);

  // power down sensors -> function?
  for (unsigned int i = 0; i < sensorCount; i++)
  {
    drivers[i]->stop();
  }

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

  reenableAllInterrupts(iser1, iser2, iser3);
  disableManualWakeInterrupt();
  nvic_irq_disable(NVIC_RTCALARM);

  enableSerialLog();
  enableSwitchedPower();

  // power up sensors -> function?
  for (unsigned int i = 0; i < sensorCount; i++)
  {
    drivers[i]->setup();
  }

  setupHardwarePins(); // used from setup steps in datalogger

  debug(F("Awoke"));

  startCustomWatchDog(); // could go earlier once working reliably
  // delay( (DEFAULT_WATCHDOG_TIMEOUT_SECONDS + 5) * 1000); // to test the watchdog

  if (awakenedByUser == true)
  {
    notify(F("User interrupt"));
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

void Datalogger::storeSensorConfiguration(SensorDriver * driver)
{
  const configuration_bytes configurationBytes = driver->getConfigurationBytes();
  writeSensorConfigurationToEEPROM(driver->getSlot(), &configurationBytes);
}

void Datalogger::setSiteName(char *siteName)
{
  strncpy(this->settings.siteName, siteName, 8);
  storeDataloggerConfiguration();
}

void Datalogger::setDeploymentIdentifier(char *deploymentIdentifier)
{
  strncpy(this->settings.deploymentIdentifier, deploymentIdentifier, 16);
  storeDataloggerConfiguration();
}

void Datalogger::setLoggerName(char *loggerName)
{
  strncpy(this->settings.loggerName, loggerName, 8);
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


unsigned int Datalogger::minMillisecondsUntilNextReading()
{
  unsigned int minimumDelayUntilNextRequestedReading = MAX_REQUESTED_READING_DELAY; 
  unsigned int maxDelayUntilNextAvailableReading = 0;
  for(int i=0; i<sensorCount; i++)
  {
    // retrieve the fastest time requested for sampling
    minimumDelayUntilNextRequestedReading = min(minimumDelayUntilNextRequestedReading, drivers[i]->millisecondsUntilNextRequestedReading());
    // retrieve the slowest response time for sampling
    maxDelayUntilNextAvailableReading = max(maxDelayUntilNextAvailableReading, drivers[i]->millisecondsUntilNextReadingAvailable());
  }

  // return max to prioritize sampling as soon as ALL sensors are ready to sample
  return max(minimumDelayUntilNextRequestedReading, maxDelayUntilNextAvailableReading);
  
  // return min to prioritize sampling at desired speed for ONE specific sensor
  // if (maxDelayUntilNextAvailableReading == 0) // meaning all sensors have no delay between readings available
  // {
  //   return minimumDelayUntilNextRequestedReading;
  // }
  // return min(minimumDelayUntilNextRequestedReading, maxDelayUntilNextAvailableReading);
}


void Datalogger::setSensorDebugModes(bool debug)
{
  for(unsigned short i=0; i<sensorCount; i++)
  {
    drivers[i]->setDebugMode(debug);
  }
}

void Datalogger::outputLastMeasurement()
{
  Serial2.print("\n");
  for (unsigned short i = 0; i < sensorCount; i++)
  {
    Serial2.print(drivers[i]->getCSVColumnHeaders());
    Serial2.print(i < sensorCount - 1 ? "," : "\n");
  }

  for (unsigned short i = 0; i < sensorCount; i++)
  {
    Serial2.print(drivers[i]->getRawDataString());
    Serial2.print(i < sensorCount - 1 ? "," : "\n");
  }
}
