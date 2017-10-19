#include <Wire.h>

#include <SPI.h>
#include "SdFat.h"
#include "RTClib.h"
#include <EEPROM.h>
#include "TrueRandom.h"

#define LOG_INTERVAL  1000 // mills between entries
#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()


RTC_PCF8523 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// File system object.
SdFat sd;
File logfile;
File settingsFile;

// Settings
int device_id = 0;
int sensor_1 = 0;
int sensor_2 = 0;
int sensor_3 = 0;

int state = 0;


const int buttonPin = 2;     // the number of the pushbutton pin



/*
* Initialize the SD Card
*
*/

String getNewFilname(){
  DateTime now = RTC.now();
  String uniquename = String(now.unixtime(), DEC);
  String suffix = ".CSV";
  String filename = "";
  filename.concat(uniquename);
  filename.concat(suffix);
  return filename;
}

int uniqueIdAddressStart = 0;
int uniqueIdAddressEnd = 15;
unsigned char uuid[16];

void readUniqueId(){

  for(int i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
    int address = uniqueIdAddressStart + i;
    uuid[i] = EEPROM.read(address);
  }

  Serial.println("Here's the uuid in EEPROM");
  for(int i=0; i<8; i++){
    Serial.print((unsigned int) uuid[2*i], HEX);
  }
  Serial.println("");

  unsigned char uninitializedEEPROM[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  Serial.println("Here's the uninitializedEEPROM string that should be");
  for(int i=0; i<8; i++){
    Serial.print((unsigned int) uninitializedEEPROM[2*i], HEX);
  }
  Serial.println("");

  if(memcmp(uuid, uninitializedEEPROM, 16) == 0){
    Serial.println("Generate a UUID");
    // generate the unique ID
    TrueRandomClass::uuid(uuid);
    for(int i=0; i <= uniqueIdAddressEnd - uniqueIdAddressStart; i++){
      int address = uniqueIdAddressStart + i;
      EEPROM.write(address, uuid[i]);
    }
  }

}


void initializeSDCard(void) {
  Wire.begin();
  if (!RTC.begin()) {
    #if ECHO_TO_SERIAL
    Serial.println("RTC failed");
    #endif  //ECHO_TO_SERIAL
  } else {
    Serial.println("RTC started");
    //Serial.println(RTC.now().m);
  }

  String filename = getNewFilname();
  Serial.println(filename);  // Maybe running out of memory already ?

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!sd.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");

  //String message = String("Creating new file ");
  //message.concat(filename);
  //Serial.println(message);
  Serial.println(filename);
  Serial.println("a");
  logfile = sd.open("Data/" + filename, FILE_WRITE);
  if(!logfile){
    Serial.println("file.open");
    while(1);
  }
  //message = String("Opened the File ");
  //message.concat(filename);

  Serial.print("Logging to: ");
  Serial.println(filename);


  logfile.println("time,data1,data2,data3,data4,data5,data6,data7,data8");
  #if ECHO_TO_SERIAL
  Serial.println("time,data1,data2,data3,data4,data5,data6,data7,data8");
  #endif //ECHO_TO_SERIAL// attempt to write out the header to the file

}

/**************************************************************************/
/*
Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  pinMode(buttonPin, INPUT);

  Serial.begin(115200);
  //Serial.begin(9600);
  Serial.println("Light Sensor Test"); Serial.println("");

  readUniqueId();

  /* Get the CD Card going */
  initializeSDCard();

  /* We're ready to go! */

}

