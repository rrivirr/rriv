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
#ifndef WATERBEAR_ATLAS_EC
#define WATERBEAR_ATLAS_EC

#include "sensors/sensor.h"
#include "EC_OEM.h"


typedef struct  // 64 bytes
{
    common_sensor_driver_config common; // 32 bytes
    unsigned long long cal_timestamp; // 8byte epoch timestamp at calibration // TODO: move to common
    char padding[24];
} generic_atlas_config;


class AtlasECDriver : public I2CProtocolSensorDriver
{

  public: 
    AtlasECDriver();
    ~AtlasECDriver();
  
  private:
    generic_atlas_config configuration;
    EC_OEM *oem_ec; // A pointer to the I2C driver for the Atlas EC sensor

    int value;
    const char * baseColumnHeaders = "ec.mS";
    char dataString[20]; // local storage for data string

  //
  // Interface Implementation
  //
  public:
    void setup();
    void configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurations);
    configuration_bytes_partition getDriverSpecificConfigurationBytes();
    void appendDriverSpecificConfigurationJSON(cJSON * json);
    void stop();
    bool takeMeasurement();
    const char * getDataString();
    const char * getBaseColumnHeaders();

    void initCalibration();
    void calibrationStep(char * step, int arg_cnt, char ** args);
    void addCalibrationParametersToJSON(cJSON * json);

  protected:
    // Implementatino interface
    void setDriverDefaults();
    void configureDriverFromJSON(cJSON * json);



};

#endif