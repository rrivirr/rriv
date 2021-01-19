#ifndef WATERBEAR_WATCHDOG
#define WATERBEAR_WATCHDOG

#include <libmaple/iwdg.h>

class WaterBear_Watchdog
{
  private:
    #define minTimeout 0x000
    #define maxTimeout 0xFFF

  public:
    //static bool waterBearWatchdog(short burstCount, short burstLength);
};

bool waterBearWatchdog(short burstCount, short burstLength);

#endif