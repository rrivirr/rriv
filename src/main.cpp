#include <Arduino.h>
//#include <RTClock.h>
#include <Wire.h> // Communicate with I2C/TWI devices
#include <SPI.h>
//#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
//#include "RTClib.h"
#include "DS3231.h"

#include "WaterBear_Control.h"
#include "WaterBear_FileSystem.h"

// For F103RM
#define Serial Serial2

TwoWire WIRE1 (1, I2C_REMAP); // Need the I2C_REMAP when remapping... it's a hack they deprecated support for this
#define Wire WIRE1

// The DS3231 RTC chip
DS3231 Clock;
RTClib RTC;

// The internal RTC
//RTClock rt (RTCSEL_LSE); // initialise
uint32 tt;

// Pin Mappings for Nucleo Board
#define D3 PB3
#define D4 PB5

int bluefruitModePin = D4;
Adafruit_BluefruitLE_UART ble(Serial1, bluefruitModePin);

WaterBear_FileSystem * filesystem;

char lastDownloadDate[11] = "0000000000";

char version[5] = "v2.0";

short interval = 15; // minutes between loggings
short burstLength = 100; // how many readings in a burst

short uniqueIdAddressStart = 0;
short uniqueIdAddressEnd = 15;
unsigned char uuid[16];

short deploymentIdentifierAddressStart = 16;
short deploymentIdentifierAddressEnd =  43;

short fieldCount = 9;
char ** values;

void readDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i <= deploymentIdentifierAddressEnd - deploymentIdentifierAddressStart; i++){
    //short address = deploymentIdentifierAddressStart + i;
    //deploymentIdentifier[i] = EEPROM.read(address);
    deploymentIdentifier[i] = '\0';
  }

}

void writeDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i <= deploymentIdentifierAddressEnd - deploymentIdentifierAddressStart; i++){
    //short address = deploymentIdentifierAddressStart + i;
    //EEPROM.write(address, deploymentIdentifier[i]);
  }
}

uint32_t lastTime = 0;
short burstCount = 0;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial2.println("Error:");
  Serial2.println(err);
  while (1);
}



/*
* Initialize the SD Card
*/


// TODO: we will define chip specific callbacks so that classes are not referencing specific RTC
void dateTime(uint16_t* date, uint16_t* time) {
  //DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  //*date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  //*time = FAT_TIME(now.hour(), now.minute(), now.second());
}



