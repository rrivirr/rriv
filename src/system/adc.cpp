#include "adc.h"
#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include "system/monitor.h"
#include "system/watchdog.h"
#include "utilities/i2c.h"

AD7091R::AD7091R()
{
  channel0Enabled = 0;
  channel1Enabled = 0;
  channel2Enabled = 0;
  channel3Enabled = 0;
}

void AD7091R::configure()
{
  configuration_register configurationGet = this->readConfigurationRegister();
  this->printConfigurationRegister(configurationGet);

  configuration_register configuration; //= this->readConfigurationRegister();
  // this->printConfigurationRegister(configuration);
  configuration.CYCLE_TIMER = 3; // default value
  configuration.CMD = 1;
  configuration.AUTO = 0;
  // this->printConfigurationRegister(configuration);
  this->writeConfigurationRegister(configuration);

  configuration_register configurationSet = this->readConfigurationRegister();
  this->printConfigurationRegister(configurationSet);

  
  struct channel_register channelRegister;
  channelRegister.CH0 = 0;
  channelRegister.CH1 = 0;
  channelRegister.CH2 = 0;
  channelRegister.CH3 = 0;
  this->writeChannelRegister(channelRegister);
}

void AD7091R::enableChannel(short channel)
{
  switch (channel)
  {
  case 0:
    channel0Enabled = 1;
    break;
  case 1:
    channel1Enabled = 1;
    break;
  case 2:
    channel2Enabled = 1;
    break;
  case 3:
    channel3Enabled = 1;
    break;
  } 

  this->updateChannelRegister();

  configuration_register configuration;
  configuration.CYCLE_TIMER = 3; // default value
  configuration.CMD = 1;
  configuration.AUTO = 0;
  // this->printConfigurationRegister(configuration);
  this->writeConfigurationRegister(configuration);
  
  configuration_register configurationSet = this->readConfigurationRegister();
  this->printConfigurationRegister(configurationSet);
}

void AD7091R::disableChannel(short channel)
{
  switch (channel)
  {
  case 0:
    channel0Enabled = 0;
    break;
  case 1:
    channel1Enabled = 0;
    break;
  case 2:
    channel2Enabled = 0;
    break;
  case 3:
    channel3Enabled = 0;
    break;
  }

  this->updateChannelRegister();
}

void AD7091R::updateChannelRegister()
{
  debug("update channel register");
  struct channel_register channelRegister = this->readChannelRegister();
  channelRegister.CH0 = channel0Enabled;
  channelRegister.CH1 = channel1Enabled;
  channelRegister.CH2 = channel2Enabled;
  channelRegister.CH3 = channel3Enabled;
  this->writeChannelRegister(channelRegister);
}

void AD7091R::convertEnabledChannels()
{
  debug("converting enabled channels");
  this->updateChannelRegister(); // writing to the channel register restarts the channel cycle.

  this->_channel0Value = -1;
  this->_channel1Value = -1;
  this->_channel2Value = -1;
  this->_channel3Value = -1;

  short numEnabledChannels = this->channel0Enabled + this->channel1Enabled + this->channel2Enabled + this->channel3Enabled;
  for (int i = 0; i < numEnabledChannels; i++)
  {
    conversion_result_register conversionResult = this->readConversionResultRegister();
    switch (conversionResult.CH_ID)
    {
    case 0:
      this->_channel0Value = conversionResult.CONV_RESULT;
      break;
    case 1:
      this->_channel1Value = conversionResult.CONV_RESULT;
      break;
    case 2:
      this->_channel2Value = conversionResult.CONV_RESULT;
      break;
    case 3:
      this->_channel3Value = conversionResult.CONV_RESULT;
      break;
    }
  }
}

configuration_register AD7091R::readConfigurationRegister()
{
  Monitor::instance()->writeDebugMessage(F("reading configuration register"));
  struct configuration_register configurationRegister; // 2 bytes
  this->sendTransmission(ADC_CONFIGURATION_REGISTER_ADDRESS);
  this->requestBytes((byte *)&configurationRegister, 2);
  return configurationRegister;
}

void AD7091R::writeConfigurationRegister(configuration_register configurationRegister)
{
  Monitor::instance()->writeDebugMessage(F("writing configuration register"));
  printConfigurationRegister(configurationRegister);
  Serial2.println(  *((byte *) &configurationRegister+1), BIN);
  Serial2.println(  *((byte *) &configurationRegister), BIN);
  this->sendTransmission(ADC_CONFIGURATION_REGISTER_ADDRESS, &configurationRegister, 2);
}

