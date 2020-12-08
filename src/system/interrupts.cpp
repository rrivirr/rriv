#include <Arduino.h>

bool awakenedByUser = false; // usused now, refactor into class property

void clearClockInterrupt()
{
  EXTI_BASE->PR = 0x00000080; // this clear the interrupt on exti line
  NVIC_BASE->ICPR[0] = 1 << NVIC_EXTI_9_5;
}

void disableClockInterrupt()
{
  NVIC_BASE->ICER[0] = 1 << NVIC_EXTI_9_5;
}

void enableClockInterrupt()
{
  NVIC_BASE->ISER[0] = 1 << NVIC_EXTI_9_5;
}

void enableUserInterrupt()
{
  NVIC_BASE->ISER[1] = 1 << (NVIC_EXTI_15_10 - 32);
}

void clearUserInterrupt()
{
  EXTI_BASE->PR = 0x00000400; // this clear the interrupt on exti line
  NVIC_BASE->ICPR[1] = 1 << (NVIC_EXTI_15_10 - 32);
}

void disableUserInterrupt()
{
  NVIC_BASE->ICER[1] = 1 << (NVIC_EXTI_15_10 - 32); // it's on EXTI 10
}

// Interrupt service routing for EXTI line
// Just clears out the interrupt, control will return to loop()
void handleClockInterrupt()
{
  disableClockInterrupt();
  clearClockInterrupt();
}

void setupWakeInterrupts()
{
  //
  // Set up interrupts
  //
  awakenedByUser = false;

  exti_attach_interrupt(EXTI7, EXTI_PC, handleClockInterrupt, EXTI_FALLING);

  // PB10 interrupt disabled, PB10 is I2C2, use a different user interrupt
  //exti_attach_interrupt(EXTI10, EXTI_PB, userTriggeredInterrupt, EXTI_RISING);
}

void userTriggeredInterrupt()
{

  disableUserInterrupt();
  clearUserInterrupt();
  //Logger::instance()->writeDebugMessage("USER TRIGGERED INTERRUPT");
  //enableUserInterrupt();
  awakenedByUser = true;
}

void clearAllInterrupts()
{
  // only enable the timer and user interrupts
  NVIC_BASE->ICER[0] = NVIC_BASE->ISER[0];
  NVIC_BASE->ICER[1] = NVIC_BASE->ISER[1];
  NVIC_BASE->ICER[2] = NVIC_BASE->ISER[2];
}

void clearAllPendingInterrupts()
{
  // clear any pending interrupts
  NVIC_BASE->ICPR[0] = NVIC_BASE->ISPR[0];
  NVIC_BASE->ICPR[1] = NVIC_BASE->ISPR[1];
  NVIC_BASE->ICPR[2] = NVIC_BASE->ISPR[2];
}

void storeAllInterrupts(int &iser1, int &iser2, int &iser3)
{
  iser1 = NVIC_BASE->ISER[0];
  iser2 = NVIC_BASE->ISER[1];
  iser3 = NVIC_BASE->ISER[2];
}

void reenableAllInterrupts(int iser1, int iser2, int iser3)
{
  NVIC_BASE->ISER[0] = iser1;
  NVIC_BASE->ISER[1] = iser2;
  NVIC_BASE->ISER[2] = iser3;
}