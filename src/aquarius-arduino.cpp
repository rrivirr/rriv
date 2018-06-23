#include <Wire.h>  // Communicate with I2C/TWI devices
#include <SPI.h>
#include <SoftwareSerial.h> // Debugging
#include <EEPROM.h>
#include "SdFat.h"
#include "RTClib.h"
#include "TrueRandom.h"
#include "LowPower.h"


RTC_PCF8523 RTC; // define the Real Time Clock object

SoftwareSerial mySerial(4, 3); // RX, TX for easier debuggin


// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// File system object.
SdFat sd;
File logfile;

int state = 0;

char lastDownloadDate[11] = "0000000000";

char version[5] = "v1.2";


int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

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



  char dataFolder[6] = "/Data";
  if(!sd.exists(dataFolder)){
    sd.chdir("/");
    sd.mkdir("/Data");
  }

  if (!sd.chdir(dataFolder)) {
    Serial.println(F("chdir failed for Data."));
  }
  logfile = sd.open(filename, FILE_WRITE);
  //sd.chdir();
  if(!logfile){
    Serial.println(F("file did not open"));
    while(1);
  }

  char line[100];
  if (!sd.chdir("/")) {
    Serial.println(F("chdir failed for Columns."));
  }
  File file;
  file = sd.open("COLUMNS.TXT");

  if (!file.isOpen()){
    Serial.println(F("COLUMNS.TXT did not open"));
    //while(1);
  }

  int n = file.fgets(line, sizeof(line));
  if (line[n - 1] == '\n') {
        // remove '\n'
        line[n-1] = 0;
  }
  Serial.println(line);
  file.close();
  logfile.println(line); // write the headers to the new logfile

  Serial.print(F("Logging to: "));
  Serial.println(filename);
}

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

int uniqueIdAddressStart = 0;
int uniqueIdAddressEnd = 15;
unsigned char uuid[16];

void readUniqueId(){

  for(int i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
    int address = uniqueIdAddressStart + i;
    uuid[i] = EEPROM.read(address);
  }

  Serial.println(F("Here's the uuid in EEPROM"));
  for(int i=0; i<8; i++){
    Serial.print((unsigned int) uuid[2*i], HEX);
  }
  Serial.println("");

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  //Serial.println("Here's the uninitializedEEPROM string that should be");
  //for(int i=0; i<8; i++){
    //Serial.print((unsigned int) uninitializedEEPROM[2*i], HEX);
  //}
  //Serial.println("");


  if(memcmp(uuid, uninitializedEEPROM, 16) == 0){
    Serial.println(F("Generate UUID"));
    // generate the unique ID
    TrueRandomClass::uuid(uuid);
    for(int i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
      int address = uniqueIdAddressStart + i;
      EEPROM.write(address, uuid[i]);
    }
  }

}


void initializeSDCard(void) {
  Wire.begin();

  if (! RTC.initialized()) {
    Serial.println(F("RTC is NOT initialized!"));
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!RTC.begin()) {
    #if ECHO_TO_SERIAL
    Serial.println(F("RTC failed"));
    #endif  //ECHO_TO_SERIAL
  } else {
    Serial.println(F("RTC started"));


    DateTime now = RTC.now();
    char dateString[11];
    sprintf(dateString, "%lu", now.unixtime());
    Serial.print(">Datalogger Time: ");
    Serial.print(now.year());
    Serial.print("-");
    Serial.print(now.month());
    Serial.print("-");
    Serial.print(now.day());
    Serial.print(" ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.print(now.second());
    Serial.print("<");
    //Serial.println(RTC.now().m);
  }

  SdFile::dateTimeCallback(dateTime);

  // initialize the SD card
  Serial.print(F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!sd.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    while(1);
  } else {
    Serial.println(F("card initialized."));
  }



  setNewDataFile();


}


bool wake = false;
uint32_t awakeTime = 0;

/**************************************************************************/
/*
Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);

  //while (!Serial);
  Serial.println(F("Hello, world.  Primary Serial."));

  //Serial.begin(9600);

  //mySerial.begin(115200);
  //mySerial.println(F("Hello, world?  Secondary Serial!"));

  readUniqueId();

  /* Get the CD Card going */
  initializeSDCard();

  /* We're ready to go! */
  Serial.println(F("done with setup"));

  wake = true;
  awakeTime = RTC.now().unixtime();

}

void printCurrentDirListing(){
  sd.vwd()->rewind();
  SdFile dirFile;
  char sdFilename[20];
  while (dirFile.openNext(sd.vwd(), O_READ)) {
    dir_t d;
    if (!dirFile.dirEntry(&d)) {
      Serial.println(F("f.dirEntry failed"));
      while(1);
    }

    dirFile.getName(sdFilename, 20);
    Serial.println(sdFilename);
  }

}

/**************************************************************************/
/*
Arduino loop function, called once 'setup' is complete (your own code
should go here)
*/
/**************************************************************************/
uint32_t lastTime = 0;
const int maxRequestLength = 34;

