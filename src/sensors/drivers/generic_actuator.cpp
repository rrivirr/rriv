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
#include "sensors/drivers/generic_actuator.h"
#include "system/logs.h" // for debug() and notify()
// #include "system/measurement_components.h" // if external adc is used

GenericActuator::GenericActuator()
{
  // debug("allocating generic actuator");
}

GenericActuator::~GenericActuator(){}

protocol_type GenericActuator::getProtocol()
{
  return genericactuator; //actuators don't have protocol type ? 
}

const char * GenericActuator::getSensorTypeString()
{
  return sensorTypeString;
}

void GenericActuator::setup()
{
  // debug("setup DriverTemplate");
  
}

void GenericActuator::stop()
{
  // debug("stop/delete DriverTemplate");
}

bool GenericActuator::takeMeasurement()
{
  // debug("taking measurement from driver template");
  //return true if measurement taken store in class value(s), false if not
  // bool measurementTaken = false;
  // if(true)
  // {
  //   value = 42; //why 42? 
  //   measurementTaken = true;
  // }
  // addValueToBurstSummaryMean("var", value); // use the default option for computing the burst summary value
  // return measurementTaken;

  //for actuators, no measurement taken
  return true;
}

const char *GenericActuator::getRawDataString()
{
  // debug("configuring driver template dataString");
  // process data string for .csv
  //sprintf(dataString, "%d,%0.3f",value,value*31.83);
  //return dataString;

  sprintf(dataString, "%d", value);
  return dataString; 
}

const char *GenericActuator::getSummaryDataString()
{
    //actuators likely don't need bursts / burst sumary? 
  // double burstSummaryMean = getBurstSummaryMean("var");
  // sprintf(dataString, "%0.3f,%0.3f", burstSummaryMean, burstSummaryMean*31.83);
  //return dataString
  sprintf(dataString, "%d", value);
  return dataString; 
 
}

const char *GenericActuator::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting driver template base column headers");
  return baseColumnHeaders;
}

void GenericActuator::initCalibration()
{
  // debug("init driver template sensor calibration");
}

void GenericActuator::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("driver template calibration steps");
}

//actuaotor funcs
void GenericActuator::actuateBeforeWarmUp()
{

}

void GenericActuator::actuateAfterMeasurementCycle()
{

}

  
void GenericActuator::actuatePeriodicalyMeasurementCycle() 
{
}

void GenericActuator::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  //memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}

configuration_bytes_partition GenericActuator::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  //memcpy(&partition, &configuration, sizeof(driver_configuration));
  return partition;
}

bool GenericActuator::configureDriverFromJSON(cJSON *json)
{
  return true;
}

void GenericActuator::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("appeding driver specific driver template json");
  
  //driver specific config, customize
  //addCalibrationParametersToJSON(json);
}

void GenericActuator::setDriverDefaults()
{
  // debug("setting driver template driver defaults");
  // set default values for driver struct specific values
  //configuration.cal_timestamp = 0;
}

void GenericActuator::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add driver template calibration parameters to json");
  //cJSON_AddNumberToObject(json, CALIBRATION_TIME_STRING, configuration.cal_timestamp);
}

