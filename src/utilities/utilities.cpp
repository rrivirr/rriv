#include "utilities.h"
#include "system/monitor.h"
#include "system/clock.h"

// For F103RM convienience in this file
#define Serial Serial2



void printInterruptStatus(HardwareSerial &serial)
{
  serial.println("NVIC->ISER:");
  serial.println(NVIC_BASE->ISER[0], BIN);
  serial.println(NVIC_BASE->ISER[1], BIN);
  serial.println(NVIC_BASE->ISER[2], BIN);
  serial.println(EXTI_BASE->PR, BIN);
  serial.flush();
}

void printDateTime(HardwareSerial &serial, DateTime now)
{
  serial.println(now.unixtime());

  serial.print(now.year(), DEC);
  serial.print('/');
  serial.print(now.month(), DEC);
  serial.print('/');
  serial.print(now.day(), DEC);
  // Serial.print(" (");
  // Serial.print(now.dayOfTheWeek());
  // Serial.print(") ");
  serial.print("  ");
  serial.print(now.hour(), DEC);
  serial.print(':');
  serial.print(now.minute(), DEC);
  serial.print(':');
  serial.print(now.second(), DEC);
  serial.println();
  serial.flush();
}

void blink(int times, int duration)
{
  pinMode(PA5, OUTPUT);
  for (int i = times; i > 0; i--)
  {
    digitalWrite(PA5, HIGH);
    delay(duration);
    digitalWrite(PA5, LOW);
    delay(duration);
  }
}

void printDS3231Time()
{
  char testTime[11]; // timestamp responses
  Serial2.print(F("TS:"));
  sprintf(testTime, "%lld", timestamp()); // convert time_t value into string
  Serial2.println(testTime);
  Serial2.flush();
}

void printNVICStatus(){
  char  message[100];
  sprintf(message, "1: NVIC_BASE->ISPR\n%" PRIu32"\n%" PRIu32"\n%" PRIu32, NVIC_BASE->ISPR[0], NVIC_BASE->ISPR[1], NVIC_BASE->ISPR[2]);
  Monitor::instance()->writeSerialMessage(F(message));

}