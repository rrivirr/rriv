#include "datalogger.h"

// Settings
char version[5] = "v2.0";

short interval = 2;     // minutes between loggings when not in short sleep
short burstLength = 10; // how many readings in a burst

short fieldCount = 22; // number of fields to be logged to SDcard file

// Pin Mappings for Nucleo Board
// BLE USART
//#define D4 PB5
//int bluefruitModePin = D4;
//Adafruit_BluefruitLE_UART ble(Serial1, bluefruitModePin);

// State
WaterBear_FileSystem *filesystem;
unsigned char uuid[UUID_LENGTH];
char lastDownloadDate[11] = "0000000000";
char **values;
unsigned long lastMillis = 0;
uint32_t awakeTime = 0;
uint32_t lastTime = 0;
short burstCount = 0;
bool configurationMode = false;
bool debugValuesMode = false;
bool clearModes = false;
bool tempCalMode = false;
bool tempCalibrated = false;
short controlFlag = 0;

void enableI2C2()
{
  i2c_disable(I2C2);
  i2c_master_enable(I2C2, 0);
  Monitor::instance()->writeDebugMessage(F("Enabled I2C2"));

  i2c_bus_reset(I2C2);
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

void powerDownSwitchableComponents() // called in stopAndAwaitTrigger
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
  Monitor::instance()->writeDebugMessage(F("setting up hardware pins"));
  //pinMode(BLE_COMMAND_MODE_PIN, OUTPUT); // Command Mode pin for BLE
  pinMode(INTERRUPT_LINE_7_PIN, INPUT_PULLUP); // This the interrupt line 7

  pinMode(ANALOG_INPUT_1_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_2_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_3_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_4_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_5_PIN, INPUT_ANALOG);
  pinMode(ONBOARD_LED_PIN, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  niiiiice.

  pinMode(PA2, OUTPUT); // USART2_TX/ADC12_IN2/TIM2_CH3
  pinMode(PA3, OUTPUT); // USART2_RX/ADC12_IN3/TIM2_CH4
  pinMode(PA4, INPUT_PULLDOWN);
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
  values[2] = (char *)malloc(sizeof(char) * 15); // epoch timestamp.millis
  sprintf(values[2], "%10.3f", (double)0);
  values[3] = (char *)malloc(sizeof(char) * 24); // human readable timestamp
  sprintf(values[3], "%23d", 0);
  for (int i = 4; i <= 10; i++)
  { // 6 sensors + conductivity + 6 for temp calibration data
    values[i] = (char *)malloc(sizeof(char) * 5);
    sprintf(values[i], "%4d", 0);
  }

  values[11] = (char *)malloc(sizeof(char) * 11); // epoch timestamp for temp calibration
  sprintf(values[11], "%10d", 0);
  for (int i = 12; i <= 18; i++)
  { // temp calibration data C1, V1, C2, V2, M, B, temp reading
    values[i] = (char *)malloc(sizeof(char) * 7);
    sprintf(values[i], "%6d", 0);
  }
  values[19] = (char *)malloc(sizeof(char) * 3); // burst count
  sprintf(values[19], "%2d", 0);
  values[20] = (char *)malloc(sizeof(char) * 11); // user serial value input
  sprintf(values[20], "%10d", 0);
  values[21] = (char *)malloc(sizeof(char) * 31); // user serial notes input
  sprintf(values[21], "%30d", 0);
}

void prepareForTriggeredMeasurement()
{
  burstCount = 0;
}

void prepareForUserInteraction()
{
  char humanTime[26];
  time_t awakenedTime = timestamp();

  t_t2ts(awakenedTime, millis(), humanTime);
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
  static double currentTime;
  static time_t currentEpoch;
  static uint32 offsetMillis;
  if (burstCount == 0)
  {
    Monitor::instance()->writeDebugMessage(F("setting base time"));
    currentEpoch = timestamp();
    offsetMillis = millis();
  }
  /*else if (!currentEpoch && !offsetMillis)
  {
    currentEpoch = timestamp();
    offsetMillis = millis();
  }*/
  uint32 currentMillis = millis();
  currentTime = (double)currentEpoch + (((double)currentMillis - offsetMillis) / 1000);

  //debug timestamp calculations
  char valuesBuffer[190];
  sprintf(valuesBuffer, "burstCount=%i currentMillis=%i offsetMillis=%i currentEpoch=%lld currentTime=%10.3f\n", burstCount, currentMillis, offsetMillis, currentEpoch, currentTime);
  Monitor::instance()->writeDebugMessage(F(valuesBuffer));

  sprintf(values[2], "%10.3f", currentTime); // convert double value into string
  t_t2ts(currentTime, currentMillis-offsetMillis, values[3]);        // convert time_t value to human readable timestamp

  // Measure the new data
  short sensorCount = 6;
  short sensorPins[6] = {PB0, PB1, PC0, PC1, PC2, PC3};
  for (short i = 0; i < sensorCount; i++)
  {
    int value = analogRead(sensorPins[i]);
    sprintf(values[4 + i], "%4d", value);
  }
  // Measure and log temperature data and calibration info -> move to seperate function?
  unsigned int uiData = 0;
  unsigned short usData = 0;

  readEEPROMBytes(TEMPERATURE_TIMESTAMP_ADDRESS_START, (unsigned char *)&uiData, TEMPERATURE_TIMESTAMP_ADDRESS_LENGTH);
  sprintf(values[11], "%i", uiData);
  readEEPROMBytes(TEMPERATURE_C1_ADDRESS_START, (unsigned char *)&usData, TEMPERATURE_C1_ADDRESS_LENGTH);
  sprintf(values[12], "%i", usData);
  readEEPROMBytes(TEMPERATURE_V1_ADDRESS_START, (unsigned char *)&usData, TEMPERATURE_V1_ADDRESS_LENGTH);
  sprintf(values[13], "%i", usData);
  readEEPROMBytes(TEMPERATURE_C2_ADDRESS_START, (unsigned char *)&usData, TEMPERATURE_C2_ADDRESS_LENGTH);
  sprintf(values[14], "%i", usData);
  readEEPROMBytes(TEMPERATURE_V2_ADDRESS_START, (unsigned char *)&usData, TEMPERATURE_V2_ADDRESS_LENGTH);
  sprintf(values[15], "%i", usData);
  readEEPROMBytes(TEMPERATURE_M_ADDRESS_START, (unsigned char *)&usData, TEMPERATURE_M_ADDRESS_LENGTH);
  sprintf(values[16], "%i", usData);
  readEEPROMBytes(TEMPERATURE_B_ADDRESS_START, (unsigned char *)&uiData, TEMPERATURE_B_ADDRESS_LENGTH);
  sprintf(values[17], "%i", uiData);
  sprintf(values[18], "%.2f", calculateTemperature());
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

bool checkThermistorCalibration()
{
  unsigned int calTime = 0;
  bool thermistorCalibrated = false;

  readEEPROMBytes(TEMPERATURE_TIMESTAMP_ADDRESS_START, (unsigned char*)&calTime, TEMPERATURE_TIMESTAMP_ADDRESS_LENGTH);
  if (calTime > 1617681773 && calTime != 4294967295)
  {
    thermistorCalibrated = true;
  }
  return thermistorCalibrated;
}

bool checkAwakeForUserInteraction(bool debugLoop)
{
  // Are we awake for user interaction?
  bool awakeForUserInteraction = false;
  if (timestamp() < awakeTime + USER_WAKE_TIMEOUT)
  {
    Monitor::instance()->writeDebugMessage(F("Awake for user interaction"));
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
    unsigned int interactiveMeasurementDelay = 1000;
    if (currentMillis - lastMillis >= interactiveMeasurementDelay)
    {
      lastMillis = currentMillis;
      takeMeasurement = true;
    }
  }
  return takeMeasurement;
}

void stopAndAwaitTrigger()
{
  Monitor::instance()->writeDebugMessage(F("Await measurement trigger"));

  if (Clock.checkIfAlarm(1))
  {
    Monitor::instance()->writeDebugMessage(F("Alarm 1"));
  }

  //setNextAlarm(interval); // If we are in this block, alawys set the next alarm
  // setNextAlarmInternalRTC(interval); // with interrupt
  
  // powerDownSwitchableComponents();
  // disableSwitchedPower();

  printInterruptStatus(Serial2);
  Monitor::instance()->writeDebugMessage(F("Going to sleep"));

  // save enabled interrupts
  int iser1, iser2, iser3;
  storeAllInterrupts(iser1, iser2, iser3);

<<<<<<< HEAD

  clearClockInterrupt();

  ///
  /// Junk code
  ///
  // clearUserInterrupt(); // no longer used
  // enableClockInterrupt(); // The DS3231, which is not powered during stop mode on v0.2 hardware
                            // Wake button and DS3231 can both access this interrupt on v0.2
  // printInterruptStatus(Serial2);
  // enableRTCAlarmInterrupt(); // The internal RTC Alarm interrupt, in the backup domain, wrong code though

  setNextAlarmInternalRTC(interval); // close to the right code
  Serial2.flush();

  clearAllInterrupts();
  clearAllPendingInterrupts();
  // clearUserInterrupt(); // no longer used
=======
>>>>>>> b0140f7 (Internal RTC interrupt wake working)

  clearClockInterrupt();

  ///
  /// Junk code
  ///
  // clearUserInterrupt(); // no longer used
  // enableClockInterrupt(); // The DS3231, which is not powered during stop mode on v0.2 hardware
                            // Wake button and DS3231 can both access this interrupt on v0.2
  // printInterruptStatus(Serial2);
  // enableRTCAlarmInterrupt(); // The internal RTC Alarm interrupt, in the backup domain, wrong code though

  setNextAlarmInternalRTC(interval); // close to the right code
  Serial2.flush();

  clearAllInterrupts();
  clearAllPendingInterrupts();

  enableManualWakeInterrupt(); // The DS3231, which is not powered during stop mode on v0.2 hardware
  nvic_irq_enable(NVIC_RTCALARM);

  // while(1){
  //   Serial2.println("here in the loop");
  //   delay(2000);
  // }


  awakenedByUser = false; // Don't go into sleep mode with any interrupt state

  Serial2.end();

  /////WaterBear_FileSystem::closeFileSystem(); // close file, filesystem, turn off sdcard?

  enterStopMode();
  //enterSleepMode();

  Serial2.begin(SERIAL_BAUD);

  reenableAllInterrupts(iser1, iser2, iser3);
  Monitor::instance()->writeDebugMessage(F("Awakened by interrupt"));

  disableClockInterrupt();
  nvic_irq_disable(NVIC_RTCALARM);

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
}

void handleControlCommand()
{
  Monitor::instance()->writeDebugMessage(F("SERIAL2 Input Ready"));
  awakeTime = timestamp(); // Push awake time forward
  int command = WaterBear_Control::processControlCommands(Serial2);
  switch (command)
  {
  case WT_CLEAR_MODES:
  {
    Monitor::instance()->writeDebugMessage(F("Clearing Config, Debug, & TempCal modes"));
    configurationMode = false;
    debugValuesMode = false;
    tempCalMode = false;
    controlFlag = 0;
    break;
  }
  case WT_CONTROL_CONFIG:
  {
    Monitor::instance()->writeDebugMessage(F("Entering Configuration Mode"));
    Monitor::instance()->writeDebugMessage(F("Reset device to enter normal operating mode"));
    Monitor::instance()->writeDebugMessage(F("Or >WT_CLEAR_MODES<"));
    configurationMode = true;
    char *flagPtr = (char *)WaterBear_Control::getLastPayload();
    char logMessage[30];
    sprintf(&logMessage[0], "%s%s", reinterpret_cast<const char *> F("ConfigMode: "), flagPtr);
    Monitor::instance()->writeDebugMessage(logMessage);
    processControlFlag(flagPtr);
    break;
  }
  case WT_DEBUG_VAlUES:
  {
    Monitor::instance()->writeDebugMessage(F("Entering Value Debug Mode"));
    Monitor::instance()->writeDebugMessage(F("Reset device to enter normal operating mode"));
    Monitor::instance()->writeDebugMessage(F("Or >WT_CLEAR_MODES<"));
    debugValuesMode = true;
    break;
  }
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
  case WT_CAL_TEMP: // display raw temperature readings, and calibrated if available
  {
    Monitor::instance()->writeDebugMessage(F("Entering Temperature Calibration Mode"));
    Monitor::instance()->writeDebugMessage(F("Reset device to enter normal operating mode"));
    Monitor::instance()->writeDebugMessage(F("Or >WT_CLEAR_MODES<"));
    tempCalMode = true;
    break;
  }
  case WT_TEMP_CAL_LOW:
  {
    clearThermistorCalibration();
    Monitor::instance()->writeDebugMessage(F("LOW_TEMP_CALIBRATION")); // input in xxx.xxC
    unsigned short *lowTempPtr = (unsigned short *)WaterBear_Control::getLastPayload();
    unsigned short lowTemp = *lowTempPtr;
    char logMessage[30];
    sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("LOW_TEMP_CAL: "), lowTemp);
    Monitor::instance()->writeDebugMessage(logMessage);
    writeEEPROMBytes(TEMPERATURE_C1_ADDRESS_START, (unsigned char*)&lowTemp, TEMPERATURE_C1_ADDRESS_LENGTH);

    unsigned short voltage = analogRead(PB1);
    sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("LOW_TEMP_VOLTAGE: "), voltage);
    Monitor::instance()->writeDebugMessage(logMessage);
    writeEEPROMBytes(TEMPERATURE_V1_ADDRESS_START, (unsigned char*)&voltage, TEMPERATURE_V1_ADDRESS_LENGTH);
    break;
  }
  case WT_TEMP_CAL_HIGH:
  {
    Monitor::instance()->writeDebugMessage(F("HIGH_TEMP_CALIBRATION")); // input in xxx.xxC
    unsigned short *highTempPtr = (unsigned short *)WaterBear_Control::getLastPayload();
    unsigned short highTemp = *highTempPtr;
    char logMessage[30];
    sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("HIGH_TEMP_CAL: "), highTemp);
    Monitor::instance()->writeDebugMessage(logMessage);
    writeEEPROMBytes(TEMPERATURE_C2_ADDRESS_START, (unsigned char*)&highTemp, TEMPERATURE_C2_ADDRESS_LENGTH);

    unsigned short voltage = analogRead(PB1);
    sprintf(&logMessage[0], "%s%i", reinterpret_cast<const char *> F("HIGH_TEMP_VOLTAGE: "), voltage);
    Monitor::instance()->writeDebugMessage(logMessage);
    writeEEPROMBytes(TEMPERATURE_V2_ADDRESS_START, (unsigned char*)&voltage, TEMPERATURE_V2_ADDRESS_LENGTH);
    calibrateThermistor();
    break;
  }
  case WT_USER_VALUE:
  {
    Monitor::instance()->writeDebugMessage(F("USER_VALUE"));
    char *userValuePtr = (char *)WaterBear_Control::getLastPayload();
    char logMessage[24];
    sprintf(&logMessage[0], "%s%s", reinterpret_cast<const char *> F("USER_VALUE: "), userValuePtr);
    Monitor::instance()->writeDebugMessage(logMessage);
    sprintf(values[20], "%s", userValuePtr);
    break;
  }
  case WT_USER_NOTE:
  {
    Monitor::instance()->writeDebugMessage(F("USER_NOTE"));
    char *userNotePtr = (char *)WaterBear_Control::getLastPayload();
    char logMessage[42];
    sprintf(&logMessage[0], "%s%s", reinterpret_cast<const char *> F("USER_NOTE: "), userNotePtr);
    Monitor::instance()->writeDebugMessage(logMessage);
    sprintf(values[21], "%s", userNotePtr);
    break;
  }
  case WT_USER_INPUT:
  {
    Monitor::instance()->writeDebugMessage(F("USER_INPUT"));
    char *userInputPtr = (char *)WaterBear_Control::getLastPayload();
    char logMessage[55];
    sprintf(&logMessage[0], "%s%s", reinterpret_cast<const char *> F("USER_INPUT: "), userInputPtr);
    Monitor::instance()->writeDebugMessage(logMessage);
    for (size_t i = 0; i < 42 ; i++)
    {
      if (userInputPtr[i] == '&')
      {
        sprintf(values[21], "%s", &userInputPtr[i+1]);
        userInputPtr[i] = '\0';
        sprintf(values[20], "%s", userInputPtr);
        break;
      }
      else if (userInputPtr[i] == '\0')
      {
        Monitor::instance()->writeDebugMessage(F("incorrect format, delimiter is &"));
        break;
      }
    }
    break;
  }
  default:
    Monitor::instance()->writeDebugMessage(F("Invalid command code"));
    break;
  }
}