/**************************************************************************/
/*
Arduino loop function, called once 'setup' is complete (your own code
should go here)
*/
/**************************************************************************/
uint32_t lastTime = 0;
void loop(void)
{
  DateTime now;

  // Just trigger the dump event using a basic button
  int buttonState = digitalRead(buttonPin);

  if(buttonState == HIGH && state == 0){
    state = 1;

    //Flush the input, would be better to use a delimiter
    unsigned long now = millis ();
    while (millis () - now < 1000)
    Serial.read ();  // read and discard any input

    Serial.print(">AQ_TRANSFER_READY<");
    Serial.flush();

  } else if(state == 1){
    /*
    Serial.setTimeout(10000);
    char ack[7] = "";
    Serial.readBytesUntil('<', ack, 7);
    if(strcmp(ack, ">AQ_OK") != 0) {
      char message[30] = "";
      sprintf(message, "ERROR #%s#", ack);
      Serial.print(message);
      state = 0;
      return;
    }
    */

    // Get list of files

    char date[11] = "1112632576";
    Serial.println(date);

    //SdFile dataDirectory;
    //dirFile.open("Data");

    if (!sd.chdir("Data")) {
       Serial.println("chdir failed for Data.\n");
    }
    sd.vwd()->rewind();
    SdFile dirFile;
    while (dirFile.openNext(sd.vwd(), O_READ)) {
      dir_t d;
      if (!dirFile.dirEntry(&d)) {
        Serial.println("f.dirEntry failed");
        while(1);
      }
      char sdFilename[20];
      dirFile.getName(sdFilename, 20);

      /*
      Serial.print(sdFilename);
      Serial.print(" ");
      Serial.print(strncmp(sdFilename, date, 10) );
      Serial.println();
      */

      if(strncmp(sdFilename, date, 10) > 0){
        //Serial.println(sdFilename);

        File datafile = sd.open(sdFilename);
        // send size of transmission ?
        // Serial.println(datafile.fileSize());
        while (datafile.available()) {
          Serial.write(datafile.read());
        }
        datafile.close();
      }
      dirFile.close();

    }
    if (!sd.chdir()) {
       Serial.println("chdir failed for ../\n");
    }
    state = 0;


    // So what is our strategy for dealing with files?
    // Get the files past a certain date, I guess?
    // But how would we compare this ?
    // Filenames are gonna be date strings, not numbers
    // and even so...

    /*
    File datafile = sd.open("DATALOG.TXT", FILE_WRITE);

    // send size of transmission
    // Serial.println(datafile.fileSize());
    // return;

    while (datafile.available()) {
    Serial.write(datafile.read());
  }
  datafile.close();

  Serial.print("<"); // The byte to stop reading when we don't send total transfer size
  Serial.flush();
  state = 0;
}
*/
} else {
  //Serial.print("STATE_0");
}


// Fetch the time
now = RTC.now();
uint32_t elapsedTime = now.unixtime() - lastTime;
if(elapsedTime < 5){
  return;
}
lastTime = now.unixtime();


// log time
logfile.print(now.unixtime()); // seconds since 2000
logfile.print(",");

#if ECHO_TO_SERIAL
Serial.print(now.unixtime()); // seconds since 2000
Serial.print(",");
/* Display the results (light is measured in lux) */


/*Serial.print(now.year(), DEC);
Serial.print("/");
Serial.print(now.month(), DEC);
Serial.print("/");
Serial.print(now.day(), DEC);
Serial.print(" ");
Serial.print(now.hour(), DEC);
Serial.print(":");
Serial.print(now.minute(), DEC);
Serial.print(":");
Serial.print(now.second(), DEC);
Serial.println();
*/
#endif

logfile.println();
logfile.flush();

delay(250);

}



/* Apply the value to the parameter by searching for the parameter name
Using String.toInt(); for Integers
toFloat(string); for Float
toBoolean(string); for Boolean
toLong(string); for Long
*/
void applySetting(String settingName, String settingValue) {
  if(settingName == "device_id") {
    device_id=settingValue.toInt();
  }
  if(settingName == "sensor_1") {
    sensor_1=settingValue.toInt();
  }
  if(settingName == "sensor_2") {
    sensor_2=settingValue.toInt();
  }
  if(settingName == "sensor_3") {
    sensor_3=settingValue.toInt();
  }
}


// Saved settings should go into the EEPROM
// not be saved here

// Work with saved settings
// from AlexShu http://overskill.alexshu.com/saving-loading-settings-on-sd-card-with-arduino/
void readSDSettings(){
  char character;
  String settingName;
  String settingValue;
  settingsFile = sd.open("settings.txt");
  if (settingsFile) {
    while (settingsFile.available()) {
      character = settingsFile.read();
      while((settingsFile.available()) && (character != '[')){
        character = settingsFile.read();
      }
      character = settingsFile.read();
      while((settingsFile.available()) && (character != '=')){
        settingName = settingName + character;
        character = settingsFile.read();
      }
      character = settingsFile.read();
      while((settingsFile.available()) && (character != ']')){
        settingValue = settingValue + character;
        character = settingsFile.read();
      }
      if(character == ']'){

        /*
        //Debuuging Printing
        Serial.print("Name:");
        Serial.println(settingName);
        Serial.print("Value :");
        Serial.println(settingValue);
        */

        // Apply the value to the parameter
        applySetting(settingName,settingValue);
        // Reset Strings
        settingName = "";
        settingValue = "";
      }
    }
    // close the file:
    settingsFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening settings.txt");
  }
}

// converting string to Float
float toFloat(String settingValue){
  char floatbuf[settingValue.length()+1];
  settingValue.toCharArray(floatbuf, sizeof(floatbuf));
  float f = atof(floatbuf);
  return f;
}

long toLong(String settingValue){
  char longbuf[settingValue.length()+1];
  settingValue.toCharArray(longbuf, sizeof(longbuf));
  long l = atol(longbuf);
  return l;
}

// Converting String to integer and then to boolean
// 1 = true
// 0 = false
boolean toBoolean(String settingValue) {
  if(settingValue.toInt()==1){
    return true;
  } else {
    return false;
  }
}

// Writes A Configuration file
void writeSDSettings() {
  // Delete the old One
  sd.remove("settings.txt");
  // Create new one
  settingsFile = sd.open("settings.txt", FILE_WRITE);
  // writing in the file works just like regular print()/println() function
  settingsFile.print("[");
  settingsFile.print("device_id=");
  settingsFile.print(device_id);
  settingsFile.println("]");
  settingsFile.print("[");
  settingsFile.print("sensor_1=");
  settingsFile.print(sensor_1);
  settingsFile.println("]");
  settingsFile.print("[");
  settingsFile.print("sensor_2=");
  settingsFile.print(sensor_2);
  settingsFile.println("]");
  settingsFile.print("[");
  settingsFile.print("sensor_3=");
  settingsFile.print(sensor_3);
  settingsFile.println("]");
  // close the file:
  settingsFile.close();
  //Serial.println("Writing done.");
}
