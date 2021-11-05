#include "i2c.h"
#include <libmaple/libmaple.h>
#include "system/monitor.h"
#include "system/hardware.h"


void scanIC2(TwoWire *wire)
{
  Monitor::instance()->writeDebugMessage(F("Scanning..."));
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
    if (error == 0)
    {
      Monitor::instance()->writeDebugMessage(F("I2C2: I2C device found at address 0x"));
      if (address < 16)
        Monitor::instance()->writeDebugMessage(F("0"));
      Monitor::instance()->writeDebugMessage(address, HEX);
      nDevices++;
    }
    else if (error == 4)
    {
      Monitor::instance()->writeDebugMessage(F("Unknown error at address 0x"));
      if (address < 16)
        Monitor::instance()->writeDebugMessage(F("0"));
      Monitor::instance()->writeDebugMessage(address, HEX);
    }
  }
  if (nDevices == 0)
    Monitor::instance()->writeDebugMessage(F("No I2C devices found"));
  else
    Monitor::instance()->writeDebugMessage(F("done"));
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