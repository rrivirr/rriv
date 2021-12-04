#include "command.h"
#include <re.h>
#include <Cmd.h>
#include <libmaple/libmaple.h>
#include "version.h"
#include "system/clock.h"
#include "utilities/qos.h"

#define MAX_REQUEST_LENGTH 70 // serial commands

CommandInterface * commandInterface;

CommandInterface * CommandInterface::create(HardwareSerial &port, Datalogger * datalogger)
{
  commandInterface = new CommandInterface(Serial2, datalogger);
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

bool CommandInterface::ready(Adafruit_BluefruitLE_UART &ble)
{
  Stream *myStream = &ble;
  return CommandInterface::ready(myStream);
}

bool CommandInterface::ready(Adafruit_BluefruitLE_SPI &ble)
{
  Stream *myStream = &ble;
  return CommandInterface::ready(myStream);
}

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

int CommandInterface::processControlCommands(Adafruit_BluefruitLE_UART &ble, Datalogger * datalogger)
{
  Stream *myStream = &ble;
  return CommandInterface::processControlCommands(myStream, datalogger);
}

int CommandInterface::processControlCommands(Adafruit_BluefruitLE_SPI &ble, Datalogger * datalogger)
{
  Stream *myStream = &ble;
  return CommandInterface::processControlCommands(myStream, datalogger);
}

void * CommandInterface::getLastPayload()
{
  return lastCommandPayload;
}

void toggleDebug(int arg_cnt, char **args)
{
  Monitor::instance()->debugToSerial = !Monitor::instance()->debugToSerial;
  // CommandInterface::instance()->_toggleDebug();
}

void CommandInterface::_toggleDebug()
{
  this->datalogger->changeMode(debugging);
  Serial2.println("OK");
}

void printVersion(int arg_cnt, char **args)
{
  char message[100];
  sprintf(message, "Firmware Version: %s", WATERBEAR_FIRMWARE_VERSION);
  Serial2.println(message);
}

void invalidArgumentsMessage(const __FlashStringHelper * message)
{
  Serial2.println(F("Invalid arguments"));
  Serial2.println(message);
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
  Serial2.println("OK");
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
  Serial2.println("OK");
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
  Serial2.println("OK");
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
  Serial2.println("OK");
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
  Serial2.println("OK");
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
  Serial2.println("OK");
}


void printWarranty(int arg_cnt, char **args)
{
  Serial2.println(F("THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION."));
}

void getConfig(int arg_cnt, char **args)
{
  CommandInterface::instance()->_getConfig();
}

void CommandInterface::_getConfig()
{
  datalogger_settings_type dataloggerSettings;
  this->datalogger->getConfiguration(&dataloggerSettings);
  
  cJSON* json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "site_name", dataloggerSettings.siteName);
  cJSON_AddNumberToObject(json, "interval", dataloggerSettings.interval);
  cJSON_AddNumberToObject(json, "burst_size", dataloggerSettings.burstLength);
  cJSON_AddNumberToObject(json, "burst_number", dataloggerSettings.burstCount);
  cJSON_AddNumberToObject(json, "start_up_delay", dataloggerSettings.startUpDelay);
  cJSON_AddNumberToObject(json, "burst_delay", dataloggerSettings.intraBurstDelay);

  char * string = cJSON_Print(json);
  if (string == NULL)
  {
    fprintf(stderr, "Failed to print json.\n");
  }
  Serial2.println(string);
  free(string);
  free(json);

  cJSON ** sensorConfigurations = this->datalogger->getSensorConfigurations();
  for(int i=0; i<this->datalogger->sensorCount; i++)
  {
    char * string = cJSON_Print(sensorConfigurations[i]);
    if (string == NULL)
    {
      fprintf(stderr, "Failed to print json.\n");
    }
    Serial2.println(string);
    free(sensorConfigurations[i]);
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
  Serial2.println(printString);

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
  Serial2.println(printString);
  delete(printString);

  const cJSON* slotJSON = cJSON_GetObjectItemCaseSensitive(json, "slot");
  const cJSON* typeJSON = cJSON_GetObjectItemCaseSensitive(json, "type");

  char type[30];

  if(slotJSON != NULL && cJSON_IsNumber(slotJSON)){
    short slot = slotJSON->valueint;
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

void setRTC(int arg_cnt, char **args)
{
  if(arg_cnt < 2){
    invalidArgumentsMessage(F("set-rtc UNIX_EPOCH_TIMESTAMP"));
    return;
  }

  int timestamp = atoi(args[1]);
  setTime(timestamp);
  Serial2.println("OK");
}

void getRTC(int arg_cnt, char **args)
{
  int time = timestamp();
  char message[100];
  sprintf(message, "current timestamp: %i", time);
  Serial2.println(message);
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
}

void CommandInterface::setup(){
  cmdAdd("version", printVersion);
  cmdAdd("show-warranty", printWarranty);
  cmdAdd("get-config", getConfig);
  cmdAdd("set-config", setConfig);
  cmdAdd("set-slot-config", setSlotConfig);

  cmdAdd("set-rtc", setRTC);
  cmdAdd("get-rtc", getRTC);
  cmdAdd("restart", restart);

  cmdAdd("set-site-name", setSiteName);
  cmdAdd("set-interval", setInterval);
  cmdAdd("set-burst-size", setBurstSize); // datalogger - deprecated
  cmdAdd("set-burst-number", setBurstNumber);
  cmdAdd("set-start-up-delay", setStartUpDelay);
  cmdAdd("set-burst-delay", setBurstDelay);

  cmdAdd("trace", toggleDebug);
  // cmdAdd("start-logging", toggleInteractiveLogging);

  // cmdAdd("debug", debugMode);
  cmdAdd("deploy-now", deployNow);
  cmdAdd("interactive", switchToInteractiveMode);

}




void CommandInterface::poll()
{
  cmdPoll();
}


int CommandInterface::processControlCommands(Stream * myStream, Datalogger * datalogger)
{
  char lastDownloadDate[15] = "NOTIMPLEMENTED"; // placeholder

  if(CommandInterface::state == 0)
  {
    if(lastCommandPayloadAllocated == true)
    {
      free(lastCommandPayload);
      lastCommandPayloadAllocated = false;
    }
    //awakeTime = WaterBear_Control::timestamp(); // Keep us awake once we are talking to the phone

    char request[MAX_REQUEST_LENGTH] = "";
    myStream->readBytesUntil('\r', request, MAX_REQUEST_LENGTH);
    myStream->write(">COMMAND RECIEVED:");
    myStream->write(&request[0]);
    myStream->write("<");
    myStream->flush();
    delay(100);

    int matchLength;
    SensorDriver * driver;

    if (!strncmp(request, "version", 7)) {

    }
    else if (!re_match("^set-config .*\\.json$", request, &matchLength)){
      char * fileName = strchr(request, ' ') + 1;
    }
    else if (!re_match("^get-config [0-9]$", request, &matchLength)){
      char * find = strchr(request, ' ') + 1;
      unsigned short slot = atoi(find);
      driver = datalogger->drivers[slot];
    }
    else if (!strncmp(request, "list", 4)) {

    }
    else if (!re_match("^calibrate [0-9] init$", request, &matchLength)){
      char * find = (strchr(request, ' ')) + 1;
      char * slotString;
      strncpy(slotString, find, 1);
      unsigned short slot = atoi(find);
      driver = datalogger->drivers[slot];
    }
    else if (!re_match("^calibrate [0-9] .*$", request, &matchLength)){
      char * find = (strchr(request, ' ')) + 1;
      char * slotString;
      strncpy(slotString, find, 1);
      unsigned short slot = atoi(find);
      driver = datalogger->drivers[slot];
    }
    else if (!re_match("^set-rtc .*$", request, &matchLength)){
      char * time = strchr(request, ' ') + 1;
    }
    else if (!strncmp(request, "get-rtc", 7)) {

    }
    else if (!re_match("^user-note .*$", request, &matchLength)){
      char * note = strchr(request, ' ') + 1;
    }
    else if (!re_match("^user-value [0-9]*$", request, &matchLength)){
      char * find = strchr(request, ' ') + 1;
      unsigned short value = atoi(find);
    }
    else if (!strncmp(request, "delete-config", 13)) {

    }
    else if (!re_match("^reset-slot [0-9]$", request, &matchLength)){
      char * find = strchr(request, ' ') + 1;
      unsigned short slot = atoi(find);
      driver = datalogger->drivers[slot];
    }
    else if (!re_match("^delete-slot [0-9]$", request, &matchLength)){
      char * find = strchr(request, ' ') + 1;
      unsigned short slot = atoi(find);
      driver = datalogger->drivers[slot];
    }
    else if (!strncmp(request, "deploy-now", 10)) {

    }
    else if (!strncmp(request, "deploy-on-trigger", 17)) {

    }
    else if (!strncmp(request, "deploy-at-time", 14)) {

    }
    else if (!strncmp(request, "debug", 5)) {
      Monitor::instance()->debugToSerial = !Monitor::instance()->debugToSerial;
    }
    else if (!strncmp(request, "interactive", 11)) {

    }
    else if (!strncmp(request, "start-logging", 13)) {

    }
    else if (!re_match("^set-site-name .*$", request, &matchLength)){
      char * siteName = strchr(request, ' ') + 1;
    }
    else if (!strncmp(request, "show-copyright", 14)) {

    }
    else if (!strncmp(request, "show-warranty", 13)) {

    }
    // if(strncmp(request, ">WT_OPEN", 19) == 0)
    // {
    //   // TODO:  Need to pass firmware version to control somehow
    //   /*
    //   myStream->write(">VERSION:");
    //   myStream->write(version);
    //   myStream->write("<");
    //   myStream->flush();
    //   delay(100);
    //   */

    //   char dateString[26];
    //   time_t timeNow = timestamp();

    //   t_t2ts(timeNow, millis(), dateString); // change to 'yyyy-mm-dd hh:mm:ss.sss'?
    //   myStream->print(">Datalogger Time: ");
    //   myStream->print(dateString);
    //   myStream->print("<");
    //   delay(100);

    //   myStream->write(">WT_IDENTIFY:");
    //   // TODO: create and pass a device info object
    //   myStream->print(F("NOTIMPLEMENTED"));
    //   for(int i=0; i<8; i++)
    //   {
    //   }
    //   myStream->write("<");
    //   myStream->flush();

    //   myStream->write(">WT_TIMESTAMP:");
    //   myStream->println(timestamp());
    //   myStream->write("<");
    //   myStream->flush();
    //   delay(100);
    // }
    // else if(strncmp(request, ">WT_DOWNLOAD",12) == 0)
    // {
    //   // Flush the input, would be better to use a delimiter
    //   // May not be necessary now
    //   unsigned long now = millis ();
    //   while (millis () - now < 1000)
    //   myStream->read ();  // read and discard any input

    //   if(request[20] == ':')
    //   {
    //     // we have a reference date
    //     // TODO: create and pass a device info object
    //     strncpy(lastDownloadDate, &request[21], 10);
    //   }

    //   myStream->print(">WT_READY<");
    //   myStream->flush();

    //   WaterBear_Control::state = 1;
    //   return WT_CONTROL_NONE;
    // }
    // else if(strncmp(request, ">WT_SET_RTC:", 12) == 0)
    // {
    //   myStream->println("GOT SET_RTC<"); //acknowledge command sent
    //   char UTCTime[11] = "0000000000"; // buffer for data to read
    //   strncpy(UTCTime, &request[12], 10); // read serial data into char string
    //   time_t value;
    //   int found = sscanf(&UTCTime[0], "%lld", &value); // turn char string into correct value type
    //   if(found == 1)
    //   {
    //     time_t * commandPayloadPointer = (time_t *) malloc(sizeof(time_t));
    //     *commandPayloadPointer = value;
    //     lastCommandPayloadAllocated = true;
    //     lastCommandPayload = commandPayloadPointer;
    //   }
    //   return WT_SET_RTC;
    //   // TODO: create and pass a data file writer class
    //   // setNewDataFile();
    // }
    // else if(strncmp(request, ">WT_DEPLOY:", 11) == 0)
    // {
    //   myStream->println("GOT WT_DEPLOY<");
    //   char * commandPayloadPointer = (char *) malloc(26);
    //   strncpy(commandPayloadPointer, &request[11], 26);
    //   commandPayloadPointer[26] ='\0';
    //   lastCommandPayloadAllocated = true;
    //   lastCommandPayload = commandPayloadPointer;
    //   return WT_DEPLOY;

    //   // TODO: create and pass a data file writer class
    //   // setNewDataFile();
    // }
    // else if(strncmp(request, ">WT_CONFIG:", 10) == 0)
    // { //flags:time, conduct, therm
    //   myStream->println(">CONFIG<");
    //   char * commandPayloadPointer = (char *)malloc(8);
    //   strncpy(commandPayloadPointer, &request[11],8);
    //   commandPayloadPointer[8] ='\0';
    //   lastCommandPayloadAllocated = true;
    //   lastCommandPayload = commandPayloadPointer;
    //   return WT_CONTROL_CONFIG;
    //   // go into config mode
    // }
    // else if(strncmp(request, ">WT_DEBUG_VALUES", 16) == 0)
    // {
    //   myStream->println(">DEBUG_VALUES<");
    //   return WT_DEBUG_VAlUES;
    // }
    // else if(strncmp(request, ">WT_CLEAR_MODES", 15) == 0)
    // {
    //   myStream->println(">CLEAR_MODES<");
    //   return WT_CLEAR_MODES;
    // }
    // else if(strncmp(request, ">CAL_DRY", 8) == 0)
    // {
    //   myStream->println(">GOT CAL_DRY<");
    //   return WT_CONTROL_CAL_DRY;
    // }
    // else if(strncmp(request, ">CAL_LOW:", 9) == 0)
    // {
    //   myStream->println(">GOT CAL_LOW<");
    //   char calibrationPointStringValue[10];
    //   strncpy(calibrationPointStringValue, &request[9], 9);
    //   int value;
    //   int found = sscanf(&calibrationPointStringValue[0], "%d", &value);
    //   if(found == 1)
    //   {
    //     int * commandPayloadPointer = (int *) malloc(sizeof(int));
    //     *commandPayloadPointer = value;
    //     lastCommandPayloadAllocated = true;
    //     lastCommandPayload = commandPayloadPointer;
    //   }
    //   return WT_CONTROL_CAL_LOW;
    // }
    // else if(strncmp(request, ">CAL_HIGH:", 10) == 0)
    // {
    //   myStream->println(">GOT CAL_HIGH<");
    //   char calibrationPointStringValue[10];
    //   strncpy(calibrationPointStringValue, &request[10], 9);
    //   int value;
    //   myStream->println(calibrationPointStringValue); // what is this line for???
    //   int found = sscanf(&calibrationPointStringValue[0], "%d", &value);
    //   if(found == 1)
    //   {
    //     int * commandPayloadPointer = (int *) malloc(sizeof(int));
    //     *commandPayloadPointer = value;
    //     lastCommandPayloadAllocated = true;
    //     lastCommandPayload = commandPayloadPointer;
    //   }
    //   return WT_CONTROL_CAL_HIGH;
    // }
    // else if(strncmp(request, ">WT_CAL_TEMP", 12) == 0)
    // {
    //   myStream->println(">CAL_TEMP<");
    //   return WT_CAL_TEMP;
    // }
    // else if(strncmp(request, ">TEMP_CAL_LOW:", 14) == 0)
    // {
    //   myStream->println(">GOT TEMP_CAL_LOW<");
    //   char calibrationPointStringValue[8];
    //   strncpy(calibrationPointStringValue, &request[14], 7);
    //   float value;
    //   myStream->println(calibrationPointStringValue);
    //   int found = sscanf(&calibrationPointStringValue[0], "%f", &value);
    //   if(found == 1){
    //     value = value * 100;
    //     unsigned short * commandPayloadPointer = (unsigned short *) malloc(sizeof(unsigned short));
    //     *commandPayloadPointer = (unsigned short)value;
    //     lastCommandPayloadAllocated = true;
    //     lastCommandPayload = commandPayloadPointer;
    //   }
    //   return WT_TEMP_CAL_LOW;
    // }
    // else if(strncmp(request, ">TEMP_CAL_HIGH:", 15) == 0)
    // {
    //   myStream->println(">GOT TEMP_CAL_HIGH<");
    //   char calibrationPointStringValue[8];
    //   strncpy(calibrationPointStringValue, &request[15], 7);
    //   float value;
    //   myStream->println(calibrationPointStringValue);
    //   int found = sscanf(&calibrationPointStringValue[0], "%f", &value); // xxx.xx
    //   if(found == 1){
    //     value = value * 100; // either here or at the case
    //     unsigned short * commandPayloadPointer = (unsigned short *) malloc(sizeof(unsigned short));
    //     *commandPayloadPointer = (unsigned short)value;
    //     lastCommandPayloadAllocated = true;
    //     lastCommandPayload = commandPayloadPointer;
    //   }
    //   return WT_TEMP_CAL_HIGH;
    // }
    // else if(strncmp(request, ">WT_USER_VALUE:", 15) == 0)
    // {
    //   myStream->println("GOT WT_USER_VALUE<");
    //   char * commandPayloadPointer = (char *) malloc(11);
    //   strncpy(commandPayloadPointer, &request[15], 11);
    //   commandPayloadPointer[11] ='\0';
    //   lastCommandPayloadAllocated = true;
    //   lastCommandPayload = commandPayloadPointer;
    //   return WT_USER_VALUE;
    // }
    // else if(strncmp(request, ">WT_USER_NOTE:", 14) == 0)
    // {
    //   myStream->println("GOT WT_USER_NOTE<");
    //   char * commandPayloadPointer = (char *) malloc(31);
    //   strncpy(commandPayloadPointer, &request[14], 31);
    //   commandPayloadPointer[31] ='\0';
    //   lastCommandPayloadAllocated = true;
    //   lastCommandPayload = commandPayloadPointer;
    //   return WT_USER_NOTE;
    // }
    // else if(strncmp(request, ">WT_USER_INPUT:", 15) == 0)
    // {
    //   myStream->println("GOT WT_USER_INPUT<");
    //   char * commandPayloadPointer = (char *) malloc(42);
    //   strncpy(commandPayloadPointer, &request[15], 42);
    //   commandPayloadPointer[42] ='\0';
    //   lastCommandPayloadAllocated = true;
    //   lastCommandPayload = commandPayloadPointer;
    //   return WT_USER_INPUT;
    // }
    // else
    // {
    //   char lastDownloadDateEmpty[11] = "0000000000";
    //   strcpy(lastDownloadDate, lastDownloadDateEmpty);
    //   myStream->flush();
    // }
  }
  else if(CommandInterface::state == 1)
  {
    char ack[7] = "";
    myStream->readBytesUntil('<', ack, 7);
    if(strcmp(ack, ">WT_OK") != 0)
    {
      char message[30] = "";
      sprintf(message, "ERROR #%s#", ack);
      myStream->print(message);

      //Flush
      unsigned long now = millis ();
      while (millis () - now < 1000)
      myStream->read ();  // read and discard any input

      CommandInterface::state = 0;
      return WT_CONTROL_NONE;
    }

    char lastFileNameSent[10];
    //bool success = WaterBear_FileSystem::dumpLoggedDataToStream(myStream, &lastFileNameSent); //also needs lastDownloadDate
    if(true)
    {
      // Send last download date to phone for book keeeping
      char transferCompleteMessage[34] = ">WT_COMPLETE:0000000000<";
      strncpy(&transferCompleteMessage[22], lastFileNameSent, 10); // Send timestamp of last file sent
      myStream->write(transferCompleteMessage);
      // TODO: create and pass a data file writer class
      // setNewDataFile();
    }
    else
    {
      // There was some kind of error
    }
    CommandInterface::state = 0;
  }
  return WT_CONTROL_NONE;
}