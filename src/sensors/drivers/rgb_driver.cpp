#include "sensors/drivers/rgb_driver.h"
#include "system/logs.h" // for debug() and notify()
// #include "system/measurement_components.h" // if external adc is used

#define RGBI2CADDRESS 0x70
#define RGBDELAY 300

#define REDVALUE_TAG "redValue"
#define GREENVALUE_TAG "greenValue"
#define BLUEVALUE_TAG "blueValue"
#define LUX_TAG "lux"
#define CIExVALUE_TAG "ciexValue"
#define CIEyVALUE_TAG "cieyValue"
#define CIEYVALUE_TAG "cieYValue"

rgbDriver::rgbDriver()
{
  // debug("allocating driver template");
}

rgbDriver::~rgbDriver(){}

const char * rgbDriver::getSensorTypeString()
{
  return sensorTypeString;
}

configuration_bytes_partition rgbDriver::getDriverSpecificConfigurationBytes()
{
  configuration_bytes_partition partition;
  memcpy(&partition, &configuration, sizeof(driver_configuration));
  return partition;
}

bool rgbDriver::configureDriverFromJSON(cJSON *json)
{
}


void rgbDriver::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
  memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}

void rgbDriver::appendDriverSpecificConfigurationJSON(cJSON * json)
{
  // debug("appeding driver specific driver template json");
  
  //driver specific config, customize
  addCalibrationParametersToJSON(json);
}

void rgbDriver::setup()
{
  const char * turnOnRGBParameterCommand = "O,RGB,1";
  const char * turnOnLUXParameterCommand = "O,LUX,1";
  const char * turnOnCIEParameterCommand = "O,CIE,1";
  
  wire->beginTransmission(RGBI2CADDRESS);
  wire->write(turnOnRGBParameterCommand);
  wire->endTransmission(); 
  
  delay(RGBDELAY);

  wire->beginTransmission(RGBI2CADDRESS);
  wire->write(turnOnLUXParameterCommand);
  wire->endTransmission(); 

  delay(RGBDELAY);
  
  wire->beginTransmission(RGBI2CADDRESS);
  wire->write(turnOnCIEParameterCommand);
  wire->endTransmission(); 

  delay(RGBDELAY);
}

void rgbDriver::stop()
{
  const char * sleepCommand = "Sleep";
  wire->beginTransmission(RGBI2CADDRESS);
  wire->write(sleepCommand);
  wire->endTransmission();
}

bool rgbDriver::takeMeasurement()
{
  

  // debug("taking measurement from driver template");
  // return true if measurement taken store in class value(s), false if not
  const char *RGBRead = "R";
  wire->beginTransmission(RGBI2CADDRESS);
  wire->write(RGBRead);
  wire->endTransmission();

  delay(RGBDELAY);             

  wire->requestFrom(RGBI2CADDRESS, 43, 1);

  byte code = wire->read(); // the first byte is the response code, we read this separately.

  switch (code)
  {                            // switch case based on what the response code is.
  case 1:                      // decimal 1.
    debug("Success"); // means the command was successful.
    break;                     // exits the switch case.

  case 2:                     // decimal 2.
    notify("Failed"); // means the command has failed.
    return false;
    break;                    // exits the switch case.

  case 254:                    // decimal 254.
    notify("Pending"); // means the command has not yet been finished calculating.
    return false;
    break;                     // exits the switch case.

  case 255:                    // decimal 255.
    notify("No Data"); // means there is no further data to send.
    return false;
    break;                     // exits the switch case.
  }

  int i = 0;
  while (wire->available())
  {                               // are there bytes to receive.
    char readByte = wire->read(); // receive a byte.
    value[i] = readByte;          // load this byte into our array.
    i += 1;                       // incur the counter for the array element.
    if (readByte == 0)
    {        // if we see that we have been sent a null command.
      break; // exit the while loop.
    }
  }
  //notify(value);
     /* get the first token */
    char * token = strtok(value, ",");
    if(token==NULL){return false;}
    redValue = atoi(token);
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    greenValue = atoi(token);
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    blueValue = atoi(token);
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    lux = atoi(token);
    token = strtok(NULL, ",");
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    ciexValue = atof(token);
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    cieyValue = atof(token);
    token = strtok(NULL, ",");
    if(token==NULL){return false;}
    cieYValue = atof(token);

    addValueToBurstSummaryMean(REDVALUE_TAG, redValue);
    addValueToBurstSummaryMean(GREENVALUE_TAG, greenValue);
    addValueToBurstSummaryMean(BLUEVALUE_TAG, blueValue);
    addValueToBurstSummaryMean(LUX_TAG, lux);
    addValueToBurstSummaryMean(CIExVALUE_TAG, ciexValue);
    addValueToBurstSummaryMean(CIEyVALUE_TAG, cieyValue);
    addValueToBurstSummaryMean(CIEYVALUE_TAG, cieYValue);
  return true;
}

const char *rgbDriver::getRawDataString()
{
  // debug("configuring driver template dataString");
  // process data string for .csv
  //sprintf(dataString, "%d,%d",value,int(value*31.83));
  sprintf(dataString, "%d,%d,%d,%d,%.3f,%.3f,%f",redValue,greenValue,blueValue,lux,ciexValue,cieyValue,cieYValue);
  return dataString;
}

const char *rgbDriver::getSummaryDataString()
{
  //double burstSummaryMean = getBurstSummaryMean(VAR_TAG);
  sprintf(dataString, "%d,%d,%d,%d,%.3f,%.3f,%f", getBurstSummaryMean(REDVALUE_TAG),getBurstSummaryMean(GREENVALUE_TAG),
  getBurstSummaryMean(BLUEVALUE_TAG),getBurstSummaryMean(LUX_TAG),getBurstSummaryMean(CIExVALUE_TAG),
  getBurstSummaryMean(CIEyVALUE_TAG),getBurstSummaryMean(CIEYVALUE_TAG));
  return dataString;
}

const char *rgbDriver::getBaseColumnHeaders()
{
  // for debug column headers defined in the .h
  // debug("getting driver template base column headers");
  return baseColumnHeaders;
}

void rgbDriver::initCalibration()
{
  // debug("init driver template sensor calibration");
}

void rgbDriver::calibrationStep(char *step, int arg_cnt, char ** args)
{
  // for intermediary steps of calibration process
  // debug("driver template calibration steps");
}

void rgbDriver::addCalibrationParametersToJSON(cJSON *json)
{
  // follows structure of calibration parameters in .h
  // debug("add driver template calibration parameters to json");
  cJSON_AddNumberToObject(json, "calibration_time", configuration.cal_timestamp);
}

void rgbDriver::setDriverDefaults()
{
  // debug("setting driver template driver defaults");
  // set default values for driver struct specific values
  configuration.cal_timestamp = 0;
}

unsigned int rgbDriver::millisecondsUntilNextRequestedReading()
{
  return 0;
}
