#include "Utilities.h"
#include "Configuration.h"
#include "WaterBear_Control.h"

// For F103RM - these should go into their own .h
#define Serial Serial2


// TODO: change Serial.print calls to writeDebugMessage but after filesystem is writable

void scanIC2(TwoWire * wire){
  Serial.println("Scanning...");
  byte error, address;
  int nDevices;
    nDevices = 0;
    for(address = 1; address < 127; address++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.

      wire->beginTransmission(address);
      error = wire->endTransmission();

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


void printInterruptStatus(HardwareSerial &serial){
    serial.println("NVIC->ISER:");
    serial.println(NVIC_BASE->ISER[0], BIN);
    serial.println(NVIC_BASE->ISER[1], BIN);
    serial.println(NVIC_BASE->ISER[2], BIN);
    serial.println(EXTI_BASE->PR, BIN);
    serial.flush();
}


void printDateTime(HardwareSerial &serial, DateTime now){
  serial.println(now.unixtime());

  serial.print(now.year(), DEC);
  serial.print('/');
  serial.print(now.month(), DEC);
  serial.print('/');
  serial.print(now.day(), DEC);
  // Serial.print(" (");
  // Serial.print(now.dayOfTheWeek());
  // Serial.print(") ");
  serial.print("  ");
  serial.print(now.hour(), DEC);
  serial.print(':');
  serial.print(now.minute(), DEC);
  serial.print(':');
  serial.print(now.second(), DEC);
  serial.println();
  serial.flush();
}

void writeEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress, byte data )
{
  wire->beginTransmission(deviceaddress);
  wire->write(eeaddress);
  wire->write(data);
  wire->endTransmission();

  delay(5);
}

byte readEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress )
{
  byte rdata = 0xFF;

  wire->beginTransmission(deviceaddress);
  wire->write(eeaddress);
  wire->endTransmission();

  wire->requestFrom(deviceaddress,1);

  if (wire->available()) rdata = wire->read();

  return rdata;
}


void blink(int times, int duration){
  pinMode(PA5, OUTPUT);
  for (int i = times; i > 0; i--)
  {
    digitalWrite(PA5, HIGH);
    delay(duration);
    digitalWrite(PA5, LOW);
    delay(duration);
  }
}

void printDS3231Time(){
    char testTime[11]; // timestamp responses
    Serial2.print(F("configMode TS:"));
    sprintf(testTime, "%lld", WaterBear_Control::timestamp()); // convert time_t value into string
    Serial2.println(testTime);
    Serial2.flush();
}



