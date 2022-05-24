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

#ifndef WATERBEAR_WRITE_CACHE
#define WATERBEAR_WRITE_CACHE

#define MAX_CACHE_SIZE 1000

class OutputDevice
{
  public:
    virtual void writeString(const char * string);


};

class WriteCache 
{

  public:
  // methods
  WriteCache(OutputDevice * outputDevice);
  void writeString(const char * string);
  void endOfLine();
  void flushCache();
  void setOutputToSerial(bool);

  // variables
  unsigned int cacheSize = MAX_CACHE_SIZE; // must be MAX_CACHE_SIZE or less

  private:
  // methods
  void initCache();

  // variables
  OutputDevice * outputDevice;
  char cache[MAX_CACHE_SIZE];
  unsigned int nextPosition = 0;

  bool outputToSerial = false;

};


#endif