#ifndef WATERBEAR_INTERRUPTS
#define WATERBEAR_INTERRUPTS

void clearManualWakeInterrupt();
void disableManualWakeInterrupt();
void enableManualWakeInterrupt();
void handleManualWakeInterrupt();
void setupManualWakeInterrupts();


void enableRTCAlarmInterrupt();
void clearRTCAlarmInterrupt();
void disableRTCAlarmInterrupt();

void clearAllInterrupts();
void clearAllPendingInterrupts();
void storeAllInterrupts(int& iser1, int& iser2, int& iser3);
void reenableAllInterrupts(int iser1, int iser2, int iser3);

extern bool awakenedByUser;

#endif
