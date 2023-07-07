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

#ifndef WATERBEAR_AIR_PUMP
#define WATERBEAR_AIR_PUMP

#include "sensors/sensor.h"
#include "generic_actuator.h"

#define AIR_PUMP_TYPE_STRING "air_pump"

//for json stuff? 
#define DUTY_CYCLE_TYPE_STRING reinterpret_cast<const char*>(F("duty_cycle"))
#define FLOW_RATE_TYPE_STRING reinterpret_cast<const char*>(F("flow_rate"))
#define VOLUME_TYPE_STRING reinterpret_cast<const char*>(F("volume_evacuate"))
#define TIME_CYCLE_TYPE_STRING reinterpret_cast<const char*>(F("time_cycle"))

class AirPump : public GenericActuator
{
    typedef struct
    {
        //unsigned long long cal_timestamp;   // 8 bytes for epoch time of calibration (optional)
        //short sensor_pin : 5; //4 bits (16 states) (this is GPIO pin?)

        //note maxiumum duty cycle maximum is 1 
        //note for this air pump maximum time on is 5 seconds, so maximum time cycle 10. 
        // float dutyCycle = 0.5;
        // float flowRate = 2.5; // needs to be in L / min
        // float volumeEvacuate = 7.57; //needs to be in L 
        // int timeCycle = 4; //total time of 1 on off cycle in seconds
      
        //float timeOn = (volumeEvacuate/(flowRate/60))*dutyCycle;
        //float timeOn = dutyCycle*totalTimeCycle; //in seconds, based off maximum rating (not consistent for all types air pumps? )
        
        //float timeOff = (volumeEvacuate/(flowRate/60))*(1-dutyCycle);
        //float timeOff = (1-dutyCycle)*totalTimeCycle;
        // int numIterations = ceil((volumeEvacuate/(flowRate/60))/(dutyCycle*timeCycle));
        // int numIterations = 37;
        unsigned long long cal_timestamp;
  
    
    } driver_configuration;

    public:
    //Constructor
    AirPump();
    ~AirPump();

    const char * getSensorTypeString();
    void setup();
    void stop();
    bool takeMeasurement();
    const char * getRawDataString();
    const char * getSummaryDataString();
    const char * getBaseColumnHeaders();
    void initCalibration();
    void calibrationStep(char *step, int arg_cnt, char ** args);

  
    //unsigned int millisecondsUntilNextReadingAvailable();
    uint32 millisecondsUntilNextRequestedReading();
  



    void actuateBeforeWarmUp();
    void actuateAfterMeasurementCycle();
    //needs renaiming, function for actuators every X measurement cycles
    void actuatePeriodicalyMeasurementCycle();

    protected:
    void configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurations);
    configuration_bytes_partition getDriverSpecificConfigurationBytes();
    bool configureDriverFromJSON(cJSON *json); //name correct?? 
    bool configureFromJSON(cJSON *json);
    void appendDriverSpecificConfigurationJSON(cJSON *json);
    void setDriverDefaults();
    


    private:
    const char *sensorTypeString = AIR_PUMP_TYPE_STRING;
    driver_configuration configuration;
    
    // float dutyCycle; //rating of duty cycle 
    // float flowRate; //in Liters / Min
    // float volumeEvacuate; //total volume to evacuate
    //float totalTime; //total time air pump needs to be on
    // float timeOn; //time air pump on
    // float timeOff; //time air pump off
    //int numIterations = ceil((configuration.volEvacuate/(configuration.flowRate/60))/(configuration.dutyCycle*configuration.timeCycle));
    


    //void calcIterations(int* iterations, float duty, float rate, float vol, int cycle);
    // int value; // sensor raw return(s) to be added to dataString
    const char *baseColumnHeaders = "actuate"; // will be written to .csv
    char dataString[16]; // will be written to .csv

    
    void addCalibrationParametersToJSON(cJSON *json);
    

};

#endif