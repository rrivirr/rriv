#include "sensors/drivers/atlas_co2_driver.h"
#include "system/logs.h" // for debug() and notify()

AtlasCO2Driver::AtlasCO2Driver()
{
  // debug("allocating driver template");
}

AtlasCO2Driver::~AtlasCO2Driver(){}

const char * AtlasCO2Driver::getSensorTypeString()
{
  return sensorTypeString;
}

configuration_bytes_partition AtlasCO2Driver::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  memcpy(&partition, &configuration, sizeof(driver_config));
  return partition;
}

void AtlasCO2Driver::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  memcpy(&configuration, &configurationPartition, sizeof(driver_config));
}

void AtlasCO2Driver::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("appeding driver specific driver template json");
  
  //driver specific config, customize
  addCalibrationParametersToJSON(json);
}

void AtlasCO2Driver::setup()
{
  // debug("setup AtlasCO2Driver");
  modularSensorDriver = new AtlasScientificCO2(wire,-1);
  if(!modularSensorDriver->setup()){
    notify("CO2 setup failed");
  }
  modularSensorDriver->wake();
}

void AtlasCO2Driver::stop()
{
  // modularSensorDriver->sleep(); // i2c sensors are on boosted 3v3 power which gets shut off during sleep
  // debug("stop/delete AtlasCO2Driver");
}

bool AtlasCO2Driver::takeMeasurement()
{
  // debug("taking measurement from driver template");
  //return true if measurement taken store in class value(s), false if not
  modularSensorDriver->startSingleMeasurement();
  modularSensorDriver->waitForMeasurementCompletion();
  bool measurementTaken = modularSensorDriver->addSingleMeasurementResult();
  if(measurementTaken)
  {
    value = modularSensorDriver->sensorValues[0];
    modularSensorDriver->clearValues();
    measurementTaken = true;
  }
  else
  {
    value = -1;
  }

  return measurementTaken;
}

const char *AtlasCO2Driver::getRawDataString()
{
  // debug("configuring driver template dataString");
  // process data string for .csv
  sprintf(dataString, "%d,%d",value,0);
  return dataString;
}
const char *AtlasCO2Driver::getSummaryDataString()
{
  // debug("configuring driver template dataString");
  // process data string for .csv
  // TODO: just reporting the last value, not a true summary
  sprintf(dataString, "%d,%d",value,0);
  return dataString;
}

const char *AtlasCO2Driver::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting driver template base column headers");
  return baseColumnHeaders;
}

void AtlasCO2Driver::initCalibration()
{
  // debug("init driver template sensor calibration");
}

void AtlasCO2Driver::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("driver template calibration steps");
}

void AtlasCO2Driver::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add driver template calibration parameters to json");
  cJSON_AddNumberToObject(json, CALIBRATION_TIME_STRING, configuration.cal_timestamp);
}

bool AtlasCO2Driver::configureDriverFromJSON(cJSON *json)
{
  // debug("configuring driver template driver from json");
  return true;
}

void AtlasCO2Driver::setDriverDefaults()
{
  // debug("setting driver template driver defaults");
  // set default values for driver struct specific values
  configuration.cal_timestamp = 0;
}

unsigned int AtlasCO2Driver::millisecondsUntilNextRequestedReading()
{
  return 100;
}
