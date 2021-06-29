#ifndef WATERBEAR_DATALOGGER
#define WATERBEAR_DATALOGGER

#include <Wire_slave.h> // Communicate with I2C/TWI devices
#include <SPI.h>
#include "SdFat.h"

#include "configuration.h"
#include "utilities/utilities.h"

#include "system/ble.h"
#include "system/clock.h"
#include "system/control.h"
#include "system/eeprom.h"
#include "system/filesystem.h"
#include "system/hardware.h"
#include "system/interrupts.h"
#include "system/low_power.h"
#include "system/monitor.h"
#include "system/switched_power.h"

#include <sensors/atlas_oem.h>
#include "sensors/sensor.h"



typedef struct datalogger_settings {
    short interval;  // 2 bytes
    char mode;       // i(interative), d(debug), l(logging), t(deploy on trigger)
    char padding[61];        // padded  
} datalogger_settings_type;

typedef enum mode { interactive, debug, logging, deploy_on_trigger } mode_type;

class Datalogger
{

public:
    // configuration
    short interval = 15;

    // sensors
    int sensorCount = 0;
    bool * dirtyConfigurations = NULL;      // configuration change tracking
    short * sensorTypes = NULL;
    void ** sensorConfigurations = NULL;
    SensorDriver ** drivers = NULL;

    Datalogger(datalogger_settings_type * settings);

    void setup();
    void loop();

    bool inMode(mode_type mode);
    void deploy();

private:
    // state
    char mode = 'i';
    bool powerCycle = true;
    bool interactiveModeLogging = false;

    void loadSensorConfigurations();
    bool shouldExitLoggingMode();
    bool measureSensorValues();
    bool writeMeasurementToLogFile();
    void processCLI();
    bool configurationIsDirty();
    void storeConfiguration();
    void stopLogging();
    void startLogging();
};

// Settings

extern short burstLength; // how many readings in a burst

extern short fieldCount; // number of fields to be logged to SDcard file

// State
extern WaterBear_FileSystem *filesystem;
extern unsigned char uuid[UUID_LENGTH];
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

void enableI2C1();

void enableI2C2();

void powerUpSwitchableComponents();

void powerDownSwitchableComponents();

void startSerial2();

void setupHardwarePins();

void blinkTest();

void initializeFilesystem();

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

void stopAndAwaitTrigger();

void handleControlCommand();

void monitorConfiguration();

void takeNewMeasurement();

void trackBurst(bool bursting);

void monitorValues();

void calibrateThermistor();

void monitorTemperature();

bool checkThermistorCalibration();

void clearThermistorCalibration();

float calculateTemperature();

void processControlFlag(char *flag);

#endif