#include "sensor.h"

class GenericAnalog : public AnalogSensorDriver
{

  public: 
    // Constructor
    GenericAnalog();

    // Interface
    void setup();
    void stop();
    bool takeMeasurement();
    char * getDataString();
    char * getCSVColumns();
    protocol_type getProtocol();

  private:
    char csvColumnHeaders[100] = "ms,ne,h";

    char dataString[16];
};