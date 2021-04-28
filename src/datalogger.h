#include <Wire.h> // Communicate with I2C/TWI devices
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

#include "sensors/atlas_oem.h"

// Settings

extern short interval;     // minutes between loggings when not in short sleep
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
//extern bool thermistorCalibrated;

void enableI2C2();

void powerUpSwitchableComponents();

void powerDownSwitchableComponents();

void startSerial2();

void setupHardwarePins();

void blinkTest();

void initializeFilesystem();

void allocateMeasurementValuesMemory();

void prepareForTriggeredMeasurement();

void prepareForUserInteraction();

void setNotBursting();

void measureSensorValues();

bool checkBursting();

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