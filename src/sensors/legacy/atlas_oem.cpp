/* 
 *  RRIV - Open Source Environmental Data Logging Platform
 *  Copyright (C) 20202  Zaven Arra  zaven.arra@gmail.com
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "atlas_oem.h"
#include "system/monitor.h"

EC_OEM *oem_ec;

void setupEC_OEM(TwoWire *wire)
{
  // note: this function needs to be cleaned up, debug code not necessary anymore
  debug(F("EC I2C setup"));
  debug(F("Setting up board"));

  oem_ec = new EC_OEM(wire, NONE_INT, ec_i2c_id);
  
  /*
  bool awoke = oem_ec->wakeUp();

  char message[300];
  sprintf(message, "Device addr EC: %x\nDevice type EC: %x\nFirmware EC: %x\nAwoke: %i\nHibernating: %i",
          oem_ec->getStoredAddr(), oem_ec->getDeviceType(), oem_ec->getFirmwareVersion(), awoke, oem_ec->isHibernate());
  debug(message);

  oem_ec->singleReading();
  struct param_OEM_EC parameter;
  parameter = oem_ec->getAllParam();

  debug(F("test:"));
  sprintf(message, "salinity= %f\nconductivity= %f\ntds= %f\nSalinity stable = %s",
          parameter.salinity, parameter.conductivity, parameter.tds, (oem_ec->isSalinityStable() ? "yes" : "no"));
  debug(message);
  */

  oem_ec->setLedOn(true);
  //oem_ec->setLedOn(false);
  //oem_ec->setLedOn(true);
  oem_ec->setProbeType(1.0);

  debug(F("Done with EZO I2C setup"));
}

void hibernateEC_OEM()
{
  oem_ec->setHibernate();
  delete  oem_ec;
}

void clearECCalibrationData()
{
  oem_ec->clearCalibrationData();
}

void setECDryPointCalibration()
{
  oem_ec->setCalibration(DRY_CALIBRATION);
}

void setECLowPointCalibration(float lowPoint)
{
  oem_ec->setCalibration(LOW_POINT_CALIBRATION, lowPoint);
}

void setECHighPointCalibration(float highPoint)
{
  oem_ec->setCalibration(HIGH_POINT_CALIBRATION, highPoint);
}

bool readECDataIfAvailable(float *ecValue)
{
  bool newDataAvailable = oem_ec->singleReading();

  if (newDataAvailable)
  {
    *ecValue = oem_ec->getConductivity();
    oem_ec->clearNewDataRegister();
  }
  return true;
}