#include <Arduino.h>
#include <RTClock.h>
#include <Wire.h>  // Communicate with I2C/TWI devices
#include <SPI.h>
#include "SdFat.h"
//#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"


// Old code from Uno
//#include <EEPROM.h>
//#include "RTClib.h"
//#include "TrueRandom.h"
//#include "LowPower.h"


//RTC_PCF8523 RTC; // define the Real Time Clock object
//struct rtc_module rtc_instance;

// Pin Mappings for Nucleo Board
#define D3 PB3 // Why isn't D3 working ??
#define D4 PB5

int bluefruitModePin = D3;
Adafruit_BluefruitLE_UART ble(Serial1, bluefruitModePin);


// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// File system object.
SdFat sd;
File logfile;

int state = 0;

char lastDownloadDate[11] = "0000000000";

char version[5] = "v2.0";

short interval = 15; // minutes between loggings
short burstLength = 100; // how many readings in a burst

uint32_t lastTime = 0;
const int maxRequestLength = 34;
short burstCount = 0;

/* Uno specific code
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
*/

// A small helper
void error(const __FlashStringHelper*err) {
  Serial2.println("Error:");
  Serial2.println(err);
  while (1);
}


/*
* Initialize the SD Card
*/
void setNewDataFile(){
  //DateTime now = RTC.now();

  //rtc_calendar_time calendarTime;
  // rtc_calendar_get_time (struct rtc_module *const module, struct rtc_calendar_time *const time)
  //http://asf.atmel.com/docs/latest/samr21/html/asfdoc_sam0_rtc_calendar_basic_use_case.html

  char uniquename[11];
  //sprintf(uniquename, "%lu", now.unixtime());
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
    Serial2.println(F("chdir failed for Data."));
  }
  logfile = sd.open(filename, FILE_WRITE);
  //sd.chdir();
  if(!logfile){
    Serial2.println(F("file did not open"));
    while(1);
  }

  char line[100];
  if (!sd.chdir("/")) {
    Serial2.println(F("chdir failed for Columns."));
  }
  File file;
  file = sd.open("COLUMNS.TXT");

  if (!file.isOpen()){
    Serial2.println(F("COLUMNS.TXT did not open"));
    //while(1);
  }

  int n = file.fgets(line, sizeof(line));
  if (line[n - 1] == '\n') {
        // remove '\n'
        line[n-1] = 0;
  }
  Serial2.println(line);
  file.close();
  logfile.println(line); // write the headers to the new logfile

  Serial2.print(F("Logging to: "));
  Serial2.println(filename);
}

void dateTime(uint16_t* date, uint16_t* time) {
  //DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  //*date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  //*time = FAT_TIME(now.hour(), now.minute(), now.second());
}

int uniqueIdAddressStart = 0;
int uniqueIdAddressEnd = 15;
unsigned char uuid[16];


void firstRun(){

  // if we don't have a UUID yet, we are running for the first time
  // set a mode pin for USART1 if we need to


  Serial2.println("First Run");
  ble.factoryReset();
  ble.setMode(BLUEFRUIT_MODE_COMMAND);
  //digitalWrite(D4, HIGH);

  // Send command
  ble.println(F("AT+GAPDEVNAME=WaterBear2"));
  ble.waitForOK();
  ble.println(F("ATZ"));
  ble.waitForOK();

  // Place bluefruit into a data mode
  //delay(5000);
  ble.setMode(BLUEFRUIT_MODE_DATA);
  //digitalWrite(D4, LOW);

}

/*void readUniqueId(){

  for(int i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
    int address = uniqueIdAddressStart + i;
    uuid[i] = EEPROM.read(address);
  }

  Serial2.println(F("UUID in EEPROM:"));
  for(int i=0; i<8; i++){
    Serial2.print((unsigned int) uuid[2*i], HEX);
  }
  Serial2.println("");

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  //Serial2.println("Here's the uninitializedEEPROM string that should be");
  //for(int i=0; i<8; i++){
    //Serial2.print((unsigned int) uninitializedEEPROM[2*i], HEX);
  //}
  //Serial2.println("");


  if(memcmp(uuid, uninitializedEEPROM, 16) == 0){
    Serial2.println(F("Generate UUID"));
    // generate the unique ID
    TrueRandomClass::uuid(uuid);
    for(int i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
      int address = uniqueIdAddressStart + i;
      EEPROM.write(address, uuid[i]);
    }
  }

}
*/

