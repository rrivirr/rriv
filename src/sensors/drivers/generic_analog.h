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

#include "sensors/sensor.h"

#define ADC_SELECT_INTERNAL 0b00
#define ADC_SELECT_EXTERNAL 0b01

#define ANALOG_INPUT_1_PIN PB1 // A2
#define ANALOG_INPUT_2_PIN PC0 // A3
#define ANALOG_INPUT_3_PIN PC1 // A4
#define ANALOG_INPUT_4_PIN PC2 // A5
#define ANALOG_INPUT_5_PIN PC3 // A6

typedef struct generic_linear_analog_type // 64 bytes
{
  // analog sensor that can be 2pt calibrated
    common_config_sensor common; // 32 bytes
    short adc_select: 2;  // two bits, support hardware expansion (addnl adc chips)
    short sensor_port: 4;
    short calibrated: 1;
    short reserved: 1;
    unsigned short m; // 2bytes, slope
    int b; // 4bytes, y-intercept
    unsigned long long cal_timestamp; // 8 byte epoch timestamp at calibration
    short x1; // 2bytes for 2pt calibration
    short y1; // 2bytes for 2pt calibration
    short x2; // 2bytes for 2pt calibration
    short y2; // 2bytes for 2pt calibration

    char padding[8];
} generic_linear_analog_sensor;

class GenericAnalog : public AnalogSensorDriver
{

  public: 
    // Constructor
    GenericAnalog();
    ~GenericAnalog();

    // Interface
    void setup();
    void setConfiguration(generic_config configuration);
    generic_config getConfiguration();
    cJSON * getConfigurationJSON(); // returns unprotected pointer
    void stop();
    bool takeMeasurement();
    char * getDataString();
    char * getCSVColumnNames();
    protocol_type getProtocol();
    const char * getBaseColumnHeaders();

    void initCalibration();
    void calibrationStep(char * step, int value);
    void addCalibrationParametersToJSON(cJSON * json);

  protected:
    void setDriverDefaults();
    void configureDriverFromJSON(cJSON * json);


  private:
    generic_linear_analog_sensor configuration;

    int value;
    const char * baseColumnHeaders = "raw,cal";
    char dataString[16];

    int calibrate_high_reading = 0;
    int calibrate_high_value = 0;
    int calibrate_low_reading = 0;
    int calibrate_low_value = 0;

    void computeCalibratedCurve();
    void printCalibrationStatus();

};