#include "ble.h"
#include "configuration.h"
#include "monitor.h"

//SPIClass SPI_2(2); //Create an SPI2 object.  This has been moved to a tweak on Adafruit_BluefruitLE_SPI
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
bool bleActive = false;

Adafruit_BluefruitLE_SPI& getBLE()
{
  return ble;
}

void bleFirstRun()
{
  // if we don't have a UUID yet, we are running for the first time
  // set a mode pin for USART1 if we need to

  if(true)
  {
    Monitor::instance()->writeDebugMessage(F("BLE First Run"));
  }

  //ble.setMode(BLUEFRUIT_MODE_COMMAND);
  //digitalWrite(D4, HIGH);

  ble.println(F("AT"));
  if(ble.waitForOK())
  {
    Monitor::instance()->writeDebugMessage(F("BLE OK"));
  }
  else
  {
    Monitor::instance()->writeDebugMessage(F("BLE Not OK"));
  }

  // Send command
  ble.println(F("AT+GAPDEVNAME=WaterBear3"));
  if(ble.waitForOK())
  {
    Monitor::instance()->writeDebugMessage(F("Got OK"));
  }
  else
  {
    Monitor::instance()->writeDebugMessage(F("BLE Error"));
    while(1);
  }
  ble.println(F("ATZ"));
  ble.waitForOK();
  Monitor::instance()->writeDebugMessage(F("Got OK"));
//  ble.setMode(BLUEFRUIT_MODE_DATA);
}

void initBLE()
{
  bool debugBLE = true;
  if(debugBLE)
  {
    Monitor::instance()->writeDebugMessage(F("Initializing the Bluefruit LE module: "));
  }
  bleActive = ble.begin(true, true);

  if(debugBLE)
  {
    if(bleActive)
    {
      Monitor::instance()->writeDebugMessage(F("Tried to init - BLE active"));
    }
    else
    {
      Monitor::instance()->writeDebugMessage(F("Tried to init - BLE NOT active"));
    }
  }
  if ( !bleActive )
  {
    if(debugBLE)
    {
      Monitor::instance()->writeDebugMessage(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    return;
    // error
  }
  else
  {
    Monitor::instance()->writeDebugMessage(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() )
    {
      Monitor::instance()->error(F("Couldn't factory reset"));
    }
    ble.println(F("AT"));
    if(ble.waitForOK())
    {
      Monitor::instance()->writeDebugMessage(F("AT OK"));
    }
    else
    {
      Monitor::instance()->writeDebugMessage(F("AT NOT OK"));
    }
    bleFirstRun();
  }
  if(debugBLE)
  {
    Monitor::instance()->writeDebugMessage(F("BLE OK!") );
  }
/*
  if ( FACTORYRESET_ENABLE )
  {
    // Perform a factory reset to make sure everything is in a known state
    Logger::instance()->writeDebugMessage(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  */

  /* Disable command echo from Bluefruit */
  //ble.echo(false);
  ble.println("+++\r\n");
}

void printToBLE(char * valuesBuffer)
{
  // Send along to BLE
  if(bleActive)
  {
    ble.println(valuesBuffer);
  }
}
