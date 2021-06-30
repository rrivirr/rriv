#include "sensor.h"

class TempAnalog : public AnalogSensorDriver
{

  public: 
    // Constructor
    TempAnalog();

    // Interface
    void stop();
    bool takeMeasurement();
    char * getDataString();
    const char * getCSVColumns();
    protocol_type getProtocol();

  private:
    char dataString[16];
    const char CSVColumns[100] = {"temp.mv, temp.c, temp.slope, temp.int"};

};