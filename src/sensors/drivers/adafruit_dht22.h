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
#ifndef WATERBEAR_ADAFRUIT_DHT22
#define WATERBEAR_ADAFRUIT_DHT22

#include "sensors/sensor.h"
#include "sensors/sensor_types.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN PC11     // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT22   // DHT 22 (AM2302)

typedef struct adafruit_dht22_type // 64 bytes
{
  common_sensor_driver_config common;      // 32 bytes
  unsigned long long cal_timestamp; // 8 bytes for epoch time of calibration (optional)

  char padding[24]; // space to be used for any sensor specific variables

} adafruit_dht22_sensor;


class AdaDHT22 : public GPIOProtocolSensorDriver
{
  public:
    // Constructor
    AdaDHT22();
    ~AdaDHT22();

    // Interface
    generic_config getConfiguration();
    void setConfiguration(generic_config configuration);
    void appendDriverSpecificConfigurationJSON(cJSON * json);
    void setup();
    void stop();
    bool takeMeasurement();
    const char * getDataString();
    const char * getBaseColumnHeaders();
    void initCalibration();
    void calibrationStep(char *step, int arg_cnt, char ** args);

  protected:
    void configureDriverFromJSON(cJSON *json);
    void setDriverDefaults();

  private:
    adafruit_dht22_sensor configuration;
    DHT_Unified *dht;

    float temperature;
    float humidity;
    const char *baseColumnHeaders = "C,RH"; // will be written to .csv
    char dataString[16]; // will be written to .csv

    void addCalibrationParametersToJSON(cJSON *json);
};

#endif