void initializeSDCard(void) {
  Wire.begin();
/*
  if (! RTC.initialized()) {
    Serial2.println(F("RTC is NOT initialized!"));
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!RTC.begin()) {
    Serial2.println(F(">RTC failed<"));
  } else {
    Serial2.println(F("RTC started"));
    //Serial2.println(RTC.now().m);
  }
*/
  SdFile::dateTimeCallback(dateTime);


  // initialize the SD card
  Serial2.print(F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(D13, OUTPUT);
  pinMode(PA5, OUTPUT);

  // see if the card is present and can be initialized:
  if (!sd.begin(chipSelect)) {
    Serial2.println(F("Card failed, or not present"));
    while(1);
  } else {
    Serial2.println(F("card initialized."));
  }

  setNewDataFile();

}

void initBLE(){
  //Serial1.println("Hello");
  Serial2.print(F("Initializing the Bluefruit LE module: "));
  //Serial3.println("Hello");

  bool success = false;
  success = ble.begin(true);

  Serial2.println("Tried to init");
  Serial2.println(success);

  if ( !success )
  {
    Serial2.print(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    // error
  }
  Serial2.println( F("OK!") );

/*
  if ( FACTORYRESET_ENABLE )
  {
    // Perform a factory reset to make sure everything is in a known state
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  */

  /* Disable command echo from Bluefruit */
  /*ble.echo(false);

  Serial2.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  //ble.info();

  //
  //
  //

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
  Serial2.begin(115200);
  while(!Serial2){
    delay(100);
  }
  Serial2.println("Setup");

  pinMode(PA5, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(PB3, OUTPUT);
  //pinMode(PB5, OUTPUT);



  //while (!Serial2);
  Serial2.println(F("Hello, world.  Primary Serial2."));

  //Serial2.begin(9600);

  //
  // init ble
  //
  initBLE();

  // readUniqueId();
  firstRun();

  /* Get the CD Card going */
  // initializeSDCard();

  /* We're ready to go! */
  Serial2.println(F("done with setup"));

  wake = true;
  // awakeTime = RTC.now().unixtime();
  burstCount = burstLength;

}

void printCurrentDirListing(){
  sd.vwd()->rewind();
  SdFile dirFile;
  char sdFilename[20];
  while (dirFile.openNext(sd.vwd(), O_READ)) {
    dir_t d;
    if (!dirFile.dirEntry(&d)) {
      Serial2.println(F("f.dirEntry failed"));
      while(1);
    }

    dirFile.getName(sdFilename, 20);
    Serial2.println(sdFilename);
  }

}

/**************************************************************************/
/*
Arduino loop function, called once 'setup' is complete (your own code
should go here)
*/
/**************************************************************************/

#define BUFSIZE                        160   // Size of the read buffer for incoming data

void loop(void)
{
  Serial2.println(F("Loop"));

  // Display command prompt
  Serial2.print(F("AT > "));

  // Check for user input and echo it back if anything was found
  //  char command[BUFSIZE+1];
  //  getUserInput(command, BUFSIZE);

  digitalWrite(PA5, 1);
  delay(1000);
  digitalWrite(PA5, 0);
  delay(1000);

  // Just trigger the dump event using a basic button
  //int buttonState = digitalRead(buttonPin);

  if(Serial2.peek() == '>' && state == 0){
    Serial2.println("Peek");

    wake = true;
    // awakeTime = RTC.now().unixtime(); // Keep us awake once we are talking to the phone

    char request[maxRequestLength] = "";
    Serial2.readBytesUntil('<', request, maxRequestLength);
    Serial2.write(">COMMAND RECIEVED: ");
    Serial2.write(&request[1]);
    Serial2.write("<");
    Serial2.flush();
    delay(100);

    if(strncmp(request, ">WT_OPEN_CONNECTION", 19) == 0) {
      Serial2.write(">VERSION:");
      Serial2.write(version);
      Serial2.write("<");
      Serial2.flush();
      delay(100);

      // DateTime now = RTC.now();
      char dateString[11];
      // sprintf(dateString, "%lu", now.unixtime());
      Serial2.print(">Datalogger Time: ");
      //Serial2.print(now.year());
      Serial2.print("-");
      //Serial2.print(now.month());
      Serial2.print("-");
      //Serial2.print(now.day());
      Serial2.print(" ");
      //Serial2.print(now.hour());
      Serial2.print(":");
      //Serial2.print(now.minute());
      Serial2.print(":");
      //Serial2.print(now.second());
      Serial2.print("<");
      delay(100);


      Serial2.write(">WT_IDENTIFY_DEVICE:");
      for(int i=0; i<8; i++){
        Serial2.print((unsigned int) uuid[2*i], HEX);
      }
      Serial2.write("<");
      Serial2.flush();

    }
    else if(strncmp(request, ">WT_REQUEST_DOWNLOAD",20) == 0) {
      // Flush the input, would be better to use a delimiter
      // May not be necessary now
      unsigned long now = millis ();
      while (millis () - now < 1000)
      Serial2.read ();  // read and discard any input

      if(request[20] == ':'){
        // we have a reference date
        strncpy(lastDownloadDate, &request[21], 10);
      }

      Serial2.print(">WT_TRANSFER_READY<");
      Serial2.flush();
      state = 1;
      return;
    } else if(strncmp(request, ">WT_SET_RTC:", 12) == 0){
      char UTCTime[11] = "0000000000";
      strncpy(UTCTime, &request[12], 10);
      //UTCTime[10] = '\0';
      long time = atol(UTCTime);
      delay(100);

      // RTC.adjust(DateTime(time));

      Serial2.print(">Received UTC time: ");
      Serial2.print(UTCTime);
      Serial2.print("---");
      Serial2.print(time);
      Serial2.print("---");
      // Serial2.print(RTC.now().unixtime());
      Serial2.print("<");
      Serial2.flush();

      setNewDataFile();
      return;

    } else {
      char lastDownloadDateEmpty[11] = "0000000000";
      strcpy(lastDownloadDate, lastDownloadDateEmpty);
    }

  } else if(state == 1){

    char ack[7] = "";
    Serial2.readBytesUntil('<', ack, 7);
    if(strcmp(ack, ">WT_OK") != 0) {
      char message[30] = "";
      sprintf(message, "ERROR #%s#", ack);
      Serial2.print(message);

      //Flush
      unsigned long now = millis ();
      while (millis () - now < 1000)
      Serial2.read ();  // read and discard any input

      state = 0;
      return;
    }


    // Debug
    //printCurrentDirListing();

    if (!sd.chdir("/Data")) {
      Serial2.println(F("chdir failed for Data."));
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
        Serial2.println(F("f.dirEntry failed"));
        while(1);
      }

      dirFile.getName(sdFilename, 20);

      if(strncmp(sdFilename, lastDownloadDate, 10) > 0){
        //Serial2.println(sdFilename);

        File datafile = sd.open(sdFilename);
        // send size of transmission ?
        // Serial2.println(datafile.fileSize());
        while (datafile.available()) {
          Serial2.write(datafile.read());
        }
        datafile.close();
      }
      dirFile.close();

    }
    if (!sd.chdir()) {
      Serial2.println(F("chdir failed for ../"));
      state = 0;
      return;
    }

    char transferCompleteMessage[34] = ">WT_TRANSFER_COMPLETE:0000000000<";
    strncpy(&transferCompleteMessage[22], sdFilename, 10);
    Serial2.write(transferCompleteMessage);

    setNewDataFile();

    // Send last download date to phone for book keeeping
    state = 0;

  }


  // Fetch the time
  // DateTime now = RTC.now();

  uint32_t trigger = 60*interval;
  uint32_t currentTime = 1; // now.unixtime();
  uint32_t elapsedTime = currentTime - lastTime;
  short minute = 0; //now.minute();

/*  For the moment we are skilling the timing stuff
    RTC example to use timing should be OK for the moment
  if(currentTime > awakeTime + 60 * 5){
    wake = false;
  }
  //Serial2.println(elapsedTime);
  if( ( elapsedTime < trigger
        || ( minute % interval != 0  )
      ) && ! (burstCount < burstLength) ){ // bursting clause
    if(!wake) {
      if(elapsedTime < trigger - 10){ // If we are withing ten secs of the trigger, don't sleep
        Serial2.println("power down");
        Serial2.flush();
    //    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      } else if (elapsedTime < trigger - 3){
    //    LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
      }
    }
    return;
  } else if (burstCount == burstLength) { // reinitialize bursting
    burstCount = 0;
  }
  //lastTime = now.unixtime();
*/

  // get the new data

  // log uuid and time

  for(int i=0; i<8; i++){
    logfile.print((unsigned int) uuid[2*i], HEX);
  }
  char comma[2] = ",";
  logfile.print(comma);
  //logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(comma);
  Serial2.print(currentTime);


  float value0 = analogRead(0); // * .0049;
  Serial2.print(" 0: ");
  Serial2.print(value0);
  logfile.print(value0);
  logfile.print(comma);

  float value1 = analogRead(1); // * .0049;
  Serial2.print("   1: ");
  Serial2.print(value1);
  logfile.print(value1);
  logfile.print(comma);

  float value2 = analogRead(2); // * .0049;
  Serial2.print("   2: ");
  Serial2.print(value2);
  logfile.print(value2);
  logfile.print(comma);

  float value3 = analogRead(3); // * .0049;
  //Serial2.println(value3);
  Serial2.print("   3: ");
  Serial2.print(value3);
  logfile.print(value3);
  logfile.print(comma);

  float value4 = analogRead(4); // * .0049;
  Serial2.print("   4: ");
  Serial2.print(value4);
  logfile.print(value4);
  logfile.print(comma);

  float value5 = analogRead(5); // * .0049;
  Serial2.print("   5: ");
  Serial2.println(value5);
  logfile.print(value5);
  logfile.print(comma);

  logfile.println();
  logfile.flush();

  // Get last line from logfile, or assemble the whole within
  // for the moment just sending A0
  ble.println("THEVALUE");


  burstCount = burstCount + 1;

}
