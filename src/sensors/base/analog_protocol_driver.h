#ifndef WATERBEAR_ANALOG_PROTOCOL_DRIVER
#define WATERBEAR_ANALOG_PROTOCOL_DRIVER

/*
*  Base class for sensor drivers using the Analog protocol
*/

#define ADC_SELECT_INTERNAL 0b00
#define ADC_SELECT_EXTERNAL 0b01

// TODO: Refactoring this to contain most of generic analog could be a good idea

class AnalogProtocolSensorDriver : public SensorDriver
{
public:
  ~AnalogProtocolSensorDriver();
  protocol_type getProtocol();
};

#endif