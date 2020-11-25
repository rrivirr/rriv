#include <Arduino.h>
#include <Wire.h> // Communicate with I2C/TWI devices
#include <SPI.h>

#include "Adafruit_BluefruitLE_SPI.h"
#include "SdFat.h"
#include "STM32-UID.h"

#include "Utilities.h"
#include "WaterBear_Control.h"
#include "WaterBear_FileSystem.h"

#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <libmaple/rcc.h>

#include "EC_OEM.h"


const uint8_t bufferlen = 32;                         //total buffer size for the response_data array
char response_data[bufferlen];                        //character array to hold the response data from modules
String inputstring = "";

#define SERIAL_BAUD 115200
int serialBaud = SERIAL_BAUD;
#define BUFSIZE                        160   // Size of the read buffer for incoming data

// Settings
char version[5] = "v2.0";

#define DEBUG_MEASUREMENTS false // enable log messages related to measurement & bursts
#define DEBUG_LOOP true         // don't sleep
#define DEBUG_USING_SHORT_SLEEP false // sleep for a hard coded short amount of time
//#define DEBUG_TO_FILE 0   // Also send debug messages to the output file [comment out to disable]
#define DEBUG_TO_SERIAL 1 // Send debug messages to the serial interface

short interval = 5; // minutes between loggings when not in short sleep
short burstLength = 25; // how many readings in a burst
#define USER_WAKE_TIMEOUT           60 * 5 // Timeout after wakeup from user interaction, seconds
//#define USER_WAKE_TIMEOUT           15 // Timeout after wakeup from user interaction, seconds

short fieldCount = 11;


// For F103RB
#define Serial Serial2

TwoWire WIRE1 (1);
#define Wire WIRE1

TwoWire Wire2 (2);

//Ezo_board * ezo_ec;
EC_OEM * oem_ec;

// The DS3231 RTC chip
DS3231 Clock;

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

unsigned char uuid[UUID_LENGTH];

// The internal RTC
//RTClock rt (RTCSEL_LSE); // initialise
//uint32 tt;

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

// State
WaterBear_FileSystem * filesystem;
char lastDownloadDate[11] = "0000000000";
char ** values;
unsigned long lastMillis = 0;
bool awakenedByUser;
uint32_t awakeTime = 0;
uint32_t lastTime = 0;
short burstCount = 0;
bool configurationMode = false;

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


void writeSerialMessage(const char * message){
  Serial2.println(message);
  Serial2.flush();
}

void writeSerialMessage(const __FlashStringHelper * message){
  Serial2.println(message);
  Serial2.flush();
}

void writeDebugMessage(const char * message){
#ifdef DEBUG_TO_SERIAL
  Serial2.println(message);
  Serial2.flush();
#endif

#ifdef DEBUG_TO_FILE
  filesystem->writeDebugMessage(message);
#endif
}

void writeDebugMessage(const __FlashStringHelper * message){
#ifdef DEBUG_TO_SERIAL
  Serial2.println(message);
  Serial2.flush();
#endif

#ifdef DEBUG_TO_FILE
  filesystem->writeDebugMessage(reinterpret_cast<const char *>(message));
#endif
}

// A small helper
void error(const __FlashStringHelper*err) {
  writeDebugMessage(F("Error:"));
  writeDebugMessage(err);
  while (1);
}


void bleFirstRun(){

  // if we don't have a UUID yet, we are running for the first time
  // set a mode pin for USART1 if we need to

  if(true){
    writeDebugMessage("BLE First Run");
  }

  //ble.setMode(BLUEFRUIT_MODE_COMMAND);
  //digitalWrite(D4, HIGH);

  ble.println(F("AT"));
  if(ble.waitForOK()){
    writeDebugMessage("BLE OK");
  } else {
    writeDebugMessage("BLE Not OK");
  }

  // Send command
  ble.println(F("AT+GAPDEVNAME=WaterBear3"));
  if(ble.waitForOK()){
    writeDebugMessage("Got OK");
  } else {
    writeDebugMessage("BLE Error");
    while(1);
  }
  ble.println(F("ATZ"));
  ble.waitForOK();
  writeDebugMessage("Got OK");

//  ble.setMode(BLUEFRUIT_MODE_DATA);

}

