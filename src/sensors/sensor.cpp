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

#include "sensor.h"
#include "system/monitor.h"

SensorDriver::SensorDriver(){}
SensorDriver::~SensorDriver(){}


void SensorDriver::initializeBurst(){
  burstCount = 0;
}

void SensorDriver::incrementBurst(){
  burstCount++;
}

bool SensorDriver::burstCompleted(){
  notify(burstCount);
  notify(getConfiguration().common.burst_size);
  return burstCount == getConfiguration().common.burst_size;
}


void SensorDriver::configureCSVColumns()
{
  notify("config csv columns");
  char csvColumnHeaders[100] = "\0";
  char buffer[100];
  strcpy(buffer, this->getBaseColumnHeaders());
  debug(buffer);
  char * token = strtok(buffer, ",");
  while(token != NULL)
  {
    debug(token);
    strcat(csvColumnHeaders, this->getConfiguration().common.tag);
    strcat(csvColumnHeaders, "_");
    strcat(csvColumnHeaders, token);
    token = strtok(NULL, ",");
    if(token != NULL)
    {
      strcat(csvColumnHeaders, ",");
    }
  }
  strcpy(this->csvColumnHeaders, csvColumnHeaders);
  notify("done");
}

void SensorDriver::setDefaults()
{
  generic_config configuration = this->getConfiguration();

  if(configuration.common.burst_size <= 0 || configuration.common.burst_size > 100)
  {
    configuration.common.burst_size = 10;
  }
  this->setConfiguration(configuration);
  this->setDriverDefaults();
}


void SensorDriver::configureFromJSON(cJSON * json)
{
  generic_config configuration = this->getConfiguration();
  memset(&configuration, SENSOR_CONFIGURATION_SIZE, 0);

  const cJSON* slotJSON = cJSON_GetObjectItemCaseSensitive(json, "slot");
  if(slotJSON != NULL && cJSON_IsNumber(slotJSON))
  {
    configuration.common.slot = slotJSON->valueint;
  } else {
    notify("Invalid slot");
  }

  const cJSON * tagJSON = cJSON_GetObjectItemCaseSensitive(json, "tag");
  if(tagJSON != NULL && cJSON_IsString(tagJSON) && strlen(tagJSON->valuestring) <= 5)
  {
    strcpy(configuration.common.tag, tagJSON->valuestring);
  } else {
    notify("Invalid tag");
  }

  const cJSON * burstSizeJson = cJSON_GetObjectItemCaseSensitive(json, "burst_size");
  if(burstSizeJson != NULL && cJSON_IsNumber(burstSizeJson) && burstSizeJson->valueint > 0)
  {
    configuration.common.burst_size = (byte) burstSizeJson->valueint;
  } else {
    notify("Invalid burst size");
  }

  notify("set configuration");
  this->setConfiguration(configuration);
  notify("set defaults");
  this->setDefaults();
  notify("configure driver from JSON");
  this->configureDriverFromJSON(json);
  notify("conf csv cols");
  this->configureCSVColumns();

}


void SensorDriver::configure(generic_config configuration)
{
  this->setConfiguration(configuration);
  // this->setDefaults();
  this->configureCSVColumns();
}

void I2CSensorDriver::setWire(TwoWire * wire)
{
  this->wire = wire;
}

// placeholder for required virtual destructors
AnalogSensorDriver::~AnalogSensorDriver(){}
I2CSensorDriver::~I2CSensorDriver(){}