#include "write_cache.h"
#include "string.h"
#include "Arduino.h"
#include "monitor.h"

WriteCache::WriteCache(OutputDevice * outputDevice)
{
  this->outputDevice = outputDevice;
  initCache();
}


void WriteCache::writeString(char * string)
{
  if(nextPosition + strlen(string) > cacheSize - 1)
  {
    flushCache();
  }

  strcpy(&cache[nextPosition], string);
  nextPosition = nextPosition + strlen(string);
}

void WriteCache::endOfLine()
{
  if(nextPosition + 1 > cacheSize - 1)
  {
    flushCache();
  }
  strncpy(&cache[nextPosition], "\n", 1);
  nextPosition++;
}

void WriteCache::flushCache()
{
  notify("flushing cache");
  char hello[100] = "\0";
  outputDevice->writeString(hello); // why is this required??
  outputDevice->writeString(cache);
  initCache();
}

void WriteCache::initCache()
{
  memset( cache, 0, MAX_CACHE_SIZE );
  nextPosition = 0;
}