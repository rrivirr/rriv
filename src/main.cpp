#include <Arduino.h>
#include <libmaple/libmaple.h>
#include <libmaple/pwr.h> // necessary?
#include <string.h>
#include <Ezo_i2c.h>

#include "datalogger.h"
#include "scratch/dbgmcu.h"
#include "system/watchdog.h"
#include "sensors/atlas_rgb.h"
#include "utilities/qos.h"

// Setup and Loop

void setupSensors(){

  // read sensors types from EEPROM
  // malloc configuration structs
  // read configuration structs from EEPROM for each sensor type
  // run setup for each sensor

  
  // Setup RGB Sensor
  AtlasRGB::instance()->setup(&WireTwo);

}

#define ADC_I2C_ADDRESS 0x2F
#define ADC_CONVERSION_RESULT_REGISTER_ADDRESS 0x00
#define ADC_CHANNEL_REGISTER_ADDRESS 0x01
#define ADC_CONFIGURATION_REGISTER_ADDRESS 0x02

struct conversion_result_register {
  unsigned int CONV_RESULT : 12;
  unsigned int ALERT : 1;
  unsigned int CH_ID : 2;
  unsigned int RSV : 1;
};

struct channel_register {
  unsigned int CH0 : 1;
  unsigned int CH1 : 1;
  unsigned int CH2 : 1;
  unsigned int CH3 : 1;
  unsigned int RSV : 4;
};

struct configuration_register {

  unsigned int P_DOWN : 2;
  unsigned int GPO1 : 1;
  unsigned int ALERT_POL_OR_GPO0 : 1;
  unsigned int ALERT_EN_OR_GPO0 : 1;
  unsigned int BUSY : 1;
  unsigned int CYCLE_TIMER : 2;   
  unsigned int AUTO : 1;
  unsigned int SWRS : 1;
  unsigned int CMD : 1;
  unsigned int FLTR : 1;
  unsigned int RSV : 2;
  unsigned int GPO_2 : 1;
  unsigned int ALERT_DRIVE_TYPE : 1;

};

void printConfigurationRegister(configuration_register configurationRegister){
  Serial.println("Printing configuration register");
  Serial.println( configurationRegister.ALERT_DRIVE_TYPE );
  Serial.println( configurationRegister.GPO_2 );
  Serial.println( configurationRegister.RSV );
  Serial.println( configurationRegister.FLTR );
  Serial.println( configurationRegister.CMD );
  Serial.println( configurationRegister.SWRS );
  Serial.println( configurationRegister.AUTO );
  Serial.println( configurationRegister.CYCLE_TIMER );
  Serial.println( configurationRegister.BUSY );
  Serial.println( configurationRegister.ALERT_EN_OR_GPO0 );
  Serial.println( configurationRegister.ALERT_POL_OR_GPO0 );
  Serial.println( configurationRegister.GPO1 );
  Serial.println( configurationRegister.P_DOWN );

}

void copyBytesToRegister(byte * registerPtr, byte msb, byte lsb){
  memcpy( registerPtr + 1, &msb, 1);
  memcpy( registerPtr, &lsb, 1);
}

