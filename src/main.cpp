#include <Arduino.h>
#include <Wire.h> // Communicate with I2C/TWI devices
#include <SPI.h>

#include "SdFat.h"
#include "lib/STM32-UID.h"

#include "configuration.h"
#include "WaterBear_Control.h"
#include "WaterBear_FileSystem.h"
#include "utilities.h"

#include "components/hardware.h"
#include "components/interrupts.h"
#include "components/eeprom.h"
#include "components/low_power.h"
#include "components/monitor.h"
#include "components/clock.h"
#include "components/ble.h"
#include "components/switched_power.h"
#include "sensors/atlas_oem.h"

#include <libmaple/iwdg.h>

// Settings
char version[5] = "v2.0";

short interval = 5;     // minutes between loggings when not in short sleep
short burstLength = 25; // how many readings in a burst

short fieldCount = 11; // number of fields to be logged to SDcard file

// Pin Mappings for Nucleo Board
// BLE USART
//#define D4 PB5
//int bluefruitModePin = D4;
//Adafruit_BluefruitLE_UART ble(Serial1, bluefruitModePin);

/// Begin Main System Entity

// State
WaterBear_FileSystem *filesystem;
unsigned char uuid[UUID_LENGTH];
char lastDownloadDate[11] = "0000000000";
char **values;
unsigned long lastMillis = 0;
bool awakenedByUser;
uint32_t awakeTime = 0;
uint32_t lastTime = 0;
short burstCount = 0;
bool configurationMode = false;
bool debugValuesMode = false;
bool clearModes = false;

void enableI2C2()
{
  i2c_master_enable(I2C2, 0);
  Monitor::instance()->writeDebugMessage(F("Enabled I2C2"));

  Wire2.begin();
  delay(1000);

  Monitor::instance()->writeDebugMessage(F("Began TwoWire 2"));
  scanIC2(&Wire2);
}

void powerUpSwitchableComponents()
{
  cycleSwitchablePower();
  enableI2C2();
  setupEC_OEM(&Wire2);
  Monitor::instance()->writeDebugMessage(F("Switchable components powered up"));
}

void powerDownSwitchableComponents()
{
  hibernateEC_OEM();
  i2c_disable(I2C2);
  Monitor::instance()->writeDebugMessage(F("Switchable components powered down"));
}

void startSerial2()
{
  // Start up Serial2
  // TODO: Need to do an if(Serial2) after an amount of time, just disable it
  Serial2.begin(SERIAL_BAUD);
  while (!Serial2)
  {
    delay(100);
  }
  Monitor::instance()->writeSerialMessage(F("Hello world: serial2"));
  Monitor::instance()->writeSerialMessage(F("Begin Setup"));
}

