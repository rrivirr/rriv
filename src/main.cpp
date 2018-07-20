#include <Arduino.h>
#include <RTClock.h>
#include <Wire.h>  // Communicate with I2C/TWI devices
#include <SPI.h>
//#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "WaterBear_Control.h"
#include "WaterBear_FileSystem.h"

// For F103RM
//#define Serial Serial2

// Old code from Uno
//#include <EEPROM.h>
//#include "RTClib.h"
//#include "TrueRandom.h"
//#include "LowPower.h"


//RTC_PCF8523 RTC; // define the Real Time Clock object
//struct rtc_module rtc_instance;

RTClock rt (RTCSEL_LSE); // initialise
uint32 tt;

// Pin Mappings for Nucleo Board
#define D3 PB3 // Why isn't D3 working ??
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

void readDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i <= deploymentIdentifierAddressEnd - deploymentIdentifierAddressStart; i++){
    short address = deploymentIdentifierAddressStart + i;
    //deploymentIdentifier[i] = EEPROM.read(address);
    deploymentIdentifier[i] = '0';
  }

}

void writeDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i <= deploymentIdentifierAddressEnd - deploymentIdentifierAddressStart; i++){
    short address = deploymentIdentifierAddressStart + i;
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

void initBLE(){
  //Serial2.println("Hello");
  Serial2.print(F("Initializing the Bluefruit LE module: "));
  //Serial2.println("Hello");

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
    Serial2.println(F("Performing a factory reset: "));
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
  Serial2.begin(9600);
  while(!Serial2){
    delay(100);
  }
  Serial2.println("Setup");

  pinMode(PA5, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(PB3, OUTPUT);
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
  firstRun();

  Wire.begin();


  /* Get the CD Card going */
  // initializeSDCard();

  /* We're ready to go! */
  Serial2.println(F("done with setup"));

  wake = true;
  // awakeTime = RTC.now().unixtime();
  burstCount = burstLength;

  rt.setTime(1000);

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
    digitalWrite(PA5, 1);

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

  tt = rt.getTime();
  Serial2.println(tt);
  delay(1000);

  uint32_t trigger = 60*interval;
  uint32_t currentTime = tt; // now.unixtime();
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

  // Write the deployment identifier
  char comma[2] = ",";
  char deploymentIdentifier[29];// = "DEPLOYMENT";
  readDeploymentIdentifier(deploymentIdentifier);

  File logfile; // So we can skip this
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
  //logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(comma);
  Serial2.print(currentTime);


  // Get the new data
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

/*
  digitalWrite(PA5, 1);
  delay(1000);

  // Send along to BLE

  char valuesBuffer[52];
  sprintf(valuesBuffer, ">WT_VALUES:%7.4f,%7.4f,%7.4f,%7.4f,%7.4f,%7.4f<", value0, value1, value2, value3, value4, value5);
  Serial2.print(valuesBuffer);
  ble.println(valuesBuffer);

  digitalWrite(PA5, 0);
  delay(1000);
*/

  burstCount = burstCount + 1;

}
