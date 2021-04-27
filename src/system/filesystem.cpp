#include "filesystem.h"

char dataDirectory[6] = "/Data";

WaterBear_FileSystem::WaterBear_FileSystem(char * deploymentIdentifier, int chipSelectPin)
{
  strcpy(this->deploymentIdentifier, deploymentIdentifier);

  // initialize the SD card
  Serial2.print(F("Initializing SD card..."));

  // Make sure chip select pin is set to output
  pinMode(chipSelectPin, OUTPUT);

  // see if the card is present and can be initialized:
  if(!this->sd.begin(chipSelectPin, SPI_CLOCK_DIV4))
  {
    Serial2.println(F(">Card fail<"));
    while(1);
  }
  else
  {
    Serial2.println(F("card initialized."));
  }
  this->setDeploymentIdentifier(deploymentIdentifier);
  Serial2.println("Set deployment identifier");
}

void WaterBear_FileSystem::writeLog(char **values, short fieldCount){
  for(int i=0; i<fieldCount; i++)
  {
    //Serial2.println(values[i]);
    this->logfile.print(values[i]);
    if(i+1 < fieldCount)
    {
      this->logfile.print(',');
    }
  }
  this->logfile.println();
  this->logfile.flush();
}

void WaterBear_FileSystem::writeDebugMessage(const char* message)
{
  this->logfile.print("debug,");
  this->logfile.print(message);
  this->logfile.println();
  this->logfile.flush();
}

void WaterBear_FileSystem::dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent)
{
  /*
    // Debug
    // printCurrentDirListing();

    if (!sd.chdir(dataDirectory)) {
      Serial2.println(F("fail: Data."));
      state = 0;
      return;
    }

    // Debug
    // printCurrentDirListing();
    // Serial2.println(lastDownloadDate);

    sd.vwd()->rewind();
    SdFile dirFile;
    char sdDirName[30];
    char sdFileName[30];
    short rootDirIndex = 0;
    while (dirFile.openNext(sd.vwd(), O_READ)) {
      dir_t d;
      if (!dirFile.dirEntry(&d)) {
        //Serial2.println(F("dirEntry failed"));
        state = 0;
        return;
      }
      if(!dirFile.isDir()){
        // Descend into all the deployment directories
        continue;
      }
      dirFile.getName(sdDirName, 30);
      //Serial2.write("Dir: ");
      //Serial2.println(sdDirName);

      if(! sd.chdir(sdDirName) ){
        Serial2.write("fail:");
        Serial2.println(sdDirName);
        state = 0;
        return;
      }
      dirFile.close();

      sd.vwd()->rewind();
      SdFile deploymentFile;
      while (deploymentFile.openNext(sd.vwd(), O_READ)) {
        dir_t d;
        if (!deploymentFile.dirEntry(&d)) {
          // Serial2.println(F("depl file fail"));
          state = 0;
          return;
        }
        deploymentFile.getName(sdFileName, 24);


        if(strncmp(sdFileName, lastDownloadDate, 10) > 0){
            //Serial2.println(sdFilename);

            File datafile = sd.open(sdFileName);
            // send size of transmission ?
            // Serial2.println(datafile.fileSize());
            while (datafile.available()) {
                Serial2.write(datafile.read());
            }
            datafile.close();
         }
         deploymentFile.close();
      }

      //char deploymentCompleteMessage[34] = ">WT_DEPLOYMENT_TRANSFERRED<";
      //Serial2.write(deploymentCompleteMessage);

      sd.chdir(dataDirectory);

      rootDirIndex = rootDirIndex + 1;

      // sd.vwd()->rewind();
      // printCurrentDirListing();
      // Advance to the last directoy we were at
      sd.vwd()->rewind();

      for(short i = 0; i < rootDirIndex; i = i + 1){
        dirFile.openNext(sd.vwd(), O_READ);
        dirFile.close();
      }
    }

    if (!sd.chdir("/")) {
      Serial2.println(F("fail /"));
      state = 0;
      return;
    }
*/
}

void WaterBear_FileSystem::setDeploymentIdentifier(char *newDeploymentIdentifier)
{
  strcpy(deploymentIdentifier, newDeploymentIdentifier);
}

void WaterBear_FileSystem::setNewDataFile(long unixtime)
{
  /*Serial2.println("oki");
    Serial2.flush();

    DateTime now = this->rtc->now();
    Serial2.println("ok");
    Serial2.flush();

    Serial2.println(now.unixtime());
    Serial2.flush();
*/
  Serial2.println("ok");
  Serial2.flush();
  char uniquename[11] = "NEWFILE";
  sprintf(uniquename, "%lu", unixtime);
  char suffix[5] = ".CSV";
  char filename[15];
  strncpy(filename, uniquename, 10);
  strncpy(&filename[10], suffix, 5);

  Serial2.println("cd");
  Serial2.flush();
  this->sd.chdir("/");
  delay(10);

  File columnsFile;
  columnsFile = this->sd.open("/COLUMNS.TXT");

  if(!columnsFile)
  {
    Serial2.println(F("COLUMNS.TXT did not open"));
    while(1);
  }

  char header[100];
  int n = columnsFile.fgets(header, sizeof(header));
  if(header[n - 1] == '\n')
  {
    // remove '\n'
    header[n - 1] = 0;
  }

  Serial2.print(F(">log:"));
  Serial2.println(header);
  columnsFile.close();

  printCurrentDirListing();
  Serial2.println("OK in root");
  if(!this->sd.exists(dataDirectory))
  {
    Serial2.println("mkdir");
    this->sd.mkdir(dataDirectory);
    delay(10);
  }

  if(!this->sd.chdir(dataDirectory))
  {
    //Serial2.println("OK");
    Serial2.println(F("failed: /Data."));
  }
  else
  {
    //Serial2.println(F("cd /Data."));
  }

  if(!this->sd.exists(this->deploymentIdentifier))
  {
    // printCurrentDirListing();
    Serial2.write("mkdir:");
    Serial2.println(deploymentIdentifier);
    this->sd.mkdir(deploymentIdentifier);
     //delay(10);
  }

  if(!this->sd.chdir(this->deploymentIdentifier))
  {
    Serial2.print("failed:");
    Serial2.println(deploymentIdentifier);
  }
  else
  {
    Serial2.print("cd:");
    Serial2.println(deploymentIdentifier);
  }
  //delay(10);

  Serial2.print("Opening file ");
  Serial2.println(filename);
  Serial2.flush();
  this->logfile = this->sd.open(filename, FILE_WRITE); //O_CREAT | O_WRITE | O_APPEND);
  //delay(10);

  //sd.chdir();
  if(!logfile)
  {
    Serial2.print(F(">not found<"));
    while(1);
  }

  //if (!sd.chdir("/")) {
  //  Serial2.println("fail /");
  //}

  this->logfile.println(header); // write the headers to the new logfile
  this->logfile.flush();
  //Serial2.print("wrote:");
  //Serial2.println(ret);
}

void WaterBear_FileSystem::printCurrentDirListing()
{
  this->sd.vwd()->rewind();
  SdFile dirFile;
  char sdFilename[30];
  while (dirFile.openNext(sd.vwd(), O_READ))
  {
    dirFile.getName(sdFilename, 30);
    Serial2.println(sdFilename);
    dirFile.close();
  }
}

void WaterBear_FileSystem::closeFileSystem()
{
  //this->logfile.sync();
  this->logfile.close(); // syncs then closes
  //this->sd.end // doesn't exist
}