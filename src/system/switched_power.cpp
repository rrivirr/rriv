#include "switched_power.h"
#include "hardware.h"
#include "monitor.h"

void setupSwitchedPower()
{
  pinMode(SWITCHED_POWER_ENABLE, OUTPUT); // enable pin on switchable boost converter
  pinMode(EXT_MOSFET_ENABLE, OUTPUT); // enable pin on switchable external MOSFET
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
}

void enableSwitchedPower()
{
  Monitor::instance()->writeDebugMessage(F("Enabling switched power"));
  digitalWrite(SWITCHED_POWER_ENABLE, HIGH);
  delay(500);
  digitalWrite(EXT_MOSFET_ENABLE, HIGH);
}

void disableSwitchedPower()
{
  Monitor::instance()->writeDebugMessage(F("Disabling switched power"));
  digitalWrite(EXT_MOSFET_ENABLE, LOW);
  delay(500);
  digitalWrite(SWITCHED_POWER_ENABLE, LOW);
}

void cycleSwitchablePower()
{
  Monitor::instance()->writeDebugMessage(F("Cycle switched power"));
  disableSwitchedPower();
  delay(500);
  enableSwitchedPower();
  delay(500);
}