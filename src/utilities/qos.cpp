#include "qos.h"
#include <Arduino.h>
#include <libmaple/libmaple.h>
#include "system/monitor.h"

extern "C" char* _sbrk(int incr);
int freeMemory(){
  char top;
  Serial2.println( (int) &top );
  Serial2.println( (int) reinterpret_cast<char*>(_sbrk(0)) );

  return &top - reinterpret_cast<char*>(_sbrk(0));
}

void intentionalMemoryLeak(){
  // cause a memory leak
  char * mem = (char *) malloc(400); // intentional memory leak, big enough to get around buffering
  Serial2.println(mem); // use it so compiler doesn't remove the leak
}

void checkMemory(){
  // calculate and print free memory
  // reset the system if we are running out of memory
  char freeMemoryMessage[21];
  int freeMemoryAmount = freeMemory();
  sprintf(freeMemoryMessage, "Free Memory: %d", freeMemoryAmount);
  Monitor::instance()->Monitor::instance()->writeSerialMessage(freeMemoryMessage);
  if(freeMemoryAmount < 500){
    Monitor::instance()->Monitor::instance()->writeSerialMessage(F("Low memory, resetting!"));
    nvic_sys_reset(); // software reset, takes us back to init
  }
}