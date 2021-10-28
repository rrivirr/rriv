#include "adc.h"
#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include "system/monitor.h"


AD7091R::AD7091R()
{
  channel0Enabled = 0;
  channel1Enabled = 0;
  channel2Enabled = 0;
  channel3Enabled = 0;
}

void AD7091R::configure()
{
  configuration_register configuration; //= this->readConfigurationRegister();
  // this->printConfigurationRegister(configuration);
  configuration.CYCLE_TIMER = 3; // default value
  configuration.CMD = 1;
  configuration.AUTO = 0;
  this->printConfigurationRegister(configuration);
  this->writeConfigurationRegister(configuration);

  struct channel_register channelRegister;
  channelRegister.CH0 = 0;
  channelRegister.CH1 = 0;
  channelRegister.CH2 = 0;
  channelRegister.CH3 = 0;
  this->writeChannelRegister(channelRegister);
}

void AD7091R::enableChannel(short channel)
{
  switch(channel){
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
  
  Serial2.println(channel0Enabled);
  Serial2.println(channel1Enabled);
  Serial2.println(channel2Enabled);
  Serial2.println(channel3Enabled);

  this->updateChannelRegister();
}

void AD7091R::disableChannel(short channel)
{
  switch(channel){
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
  struct channel_register channelRegister = this->readChannelRegister();
  channelRegister.CH0 = channel0Enabled;
  channelRegister.CH1 = channel1Enabled;
  channelRegister.CH2 = channel2Enabled;
  channelRegister.CH3 = channel3Enabled;
  this->writeChannelRegister(channelRegister);
}



void AD7091R::convertEnabledChannels()
{
  this->updateChannelRegister(); // writing to the channel register restarts the channel cycle.

  this->_channel0Value = -1;
  this->_channel1Value = -1;
  this->_channel2Value = -1;
  this->_channel3Value = -1;

  short numEnabledChannels = this->channel0Enabled + this->channel1Enabled + this->channel2Enabled + this->channel3Enabled;
  for(int i=0; i<numEnabledChannels; i++){
    conversion_result_register conversionResult = this->readConversionResultRegister();
    switch(conversionResult.CH_ID){
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
  struct configuration_register configurationRegister; // 2 bytes
  short msb = 0xFF;
  short lsb = 0xFF;

  Monitor::instance()->writeDebugMessage(F("reading configration register"));
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIGURATION_REGISTER_ADDRESS);
  Wire.endTransmission();

  while(Wire.requestFrom(ADC_I2C_ADDRESS,2) == 0){}
  while(!Wire.available()){}
  msb = (short) Wire.read();
  while(!Wire.available()){}
  lsb = (short) Wire.read();

  // Serial2.println(msb);
  // Serial2.println(lsb);
  
  this->copyBytesToRegister((byte *) &configurationRegister, msb, lsb);
  return configurationRegister;
}

conversion_result_register AD7091R::readConversionResultRegister()
{
  Monitor::instance()->writeDebugMessage(F("reading conversion result register"));
  short lsb = 0xFF;
  short msb = 0xFF;
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write((int)ADC_CONVERSION_RESULT_REGISTER_ADDRESS);
  Wire.endTransmission();

  while(Wire.requestFrom(ADC_I2C_ADDRESS,2) == 0){}
  while(!Wire.available()){}
  msb = Wire.read();
  while(!Wire.available()){}
  lsb = Wire.read();

  // Serial.println(lsb);
  // Serial.println(msb);x`

  // Serial.println("decode");

  struct conversion_result_register conversionResult;
  copyBytesToRegister((byte *)&conversionResult, msb, lsb);

  short value = 0;
  memcpy(&value, &conversionResult, 2);
  // Serial2.println(value);
  // Serial2.println(conversionResult.CH_ID);
  // Serial2.println(conversionResult.CONV_RESULT);

  return conversionResult;
}

void AD7091R::writeConfigurationRegister(configuration_register configurationRegister)
{
  Monitor::instance()->writeDebugMessage(F("writing configuration register"));
  printConfigurationRegister(configurationRegister);

  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CONFIGURATION_REGISTER_ADDRESS);
  
  Monitor::instance()->writeDebugMessage(F("bytes"));
  // Serial2.println( (byte) *( (byte *) &configurationRegister+1) );
  // Serial2.println( (byte) *( (byte *) &configurationRegister) );

  Wire.write(  (byte *) &configurationRegister+1, 1);
  Wire.write(  (byte *) &configurationRegister, 1);
  
  Wire.endTransmission();
}



channel_register AD7091R::readChannelRegister(){
  struct channel_register channelRegister;
  Monitor::instance()->writeDebugMessage(F("reading channel register"));
  short lsb = 0xFF;
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CHANNEL_REGISTER_ADDRESS);
  Wire.endTransmission();

  while(Wire.requestFrom(ADC_I2C_ADDRESS,1) == 0){}
  while(!Wire.available()){}
  lsb = Wire.read();

  // Serial2.println(lsb);
  memcpy(&channelRegister, &lsb, 1);
  return channelRegister;
}

void AD7091R::writeChannelRegister(channel_register channelConfiguration)
{
  Monitor::instance()->writeDebugMessage(F("writing channel register"));
  // Monitor::instance()->writeDebugMessage( *(byte*) &channelConfiguration);
  Wire.beginTransmission(ADC_I2C_ADDRESS);
  Wire.write(ADC_CHANNEL_REGISTER_ADDRESS);
  Wire.write((byte *) &channelConfiguration, 1);
  Wire.endTransmission();
}


void AD7091R::copyBytesToRegister(byte * registerPtr, byte msb, byte lsb)
{
  memcpy( registerPtr + 1, &msb, 1);
  memcpy( registerPtr, &lsb, 1);
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
  Serial2.println( configurationRegister.ALERT_DRIVE_TYPE );
  Serial2.println( configurationRegister.GPO_2 );
  Serial2.println( configurationRegister.RSV );
  Serial2.println( configurationRegister.FLTR );
  Serial2.println( configurationRegister.CMD );
  Serial2.println( configurationRegister.SWRS );
  Serial2.println( configurationRegister.AUTO );
  Serial2.println( configurationRegister.CYCLE_TIMER );
  Serial2.println( configurationRegister.BUSY );
  Serial2.println( configurationRegister.ALERT_EN_OR_GPO0 );
  Serial2.println( configurationRegister.ALERT_POL_OR_GPO0 );
  Serial2.println( configurationRegister.GPO1 );
  Serial2.println( configurationRegister.P_DOWN );
}