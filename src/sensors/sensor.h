#ifndef WATERBEAR_SENSOR_DRIVER
#define WATERBEAR_SENSOR_DRIVER

#include <Arduino.h>
#include <Wire_slave.h>

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
    virtual const char * getCSVColumnNames();
    virtual protocol_type getProtocol();
    
    // JSON
    virtual char * exportConfigurationJSON(); // TODO: where should memory be malloc'd?
    virtual void loadConfigurationJSON(char * configurationJSON);

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
    char slot; // 1 byte
    char column_prefix[5]; // 5 bytes
    char sensor_burst; // 1 byte
    unsigned short int warmup; // 2 bytes, in seconds? (65535/60=1092)
    char tag[4]; // 4 bytes

    char padding[17]; // 17bytes
}common_config_sensor;

#endif