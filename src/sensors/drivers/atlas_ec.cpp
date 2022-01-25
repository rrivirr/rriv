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

AtlasEC::AtlasEC()
{
  debug("allocation GenericAtlas");
}

AtlasEC::~AtlasEC(){}

void AtlasEC::configureDriverFromJSON(cJSON * json)
{
  configuration.common.sensor_type = GENERIC_ATLAS_SENSOR;
}


protocol_type AtlasEC::getProtocol(){
  return i2c;
}

void AtlasEC::setup()
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


void AtlasEC::stop()
{
  oem_ec->setHibernate();
  delete  oem_ec;
}


void AtlasEC::setDriverDefaults()
{
  configuration.cal_timestamp = 0;
}


// base class
generic_config AtlasEC::getConfiguration()
{
  generic_config configuration;
  memcpy(&configuration, &this->configuration, sizeof(generic_atlas_sensor));
  return configuration;
}


void AtlasEC::setConfiguration(generic_config configuration)
{
  memcpy(&this->configuration, &configuration, sizeof(generic_config));
}


// split between base class and this class
// getConfigurationJSON: base class
// getDriverSpecificConfigurationJSON: this class
cJSON * AtlasEC::getConfigurationJSON() // returns unprotected pointer
{
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "slot", configuration.common.slot);
  cJSON_AddStringToObject(json, "type", "atlas_ec");
  cJSON_AddStringToObject(json, "tag", configuration.common.tag);
  cJSON_AddNumberToObject(json, "burst_size", configuration.common.burst_size);
  addCalibrationParametersToJSON(json);
  return json;
}


const char * AtlasEC::getBaseColumnHeaders()
{
  return baseColumnHeaders;
}


bool AtlasEC::takeMeasurement()
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

char * AtlasEC::getDataString()
{
  sprintf(dataString, "%d", value);
  return dataString;
}

char * AtlasEC::getCSVColumnNames()
{
   debug(csvColumnHeaders);
   return csvColumnHeaders;
}


void AtlasEC::initCalibration()
{
  notify("init calibration");
  oem_ec->clearCalibrationData();
}

void AtlasEC::calibrationStep(char * step, int trueValue)
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
    oem_ec->setCalibration(LOW_POINT_CALIBRATION, trueValue);
  }
  else if (strcmp(step, "high") == 0)
  {
    notify("High point calibration");
    oem_ec->setCalibration(HIGH_POINT_CALIBRATION, trueValue);
    configuration.cal_timestamp = timestamp();
  }
  else
  {
    notify("Invalid calibration step");
  }
}



void AtlasEC::addCalibrationParametersToJSON(cJSON * json)
{
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

