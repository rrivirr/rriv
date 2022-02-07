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

// #include "ble.h"
// #include "configuration.h"
// #include "monitor.h"

// //SPIClass SPI_2(2); //Create an SPI2 object.  This has been moved to a tweak on Adafruit_BluefruitLE_SPI
// Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// bool bleActive = false;

// Adafruit_BluefruitLE_SPI& getBLE()
// {
//   return ble;
// }

// void bleFirstRun()
// {
//   // if we don't have a UUID yet, we are running for the first time
//   // set a mode pin for USART1 if we need to

//   if(true)
//   {
//     debug(F("BLE First Run"));
//   }

//   //ble.setMode(BLUEFRUIT_MODE_COMMAND);
//   //digitalWrite(D4, HIGH);

//   ble.println(F("AT"));
//   if(ble.waitForOK())
//   {
//     debug(F("BLE OK"));
//   }
//   else
//   {
//     debug(F("BLE Not OK"));
//   }

//   // Send command
//   ble.println(F("AT+GAPDEVNAME=WaterBear3"));
//   if(ble.waitForOK())
//   {
//     debug(F("Got OK"));
//   }
//   else
//   {
//     debug(F("BLE Error"));
//     while(1);
//   }
//   ble.println(F("ATZ"));
//   ble.waitForOK();
//   debug(F("Got OK"));
// //  ble.setMode(BLUEFRUIT_MODE_DATA);
// }

// void initBLE()
// {
//   bool debugBLE = true;
//   if(debugBLE)
//   {
//     debug(F("Initializing the Bluefruit LE module: "));
//   }
//   bleActive = ble.begin(true, true);

//   if(debugBLE)
//   {
//     if(bleActive)
//     {
//       debug(F("Tried to init - BLE active"));
//     }
//     else
//     {
//       debug(F("Tried to init - BLE NOT active"));
//     }
//   }
//   if ( !bleActive )
//   {
//     if(debugBLE)
//     {
//       debug(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
//     }
//     return;
//     // error
//   }
//   else
//   {
//     debug(F("Performing a factory reset: "));
//     if ( ! ble.factoryReset() )
//     {
//       Monitor::instance()->error(F("Couldn't factory reset"));
//     }
//     ble.println(F("AT"));
//     if(ble.waitForOK())
//     {
//       debug(F("AT OK"));
//     }
//     else
//     {
//       debug(F("AT NOT OK"));
//     }
//     bleFirstRun();
//   }
//   if(debugBLE)
//   {
//     debug(F("BLE OK!") );
//   }
// /*
//   if ( FACTORYRESET_ENABLE )
//   {
//     // Perform a factory reset to make sure everything is in a known state
//     Logger::instance()->writeDebugMessage(F("Performing a factory reset: "));
//     if ( ! ble.factoryReset() ){
//       error(F("Couldn't factory reset"));
//     }
//   }
//   */

//   /* Disable command echo from Bluefruit */
//   //ble.echo(false);
//   ble.println("+++\r\n");
// }

// void printToBLE(char * valuesBuffer)
// {
//   // Send along to BLE
//   if(bleActive)
//   {
//     ble.println(valuesBuffer);
//   }
// }
