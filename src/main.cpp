#include <Arduino.h>
//#include <RTClock.h>
#include <Wire.h> // Communicate with I2C/TWI devices
#include <SPI.h>
#include "plotter/Plotter.h"
//#include <SoftwareSerial.h>                           //we have to include the SoftwareSerial library, or else we can't use it.

#include "Adafruit_BluefruitLE_SPI.h"
//#include "Adafruit_BluefruitLE_UART.h"
#include "DS3231.h"
#include "SdFat.h"
#include "STM32-UID.h"

#include "Utilities.h"
#include "WaterBear_Control.h"
#include "WaterBear_FileSystem.h"

#include <libmaple/pwr.h>
#include <libmaple/scb.h>

#define DEBUG_MESSAGES false
#define DEBUG_BLE false
#define DEBUG_MEASUREMENTS false
#define DEBUG_LOOP true

// For F103RB
#define Serial Serial2

TwoWire WIRE1 (1); // Need the I2C_REMAP when remapping... it's a hack they deprecated support for this
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

#define EEPROM_I2C_ADDRESS 0x50

#define EEPROM_UUID_ADDRESS_START 0
#define EEPROM_UUID_ADDRESS_END 15
#define UUID_LENGTH 12 // STM32 has a 12 byte UUID, leave extra space for the future

#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START 16
#define EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_END   43
#define DEPLOYMENT_IDENTIFIER_LENGTH 25

//#define rx PA11                                          //define what pin rx is going to be.
//#define tx PA12                                          //define what pin tx is going to be.
//SoftwareSerial myserial(rx, tx);

String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_stringcomplete = false;                 //have we received all the data from the PC
boolean sensor_stringcomplete = false;                //have we received all the data from the Atlas Scientific product
float ORP;                                            //used to hold a floating point number that is the ORP.

float x; // global variables
float x1; // global variables
float x2; // global variables
Plotter p; // create plotter

unsigned char uuid[UUID_LENGTH];

// The internal RTC
//RTClock rt (RTCSEL_LSE); // initialise
uint32 tt;

// Pin Mappings for Nucleo Board

// BLE USART
//#define D4 PB5
//int bluefruitModePin = D4;
//Adafruit_BluefruitLE_UART ble(Serial1, bluefruitModePin);

// Bluefruit on SPI
#define BLUEFRUIT_SPI_SCK   PB13
#define BLUEFRUIT_SPI_MISO  PB14
#define BLUEFRUIT_SPI_MOSI  PB15

// Pullup
#define BLUEFRUIT_SPI_CS    PB8

#define BLUEFRUIT_SPI_IRQ   PB9
#define BLUEFRUIT_SPI_RST   PC4

//SPIClass SPI_2(2); //Create an SPI2 object.  This has been moved to a tweak on Adafruit_BluefruitLE_SPI
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


WaterBear_FileSystem * filesystem;

char lastDownloadDate[11] = "0000000000";

char version[5] = "v2.0";

short interval = 1; // minutes between loggings
short burstLength = 10; // how many readings in a burst


short fieldCount = 9;
char ** values;

#define BUFSIZE                        160   // Size of the read buffer for incoming data
unsigned long lastMillis = 0;

bool awakenedByUser;
uint32_t awakeTime = 0;
#define USER_WAKE_TIMEOUT           60 * 5 // Timeout after wakeup from user interaction, seconds

unsigned int interactiveModeMeasurementDelay = 10;



void readDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i < DEPLOYMENT_IDENTIFIER_LENGTH; i++){
    short address = EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START + i;
    deploymentIdentifier[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
  }
  deploymentIdentifier[DEPLOYMENT_IDENTIFIER_LENGTH] = '\0';
}

void writeDeploymentIdentifier(char * deploymentIdentifier){
  for(short i=0; i < DEPLOYMENT_IDENTIFIER_LENGTH; i++){
    short address = EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START + i;
    writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address, deploymentIdentifier[i]);
  }
}

uint32_t lastTime = 0;
short burstCount = 0;

// A small helper
void error(const __FlashStringHelper*err) {
   if(DEBUG_MESSAGES) {
        Serial2.println("Error:");
        Serial2.println(err);
    }
  while (1);
}


void serialEvent() {                                  //if the hardware serial port_0 receives a char
  char inchar = (char)Serial.read();                  //get the char we just received
  inputstring += inchar;                              //add it to the inputString
  if (inchar == '\r') {
    input_stringcomplete = true;                      //if the incoming character is a <CR>, set the flag
  }
}