void setupHardwarePins()
{
  //pinMode(BLE_COMMAND_MODE_PIN, OUTPUT); // Command Mode pin for BLE
  pinMode(INTERRUPT_LINE_7_PIN, INPUT_PULLUP); // This the interrupt line 7
  //pinMode(PB10, INPUT_PULLDOWN); // This WAS interrupt line 10, user interrupt. Needs to be reassigned.
  pinMode(ANALOG_INPUT_1_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_2_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_3_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_4_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_5_PIN, INPUT_ANALOG);
  pinMode(ONBOARD_LED_PIN, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  niiiiice.
}

void blinkTest()
{
  //Logger::instance()->writeDebugMessage(F("blink test:"));
  //blink(10,250);
}

void initializeFilesystem()
{

  SdFile::dateTimeCallback(dateTime);

  char defaultDeployment[25] = "SITENAME_00000000000000";
  char *deploymentIdentifier = defaultDeployment;

  // get any stored deployment identifier from EEPROM
  readDeploymentIdentifier(deploymentIdentifier);
  unsigned char empty[1] = {0xFF};
  if (memcmp(deploymentIdentifier, empty, 1) == 0)
  {
    //Logger::instance()->writeDebugMessage(F(">NoDplyment<"));

    writeDeploymentIdentifier(defaultDeployment);
    readDeploymentIdentifier(deploymentIdentifier);
  }

  filesystem = new WaterBear_FileSystem(deploymentIdentifier, SD_ENABLE_PIN);
  Monitor::instance()->filesystem = filesystem;
  Monitor::instance()->Monitor::instance()->writeDebugMessage(F("Filesystem started OK"));

  time_t setupTime = timestamp();
  char setupTS[21];
  sprintf(setupTS, "unixtime: %lld", setupTime);
  Monitor::instance()->Monitor::instance()->writeSerialMessage(setupTS);
  filesystem->setNewDataFile(setupTime); // name file via epoch timestamp
}

void allocateMeasurementValuesMemory()
{

  values = (char **)malloc(sizeof(char *) * fieldCount);

  values[0] = (char *)malloc(sizeof(char) * (DEPLOYMENT_IDENTIFIER_LENGTH + 2 * UUID_LENGTH + 2)); // Deployment UUID 51
  sprintf(values[0], "%50d", 0);
  values[1] = (char *)malloc(sizeof(char) * ((2 * UUID_LENGTH + 1))); // UUID 25
  sprintf(values[1], "%24d", 0);
  values[2] = (char *)malloc(sizeof(char) * 11); // epoch timestamp
  sprintf(values[2], "%10d", 0);
  values[3] = (char *)malloc(sizeof(char) * 24); // human readable timestamp
  sprintf(values[3], "%23d", 0);
  for (int i = 4; i < fieldCount; i++)
  { // 6 sensors + conductivity
    values[i] = (char *)malloc(sizeof(char) * 5);
    sprintf(values[i], "%4d", 0);
  }
}

void prepareForTriggeredMeasurement()
{
  burstCount = 0;
}

void prepareForUserInteraction()
{
  char humanTime[26];
  time_t awakenedTime = timestamp();

  t_t2ts(awakenedTime, humanTime);
  Monitor::instance()->writeDebugMessage(F("Awakened by user"));
  Monitor::instance()->writeDebugMessage(F(humanTime));

  awakenedByUser = false;
  awakeTime = awakenedTime;
}

void setNotBursting()
{
  burstCount = burstLength; // Set to not bursting
}

void measureSensorValues()
{
  // TODO: do we need to do this every time ??
  char uuidString[2 * UUID_LENGTH + 1];
  uuidString[2 * UUID_LENGTH] = '\0';
  for (short i = 0; i < UUID_LENGTH; i++)
  {
    sprintf(&uuidString[2 * i], "%02X", (byte)uuid[i]);
  }

  // Get the deployment identifier
  // TODO: do we need to do this every time ??
  char deploymentIdentifier[26];
  readDeploymentIdentifier(deploymentIdentifier);
  char deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH + 2 * UUID_LENGTH + 2];
  memcpy(deploymentUUID, deploymentIdentifier, DEPLOYMENT_IDENTIFIER_LENGTH);
  deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH] = '_';
  memcpy(&deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH + 1], uuidString, 2 * UUID_LENGTH);
  deploymentUUID[DEPLOYMENT_IDENTIFIER_LENGTH + 2 * UUID_LENGTH] = '\0';

  // Log Deployment UUID
  sprintf(values[0], "%s", deploymentUUID);

  // Log UUID
  sprintf(values[1], "%s", uuidString);

  // Fetch and Log time from DS3231 RTC as epoch and human readable timestamps
  time_t currentTime = timestamp();
  sprintf(values[2], "%lld", currentTime);           // convert time_t value into string
  t_t2ts(currentTime, values[3]); // convert time_t value to human readable timestamp

  // Measure the new data
  short sensorCount = 6;
  short sensorPins[6] = {PB0, PB1, PC0, PC1, PC2, PC3};
  for (short i = 0; i < sensorCount; i++)
  {
    int value = analogRead(sensorPins[i]);
    // malloc or ?
    sprintf(values[4 + i], "%4d", value);
  }
}

/// End Main System Entity

void setup(void)
{

  startSerial2();

  setupSwitchedPower();
  enableSwitchedPower();

  setupHardwarePins();

  //blinkTest();

  // Don't respond to interrupts during setup
  disableClockInterrupt();
  disableUserInterrupt();

  clearClockInterrupt();
  clearUserInterrupt();

  //  Prepare I2C
  Wire.begin();
  delay(1000);
  scanIC2(&Wire);

  // Clear the alarms so they don't go off during setup
  clearAllAlarms();

  initializeFilesystem();

  //initBLE();

  readUniqueId(uuid);

  allocateMeasurementValuesMemory();

  setupWakeInterrupts();

  powerUpSwitchableComponents();

  setNotBursting();

  /* We're ready to go! */
  Monitor::instance()->writeDebugMessage(F("done with setup"));
  Serial2.flush();
}

bool checkBursting()
{
  bool bursting = false;
  if (burstCount < burstLength)
  {
    Monitor::instance()->writeDebugMessage(F("Bursting"));
    bursting = true;
  }
  return bursting;
}

bool checkDebugLoop()
{
  // Debug debugLoop
  // this should be a jumper
  bool debugLoop = false;
  if (debugLoop == false)
  {
    debugLoop = DEBUG_LOOP;
  }
  return debugLoop;
}