void clearThermistorCalibration()
{
  Monitor::instance()->writeDebugMessage(F("clearing thermistor EEPROM registers"));
  for (size_t i = 0; i < TEMPERATURE_BLOCK_LENGTH; i++)
  {
    writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, TEMPERATURE_C1_ADDRESS_START+i, 255);
  }
}

void calibrateThermistor() // calibrate using linear slope equation, log time
{
  //v = mc+b    m = (v2-v1)/(c2-c1)    b = (m*-c1)+v1
  //C1 C2 M B are scaled up for storage, V1 V2 are scaled up for calculation
  float c1,v1,c2,v2,m,b;
  unsigned short slope, read;
  unsigned int intercept;
  unsigned char * dataPtr = (unsigned char *)&read;
  readEEPROMBytes(TEMPERATURE_C1_ADDRESS_START, dataPtr, TEMPERATURE_C1_ADDRESS_LENGTH);
  c1 = *(unsigned short *)dataPtr;
  readEEPROMBytes(TEMPERATURE_V1_ADDRESS_START, dataPtr, TEMPERATURE_V1_ADDRESS_LENGTH);
  v1 = *(unsigned short *)dataPtr * TEMPERATURE_SCALER;
  readEEPROMBytes(TEMPERATURE_C2_ADDRESS_START, dataPtr, TEMPERATURE_C2_ADDRESS_LENGTH);
  c2 = *(unsigned short *)dataPtr;
  readEEPROMBytes(TEMPERATURE_V2_ADDRESS_START, dataPtr, TEMPERATURE_V2_ADDRESS_LENGTH);
  v2 = *(unsigned short *)dataPtr * TEMPERATURE_SCALER;
  m = (v2-v1)/(c2-c1);
  b = (((m*(0-c1)) + v1) + ((m*(0-c2)) + v2))/2; //average at two points

  slope = m * TEMPERATURE_SCALER;
  writeEEPROMBytes(TEMPERATURE_M_ADDRESS_START, (unsigned char*)&slope, TEMPERATURE_M_ADDRESS_LENGTH);
  intercept = b;
  writeEEPROMBytes(TEMPERATURE_B_ADDRESS_START, (unsigned char*)&intercept, TEMPERATURE_B_ADDRESS_LENGTH);
  unsigned int tempCalTime= timestamp();
  writeEEPROMBytes(TEMPERATURE_TIMESTAMP_ADDRESS_START, (unsigned char*)&tempCalTime, TEMPERATURE_TIMESTAMP_ADDRESS_LENGTH);
  Monitor::instance()->writeDebugMessage(F("thermistor calibration complete"));
}

