#ifndef WATERBEAR_FILESYSTEM
#define WATERBEAR_FILESYSTEM

// #include "Arduino.h"
#include "SdFat.h"
#include "DS3231.h"

class WaterBear_FileSystem
{

private:
  // File system object.
  SdFat sd;
  File logfile;
  char deploymentIdentifier[29];

  void printCurrentDirListing();


public:
  WaterBear_FileSystem(char * deploymentIdentifier, int chipSelectPin);
  void writeLog(char** values, short fieldCount);
  void writeDebugMessage(const char* message);
  void setDeploymentIdentifier(char * deploymentIdentifier);
  void setNewDataFile(long unixtime);
  void dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent);
  void closeFileSystem(); // close filesystem when sleeping
  // when reopening, either same file or new file
};

#endif
