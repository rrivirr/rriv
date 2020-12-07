#include "logger.h"

Logger * logger = new Logger();

// get default logger
static Logger* instance(){
  return logger;
}

void Logger::writeSerialMessage(const char * message){
  Serial2.println(message);
  Serial2.flush();
}

void Logger::writeSerialMessage(const __FlashStringHelper * message){
  Serial2.println(message);
  Serial2.flush();
}

void Logger::writeDebugMessage(const char * message){
  if(this->logToSerial){
    this->writeSerialMessage(message);
  }

  if(this->logToFile && this->filesystem != NULL){
    this->filesystem->writeDebugMessage(message);
  }
}

void Logger::writeDebugMessage(const __FlashStringHelper * message){
  if(this->logToSerial){
    this->writeSerialMessage(message);
  }

  if(this->logToFile && this->filesystem != NULL){
    this->filesystem->writeDebugMessage(reinterpret_cast<const char *>(message));
  }
}

// A small helper
void Logger::error(const __FlashStringHelper*err) {
  writeDebugMessage(F("Error:"));
  writeDebugMessage(err);
  while (1);
}