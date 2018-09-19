#include <Arduino.h>
//#include <RTClock.h>
#include <Wire.h> // Communicate with I2C/TWI devices
#include <SPI.h>
//#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
//#include "RTClib.h"
#include "DS3231.h"

#include "Utilities.h"
#include "WaterBear_Control.h"
#include "WaterBear_FileSystem.h"

#include <libmaple/pwr.h>
#include <libmaple/scb.h>

// For F103RM
#define Serial Serial2

TwoWire WIRE1 (1, I2C_REMAP); // Need the I2C_REMAP when remapping... it's a hack they deprecated support for this
#define Wire WIRE1

// The DS3231 RTC chip
DS3231 Clock;
RTClib RTC;
#define ALRM1_MATCH_EVERY_SEC  0b1111  // once a second
#define ALRM1_MATCH_SEC        0b1110  // when seconds match
#define ALRM1_MATCH_MIN_SEC    0b1100  // when minutes and seconds match
#define ALRM1_MATCH_HR_MIN_SEC 0b1000  // when hours, minutes, and seconds match
//byte ALRM1_SET = ALRM1_MATCH_SEC;

#define ALRM2_ONCE_PER_MIN     0b111   // once per minute (00 seconds of every minute)
#define ALRM2_MATCH_MIN        0b110   // when minutes match
#define ALRM2_MATCH_HR_MIN     0b100   // when hours and minutes match
//byte ALRM2_SET = ALRM2_ONCE_PER_MIN;


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

short interval = 1; // minutes between loggings
short burstLength = 100; // how many readings in a burst

short uniqueIdAddressStart = 0;
short uniqueIdAddressEnd = 15;
unsigned char uuid[16];

short deploymentIdentifierAddressStart = 16;
short deploymentIdentifierAddressEnd =  43;

short fieldCount = 9;
char ** values;

