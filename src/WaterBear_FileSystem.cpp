#include "WaterBear_FileSystem.h"

// for the data logging shield, we use digital pin 10 for the SD cs line

#define SPI1_NSS_PIN PB6    //SPI_1 Chip Select pin is PA4 by default. You can change it to the STM32 pin you want.
                            // But the SDCard shield wants PB6, i.e. D10
#define CHIP_SELECT SPI1_NSS_PIN  // not '10' like for normal arduino
                                  // note that the other SPI1 pins map just like the 328


char dataDirectory[6] = "/Data";

WaterBear_FileSystem::WaterBear_FileSystem(void){

/*
    if (! RTC.initialized()) {
      Serial2.println(F("RTC is NOT initialized!"));
      // following line sets the RTC to the date & time this sketch was compiled
      RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    if (!RTC.begin()) {
      Serial2.println(F(">RTC failed<"));
    } else {
      Serial2.println(F(">RTC started<"));
      //Serial2.println(RTC.now().m);
    }
*/
  //  SdFile::dateTimeCallback(dateTime);

  // initialize the SD card
  Serial2.print(F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(D13, OUTPUT);
  pinMode(PA5, OUTPUT);

  // see if the card is present and can be initialized:
  if (!this->sd.begin(CHIP_SELECT)) {
    Serial2.println(F(">Card fail<"));
    while(1);
  } else {
    Serial2.println(F("card initialized."));
  }

  this->setNewDataFile();

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

void WaterBear_FileSystem::setNewDataFile() {
    //DateTime now = RTC.now();

    //rtc_calendar_time calendarTime;
    // rtc_calendar_get_time (struct rtc_module *const module, struct rtc_calendar_time *const time)
    //http://asf.atmel.com/docs/latest/samr21/html/asfdoc_sam0_rtc_calendar_basic_use_case.html

    char uniquename[11] = "HELLO";
    //sprintf(uniquename, "%lu", now.unixtime());
    char suffix[5] = ".CSV";
    char filename[15];
    strncpy(filename, uniquename, 10);
    strncpy(&filename[10], suffix, 5);

    Serial2.println("cd");
    this->sd.chdir("/");
    delay(10);

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
    //delay(10);

    //Serial2.println("OK");


    char deploymentIdentifier[29] = "DEPLOYMENT";
    // TODO not sure what to do there
    /*readDeploymentIdentifier(deploymentIdentifier);
    unsigned char empty[1] = {0xFF};
    if(memcmp(deploymentIdentifier, empty, 1) == 0 ) {
      //Serial2.print(">NoDplyment<");
      //Serial2.flush();

      char defaultDeployment[25] = "SITENAME_00000000000000";
      writeDeploymentIdentifier(defaultDeployment);
      readDeploymentIdentifier(deploymentIdentifier);
    }
    */


    if(!this->sd.exists(deploymentIdentifier)){
      // printCurrentDirListing();
      Serial2.write("mkdir:");
      Serial2.println(deploymentIdentifier);
      this->sd.mkdir(deploymentIdentifier);
      //delay(10);
    }

    if (!this->sd.chdir(deploymentIdentifier)) {
      Serial2.print("failed:");
      Serial2.println(deploymentIdentifier);
    } else {
      Serial2.print("cd:");
      Serial2.println(deploymentIdentifier);
    }
    //delay(10);

    Serial2.print("Opening file ");
    Serial2.println(filename);
    logfile = this->sd.open(filename, FILE_WRITE);
    //delay(10);

    //sd.chdir();
    if(!logfile){
      Serial2.print(F(">not found<"));
      while(1);
    }

    //if (!sd.chdir("/")) {
    //  Serial2.println("fail /");
    //}

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
    Serial2.println(line);
    file.close();
    logfile.println(line); // write the headers to the new logfile

    Serial2.print(F(">log:"));
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
