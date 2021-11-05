#ifndef WATERBEAR_LOGGER
#define WATERBEAR_LOGGER

#include "filesystem.h"

void debug(const char* message);
void debug(const __FlashStringHelper * message);

class Monitor
{

public:
    bool debugToFile = false;
    bool debugToSerial = false;
    WaterBear_FileSystem * filesystem = NULL;

public:
    static Monitor* instance();

    Monitor();

    void writeSerialMessage(const char * message);
    void writeSerialMessage(const __FlashStringHelper * message);
    void writeDebugMessage(const char * message);
    void writeDebugMessage(const __FlashStringHelper * message);
    void writeDebugMessage(int message);
    void writeDebugMessage(int message, int base);

    void error(const __FlashStringHelper*err);

};

#endif
