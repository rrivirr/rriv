#include "generic_analog.h"
#include "system/monitor.h"

int ADC_PINS[5] = {
  ANALOG_INPUT_1_PIN, 
  ANALOG_INPUT_2_PIN, 
  ANALOG_INPUT_3_PIN, 
  ANALOG_INPUT_4_PIN, 
  ANALOG_INPUT_5_PIN
};

GenericAnalog::GenericAnalog()
{
  Serial2.println("hello");
  Serial2.flush();
  // debug("allocation GenericAnalog");
}

void GenericAnalog::setup()
{
  debug("setup GenericAnalog");
}

void GenericAnalog::configure(generic_config * configuration)
{
  memcpy(&this->configuration, configuration, sizeof(generic_linear_analog_sensor));
}

void GenericAnalog::stop(){}

bool GenericAnalog::takeMeasurement(){
  // take measurement and write to dataString member variable
  switch(this->configuration.adc_select)
  {
    case ADC_SELECT_INTERNAL:
    {
      int adcPin = ADC_PINS[this->configuration.sensor_port];
      this->value = analogRead(adcPin);
    }
      break;
    
    case ADC_SELECT_EXTERNAL:
    {}
      break;

    default:
    {}       // bad configuration
      break;
  }

  return true;
}

char * GenericAnalog::getDataString(){
  sprintf(dataString, "%d", value);
  return dataString;
}

char * GenericAnalog::getCSVColumns(){
   return csvColumnHeaders;
}

protocol_type GenericAnalog::getProtocol(){
  return analog;
}