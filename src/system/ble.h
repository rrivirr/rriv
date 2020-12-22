#ifndef WATERBEAR_BLE
#define WATERBEAR_BLE

#include "Adafruit_BluefruitLE_SPI.h"

Adafruit_BluefruitLE_SPI& getBLE();
void bleFirstRun();
void initBLE();
void printToBLE(char * valuesBuffer);

#endif