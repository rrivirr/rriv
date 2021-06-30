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
    int TempMV = -1;
    int TempC = -1;
    int TempCMult = 100;
    int CalB = 5;
    int CalBMult = 10;
    int CalM = 0.2;
    int CalMMult = 10;
};