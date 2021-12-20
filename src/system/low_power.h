#ifndef WATERBEAR_LOW_POWER
#define WATERBEAR_LOW_POWER

#include "datalogger.h"

// Port everyting into C++ classes and don't be lazy
// class WB_LowPower {

// public:
  void enterStopMode();
  void enterSleepMode();
  void componentsAlwaysOff(); // turn off unused components during setup
  void hardwarePinsAlwaysOff(); // disable unused hardware pins during setup
  void componentsStopMode(); // for stop/sleep mode
  void hardwarePinsStopMode(); // for stop/sleep mode
  void restorePinDefaults(); // not necessary?
  void componentsBurstMode(); // wake up components
// }

void disableSerialLog();
void enableSerialLog();

#endif