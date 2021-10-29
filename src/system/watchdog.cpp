#include "watchdog.h"
#include <Arduino.h>
#include <libmaple/libmaple.h>
#include "monitor.h"

void timerFired(){
  timer_pause(TIMER1);
  Serial2.println("FT!");
  // Serial2.flush(); // causes crash
  delay(5000);
  nvic_sys_reset();
}

void startCustomWatchDog()
{
  // Serial2.println("skipped custom watchdog!");
  // return;

  Monitor::instance()->writeDebugMessage(F("Setup custom watchdog"));
 
  timer_init(TIMER1);
  timer_set_prescaler(TIMER1, 65535);  //  64000000 / 65536 = 976.5 Hz
  // int watchdogValue = 5 * 976.5 ; // 976.5 Hz * ( 5 * 976.5 counts) = 5s
  int watchdogValue = WATCHDOG_TIMEOUT_SECONDS * 976.5 ; // 976.5 Hz * ( 5 * 976.5 counts) = 5s
  timer_set_compare(TIMER1, TIMER_CH1,  watchdogValue);
  // timer_cc_enable(TIMER1, TIMER_CH1); // not necessary?

  timer_set_reload(TIMER1, watchdogValue);
  timer_generate_update(TIMER1);

  timer_resume(TIMER1);
  Monitor::instance()->writeDebugMessage(F("Resumed timer"));

  timer_attach_interrupt(TIMER1, TIMER_CC1_INTERRUPT, timerFired);
  Monitor::instance()->writeDebugMessage(F("Attached interrupt!"));
}

void disableCustomWatchDog()
{
  // Serial2.println("skipped disable custom watchdog!");
  // return;
  timer_detach_interrupt(TIMER1, TIMER_CC1_INTERRUPT);
  // timer_pause(TIMER1);
  // timer_cc_disable(TIMER1, TIMER_CH1);
  // timer_generate_update(TIMER1);
  // timer_disable(TIMER1);
  
  //timer_disable_irq ??
  // nvic_irq_disable()
  
  rcc_reset_dev(TIMER1->clk_id);
  rcc_clk_disable(TIMER1->clk_id);


  // timer_init(TIMER1); // not necessary
}

void printWatchDogStatus(){
  char message[100];
  int timerCount = timer_get_count(TIMER1);
  sprintf(message, reinterpret_cast<const char *> F("Timer Count: %d\0"), timerCount);
  Monitor::instance()->writeDebugMessage(timerCount);
}
