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

#ifndef WATERBEAR_CONFIGURATION
#define WATERBEAR_CONFIGURATION

#define SERIAL_BAUD 115200
#define USER_WAKE_TIMEOUT           60 * 15 // Timeout after wakeup from user interaction, seconds

<<<<<<< HEAD
// DEBUG behaviors
#define DEBUG_MEASUREMENTS true // enable log messages related to measurement & bursts
#define DEBUG_LOOP false         // don't sleep
#define DEBUG_USING_SHORT_SLEEP false // sleep for a hard coded short amount of time
#define DEBUG_TO_FILE 1   // Also send debug messages to the output file [comment out to disable]


// Bluefruit on SPI
#define BLUEFRUIT_SPI_SCK   PB13
#define BLUEFRUIT_SPI_MISO  PB14
#define BLUEFRUIT_SPI_MOSI  PB15
#define BLUEFRUIT_SPI_CS    PB8  // Pullup
#define BLUEFRUIT_SPI_IRQ   PB9
#define BLUEFRUIT_SPI_RST   PC4

#define USE_EC_OEM true


=======
>>>>>>> d104042cd76c8e3d785414559ffec288efdb6847
#endif