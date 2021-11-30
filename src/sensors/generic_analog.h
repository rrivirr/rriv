#include "sensor.h"

#define ADC_SELECT_INTERNAL 0b00
#define ADC_SELECT_EXTERNAL 0b01

#define ANALOG_INPUT_1_PIN PB1 // A2
#define ANALOG_INPUT_2_PIN PC0 // A3
#define ANALOG_INPUT_3_PIN PC1 // A4
#define ANALOG_INPUT_4_PIN PC2 // A5
#define ANALOG_INPUT_5_PIN PC3 // A6

typedef struct generic_linear_analog_type // 32 bytes
{
  // analog sensor that can be 2pt calibrated
    common_config_sensor common;
    byte calibrated: 1;
    byte adc_select: 2;  // two bits, support hardware expansion (addnl adc chips)
    byte sensor_port: 4;
    byte reserved: 1;
    unsigned short m; // 2bytes, slope
    int b; // 4bytes, y-intercept
    unsigned int cal_timestamp; // 4byte epoch timestamp at calibration
    short int x1; // 2bytes for 2pt calibration
    short int y1; // 2bytes for 2pt calibration
    short int x2; // 2bytes for 2pt calibration
    short int y2; // 2bytes for 2pt calibration

    char padding[13];
} generic_linear_analog_sensor;

class GenericAnalog : public AnalogSensorDriver
{

  public: 
    // Constructor
    GenericAnalog();

    // Interface
    void setup();
    void configure(generic_config * configuration);
    void stop();
    bool takeMeasurement();
    char * getDataString();
    char * getCSVColumns();
    protocol_type getProtocol();

  private:
    generic_linear_analog_sensor configuration;
    char csvColumnHeaders[100] = "raw";

    int value;
    char dataString[16];
};