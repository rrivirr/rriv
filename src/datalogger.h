#ifndef WATERBEAR_DATALOGGER
#define WATERBEAR_DATALOGGER

#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include <SPI.h>
#include "SdFat.h"

#include "configuration.h"
#include "utilities/utilities.h"

// #include "system/ble.h"
#include "system/clock.h"
#include "system/command.h"
#include "system/eeprom.h"
#include "system/filesystem.h"
#include "system/hardware.h"
#include "system/interrupts.h"
#include "system/low_power.h"
#include "system/monitor.h"
#include "system/switched_power.h"
#include "system/adc.h"
#include "system/write_cache.h"

#include "sensors/sensor.h"

#define DEPLOYMENT_IDENTIFIER_LENGTH 16

typedef struct datalogger_settings { // 64 bytes
    byte deploymentIdentifier[16];
    unsigned short interval;  // 2 bytes
    unsigned short reserved; // 2 bytes, unused
    unsigned short burstNumber; // 2 bytes
    unsigned short startUpDelay; // 2 bytes
    unsigned short interBurstDelay; // 2 bytes
    char mode;       // i(interative), d(debug), l(logging), t(deploy on trigger) 1 byte
    char siteName[8];
    unsigned long deploymentTimestamp;
    byte externalADCEnabled : 1;
    byte debug_values : 1;
    byte withold_incomplete_readings : 1; // only publish complete readings, default to withold.
    byte reserved2 : 5;
    char unused[20];        // padding
} datalogger_settings_type;

typedef enum mode { interactive, debugging, logging, deploy_on_trigger } mode_type;

// Forward declaration of class
class CommandInterface;

class Datalogger
{

public:

    // sensors
    int sensorCount = 0;
    bool * dirtyConfigurations = NULL;      // configuration change tracking
    short * sensorTypes = NULL;
    void ** sensorConfigurations = NULL;
    SensorDriver ** drivers = NULL;
    datalogger_settings_type settings;

    static void readConfiguration(datalogger_settings_type * settings);
    Datalogger(datalogger_settings_type * settings);

    void setup();
    void loop();

    void changeMode(mode_type mode);
    bool inMode(mode_type mode);
    void storeMode(mode_type mode);
    void deploy();

    void processCLI();

    // settings
    void setSiteName(char * siteName);
    void setDeploymentIdentifier();
    void setDeploymentTimestamp(int timestamp);

    void setInterval(int interval);
    void setBurstSize(int size);
    void setBurstNumber(int number);
    void setStartUpDelay(int delay);
    void setIntraBurstDelay(int delay);

    void getConfiguration(datalogger_settings_type * dataloggerSettings);
    cJSON ** getSensorConfigurations();

    void setSensorConfiguration(char * type, cJSON * json);
    void clearSlot(unsigned short slot);
    void calibrate(unsigned short slot, char * subcommand, int arg_cnt, char ** args);
    void setExternalADCEnabled(bool enabled);

    void setUserNote(char * note);
    void setUserValue(int value);

    void toggleTraceValues();
    void stopLogging();
    void startLogging();

private:
    // modules
    WaterBear_FileSystem *fileSystem;
    WriteCache * fileSystemWriteCache = NULL;

    // state
    char uuidString[25]; // 2 * UUID_LENGTH + 1
    mode_type mode = interactive;
    bool powerCycle = true;
    bool interactiveModeLogging = false;
    time_t currentEpoch;
    uint32 offsetMillis;
    char loggingFolder[26];
    int completedBursts;
    int awakeTime;

    // user
    char userNote[100] = "\0";
    int userValue = INT_MIN;
    int lastInteractiveLogTime = 0;

    void loadSensorConfigurations();
    bool shouldExitLoggingMode();
    void measureSensorValues(bool performingBurst = true);
    bool writeMeasurementToLogFile();
    void writeDebugFieldsToLogFile();
    bool configurationIsDirty();
    void storeConfiguration();
    void initializeBurst();
    bool shouldContinueBursting();

    // CLI
    CommandInterface * cli;
    
    void setUpCLI();

    // utility
    void writeStatusFieldsToLogFile();
    void initializeMeasurementCycle();

    void storeDataloggerConfiguration();
    void storeSensorConfiguration(generic_config * configuration);

 
    // run loop
    void initializeFilesystem();
    void stopAndAwaitTrigger();
    void writeStatusFields();
    void prepareForUserInteraction();
    void powerUpSwitchableComponents();
    void powerDownSwitchableComponents();

    // utility
    SensorDriver * getDriver(unsigned short slot);

    // debugging
    void debugValues(char * buffer);


};



#endif