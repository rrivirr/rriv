#include <Wire.h>

#include <SPI.h>
#include "SdFat.h"
#include "RTClib.h"
#include <EEPROM.h>
#include "TrueRandom.h"
#include <SoftwareSerial.h>


#define LOG_INTERVAL  1000 // mills between entries
#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()


RTC_PCF8523 RTC; // define the Real Time Clock object

SoftwareSerial mySerial(4, 3); // RX, TX for easier debuggin


// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// File system object.
SdFat sd;
File logfile;

int state = 0;

char lastDownloadDateEmpty[11] = "0000000000";
char lastDownloadDate[11] = "0000000000";


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

  Serial.print("Logging to: ");
  Serial.println(filename);

  //d.chdir("Data", true);
  logfile = sd.open(filename, FILE_WRITE);
  //sd.chdir();
  if(!logfile){
    Serial.println("file did not open");
    while(1);
  }
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

  Serial.println("Here's the uuid in EEPROM");
  for(int i=0; i<8; i++){
    Serial.print((unsigned int) uuid[2*i], HEX);
  }
  Serial.println("");

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  //erial.println("Here's the uninitializedEEPROM string that should be");
  for(int i=0; i<8; i++){
    Serial.print((unsigned int) uninitializedEEPROM[2*i], HEX);
  }

  if(memcmp(uuid, uninitializedEEPROM, 16) == 0){
    Serial.println("Generate UUID");
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
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!RTC.begin()) {
    #if ECHO_TO_SERIAL
    Serial.println("RTC failed");
    #endif  //ECHO_TO_SERIAL
  } else {
    Serial.println("RTC started");
    //Serial.println(RTC.now().m);
  }

  SdFile::dateTimeCallback(dateTime);

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!sd.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");

  setNewDataFile();

  logfile.println("time,data1,data2,data3,data4,data5,data6,data7,data8");


}

/**************************************************************************/
/*
Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  Serial.println("Hello, world.  Primary Serial.");
  //Serial.begin(9600);

  mySerial.begin(115200);
  mySerial.println("Hello, world?  Secondary Serial!");

  return;

  readUniqueId();

  /* Get the CD Card going */
  initializeSDCard();

  /* We're ready to go! */
  Serial.println("done with setup");

}

void printCurrentDirListing(){
  sd.vwd()->rewind();
  SdFile dirFile;
  char sdFilename[20];
  while (dirFile.openNext(sd.vwd(), O_READ)) {
    dir_t d;
    if (!dirFile.dirEntry(&d)) {
      Serial.println("f.dirEntry failed");
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
  //Serial.println("Loop");
  DateTime now;

  // Just trigger the dump event using a basic button
  //int buttonState = digitalRead(buttonPin);

  if(Serial.peek() == '>' && state == 0){
    //Serial.println("Peek");

    char request[maxRequestLength] = "";
    Serial.readBytesUntil('<', request, maxRequestLength);
    Serial.println(request);
    if(strncmp(request, ">WT_OPEN_CONNECTION", 19) == 0) {
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
    } else {
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
    printCurrentDirListing();

    if (!sd.chdir("/Data")) {
      Serial.println("chdir failed for Data.\n");
      state = 0;
      return;
    }

    // Debug
    printCurrentDirListing();


    sd.vwd()->rewind();
    SdFile dirFile;
    char sdFilename[20];
    while (dirFile.openNext(sd.vwd(), O_READ)) {
      dir_t d;
      if (!dirFile.dirEntry(&d)) {
        Serial.println("f.dirEntry failed");
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
      Serial.println("chdir failed for ../\n");
      state = 0;
      return;
    }


    setNewDataFile();
    char transferCompleteMessage[34] = ">WT_TRANSFER_COMPLETE:0000000000<";
    strncpy(&transferCompleteMessage[22], sdFilename, 10);
    Serial.write(transferCompleteMessage);


    // Send last download date to phone for book keeeping
    state = 0;

  }


  // Fetch the time
  now = RTC.now();
  uint32_t elapsedTime = now.unixtime() - lastTime;
  if(elapsedTime < 5){
    return;
  }
  lastTime = now.unixtime();


  // log uuid and time
  for(int i=0; i<8; i++){
    logfile.print((unsigned int) uuid[2*i], HEX);
  }
  logfile.print(",");
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(",");

  logfile.println();
  logfile.flush();

  delay(250);

}