void bleFirstRun(){

  // if we don't have a UUID yet, we are running for the first time
  // set a mode pin for USART1 if we need to

  if(DEBUG_MESSAGES) {
    Serial2.println("BLE First Run");
  }
  //ble.factoryReset();

  //ble.setMode(BLUEFRUIT_MODE_COMMAND);
  //digitalWrite(D4, HIGH);

  ble.println(F("AT"));
  if(ble.waitForOK()){
      if(DEBUG_MESSAGES) {
          Serial2.println("OK");
          Serial2.flush();
      }
  } else {
      if(DEBUG_MESSAGES) {
          Serial2.println("Not OK");
          Serial2.flush();
      }
  }

  // Send command
  ble.println(F("AT+GAPDEVNAME=WaterBear3"));
  if(ble.waitForOK()){
      if(DEBUG_MESSAGES) {
          Serial2.println("Got OK");
          Serial2.flush();
      }
  } else {
      Serial2.println("BLE Error");
      Serial2.flush();
      while(1);
  }
  ble.println(F("ATZ"));
  ble.waitForOK();
  if(DEBUG_MESSAGES) {
      Serial2.println("Got OK");
      Serial2.flush();
  }

//  ble.setMode(BLUEFRUIT_MODE_DATA);

}

void readUniqueId(){

  for(int i=0; i < UUID_LENGTH; i++){
    unsigned int address = EEPROM_UUID_ADDRESS_START + i;
    uuid[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
  }

  if(DEBUG_MESSAGES) {
      Serial2.println(F("OK.. UUID in EEPROM:"));
  }
  // Log uuid and time
  // TODO: this is confused.  each byte is 00-FF, which means 12 bytes = 24 chars in hex
  char uuidString[2 * UUID_LENGTH + 1];
  uuidString[2 * UUID_LENGTH] = '\0';
  for(short i=0; i < UUID_LENGTH; i++){
      sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
  }
  if(DEBUG_MESSAGES) {
      Serial2.println(uuidString);
  }

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

  if(memcmp(uuid, uninitializedEEPROM, UUID_LENGTH) == 0){
      if(DEBUG_MESSAGES) {
          Serial2.println(F("Generate or Retrieve UUID"));
      }
      getSTM32UUID(uuid);

    if(DEBUG_MESSAGES) {
        Serial2.println(F("UUID to Write:"));
    }
    char uuidString[2 * UUID_LENGTH + 1];
    uuidString[2 * UUID_LENGTH] = '\0';
    for(short i=0; i < UUID_LENGTH; i++){
        sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }
    if(DEBUG_MESSAGES) {
        Serial2.println(uuidString);
        Serial2.flush();
    }

    for(int i=0; i < UUID_LENGTH; i++){
      unsigned int address = EEPROM_UUID_ADDRESS_START + i;
      writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address, uuid[i]);
    }

    for(int i=0; i < UUID_LENGTH; i++){
      unsigned int address = EEPROM_UUID_ADDRESS_START + i;
      uuid[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
    }

    if(DEBUG_MESSAGES) {
        Serial2.println(F("UUID in EEPROM:"));
    }
    for(short i=0; i < UUID_LENGTH; i++){
        sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }
    if(DEBUG_MESSAGES) {
        Serial2.println(uuidString);
        Serial2.flush();
    }

   }

}


bool bleActive = false;

void initBLE(){
  bool debugBLE = DEBUG_BLE;
  if(debugBLE){
    Serial2.print(F("Initializing the Bluefruit LE module: "));
  }
  bleActive = ble.begin(true, true);

  if(debugBLE){
    Serial2.println("Tried to init");
    Serial2.println(bleActive);
  }

  if ( !bleActive )
  {
    if(debugBLE){
      Serial2.print(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    return;

    // error
  } else {
      if(debugBLE){
          Serial2.println(F("Performing a factory reset: "));
      }
      if ( ! ble.factoryReset() ){
        error(F("Couldn't factory reset"));
      }

      ble.println(F("AT"));
      if(ble.waitForOK()){
          if(debugBLE){
              Serial2.println("AT OK");
              Serial2.flush();
          }
      } else {
          if(debugBLE){
           Serial2.println("AT NOT OK");
           Serial2.flush();
          }
      }

    /*Serial2.println(F("Performing a factory reset: "));
      if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
*/

    bleFirstRun();


  }

  if(debugBLE){
    Serial2.println( F("BLE OK!") );
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

  ble.println("+++\r\n");

}

void dateTime(uint16_t* date, uint16_t* time) {

  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}



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
  short debugSleepSeconds = 30;
  short nextSeconds = (seconds + debugSleepSeconds - (seconds % debugSleepSeconds)) % 60;
  if(DEBUG_MEASUREMENTS){
    Serial2.print("Next Alarm");
    Serial2.println(nextSeconds);
  }
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


void clearTimerInterrupt(){
    EXTI_BASE->PR = 0x00000080; // this clear the interrupt on exti line
    NVIC_BASE->ICPR[0] = 1 << NVIC_EXTI_9_5;
}

void disableTimerInterrupt(){
    NVIC_BASE->ICER[0] = 1 << NVIC_EXTI_9_5;
}

void enableTimerInterrupt(){
    NVIC_BASE->ISER[0] = 1 << NVIC_EXTI_9_5;
}

void enableUserInterrupt(){
    NVIC_BASE->ISER[1] = 1 << (NVIC_EXTI_15_10-32);
}

void clearUserInterrupt(){
    EXTI_BASE->PR = 0x00000400; // this clear the interrupt on exti line
    NVIC_BASE->ICPR[1] = 1 << (NVIC_EXTI_15_10-32);
}

void disableUserInterrupt(){
    NVIC_BASE->ICER[1] = 1 << (NVIC_EXTI_15_10-32); // it's on EXTI 10
}


// Interrupt service routing for EXTI line
// Just clears out the interrupt, control will return to loop()

void timerAlarm(){

  disableTimerInterrupt();
  clearTimerInterrupt();
  Serial2.println("TIMER ALARM");
  enableTimerInterrupt();

}

void userTriggeredInterrupt(){

    disableUserInterrupt();
    clearUserInterrupt();
    Serial2.println("USER TRIGGERED INTERRUPT");
    enableUserInterrupt();
    awakenedByUser = true;

}



void setup(void)
{

    // Start up Serial2
    // Need to do an if(Serial2) after an amount of time, just disable it
    // Note that this is double the actual BAUD due to HSI clocking of processor
     Serial2.begin(115200);
     while(!Serial2){
       delay(100);
     }
     if(DEBUG_MESSAGES){
         Serial2.println(F("Hello, world.  Primary Serial2.."));
         Serial2.println(F("Setup"));
         Serial2.flush();
     }

  //pinMode(PB5, OUTPUT); // Command Mode pin for BLE

  pinMode(PC7, INPUT_PULLUP); // This the interrupt line 7
  pinMode(PB10, INPUT_PULLDOWN); // This is interrupt line 10, user interrupt

  pinMode(PB1, INPUT_ANALOG);
  pinMode(PC0, INPUT_ANALOG);
  pinMode(PC1, INPUT_ANALOG);
  pinMode(PC2, INPUT_ANALOG);
  pinMode(PC3, INPUT_ANALOG);


  //pinMode(PA5, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  niiiiice.

  // Set up global date time callback for SdFile
  SdFile::dateTimeCallback(dateTime);

  // Use remap of I2C1 so that it matches with the arduino sheild header
  // AFIO_BASE->MAPR = AFIO_MAPR_I2C1_REMAP;

  // Clear interrupts
  if(DEBUG_MESSAGES){
      Serial2.print("1: NVIC_BASE->ISPR ");
      Serial2.println(NVIC_BASE->ISPR[0]);
      Serial2.println(NVIC_BASE->ISPR[1]);
      Serial2.println(NVIC_BASE->ISPR[2]);
  }

  NVIC_BASE->ICER[0] =  1 << NVIC_EXTI_9_5; // Don't respond to interrupt during setup
  //NVIC_BASE->ICER[0] =  1 << NVIC_EXTI3; // Don't respond to interrupt during setup


  clearTimerInterrupt();
  clearUserInterrupt();

  if(DEBUG_MESSAGES){
  Serial2.print("2: NVIC_BASE->ISPR ");
  Serial2.println(NVIC_BASE->ISPR[0]);
  Serial2.println(NVIC_BASE->ISPR[1]);
  Serial2.println(NVIC_BASE->ISPR[2]);
  }

  //  Prepare I2C
  Wire.begin();
  if(DEBUG_MESSAGES) {
        scanIC2(&Wire);
  }

  // Clear the alarms so they don't go off during setup
  Clock.turnOffAlarm(1);
  Clock.turnOffAlarm(2);
  Clock.checkIfAlarm(1); // Clear the Status Register
  Clock.checkIfAlarm(2);


  //
  // init filesystem
  //
  char defaultDeployment[25] = "SITENAME_00000000000000";
  char * deploymentIdentifier = defaultDeployment;

  // get any stored deployment identifier from EEPROM
  readDeploymentIdentifier(deploymentIdentifier);
  unsigned char empty[1] = {0xFF};
  if(memcmp(deploymentIdentifier, empty, 1) == 0 ) {
    //Serial2.print(">NoDplyment<");
    //Serial2.flush();

    writeDeploymentIdentifier(defaultDeployment);
    readDeploymentIdentifier(deploymentIdentifier);                          //set aside some bytes for receiving data from Atlas Scientific product
  }

  DateTime now3 = RTC.now();
  if(DEBUG_MESSAGES){
    Serial2.println(now3.unixtime());
    Serial2.flush();
  }


  // SS is on PC6 for now
  filesystem = new WaterBear_FileSystem(deploymentIdentifier, PC8);
  if(DEBUG_MESSAGES){
  Serial2.println("Filesystem started OK");
  }

  DateTime now2 = RTC.now();
  if(DEBUG_MESSAGES){
  Serial2.println(now2.unixtime());
  Serial2.flush();
  }

  filesystem->setNewDataFile(RTC.now().unixtime());


  //
  // init ble
  //

  //initBLE();

  readUniqueId();

  burstCount = burstLength;  // Set to not bursting

  //
  // Allocate needed memory
  //
  values = (char **) malloc(sizeof(char *) * fieldCount);
  for(int i = 3; i < 3+fieldCount; i++){
      values[i] = (char *) malloc(sizeof(char) * 5);
      sprintf(values[i], "%4d", 0);
  }
  if(DEBUG_MESSAGES){
      Serial2.flush();
  }

  exti_attach_interrupt(EXTI7, EXTI_PC, timerAlarm, EXTI_FALLING);
  awakenedByUser = false;
  exti_attach_interrupt(EXTI10, EXTI_PB, userTriggeredInterrupt, EXTI_RISING);

  /* We're ready to go! */
  if(DEBUG_MESSAGES){
      Serial2.println(F("done with setup"));
  }
  Serial1.begin(9600);                               //set baud rate for software serial port_3 to 9600
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);
  p.Begin(); // start plotter
  p.AddTimeGraph( "Some title of a graph", 50, "label for x", ORP); // add any graphs you want

}

void printDateTime(DateTime now){
    if(!DEBUG_MESSAGES){
        return;
    }
    Serial2.println(now.unixtime());

     Serial2.print(now.year(), DEC);
     Serial2.print('/');
     Serial2.print(now.month(), DEC);
     Serial2.print('/');
     Serial2.print(now.day(), DEC);
     // Serial.print(" (");
     // Serial.print(now.dayOfTheWeek());
     // Serial.print(") ");
     Serial2.print("  ");
     Serial2.print(now.hour(), DEC);
     Serial2.print(':');
     Serial2.print(now.minute(), DEC);
     Serial2.print(':');
     Serial2.print(now.second(), DEC);
     Serial2.println();

}


void prepareForTriggeredMeasurement(){
    burstCount = 0;
}

void measureSensorValues(){

    // Fetch the time
    unsigned long currentTime = RTC.now().unixtime();

    // TODO: do we need to do this every time ??
    char uuidString[2 * UUID_LENGTH + 1];
    uuidString[2 * UUID_LENGTH] = '\0';
    for(short i=0; i < UUID_LENGTH; i++){
        sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }


    // Get the deployment identifier
    // TODO: do we need to do this every time ??
    char deploymentIdentifier[29];// = "DEPLOYMENT";
    readDeploymentIdentifier(deploymentIdentifier);
    char deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH + 2*UUID_LENGTH + 2];
    memcpy(deploymentUUID, deploymentIdentifier, DEPLOYMENT_IDENTIFIER_LENGTH);
    deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH] = '_';

    memcpy(&deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH+1], uuidString, 2*UUID_LENGTH);
    deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH + 2*UUID_LENGTH] = '\0';
    values[0] = deploymentUUID; // TODO: change to deploymentIdentifier_UUID

    // Log uuid and time
    values[1] = uuidString;

    //Serial2.println(currentTime);
    char timeString[11];
    sprintf(timeString, "%lu", currentTime);
    values[2] = timeString;
    if(DEBUG_MESSAGES){
       Serial2.println(timeString);
    }

    // Measure the new data
    short sensorCount = 5;
    short sensorPins[5] = {PB1, PC1, PC2, PC3, PC4};
    for(short i=0; i<sensorCount; i++){
        //int value = analogRead(i);  // NO WAY!  A2 in ADC12_IN9, on PB1
        int value = analogRead(sensorPins[i]);  // NO WAY!  A2 in ADC12_IN9, on PB1

        // malloc or ?
        sprintf(values[3+i], "%4d", value);

        //Serial2.print(" ");
        //Serial2.print(i);
        //Serial2.print(": ");
        //Serial2.println(value);
    }


      if (input_stringcomplete) {                         //if a string from the PC has been received in its entirety
        Serial1.print(inputstring);                      //send that string to the Atlas Scientific product
        inputstring = "";                                 //clear the string
        input_stringcomplete = false;                     //reset the flag used to tell if we have received a completed string from the PC
      }

      if (Serial1.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character.
        char inchar = (char)Serial1.read();              //get the char we just received
        sensorstring += inchar;
        if (inchar == '\r') {
          sensor_stringcomplete = true;                   //if the incoming character is a <CR>, set the flag
        }
        p.Plot();
      }


      if (sensor_stringcomplete) {                        //if a string from the Atlas Scientific product has been received in its entirety
        // Serial.println(sensorstring);                     //send that string to the PC's serial monitor
        ORP = sensorstring.toFloat();                     //convert the string to a floating point number so it can be evaluated by the Arduino
        sensorstring = "";                                //clear the string:
        sensor_stringcomplete = false;                    //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
      }

      //x1= analogRead(sensorPins[1]);
      //x2= analogRead(sensorPins[2]);
      //x = 10*sin( 2.0*PI*( millis() / 5000.0 ) ); // update your variables like usual
      p.Plot();
}



