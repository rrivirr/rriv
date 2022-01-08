#include "generic_analog.h"
#include "system/monitor.h"
#include "system/measurement_components.h"
#include "system/eeprom.h" // TODO: ideally not included in this scope
#include "system/clock.h"  // TODO: ideally not included in this scope
#include "sensors/sensor_types.h"

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

GenericAnalog::~GenericAnalog(){}

void GenericAnalog::configureDriverFromJSON(cJSON * json)
{
  configuration.common.sensor_type = GENERIC_ANALOG_SENSOR; // redundant?

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
      notify(F("Invalid adc select"));
    }
  } 
  else 
  {
    notify(F("Invalid adc select"));
  }
 

  const cJSON * sensorPortJSON = cJSON_GetObjectItemCaseSensitive(json, "sensor_port");
  if(sensorPortJSON != NULL && cJSON_IsNumber(sensorPortJSON) && sensorPortJSON->valueint < 5)
  {
    configuration.sensor_port = (byte) sensorPortJSON->valueint;
  } else {
    notify(F("Invalid sensor port"));
  }

  this->configureCSVColumns();
}


void GenericAnalog::setup()
{
  debug("setup GenericAnalog");
}


void GenericAnalog::setDriverDefaults()
{
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


void GenericAnalog::setConfiguration(generic_config configuration)
{
  memcpy(&this->configuration, &configuration, sizeof(generic_config));
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
  addCalibrationParametersToJSON(json);
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
    {
      debug("getting external ADC measurement");
      this->value = externalADC->getChannelValue(this->configuration.sensor_port - 1);
    }
      break;

    default:
    {}       // bad configuration
      break;
  }

  return true;
}

char * GenericAnalog::getDataString(){
  //   int parameterValue = (value-(b/TEMPERATURE_SCALER))/(m/TEMPERATURE_SCALER);
  int parameterValue = (value-(configuration.b))/(configuration.m);

  sprintf(dataString, "%d,%d", value, parameterValue);
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


void GenericAnalog::initCalibration()
{
  notify(F("Two point calibration"));
  notify(F("calibrate SLOT low VALUE"));
  notify(F("calibrate SLOT high VALUE"));
  notify(F("calibrate SLOT store"));
  calibrate_high_reading = calibrate_high_value = calibrate_low_reading = calibrate_low_value = 0;

}

void GenericAnalog::printCalibrationStatus()
{
  notify(F("Calibration status:"));
  char buffer[50];
  sprintf(buffer, "calibrate_high_reading: %d", calibrate_high_reading);
  notify(buffer);
  sprintf(buffer, "calibrate_high_value: %d", calibrate_high_value);
  notify(buffer);
  sprintf(buffer, "calibrate_low_reading: %d", calibrate_low_reading);
  notify(buffer);
  sprintf(buffer, "calibrate_low_value: %d", calibrate_low_value);
  notify(buffer);
}

void GenericAnalog::calibrationStep(char * step, int trueValue)
{
  takeMeasurement();
  if(strcmp(step, "high") == 0)
  {
    calibrate_high_reading = this->value;
    calibrate_high_value = trueValue;
    printCalibrationStatus();
  }
  else if (strcmp(step, "low") == 0)
  {
    calibrate_low_reading = this->value;
    calibrate_low_value = trueValue;
    printCalibrationStatus();
  }
  else if (strcmp(step, "store") == 0)
  {
    printCalibrationStatus();
    if(
      calibrate_high_reading == 0
      || calibrate_high_value == 0
      || calibrate_low_reading == 0
      || calibrate_low_value == 0
    )
    {
      notify("Incomplete calibration");
      return;
    }

    computeCalibratedCurve();

    // TODO: ideally this function would not be called from within a driver
    // but how does datalogger know the configuration is dirty, so it can write?
    writeSensorConfigurationToEEPROM(configuration.common.slot, &configuration); 

    cJSON* json = cJSON_CreateObject();
    addCalibrationParametersToJSON(json);
    char * string = cJSON_Print(json);
    if (string == NULL)
    {
      notify("Failed to print json.");
    }
    notify(string);
    free(json);
    
  }
  else
  {
    notify("Invalid calibration step");
  }
}


void GenericAnalog::computeCalibratedCurve() // calibrate using linear slope equation, log time
{
  //v = mc+b    m = (v2-v1)/(c2-c1)    b = (m*-c1)+v1
  //C1 C2 M B are scaled up for storage, V1 V2 are scaled up for calculation

  int m = (calibrate_high_value - calibrate_low_value) / ( calibrate_high_reading - calibrate_low_reading);
  int b = (((m*(0-calibrate_low_reading)) + calibrate_low_value) + ((m*(0-calibrate_high_reading)) + calibrate_high_value))/2; //average at two points

  // slope = m * TEMPERATURE_SCALER;
  // intercept = b;
  configuration.m = m;
  configuration.b = b;
  configuration.x1 = calibrate_low_reading;
  configuration.x2 = calibrate_high_reading;
  configuration.y1 = calibrate_low_value;
  configuration.y2 = calibrate_high_value;
  configuration.cal_timestamp = timestamp();
}


void GenericAnalog::addCalibrationParametersToJSON(cJSON * json)
{
  cJSON_AddNumberToObject(json, "m", configuration.m);
  cJSON_AddNumberToObject(json, "b", configuration.b);
  cJSON_AddNumberToObject(json, "x1", configuration.x1);
  cJSON_AddNumberToObject(json, "x2", configuration.x2);
  cJSON_AddNumberToObject(json, "y1", configuration.y1);
  cJSON_AddNumberToObject(json, "y2", configuration.y2);
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

