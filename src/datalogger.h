#ifndef WATERBEAR_DATALOGGER
#define WATERBEAR_DATALOGGER

#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include <SPI.h>
#include "SdFat.h"

#include "configuration.h"
#include "utilities/utilities.h"

#include "system/ble.h"
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

#include <sensors/atlas_oem.h>
#include "sensors/sensor.h"

#define DEPLOYMENT_IDENTIFIER_LENGTH 26

typedef struct datalogger_settings {
    char deploymentIdentifier[DEPLOYMENT_IDENTIFIER_LENGTH];
    short interval;  // 2 bytes
    short burstLength; // 2 bytes
    short burstCount; // 2 bytes
    short startUpDelay; // 2 bytes
    short intraBurstDelay; // 2 bytes
    char mode;       // i(interative), d(debug), l(logging), t(deploy on trigger) 1 byte
    char padding[31];        // padding
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
    void deploy();

    void processCLI();

    // settings
    void setDeploymentIdentifier(char * deploymentIdentifier);
    void setInterval(int interval);
    void setBurstSize(int size);
    void setBurstNumber(int number);
    void setStartUpDelay(int delay);
    void setIntraBurstDelay(int delay);

    void getConfiguration(datalogger_settings_type * dataloggerSettings);

private:
    // state
    mode_type mode = interactive;
    bool powerCycle = true;
    bool interactiveModeLogging = false;
    time_t currentEpoch;
    uint32 offsetMillis;
    char loggingFolder[26];

    void loadSensorConfigurations();
    bool shouldExitLoggingMode();
    void measureSensorValues(bool performingBurst = true);
    bool writeMeasurementToLogFile();
    void writeDebugFieldsToLogFile();
    bool configurationIsDirty();
    void storeConfiguration();
    void stopLogging();
    void startLogging();
    bool shouldContinueBursting();

    // CLI
    CommandInterface * cli;
    void setUpCLI();

    // utility
    void writeStatusFieldsToLogFile();
    void initializeBurst();

    void storeMode(mode_type mode);
    void storeDataloggerConfiguration();

 
    // run loop
    void initializeFilesystem();
    void stopAndAwaitTrigger();
    void takeNewMeasurement();
    void captureInternalADCValues();



};

// Settings

extern short burstLength; // how many readings in a burst

extern short fieldCount; // number of fields to be logged to SDcard file


// State
extern WaterBear_FileSystem *filesystem;
extern unsigned char uuid[UUID_LENGTH];
extern char uuidString[25]; // 2 * UUID_LENGTH + 1
extern char **values;

extern unsigned long lastMillis;
extern uint32_t awakeTime;
extern uint32_t lastTime;
extern short burstCount;
extern bool configurationMode;
extern bool debugValuesMode;
extern bool clearModes;
extern bool tempCalMode;
// extern AtlasRGB rgbSensor;   
//extern bool thermistorCalibrated;


extern AD7091R * externalADC;



void enableI2C1();

void enableI2C2();

void powerUpSwitchableComponents();

void powerDownSwitchableComponents();

void startSerial2();

void setupHardwarePins();

void blinkTest();


void setupSensors();

void allocateMeasurementValuesMemory();

void prepareForTriggeredMeasurement();

void prepareForUserInteraction();

void setNotBursting();

void measureSensorValues();

bool shouldContinueBursting();

bool checkDebugLoop();

bool checkAwakeForUserInteraction(bool debugLoop);

bool checkTakeMeasurement(bool bursting, bool awakeForUserInteraction);


void handleControlCommand();

void monitorConfiguration();


void trackBurst(bool bursting);

void monitorValues();

void calibrateThermistor();

void monitorTemperature();

bool checkThermistorCalibration();

void clearThermistorCalibration();

float calculateTemperature();

void processControlFlag(char *flag);

#endif