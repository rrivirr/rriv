#include "command.h"
#include "clock.h"
#include <re.h>
#include <Cmd.h>

#define MAX_REQUEST_LENGTH 70 // serial commands

int CommandInterface::state = 0;
void * lastCommandPayload;
bool lastCommandPayloadAllocated = false;

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
}

void CommandInterface::setup()
{
  cmdInit(&Serial2);
  cmdAdd("debug", toggleDebug);
  
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