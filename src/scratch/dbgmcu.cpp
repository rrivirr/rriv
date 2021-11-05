
#include "Arduino.h"
#include "dbgmcu.h" 

void print_debug_status(){
  Serial2.print("Setup:DBGMCU_BASE->CR=");
  Serial2.println(DBGMCU_BASE->CR);
  Serial2.flush();

  if(DBGMCU_BASE->CR != 0){
      Serial2.println("**** WARNING *****");
      Serial2.println("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      Serial2.println("**** WARNING *****");
      Serial2.println("**** WARNING *****");
      Serial2.println("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      Serial2.println("**** WARNING *****");
      Serial2.println("**** WARNING *****");
      Serial2.println("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      Serial2.println("**** WARNING *****");
      Serial2.println("**** WARNING *****");
      Serial2.println("**** LOW POWER MODE REQUIRES POWER CYCLE *****");
      Serial2.println("**** WARNING *****");
      // delay(10000);
  } else {
    Serial2.println("DEBUG MODE OFF - OK!");
  }
}