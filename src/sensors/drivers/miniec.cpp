#include "sensors/drivers/miniec.h"
#include "system/logs.h" // for debug() and notify()
// #include "system/measurement_components.h" // if external adc is used

#define MINIEC_TAG "ec"

byte i2cAddress = 0x4D; // MCP3221 A5 in Dec 77 A0 = 72 A7 = 79)
						// A0 = x48, A1 = x49, A2 = x4A, A3 = x4B, 
						// A4 = x4C, A5 = x4D, A6 = x4E, A7 = x4F
            

MinieCDriver::MinieCDriver()
{
  // debug("allocating driver template");
}

MinieCDriver::~MinieCDriver(){}

const char * MinieCDriver::getSensorTypeString()
{
  return sensorTypeString;
}

configuration_bytes_partition MinieCDriver::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  memcpy(&partition, &configuration, sizeof(driver_configuration));
  return partition;
}

bool MinieCDriver::configureDriverFromJSON(cJSON *json)
{
  return true;
}


void MinieCDriver::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}

void MinieCDriver::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("appeding driver specific driver template json");
  
  //driver specific config, customize
  addCalibrationParametersToJSON(json);
}

void MinieCDriver::setup()
{
  // debug("setup MinieCDriver");
  i2cADC = new MCP3221(this->wire, i2cAddress, 3300);
}

void MinieCDriver::stop()
{
  // debug("stop/delete MinieCDriver");
  delete i2cADC;
}

bool MinieCDriver::takeMeasurement()
{
  // debug("taking measurement from driver template");
  //return true if measurement taken store in class value(s), false if not
  bool measurementTaken = false;
  if(true)
  {
    measurementTaken = true;
    int adcRaw = i2cADC->getData();
    addValueToBurstSummaryMean(MINIEC_TAG, value); // use the default option for computing the burst summary value
  }
  return measurementTaken;
}

const char *MinieCDriver::getRawDataString()
{
  // debug("configuring driver template dataString");
  // process data string for .csv
  sprintf(dataString, "%d,%0.3f",value,value*31.83);
  return dataString;
}

const char *MinieCDriver::getSummaryDataString()
{
  double burstSummaryMean = getBurstSummaryMean(MINIEC_TAG);
  sprintf(dataString, "%0.3f,%0.3f", burstSummaryMean, burstSummaryMean*31.83);
  return dataString;  
}

const char *MinieCDriver::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting driver template base column headers");
  return baseColumnHeaders;
}

void MinieCDriver::initCalibration()
{
  // debug("init driver template sensor calibration");
}

void MinieCDriver::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("driver template calibration steps");
}

void MinieCDriver::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add driver template calibration parameters to json");
  cJSON_AddNumberToObject(json, CALIBRATION_TIME_STRING, configuration.cal_timestamp);
}

void MinieCDriver::setDriverDefaults()
{
  // debug("setting driver template driver defaults");
  // set default values for driver struct specific values
  configuration.cal_timestamp = 0;
}