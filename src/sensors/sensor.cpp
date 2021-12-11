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
  debug("config csv columns");
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
}

void SensorDriver::setCommonDefaults(common_config_sensor * common)
{
  if(common->burst_size <= 0 || common->burst_size > 100)
  {
    common->burst_size = 10;
  }
}


void SensorDriver::configureCommonFromJSON(cJSON * json, common_config_sensor * common)
{
  const cJSON* slotJSON = cJSON_GetObjectItemCaseSensitive(json, "slot");
  if(slotJSON != NULL && cJSON_IsNumber(slotJSON))
  {
    common->slot = slotJSON->valueint;
  } else {
    Serial2.println("Invalid slot");
  }

  const cJSON * tagJSON = cJSON_GetObjectItemCaseSensitive(json, "tag");
  if(tagJSON != NULL && cJSON_IsString(tagJSON) && strlen(tagJSON->valuestring) <= 5)
  {
    strcpy(common->tag, tagJSON->valuestring);
  } else {
    Serial2.println("Invalid tag");
  }

  const cJSON * burstSizeJson = cJSON_GetObjectItemCaseSensitive(json, "burst_size");
  if(burstSizeJson != NULL && cJSON_IsNumber(burstSizeJson) && burstSizeJson->valueint > 0)
  {
    common->burst_size = (byte) burstSizeJson->valueint;
  } else {
    Serial2.println("Invalid burst size");
  }
}

// placeholder for required virtual destructors
AnalogSensorDriver::~AnalogSensorDriver(){}
I2CSensorDriver::~I2CSensorDriver(){}