#include "switched_power.h"
#include "hardware.h"
#include "monitor.h"

void setupSwitchedPower()
{
  // enable pin on switchable integrated 3v3 boost converter
  pinMode(SWITCHED_POWER_ENABLE, OUTPUT);
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
}

void enableSwitchedPower()
{
  debug(F("Enabling switched power"));
  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
}

void disableSwitchedPower()
{
  debug(F("Disabling switched power"));
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
}

void cycleSwitchablePower()
{
  debug(F("Cycle switched power"));
  disableSwitchedPower();
  delay(500);
  enableSwitchedPower();
  delay(500);
}