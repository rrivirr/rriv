#include "sensors/sensor.h"
#include "EC_OEM.h"


typedef struct generic_atlas_type // 64 bytes
{
    common_config_sensor common; // 32 bytes
    unsigned long long cal_timestamp; // 4byte epoch timestamp at calibration
    char padding[24];
} generic_atlas_sensor;


class AtlasEC : public I2CSensorDriver
{

  public: 
    // Constructor
    AtlasEC();
    ~AtlasEC();

    // Interface
    void setup();
    void configure(generic_config * configuration);
    generic_config getConfiguration();
    void setConfiguration(generic_config configuration);
    cJSON * getConfigurationJSON(); // returns unprotected pointer
    void stop();
    bool takeMeasurement();
    char * getDataString();
    char * getCSVColumnNames();
    protocol_type getProtocol();
    const char * getBaseColumnHeaders();

    void initCalibration();
    void calibrationStep(char * step, int value);
    void addCalibrationParametersToJSON(cJSON * json);

  protected:
    // Implementatino interface
    void setDriverDefaults();
    void configureDriverFromJSON(cJSON * json);

  private:
    generic_atlas_sensor configuration;
    EC_OEM *oem_ec;

    int value;
    const char * baseColumnHeaders = "ec.mS";
    char dataString[16];

};