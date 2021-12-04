#ifndef WATERBEAR_LOGGER
#define WATERBEAR_LOGGER

#include "filesystem.h"

// Convienience functions
void debug(const char*);
void debug(const __FlashStringHelper * message);
void debug(short number);
void debug(int number);

void notify(const char*);
void notify(const __FlashStringHelper * message);
void notify(short number);
void notify(int number);

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
