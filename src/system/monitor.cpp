#include "monitor.h"

Monitor * monitor = new Monitor();

// get default logger
Monitor* Monitor::instance()
{
  return monitor;
}

Monitor::Monitor(){}

void Monitor::writeSerialMessage(const char * message)
{
  Serial2.println(message);
  Serial2.flush();
}

void Monitor::writeSerialMessage(const __FlashStringHelper * message)
{
  Serial2.println(message);
  Serial2.flush();
}

void Monitor::writeDebugMessage(const char * message)
{
  if(this->logToSerial)
  {
    this->writeSerialMessage(message);
  }

  if(this->logToFile && this->filesystem != NULL)
  {
    this->filesystem->writeDebugMessage(message);
  }
}

void Monitor::writeDebugMessage(const __FlashStringHelper * message)
{
  if(this->logToSerial)
  {
    this->writeSerialMessage(message);
  }

  if(this->logToFile && this->filesystem != NULL)
  {
    this->filesystem->writeDebugMessage(reinterpret_cast<const char *>(message));
  }
}

// A small helper
void Monitor::error(const __FlashStringHelper*err)
{
  writeDebugMessage(F("Error:"));
  writeDebugMessage(err);
  while (1);
}