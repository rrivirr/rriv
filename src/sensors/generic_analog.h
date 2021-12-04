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
    short calibrated: 1;
    short adc_select: 2;  // two bits, support hardware expansion (addnl adc chips)
    short sensor_port: 4;
    short reserved: 1;
    unsigned short m; // 2bytes, slope
    int b; // 4bytes, y-intercept
    unsigned int cal_timestamp; // 4byte epoch timestamp at calibration
    short x1; // 2bytes for 2pt calibration
    short y1; // 2bytes for 2pt calibration
    short x2; // 2bytes for 2pt calibration
    short y2; // 2bytes for 2pt calibration

    char padding[12];
} generic_linear_analog_sensor;

class GenericAnalog : public AnalogSensorDriver
{

  public: 
    // Constructor
    GenericAnalog();
    void configureFromJSON(cJSON * json);

    // Interface
    void setup();
    void configure(generic_config * configuration);
    generic_config getConfiguration();
    cJSON * getConfigurationJSON(); // returns unprotected pointer
    void stop();
    bool takeMeasurement();
    char * getDataString();
    char * getCSVColumnNames();
    protocol_type getProtocol();
    const char * getBaseColumnHeaders();

  private:
    generic_linear_analog_sensor configuration;

    int value;
    const char * baseColumnHeaders = "raw";
    char dataString[16];
};