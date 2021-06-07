#include "atlas_rgb.h"
#include <SoftwareSerial.h>

void setupRGB(int rx, int tx) {
  rgbSerial = SoftwareSerial(rx, tx);
  inputString = "";
  sensorString = "";
  inputStringComplete = false;
  
}




