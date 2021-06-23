#ifndef WATERBEAR_ATLAS_OEM
#define WATERBEAR_ATLAS_OEM

#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include <EC_OEM.h>

void setupEC_OEM(TwoWire * wire);
void hibernateEC_OEM();
void clearECCalibrationData();
void setECDryPointCalibration();
void setECLowPointCalibration(float lowPoint);
void setECHighPointCalibration(float highPoint);
bool readECDataIfAvailable(float * value);

#endif