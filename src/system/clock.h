#ifndef WATERBEAR_CLOCK
#define WATERBEAR_CLOCK

#include "DS3231.h"

// The DS3231 RTC chip
extern DS3231 Clock;

#define ALRM1_MATCH_EVERY_SEC  0b1111  // once a second
#define ALRM1_MATCH_SEC        0b1110  // when seconds match
#define ALRM1_MATCH_MIN_SEC    0b1100  // when minutes and seconds match
#define ALRM1_MATCH_HR_MIN_SEC 0b1000  // when hours, minutes, and seconds match
//byte ALRM1_SET = ALRM1_MATCH_SEC;

#define ALRM2_ONCE_PER_MIN     0b111   // once per minute (00 seconds of every minute)
#define ALRM2_MATCH_MIN        0b110   // when minutes match
#define ALRM2_MATCH_HR_MIN     0b100   // when hours and minutes match
//byte ALRM2_SET = ALRM2_ONCE_PER_MIN;


void setNextAlarmInternalRTC(short interval);
void setNextAlarm(short interval);
void dateTime(uint16_t* date, uint16_t* time);
void clearAllAlarms();
time_t timestamp();
void setTime(time_t toSet);
void t_t2ts(time_t epochTS, uint32 currentMillis, char *humanTime);

#endif