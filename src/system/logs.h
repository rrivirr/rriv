#ifndef WATERBEAR_LOGS
#define WATERBEAR_LOGS

#include <Arduino.h>

// Convienience functions
void debug(const char*);
void debug(const __FlashStringHelper * message);
void debug(short number);
void debug(int number);
void debug(uint32 number);
void debug(float number);
void debug(double number);


void notify(const char*);
void notify(const __FlashStringHelper * message);
void notify(short number);
void notify(int number);
void notify(unsigned int number);
void notify(uint32 number);
void notify(double number);

#endif