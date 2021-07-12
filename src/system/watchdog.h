#ifndef WATERBEAR_WATCHDOG
#define WATERBEAR_WATCHDOG

#define WATCHDOG_TIMEOUT_SECONDS 20

void timerFired();

void startCustomWatchDog();

void disableCustomWatchDog();

void printWatchDogStatus();

#endif