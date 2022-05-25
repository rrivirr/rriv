#include "logs.h"
#include "monitor.h"
#include "utilities/utilities.h"



void debug(const char* message)
{
  Monitor::instance()->writeDebugMessage(message);
}

void debug(const __FlashStringHelper * message)
{
  debug(reinterpretCharPtr(message));
}

void debug(int number)
{
  char message[10];
  sprintf(message, "%d", number);
  debug(message);
}

void debug(uint32 number)
{
  char message[10];
  sprintf(message, "%ld", number);
  debug(message);
}

void debug(short number)
{
  char message[10];
  sprintf(message, "%d", number);
  debug(message);
}

void debug(float number)
{
  char message[10];
  sprintf(message, "%f", number);
  debug(message);
}

void debug(double number)
{
  char message[10];
  sprintf(message, "%f", number);
  debug(message);
}

void notify(const char* message)
{
  Monitor::instance()->writeSerialMessage(message);
}

void notify(const __FlashStringHelper * message)
{
  notify(reinterpretCharPtr(message));
}

void notify(int number)
{
  char message[10];
  sprintf(message, "%d", number);
  notify(message);
}


void notify(unsigned int number)
{
  char message[10];
  sprintf(message, "%d", number);
  notify(message);
}

void notify(short number)
{
  char message[10];
  sprintf(message, "%d", number);
  notify(message);
}

void notify(uint32 number)
{
  char message[10];
  sprintf(message, "%ld", number);
  notify(message);
}


void notify(double number)
{
  char message[10];
  sprintf(message, "%f", number);
  notify(message);
}