#include "sensors/drivers/MHZ16_CO2.h"
#include "system/logs.h" // for debug() and notify()
// #include "system/measurement_components.h" // if external adc is used

#define VAR_TAG "var"

MHZ16_CO2Driver::MHZ16_CO2Driver()
{
    // debug("allocating driver template");
}

MHZ16_CO2Driver::~MHZ16_CO2Driver() {}

const char *MHZ16_CO2Driver::getSensorTypeString()
{
    return sensorTypeString;
}

configuration_bytes_partition MHZ16_CO2Driver::getDriverSpecificConfigurationBytes()
{
    configuration_bytes_partition partition;
    memcpy(&partition, &configuration, sizeof(driver_configuration));
    return partition;
}

bool MHZ16_CO2Driver::configureDriverFromJSON(cJSON *json)
{
    return true;
}

void MHZ16_CO2Driver::configureSpecificConfigurationsFromBytes(configuration_bytes_partition configurationPartition)
{
    memcpy(&configuration, &configurationPartition, sizeof(driver_configuration));
}

void MHZ16_CO2Driver::appendDriverSpecificConfigurationJSON(cJSON *json)
{
    // debug("appeding driver specific driver template json");

    // driver specific config, customize
    addCalibrationParametersToJSON(json);
}

void MHZ16_CO2Driver::setup()
{
    // debug("setup MHZ16_CO2Driver");
}

void MHZ16_CO2Driver::stop()
{
    // debug("stop/delete MHZ16_CO2Driver");
}

bool MHZ16_CO2Driver::takeMeasurement()
{
    // debug("taking measurement from driver template");
    // return true if measurement taken store in class value(s), false if not
    bool measurementTaken = false;
    if (true)
    {
        value = 42;
        measurementTaken = true;
    }
    addValueToBurstSummaryMean(VAR_TAG, value); // use the default option for computing the burst summary value
    return measurementTaken;
}

const char *MHZ16_CO2Driver::getRawDataString()
{
    // debug("configuring driver template dataString");
    // process data string for .csv
    sprintf(dataString, "%d,%0.3f", value, value * 31.83);
    return dataString;
}

const char *MHZ16_CO2Driver::getSummaryDataString()
{
    double burstSummaryMean = getBurstSummaryMean(VAR_TAG);
    sprintf(dataString, "%0.3f,%0.3f", burstSummaryMean, burstSummaryMean * 31.83);
    return dataString;
}

const char *MHZ16_CO2Driver::getBaseColumnHeaders()
{
    // for debug column headers defined in the .h
    // debug("getting driver template base column headers");
    return baseColumnHeaders;
}

void MHZ16_CO2Driver::initCalibration()
{
    // debug("init driver template sensor calibration");
}

void MHZ16_CO2Driver::calibrationStep(char *step, int arg_cnt, char **args)
{
    // for intermediary steps of calibration process
    // debug("driver template calibration steps");
}

void MHZ16_CO2Driver::addCalibrationParametersToJSON(cJSON *json)
{
    // follows structure of calibration parameters in .h
    // debug("add driver template calibration parameters to json");
    cJSON_AddNumberToObject(json, CALIBRATION_TIME_STRING, configuration.cal_timestamp);
}

void MHZ16_CO2Driver::setDriverDefaults()
{
    // debug("setting driver template driver defaults");
    // set default values for driver struct specific values
    configuration.cal_timestamp = 0;
}