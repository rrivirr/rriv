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
    virtual void stop();
    virtual bool takeMeasurement(); // return true if measurement successful
    virtual char * getDataString();
    virtual char * getCSVColumnNames();
    virtual protocol_type getProtocol();
    
    virtual void initializeBurst();
    virtual void incrementBurst();
    virtual bool burstCompleted();

  private:
    short burstCount = 0;
};

class AnalogSensorDriver : public SensorDriver {
  public:
    virtual void setup();
};

class I2CSensorDriver : public SensorDriver {
  public:
    virtual void setup(TwoWire * wire);
};




#endif