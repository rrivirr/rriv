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
  int chipSelectPin;
  char filename[15];
  char loggingFolder[29];

  void printCurrentDirListing();
  bool openFile(char * filename);


public:
  WaterBear_FileSystem(char * loggingFolder, int chipSelectPin);
  void initializeSDCard();
  void writeDebugMessage(const char* message);
  void setLoggingFolder(char * loggingFolder);
  void setNewDataFile(long unixtime);
  void dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent);
  void closeFileSystem(); // close filesystem when sleeping
  void reopenFileSystem(); // reopen filesystem after wakeup
  void writeString(char * dataString);
  void endOfLine();

  // deprecated
  void writeLog(char** values, short fieldCount);

};

#endif
