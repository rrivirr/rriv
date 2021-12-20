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

#define SENSOR_CONFIGURATION_SIZE 64

typedef struct 
{
    // note needs to be 32 bytes total (multiple of 4)
    // rearrange in blocks of 4bytes for diagram
    // sensor.h
    ushort sensor_type; // 2 bytes
    byte slot; // 1 byte
    byte burst_size; // 1 byte
    unsigned short int warmup; // 2 bytes, in seconds? (65535/60=1092)
    char tag[4]; // 4 bytes
       
    char padding[22]; // 17bytes
} common_config_sensor;

typedef struct 
{
  common_config_sensor common;
  char padding[32];
} generic_config;


class SensorDriver {

  public: 
    // Constructor
    SensorDriver();
    virtual ~SensorDriver();
    virtual void configureFromJSON(cJSON * json);


    // Interface
    virtual void configure(generic_config * configuration); // pass block of configuration memory, read from EEPROM
    virtual generic_config getConfiguration();
    virtual cJSON * getConfigurationJSON(); // returns unprotected pointer
    virtual void stop();
    virtual bool takeMeasurement(); // return true if measurement successful
    virtual char * getDataString();
    virtual char * getCSVColumnNames();
    virtual protocol_type getProtocol();
    virtual const char * getBaseColumnHeaders();
    

    // // Calibration
    virtual void initCalibration();
    virtual void calibrationStep(char * step, int value);

    void initializeBurst();
    void incrementBurst();
    bool burstCompleted();

  protected:
    char csvColumnHeaders[100] = "column_header";
    void configureCommonFromJSON(cJSON * json, common_config_sensor * common);
    void configureCSVColumns();
    void setCommonDefaults(common_config_sensor * common);

  private:
    short burstCount = 0;

};

class AnalogSensorDriver : public SensorDriver {
  public:
    ~AnalogSensorDriver();
    virtual void setup();
};

class I2CSensorDriver : public SensorDriver {
  public:
    ~I2CSensorDriver();
    virtual void setup(TwoWire * wire);
};


void getDefaultsCommon(common_config_sensor *fillValues);
void readCommonConfigOnly(common_config_sensor *readValues); //not made


#endif