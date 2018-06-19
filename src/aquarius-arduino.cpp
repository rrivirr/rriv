#include <Wire.h>  // Communicate with I2C/TWI devices
#include <SPI.h>
//#include <SoftwareSerial.h> // Debugging
#include <EEPROM.h>
#include "SdFat.h"
#include "RTClib.h"
#include "TrueRandom.h"

RTC_PCF8523 RTC; // define the Real Time Clock object

//SoftwareSerial mySerial(4, 3); // RX, TX for easier debuggin


// for the data logging shield, we use digital pin 10 for the SD cs line
const short chipSelect = 10;

// File system object.
SdFat sd;
File logfile;

short state = 0;

char lastDownloadDate[11] = "0000000000";
char dataDirectory[6] = "/Data";


//int freeRam () {
//  extern int __heap_start, *__brkval;
//  int v
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
//}

short uniqueIdAddressStart = 0;
short uniqueIdAddressEnd = 15;
unsigned char uuid[16];

short deploymentIdentifierAddressStart = 16;
short deploymentIdentifierAddressEnd =  43;

void readUniqueId(){

  for(short i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
    short address = uniqueIdAddressStart + i;
    uuid[i] = EEPROM.read(address);
  }

/*
  Serial.write( (char *) F("uuid:"));
  for(short i=0; i<8; i++){
    Serial.print((unsigned int) uuid[2*i], HEX);
  }
  Serial.println("");
*/

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

  if(memcmp(uuid, uninitializedEEPROM, 16) == 0){
    //Serial.println(F("Generate UUID"));
    // generate the unique ID
    TrueRandomClass::uuid(uuid);
    for(short i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
      short address = uniqueIdAddressStart + i;
      EEPROM.write(address, uuid[i]);
    }
  }

}

void readDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i <= deploymentIdentifierAddressEnd - deploymentIdentifierAddressStart; i++){
    short address = deploymentIdentifierAddressStart + i;
    deploymentIdentifier[i] = EEPROM.read(address);
  }
}

void writeDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i <= deploymentIdentifierAddressEnd - deploymentIdentifierAddressStart; i++){
    short address = deploymentIdentifierAddressStart + i;
    EEPROM.write(address, deploymentIdentifier[i]);
  }
}

/*
void printCurrentDirListing(){
  sd.vwd()->rewind();
  SdFile dirFile;
  char sdFilename[30];
  while (dirFile.openNext(sd.vwd(), O_READ)) {
    dirFile.getName(sdFilename, 30);
    Serial.println(sdFilename);
    dirFile.close();
  }
}
*/

/*
* Initialize the SD Card
*/

void setNewDataFile(){
  DateTime now = RTC.now();
  char uniquename[11];
  sprintf(uniquename, "%lu", now.unixtime());
  char suffix[5] = ".CSV";
  char filename[15];
  strncpy(filename, uniquename, 10);
  strncpy(&filename[10], suffix, 5);

  //Serial.println("cd");
  //sd.chdir("/");
  //delay(10);

  // printCurrentDirListing();
  //Serial.println("OK");
  // if(!sd.exists(dataDirectory)){
  //  Serial.println("mkdir");
  //  sd.mkdir(dataDirectory);
    //delay(10);
  //}
  //Serial.println("OK");
  if (!sd.chdir(dataDirectory)) {
    Serial.println(F("failed: /Data."));
  } else {
    //Serial.println(F("cd /Data."));
  }
  //delay(10);

  //Serial.println("OK");

  char deploymentIdentifier[29];// = "DEPLOYMENT";
  readDeploymentIdentifier(deploymentIdentifier);
  unsigned char empty[1] = {0xFF};
  if(memcmp(deploymentIdentifier, empty, 1) == 0 ) {
    //Serial.print(">NoDplyment<");
    //Serial.flush();

    char defaultDeployment[25] = "SITENAME_00000000000000";
    writeDeploymentIdentifier(defaultDeployment);
    readDeploymentIdentifier(deploymentIdentifier);
  }


  if(!sd.exists(deploymentIdentifier)){
    // printCurrentDirListing();
    Serial.write("mkdir:");
    Serial.println(deploymentIdentifier);
    sd.mkdir(deploymentIdentifier);
    //delay(10);
  }

  if (!sd.chdir(deploymentIdentifier)) {
    Serial.print("failed:");
    Serial.println(deploymentIdentifier);
  } else {
    Serial.print("cd:");
    Serial.println(deploymentIdentifier);
  }
  //delay(10);

  logfile = sd.open(filename, FILE_WRITE);
  //delay(10);

  //sd.chdir();
  if(!logfile){
    Serial.print(">not found<");
    while(1);
  }

  //if (!sd.chdir("/")) {
  //  Serial.println("fail /");
  //}


  File file;
  file = sd.open("/COLUMNS.TXT");

  if (!file){
    Serial.println("fail COLS");
    while(1);
  }

  char line[100];
  int n = file.fgets(line, sizeof(line));
  if (line[n - 1] == '\n') {
        // remove '\n'
        line[n - 1] = 0;
  }
  Serial.println(line);
  file.close();
  logfile.println(line); // write the headers to the new logfile

  Serial.print( ">log:");
  Serial.print(filename);
  Serial.println( "<");
/* */
}

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


