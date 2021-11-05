#include "monitor.h"

void debug(const char* message)
{
  Monitor::instance()->writeDebugMessage(message);
}

void debug(const __FlashStringHelper * message)
{
  Monitor::instance()->writeDebugMessage(message);
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
  size_t s = 0;
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