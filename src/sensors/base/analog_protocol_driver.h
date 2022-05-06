#ifndef WATERBEAR_ANALOG_PROTOCOL_DRIVER
#define WATERBEAR_ANALOG_PROTOCOL_DRIVER

#include "sensors/sensor.h"

/*
 *  Base class for sensor drivers using the Analog protocol
 */

#define ADC_SELECT_INTERNAL 0b00
#define ADC_SELECT_EXTERNAL 0b01

// TODO: Refactoring this to contain most of generic analog could be a good idea

class AnalogProtocolSensorDriver : public SensorDriver
{
public:
  AnalogProtocolSensorDriver();
  ~AnalogProtocolSensorDriver();
  protocol_type getProtocol();

  //
  // Interface Implementation
  //
  void setup();
  void stop();
  bool takeMeasurement();
  const char *getDataString();
  const char *getBaseColumnHeaders();
  void initCalibration();
  void calibrationStep(char *step, int arg_cnt, char **args);

protected:
  void configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurations);
  configuration_bytes_partition getDriverSpecificConfigurationBytes();
  void configureDriverFromJSON(cJSON *json);
  void appendDriverSpecificConfigurationJSON(cJSON *json);
  void setDriverDefaults();
};

#endif