float calculateTemperature()
{
  //v = mx+b  =>  x = (v-b)/m
  //C1 C2 M B are scaled up for storage, V1 V2 are scaled up for calculation
  float temperature = -1;
  if (checkThermistorCalibration() == true)
  {
    unsigned short m = 0;
    unsigned int b = 0;
    float rawData = analogRead(PB1);
    if (rawData == 0) // indicates thermistor disconnect
    {
      temperature = -2;
    }
    else
    {
      readEEPROMBytes(TEMPERATURE_M_ADDRESS_START, (unsigned char*)&m, TEMPERATURE_M_ADDRESS_LENGTH);
      readEEPROMBytes(TEMPERATURE_B_ADDRESS_START, (unsigned char *)&b, TEMPERATURE_B_ADDRESS_LENGTH);
      temperature = (rawData-(b/TEMPERATURE_SCALER))/(m/TEMPERATURE_SCALER);
    }
  }
  else
  {
    Monitor::instance()->writeDebugMessage(F("Thermistor not calibrated"));
  }
  return temperature;
}

void takeNewMeasurement()
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

  sprintf(values[19], "%i", burstCount); // log burstCount

  if (DEBUG_MEASUREMENTS)
  {
    Monitor::instance()->writeDebugMessage(F("writeLog"));
  }
  filesystem->writeLog(values, fieldCount);
  if (DEBUG_MEASUREMENTS)
  {
    Monitor::instance()->writeDebugMessage(F("writeLog done"));
  }
}