void readUniqueId(){

  for(int i=0; i < UUID_LENGTH; i++){
    unsigned int address = EEPROM_UUID_ADDRESS_START + i;
    uuid[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
  }

  writeDebugMessage(F("OK.. UUID in EEPROM:")); // TODO: need to create another function and read from flash
  // Log uuid and time
  // TODO: this is confused.  each byte is 00-FF, which means 12 bytes = 24 chars in hex
  char uuidString[2 * UUID_LENGTH + 1];
  uuidString[2 * UUID_LENGTH] = '\0';
  for(short i=0; i < UUID_LENGTH; i++){
      sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
  }
  writeDebugMessage(uuidString);

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

  if(memcmp(uuid, uninitializedEEPROM, UUID_LENGTH) == 0){
    writeDebugMessage(F("Generate or Retrieve UUID"));
    getSTM32UUID(uuid);

    Serial2.println(F("UUID to Write:"));
    char uuidString[2 * UUID_LENGTH + 1];
    uuidString[2 * UUID_LENGTH] = '\0';
    for(short i=0; i < UUID_LENGTH; i++){
        sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }
    writeDebugMessage(uuidString);

    for(int i=0; i < UUID_LENGTH; i++){
      unsigned int address = EEPROM_UUID_ADDRESS_START + i;
      writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address, uuid[i]);
    }

    for(int i=0; i < UUID_LENGTH; i++){
      unsigned int address = EEPROM_UUID_ADDRESS_START + i;
      uuid[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
    }

    writeDebugMessage(F("UUID in EEPROM:"));
    for(short i=0; i < UUID_LENGTH; i++){
        sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }
    writeDebugMessage(uuidString);

   }

}


bool bleActive = false;

void initBLE(){
  bool debugBLE = true;
  if(debugBLE){
    writeDebugMessage(F("Initializing the Bluefruit LE module: "));
  }
  bleActive = ble.begin(true, true);

  if(debugBLE){
    if(bleActive){
      writeDebugMessage("Tried to init - BLE active");
    } else {
      writeDebugMessage("Tried to init - BLE NOT active");
    }
  }

  if ( !bleActive )
  {
    if(debugBLE){
      writeDebugMessage(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    return;

    // error
  } else {

    writeDebugMessage(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }

    ble.println(F("AT"));
    if(ble.waitForOK()){
      writeDebugMessage("AT OK");
    } else {
         writeDebugMessage("AT NOT OK");
    }

    bleFirstRun();

  }

  if(debugBLE){
    writeDebugMessage(F("BLE OK!") );
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
  // Fetch time from DS3231 RTC
  bool century = false;
	bool h24Flag;
	bool pmFlag;
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(Clock.getYear() + 1900, Clock.getMonth(century) + 1, Clock.getDate()); // year is since 1900, months range 0-11

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(Clock.getHour(h24Flag, pmFlag), Clock.getMinute(), Clock.getSecond());
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
  if(DEBUG_USING_SHORT_SLEEP == true) {
    writeDebugMessage(F("Using short sleep"));
    int AlarmBits = ALRM2_ONCE_PER_MIN;
    AlarmBits <<= 4;
    AlarmBits |= ALRM1_MATCH_SEC;
    short seconds = Clock.getSecond();
    short debugSleepSeconds = 30;
    short nextSeconds = (seconds + debugSleepSeconds - (seconds % debugSleepSeconds)) % 60;
    char message[200];
    sprintf(message, "Next Alarm, with seconds: %i, now seconds: %i", nextSeconds, seconds);
    writeDebugMessage(message);
    Clock.setA1Time(0b0, 0b0, 0b0, nextSeconds, AlarmBits, true, false, false);
  }

  //
  // Alarm every interval minutes for the real world
  //
  else {
    int AlarmBits = ALRM2_ONCE_PER_MIN;
    AlarmBits <<= 4;
    AlarmBits |= ALRM1_MATCH_MIN_SEC;
    short minutes = Clock.getMinute();
    short nextMinutes = (minutes + interval - (minutes % interval)) % 60;
    char message[200];
    sprintf(message, "Next Alarm, with minutes: %i", nextMinutes);
    writeDebugMessage(message);
    Clock.setA1Time(0b0, 0b0, nextMinutes, 0b0, AlarmBits, true, false, false);
  }

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
  //Serial2.println("TIMER ALARM");
  //enableTimerInterrupt();

}

void userTriggeredInterrupt(){

  disableUserInterrupt();
  clearUserInterrupt();
  //Serial2.println("USER TRIGGERED INTERRUPT");
  //Serial2.flush();
  //enableUserInterrupt();
  awakenedByUser = true;

}

#define SWITCHED_POWER_ENABLE PC6

void setupSwitchedPower(){
  pinMode(SWITCHED_POWER_ENABLE, OUTPUT); // enable pin on switchable boost converter
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
}

void enableSwitchedPower(){
  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
}

void disableSwitchedPower(){
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
}



void setupEZOI2C() {

    Serial2.println("EZO I2C setup");

    Serial2.println("Power cycle EZO");
    disableSwitchedPower();
    delay(1000);
    enableSwitchedPower();
    delay(1000);

    i2c_master_enable(I2C2, 0);
    Serial2.println("Enabled TwoWire 2");
    Wire2.begin();
    delay(1000);

    Serial2.println("Began TwoWire 2");
    scanIC2(&Wire2);

    Serial2.println("Setting up board");

    oem_ec = new EC_OEM(&Wire2, NONE_INT, i2c_id);
    bool awoke = oem_ec->wakeUp();
    Serial.println("Device addr EC: "+String(oem_ec->getStoredAddr()) );
    Serial.println("Device type EC: "+String(oem_ec->getDeviceType()) );
    Serial.println("Firmware EC: "+String(oem_ec->getFirmwareVersion()) );
    Serial.println("Awoke: "+String(awoke));
    Serial.println("Hibernating: "+String(oem_ec->isHibernate()) );

    oem_ec->singleReading();
    struct param_OEM_EC parameter;
    parameter = oem_ec->getAllParam();
    Serial.println("salinity= " + String(parameter.salinity)+
                   "\nconductivity= " +String(parameter.conductivity)+
                   "\ntds= " +String(parameter.tds)+
                   "\nSalinity stable = "+(oem_ec->isSalinityStable()?"yes":"no")
                   );


    //ezo_ec = new Ezo_board(&Wire2, 0x64);

    // inputstring.reserve(20);

    // Serial2.println("Turning light on");
    // ezo_ec->send_cmd("L,1");
    // delay(1000);
    // Serial2.println("Turning light off");
    // ezo_ec->send_cmd("L,0");
    // delay(1000);
    // Serial2.println("Turning light on");
    // ezo_ec->send_cmd("L,1");
    // delay(1000);

    // Set probe type
    // ezo_ec->send_cmd("K,1.0");
    // delay(300);
    //
    // // Set outputs
    // ezo_ec->send_cmd("O,EC,1");
    // delay(300);
    // ezo_ec->send_cmd("O,TDS,0");
    // delay(300);
    // ezo_ec->send_cmd("O,S,0");
    // delay(300);


    oem_ec->setLedOn(true);
    //oem_ec->setLedOn(false);
    //oem_ec->setLedOn(true);
    oem_ec->setProbeType(1.0);

    Serial2.println("Done with EZO I2C setup");

}

void stopEZOI2C(){

  //ezo_ec->send_cmd("Sleep");
  oem_ec->setHibernate();

  i2c_disable(I2C2);

  Serial2.println("Done with EZO I2C stop");

}

/*
void setupEZOSerial(){

  Serial1.begin(ezoBaud);

  Serial2.println("Start EZO setup");

  inputstring.reserve(20);                            //set aside some bytes for receiving data from the PC

  ezo_ec.send_cmd_no_resp("*ok,0");             //send the command to turn off the *ok response

  // in order to use multiple circuits more effectively we need to turn off continuous mode and the *ok response
  Serial2.print("C,? : ");
  ezo_ec.send_cmd("c,?", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);                  //print the modules response

  Serial2.print("K,? : ");
  ezo_ec.send_cmd("K,?", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);


  ezo_ec.send_cmd("O,EC,1", response_data, bufferlen); // send it to the module of the port we opened
  ezo_ec.send_cmd("O,TDS,0", response_data, bufferlen); // send it to the module of the port we opened
  ezo_ec.send_cmd("O,S,0", response_data, bufferlen); // send it to the module of the port we opened

  Serial2.print("O,? : ");
  ezo_ec.send_cmd("O,?", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);

  ezo_ec.send_cmd_no_resp("c,0");               //send the command to turn off continuous mode
                                          //in this case we arent concerned about waiting for the reply
  ezo_ec.flush_rx_buffer();                     //clear all the characters that we received from the responses of the above commands

  Serial2.println("Done with EZO setup");
}
*/

/*
void stopEZOSerial(){

  ezo_ec.send_cmd("Sleep", response_data, bufferlen); // send it to the module of the port we opened
  Serial2.println(response_data);
  Serial2.flush();

  Serial1.end();

}
*/


void setup(void)
{
  i2c_bus_reset(I2C1);
  // Start up Serial2
  // Need to do an if(Serial2) after an amount of time, just disable it
  // Note that this is double the actual BAUD due to HSI clocking of processor
   Serial2.begin(serialBaud);
   while(!Serial2){
     delay(100);
   }
   writeSerialMessage(F("Hello world: serial2"));
   writeSerialMessage(F("Begin setup"));

   setupSwitchedPower();
   enableSwitchedPower();

  //pinMode(PB5, OUTPUT); // Command Mode pin for BLE

  pinMode(PC7, INPUT_PULLUP); // This the interrupt line 7
  //pinMode(PB10, INPUT_PULLDOWN); // This WAS interrupt line 10, user interrupt. Needs to be reassigned.

  pinMode(PB1, INPUT_ANALOG);
  pinMode(PC0, INPUT_ANALOG);
  pinMode(PC1, INPUT_ANALOG);
  pinMode(PC2, INPUT_ANALOG);
  pinMode(PC3, INPUT_ANALOG);

  pinMode(PA5, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  niiiiice.
  //writeSerialMessage(F("blink test:"));
  //WaterBear_Control::blink(10,250);

  // Set up global date time callback for SdFile
  SdFile::dateTimeCallback(dateTime);

  // Clear interrupts
  Serial2.print("1: NVIC_BASE->ISPR ");
  Serial2.println(NVIC_BASE->ISPR[0]);
  Serial2.println(NVIC_BASE->ISPR[1]);
  Serial2.println(NVIC_BASE->ISPR[2]);

  NVIC_BASE->ICER[0] =  1 << NVIC_EXTI_9_5; // Don't respond to interrupt during setup
  //NVIC_BASE->ICER[0] =  1 << NVIC_EXTI3; // Don't respond to interrupt during setup

  clearTimerInterrupt();
  clearUserInterrupt();

  Serial2.print("2: NVIC_BASE->ISPR ");
  Serial2.println(NVIC_BASE->ISPR[0]);
  Serial2.println(NVIC_BASE->ISPR[1]);
  Serial2.println(NVIC_BASE->ISPR[2]);

  //  Prepare I2C
  Wire.begin();
  delay(1000);
  scanIC2(&Wire);

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
    readDeploymentIdentifier(deploymentIdentifier);
  }

  time_t setupTime = WaterBear_Control::timestamp();
  char setupTS[21];
  sprintf(setupTS, "unixtime: %lld", setupTime);
  writeSerialMessage(setupTS);

  filesystem = new WaterBear_FileSystem(deploymentIdentifier, PC8);
  writeDebugMessage(F("Filesystem started OK"));

  filesystem->setNewDataFile(setupTime); // name file via epoch timestamp

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

  values[0] = (char *) malloc(sizeof(char) * ((2 * UUID_LENGTH + 1))); // Deployment UUID 25
    sprintf(values[0], "%24d", 0);
  values[1] = (char *) malloc(sizeof(char) * (DEPLOYMENT_IDENTIFIER_LENGTH + 2 * UUID_LENGTH + 2)); // UUID 51
    sprintf(values[1], "%50d", 0);
  values[2] = (char *) malloc(sizeof(char) * 11); // epoch timestamp
    sprintf(values[2], "%10d", 0);
  values[3] = (char *) malloc(sizeof(char) * 24); // human readable timestamp
    sprintf(values[3], "%23d", 0);
  for(int i = 4; i < fieldCount; i++){ // 6 sensors + conductivity
    values[i] = (char *) malloc(sizeof(char) * 5);
    sprintf(values[i], "%4d", 0);
  }

  //
  // Set up interrupts
  //
  exti_attach_interrupt(EXTI7, EXTI_PC, timerAlarm, EXTI_FALLING);
  awakenedByUser = false;

  // PB10 interrupt disabled, PB10 is I2C2, use a different user interrupt
  //exti_attach_interrupt(EXTI10, EXTI_PB, userTriggeredInterrupt, EXTI_RISING);

  //setupEZOSerial();
  setupEZOI2C();

  /* We're ready to go! */
  writeDebugMessage(F("done with setup"));
  Serial2.flush();
}




void prepareForTriggeredMeasurement(){
  burstCount = 0;
}

void measureSensorValues(){
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

  // Log Deployment UUID
  sprintf(values[0], "%s", deploymentUUID);

  // Log UUID
  sprintf(values[1], "%s", uuidString);

  // Fetch and Log time from DS3231 RTC as epoch and human readable timestamps
  time_t currentTime = WaterBear_Control::timestamp();

  sprintf(values[2], "%lld", currentTime); // convert time_t value into string
    writeDebugMessage(F("debugging values[2]:"));
    Serial2.println(values[2]);
    Serial2.flush();

  WaterBear_Control::t_t2ts(currentTime, values[3]); // convert time_t value to human readable timestamp
    writeDebugMessage(F("debugging values[3]:"));
    Serial2.println(values[3]);
    Serial2.flush();

  // Measure the new data
  short sensorCount = 6;
  short sensorPins[6] = {PB0, PB1, PC0, PC1, PC2, PC3};
  for(short i=0; i<sensorCount; i++){
    int value = analogRead(sensorPins[i]);
    // malloc or ?
    sprintf(values[4+i], "%4d", value);
  }

}

unsigned int interactiveModeMeasurementDelay = 1000;
bool newDataAvailable = false;

void loop(void)
{
  //WaterBear_Control::blink(1,50);
  // Are we bursting ?
  bool bursting = false;
  if(burstCount < burstLength){
    writeDebugMessage(F("Bursting"));
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
  if(WaterBear_Control::timestamp() < awakeTime + USER_WAKE_TIMEOUT){ // 5 minute timeout
    awakeForUserInteraction = true;
  } else {
    if(!debugLoop){
      writeDebugMessage(F("Not awake for user interaction"));
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
      lastMillis = currentMillis;
      takeMeasurement = true;
    }
  }


  // Should we sleep until a measurement is triggered?
  bool awaitMeasurementTrigger = false;
  if(!bursting && !awakeForUserInteraction){
    writeDebugMessage(F("Not bursting or awake"));
    awaitMeasurementTrigger = true;
  }


  // Go to sleep
  if(awaitMeasurementTrigger){

    writeDebugMessage(F("Await measurement trigger"));

    if(Clock.checkIfAlarm(1)){
      writeDebugMessage(F("Alarm 1"));
    }

    setNextAlarm(); // If we are in this block, alawys set the next alarm
    //stopEZOSerial();
    writeDebugMessage(F("Stopping EZO I2C"));
    stopEZOI2C();


    writeDebugMessage(F("Disabling switched power"));
    disableSwitchedPower();


    printInterruptStatus(Serial2);
    writeDebugMessage(F("Going to sleep"));

    //delay(1000);

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

    clearUserInterrupt();

    enableTimerInterrupt();
    enableUserInterrupt();
    awakenedByUser = false; // Don't go into sleep mode with any interrupt state


    Serial2.end();

    // TODO: use STOP mode
    if(true) { // STOP mode
      // Clear PDDS and LPDS bits
      PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

      // Set PDDS and LPDS bits for standby mode, and set Clear WUF flag (required per datasheet):
      PWR_BASE->CR |= PWR_CR_CWUF;
      PWR_BASE->CR |= PWR_CR_PDDS; // Enter stop/standby mode when cpu goes into deep sleep

      // PWR_BASE->CR |=  PWR_CSR_EWUP;   // Enable wakeup pin bit.  This is for wake from the WKUP pin specifically

      //  Unset Power down deepsleep bit.
      PWR_BASE->CR &= ~PWR_CR_PDDS; // Also have to unset this to get into STOP mode
      // set Low-power deepsleep.
      PWR_BASE->CR |= PWR_CR_LPDS; // Puts voltage regulator in low power mode.  This seems to cause problems

      SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
      //SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;

      SCB_BASE->SCR &= ~SCB_SCR_SLEEPONEXIT;

      rcc_switch_sysclk(RCC_CLKSRC_HSI);
      rcc_turn_off_clk(RCC_CLK_PLL);

      __asm volatile( "dsb" );
      systick_disable();
      __asm volatile( "wfi" );
      systick_enable();
      __asm volatile( "isb" );

      rcc_turn_on_clk(RCC_CLK_PLL);
      while(!rcc_is_clk_ready(RCC_CLK_PLL))
      ;

      // Finally, switch to the now-ready PLL as the main clock source.
      rcc_switch_sysclk(RCC_CLKSRC_PLL);

    } else { // SLEEP mode

      __asm volatile( "dsb" );
      systick_disable();
      __asm volatile( "wfi" );
      systick_enable();
      //__asm volatile( "isb" );

    }

    Serial2.begin(serialBaud);
    //setupEZOSerial();


    // reenable interrupts
    NVIC_BASE->ISER[0] = iser1;
    NVIC_BASE->ISER[1] = iser2;
    NVIC_BASE->ISER[2] = iser3;
    disableTimerInterrupt();
    disableUserInterrupt();

    // We have woken from the interrupt
    writeDebugMessage(F("Awakened by interrupt"));
    printInterruptStatus(Serial2);

    enableSwitchedPower();
    delay(500);
    setupEZOI2C();

    // Actually, we need to check on which interrupt was triggered
    if(awakenedByUser){
      char humanTime[26];
      time_t awakenedTime = WaterBear_Control::timestamp();

      WaterBear_Control::t_t2ts(awakenedTime, humanTime);
      writeDebugMessage(F("Awakened by user"));
      writeDebugMessage(F(humanTime));

      awakenedByUser = false;
      awakeTime = awakenedTime;

    } else {
      prepareForTriggeredMeasurement();
    }

    return; // Go to top of loop
  }



  if( WaterBear_Control::ready(Serial2) ){
    writeDebugMessage(F("SERIAL2 Input Ready"));
    awakeTime = WaterBear_Control::timestamp(); // Push awake time forward
    int command = WaterBear_Control::processControlCommands(Serial2);
    switch(command){
      case WT_CONTROL_CONFIG:
        writeDebugMessage(F("Entering Configuration Mode"));
        writeDebugMessage(F("Reset device to enter normal operating mode"));
        configurationMode = true;
        return;
        break;

      case WT_CONTROL_CAL_DRY:
        writeDebugMessage(F("DRY_CALIBRATION"));
        oem_ec->clearCalibrationData();
        oem_ec->setCalibration(DRY_CALIBRATION);
        break;

      case WT_CONTROL_CAL_LOW:
      {
        writeDebugMessage(F("LOW_POINT_CALIBRATION"));
        int * lowPointPtr = (int *) WaterBear_Control::getLastPayload();
        int lowPoint = *lowPointPtr;
        char logMessage[30];
        sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("LOW_POINT_CALIBRATION: "), lowPoint);
        writeDebugMessage(logMessage);
        oem_ec->setCalibration(LOW_POINT_CALIBRATION, lowPoint);
      }
        break;

      case WT_CONTROL_CAL_H:
      {
        writeDebugMessage(F("HIGH_POINT_CALIBRATION"));
        int * highPointPtr = (int *) WaterBear_Control::getLastPayload();
        int highPoint = *highPointPtr;
        char logMessage[31];
        sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("HIGH_POINT_CALIBRATION: "), highPoint);
        oem_ec->setCalibration(HIGH_POINT_CALIBRATION, highPoint);
      }
        break;

      case WT_SET_RTC: // DS3231
      {
        writeDebugMessage(F("SET_RTC"));
        time_t * RTCPtr = (time_t *) WaterBear_Control::getLastPayload();
        time_t RTC = *RTCPtr;
        char logMessage[24];
        sprintf(&logMessage[0], "%s%lld", reinterpret_cast<const char *> F("SET_RTC_TO: "), RTC);
        WaterBear_Control::setTime(RTC);
      }
        break;
      // default:
      //   break;
    }

    return;
  }

  if(configurationMode){
    char testTime[11];

    Serial2.print(F("configMode TS:"));
    sprintf(testTime, "%lld", WaterBear_Control::timestamp()); // convert time_t value into string
    Serial2.println(testTime);
    Serial2.flush();

    if(newDataAvailable){
      float ecValue = -1;
      ecValue = oem_ec->getConductivity();
      oem_ec->clearNewDataRegister();
      newDataAvailable = false;
      Serial2.print(F("Got EC value: "));
      Serial2.println(ecValue);
      Serial2.flush();
    } else {
      newDataAvailable = oem_ec->singleReading();
    }
    return;
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

  //Serial2.println("Checking BLE");
  if(WaterBear_Control::ready(ble) ){
    writeDebugMessage(F("BLE Input Ready"));
    awakeTime = WaterBear_Control::timestamp(); // Push awake time forward
    WaterBear_Control::processControlCommands(ble);
    return;
  }

  if(takeMeasurement){

    if(DEBUG_MEASUREMENTS) {
      writeDebugMessage(F("Taking new measurement"));
    }

    measureSensorValues();


    // EZO
    // wake/sleep.  Or re-run setup
    /*
    Serial2.print(ezo_ec.get_name());     //print the modules name
    Serial2.print(": ");
    Serial2.println(response_data);                  //print the modules response
    response_data[0] = 0;                           //clear the modules response

    ezo_ec.send_read();
    Serial2.print("EZO Reading:");
    float ecValue = ezo_ec.get_reading();
    Serial2.print(ecValue);
    Serial2.println();
    sprintf(values[4], "%4f", ecValue); // stuff EC value into values[4] for the moment.
    */

    // EZO i2c
    /* */
    // ezo_ec->send_read_cmd();
    // delay(600);
    // float ecValue = ezo_ec->get_last_received_reading();
    /* */

    // OEM EC
    bool newDataAvailable = oem_ec->singleReading();
    float ecValue = -1;
    if(newDataAvailable){
      ecValue = oem_ec->getConductivity();
      oem_ec->clearNewDataRegister();
    } else {
      writeDebugMessage(F("New EC data not available"));
    }

    //Serial2.print(F("Got EC value: "));
    //Serial2.print(ecValue);
    //Serial2.println();
    sprintf(values[10], "%4f", ecValue); // stuff EC value into values[10] for the moment.

    if(DEBUG_MEASUREMENTS) {
      writeDebugMessage(F("writeLog"));
    }
    filesystem->writeLog(values, fieldCount);
    if(DEBUG_MEASUREMENTS) {
      writeDebugMessage(F("writeLog done"));
    }

    char valuesBuffer[180]; // 51+25+11+24+(7*5)+33
    sprintf(valuesBuffer, ">WT_VALUES: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s<", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10]);
    if(DEBUG_MEASUREMENTS) {
      writeDebugMessage(F(valuesBuffer));
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
