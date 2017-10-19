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