#define BUFSIZE                        160   // Size of the read buffer for incoming data
unsigned long lastMillis = 0;


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

  if(false){
    Serial2.println("BLE First Run");
  }
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
  if(false){
    Serial2.print(F("Initializing the Bluefruit LE module: "));
  }
  bleActive = ble.begin(true);

  if(false){
    Serial2.println("Tried to init");
    Serial2.println(bleActive);
  }

  if ( !bleActive )
  {
    if(false){
      Serial2.print(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    return;

    // error
  } else {
    bleFirstRun();

  }
  if(false){
    Serial2.println( F("OK!") );
  }
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
  ble.info();

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

void setNextAlarm(){

  Clock.turnOffAlarm(1); // Clear the Control Register
  Clock.turnOffAlarm(2);
  Clock.checkIfAlarm(1); // Clear the Status Register
  Clock.checkIfAlarm(2);

  //
  // Alarm every 10 seconds for debugging
  //
  int AlarmBits = ALRM2_ONCE_PER_MIN;
  AlarmBits <<= 4;
  AlarmBits |= ALRM1_MATCH_SEC;
  short seconds = Clock.getSecond();
  short nextSeconds = (seconds + 10 - (seconds % 10)) % 60;
  Serial.print("Next Alarm");
  Serial.println(nextSeconds);
  Clock.setA1Time(0b0, 0b0, 0b0, nextSeconds, AlarmBits, true, false, false);

  //
  // Alarm every interval minutes for the real world
  //
  //int AlarmBits = ALRM2_ONCE_PER_MIN;
  //AlarmBits <<= 4;
  //AlarmBits |= ALRM1_MATCH_MIN_SEC;
  //short minutes = Clock.getMinute();
  //short nextMinutes = (minutes + interval - (minutes % interval)) % 60;
  //Serial.println(nextMinutes);
  //Clock.setA1Time(0b0, 0b0, nextMinutes, 0b0, AlarmBits, true, false, false);

  // set both alarms to :00 and :30 seconds, every minute
      // Format: .setA*Time(DoW|Date, Hour, Minute, Second, 0x0, DoW|Date, 12h|24h, am|pm)
      //                    |                                    |         |        |
      //                    |                                    |         |        +--> when set for 12h time, true for pm, false for am
      //                    |                                    |         +--> true if setting time based on 12 hour, false if based on 24 hour
      //                    |                                    +--> true if you're setting DoW, false for absolute date
      //                    +--> INTEGER representing day of the week, 1 to 7 (Monday to Sunday)
      //


  Clock.turnOnAlarm(1);
}

// Interrupt service routing for EXTI line
// Just clears out the interrupt, control will return to loop()
void timerAlarm(){

  NVIC_BASE->ICER[0] = 1 << NVIC_EXTI_9_5;
  EXTI_BASE->PR = 0x00000080; // this clear the interrupt on exti line
  NVIC_BASE->ICPR[0] = 1 << NVIC_EXTI_9_5;

  Serial2.println("TIMER ALAERM");
  NVIC_BASE->ISER[0] = 1 << NVIC_EXTI_9_5;

}



void setup(void)
{

  pinMode(D3, OUTPUT); // D2 and PB3 are the same
  pinMode(PB3, OUTPUT);
  pinMode(D4, OUTPUT);
  //pinMode(PB5, OUTPUT);
  pinMode(PC7, INPUT_PULLUP); // This is the interrupt line 7
  //pinMode(PA5, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  niiiiice.


  // Use remap of I2C1 so that it matches with the arduino sheild header
  AFIO_BASE->MAPR = AFIO_MAPR_I2C1_REMAP;

 // Start up Serial2
  Serial2.begin(9600);
  while(!Serial2){
    delay(100);
  }
  Serial2.println(F("Hello, world.  Primary Serial2.."));
  Serial2.println(F("Setup"));

  // Clear interrupts
  //exti_detach_interrupt(EXTI7);
  Serial.print("1: NVIC_BASE->ISPR ");
  Serial.println(NVIC_BASE->ISPR[0]);
  Serial.println(NVIC_BASE->ISPR[1]);
  Serial.println(NVIC_BASE->ISPR[2]);

  NVIC_BASE->ICER[0] =  1 << NVIC_EXTI_9_5; // Don't respond to interrupt during setup
  EXTI_BASE->PR = 0x00000080; // this clear the interrupt on exti line
  NVIC_BASE->ICPR[0] = 1 << NVIC_EXTI_9_5; // Clear any pending interrupts

  Serial.print("2: NVIC_BASE->ISPR ");
  Serial.println(NVIC_BASE->ISPR[0]);
  Serial.println(NVIC_BASE->ISPR[1]);
  Serial.println(NVIC_BASE->ISPR[2]);

  delay(200);

  //  Prepare I2C
  Wire.begin();
  scanIC2();
  Serial.println("OKOK");

  // Clear the alarms so they don't go off during setup
  Clock.turnOffAlarm(1);
  Clock.turnOffAlarm(2);
  Clock.checkIfAlarm(1); // Clear the Status Register
  Clock.checkIfAlarm(2);
  Serial.println("OKOK");

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
  Serial2.flush();


  exti_attach_interrupt(EXTI7, EXTI_PC, timerAlarm, EXTI_FALLING);

/*
  AFIO_BASE->EXTICR2 = 0x2000; // PC
  EXTI_BASE->IMR = 0x00000080; // turn on line #7
  EXTI_BASE->FTSR = 0x00000080; // detect falling edge of line #7

  //EXTI_BASE->RTSR = 0x00000200; // detect falling edge of line #9
  //EXTI_BASE->IMR =  0x000FFFFF; // tsurn on all
  //EXTI_BASE->FTSR = 0x000FFFFF; // detect falling edge

  // EXTI9_5  for NVIC
  //NVIC_EXTI_9_5
  Serial.println(AFIO_BASE->EXTICR3);
  Serial.println(EXTI_BASE->IMR);
  Serial.println(EXTI_BASE->FTSR);
  Serial.println(EXTI_BASE->RTSR);

  //EXTI_BASE->SWIER = 0x00000009; // Just for testing

  NVIC_BASE->ISER[0] = 1 << NVIC_EXTI_9_5;   //NVIC_EXTI_9_5; // this sets the enabled interrupts
  Serial2.flush();
  Serial2.println( (1 << NVIC_EXTI_9_5) );
  Serial.print("OK: ");
  Serial.println(NVIC_BASE->ISER[0]);
  Serial2.flush();


  //Serial.println(NVIC_BASE->ISER[0]);
  NVIC_BASE->ICPR[0] = 1 << NVIC_EXTI_9_5;
  //Serial.println(NVIC_BASE->ISPR[0]);
  //Serial.println("ok");
  */

  setNextAlarm();

  /* We're ready to go! */
  Serial2.println(F("done with setup"));

}

void printDateTime(DateTime now){
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

}




void loop(void)
{
  //Serial2.println(F("Loop"));

  DateTime now = RTC.now();

  unsigned long currentMillis = millis();
  if(currentMillis - lastMillis >= 1000){
    lastMillis = currentMillis;
  } else {
    return;
  }

  printDateTime(now);

     //EXTI_BASE->SWIER = 0x100;
     /*
     Serial.println(AFIO_BASE->EXTICR3);
     Serial.println(EXTI_BASE->IMR);
     Serial.println(EXTI_BASE->FTSR);
     Serial.println(EXTI_BASE->RTSR);
     */

/*     Serial.print("EXTI_BASE->PR ");
     Serial.println(EXTI_BASE->PR);
     Serial.print("NVIC_BASE->ISPR ");
     Serial.println(NVIC_BASE->ISPR[0]);
     Serial.println(NVIC_BASE->ISPR[1]);
     Serial.println(NVIC_BASE->ISPR[2]);
     Serial.flush();
*/

   if(Clock.checkIfAlarm(1)){
      Serial.println("Alarm 1");
      setNextAlarm();
    }


    Serial.print("1: NVIC_BASE->ISPR ");
    Serial.println(NVIC_BASE->ISPR[0]);
    Serial.println(NVIC_BASE->ISPR[1]);
    Serial.println(NVIC_BASE->ISPR[2]);
    Serial.println(EXTI_BASE->PR);
    NVIC_BASE->ICPR[0] = NVIC_BASE->ISPR[0];
    Serial.print("1: NVIC_BASE->ISPR ");
    Serial.println(NVIC_BASE->ISPR[0]);
    Serial.println(NVIC_BASE->ISPR[1]);
    Serial.println(NVIC_BASE->ISPR[2]);

    Serial.print("Enabled: ");
    Serial.println(NVIC_BASE->ISER[0]);
    Serial.println(NVIC_BASE->ISER[1]);
    Serial.println(NVIC_BASE->ISER[2]);
    Serial.flush();

    int iser1 = NVIC_BASE->ISER[0];
    int iser2 = NVIC_BASE->ISER[1];
    int iser3 = NVIC_BASE->ISER[2];

    NVIC_BASE->ICER[0] = NVIC_BASE->ISER[0];
    NVIC_BASE->ICER[1] = NVIC_BASE->ISER[1];
    NVIC_BASE->ICER[2] = NVIC_BASE->ISER[2];
    NVIC_BASE->ISER[0] = 1 << NVIC_EXTI_9_5;

    NVIC_BASE->ICPR[0] = NVIC_BASE->ISPR[0];
    NVIC_BASE->ICPR[1] = NVIC_BASE->ISPR[1];
    NVIC_BASE->ICPR[2] = NVIC_BASE->ISPR[2];

    if(true) { // STOP mode WIP
      PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

      PWR_BASE->CR |= PWR_CR_CWUF;
      PWR_BASE->CR |= PWR_CR_PDDS; // Enter stop/standby mode when cpu goes into deep sleep
      // PWR_BASE->CR |= PWR_CR_LPDS; // Puts voltage regulator in low power mode.  This seems to cause problems

      PWR_BASE->CR &= ~PWR_CR_PDDS; // Also have to unset this to get into STOP mode
      SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

      // PWR_BASE->CR |=  PWR_CSR_EWUP;   // Enable wakeup pin bit.  This is for wake from the WKUP pin specifically

      SCB_BASE->SCR &= ~SCB_SCR_SLEEPONEXIT;

      __asm volatile( "wfi" );

    } else { // SLEEP mode

      __asm volatile( "dsb" );
      systick_disable();
      __asm volatile( "wfi" );
      systick_enable();
    }
    //__asm volatile( "isb" );

    NVIC_BASE->ISER[0] = iser1;
    NVIC_BASE->ISER[1] = iser2;
    NVIC_BASE->ISER[2] = iser3;


    Serial.println("YES");
    Serial.flush();


    /*PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;
    PWR_BASE->CR |= PWR_CR_CWUF;
    PWR_BASE->CR |= PWR_CR_PDDS;
    PWR_BASE->CR |=  PWR_CSR_EWUP;   // Enable wakeup pin bit.
    PWR_BASE->CR &= ~PWR_CR_PDDS;  //  Unset Power down deepsleep bit.
*/

/*
  // Clear PDDS and LPDS bits
  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

  // Set PDDS and LPDS bits for standby mode, and set Clear WUF flag (required per datasheet):
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_PDDS;

  // Enable wakeup pin bit.
  PWR_BASE->CR |=  PWR_CSR_EWUP;

  //  Unset Power down deepsleep bit.
  PWR_BASE->CR &= ~PWR_CR_PDDS;

  // set Low-power deepsleep.
  PWR_BASE->CR |= PWR_CR_LPDS;

  // Unset sleepdeep in the system control register - if not set then we only sleep and can wake from RTC or pin interrupts.
  //SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP; // This causes problems, ref: https://www.stm32duino.com/viewtopic.php?t=658
  // Low-power deepsleep bit.

  // This should be 'sleep mode'.  Stop mode is different
  //SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;
  //SCB_BASE->SCR &= ~SCB_SCR_SLEEPONEXIT;
*/

  // ok cool!!
  // now we stop when the nvic interrupt comes through
  // which means we have to handle it and clear it, because we are interrupted.
  //asm("wfi");

  // wow look at this, actually handles it all for you dummy.
  // arrggghghghghg  this handles all that BS you just spent a day on
  // exti_attach_interrupt(exti_num num, exti_cfg port, voidFuncPtr handler, exti_trigger_mode mode)
  // exti_attach_callback(exti_num num, exti_cfg port, voidArgumentFuncPtr handler, void *arg, exti_trigger_mode mode)


  return;

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

        // TODO


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
