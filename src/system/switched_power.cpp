#include "switched_power.h"
#include "hardware.h"
#include "monitor.h"

void setupSwitchedPower()
{
  pinMode(SWITCHED_POWER_ENABLE, OUTPUT); // enable pin on switchable boost converter
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