
#include "Arduino.h"
#include "system/monitor.h"
#include "dbgmcu.h" 

void printDebugStatus()
{
  notify("Setup:DBGMCU_BASE->CR=");
  notify(DBGMCU_BASE->CR);

  if(DBGMCU_BASE->CR != 0){
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      notify("**** WARNING *****");
      notify("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      notify("**** WARNING *****");
      delay(2000);
  } else {
    notify("DEBUG MODE OFF - OK!");
  }
}