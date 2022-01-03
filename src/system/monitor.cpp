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


Monitor * monitor = new Monitor();

// get default logger
Monitor *Monitor::instance()
{
  return monitor;
}

Monitor::Monitor() {}

void Monitor::writeSerialMessage(const char *message)
{
  Serial2.println(message);
  Serial2.flush();
}

void Monitor::writeSerialMessage(const __FlashStringHelper *message)
{
  Serial2.println(message);
  Serial2.flush();
}

void Monitor::writeDebugMessage(const char *message)
{
  if (this->debugToSerial)
  {
    this->writeSerialMessage(message);
  }

  if (this->debugToFile && this->filesystem != NULL)
  {
    this->filesystem->writeDebugMessage(message);
  }
}

void Monitor::writeDebugMessage(const int number)
{
  char message[10];
  sprintf(message, "%d", number);
  this->writeDebugMessage(message);
}

void Monitor::writeDebugMessage(int number, int base)
{
  unsigned char buf[CHAR_BIT * sizeof(long long)];
  unsigned long i = 0;
  if (number == 0)
  {
    this->writeDebugMessage('0');
    return;
  }

  while (number > 0)
  {
    buf[i++] = number % base;
    number /= base;
  }

  for (; i > 0; i--)
  {
    this->writeDebugMessage((char)(buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
  }
}

void Monitor::writeDebugMessage(const __FlashStringHelper *message)
{
  if (this->debugToSerial)
  {
    this->writeSerialMessage(message);
  }

  if (this->debugToFile && this->filesystem != NULL)
  {
    this->filesystem->writeDebugMessage(reinterpret_cast<const char *>(message));
  }
}

// A small helper
void Monitor::error(const __FlashStringHelper *err)
{
  writeDebugMessage(F("Error:"));
  writeDebugMessage(err);
  while (1)
    ;
}