bool checkAwakeForUserInteraction(bool debugLoop)
{
  // Are we awake for user interaction?
  bool awakeForUserInteraction = false;
  if (timestamp() < awakeTime + USER_WAKE_TIMEOUT)
  { // 5 minute timeout
    awakeForUserInteraction = true;
  }
  else
  {
    if (!debugLoop)
    {
      Monitor::instance()->writeDebugMessage(F("Not awake for user interaction"));
    }
  }
  if (!awakeForUserInteraction)
  {
    awakeForUserInteraction = debugLoop;
  }
  return awakeForUserInteraction;
}

bool checkTakeMeasurement(bool bursting, bool awakeForUserInteraction)
{
  // See if we should send a measurement to an interactive user
  // or take a bursting measurement
  bool takeMeasurement = false;
  if (bursting)
  {
    takeMeasurement = true;
  }
  else if (awakeForUserInteraction)
  {
    unsigned long currentMillis = millis();
    int interactiveMeasurementDelay = 1000;
    if (currentMillis - lastMillis >= interactiveMeasurementDelay)
    {
      lastMillis = currentMillis;
      takeMeasurement = true;
    }
  }
  return takeMeasurement;
}

void loop(void)
{
  bool bursting = checkBursting();
  bool debugLoop = checkDebugLoop();
  bool awakeForUserInteraction = checkAwakeForUserInteraction(debugLoop);
  bool takeMeasurement = checkTakeMeasurement(bursting, awakeForUserInteraction);

  // Should we sleep until a measurement is triggered?
  bool awaitMeasurementTrigger = false;
  if (!bursting && !awakeForUserInteraction)
  {
    Monitor::instance()->writeDebugMessage(F("Not bursting or awake"));
    awaitMeasurementTrigger = true;
  }

  // Go to sleep
  if (awaitMeasurementTrigger)
  {

    Monitor::instance()->writeDebugMessage(F("Await measurement trigger"));

    if (Clock.checkIfAlarm(1))
    {
      Monitor::instance()->writeDebugMessage(F("Alarm 1"));
    }

    setNextAlarm(interval); // If we are in this block, alawys set the next alarm
    powerDownSwitchableComponents();
    disableSwitchedPower();

    printInterruptStatus(Serial2);
    Monitor::instance()->writeDebugMessage(F("Going to sleep"));

    // save enabled interrupts
    int iser1, iser2, iser3;
    storeAllInterrupts(iser1, iser2, iser3);

    clearAllInterrupts();
    clearAllPendingInterrupts();
    clearUserInterrupt();

    enableClockInterrupt();
    enableUserInterrupt();
    awakenedByUser = false; // Don't go into sleep mode with any interrupt state

    Serial2.end();

    enterStopMode();
    //enterSleepMode()

    Serial2.begin(SERIAL_BAUD);

    reenableAllInterrupts(iser1, iser2, iser3);
    disableClockInterrupt();
    disableUserInterrupt();

    // We have woken from the interrupt
    Monitor::instance()->writeDebugMessage(F("Awakened by interrupt"));
    printInterruptStatus(Serial2);

    powerUpSwitchableComponents();

    // We need to check on which interrupt was triggered
    if (awakenedByUser)
    {
      prepareForUserInteraction();
    }
    else
    {
      prepareForTriggeredMeasurement();
    }

    return; // Go to top of loop
  }

  if (WaterBear_Control::ready(Serial2))
  {
    Monitor::instance()->writeDebugMessage(F("SERIAL2 Input Ready"));
    awakeTime = timestamp(); // Push awake time forward
    int command = WaterBear_Control::processControlCommands(Serial2);
    switch (command)
    {
    case WT_CLEAR_MODES:
      Monitor::instance()->writeDebugMessage(F("Clearing Config & Debug Mode"));
      configurationMode = false;
      debugValuesMode = false;
      break;
    case WT_CONTROL_CONFIG:
      Monitor::instance()->writeDebugMessage(F("Entering Configuration Mode"));
      Monitor::instance()->writeDebugMessage(F("Reset device to enter normal operating mode"));
      Monitor::instance()->writeDebugMessage(F("Or >WT_CLEAR_MODES<"));
      configurationMode = true;
      break;
    case WT_DEBUG_VAlUES:
      Monitor::instance()->writeDebugMessage(F("Entering Value Debug Mode"));
      Monitor::instance()->writeDebugMessage(F("Reset device to enter normal operating mode"));
      Monitor::instance()->writeDebugMessage(F("Or >WT_CLEAR_MODES<"));
      debugValuesMode = true;
      break;
    case WT_CONTROL_CAL_DRY:
      Monitor::instance()->writeDebugMessage(F("DRY_CALIBRATION"));
      clearECCalibrationData();
      setECDryPointCalibration();
      break;
    case WT_CONTROL_CAL_LOW:
    {
      Monitor::instance()->writeDebugMessage(F("LOW_POINT_CALIBRATION"));
      int *lowPointPtr = (int *)WaterBear_Control::getLastPayload();
      int lowPoint = *lowPointPtr;
      char logMessage[30];
      sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("LOW_POINT_CALIBRATION: "), lowPoint);
      Monitor::instance()->writeDebugMessage(logMessage);
      setECLowPointCalibration(lowPoint);
      break;
    }
    case WT_CONTROL_CAL_HIGH:
    {
      Monitor::instance()->writeDebugMessage(F("HIGH_POINT_CALIBRATION"));
      int *highPointPtr = (int *)WaterBear_Control::getLastPayload();
      int highPoint = *highPointPtr;
      char logMessage[31];
      sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("HIGH_POINT_CALIBRATION: "), highPoint);
      setECHighPointCalibration(highPoint);
      break;
    }
    case WT_SET_RTC: // DS3231
    {
      Monitor::instance()->writeDebugMessage(F("SET_RTC"));
      time_t *RTCPtr = (time_t *)WaterBear_Control::getLastPayload();
      time_t RTC = *RTCPtr;
      char logMessage[24];
      sprintf(&logMessage[0], "%s%lld", reinterpret_cast<const char *> F("SET_RTC_TO: "), RTC);
      setTime(RTC);
      break;
    }
    case WT_DEPLOY: // Set deployment identifier via serial
    {
      Monitor::instance()->writeDebugMessage(F("SET_DEPLOYMENT_IDENTIFIER"));
      char *deployPtr = (char *)WaterBear_Control::getLastPayload();
      char logMessage[46];
      sprintf(&logMessage[0], "%s%s", reinterpret_cast<const char *> F("SET_DEPLOYMENT_TO: "), deployPtr);
      Monitor::instance()->writeDebugMessage(logMessage);
      writeDeploymentIdentifier(deployPtr);
      break;
    }
    default:
      Monitor::instance()->writeDebugMessage(F("Invalid command code"));
      break;
    }
    return;
  }

  if (configurationMode)
  {
    // blink(1,500); //slow down rate of responses
    // printDS3231Time();

    float ecValue = -1;
    bool newDataAvailable = readECDataIfAvailable(&ecValue);
    if (newDataAvailable)
    {
      char message[100];
      sprintf(message, "Got EC value: %f", ecValue);
      Monitor::instance()->writeDebugMessage(message);
    }
    return;
  }

  if (WaterBear_Control::ready(getBLE()))
  {
    Monitor::instance()->writeDebugMessage(F("BLE Input Ready"));
    awakeTime = timestamp(); // Push awake time forward
    WaterBear_Control::processControlCommands(getBLE());
    return;
  }

  if (takeMeasurement)
  {

    if (DEBUG_MEASUREMENTS)
    {
      Monitor::instance()->writeDebugMessage(F("Taking new measurement"));
    }

    measureSensorValues();

    // OEM EC
    float ecValue = -1;
    bool newDataAvailable = readECDataIfAvailable(&ecValue);
    if (!newDataAvailable)
    {
      Monitor::instance()->writeDebugMessage(F("New EC data not available"));
    }

    //Serial2.print(F("Got EC value: "));
    //Serial2.print(ecValue);
    //Serial2.println();
    sprintf(values[10], "%4f", ecValue); // stuff EC value into values[10] for the moment.

    if (DEBUG_MEASUREMENTS)
    {
      Monitor::instance()->writeDebugMessage(F("writeLog"));
    }
    filesystem->writeLog(values, fieldCount);
    if (DEBUG_MEASUREMENTS)
    {
      Monitor::instance()->writeDebugMessage(F("writeLog done"));
    }

    char valuesBuffer[180]; // 51+25+11+24+(7*5)+33
    sprintf(valuesBuffer, ">WT_VALUES: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s<", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10]);
    if (DEBUG_MEASUREMENTS)
    {
      Monitor::instance()->writeDebugMessage(F(valuesBuffer));
    }

    printToBLE(valuesBuffer);

    if (bursting)
    {
      burstCount = burstCount + 1;
    }
  }
  if (debugValuesMode)
  { // print content being logged each second
    blink(1, 500);
    char valuesBuffer[180]; // 51+25+11+24+(7*5)+33
    sprintf(valuesBuffer, ">WT_VALUES: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s<", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10]);
    Monitor::instance()->writeDebugMessage(F(valuesBuffer));
    return;
  }
}
