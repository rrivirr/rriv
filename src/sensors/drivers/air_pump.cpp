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

#include "sensors/drivers/air_pump.h"
#include "system/logs.h" // for debug() and notify()
#include "system/hardware.h"

// #include "system/measurement_components.h" // if external adc is used


AirPump::AirPump()
{
  // debug("allocating generic actuator");
}

AirPump::~AirPump(){}



const char * AirPump::getSensorTypeString()
{
  return sensorTypeString;
}

void AirPump::setup()
{
  //setup struct using JSON?  
  
   
}


bool AirPump::takeMeasurement()
{
  //AE temp fix: measurement turns pump on & off 
  //notify(F("in take measurement"));
  digitalWrite(GPIO_PIN_6, HIGH);
  //delay(configuration.dutyCycle*configuration.timeCycle*1000);
  delay(5000);
  digitalWrite(GPIO_PIN_6, LOW);
  delay(5000);
  // delay((1-configuration.dutyCycle)*configuration.timeCycle*1000);


  // // debug("taking measurement from driver template");
  // //return true if measurement taken store in class value(s), false if not
  addValueToBurstSummaryMean("actuate", 1); // use the default option for computing the burst summary value
  // return measurementTaken;

  //for actuators, no measurement taken
  return true;
}

const char *AirPump::getRawDataString()
{
  // // debug("configuring driver template dataString");
  // // process data string for .csv
  sprintf(dataString, "%d",1);
  return dataString;

}

const char *AirPump::getSummaryDataString()
{
  //   //actuators likely don't need bursts / burst sumary? 
  // double burstSummaryMean = getBurstSummaryMean("var");
  // sprintf(dataString, "%0.3f,%0.3f", burstSummaryMean, burstSummaryMean*31.83);
  // return dataString; 
  sprintf(dataString, "%d",1); 
  return dataString;
}

const char *AirPump::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting driver template base column headers");
  return baseColumnHeaders;
}

void AirPump::initCalibration()
{
  // debug("init driver template sensor calibration");
}

void AirPump::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("driver template calibration steps");
}


// unsigned int AirPump::millisecondsUntilNextReadingAvailable()
// {
//   return ((1-configuration.dutyCycle)*configuration.timeCycle*1000);
//   //return 5000; // return min by default, a larger number in driver implementation causes correct delay
// }

uint32 AirPump::millisecondsUntilNextRequestedReading()
{
  //return ((1-configuration.dutyCycle)*configuration.timeCycle*1000); // as slow as possible by default, a smaller number in driver implementation forces faster read
  return(10000);
}

void AirPump::stop()
{
  //stop
  digitalWrite(GPIO_PIN_6, LOW);
}

bool isWarmedUp()
{
  return true; 
}


//actuator funcs
 void AirPump::actuateBeforeWarmUp()
{
   
  // for(int i = 0; i < numIterations; i++)
  // {
  //   digitalWrite(GPIO_PIN_6, HIGH);
  //   delay(configuration.dutyCycle*configuration.timeCycle*1000);
  //   digitalWrite(GPIO_PIN_6, LOW);
  //   delay((1-configuration.dutyCycle)*configuration.timeCycle*1000);
  // }


}


void AirPump::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}

configuration_bytes_partition AirPump::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  memcpy(&partition, &configuration, sizeof(driver_configuration));
  return partition;
}

bool AirPump::configureDriverFromJSON(cJSON *json)
{
  // const cJSON *dutyCycleJSON = cJSON_GetObjectItemCaseSensitive(json, "duty_cycle");
  // const cJSON *flowRateJSON = cJSON_GetObjectItemCaseSensitive(json, "flow_rate");
  // const cJSON *volumeEvacuateJSON = cJSON_GetObjectItemCaseSensitive(json, "volume_evacuate");
  // const cJSON *timeCycleJSON = cJSON_GetObjectItemCaseSensitive(json, "time_cycle");
  // configuration.dutyCycle = (byte)dutyCycleJSON->valueint;
  // configuration.flowRate = (byte)flowRateJSON->valueint;
  // configuration.volumeEvacuate = (byte)volumeEvacuateJSON->valueint;
  // configuration.timeCycle = (byte)timeCycleJSON->valueint;
  

  return true;
}

void AirPump::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("appeding driver specific driver template json");
  
  //driver specific config, customize
  addCalibrationParametersToJSON(json);
}

void AirPump::setDriverDefaults()
{
  // debug("setting driver template driver defaults");
  // set default values for driver struct specific values
  //configuration.cal_timestamp = 0;
  // configuration.dutyCycle = 0.5;
  // configuration.flowRate = 1;
  configuration.cal_timestamp = 0;

}

// void AirPump::calcIterations(int* iterations, float duty, float rate, float vol, int cycle)
// {
//    //*numIterations = 5; //ceil((configuration.volEvacuate/(configuration.flowRate/60))/(configuration.dutyCycle*configuration.timeCycle));
 
// }



void AirPump::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add driver template calibration parameters to json");
  //cJSON_AddNumberToObject(json, CALIBRATION_TIME_STRING, configuration.cal_timestamp);
  //cJSON_AddNumberToObject(json, DUTY_CYCLE_TYPE_STRING, configuration.dutyCycle);
  cJSON_AddNumberToObject(json, CALIBRATION_TIME_STRING, configuration.cal_timestamp);
}


