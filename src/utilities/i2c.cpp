
#include "i2c.h"
#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include "system/monitor.h"
#include "system/monitor.h"

void i2cError(int transmissionCode)
{

  switch(transmissionCode){
    case SUCCESS:
      debug("i2c success");
      break;
    case EDATA:
      debug("i2c data error");
      break;
    case ENACKADDR:
      debug("i2c address not acknowledged");
      break;
    case ENACKTRNS:
      debug("i2c transmission not acknowledged");
      break;
    case EOTHER:
      debug("i2c error: other");
      break;
    default:
      debug("i2c: unknown response code");
      break;
  }
}



void i2cSendTransmission(byte i2cAddress, byte registerAddress, const void * data, int numBytes)
{
  // char debugMessage[100];
  // sprintf(debugMessage, "i2c %d %d %d", i2cAddress, registerAddress, numBytes);
  // debug(debugMessage);

  short rval = -1;
  while (rval != 0)
  {
    Wire.beginTransmission(i2cAddress);
    Wire.write(registerAddress);

    for(int i=numBytes-1; i>=0; i--){ // correct order
    // for(int i=0; i<numBytes; i++){
      Serial2.println(i);
      Serial2.println( *( ((byte *) data) +i), BIN );
      Wire.write( ((byte *) data) +i, 1);
    }

    rval = Wire.endTransmission();
    // Serial2.println(rval);

    if(rval != 0){
      i2cError(rval);
      delay(1000); // give it a chance to fix itself
      // i2c_bus_reset(I2C1);            // consider i2c reset?
    }
  }
}

void scanIC2(TwoWire *wire)
{
  Serial.println("Scanning...");
  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    wire->beginTransmission(address);
    error = wire->endTransmission(); 
    if(address == 0x2F){
      Serial.println(address, HEX);
      Serial.println(error);
    }

    if (error == 0)
    {
      Serial.print("I2C2: I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("done");
}