void bleFirstRun(){

  // if we don't have a UUID yet, we are running for the first time
  // set a mode pin for USART1 if we need to

  Serial2.println("BLE First Run");
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

bool bleActive = false;

void initBLE(){
  //Serial2.println("Hello");
  Serial2.print(F("Initializing the Bluefruit LE module: "));
  //Serial2.println("Hello");

  bleActive = ble.begin(true);

  Serial2.println("Tried to init");
  Serial2.println(bleActive);

  if ( !bleActive )
  {
    Serial2.print(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    // error
  } else {
    bleFirstRun();

  }
  Serial2.println( F("OK!") );

/*
  if ( FACTORYRESET_ENABLE )
  {
    // Perform a factory reset to make sure everything is in a known state
    Serial2.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  */

  /* Disable command echo from Bluefruit */
  //ble.echo(false);

  Serial2.println("Requesting Bluefruit info:");
  // Print Bluefruit information
  //ble.info();

  //
  //
  //
}



void setupRTC(){
/*  Serial2.println("Start RTC");

  if (! rtc.begin()) {
    Serial2.println("Couldn't find RTC");
    while (1);
  }
*/

  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


// Time will be set during setup by Android / Serial1
// No need to set the time from the script build time.
/*
  if (rtc.lostPower()) {
    Serial2.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
  }
*/
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

  // Use remap of I2C1 so that it matches with the arduino sheild header
  AFIO_BASE->MAPR = AFIO_MAPR_I2C1_REMAP;

  Serial2.begin(9600);
  while(!Serial2){
    delay(100);
  }
  Serial2.println("Setup");

  //  Prepare I2C
  Wire.begin();

  // Set up the realtime clock
  setupRTC();


  //pinMode(PA5, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  nice.

  pinMode(D3, OUTPUT); // D2 and PB3 are the same
  pinMode(PB3, OUTPUT);

  pinMode(D4, OUTPUT);
  //pinMode(PB5, OUTPUT);


  Serial2.println(F("Hello, world.  Primary Serial2.."));

  //
  // init filesystem
  //

  filesystem = new WaterBear_FileSystem();

  //
  // init ble
  //
  initBLE();

  // readUniqueId();

  wake = true;
  // awakeTime = RTC.now().unixtime();

  burstCount = burstLength;


  //
  // Allocate needed memory
  //
  values = (char **) malloc(sizeof(char *) * fieldCount);
  for(int i = 3; i < 3+fieldCount; i++){
      values[i] = (char *) malloc(sizeof(char) * 5);
  }

  //rt.setTime(1000);

  /* We're ready to go! */
  Serial2.println(F("done with setup"));

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

  DateTime now = RTC.now();
  Serial.println(now.unixtime());

     Serial.print(now.year(), DEC);
     Serial.print('/');
     Serial.print(now.month(), DEC);
     Serial.print('/');
     Serial.print(now.day(), DEC);
    // Serial.print(" (");
    // Serial.print(now.dayOfTheWeek());
    // Serial.print(") ");
     Serial.print(now.hour(), DEC);
     Serial.print(':');
     Serial.print(now.minute(), DEC);
     Serial.print(':');
     Serial.print(now.second(), DEC);
     Serial.println();


  // Display command prompt

  // Check for user input and echo it back if anything was found
  //  char command[BUFSIZE+1];
  //  getUserInput(command, BUFSIZE);

  // Just trigger the dump event using a basic button
  //int buttonState = digitalRead(buttonPin);

  /*char request[30] = "";
  Serial2.readBytesUntil('<', request, 30);
  Serial2.write(">RECIEVED: ");
  Serial2.write(request);
  Serial2.println("<");
*/

  if( WaterBear_Control::ready(Serial2) ){
    //digitalWrite(PA5, 1);

    wake = true;
    WaterBear_Control::processControlCommands(Serial2);
    return;
  } else if(WaterBear_Control::ready(ble) ){
    wake = true;
    WaterBear_Control::processControlCommands(ble);
    return;
  }

  // Fetch the time
  // DateTime now = RTC.now();

  delay(1000);

  //tt = rt.getTime();
  tt = 0;
  Serial2.println(tt);

  uint32_t trigger = 60*interval;
  uint32_t currentTime = now.unixtime();
  uint32_t elapsedTime = currentTime - lastTime;
  short minute = now.minute();

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
  lastTime = now.unixtime();

  // Write the deployment identifier
  char deploymentIdentifier[29];// = "DEPLOYMENT";
  readDeploymentIdentifier(deploymentIdentifier);


  values[0] = deploymentIdentifier; // TODO: change to deploymentIdentifier_UUID

  /*logfile.print(deploymentIdentifier);
  logfile.write("_");
  for(short i=0; i<8; i++){
    logfile.print((unsigned int) uuid[2*i], HEX);
  }
  logfile.print(comma);
  */

  // Log uuid and time
  char uuid[9];
  for(short i=0; i<8; i++){
    //logfile.print((unsigned int) uuid[2*i], HEX);
    sprintf(&uuid[i], "%02x", (unsigned int) uuid[2*i]);
  }
  uuid[8] = '\0';
  values[1] = uuid;

  Serial2.println(currentTime);
  char timeString[11];
  sprintf(timeString, "%lu", currentTime);
  values[2] = timeString;
  Serial2.println(timeString);


  // Get the new data
  short sensorCount = 6;
  for(short i=0; i<sensorCount; i++){
    int value = analogRead(i);
    // malloc or ?
    sprintf(values[3+i], "%4d", value);

    Serial2.print(" ");
    Serial2.print(i);
    Serial2.print(": ");
    Serial2.println(value);
  }

  Serial2.println("writeLog");
  filesystem->writeLog(values, fieldCount);
  Serial2.println("writeLog done");

  //digitalWrite(PA5, 1);
  //delay(1000);

  // Send along to BLE

  if(bleActive) {
    char valuesBuffer[52];
    sprintf(valuesBuffer, ">WT_VALUES:%s,%s,%s,%s,%s,%s<", values[3], values[4], values[5], values[6], values[7], values[8]);
    Serial2.print(valuesBuffer);
    ble.println(valuesBuffer);
  }

  //digitalWrite(PA5, 0);
  //delay(1000);

  burstCount = burstCount + 1;

}
