#include "generic_analog.h"
#include "system/monitor.h"
#include "sensor_types.h"

int ADC_PINS[5] = {
  ANALOG_INPUT_1_PIN, 
  ANALOG_INPUT_2_PIN, 
  ANALOG_INPUT_3_PIN, 
  ANALOG_INPUT_4_PIN, 
  ANALOG_INPUT_5_PIN
};

GenericAnalog::GenericAnalog()
{
  debug("allocation GenericAnalog");
}

void GenericAnalog::configureFromJSON(cJSON * json)
{
  common_config_sensor common;
  this->configureCommonFromJSON(json, &common);
  this->setCommonDefaults(&common);
  configuration.common = common;
  
  configuration.common.sensor_type = GENERIC_ANALOG_SENSOR;
  this->configureCSVColumns();
}


void GenericAnalog::setup()
{
  debug("setup GenericAnalog");
}

void GenericAnalog::configure(generic_config * configuration)
{
  this->setCommonDefaults(&configuration->common);
  memcpy(&this->configuration, configuration, sizeof(generic_linear_analog_sensor));
  this->configureCSVColumns();
}

generic_config GenericAnalog::getConfiguration()
{
  generic_config configuration;
  memcpy(&configuration, &this->configuration, sizeof(generic_linear_analog_sensor));
  return configuration;
}

cJSON * GenericAnalog::getConfigurationJSON() // returns unprotected pointer
{
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "slot", configuration.common.slot);
  cJSON_AddStringToObject(json, "type", "generic_analog");
  cJSON_AddStringToObject(json, "tag", configuration.common.tag);
  cJSON_AddNumberToObject(json, "burst_size", configuration.common.burst_size);

  return json;
}

const char * GenericAnalog::getBaseColumnHeaders()
{
  return baseColumnHeaders;
}


void GenericAnalog::stop(){}

bool GenericAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable
  switch(this->configuration.adc_select)
  {
    case ADC_SELECT_INTERNAL:
    {
      int adcPin = ADC_PINS[this->configuration.sensor_port];
      this->value = analogRead(adcPin);
    }
      break;
    
    case ADC_SELECT_EXTERNAL:
    {}
      break;

    default:
    {}       // bad configuration
      break;
  }

  return true;
}

char * GenericAnalog::getDataString(){
  sprintf(dataString, "%d", value);
  return dataString;
}

char * GenericAnalog::getCSVColumnNames()
{
   debug(csvColumnHeaders);
   return csvColumnHeaders;
}

protocol_type GenericAnalog::getProtocol(){
  return analog;
}