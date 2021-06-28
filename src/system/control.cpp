#include "control.h"
#include "clock.h"

#define MAX_REQUEST_LENGTH 70 // serial commands

int WaterBear_Control::state = 0;
void * lastCommandPayload;
bool lastCommandPayloadAllocated = false;

bool WaterBear_Control::ready(HardwareSerial &port)
{
  Stream *myStream = &port;
  return WaterBear_Control::ready(myStream);
}

bool WaterBear_Control::ready(Adafruit_BluefruitLE_UART &ble)
{
  Stream *myStream = &ble;
  return WaterBear_Control::ready(myStream);
}

bool WaterBear_Control::ready(Adafruit_BluefruitLE_SPI &ble)
{
  Stream *myStream = &ble;
  return WaterBear_Control::ready(myStream);
}

bool WaterBear_Control::ready(Stream * myStream)
{
  if( (myStream->peek() == '>' && WaterBear_Control::state == 0)
      || WaterBear_Control::state == 1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

int WaterBear_Control::processControlCommands(HardwareSerial &port)
{
  Stream *myStream = &port;
  return WaterBear_Control::processControlCommands(myStream);
}

int WaterBear_Control::processControlCommands(Adafruit_BluefruitLE_UART &ble)
{
  Stream *myStream = &ble;
  return WaterBear_Control::processControlCommands(myStream);
}

int WaterBear_Control::processControlCommands(Adafruit_BluefruitLE_SPI &ble)
{
  Stream *myStream = &ble;
  return WaterBear_Control::processControlCommands(myStream);
}

void * WaterBear_Control::getLastPayload()
{
  return lastCommandPayload;
}


int WaterBear_Control::processControlCommands(Stream * myStream)
{
  char lastDownloadDate[15] = "NOTIMPLEMENTED"; // placeholder

  if(WaterBear_Control::state == 0)
  {
    if(lastCommandPayloadAllocated == true)
    {
      free(lastCommandPayload);
      lastCommandPayloadAllocated = false;
    }
    //awakeTime = WaterBear_Control::timestamp(); // Keep us awake once we are talking to the phone

    char request[MAX_REQUEST_LENGTH] = "";
    myStream->readBytesUntil('<', request, MAX_REQUEST_LENGTH);
    myStream->write(">COMMAND RECIEVED: ");
    myStream->write(&request[1]);
    myStream->write("<");
    myStream->flush();
    delay(100);

    if(strncmp(request, ">WT_OPEN", 19) == 0)
    {
      // TODO:  Need to pass firmware version to control somehow
      /*
      myStream->write(">VERSION:");
      myStream->write(version);
      myStream->write("<");
      myStream->flush();
      delay(100);
      */

      char dateString[26];
      time_t timeNow = timestamp();

      t_t2ts(timeNow, millis(), dateString); // change to 'yyyy-mm-dd hh:mm:ss.sss'?
      myStream->print(">Datalogger Time: ");
      myStream->print(dateString);
      myStream->print("<");
      delay(100);

      myStream->write(">WT_IDENTIFY:");
      // TODO: create and pass a device info object
      myStream->print(F("NOTIMPLEMENTED"));
      for(int i=0; i<8; i++)
      {
      }
      myStream->write("<");
      myStream->flush();

      myStream->write(">WT_TIMESTAMP:");
      myStream->println(timestamp());
      myStream->write("<");
      myStream->flush();
      delay(100);
    }
    else if(strncmp(request, ">WT_DOWNLOAD",12) == 0)
    {
      // Flush the input, would be better to use a delimiter
      // May not be necessary now
      unsigned long now = millis ();
      while (millis () - now < 1000)
      myStream->read ();  // read and discard any input

      if(request[20] == ':')
      {
        // we have a reference date
        // TODO: create and pass a device info object
        strncpy(lastDownloadDate, &request[21], 10);
      }

      myStream->print(">WT_READY<");
      myStream->flush();

      WaterBear_Control::state = 1;
      return WT_CONTROL_NONE;
    }
    else if(strncmp(request, ">WT_SET_RTC:", 12) == 0)
    {
      myStream->println("GOT SET_RTC<"); //acknowledge command sent
      char UTCTime[11] = "0000000000"; // buffer for data to read
      strncpy(UTCTime, &request[12], 10); // read serial data into char string
      time_t value;
      int found = sscanf(&UTCTime[0], "%lld", &value); // turn char string into correct value type
      if(found == 1)
      {
        time_t * commandPayloadPointer = (time_t *) malloc(sizeof(time_t));
        *commandPayloadPointer = value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }
      return WT_SET_RTC;
      // TODO: create and pass a data file writer class
      // setNewDataFile();
    }
    else if(strncmp(request, ">WT_DEPLOY:", 11) == 0)
    {
      myStream->println("GOT WT_DEPLOY<");
      char * commandPayloadPointer = (char *) malloc(26);
      strncpy(commandPayloadPointer, &request[11], 26);
      commandPayloadPointer[26] ='\0';
      lastCommandPayloadAllocated = true;
      lastCommandPayload = commandPayloadPointer;
      return WT_DEPLOY;

      // TODO: create and pass a data file writer class
      // setNewDataFile();
    }
    else if(strncmp(request, ">WT_CONFIG:", 10) == 0)
    { //flags:time, conduct, therm
      myStream->println(">CONFIG<");
      char * commandPayloadPointer = (char *)malloc(8);
      strncpy(commandPayloadPointer, &request[11],8);
      commandPayloadPointer[8] ='\0';
      lastCommandPayloadAllocated = true;
      lastCommandPayload = commandPayloadPointer;
      return WT_CONTROL_CONFIG;
      // go into config mode
    }
    else if(strncmp(request, ">WT_DEBUG_VALUES", 16) == 0)
    {
      myStream->println(">DEBUG_VALUES<");
      return WT_DEBUG_VAlUES;
    }
    else if(strncmp(request, ">WT_CLEAR_MODES", 15) == 0)
    {
      myStream->println(">CLEAR_MODES<");
      return WT_CLEAR_MODES;
    }
    else if(strncmp(request, ">CAL_DRY", 8) == 0)
    {
      myStream->println(">GOT CAL_DRY<");
      return WT_CONTROL_CAL_DRY;
    }
    else if(strncmp(request, ">CAL_LOW:", 9) == 0)
    {
      myStream->println(">GOT CAL_LOW<");
      char calibrationPointStringValue[10];
      strncpy(calibrationPointStringValue, &request[9], 9);
      int value;
      int found = sscanf(&calibrationPointStringValue[0], "%d", &value);
      if(found == 1)
      {
        int * commandPayloadPointer = (int *) malloc(sizeof(int));
        *commandPayloadPointer = value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }
      return WT_CONTROL_CAL_LOW;
    }
    else if(strncmp(request, ">CAL_HIGH:", 10) == 0)
    {
      myStream->println(">GOT CAL_HIGH<");
      char calibrationPointStringValue[10];
      strncpy(calibrationPointStringValue, &request[10], 9);
      int value;
      myStream->println(calibrationPointStringValue); // what is this line for???
      int found = sscanf(&calibrationPointStringValue[0], "%d", &value);
      if(found == 1)
      {
        int * commandPayloadPointer = (int *) malloc(sizeof(int));
        *commandPayloadPointer = value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }
      return WT_CONTROL_CAL_HIGH;
    }
    else if(strncmp(request, ">WT_CAL_TEMP", 12) == 0)
    {
      myStream->println(">CAL_TEMP<");
      return WT_CAL_TEMP;
    }
    else if(strncmp(request, ">TEMP_CAL_LOW:", 14) == 0)
    {
      myStream->println(">GOT TEMP_CAL_LOW<");
      char calibrationPointStringValue[8];
      strncpy(calibrationPointStringValue, &request[14], 7);
      float value;
      myStream->println(calibrationPointStringValue);
      int found = sscanf(&calibrationPointStringValue[0], "%f", &value);
      if(found == 1){
        value = value * 100;
        unsigned short * commandPayloadPointer = (unsigned short *) malloc(sizeof(unsigned short));
        *commandPayloadPointer = (unsigned short)value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }
      return WT_TEMP_CAL_LOW;
    }
    else if(strncmp(request, ">TEMP_CAL_HIGH:", 15) == 0)
    {
      myStream->println(">GOT TEMP_CAL_HIGH<");
      char calibrationPointStringValue[8];
      strncpy(calibrationPointStringValue, &request[15], 7);
      float value;
      myStream->println(calibrationPointStringValue);
      int found = sscanf(&calibrationPointStringValue[0], "%f", &value); // xxx.xx
      if(found == 1){
        value = value * 100; // either here or at the case
        unsigned short * commandPayloadPointer = (unsigned short *) malloc(sizeof(unsigned short));
        *commandPayloadPointer = (unsigned short)value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }
      return WT_TEMP_CAL_HIGH;
    }
    else if(strncmp(request, ">WT_USER_VALUE:", 15) == 0)
    {
      myStream->println("GOT WT_USER_VALUE<");
      char * commandPayloadPointer = (char *) malloc(11);
      strncpy(commandPayloadPointer, &request[15], 11);
      commandPayloadPointer[11] ='\0';
      lastCommandPayloadAllocated = true;
      lastCommandPayload = commandPayloadPointer;
      return WT_USER_VALUE;
    }
    else if(strncmp(request, ">WT_USER_NOTE:", 14) == 0)
    {
      myStream->println("GOT WT_USER_NOTE<");
      char * commandPayloadPointer = (char *) malloc(31);
      strncpy(commandPayloadPointer, &request[14], 31);
      commandPayloadPointer[31] ='\0';
      lastCommandPayloadAllocated = true;
      lastCommandPayload = commandPayloadPointer;
      return WT_USER_NOTE;
    }
    else if(strncmp(request, ">WT_USER_INPUT:", 15) == 0)
    {
      myStream->println("GOT WT_USER_INPUT<");
      char * commandPayloadPointer = (char *) malloc(42);
      strncpy(commandPayloadPointer, &request[15], 42);
      commandPayloadPointer[42] ='\0';
      lastCommandPayloadAllocated = true;
      lastCommandPayload = commandPayloadPointer;
      return WT_USER_INPUT;
    }
    else
    {
      char lastDownloadDateEmpty[11] = "0000000000";
      strcpy(lastDownloadDate, lastDownloadDateEmpty);
      myStream->flush();
    }
  }
  else if(WaterBear_Control::state == 1)
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

      WaterBear_Control::state = 0;
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
    WaterBear_Control::state = 0;
  }
  return WT_CONTROL_NONE;
}