void trackBurst(bool bursting)
{
  if (bursting)
  {
    burstCount = burstCount + 1;
  }
}

// displays relevant readings based on controlFlag
void monitorConfiguration()
{
  blink(1,500); //slow down rate of responses to 1/s
  if (controlFlag == 0)
  {
    Monitor::instance()->writeDebugMessage(F("Error: no control Flag"));
  }
  if (controlFlag == 1) // time stamps
  {
    printDS3231Time();
  }
  if (controlFlag == 2) // conductivity readings
  {
    float ecValue = -1;
    bool newDataAvailable = readECDataIfAvailable(&ecValue);
    if (newDataAvailable)
    {
      char message[100];
      sprintf(message, "Got EC value: %f", ecValue);
      Monitor::instance()->writeDebugMessage(message);
    }
  }
  if (controlFlag == 3) // thermistor readings
  {
    char valuesBuffer[35];
    sprintf(valuesBuffer, "raw voltage: %i", analogRead(PB1));
    Monitor::instance()->writeDebugMessage(valuesBuffer);
  }
  //test code simplified calls to write and read eeprom
  /*
  int test = 1337;
  Serial2.println("writing 1337");
  Serial2.flush();
  writeExposedBytes(TEST_START, (unsigned char *)&test, TEST_LENGTH);

  unsigned short read = 0;
  readExposedBytes(TEST_START,(unsigned char *)&read, TEST_LENGTH);
  Serial2.print("reading:");
  Serial2.println(read);
  Serial2.flush();
  */
}