channel_register AD7091R::readChannelRegister()
{
  Monitor::instance()->writeDebugMessage(F("reading channel register"));
  struct channel_register channelRegister;
  this->sendTransmission(ADC_CHANNEL_REGISTER_ADDRESS);
  this->requestBytes((byte *)&channelRegister, 1);
  return channelRegister;
}

void AD7091R::writeChannelRegister(channel_register channelConfiguration)
{
  Monitor::instance()->writeDebugMessage(F("writing channel register"));
  // Monitor::instance()->writeDebugMessage( *(byte*) &channelConfiguration);
  Serial2.println(*(byte*) &channelConfiguration );
  this->sendTransmission(ADC_CHANNEL_REGISTER_ADDRESS, (byte *)&channelConfiguration, 1);
}

conversion_result_register AD7091R::readConversionResultRegister()
{
  Monitor::instance()->writeDebugMessage(F("reading conversion result register"));
  struct conversion_result_register conversionResult;
  this->sendTransmission(ADC_CONVERSION_RESULT_REGISTER_ADDRESS);
  this->requestBytes((byte *)&conversionResult, 2);
  return conversionResult;
}

////

void AD7091R::sendTransmission(byte registerAddress)
{
  this->sendTransmission(registerAddress, 0, 0);
}

void AD7091R::sendTransmission(byte registerAddress, const void * data, int numBytes)
{
  // Serial2.println( "IN SEND TRANSMISSION" );
  // Serial2.println(registerAddress);
  // if(numBytes > 0){
  // Serial2.println(  *((byte *) data+1), BIN);
  // Serial2.println(  *((byte *) data), BIN);
  // }
  i2cSendTransmission(ADC_I2C_ADDRESS, registerAddress, data, numBytes);
}

void AD7091R::requestBytes(byte *buffer, int length)
{
  // char debugMessage[100];
  // sprintf(debugMessage, "reading bytes %i", length);
  // debug(debugMessage);

  short numBytes = Wire.requestFrom(ADC_I2C_ADDRESS, length);
  // sprintf(debugMessage, "got %i bytes", numBytes);
  // debug(debugMessage);

  short lsb = 0xFF;
  short msb = 0xFF;

  // Serial2.println(Wire.available());
  while (!Wire.available())
  {
    delay(100);
  }
  // debug("avail 1");
  msb = (short)Wire.read();
  if (length == 2)
  {
    // debug("get 2nd byte");
    while (!Wire.available())
    {
      delay(100);
    }
    // debug("avail 2");
    lsb = (short)Wire.read();
  }

  // Serial2.println(msb);
  // Serial2.println(lsb);
  // doing this a dumb way for the moment, extend later for other devices
  if (length == 1)
  {
    memcpy(buffer, &msb, 1);
  }
  else if (length == 2)
  {
    this->copyBytesToRegister(buffer, msb, lsb);
  }
}

void AD7091R::copyBytesToRegister(byte *registerPtr, byte msb, byte lsb)
{
  memcpy(registerPtr + 1, &msb, 1);
  memcpy(registerPtr, &lsb, 1);
}

short AD7091R::channel0Value()
{
  return this->_channel0Value;
}

short AD7091R::channel1Value()
{
  return this->_channel1Value;
}

short AD7091R::channel2Value()
{
  return this->_channel2Value;
}

short AD7091R::channel3Value()
{
  return this->_channel3Value;
}

void AD7091R::printConfigurationRegister(configuration_register configurationRegister)
{
  Monitor::instance()->writeDebugMessage(F("Printing configuration register"));
  char buffer[100];
  sprintf(buffer,
          "%d %d %d %d %d %d %d %d %d %d %d %d %d",
          configurationRegister.ALERT_DRIVE_TYPE,
          configurationRegister.GPO_2,
          configurationRegister.RSV,
          configurationRegister.FLTR,
          configurationRegister.CMD,
          configurationRegister.SWRS,
          configurationRegister.AUTO,
          configurationRegister.CYCLE_TIMER,
          configurationRegister.BUSY,
          configurationRegister.ALERT_EN_OR_GPO0,
          configurationRegister.ALERT_POL_OR_GPO0,
          configurationRegister.GPO1,
          configurationRegister.P_DOWN);
  Monitor::instance()->writeDebugMessage(buffer);
}