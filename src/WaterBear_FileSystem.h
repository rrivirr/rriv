#include "Arduino.h"
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
  WaterBear_FileSystem(char * deploymentIdentifier);
  void writeLog(char** values, short fieldCount);
  void setDeploymentIdentifier(char * deploymentIdentifier);
  void setNewDataFile(long unixtime);
  void dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent);

};
