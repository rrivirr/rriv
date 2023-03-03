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
#ifndef WATERBEAR_GENERIC_ACTUATOR
#define WATERBEAR_GENERIC_ACTUATOR

#include "sensors/sensor.h"

#define GENERIC_ACTUATOR_TYPE_STRING "generic_actuator"


class GenericActuator: public GenericActuatorProtocolSensorDriver
{

    typedef struct 
    {
        unsigned long long cal_timestamp; // 8 bytes for epoch time of calibration
        //unsigned int GPIO_pin : 6; //is this number of bits or the pin number? 

    } driver_configuration;
  
    public:
        //constructor 
        GenericActuator();
        ~GenericActuator();

        protocol_type getProtocol();
        const char * getSensorTypeString(); //is this necessary? 
        virtual void setup();
        virtual void stop();
        bool turnOn();
        bool takeMeasurement(); //for actuators measurement is turning pump on / off 
        const char * getRawDataString();
        const char * getBaseColumnHeaders();
        const char * getSummaryDataString();
        void initCalibration(); //needed? 
        void calibrationStep(char *step, int arg_cnt, char ** args); //do actuators need to be calibrated? 
        

        //temp for treating air pump as sensor 
        // virtual unsigned int millisecondsUntilNextReadingAvailable();
        // virtual unsigned int millisecondsUntilNextRequestedReading();
        //void setPulseWidth();
        //void setLoopFrequency();  // frequency of occurences in data logging loop 

        virtual void actuateBeforeWarmUp();
        virtual void actuateAfterMeasurementCycle();
        virtual void actuatePeriodicalyMeasurementCycle();
        


    protected:
        void configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurations);
        configuration_bytes_partition getDriverSpecificConfigurationBytes();
        bool configureDriverFromJSON(cJSON *json);
        void appendDriverSpecificConfigurationJSON(cJSON *json);
        void setDriverDefaults();

        

    private:
         //actuator specific variables, functions, etc.
        const char *sensorTypeString = GENERIC_ACTUATOR_TYPE_STRING;
        driver_configuration configuration;
       

        /*FROM DRIVER TEMPLATE. MAY NOT BE NEEDED. ACTUATORS DON'T WRITE TO CSV? : 
        value(s) to be placed in dataString, should correspond to number of 
        column headers and entries in dataString*/
        int value; // sensor raw return(s) to be added to dataString
        const char *baseColumnHeaders = "raw,cal"; // will be written to .csv
        char dataString[16]; // will be written to .csv

        void addCalibrationParametersToJSON(cJSON *json);
  
};

#endif