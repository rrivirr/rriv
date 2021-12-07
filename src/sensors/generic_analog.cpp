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

// TODO: place common routines in SensorDriver base class
void GenericAnalog::configureFromJSON(cJSON * json)
{
  common_config_sensor common;
  this->configureCommonFromJSON(json, &common);
  this->setDefaults();
  configuration.common = common;
  
  configuration.common.sensor_type = GENERIC_ANALOG_SENSOR;

  const cJSON * adcSelectJSON = cJSON_GetObjectItemCaseSensitive(json, "adc_select");
  if(adcSelectJSON != NULL && cJSON_IsString(adcSelectJSON)) 
  {
    if(strcmp(adcSelectJSON->valuestring, "internal") == 0)
    {
      configuration.adc_select = ADC_SELECT_INTERNAL;
    } 
    else if (strcmp(adcSelectJSON->valuestring, "external") == 0)
    {
      configuration.adc_select = ADC_SELECT_EXTERNAL;
    }
    else
    {
      Serial2.println(F("Invalid adc select"));
    }
  } 
  else 
  {
    Serial2.println(F("Invalid adc select"));
  }
 

  const cJSON * sensorPortJSON = cJSON_GetObjectItemCaseSensitive(json, "sensor_port");
  if(sensorPortJSON != NULL && cJSON_IsNumber(sensorPortJSON) && sensorPortJSON->valueint < 5)
  {
    configuration.sensor_port = (byte) sensorPortJSON->valueint;
  } else {
    Serial2.println(F("Invalid sensor port"));
  }

  this->configureCSVColumns();
}


void GenericAnalog::setup()
{
  debug("setup GenericAnalog");
}

void GenericAnalog::configure(generic_config * configuration)
{
  memcpy(&this->configuration, configuration, sizeof(generic_linear_analog_sensor));
  this->setDefaults();
  this->configureCSVColumns();
}

// this class: setDriverSpecificDefaults
// base class: setDefaults()
void GenericAnalog::setDefaults()
{
  this->setCommonDefaults(&configuration.common);
  if(configuration.adc_select != ADC_SELECT_EXTERNAL && configuration.adc_select != ADC_SELECT_INTERNAL)
  {
    configuration.adc_select = ADC_SELECT_INTERNAL;
  }

  if(configuration.sensor_port > 5)
  {
    configuration.sensor_port = 0;
  }
}

// base class
generic_config GenericAnalog::getConfiguration()
{
  generic_config configuration;
  memcpy(&configuration, &this->configuration, sizeof(generic_linear_analog_sensor));
  return configuration;
}

// split between base class and this class
// getConfigurationJSON: base class
// getDriverSpecificConfigurationJSON: this class
cJSON * GenericAnalog::getConfigurationJSON() // returns unprotected pointer
{
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "slot", configuration.common.slot);
  cJSON_AddStringToObject(json, "type", "generic_analog");
  cJSON_AddStringToObject(json, "tag", configuration.common.tag);
  cJSON_AddNumberToObject(json, "burst_size", configuration.common.burst_size);
  cJSON_AddNumberToObject(json, "sensor_port", configuration.sensor_port);
  switch(configuration.adc_select)
  {
    case ADC_SELECT_INTERNAL:
      cJSON_AddStringToObject(json, "adc_select", "internal");
      break;
    case ADC_SELECT_EXTERNAL:
      cJSON_AddStringToObject(json, "adc_select", "external");
      break;
    default:
      break;
  }
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