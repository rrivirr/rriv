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

#include "command.h"
#include <re.h>
#include <Cmd.h>
#include <libmaple/libmaple.h>
#include "version.h"
#include "system/clock.h"
#include "utilities/qos.h"

#define MAX_REQUEST_LENGTH 70 // serial commands

CommandInterface * commandInterface;

const __FlashStringHelper * conditions = F(R"RRIV(
RRIV - Open Source Environmental Data Logging Platform
Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
)RRIV");

CommandInterface * CommandInterface::create(HardwareSerial &port, Datalogger * datalogger)
{
  commandInterface = new CommandInterface(Serial2, datalogger);
  return commandInterface;
}


CommandInterface * CommandInterface::instance()
{
  return commandInterface;
}

CommandInterface::CommandInterface(HardwareSerial &port, Datalogger * datalogger)
{
  this->datalogger = datalogger;
  cmdInit(&port);
}


bool CommandInterface::ready(HardwareSerial &port)
{
  Stream *myStream = &port;
  return CommandInterface::ready(myStream);
}

// bool CommandInterface::ready(Adafruit_BluefruitLE_UART &ble)
// {
//   Stream *myStream = &ble;
//   return CommandInterface::ready(myStream);
// }

// bool CommandInterface::ready(Adafruit_BluefruitLE_SPI &ble)
// {
//   Stream *myStream = &ble;
//   return CommandInterface::ready(myStream);
// }

bool CommandInterface::ready(Stream * myStream)
{
  if( (CommandInterface::state == 0)
      || CommandInterface::state == 1)
  {
    return myStream->peek() != -1;
  }
  else
  {
    return false;
  }
}

int CommandInterface::processControlCommands(HardwareSerial &port, Datalogger * datalogger)
{
  Stream *myStream = &port;
  return CommandInterface::processControlCommands(myStream, datalogger);
}

// int CommandInterface::processControlCommands(Adafruit_BluefruitLE_UART &ble, Datalogger * datalogger)
// {
//   Stream *myStream = &ble;
//   return CommandInterface::processControlCommands(myStream, datalogger);
// }

// int CommandInterface::processControlCommands(Adafruit_BluefruitLE_SPI &ble, Datalogger * datalogger)
// {
//   Stream *myStream = &ble;
//   return CommandInterface::processControlCommands(myStream, datalogger);
// }

void * CommandInterface::getLastPayload()
{
  return lastCommandPayload;
}

void toggleTrace(int arg_cnt, char **args)
{
  Monitor::instance()->debugToSerial = !Monitor::instance()->debugToSerial;
}

void startLogging(int arg_cnt, char **args)
{
  CommandInterface::instance()->_startLogging();
}

void CommandInterface::_startLogging()
{
  this->datalogger->settings.debug_values = true;
  this->datalogger->startLogging();
  notify("OK");
}

void CommandInterface::_toggleDebug()
{
  this->datalogger->changeMode(debugging);
  notify("OK");
}

void printVersion(int arg_cnt, char **args)
{
  char message[100];
  sprintf(message, "Firmware Version: %s", WATERBEAR_FIRMWARE_VERSION);
  notify(message);
}

void invalidArgumentsMessage(const __FlashStringHelper * message)
{
  notify(F("Invalid arguments"));
  notify(message);
  return;
}

void setSiteName(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-site-name SITE_NAME"));
    return;
  }

  // use singleton to get back into OOP context
  char * siteName = args[1];
  CommandInterface::instance()->_setSiteName(siteName);
}

void CommandInterface::_setSiteName(char * siteName)
{
  this->datalogger->setSiteName(siteName);
  notify("OK");
}

void setInterval(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-interval INTERVAL_BETWEEN_MEASUREMENT_WAKE_MINUTES"));
    return;
  }

  // use singleton to get back into OOP context
  int number = atoi(args[1]);
  CommandInterface::instance()->_setInterval(number);
}

void CommandInterface::_setInterval(int size)
{
  this->datalogger->setInterval(size);
  notify("OK");
}

