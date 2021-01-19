#include "watchdog.h"
#include "datalogger.h"

bool waterBearWatchdog(short burstCount, short burstLength)
{
  if(burstCount == 0)
  {
    Monitor::instance()->writeDebugMessage(F("starting watchdog"));
    iwdg_init(IWDG_PRE_256, maxTimeout); // ~26.214 s
    //iwdg_init(IWDG_PRE_256, minTimeout); // ~6ms
    return(1);
  }
  else if(burstCount <= burstLength)
  {
    Monitor::instance()->writeDebugMessage(F("reloading watchdog"));
    iwdg_feed();
    return(1);
  }
  return(0);
}