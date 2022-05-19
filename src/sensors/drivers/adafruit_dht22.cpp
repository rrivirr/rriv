#include "sensors/drivers/adafruit_dht22.h"
#include "system/logs.h" // for debug() and notify()

AdaDHT22::AdaDHT22()
{
  //debug("allocating AdaDHT22")
}

AdaDHT22::~AdaDHT22(){}

const char * AdaDHT22::getSensorTypeString()
{
  return sensorTypeString;
}


configuration_bytes_partition AdaDHT22::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  memcpy(&partition, &configuration, sizeof(driver_configuration));
  return partition;
}

void AdaDHT22::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}


void AdaDHT22::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("getting AdaDHT22 json");
  
  //driver specific config, customize
  addCalibrationParametersToJSON(json);
}

void AdaDHT22::setup()
{
  // debug("setup AdaDHT22");
  dht = new DHT_Unified(DHTPIN, DHTTYPE);
  dht->begin();
  notify("AdaDHT22 Initialized, test reading:");
  takeMeasurement();
}

void AdaDHT22::stop()
{
  // debug("stop/delete AdaDHT22");
  delete dht;
}

bool AdaDHT22::takeMeasurement()
{
  // debug("taking measurement from AdaDHT22");
  sensors_event_t event;
  bool measurementTaken = false;

  dht->temperature().getEvent(&event);
  temperature = event.temperature;
  if(isnan(temperature))
  {
    notify("Error reading temperature)");
  }
  else
  {
    notify("Temperature: ");
    debug(temperature);
    notify("°C");
    measurementTaken = true;
  }

  dht->humidity().getEvent(&event);
  humidity = event.relative_humidity;
  if(isnan(humidity))
  {
    notify("Error reading humidity");
  }
  else
  {
    notify("Humidity: ");
    debug(humidity);
    notify("%");
    measurementTaken = true;
  }

  if(measurementTaken)
  {
    addValueToBurstSummaryMean("temperature", temperature);
    addValueToBurstSummaryMean("humidity", humidity);
  }

  return measurementTaken;
}

const char *AdaDHT22::getRawDataString()
{
  // debug("configuring AdaDHT22 dataString");
  // process data string for .csv
  sprintf(dataString, "%.2f,%.2f", temperature, humidity);
  return dataString;
}

const char *AdaDHT22::getSummaryDataString()
{
  // debug("configuring AdaDHT22 dataString");
  // process data string for .csv
  // TODO: just reporting the last value, not a true summary
  sprintf(dataString, "%.2f,%.2f", temperature, humidity);
  return dataString;
}

const char *AdaDHT22::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting AdaDHT22 base column headers");
  return baseColumnHeaders;
}

void AdaDHT22::initCalibration()
{
  // debug("init AdaDHT22 calibration");
}

void AdaDHT22::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("AdaDHT22 calibration steps");
}

void AdaDHT22::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add AdaDHT22 calibration parameters to json");
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

void AdaDHT22::configureDriverFromJSON(cJSON *json)
{

}

void AdaDHT22::setDriverDefaults()
{
  // debug("setting AdaDHT22 driver defaults");
  // set default values for driver struct specific values
  configuration.cal_timestamp = 0;
}