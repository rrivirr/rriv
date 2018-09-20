#include "WaterBear_FileSystem.h"

// for the data logging shield, we use digital pin 10 for the SD cs line

#define SPI1_NSS_PIN PB6    //SPI_1 Chip Select pin is PA4 by default. You can change it to the STM32 pin you want.
                            // But the SDCard shield wants PB6, i.e. D10
#define CHIP_SELECT SPI1_NSS_PIN  // not '10' like for normal arduino
                                  // note that the other SPI1 pins map just like the 328


char dataDirectory[6] = "/Data";

WaterBear_FileSystem::WaterBear_FileSystem(RTClib* rtc, char * deploymentIdentifier){

  this->rtc = rtc;

  // initialize the SD card
  Serial2.print(F("Initializing SD card..."));

  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(PA4, OUTPUT);

  pinMode(CHIP_SELECT, OUTPUT);

  // see if the card is present and can be initialized:
  if (!this->sd.begin(CHIP_SELECT, SPI_CLOCK_DIV4)) {
    Serial2.println(F(">Card fail<"));
    while(1);
  } else {
    Serial2.println(F("card initialized."));
  }

  this->setDeploymentIdentifier(deploymentIdentifier);

  this->setNewDataFile();

}

void WaterBear_FileSystem::writeLog(char **values, short fieldCount){
  for(int i=0; i<fieldCount; i++){
    //Serial2.println(values[i]);
    this->logfile.print(values[i]);
    if(i+1 < fieldCount){
      this->logfile.print(',');
    }
  }
  this->logfile.println();
  this->logfile.flush();
}

void WaterBear_FileSystem::dumpLoggedDataToStream(Stream * myStream, char * lastFileNameSent){
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

void WaterBear_FileSystem::setDeploymentIdentifier(char *newDeploymentIdentifier){
    strcpy(deploymentIdentifier, newDeploymentIdentifier);
}


void WaterBear_FileSystem::setNewDataFile() {
    DateTime now = this->rtc->now();

    char uniquename[11] = "NEWFILE";
    sprintf(uniquename, "%lu", now.unixtime());
    char suffix[5] = ".CSV";
    char filename[15];
    strncpy(filename, uniquename, 10);
    strncpy(&filename[10], suffix, 5);

    Serial2.println("cd");
    this->sd.chdir("/");
    delay(10);

    File file;
    file = this->sd.open("/COLUMNS.TXT");

    if (!file){
      Serial2.println(F("COLUMNS.TXT did not open"));
      while(1);
    }

    char line[100];
    int n = file.fgets(line, sizeof(line));
    if (line[n - 1] == '\n') {
          // remove '\n'
          line[n - 1] = 0;
    }

    Serial2.print(F(">log:"));
    Serial2.println(line);
    file.close();

    printCurrentDirListing();
    Serial2.println("OK in root");
    if(!this->sd.exists(dataDirectory)){
      Serial2.println("mkdir");
      this->sd.mkdir(dataDirectory);
      delay(10);
    }

    if (!this->sd.chdir(dataDirectory)) {
    //Serial2.println("OK");
      Serial2.println(F("failed: /Data."));
    } else {
      //Serial2.println(F("cd /Data."));
    }

    if(!this->sd.exists(this->deploymentIdentifier)){
      // printCurrentDirListing();
      Serial2.write("mkdir:");
      Serial2.println(deploymentIdentifier);
      this->sd.mkdir(deploymentIdentifier);
      //delay(10);
    }

    if (!this->sd.chdir(this->deploymentIdentifier)) {
      Serial2.print("failed:");
      Serial2.println(deploymentIdentifier);
    } else {
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
    if(!logfile){
      Serial2.print(F(">not found<"));
      while(1);
    }

    //if (!sd.chdir("/")) {
    //  Serial2.println("fail /");
    //}

    this->logfile.println(line); // write the headers to the new logfile
    this->logfile.flush();
    //Serial2.print("wrote:");
    //Serial2.println(ret);
}


void WaterBear_FileSystem::printCurrentDirListing(){
  this->sd.vwd()->rewind();
  SdFile dirFile;
  char sdFilename[30];
  while (dirFile.openNext(sd.vwd(), O_READ)) {
    dirFile.getName(sdFilename, 30);
    Serial2.println(sdFilename);
    dirFile.close();
  }
}
