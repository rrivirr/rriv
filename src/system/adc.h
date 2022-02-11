/* 
 *  RRIV - Open Source Environmental Data Logging Platform
 *  Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef WATERBEAR_EXTERNAL_ADC
#define WATERBEAR_EXTERNAL_ADC

#include "Arduino.h"


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


class AD7091R
{

private:
  bool channel0Enabled;
  bool channel1Enabled;
  bool channel2Enabled;
  bool channel3Enabled;

  short _channel0Value;
  short _channel1Value;
  short _channel2Value;
  short _channel3Value;

  void copyBytesToRegister(byte * registerPtr, byte msb, byte lsb);
  void updateChannelRegister();
  void sendTransmission(byte registerAddress, const void * data, int numBytes);
  void sendTransmission(byte registerAddress);
  void requestBytes(byte * buffer, int length);


public:
  AD7091R();
  void configure();
  void enableChannel(short channel);
  void disableChannel(short channel);
  void convertEnabledChannels();
  void printConfigurationRegister(configuration_register configurationRegister);
  configuration_register readConfigurationRegister();
  void writeConfigurationRegister(configuration_register);
  channel_register readChannelRegister();
  void writeChannelRegister(channel_register channelConfiguration);
  conversion_result_register readConversionResultRegister();

  short getChannelValue(short channel);
  short channel0Value();
  short channel1Value();
  short channel2Value();
  short channel3Value();

};

#endif



