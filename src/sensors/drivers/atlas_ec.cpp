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

#include "atlas_ec.h"
#include "system/monitor.h"
#include "system/measurement_components.h"
#include "system/eeprom.h" // TODO: ideally not included in this scope
#include "system/clock.h"  // TODO: ideally not included in this scope
#include "sensors/sensor_types.h"

AtlasECDriver::AtlasECDriver()
{
  debug("allocation GenericAtlas");
}

AtlasECDriver::~AtlasECDriver(){}

const char * AtlasECDriver::getSensorTypeString()
{
  return sensorTypeString;
}

void AtlasECDriver::configureDriverFromJSON(cJSON * json)
{
}


configuration_bytes_partition AtlasECDriver::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  memcpy(&partition, &configuration, sizeof(driver_configuration));
  return partition;
}

void AtlasECDriver::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}


void AtlasECDriver::setup()
{
  notify("setup GenericAtlas");
  oem_ec = new EC_OEM(wire, NONE_INT, ec_i2c_id);

  if (true)
  {
    bool awoke = oem_ec->wakeUp();
    notify(awoke);

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
  }
  // notify("led and probe type");
  oem_ec->setLedOn(true);
  oem_ec->setProbeType(1.0);
}


void AtlasECDriver::stop()
{
  oem_ec->setHibernate();
  delete  oem_ec;
}


void AtlasECDriver::setDriverDefaults()
{
  configuration.cal_timestamp = 0;
}


void AtlasECDriver::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  addCalibrationParametersToJSON(json);
}


const char * AtlasECDriver::getBaseColumnHeaders()
{
  return baseColumnHeaders;
}


bool AtlasECDriver::takeMeasurement()
{
  bool waitForMeasurement = true;
  bool newDataAvailable = false;
  while(!newDataAvailable)
  {
    newDataAvailable = oem_ec->singleReading();
    if(newDataAvailable)
    {
      value = oem_ec->getConductivity();
      oem_ec->clearNewDataRegister();
    }
    else if (!waitForMeasurement)
    {
      return false;
    }
  }

  return true;
}

const char * AtlasECDriver::getDataString()
{
  sprintf(dataString, "%d", value);
  return dataString;
}


void AtlasECDriver::initCalibration()
{
  notify("init calibration");
  oem_ec->clearCalibrationData();
}

void AtlasECDriver::calibrationStep(char * step, int arg_cnt, char ** args)
{
  takeMeasurement();
  if(strcmp(step, "dry") == 0)
  {
    notify("Dry point calibration");
    oem_ec->setCalibration(DRY_CALIBRATION);
  }
  else if (strcmp(step, "low") == 0)
  {
    notify("Low point calibration");
    oem_ec->setCalibration(LOW_POINT_CALIBRATION, atof(args[0]));
  }
  else if (strcmp(step, "high") == 0)
  {
    notify("High point calibration");
    oem_ec->setCalibration(HIGH_POINT_CALIBRATION, atof(args[0]));
    configuration.cal_timestamp = timestamp();
  }
  else
  {
    notify("Invalid calibration step");
  }
}



void AtlasECDriver::addCalibrationParametersToJSON(cJSON * json)
{
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

