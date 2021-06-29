#include "sensor.h"

class GenericAnalog : public AnalogSensorDriver
{

  public: 
    // Constructor
    GenericAnalog();

    // Interface
    void stop();
    void takeMeasurement();
    char * getDataString();
    char * getCSVColumns();
    protocol_type getProtocol();

  private:
    char dataString[16];
};