void setBurstSize(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-burst_size NUMBER_OF_MEASUREMENTS_IN_BURST"));
    return;
  }

  // use singleton to get back into OOP context
  int size = atoi(args[1]);
  CommandInterface::instance()->_setBurstSize(size);
}

void CommandInterface::_setBurstSize(int size)
{
  this->datalogger->setBurstSize(size);
  notify("OK");
}

void setBurstNumber(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-burst-number NUMBER_OF_BURSTS"));
    return;
  }

  // use singleton to get back into OOP context
  int number = atoi(args[1]);
  CommandInterface::instance()->_setBurstNumber(number);
}

void CommandInterface::_setBurstNumber(int number)
{
  this->datalogger->setBurstNumber(number);
  notify("OK");
}


void setStartUpDelay(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-startup-delay START_UP_DELAY_SECONDS"));
    return;
  }

  // use singleton to get back into OOP context
  int number = atoi(args[1]);
  CommandInterface::instance()->_setStartUpDelay(number);
}

void CommandInterface::_setStartUpDelay(int number)
{
  this->datalogger->setStartUpDelay(number);
  notify("OK");
}

void setBurstDelay(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-burst-day DELAY_BETWEEN_BURSTS_SECONDS"));
    return;
  }

  // use singleton to get back into OOP context
  int number = atoi(args[1]);
  CommandInterface::instance()->_setBurstDelay(number);
}

void CommandInterface::_setBurstDelay(int number)
{
  this->datalogger->setIntraBurstDelay(number);
  notify(F("OK"));
}


void printWarranty(int arg_cnt, char **args)
{
  notify(F("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION."));
}


void printConditions(int arg_cng, char **args)
{
  notify(conditions);
}


void getConfig(int arg_cnt, char **args)
{
  // notify(F("getting config"));
  // int free1 = freeMemory();
  CommandInterface::instance()->_getConfig();
  // notify(F("got config"));
  // int free2 = freeMemory();
  // notify(free1);
  // notify(free2);
  // notify(freeMemory());
}

#define BUFFER_SIZE 400
void CommandInterface::_getConfig()
{
  datalogger_settings_type dataloggerSettings = this->datalogger->settings;
  this->datalogger->getConfiguration(&dataloggerSettings);
 
  cJSON* json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, reinterpretCharPtr(F("device_uuid")), this->datalogger->getUUIDString());
  // cJSON_AddStringToObject(json, reinterpretCharPtr(F("device_name")), dataloggerSettings.deviceName);
  cJSON_AddStringToObject(json, reinterpretCharPtr(F("site_name")), dataloggerSettings.siteName);
  cJSON_AddNumberToObject(json, reinterpretCharPtr(F("interval")), dataloggerSettings.interval);
  cJSON_AddNumberToObject(json, reinterpretCharPtr(F("burst_number")), dataloggerSettings.burstNumber);
  cJSON_AddNumberToObject(json, reinterpretCharPtr(F("start_up_delay")), dataloggerSettings.startUpDelay);
  cJSON_AddNumberToObject(json, reinterpretCharPtr(F("burst_delay")), dataloggerSettings.interBurstDelay);

  char string[BUFFER_SIZE];
  cJSON_PrintPreallocated(json, string, BUFFER_SIZE, true);
  if (string == NULL)
  {
    fprintf(stderr, reinterpretCharPtr(F("Failed to print json.\n")));
    return;
  }
  notify(string);
  Serial2.flush();
  
  cJSON_Delete(json);
  

  cJSON ** sensorConfigurations = this->datalogger->getSensorConfigurations();
  for(int i=0; i<this->datalogger->sensorCount; i++)
  {
    cJSON_PrintPreallocated(sensorConfigurations[i], string, BUFFER_SIZE, true);
    if (string == NULL)
    {
      fprintf(stderr, reinterpretCharPtr(F("Failed to print json.\n")));
      return;
    }
    notify(string);
    Serial2.flush();

    cJSON_Delete(sensorConfigurations[i]);
  }
  free(sensorConfigurations);
}

void setConfig(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-config CONFIG_JSON"));
    return;
  }


  char * config = args[1];
  CommandInterface::instance()->_setConfig(config);
}

