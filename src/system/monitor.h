#ifndef WATERBEAR_LOGGER
#define WATERBEAR_LOGGER

#include "filesystem.h"

class Monitor
{

public:
    bool logToFile = false;
    bool logToSerial = true;
    WaterBear_FileSystem * filesystem = NULL;

public:
    static Monitor* instance();

    Monitor();

    void writeSerialMessage(const char * message);
    void writeSerialMessage(const __FlashStringHelper * message);
    void writeDebugMessage(const char * message);
    void writeDebugMessage(const __FlashStringHelper * message);
    void error(const __FlashStringHelper*err);

};

#endif