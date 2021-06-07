#include "atlas_rgb.h"

// Constructor for RGB
AtlasRGB::AtlasRGB(int recv, int trans) {
  rx = recv;
  tx = trans;
  setupSerial();
  inputString = "";
  sensorString = "";
  inputStringComplete = false;
  sensorStringComplete = false;
  
}

// Setting up serial
AtlasRGB::setupSerial() {
  rgbSerial = SoftwareSerial(this->rx, this->tx);
  rgbSerial.begin(9600);  
}











