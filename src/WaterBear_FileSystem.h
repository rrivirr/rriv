#include "Arduino.h"
#include "SdFat.h"
#include "DS3231.h"

class WaterBear_FileSystem
{

private:
  // File system object.
  SdFat sd;
  File logfile;
  RTClib * rtc;
  char deploymentIdentifier[29];

  void printCurrentDirListing();


public:
  WaterBear_FileSystem(RTClib* rtc, char * deploymentIdentifier);
  void writeLog(char** values, short fieldCount);
  void setDeploymentIdentifier(char * deploymentIdentifier);
  void setNewDataFile();
  void dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent);

};
