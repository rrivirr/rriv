#include "i2c.h"
#include "system/hardware.h"
#include "system/monitor.h"
#include "utilities/i2c.h"

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
  Serial.println("Scanning...");
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
      Serial.println(F("I2C: I2C device found at address 0x"));
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
      Serial.println(F("Unknown error at address 0x"));
      if (address < 16)
        Serial.println(F("0"));
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println(F("No I2C devices found"));
  else
    Serial.println(F("done"));

  return found;
}

void enableI2C1()
{
  
  i2c_disable(I2C1);
  i2c_master_enable(I2C1, 0, 0);
  Monitor::instance()->writeDebugMessage(F("Enabled I2C1"));

  delay(1000);
  // i2c_bus_reset(I2C1); // hangs here if this is called
  // Monitor::instance()->writeDebugMessage(F("Reset I2C1"));

  WireOne.begin();
  delay(1000);

  Monitor::instance()->writeDebugMessage(F("Began TwoWire 1"));

  scanIC2(&Wire);
}

void enableI2C2()
{
  i2c_disable(I2C2);
  i2c_master_enable(I2C2, 0, 0);
  Monitor::instance()->writeDebugMessage(F("Enabled I2C2"));

  //i2c_bus_reset(I2C2); // hang if this is called
  WireTwo.begin();
  delay(1000);

  Monitor::instance()->writeDebugMessage(F("Began TwoWire 2"));

  Monitor::instance()->writeDebugMessage(F("Scanning"));

  scanIC2(&WireTwo);
}