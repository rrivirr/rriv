#include "filesystem.h"
#include "clock.h"
#include "monitor.h"

char dataDirectory[6] = "/Data";


WaterBear_FileSystem::WaterBear_FileSystem(char * deploymentIdentifier, int chipSelectPin)
{
  strcpy(this->deploymentIdentifier, deploymentIdentifier);
  this->chipSelectPin = chipSelectPin;
  this->initializeSDCard();
 
  this->setDeploymentIdentifier(deploymentIdentifier);
  Serial2.println("Set deployment identifier");
}

void WaterBear_FileSystem::initializeSDCard(){
   // initialize the SD card
  Serial2.print(F("Initializing SD card..."));

  // Make sure chip select pin is set to output
  pinMode(this->chipSelectPin, OUTPUT);

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

bool WaterBear_FileSystem::openFile(char * filename){
  this->sd.chdir("/");
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

  if(!this->sd.exists(deploymentIdentifier))
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
    return false;
  }

  return true;

}

void WaterBear_FileSystem::setNewDataFile(long unixtime)
{

  char uniquename[11] = "NEWFILE";
  sprintf(uniquename, "%lu", unixtime);
  char suffix[5] = ".CSV";
  strncpy(filename, uniquename, 10);
  strncpy(&filename[10], suffix, 5);

  Serial2.println("cd");
  Serial2.flush();
  this->sd.chdir("/");
  delay(1);

  char header[200] = "duuid,uuid,time.s,time.h,battery.V,A2-PB1.V,A3-PC0.V,A4-PC1.V,A5-PC2.V,A6-PC3.V,conductivity.mS,time.TC,C1,V1,C2,V2,M,B,temperature.C,Burst,UserValue,UserNote";

  Serial2.print(F(">log:"));
  Serial2.println(header);
  

  bool success = this->openFile(filename);
  if( !success )
  {
    Serial2.print(F("filesystem open failure"));
    while(1);
  }

  this->logfile.println(header); // write the headers to the new logfile
  this->logfile.flush();
  //Serial2.print("wrote:");
  //Serial2.println(ret);
}

void WaterBear_FileSystem::printCurrentDirListing()
{
  Serial2.println("printCurrentDirListing");

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
  Serial2.print(F(">close filesystem<"));
  //this->logfile.sync();
  this->logfile.close(); // syncs then closes
  //this->sd.end // doesn't exist
}

void WaterBear_FileSystem::reopenFileSystem()
{

  initializeSDCard();
  bool success = this->openFile(filename);
  if( !success )
  {
    Monitor::instance()->writeDebugMessage(F("Could not reopen file"));
    time_t setupTime = timestamp();
    char setupTS[21];
    sprintf(setupTS, "unixtime: %lld", setupTime);
    Monitor::instance()->writeDebugMessage(setupTS);
    setNewDataFile(setupTime); // open a new file via epoch timestamp
  }
  else 
  {
    Monitor::instance()->writeDebugMessage(F("Successfully reopened file"));
  }

}