void initializeSDCard(void) {
  Wire.begin();

  if (! RTC.initialized()) {
    // Serial.println(F("RTC NOT init'd"));
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!RTC.begin()) {
    Serial.println(F("RTC fail"));
  } else {
    Serial.println(F("RTC start"));
  }

  SdFile::dateTimeCallback(dateTime);

  // initialize the SD card
  //Serial.write( (char *) F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!sd.begin(chipSelect)) {
    Serial.println(F(">Card fail<"));
  }
  Serial.println(F(">Card init'd.<"));

  setNewDataFile();

}

/**************************************************************************/
/*
Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  //Serial.println(F("Hello, world.  Primary Serial."));
  //Serial.begin(9600);

  //mySerial.begin(115200);
  //mySerial.println(F("Hello, world?  Secondary Serial!"));

  readUniqueId();

  /* Get the CD Card going */
  initializeSDCard();

  /* We're ready to go! */
  //Serial.println(F("done with setup"));

}

/*
void transferLoggedData(){

  // Debug
  // printCurrentDirListing();

  if (!sd.chdir(dataDirectory)) {
    Serial.println(F("fail: Data."));
    state = 0;
    return;
  }

  // Debug
  // printCurrentDirListing();
  // Serial.println(lastDownloadDate);

  sd.vwd()->rewind();
  SdFile dirFile;
  char sdDirName[30];
  char sdFileName[30];
  short rootDirIndex = 0;
  while (dirFile.openNext(sd.vwd(), O_READ)) {
    dir_t d;
    if (!dirFile.dirEntry(&d)) {
      //Serial.println(F("dirEntry failed"));
      state = 0;
      return;
    }
    if(!dirFile.isDir()){
      // Descend into all the deployment directories
      continue;
    }
    dirFile.getName(sdDirName, 30);
    //Serial.write("Dir: ");
    //Serial.println(sdDirName);

    if(! sd.chdir(sdDirName) ){
      Serial.write("fail:");
      Serial.println(sdDirName);
      state = 0;
      return;
    }
    dirFile.close();

    sd.vwd()->rewind();
    SdFile deploymentFile;
    while (deploymentFile.openNext(sd.vwd(), O_READ)) {
      dir_t d;
      if (!deploymentFile.dirEntry(&d)) {
        // Serial.println(F("depl file fail"));
        state = 0;
        return;
      }
      deploymentFile.getName(sdFileName, 24);


      if(strncmp(sdFileName, lastDownloadDate, 10) > 0){
          //Serial.println(sdFilename);

          File datafile = sd.open(sdFileName);
          // send size of transmission ?
          // Serial.println(datafile.fileSize());
          while (datafile.available()) {
              Serial.write(datafile.read());
          }
          datafile.close();
       }
       deploymentFile.close();
    }

    //char deploymentCompleteMessage[34] = ">WT_DEPLOYMENT_TRANSFERRED<";
    //Serial.write(deploymentCompleteMessage);

    sd.chdir(dataDirectory);

    rootDirIndex = rootDirIndex + 1;

    // sd.vwd()->rewind();
    // printCurrentDirListing();
    // Advance to the last directoy we were at
    sd.vwd()->rewind();

    for(short i = 0; i < rootDirIndex; i = i + 1){
      dirFile.openNext(sd.vwd(), O_READ);
      dirFile.close();
    }
  }

  if (!sd.chdir("/")) {
    Serial.println(F("fail /"));
    state = 0;
    return;
  }

  char transferCompleteMessage[34] = ">WT_COMPLETE:0000000000<";
  strncpy(&transferCompleteMessage[22], sdFileName, 10); // Send timestamp of last file sent
  Serial.write(transferCompleteMessage);

  setNewDataFile();

  // Send last download date to phone for book keeeping
  state = 0;

}

/**************************************************************************/
/*
Arduino loop function, called once 'setup' is complete (your own code
should go here)
*/
/**************************************************************************/
uint32_t lastTime = 0;
const short maxRequestLength = 50;
void loop(void)
{
/*
  //transferLoggedData();
  //while(1);

  //if(freeRam() < 100){
  //  Serial.println(freeRam());
  //}
  // Just trigger the dump event using a basic button
  //int buttonState = digitalRead(buttonPin);

  if(Serial.peek() == '>' && state == 0){
    Serial.println("Pk");

    char request[maxRequestLength] = "";
    Serial.readBytesUntil('<', request, maxRequestLength);
    Serial.write(">CMD RECV: ");
    Serial.write(&request[1]);
    Serial.write("<");
    Serial.flush();
    delay(250);

    if(strncmp(request, ">WT_OPEN", 8) == 0) {
      Serial.write(">WT_IDENTIFY:");
      for(short i=0; i<8; i++){
        Serial.print((unsigned int) uuid[2*i], HEX);
      }
      Serial.write("<");
      Serial.flush();
      delay(100);

      Serial.write(">WT_TIMESTAMP:");
      Serial.print(RTC.now().unixtime());
      Serial.write("<");
      Serial.flush();
      delay(100);

    }
    else if(strncmp(request, ">WT_DOWNLOAD",12) == 0) {
      // Flush the input, would be better to use a delimiter
      // May not be necessary now
      unsigned long now = millis ();
      while (millis () - now < 1000)
      Serial.read ();  // read and discard any input

      if(request[20] == ':'){
        // we have a reference date
        strncpy(lastDownloadDate, &request[21], 10);
      }

      Serial.write(">WT_READY<");
      Serial.flush();
      state = 1;
      return;
    } else if(strncmp(request, ">WT_SET_RTC:", 12) == 0){
      char UTCTime[11] = "0000000000";
      strncpy(UTCTime, &request[12], 10);
      //UTCTime[10] = '\0';
      long time = atol(UTCTime);
      delay(100);

      RTC.adjust(DateTime(time));

      //Serial.write( (char *) F(">RECV UTC: "));
      //Serial.print(UTCTime);
      //Serial.write( (char *) F("--"));
      //Serial.print(time);
      //Serial.write( (char *) F("--"));
      //Serial.print(RTC.now().unixtime());
      //Serial.write( (char *) F("<"));
      //Serial.flush();


      setNewDataFile();

    } else if(strncmp(request, ">WT_DEPLOY:", 11) == 0){
      char deploymentIdentifier[29];
      strncpy(deploymentIdentifier, &request[11], 28);
      writeDeploymentIdentifier(deploymentIdentifier);
      Serial.write(">Wrote: ");
      Serial.write(deploymentIdentifier);
      Serial.write("<");
      Serial.flush();

      setNewDataFile();

    } else {
      char lastDownloadDateEmpty[11] = "0000000000";
      strcpy(lastDownloadDate, lastDownloadDateEmpty);
    }

  } else if(state == 1){

    char ack[7] = "";
    Serial.readBytesUntil('<', ack, 7);
    if(strcmp(ack, ">WT_OK") != 0) {
      char message[30] = "";
      sprintf(message, "ERROR #%s#", ack);
      Serial.print(message);

      //Flush
      unsigned long now = millis ();
      while (millis () - now < 1000)
      Serial.read ();  // read and discard any input

      state = 0;
      return;
    }

    transferLoggedData();

  }

/*

  // Fetch the time
  DateTime now = RTC.now();

  uint32_t elapsedTime = now.unixtime() - lastTime;
  if(elapsedTime <5){
    return;
  }
  lastTime = now.unixtime();


  // Write the deployment identifier
  char comma[2] = ",";
  char deploymentIdentifier[29];// = "DEPLOYMENT";
  readDeploymentIdentifier(deploymentIdentifier);
  logfile.print(deploymentIdentifier);
  logfile.write("_");
  for(short i=0; i<8; i++){
    logfile.print((unsigned int) uuid[2*i], HEX);
  }
  logfile.print(comma);

  // Log uuid and time
  for(short i=0; i<8; i++){
    logfile.print((unsigned int) uuid[2*i], HEX);
  }
  logfile.print(comma);
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(comma);

  // Get the new data
  float value0 = analogRead(0); // * .0049;
  Serial.write("0:");
  Serial.print(value0);
  logfile.print(value0);
  logfile.print(comma);

  float value1 = analogRead(1); // * .0049;
  Serial.write(" 1:");
  Serial.print(value1);
  logfile.print(value1);
  logfile.print(comma);

  //float value2 = analogRead(2); // * .0049;
  //Serial.println(value2);
  //logfile.print(value2);
  //logfile.print(comma);

  //float value3 = analogRead(3); // * .0049;
  //Serial.println(value3);
  //logfile.print(value3);
  //logfile.print(comma);

  //float value4 = analogRead(4); // * .0049;
  //Serial.write(" 4:");
  //Serial.print(value4);
  //logfile.print(value4);
  //logfile.print(comma);

  //float value5 = analogRead(5); // * .0049;
  //Serial.write(" 5:");
  //Serial.println(value5);
  //logfile.print(value5);
  //logfile.print(comma);


  logfile.println();
  logfile.flush();
  */
}
