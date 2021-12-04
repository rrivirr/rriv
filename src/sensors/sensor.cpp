#include "sensor.h"
#include "system/monitor.h"

SensorDriver::SensorDriver()
{

}

void SensorDriver::initializeBurst(){
  burstCount = 0;
}

void SensorDriver::incrementBurst(){
  burstCount++;
}

bool SensorDriver::burstCompleted(){
  return burstCount == burstLength;
}

void getDefaultsCommon(common_config_sensor *fillValues)
{
  Monitor::instance()->writeDebugMessage(F("getDefaultsCCS"));
  fillValues->sensor_type = 1;
  fillValues->slot = 1;
  fillValues->sensor_burst = 10;
  fillValues->warmup = 54321;
  strcpy(fillValues->tag, "CCS");
  strcpy(fillValues->tag, "test");
  strcpy(fillValues->padding, "0000000000000000");
}

void SensorDriver::configureCSVColumns()
{
  debug("config csv columns");
  char csvColumnHeaders[100] = "\0";
  debug(this->getBaseColumnHeaders());
  char * token = strtok( (char * ) this->getBaseColumnHeaders(), ",");
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
}