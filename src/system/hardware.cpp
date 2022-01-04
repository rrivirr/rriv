#include "hardware.h"

void GPIOpinOff(uint8 pin)
{
    digitalWrite(pin, LOW);
}

void GPIOpinOn(uint8 pin)
{
    digitalWrite(pin, HIGH);
#include <libmaple/libmaple.h>
#include <libmaple/pwr.h>
#include "configuration.h"
#include "system/monitor.h"


void startSerial2()
{
  // Start up Serial2
  // TODO: Need to do an if(Serial2) after an amount of time, just disable it
  Serial2.begin(SERIAL_BAUD);
  while (!Serial2)
  {
    delay(100);
  }
  Serial2.println("Hello world");
  notify(F("Begin Setup"));
  Serial2.println("ya");
}

void setupInternalRTC()
{
  // Set up the internal RTC
  RCC_BASE->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC_BASE->APB1ENR |= RCC_APB1ENR_BKPEN;
  PWR_BASE->CR |= PWR_CR_DBP; // Disable backup domain write protection, so we can write
}

void setupHardwarePins()
{
  debug(F("setting up hardware pins"));
  //pinMode(BLE_COMMAND_MODE_PIN, OUTPUT); // Command Mode pin for BLE
  
  pinMode(INTERRUPT_LINE_7_PIN, INPUT_PULLUP); // This the interrupt line 7
  pinMode(ANALOG_INPUT_1_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_2_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_3_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_4_PIN, INPUT_ANALOG);
  pinMode(ANALOG_INPUT_5_PIN, INPUT_ANALOG);
  pinMode(ONBOARD_LED_PIN, OUTPUT); // This is the onboard LED ? Turns out this is also the SPI1 clock.  niiiiice.

  // pinMode(PA4, INPUT_PULLDOWN); // mosfet for battery measurement - should be OUTPUT ??

  // redundant?
  //pinMode(PA2, OUTPUT); // USART2_TX/ADC12_IN2/TIM2_CH3
  //pinMode(PA3, INPUT); // USART2_RX/ADC12_IN3/TIM2_CH4

  pinMode(PC5, OUTPUT); // external ADC reset
  digitalWrite(PC5, HIGH);
}