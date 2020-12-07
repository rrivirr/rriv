#ifndef WATERBEAR_FILESYSTEM
#include "WaterBear_FileSystem.h" 
#endif

class Logger
{

public:
    bool logToFile = false;
    bool logToSerial = true;
    WaterBear_FileSystem * filesystem = NULL;

public:
    static Logger* instance();

    Logger();

    void writeSerialMessage(const char * message);
    void writeSerialMessage(const __FlashStringHelper * message);
    void writeDebugMessage(const char * message);
    void writeDebugMessage(const __FlashStringHelper * message);
    void error(const __FlashStringHelper*err);

};
