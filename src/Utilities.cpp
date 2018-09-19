#include "Utilities.h"

// For F103RM - these should go into their own .h
#define Serial Serial2

void scanIC2(TwoWire wire){
  Serial.println("Scanning...");
  byte error, address;
  int nDevices;
    nDevices = 0;
    for(address = 1; address < 127; address++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.

      wire.beginTransmission(address);
      error = wire.endTransmission();

      if (error == 0) {
        Serial.print("I2C device found at address 0x");
        if (address < 16)
          Serial.print("0");
        Serial.println(address, HEX);

        nDevices++;
      }
      else if (error == 4) {
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
