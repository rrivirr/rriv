#include "Arduino.h"
#include "SdFat.h"

class WaterBear_FileSystem
{

private:
  // File system object.
  SdFat sd;
  File logfile;
  
  void printCurrentDirListing();


public:
  WaterBear_FileSystem(void);
  void dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent);
  void setNewDataFile();

};
