#ifndef WATERBEAR_LOW_POWER
#define WATERBEAR_LOW_POWER

// Port everyting into C++ classes and don't be lazy
// class WB_LowPower {

// public:
  void enterStopMode();
  void enterSleepMode();
  void alwaysPowerOff(); // split into components and pins
  //void components off
  //void hardware pins off
  void disableHardwarePins();
  void restorePinDefaults(); // not necessary?
// }

#endif