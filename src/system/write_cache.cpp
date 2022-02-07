/* 
 *  RRIV - Open Source Environmental Data Logging Platform
 *  Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

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