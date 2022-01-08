#include "generic_atlas.h"
#include "system/monitor.h"
#include "system/measurement_components.h"
#include "system/eeprom.h" // TODO: ideally not included in this scope
#include "system/clock.h"  // TODO: ideally not included in this scope
#include "sensors/sensor_types.h"

GenericAtlas::GenericAtlas()
{
  debug("allocation GenericAtlas");
}

GenericAtlas::~GenericAtlas(){}

// TODO: place common routines in SensorDriver base class
void GenericAtlas::configureDriverFromJSON(cJSON * json)
{
  configuration.common.sensor_type = GENERIC_ATLAS_SENSOR;
  this->configureCSVColumns();
}


protocol_type GenericAtlas::getProtocol(){
  return i2c;
}

void GenericAtlas::setup()
{
  notify("setup GenericAtlas");
  oem_ec = new EC_OEM(wire, NONE_INT, ec_i2c_id);
  notify("did");

  if (true)
  {
    bool awoke = oem_ec->wakeUp();
    notify(awoke);

    char message[300];
    sprintf(message, "Device addr EC: %x\nDevice type EC: %x\nFirmware EC: %x\nAwoke: %i\nHibernating: %i",
            oem_ec->getStoredAddr(), oem_ec->getDeviceType(), oem_ec->getFirmwareVersion(), awoke, oem_ec->isHibernate());
    debug(message);

    oem_ec->singleReading();
    struct param_OEM_EC parameter;
    parameter = oem_ec->getAllParam();

    debug(F("test:"));
    sprintf(message, "salinity= %f\nconductivity= %f\ntds= %f\nSalinity stable = %s",
            parameter.salinity, parameter.conductivity, parameter.tds, (oem_ec->isSalinityStable() ? "yes" : "no"));
    debug(message);
  }
  notify("led and probe type");
  oem_ec->setLedOn(true);
  oem_ec->setProbeType(1.0);
}


void GenericAtlas::stop()
{
  oem_ec->setHibernate();
  delete  oem_ec;
}


void GenericAtlas::setDriverDefaults()
{
  
}


// base class
generic_config GenericAtlas::getConfiguration()
{
  generic_config configuration;
  memcpy(&configuration, &this->configuration, sizeof(generic_atlas_sensor));
  return configuration;
}


void GenericAtlas::setConfiguration(generic_config configuration)
{
  memcpy(&this->configuration, &configuration, sizeof(generic_config));
}


// split between base class and this class
// getConfigurationJSON: base class
// getDriverSpecificConfigurationJSON: this class
cJSON * GenericAtlas::getConfigurationJSON() // returns unprotected pointer
{
  cJSON* json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "slot", configuration.common.slot);
  cJSON_AddStringToObject(json, "type", "generic_atlas");
  cJSON_AddStringToObject(json, "tag", configuration.common.tag);
  cJSON_AddNumberToObject(json, "burst_size", configuration.common.burst_size);
  addCalibrationParametersToJSON(json);
  return json;
}


const char * GenericAtlas::getBaseColumnHeaders()
{
  return baseColumnHeaders;
}


bool GenericAtlas::takeMeasurement()
{
  bool waitForMeasurement = true;
  bool newDataAvailable = false;
  while(!newDataAvailable)
  {
    newDataAvailable = oem_ec->singleReading();
    if(newDataAvailable)
    {
      value = oem_ec->getConductivity();
      oem_ec->clearNewDataRegister();
    }
    else if (!waitForMeasurement)
    {
      return false;
    }
  }

  return true;
}

char * GenericAtlas::getDataString()
{
  sprintf(dataString, "%d", value);
  return dataString;
}

char * GenericAtlas::getCSVColumnNames()
{
   debug(csvColumnHeaders);
   return csvColumnHeaders;
}


void GenericAtlas::initCalibration()
{
  notify("init calibration");
  oem_ec->clearCalibrationData();
}

// void GenericAtlas::printCalibrationStatus()
// {
//   notify(F("Calibration status:"));
//   char buffer[50];
//   sprintf(buffer, "calibrate_high_reading: %d", calibrate_high_reading);
//   notify(buffer);
//   sprintf(buffer, "calibrate_high_value: %d", calibrate_high_value);
//   notify(buffer);
//   sprintf(buffer, "calibrate_low_reading: %d", calibrate_low_reading);
//   notify(buffer);
//   sprintf(buffer, "calibrate_low_value: %d", calibrate_low_value);
//   notify(buffer);
// }

void GenericAtlas::calibrationStep(char * step, int trueValue)
{
  takeMeasurement();
  if(strcmp(step, "dry") == 0)
  {
    notify("Dry point calibration");
    oem_ec->setCalibration(DRY_CALIBRATION);
  }
  else if (strcmp(step, "low") == 0)
  {
    notify("Low point calibration");
    oem_ec->setCalibration(LOW_POINT_CALIBRATION, trueValue);
  }
  else if (strcmp(step, "high") == 0)
  {
    notify("High point calibration");
    oem_ec->setCalibration(HIGH_POINT_CALIBRATION, trueValue);
    configuration.cal_timestamp = timestamp();
  }
  else
  {
    notify("Invalid calibration step");
  }
}



void GenericAtlas::addCalibrationParametersToJSON(cJSON * json)
{
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

