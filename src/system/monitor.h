#ifndef WATERBEAR_LOGGER
#define WATERBEAR_LOGGER

#include "filesystem.h"

// Convienience functions
void debug(const char*);
void debug(const __FlashStringHelper * message);

class Monitor
{

public:
    bool logToFile = true;
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
