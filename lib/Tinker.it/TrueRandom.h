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

/**
 * TrueRandom - A true random number generator for Arduino.
 *
 * Copyright (c) 2010 Peter Knight, Tinker.it! All rights reserved.
 */

#ifndef TrueRandom_h
#define TrueRandom_h

#include <inttypes.h>
class TrueRandomClass
{
  public:
    static int rand();
    static long random();
    static long random(long howBig);
    static long random(long howsmall, long how);
    static int randomBit(void);
    static char randomByte(void);
    static void memfill(char* location, int size);
    static void mac(uint8_t* macLocation);
    static void uuid(uint8_t* uuidLocation);
  private:
    static int randomBitRaw(void);
    static int randomBitRaw2(void);
};
extern TrueRandomClass TrueRandom;
#endif