void loop(void)
{

  if(freeRam() < 100){
    Serial.println(freeRam());
  }
  // Just trigger the dump event using a basic button
  //int buttonState = digitalRead(buttonPin);

  if(Serial.peek() == '>' && state == 0){
    Serial.println("Peek");

    wake = true;
    awakeTime = RTC.now().unixtime(); // Keep us awake once we are talking to the phone

    char request[maxRequestLength] = "";
    Serial.readBytesUntil('<', request, maxRequestLength);
    Serial.write(">COMMAND RECIEVED: ");
    Serial.write(&request[1]);
    Serial.write("<");
    Serial.flush();
    delay(100);

    if(strncmp(request, ">WT_OPEN_CONNECTION", 19) == 0) {
      Serial.write(">VERSION:");
      Serial.write(version);
      Serial.write("<");
      Serial.flush();
      delay(100);
      Serial.write(">WT_IDENTIFY_DEVICE:");
      for(int i=0; i<8; i++){
        Serial.print((unsigned int) uuid[2*i], HEX);
      }
      Serial.write("<");
      Serial.flush();

    }
    else if(strncmp(request, ">WT_REQUEST_DOWNLOAD",20) == 0) {
      // Flush the input, would be better to use a delimiter
      // May not be necessary now
      unsigned long now = millis ();
      while (millis () - now < 1000)
      Serial.read ();  // read and discard any input

      if(request[20] == ':'){
        // we have a reference date
        strncpy(lastDownloadDate, &request[21], 10);
      }

      Serial.print(">WT_TRANSFER_READY<");
      Serial.flush();
      state = 1;
      return;
    } else if(strncmp(request, ">WT_SET_RTC:", 12) == 0){
      char UTCTime[11] = "0000000000";
      strncpy(UTCTime, &request[12], 10);
      //UTCTime[10] = '\0';
      long time = atol(UTCTime);
      delay(100);
      Serial.print(">Received UTC time: ");
      Serial.print(UTCTime);
      Serial.print("---");
      Serial.print(time);
      Serial.print("<");
      Serial.flush();

      RTC.adjust(DateTime(time));

      Serial.print(">Received UTC time: ");
      Serial.print(UTCTime);
      Serial.print("---");
      Serial.print(time);
      Serial.print("---");
      Serial.print(RTC.now().unixtime());
      Serial.print("<");
      Serial.flush();

      setNewDataFile();
      return;

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


    // Debug
    //printCurrentDirListing();

    if (!sd.chdir("/Data")) {
      Serial.println(F("chdir failed for Data."));
      state = 0;
      return;
    }

    // Debug
    //  printCurrentDirListing();


    sd.vwd()->rewind();
    SdFile dirFile;
    char sdFilename[20];
    while (dirFile.openNext(sd.vwd(), O_READ)) {
      dir_t d;
      if (!dirFile.dirEntry(&d)) {
        Serial.println(F("f.dirEntry failed"));
        while(1);
      }

      dirFile.getName(sdFilename, 20);

      if(strncmp(sdFilename, lastDownloadDate, 10) > 0){
        //Serial.println(sdFilename);

        File datafile = sd.open(sdFilename);
        // send size of transmission ?
        // Serial.println(datafile.fileSize());
        while (datafile.available()) {
          Serial.write(datafile.read());
        }
        datafile.close();
      }
      dirFile.close();

    }
    if (!sd.chdir()) {
      Serial.println(F("chdir failed for ../"));
      state = 0;
      return;
    }

    char transferCompleteMessage[34] = ">WT_TRANSFER_COMPLETE:0000000000<";
    strncpy(&transferCompleteMessage[22], sdFilename, 10);
    Serial.write(transferCompleteMessage);

    setNewDataFile();

    // Send last download date to phone for book keeeping
    state = 0;

  }


  // Fetch the time
  DateTime now = RTC.now();

  short interval = 15; // minutes between loggings
  uint32_t trigger = 60*interval;
  uint32_t currentTime = now.unixtime();
  uint32_t elapsedTime = currentTime - lastTime;
  short minute = now.minute();

  if(currentTime > awakeTime + 60 * 5){
    wake = false;
  }
  //Serial.println(elapsedTime);
  if(elapsedTime < trigger
  || ( minute % interval != 0  )  ){
    if(!wake) {
      if(elapsedTime < trigger - 10){ // If we are withing ten secs of the trigger, don't sleep
        Serial.println("power down");
        Serial.flush();
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      } else if (elapsedTime < trigger - 3){
        LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
      }
    }
    return;
  }
  lastTime = now.unixtime();


  // get the new data

  // log uuid and time

  for(int i=0; i<8; i++){
    logfile.print((unsigned int) uuid[2*i], HEX);
  }
  char comma[2] = ",";
  logfile.print(comma);
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(comma);
  Serial.print(currentTime);


  float value0 = analogRead(0); // * .0049;
  Serial.print(" 0: ");
  Serial.print(value0);
  logfile.print(value0);
  logfile.print(comma);

  float value1 = analogRead(1); // * .0049;
  Serial.print("   1: ");
  Serial.print(value1);
  logfile.print(value1);
  logfile.print(comma);

  float value2 = analogRead(2); // * .0049;
  //Serial.println(value2);
  logfile.print(value2);
  logfile.print(comma);

  float value3 = analogRead(3); // * .0049;
  //Serial.println(value3);
  logfile.print(value3);
  logfile.print(comma);

  float value4 = analogRead(4); // * .0049;
  Serial.print("   4: ");
  Serial.print(value4);
  logfile.print(value4);
  logfile.print(comma);

  float value5 = analogRead(5); // * .0049;
  Serial.print("   5: ");
  Serial.println(value5);
  logfile.print(value5);
  logfile.print(comma);

  logfile.println();
  logfile.flush();


}