void loop(void)
{


    // Are we bursting ?
    bool bursting = false;
    if(burstCount < burstLength){
        Serial2.println("Bursting");
        Serial2.flush();
        bursting = true;
    }

    // Debug debugLoop
    // this should be a jumper
    bool debugLoop = false;
    if(debugLoop == false){
        debugLoop = DEBUG_LOOP;
    }

    // Are we awake for user interaction?
    bool awakeForUserInteraction = false;
    if(RTC.now().unixtime() < awakeTime + USER_WAKE_TIMEOUT){ // 5 minute timeout
      awakeForUserInteraction = true;
    } else {
        if(!debugLoop){
            if(DEBUG_MESSAGES){
                Serial.println("Not awake for user interaction");
                Serial.flush();
            }
        }
    }
    if(!awakeForUserInteraction) {
        awakeForUserInteraction = debugLoop;
    }


    // See if we should send a measurement to an interactive user
    // or take a bursting measurement
    bool takeMeasurement = false;
    if(bursting){
        takeMeasurement = true;
    } else if(awakeForUserInteraction){
        unsigned long currentMillis = millis();
        if(currentMillis - lastMillis >= interactiveModeMeasurementDelay){
            DateTime now = RTC.now();
            printDateTime(now);
            lastMillis = currentMillis;
            takeMeasurement = true;
        }

    }


    // Should we sleep until a measurement is triggered?
    bool awaitMeasurementTrigger = false;
    if(!bursting && !awakeForUserInteraction){
        if(DEBUG_MESSAGES){
            Serial2.println("Not bursting or awake");
        }
        awaitMeasurementTrigger = true;
    }


    // Go to sleep
    if(awaitMeasurementTrigger){

        if(DEBUG_MESSAGES){
            Serial2.println("Await measurement trigger");
        }
        if(Clock.checkIfAlarm(1)){
            if(DEBUG_MESSAGES){
                Serial2.println("Alarm 1");
            }
        }


        setNextAlarm(); // If we are in this block, alawys set the next alarm
        if(DEBUG_MESSAGES){
            Serial2.flush();
        }

        printInterruptStatus();

        // save enabled interrupts
        int iser1 = NVIC_BASE->ISER[0];
        int iser2 = NVIC_BASE->ISER[1];
        int iser3 = NVIC_BASE->ISER[2];

        // only enable the timer and user interrupts
        NVIC_BASE->ICER[0] = NVIC_BASE->ISER[0];
        NVIC_BASE->ICER[1] = NVIC_BASE->ISER[1];
        NVIC_BASE->ICER[2] = NVIC_BASE->ISER[2];

        // clear any pending interrupts
        NVIC_BASE->ICPR[0] = NVIC_BASE->ISPR[0];
        NVIC_BASE->ICPR[1] = NVIC_BASE->ISPR[1];
        NVIC_BASE->ICPR[2] = NVIC_BASE->ISPR[2];

        enableTimerInterrupt();
        enableUserInterrupt();
        awakenedByUser = false; // Don't go into sleep mode with any interrupt state

        if(false) { // STOP mode WIP
          PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

          PWR_BASE->CR |= PWR_CR_CWUF;
          PWR_BASE->CR |= PWR_CR_PDDS; // Enter stop/standby mode when cpu goes into deep sleep
          //PWR_BASE->CR |= PWR_CR_LPDS; // Puts voltage regulator in low power mode.  This seems to cause problems

          PWR_BASE->CR &= ~PWR_CR_PDDS; // Also have to unset this to get into STOP mode
          SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

          // PWR_BASE->CR |=  PWR_CSR_EWUP;   // Enable wakeup pin bit.  This is for wake from the WKUP pin specifically

          SCB_BASE->SCR &= ~SCB_SCR_SLEEPONEXIT;

          // __asm volatile( "dsb" );
          __asm volatile( "wfi" );
          // __asm volatile( "isb" );

        } else { // SLEEP mode

          __asm volatile( "dsb" );
          systick_disable();
          __asm volatile( "wfi" );
          systick_enable();
          //__asm volatile( "isb" );

        }

        // reenable interrupts
        NVIC_BASE->ISER[0] = iser1;
        NVIC_BASE->ISER[1] = iser2;
        NVIC_BASE->ISER[2] = iser3;
        disableTimerInterrupt();
        disableUserInterrupt();

        // We have woken from the interrupt
        if(DEBUG_MESSAGES){
            Serial2.println("Awakened by interrupt");
            Serial2.flush();
        }

        // Actually, we need to check on which interrupt was triggered
        if(awakenedByUser){

            if(DEBUG_MESSAGES){
                Serial2.println("Awakened by user");
                printDateTime(RTC.now());
                Serial2.flush();
            }

            awakenedByUser = false;
            awakeTime = RTC.now().unixtime();

        } else {
            prepareForTriggeredMeasurement();
        }

        return; // Go to top of loop
    }





    // if DEBUG_BLE
    /*
    Serial2.print("BLE");
    Serial2.println(ble.peek());

    int MAX_REQUEST_LENGTH = 100;
    char request[MAX_REQUEST_LENGTH] = "";
    ble.readBytesUntil('<', request, MAX_REQUEST_LENGTH);
    Serial2.println(request);
    */


    if( ! takeMeasurement){

        if( WaterBear_Control::ready(Serial2) ){
            if(DEBUG_MESSAGES){
                Serial2.println("SERIAL2 Input Ready");
            }
            awakeTime = RTC.now().unixtime(); // Push awake time forward
            WaterBear_Control::processControlCommands(Serial2);
            return;

        }

        //Serial2.println("Checking BLE");
        if(WaterBear_Control::ready(ble) ){
            if(DEBUG_MESSAGES){
                Serial2.println("BLE Input Ready");
            }
            awakeTime = RTC.now().unixtime(); // Push awake time forward
            WaterBear_Control::processControlCommands(ble);
            return;
        }
    }


    if(takeMeasurement){

        if(DEBUG_MEASUREMENTS) {
            Serial2.println("Taking new measurement");
            Serial2.flush();
        }

        measureSensorValues();

        if(DEBUG_MEASUREMENTS) {
            Serial2.println("writeLog");
        }
        filesystem->writeLog(values, fieldCount);
        if(DEBUG_MEASUREMENTS) {
            Serial2.println("writeLog done");
        }

        char valuesBuffer[52];
        sprintf(valuesBuffer, ">WT_VALUES:%s,%s,%s,%s,%s,%s<", values[3], values[4], values[5], values[6], values[7], values[8]);
        if(DEBUG_MEASUREMENTS) {
            Serial2.println(valuesBuffer);
        }
        // Send along to BLE
        if(bleActive) {
            ble.println(valuesBuffer);
        }

        if(bursting) {
            burstCount = burstCount + 1;
        }

    }

}
