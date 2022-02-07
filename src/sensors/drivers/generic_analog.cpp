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
    ANALOG_INPUT_5_PIN};

#define FLOATING_POINT_STORAGE_MULTIPLIER 100

GenericAnalog::GenericAnalog()
{
  debug("allocation GenericAnalog");
}

GenericAnalog::~GenericAnalog() {}

void GenericAnalog::configureDriverFromJSON(cJSON *json)
{
  configuration.common.sensor_type = GENERIC_ANALOG_SENSOR; // redundant?

  const cJSON *adcSelectJSON = cJSON_GetObjectItemCaseSensitive(json, "adc_select");
  if (adcSelectJSON != NULL && cJSON_IsString(adcSelectJSON))
  {
    if (strcmp(adcSelectJSON->valuestring, "internal") == 0)
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
      return;
    }
  }
  else
  {
    notify(F("Invalid adc select"));
    return;
  }
  notify("done");

  const cJSON *sensorPortJSON = cJSON_GetObjectItemCaseSensitive(json, "sensor_port");
  if (sensorPortJSON != NULL && cJSON_IsNumber(sensorPortJSON) && sensorPortJSON->valueint < 5)
  {
    configuration.sensor_port = (byte)sensorPortJSON->valueint;
  }
  else
  {
    notify(F("Invalid sensor port"));
    return;
  }
  notify("done");
}

void GenericAnalog::setup()
{
  debug("setup GenericAnalog");
}

void GenericAnalog::setDriverDefaults()
{
  if (configuration.adc_select != ADC_SELECT_EXTERNAL && configuration.adc_select != ADC_SELECT_INTERNAL)
  {
    configuration.adc_select = ADC_SELECT_INTERNAL;
  }

  if (configuration.sensor_port > 5)
  {
    configuration.sensor_port = 0;
  }

  configuration.m = 0;
  configuration.b = 0;
  configuration.x1 = 0;
  configuration.x2 = 0;
  configuration.y1 = 0;
  configuration.y2 = 0;
  configuration.cal_timestamp = 0;
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
cJSON *GenericAnalog::getConfigurationJSON() // returns unprotected pointer
{
  cJSON *json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "slot", configuration.common.slot);
  cJSON_AddStringToObject(json, "type", "generic_analog");
  cJSON_AddStringToObject(json, "tag", configuration.common.tag);
  cJSON_AddNumberToObject(json, "burst_size", configuration.common.burst_size);
  cJSON_AddNumberToObject(json, "sensor_port", configuration.sensor_port);
  switch (configuration.adc_select)
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

const char *GenericAnalog::getBaseColumnHeaders()
{
  return baseColumnHeaders;
}

void GenericAnalog::stop() {}

bool GenericAnalog::takeMeasurement()
{
  // take measurement and write to dataString member variable
  switch (this->configuration.adc_select)
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
  {
  } // bad configuration
  break;
  }

  return true;
}

#define BURST_SIZE 20
void GenericAnalog::takeCalibrationBurstMeasurement()
{

  int x[BURST_SIZE];
  int sum = 0;
  double sum1 = 0;
  notify("Calibration measurments:");
  for (int i = 0; i < BURST_SIZE; i++)
  {
    if (this->configuration.adc_select == ADC_SELECT_EXTERNAL)
    {
      externalADC->convertEnabledChannels();
    }
    takeMeasurement();
    notify(this->value);
    sum += this->value;
    x[i] = this->value;
    delay(100);
  }
  int average = sum / BURST_SIZE;
  this->value = average;

  /*  Compute  variance */
  for (int i = 0; i < BURST_SIZE; i++)
  {
    sum1 = sum1 + pow((x[i] - average), 2);
  }
  double variance = sum1 / (float)(BURST_SIZE);
  char buffer[50];
  sprintf(buffer, "variance of measurements = %.2f\n", variance);
  notify(buffer);
}

char *GenericAnalog::getDataString()
{
  int exponent = -(4 - configuration.order_of_magnitude);
  double calibratedValue = (configuration.m * value + configuration.b) * pow(10, -exponent);
  sprintf(dataString, "%d,%0.3f", value, calibratedValue);
  return dataString;
}

char *GenericAnalog::getCSVColumnNames()
{
  debug(csvColumnHeaders);
  return csvColumnHeaders;
}

protocol_type GenericAnalog::getProtocol()
{
  notify(F("getting protocol"));
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

void GenericAnalog::calibrationStep(char *step, double trueValue)
{
  if (strcmp(step, "high") == 0)
  {
    takeCalibrationBurstMeasurement();
    calibrate_high_reading = this->value;
    calibrate_high_value = trueValue;
    printCalibrationStatus();
  }
  else if (strcmp(step, "low") == 0)
  {
    takeCalibrationBurstMeasurement();
    calibrate_low_reading = this->value;
    calibrate_low_value = trueValue;
    printCalibrationStatus();
  }
  else if (strcmp(step, "store") == 0)
  {
    printCalibrationStatus();
    if (
        calibrate_high_reading == 0 || calibrate_high_value == 0 || calibrate_low_reading == 0 || calibrate_low_value == 0)
    {
      notify("Incomplete calibration");
      return;
    }

    computeCalibratedCurve();

    // TODO: ideally this function would not be called from within a driver
    // but how does datalogger know the configuration is dirty, so it can write?
    writeSensorConfigurationToEEPROM(configuration.common.slot, &configuration);

    cJSON *json = cJSON_CreateObject();
    addCalibrationParametersToJSON(json);
    char *string = cJSON_Print(json);
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
  // y = mx+b    m = (y2-y1)/(x2-x1)    b = y - mx
  // all x and y are integers.  m and b are scale up and cast to int for storage

  // figure out orders of magnitude
  int orderOfMagnitude = floor(log10(calibrate_low_value));
  int exponent = 4 - orderOfMagnitude;
  double scaledCalibrateHighValue = calibrate_high_value * pow(10, exponent);
  double scaledCalibrateLowValue = calibrate_low_value * pow(10, exponent);


  double m = (double)(scaledCalibrateHighValue - scaledCalibrateLowValue) / (double)(calibrate_high_reading - calibrate_low_reading);
  double b = scaledCalibrateHighValue - m * calibrate_high_reading;

  configuration.m = m;
  configuration.b = b;
  configuration.order_of_magnitude = orderOfMagnitude;
  configuration.x1 = calibrate_low_reading;
  configuration.x2 = calibrate_high_reading;
  configuration.y1 = calibrate_low_value;
  configuration.y2 = calibrate_high_value;
  configuration.cal_timestamp = timestamp();
}

void GenericAnalog::addCalibrationParametersToJSON(cJSON *json)
{
  cJSON_AddNumberToObject(json, "m", configuration.m);
  cJSON_AddNumberToObject(json, "b", configuration.b);
  cJSON_AddNumberToObject(json, "order_of_magnitude", configuration.order_of_magnitude);
  cJSON_AddNumberToObject(json, "x1", configuration.x1);
  cJSON_AddNumberToObject(json, "x2", configuration.x2);
  cJSON_AddNumberToObject(json, "y1", configuration.y1);
  cJSON_AddNumberToObject(json, "y2", configuration.y2);
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}
