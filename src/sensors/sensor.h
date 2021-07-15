#ifndef WATERBEAR_SENSOR_DRIVER
#define WATERBEAR_SENSOR_DRIVER

#include <Arduino.h>
#include <Wire_slave.h>
#include <cJSON.h>

#define SENSOR_PORT_A2 0b0001
#define SENSOR_PORT_A3 0b0010
#define SENSOR_PORT_A4 0b0011
#define SENSOR_PORT_A5 0b0100
#define SENSOR_PORT_A6 0b0101
#define SENSOR_PORT_ADC1 0b0110
#define SENSOR_PORT_ADC2 0b0111
#define SENSOR_PORT_ADC3 0b1000
#define SENSOR_PORT_ADC4 0b1001

#define SENSOR_TYPE_LENGTH 2

#define NTC_10K_THERMISTOR 1
#define AS_CONDUCTIVITY 2
#define AS_RGB 3
#define AS_CO2 4
#define FIG_METHANE 5
#define AF_GPS 6

typedef enum protocol { analog, i2c } protocol_type;

class SensorDriver {

  public: 
    // Constructor
    SensorDriver();

    // Interface
    virtual void configure(void * configuration); // pass block of configuration memory, read from EEPROM
    virtual void stop();
    virtual bool takeMeasurement(); // return true if measurement successful
    virtual char * getDataString();
    virtual char * getCSVColumnNames();
    virtual protocol_type getProtocol();
    
    // JSON
    virtual char * exportConfigurationJSON(); // TODO: where should memory be malloc'd?
    virtual void loadConfigurationJSON(char * configurationJSON);
    cJSON * JSON;

    // Calibration
    virtual void initCalibration();
    virtual void calibrationStep(char * step, int value);

    void initializeBurst();
    void incrementBurst();
    bool burstCompleted();

  private:
    short burstCount = 0;
    
    // replace these with a struct
    short burstLength = 10;
};

class AnalogSensorDriver : public SensorDriver {
  public:
    virtual void setup();
};

class I2CSensorDriver : public SensorDriver {
  public:
    virtual void setup(TwoWire * wire);
};

typedef struct common_config{
    // note needs to be 32 bytes total (multiple of 4)
    // rearrange in blocks of 4bytes for diagram
    // sensor.h
    short sensor_type; // 2 bytes
    byte slot; // 1 byte
    byte sensor_burst; // 1 byte
    unsigned short int warmup; // 2 bytes, in seconds? (65535/60=1092)
    char tag[4]; // 4 bytes
    char column_prefix[5]; // 5 bytes
       
    char padding[17]; // 17bytes
}common_config_sensor;

void getDefaultsCommon(common_config_sensor *fillValues);
void readCommonConfigOnly(common_config_sensor *readValues); //not made


#endif