void CommandInterface::_setConfig(char * config)
{
  cJSON *json = cJSON_Parse(config);
  char * printString = cJSON_Print(json);
  notify(printString);

  cJSON_Delete(json);
}

void setSlotConfig(int arg_cnt, char **args)
{
  notify(F("set slot config"));
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-slot-config SLOT_CONFIG_JSON"));
    return;
  }


  char * config = args[1];
  CommandInterface::instance()->_setSlotConfig(config);
}

void CommandInterface::_setSlotConfig(char * config)
{
  debug(F("set slot config check JSON"));

  cJSON *json = cJSON_Parse(config);
  if(json == NULL){
    notify(F("Invalid JSON"));
    return;
  }
  debug(F("printing json"));

  const char * printString = cJSON_Print(json);
  notify(printString);
  delete(printString);

  const cJSON* slotJSON = cJSON_GetObjectItemCaseSensitive(json, "slot");
  const cJSON* typeJSON = cJSON_GetObjectItemCaseSensitive(json, "type");

  char type[30];

  if(slotJSON != NULL && cJSON_IsNumber(slotJSON)){
    short slot = slotJSON->valueint;
    if(slot > EEPROM_TOTAL_SENSOR_SLOTS)
    {
      notify(F("Invalid slot"));
    }
  } else {
    notify(F("Invalid slot"));
    return;
  }

  if (cJSON_IsString(typeJSON) && (typeJSON->valuestring != NULL))
  {
    strcpy(type, typeJSON->valuestring);
  } else {
    notify(F("Invalid type"));
    return;
  }

  this->datalogger->setSensorConfiguration(type, json);

  cJSON_Delete(json);
}

void clearSlot(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("clear-slot SLOT_NUMBER"));
    return;
  }

  // use singleton to get back into OOP context
  int number = atoi(args[1]);
  CommandInterface::instance()->_clearSlot(number);
}

void CommandInterface::_clearSlot(int number)
{
  this->datalogger->clearSlot(number);
  notify("OK");
}

void setRTC(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-rtc UNIX_EPOCH_TIMESTAMP"));
    return;
  }

  int timestamp = atoi(args[1]);
  setTime(timestamp);
  notify("OK");
}

void getRTC(int arg_cnt, char **args)
{
  int time = timestamp();
  char message[100];
  sprintf(message, "current timestamp: %i", time);
  notify(message);
}

void restart(int arg_cnt, char **args)
{
  nvic_sys_reset();
}

void deployNow(int arg_cnt, char **args)
{
  CommandInterface::instance()->_deployNow();
}

void CommandInterface::_deployNow()
{
  this->datalogger->deploy();
}


void switchToInteractiveMode(int arg_cnt, char **args)
{
  CommandInterface::instance()->_switchToInteractiveMode();
}

void CommandInterface::_switchToInteractiveMode()
{
  this->datalogger->changeMode(interactive);
  this->datalogger->storeMode(interactive);
}


void calibrate(int arg_cnt, char **args)
{
  notify(F("calibrate sensor"));
  if(arg_cnt < 3){
    invalidArgumentsMessage(F("calibrate SLOT init|{driver specific command}"));
    return;
  }


  int slot = atoi(args[1]);
  char * subcommand = args[2];
  CommandInterface::instance()->_calibrate(slot, subcommand, arg_cnt - 3, &args[3]);
}

void CommandInterface::_calibrate(int slot, char * subcommand, int arg_cnt, char ** args)
{
  this->datalogger->calibrate(slot, subcommand, arg_cnt, args);
}

void setUserNote(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-user-note NOTE"));
    return;
  }

  // use singleton to get back into OOP context
  CommandInterface::instance()->_setUserNote(args[1]);
}

void CommandInterface::_setUserNote(char * note)
{
  this->datalogger->setUserNote(note);
  notify("OK");
}

void setUserValue(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-user-note NOTE"));
    return;
  }

  // use singleton to get back into OOP context
  CommandInterface::instance()->_setUserValue(atoi(args[1]));
}

void CommandInterface::_setUserValue(int value)
{
  this->datalogger->setUserValue(value);
  notify("OK");
}

