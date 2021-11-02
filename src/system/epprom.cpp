#include "eeprom.h"
#include "monitor.h"
#include "utilities/STM32-UID.h"
#include "utilities/i2c.h"

void writeEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress, byte data )
{
  i2cSendTransmission(deviceaddress, eeaddress, &data, 1);
  delay(5);
}

byte readEEPROM(TwoWire * wire, int deviceaddress, byte eeaddress )
{
  byte rdata = 0xFF;
  i2cSendTransmission(deviceaddress, eeaddress, 0, 0);
  delay(5);

  short numBytes = wire->requestFrom(deviceaddress,1);
  // char debugMessage[100];
  // sprintf(debugMessage, "ee got %i bytes", numBytes);
  // debug(debugMessage);

  while(!wire->available()){} 
  rdata = wire->read();

  return(rdata);
}

void readDeploymentIdentifier(char * deploymentIdentifier)
{
  for(short i=0; i < DEPLOYMENT_IDENTIFIER_LENGTH; i++)
  {
    short address = EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START + i;
    deploymentIdentifier[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
  }
  deploymentIdentifier[DEPLOYMENT_IDENTIFIER_LENGTH] = '\0';
}

void writeDeploymentIdentifier(char * deploymentIdentifier)
{
  for(short i=0; i < DEPLOYMENT_IDENTIFIER_LENGTH; i++)
  {
    short address = EEPROM_DEPLOYMENT_IDENTIFIER_ADDRESS_START + i;
    writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address, deploymentIdentifier[i]);
  }
}

void readUniqueId(unsigned char * uuid)
{

  for(int i=0; i < UUID_LENGTH; i++)
  {
    unsigned int address = EEPROM_UUID_ADDRESS_START + i;
    uuid[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
  }

  Monitor::instance()->writeDebugMessage(F("OK.. UUID in EEPROM:")); // TODO: need to create another function and read from flash
  // Log uuid and time
  // TODO: this is confused.  each byte is 00-FF, which means 12 bytes = 24 chars in hex
  char uuidString[2 * UUID_LENGTH + 1];
  uuidString[2 * UUID_LENGTH] = '\0';
  for(short i=0; i < UUID_LENGTH; i++)
  {
    sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
  }
  Monitor::instance()->writeDebugMessage(uuidString);

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

  if(memcmp(uuid, uninitializedEEPROM, UUID_LENGTH) == 0)
  {
    Monitor::instance()->writeDebugMessage(F("Generate or Retrieve UUID"));
    getSTM32UUID(uuid);

    Monitor::instance()->writeDebugMessage(F("UUID to Write:"));
    char uuidString[2 * UUID_LENGTH + 1];
    uuidString[2 * UUID_LENGTH] = '\0';
    for(short i=0; i < UUID_LENGTH; i++)
    {
      sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }
    Monitor::instance()->writeDebugMessage(uuidString);
    
    for(int i=0; i < UUID_LENGTH; i++)
    {
      unsigned int address = EEPROM_UUID_ADDRESS_START + i;
      writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address, uuid[i]);
    }

    for(int i=0; i < UUID_LENGTH; i++)
    {
      unsigned int address = EEPROM_UUID_ADDRESS_START + i;
      uuid[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address);
    }

    Monitor::instance()->writeDebugMessage(F("UUID in EEPROM:"));
    for(short i=0; i < UUID_LENGTH; i++)
    {
        sprintf(&uuidString[2*i], "%02X", (byte) uuid[i]);
    }
    Monitor::instance()->writeDebugMessage(uuidString);
   }

}

void writeEEPROMBytes(byte address, unsigned char * data, uint8_t size) // Little Endian
{
  for (uint8_t i = 0; i < size; i++)
  {
    writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address+i, data[i]);
  }
}

void readEEPROMBytes(byte address, unsigned char * data, uint8_t size) // Little Endian
{
  for (uint8_t i = 0; i < size; i++)
  {
    data[i] = readEEPROM(&Wire, EEPROM_I2C_ADDRESS, address+i);
  }
  data[size] = '\0';
}

void clearEEPROMAddress(byte address, uint8_t length)
{
  for (uint8_t i = 0; i < length; i++)
  {
    writeEEPROM(&Wire, EEPROM_I2C_ADDRESS, address+i, EEPROM_RESET_VALUE);
  }
}