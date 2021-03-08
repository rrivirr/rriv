/*
#include "watchdog.h"
#include "datalogger.h"

bool waterBearIWDG(short burstCount, short burstLength)
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
*/

/*
  if (bursting)
  {
    waterBearWatchdog(burstCount, burstLength);
  }
*/

/*
 //DBGMCU_BASE->CR = 0;
  DBGMCU_BASE->CR |= DBGMCU_CR_WWDG_STOP;
  DBGMCU_BASE->CR |= DBGMCU_CR_IWDG_STOP;
  //DBGMCU_BASE->CR &= ~DBGMCU_CR_STANDBY;
  //DBGMCU_BASE->CR &= ~DBGMCU_CR_STOP;
  //DBGMCU_BASE->CR &= ~DBGMCU_CR_SLEEP;
  Serial2.print("Setup:DBGMCU_BASE->CR=");
  Serial2.println(DBGMCU_BASE->CR);
  Serial2.flush();
  Serial2.print("Setup:MPU_BASE->TYPE=");
  Serial2.println(MPU_BASE->TYPE);
  Serial2.flush();
  Serial2.print("Setup:MPU_BASE->CTRL=");
  Serial2.println(MPU_BASE->CTRL);
  Serial2.flush();
  Serial2.print("Setup:MPU_BASE->RNR=");
  Serial2.println(MPU_BASE->RNR);
  Serial2.flush();
  Serial2.print("Setup:MPU_BASE->RBAR=");
  Serial2.println(MPU_BASE->RBAR);
  Serial2.flush();
  Serial2.print("Setup:MPU_BASE->RASR=");
  Serial2.println(MPU_BASE->RASR);
  Serial2.flush();
*/