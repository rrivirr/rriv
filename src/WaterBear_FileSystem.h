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
  void writeLog(char** values, short fieldCount);
  void setNewDataFile();
  void dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent);

};