void setup(void)
{
  startSerial2();

  startCustomWatchDog();
  
  // disable unused components and hardware pins //
  componentsAlwaysOff();
  //hardwarePinsAlwaysOff(); // TODO are we turning off I2C pins still, which is wrong

  setupSwitchedPower();

  Serial2.println("hello");
  enableSwitchedPower();

  setupHardwarePins();
  Serial2.println("hello");
  Serial2.flush();

  //Serial2.println(atlasRGBSensor.get_name());
  // digitalWrite(PA4, LOW); // turn on the battery measurement

  //blinkTest();
  

  // Set up the internal RTC
  RCC_BASE->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC_BASE->APB1ENR |= RCC_APB1ENR_BKPEN;
  PWR_BASE->CR |= PWR_CR_DBP; // Disable backup domain write protection, so we can write
  

  // delay(20000);

  allocateMeasurementValuesMemory();

  setupManualWakeInterrupts();

  powerUpSwitchableComponents();

  // // Hacking talk to ADC
  struct configuration_register configurationRegister; // 2 bytes
  unsigned short value;
  short msb = 0xFF;
  short lsb = 0xFF;

  Serial.println("reading configration register");
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIGURATION_REGISTER_ADDRESS);
  Wire.endTransmission();

  Wire.requestFrom(ADC_I2C_ADDRESS,2);


  if(Wire.available()) msb = (short) Wire.read();
  if(Wire.available()) lsb = (short) Wire.read();
  Serial.println(msb);
  Serial.println(lsb);
  
  copyBytesToRegister((byte *) &configurationRegister, msb, lsb);
  
  printConfigurationRegister(configurationRegister);

  memcpy(&value, &configurationRegister, 2);
  Serial.println( "value" );
  Serial.println( value );
  Serial.println(configurationRegister.CYCLE_TIMER);

  configurationRegister.CYCLE_TIMER = 3; // don't change the default
  configurationRegister.CMD = 1;
  configurationRegister.AUTO = 0;

  memcpy(&value, &configurationRegister, 2);
  Serial.println( "value" );
  Serial.println( value );

  Serial.println("writing configuration register");
  printConfigurationRegister(configurationRegister);

  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIGURATION_REGISTER_ADDRESS);
  
  Serial.println("bytes");
  Serial.println( (byte) *( (byte *) &configurationRegister+1) );
  Serial.println( (byte) *( (byte *) &configurationRegister) );

  Wire.write(  (byte *) &configurationRegister+1, 1);
  Wire.write(  (byte *) &configurationRegister, 1);
  
  Wire.endTransmission();


  Serial.println("reading configration register");
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIGURATION_REGISTER_ADDRESS);
  Wire.endTransmission();

  Wire.requestFrom(ADC_I2C_ADDRESS,2);

  if(Wire.available()) msb = (short) Wire.read();
  if(Wire.available()) lsb = (short) Wire.read();
  Serial.println(msb);
  Serial.println(lsb);
  
  copyBytesToRegister((byte *) &configurationRegister, msb, lsb);
  memcpy(&value, &configurationRegister, 2);
  Serial.println( "value" );
  Serial.println( value );
  Serial.println( configurationRegister.CMD );

  // while(1){}

  // Channel register
  Serial.println("reading channel register");
  lsb = 0xFF;
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CHANNEL_REGISTER_ADDRESS);
  Wire.endTransmission();

  Wire.requestFrom(ADC_I2C_ADDRESS,1);

  if(Wire.available()) lsb = Wire.read();
  Serial.println(lsb);
  struct channel_register channelRegister;
  memcpy(&channelRegister, &lsb, 1);
  memcpy(&value, &channelRegister, 1);
  Serial.println( value );

  Serial.println("writing channel register");
  channelRegister.CH0 = 1;
  channelRegister.CH2 = 1;
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CHANNEL_REGISTER_ADDRESS);
  Wire.write((byte *) &channelRegister, 1);
  Wire.endTransmission();


  Serial.println("reading channel register");
  lsb = 0xFF;
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CHANNEL_REGISTER_ADDRESS);
  Wire.endTransmission();

  Wire.requestFrom(ADC_I2C_ADDRESS,1);

  if(Wire.available()) lsb = Wire.read();
  Serial.println(lsb);
  memcpy(&channelRegister, &lsb, 1);
  memcpy(&value, &channelRegister, 1);
  Serial.println( value );


  while(1){

    Serial.println("reading conversion result register");
    lsb = 0xFF;
    msb = 0xFF;
    Wire.beginTransmission(ADC_I2C_ADDRESS);
    Wire.write( (int) ADC_CONVERSION_RESULT_REGISTER_ADDRESS);
    Wire.endTransmission();

    Wire.requestFrom(ADC_I2C_ADDRESS, 2);

    if (Wire.available()) msb = Wire.read();
    if (Wire.available()) lsb = Wire.read();

    Serial.println(lsb);
    Serial.println(msb);

    Serial.println("decode");
    // value = 0;
    // memcpy( (byte *) &value + 1 , &msb, 1);
    // memcpy( (byte *) &value, &lsb, 1);
    // Serial.println(sizeof(value));
    // Serial.println(value, HEX);
    // Serial.println(value, BIN);
    // Serial.println(value);


    struct conversion_result_register conversionResultRegister;
    copyBytesToRegister((byte *) &conversionResultRegister, msb, lsb);


    value = 0;
    memcpy(&value, &conversionResultRegister, 2);
    Serial.println(value);
    Serial.println(conversionResultRegister.CH_ID);
    Serial.println(conversionResultRegister.CONV_RESULT);

    delay(1000);
  }

   // Don't respond to interrupts during setup
  disableManualWakeInterrupt();
  clearManualWakeInterrupt();

  // Clear the alarms so they don't go off during setup
  clearAllAlarms();

  initializeFilesystem();

  //initBLE();

  readUniqueId(uuid);

  setNotBursting(); // prevents bursting during first loop

  /* We're ready to go! */
  Monitor::instance()->writeDebugMessage(F("done with setup"));
  Serial2.flush();

  // setupSensors();
  // Monitor::instance()->writeDebugMessage(F("done with sensor setup"));
  // Serial2.flush();

  print_debug_status(); 
}




/* main run loop order of operation: */
void loop(void)
{

  startCustomWatchDog();
  printWatchDogStatus();

  // Get reading from RGB Sensor
  // char * data = AtlasRGB::instance()->mallocDataMemory();
  // AtlasRGB::instance()->takeMeasurement(data);
  // free(data);
 

  checkMemory();

  // allocate and free the ram to test if there is enough?
  //nvic_sys_reset - what does this do?

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

  if (awaitMeasurementTrigger) // Go to sleep
  {
    stopAndAwaitTrigger();
    return; // Go to top of loop
  }

  if (WaterBear_Control::ready(Serial2))
  {
    handleControlCommand();
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
    takeNewMeasurement();
    trackBurst(bursting);
    if (DEBUG_MEASUREMENTS)
    {
      monitorValues();
    }
  }

  if (configurationMode)
  {
    monitorConfiguration();
  }

  if (debugValuesMode)
  {
    if (burstCount == burstLength) // will cause loop() to continue until mode turned off
    {
      prepareForTriggeredMeasurement();
    }
    monitorValues();
  }

  if (tempCalMode)
  {
    monitorTemperature();
  }
}