void processControlFlag(char *flag)
{
  if (strcmp(flag, "time") == 0)
  {
    controlFlag = 1;
  }
  else if(strcmp(flag, "conduct") == 0)
  {
    controlFlag = 2;
  }
  else if(strcmp(flag, "therm") == 0)
  {
    controlFlag = 3;
  }
  else
  {
    controlFlag = 0;
  }
}

void monitorValues()
{
  // print content being logged each second
  blink(1, 500);
  char valuesBuffer[180]; // 51+25+11+24+(7*5)+33
  sprintf(valuesBuffer, ">WT_VALUES: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s<",
      values[0], values[1], values[2], values[3], values[4], values[5],
      values[6],values[7], values[8], values[9], values[10]);
  Monitor::instance()->writeDebugMessage(F(valuesBuffer));

  //sprintf(valuesBuffer, "burstcount = %i current millis = %i\n", burstCount, (int)millis());
  //Monitor::instance()->writeDebugMessage(F(valuesBuffer));

  printToBLE(valuesBuffer);
}

void monitorTemperature() // print out calibration information & current readings
{
  blink(1,500);
  unsigned short c1, v1, c2, v2, m;
  unsigned int b;
  unsigned int calTime;
  unsigned char data = 0;
  unsigned char * dataPtr = &data;

  //C1 C2 M B are scaled up for storage, V1 V2 are scaled up for calculation
  readEEPROMBytes(TEMPERATURE_C1_ADDRESS_START, dataPtr, TEMPERATURE_C1_ADDRESS_LENGTH);
  c1 = *(unsigned short *)dataPtr; //4
  readEEPROMBytes(TEMPERATURE_V1_ADDRESS_START, dataPtr, TEMPERATURE_V1_ADDRESS_LENGTH);
  v1 = *(unsigned short *)dataPtr; //4
  readEEPROMBytes(TEMPERATURE_C2_ADDRESS_START, dataPtr, TEMPERATURE_C2_ADDRESS_LENGTH);
  c2 = *(unsigned short *)dataPtr; //4
  readEEPROMBytes(TEMPERATURE_V2_ADDRESS_START, dataPtr, TEMPERATURE_V2_ADDRESS_LENGTH);
  v2 = *(unsigned short *)dataPtr; //4
  readEEPROMBytes(TEMPERATURE_M_ADDRESS_START, dataPtr, TEMPERATURE_M_ADDRESS_LENGTH);
  m = *(unsigned short *)dataPtr; //4
  readEEPROMBytes(TEMPERATURE_B_ADDRESS_START, dataPtr, TEMPERATURE_B_ADDRESS_LENGTH);
  b = *(unsigned int *)dataPtr; // 5
  readEEPROMBytes(TEMPERATURE_TIMESTAMP_ADDRESS_START, dataPtr, TEMPERATURE_TIMESTAMP_ADDRESS_LENGTH);
  calTime = *(unsigned int *)dataPtr; // 10

  float temperature = calculateTemperature();

  char valuesBuffer[150];
  sprintf(valuesBuffer,"EEPROM thermistor block\n(%i,%i)(%i,%i)\nv=%ic+%i\ncalTime:%i\ntemperature:%.2fC\n", c1, v1, c2, v2, m, b, calTime, temperature);
  Monitor::instance()->writeDebugMessage(F(valuesBuffer));
}
