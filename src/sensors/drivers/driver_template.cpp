#include "sensors/drivers/driver_template.h"
#include "sensors/sensor_types.h"
#include "system/monitor.h" // for debug() and notify()
// #include "system/measurement_components.h" // if external adc is used

DriverTemplate::DriverTemplate()
{
  // debug("allocating driver template");
}

DriverTemplate::~DriverTemplate(){}

generic_config DriverTemplate::getConfiguration()
{
  // debug("getting driver template configuration");
  generic_config configuration;
  memcpy(&configuration, &this->configuration, sizeof(driver_template_sensor));
  return configuration;
}

void DriverTemplate::setConfiguration(generic_config configuration)
{
  // debug("setting driver template configuration");
  memcpy(&this->configuration, &configuration, sizeof(generic_config));
}

void DriverTemplate::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("appeding driver specific driver template json");
  
  //driver specific config, customize
  addCalibrationParametersToJSON(json);
}

void DriverTemplate::setup()
{
  // debug("setup DriverTemplate");
}

void DriverTemplate::stop()
{
  // debug("stop/delete DriverTemplate");
}

bool DriverTemplate::takeMeasurement()
{
  // debug("taking measurement from driver template");
  //return true if measurement taken store in class value(s), false if not
  bool measurementTaken = false;
  if(true)
  {
    value = 42;
    measurementTaken = true;
  }
  return measurementTaken;
}

const char *DriverTemplate::getDataString()
{
  // debug("configuring driver template dataString");
  // process data string for .csv
  sprintf(dataString, "%d,%d",value,int(value*31.83));
  return dataString;
}

const char *DriverTemplate::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting driver template base column headers");
  return baseColumnHeaders;
}

void DriverTemplate::initCalibration()
{
  // debug("init driver template sensor calibration");
}

void DriverTemplate::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("driver template calibration steps");
}

void DriverTemplate::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add driver template calibration parameters to json");
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

void DriverTemplate::configureDriverFromJSON(cJSON *json)
{
  // retrieve sensor type from sensor_types.h
  // debug("configuring driver template driver from json");
  configuration.common.sensor_type = DRIVER_TEMPLATE;
}

void DriverTemplate::setDriverDefaults()
{
  // debug("setting driver template driver defaults");
  // set default values for driver struct specific values
  configuration.cal_timestamp = 0;
}