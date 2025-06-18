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

#include "i2c.h"
#include "system/hardware.h"
#include "system/logs.h"
#include "utilities/i2c.h"

void i2cError(int transmissionCode)
{

  switch(transmissionCode){
    case SUCCESS:
      debug(F("i2c success"));
      break;
    case EDATA:
      debug(F("i2c data error"));
      break;
    case ENACKADDR:
      debug(F("i2c address not ack'd"));
      break;
    case ENACKTRNS:
      debug(F("i2c transmission not ack;d"));
      break;
    case EOTHER:
      debug(F("i2c error: other"));
      break;
    default:
      debug(F("i2c: unknown response code"));
      break;
  }
}



void i2cSendTransmission(byte i2cAddress, byte registerAddress, const void * data, int numBytes)
{
  // char debugMessage[100];
  // sprintf(debugMessage, "i2c %X %X %d", i2cAddress, registerAddress, numBytes);
  // debug(debugMessage);

  short rval = -1;
  while (rval != 0)
  {
    Wire.beginTransmission(i2cAddress);
    Wire.write(registerAddress);

    for(int i=numBytes-1; i>=0; i--){ // correct order
      Wire.write( ((byte *) data) +i, 1);
    }

    rval = Wire.endTransmission();

    if(rval != 0){
      i2cError(rval);
      delay(1000); // give it a chance to fix itself
      // i2c_bus_reset(I2C1);            // consider i2c reset?
    }
  }
}

void scanIC2(TwoWire *wire)
{
  scanIC2(wire, -1);
}

bool scanIC2(TwoWire *wire, int searchAddress)
{
  Serial.println("Scanning");
  byte error, address;
  int nDevices;
  nDevices = 0;
  bool found = false;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    wire->beginTransmission(address);
    error = wire->endTransmission(); 
    if (error == 0)
    {
      Serial.print(F("I2C dev at addr 0x"));
      if (address < 16)
        Serial.println(F("0"));
      Serial.println(address, HEX);
      if(address == searchAddress)
      {
        found = true;
      }
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print(F("Unknown error at addr 0x"));
      if (address < 16)
        Serial.println(F("0"));
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println(F("No I2C devices found"));

  return found;
}


// #define US_PER_SECOND 1000000
#define SDA1 PB7
#define SCL1 PB6

void enableI2C1()
{
  //recover the i2c if we need to

  // WireOne.end();
  // i2c_disable(I2C1);

  pinMode(SDA1, OUTPUT); // SDA
  pinMode(SCL1, OUTPUT); // SCL
  digitalWrite(SDA1, LOW);
  digitalWrite(SCL1, LOW);
  delay(2000);


  digitalWrite(SDA1, HIGH);


  //manually try to unhang the i2c1
  int us_per_second = 1000000;
  int delay_us = us_per_second / (100000 * 2);
  debug(delay_us);
  delayMicroseconds(delay_us);  // wait one period


  // The i2c clock idles high
  digitalWrite(SCL1, HIGH);
  delay(delay_us);  // wait one period

  bool was_hung = false;
  bool fixed = false;

  for(int i=0; i<50; i++){ // i is max clock cycles
    if( digitalRead(SDA1) == HIGH){
      fixed = true;
      break;
    }

    was_hung = true;

    digitalWrite(SCL1, LOW);
    delay(delay_us);  // wait one period

    digitalWrite(SCL1, HIGH);
    delay(delay_us);  // wait one period          
  }

  if(fixed) {
    if(was_hung){
      debug(F("Fixed hung bus"));
    } else {
      debug(F("Bus not hung"));
    }
  } else {
    debug(F("Bus still hung"));
    delay(4000);
    nvic_sys_reset();
  }
   

  // delay(1000);
  i2c_master_enable(I2C1, 0, 0);
  debug(F("Enabled I2C1"));

  // delay(500);
  // i2c_bus_reset(I2C1); // hangs here if this is called
  // debug(F("Reset I2C1"));

  WireOne.begin();
  delay(250);

  debug(F("Began TwoWire 1"));
  
  debug(F("Scanning 1"));

  scanIC2(&Wire);
}

void enableI2C2()
{
  i2c_disable(I2C2);
  i2c_master_enable(I2C2, 0, 0);
  debug(F("Enabled I2C2"));

  //i2c_bus_reset(I2C2); // hang if this is called
  WireTwo.begin();
  delay(250);

  debug(F("Began TwoWire 2"));

  debug(F("Scanning 2"));

  scanIC2(&WireTwo);
}