void checkMemory(int arg_cnt, char **args)
{
  printFreeMemory();
}

void doScanIC2(int arg_cnt, char**args)
{
  // scanIC2(&Wire);
  // scanIC2(&Wire2);
}

void go(int arg_cnt, char**args)
{
  CommandInterface::instance()->_go();
}

void CommandInterface::_go()
{
  this->datalogger->changeMode(logging);
}


void CommandInterface::setup(){
  cmdAdd("version", printVersion);
  cmdAdd("show-warranty", printWarranty);
  cmdAdd("show-conditions", printConditions);

  cmdAdd("get-config", getConfig);
  cmdAdd("set-config", setConfig);
  cmdAdd("set-slot-config", setSlotConfig);
  cmdAdd("clear-slot", clearSlot);

  cmdAdd("set-rtc", setRTC);
  cmdAdd("get-rtc", getRTC);

  cmdAdd("set-site-name", setSiteName);
  cmdAdd("set-interval", setInterval);
  cmdAdd("set-burst-number", setBurstNumber);
  cmdAdd("set-start-up-delay", setStartUpDelay);
  cmdAdd("set-burst-delay", setBurstDelay);

  cmdAdd("calibrate", calibrate);
  
  cmdAdd("set-user-note", setUserNote);
  cmdAdd("set-user-value", setUserValue);

  cmdAdd("trace", toggleTrace);
  cmdAdd("start-logging", startLogging);
  // cmdAdd("start-logging", toggleInteractiveLogging);

  // cmdAdd("debug", debugMode);
  cmdAdd("deploy-now", deployNow);
  cmdAdd("interactive", switchToInteractiveMode);
  cmdAdd("i", switchToInteractiveMode);

  // qos commands / debug commands
  cmdAdd("restart", restart);
  cmdAdd("check-memory", checkMemory);
  cmdAdd("scan-ic2", doScanIC2);
  cmdAdd("go", go);

}




void CommandInterface::poll()
{
  cmdPoll();
}



//    Old code for reference - download process
//     // else if(strncmp(request, ">WT_DOWNLOAD",12) == 0)
//     // {
//     //   // Flush the input, would be better to use a delimiter
//     //   // May not be necessary now
//     //   unsigned long now = millis ();
//     //   while (millis () - now < 1000)
//     //   myStream->read ();  // read and discard any input

//     //   if(request[20] == ':')
//     //   {
//     //     // we have a reference date
//     //     // TODO: create and pass a device info object
//     //     strncpy(lastDownloadDate, &request[21], 10);
//     //   }

//     //   myStream->print(">WT_READY<");
//     //   myStream->flush();

//     //   WaterBear_Control::state = 1;
//     //   return WT_CONTROL_NONE;
//     // }

//   else if(CommandInterface::state == 1)
//   {
//     char ack[7] = "";
//     myStream->readBytesUntil('<', ack, 7);
//     if(strcmp(ack, ">WT_OK") != 0)
//     {
//       char message[30] = "";
//       sprintf(message, "ERROR #%s#", ack);
//       myStream->print(message);

//       //Flush
//       unsigned long now = millis ();
//       while (millis () - now < 1000)
//       myStream->read ();  // read and discard any input

//       CommandInterface::state = 0;
//       return WT_CONTROL_NONE;
//     }

//     char lastFileNameSent[10];
//     //bool success = WaterBear_FileSystem::dumpLoggedDataToStream(myStream, &lastFileNameSent); //also needs lastDownloadDate
//     if(true)
//     {
//       // Send last download date to phone for book keeeping
//       char transferCompleteMessage[34] = ">WT_COMPLETE:0000000000<";
//       strncpy(&transferCompleteMessage[22], lastFileNameSent, 10); // Send timestamp of last file sent
//       myStream->write(transferCompleteMessage);
//       // TODO: create and pass a data file writer class
//       // setNewDataFile();
//     }
//     else
//     {
//       // There was some kind of error
//     }
//     CommandInterface::state = 0;
//   }
//   return WT_CONTROL_NONE;
// }