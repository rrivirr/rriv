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

typedef struct generic_linear_analog_type
{
  // analog sensor that can be 2pt calibrated
    common_config_sensor common;
    char calibrated; // 1 byte => bit mask
    char sensor_port; // 1 byte => add into bit mask (4bits)
    unsigned short m; // 2bytes, slope
    int b; // 4bytes, y-intercept
    unsigned int cal_timestamp; // 4byte epoch timestamp at calibration
    short int x1; // 2bytes for 2pt calibration
    short int y1; // 2bytes for 2pt calibration
    short int x2; // 2bytes for 2pt calibration
    short int y2; // 2bytes for 2pt calibration

    char padding[12];
}generic_linear_analog_sensor;