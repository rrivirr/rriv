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
#ifndef RGB_DRIVER
#define RGB_DRIVER

#include "sensors/sensor.h"


#define RGB_DRIVER_TYPE_STRING "atlas_rgb"

class rgbDriver : public I2CProtocolSensorDriver
{
  // configuration parameters specific to this driver
  typedef struct
  {
    unsigned long long cal_timestamp; // 8 bytes for epoch time of calibration
  } driver_configuration;

  public:
    // Constructor
    rgbDriver();
    ~rgbDriver();

    //
    // Interface Implementation
    //
    const char * getSensorTypeString();
    void setup();
    void stop();
    bool takeMeasurement();
    const char * getRawDataString();
    const char * getSummaryDataString();
    const char * getBaseColumnHeaders();
    void initCalibration();
    void calibrationStep(char *step, int arg_cnt, char ** args);
    uint32 millisecondsUntilNextReadingAvailable();
    bool isWarmedUp();
    int millisecondsToWarmUp();

  protected:
    void configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurations);
    configuration_bytes_partition getDriverSpecificConfigurationBytes();
    bool configureDriverFromJSON(cJSON *json);
    void appendDriverSpecificConfigurationJSON(cJSON *json);
    void setDriverDefaults();

  private:
    //sensor specific variables, functions, etc.
    const char *sensorTypeString = RGB_DRIVER_TYPE_STRING;
    driver_configuration configuration;

    /*value(s) to be placed in dataString, should correspond to number of 
    column headers and entries in dataString*/
    char value[42]; // sensor raw return(s) to be added to dataString
    // Example of data string from instrument. May be incorrect.
    // XXX,XXX,XXX,xyY,X.XXX,X.XXX,XXX,Lux,XXXXX
    const char *baseColumnHeaders = "value"; // will be written to .csv
    char dataString[16]; // will be written to .csv
    uint32 setupTime; // for unix time of setup to track when ready to take samples
    int timeDiff;

    void addCalibrationParametersToJSON(cJSON *json);
};

#endif