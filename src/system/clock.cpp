#include "clock.h"
#include "configuration.h"
#include "monitor.h"
#include <RTClock.h>


DS3231 Clock;

void handleInterrupt(){
  // just do nothing
  // Serial2.println("RTC interrupt!!");
}

void setNextAlarmInternalRTC(short interval){
  short minutes = Clock.getMinute();
  Serial2.println("minutes");
  Serial2.println(Clock.getMinute(), DEC);
  Serial2.println(minutes);
  short seconds = Clock.getSecond();
  Serial2.println("seconds");
  Serial2.println(seconds);
  // // short nextMinutes = (minutes + interval - (minutes % interval)) % 60;
  short nextMinutes = (minutes + interval - (minutes % interval));
  // Serial2.println("next minutes");
  // Serial2.println(nextMinutes);
  short minutesDiff = nextMinutes - minutes;
  short minutesDiffSeconds = minutesDiff * 60;
  short secondsUntilWake = minutesDiffSeconds - seconds;
  // Serial2.println("seconds until wake");
  // Serial2.println(secondsUntilWake);

  RTClock * clock = new RTClock(RTCSEL_LSE);
  char message[100];
  // sprintf(message, "Got clock value (current): %i", clock->getTime());
  // Monitor::instance()->writeDebugMessage(message);
    
  clock->setTime(0);
  sprintf(message, "Got clock value (reset): %i", clock->getTime());
  Monitor::instance()->writeDebugMessage(message);

  clock->removeAlarm();
  // secondsUntilWake = 10;
  clock->setAlarmTime(secondsUntilWake);
  clock->createAlarm(handleInterrupt, secondsUntilWake);

  Serial2.println("set alarm time to wake");
  Serial2.println(secondsUntilWake);
}



void setNextAlarm(short interval)
{
  Clock.turnOffAlarm(1); // Clear the Control Register
  Clock.turnOffAlarm(2);
  Clock.checkIfAlarm(1); // Clear the Status Register
  Clock.checkIfAlarm(2);

  //
  // Alarm every 10 seconds for debugging
  //
  if(DEBUG_USING_SHORT_SLEEP == true)
  {
    Monitor::instance()->writeDebugMessage(F("Using short sleep"));
    int AlarmBits = ALRM2_ONCE_PER_MIN;
    AlarmBits <<= 4;
    AlarmBits |= ALRM1_MATCH_SEC;
    short seconds = Clock.getSecond();
    short debugSleepSeconds = 30;
    short nextSeconds = (seconds + debugSleepSeconds - (seconds % debugSleepSeconds)) % 60;
    char message[200];
    sprintf(message, "Next Alarm, with seconds: %i, now seconds: %i", nextSeconds, seconds);
    Monitor::instance()->writeDebugMessage(message);
    Clock.setA1Time(0b0, 0b0, 0b0, nextSeconds, AlarmBits, true, false, false);
  }
  //
  // Alarm every interval minutes for the real world
  //
  else
  {
    int AlarmBits = ALRM2_ONCE_PER_MIN;
    AlarmBits <<= 4;
    AlarmBits |= ALRM1_MATCH_MIN_SEC;
    short minutes = Clock.getMinute();
    short nextMinutes = (minutes + interval - (minutes % interval)) % 60;
    char message[200];
    sprintf(message, "Next Alarm, with minutes: %i", nextMinutes);
    Monitor::instance()->writeDebugMessage(message);
    Clock.setA1Time(0b0, 0b0, nextMinutes, 0b0, AlarmBits, true, false, false);
  }
  // set both alarms to :00 and :30 seconds, every minute
      // Format: .setA*Time(DoW|Date, Hour, Minute, Second, 0x0, DoW|Date, 12h|24h, am|pm)
      //                    |                                    |         |        |
      //                    |                                    |         |        +--> when set for 12h time, true for pm, false for am
      //                    |                                    |         +--> true if setting time based on 12 hour, false if based on 24 hour
      //                    |                                    +--> true if you're setting DoW, false for absolute date
      //                    +--> INTEGER representing day of the week, 1 to 7 (Monday to Sunday)
      //
  Clock.turnOnAlarm(1);
}

void dateTime(uint16_t* date, uint16_t* time)
{
  // Fetch time from DS3231 RTC
  bool century = false;
	bool h24Flag;
	bool pmFlag;
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(Clock.getYear() + 1900, Clock.getMonth(century) + 1, Clock.getDate()); // year is since 1900, months range 0-11

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(Clock.getHour(h24Flag, pmFlag), Clock.getMinute(), Clock.getSecond());
}

void clearAllAlarms()
{
  Clock.turnOffAlarm(1);
  Clock.turnOffAlarm(2);
  Clock.checkIfAlarm(1); // Clear the Status Register
  Clock.checkIfAlarm(2);
}


time_t timestamp()
{
  struct tm ts;
  bool century = false;
	bool h24Flag;
	bool pmFlag;

  ts.tm_year = Clock.getYear();
  ts.tm_mon = Clock.getMonth(century);
  ts.tm_mday = Clock.getDate();
  ts.tm_wday = Clock.getDoW();
  ts.tm_hour = Clock.getHour(h24Flag, pmFlag);
  ts.tm_min = Clock.getMinute();
  Serial2.println("timestamp mins");
  Serial2.println(ts.tm_min);
  ts.tm_sec = Clock.getSecond();
  ts.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
  return (mktime(&ts)); // turn tm struct into time_t value
}

void setTime(time_t toSet)
{
  struct tm ts;

  ts = *gmtime(&toSet); // Convert time_t epoch timestamp to tm as UTC time
  Clock.setClockMode(false); //true for 12h false for 24h
  Clock.setYear(ts.tm_year);
  Clock.setMonth(ts.tm_mon);
  Clock.setDate(ts.tm_mday);
  Clock.setDoW(ts.tm_wday);
  Clock.setHour(ts.tm_hour);
  Clock.setMinute(ts.tm_min);
  Clock.setSecond(ts.tm_sec);
}

void t_t2ts(time_t epochTS, uint32 currentMillis, char *humanTime)
{
  struct tm ts;
  //uint32 currentMillis = millis();
  char buf[21] ="0";

  ts = *gmtime(&epochTS); // convert unix to tm structure
  // Format time, "yyyy-mm-dd hh:mm:ss zzz" = "%Y/%m/%d %H:%M:%S %Z" (yyyy/mm/dd hh:mm:ss zzz) = 23
  //strftime(humanTime, 24, "%Y-%m-%d %H:%M:%S %Z", &ts); // converts a tm into custom date structure stored in string

  // Format time, "yyyy-mm-dd hh:mm:ss.sss" = "%Y/%m/%d %H:%M:%S" (yyyy/mm/dd hh:mm:ss.sss) = 23
  ts.tm_sec = ts.tm_sec + currentMillis/1000; // correct seconds
  strftime(buf, 20, "%Y-%m-%d %H:%M:%S", &ts);
  sprintf(humanTime, "%s.%i", buf, (int)currentMillis % 1000);
}

