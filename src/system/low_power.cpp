#include "low_power.h"

#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <libmaple/rcc.h>
#include <libmaple/systick.h>
#include <libmaple/usb.h>

#include <Arduino.h>
#include <libmaple/usart.h>
#include <libmaple/spi.h>
#include <libmaple/i2c.h>
#include <libmaple/timer.h>
#include <libmaple/adc.h>
#include <libmaple/dac.h>

void enterStopMode()
{
  // Clear PDDS and LPDS bits
  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

  // Set PDDS and LPDS bits for standby mode, and set Clear WUF flag (required per datasheet):
  PWR_BASE->CR |= PWR_CR_CWUF;
  //??PWR_BASE->CR |= PWR_CR_PDDS; // Enter stop/standby mode when cpu goes into deep sleep

  // PWR_BASE->CR |=  PWR_CSR_EWUP;   // Enable wakeup pin bit.  This is for wake from the WKUP pin specifically

  //Unset Power down deepsleep bit.
  PWR_BASE->CR &= ~PWR_CR_PDDS; // Also have to unset this to get into STOP mode
  // set Low-power deepsleep.
  PWR_BASE->CR |= PWR_CR_LPDS; // Puts voltage regulator in low power mode.  This seems to cause problems

  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
  //SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;

  SCB_BASE->SCR &= ~SCB_SCR_SLEEPONEXIT;

  usart_disable(Serial2.c_dev()); // turn back on when awakened? or not needed when deployed
  i2c_disable(I2C1);  // other chips on waterbear board
  i2c_disable(I2C2);  // Atlas EC chip, external chips
                      // this is a place where using a timer as a watchdog may be important

  //adc_disable(ADC1) // need to turn back on and recalibrate? when waking

  rcc_switch_sysclk(RCC_CLKSRC_HSI);
  rcc_turn_off_clk(RCC_CLK_PLL);
  rcc_turn_off_clk(RCC_CLK_LSI);

  __asm__ volatile( "dsb" );
  systick_disable();
  __asm__ volatile( "wfi" );
  systick_enable();
  __asm__ volatile( "isb" );

  rcc_turn_on_clk(RCC_CLK_PLL);
  while(!rcc_is_clk_ready(RCC_CLK_PLL));

  //Finally, switch to the now-ready PLL as the main clock source.
  rcc_switch_sysclk(RCC_CLKSRC_PLL);
}

void enterSleepMode()
{
  __asm__ volatile( "dsb" );
  systick_disable();
  __asm__ volatile( "wfi" );
  systick_enable();
  //__asm volatile( "isb" );
}

void alwaysPowerOff()
{
  usb_power_off();

  usart_disable(Serial1.c_dev());
  usart_disable(Serial3.c_dev());

  timer_disable(&timer1);
  timer_disable(&timer2);
  timer_disable(&timer3);
  timer_disable(&timer4);

  // this might be redundant
  ADC1->regs->CR2 &= ~ADC_CR2_TSVREFE;  // turning off the temperature sensor that's in the ADC
  ADC2->regs->CR2 &= ~ADC_CR2_TSVREFE;

  // adc_disable(ADC1); // turn off when asleep, potentially recalibrate when waking
  // adc_disable(ADC2); // should always be off
  adc_disable_all();

  // digital to analog converter, could alwayd be disabled
  DAC_BASE->CR &= ~DAC_CR_EN1;  // don't think this made a difference
  DAC_BASE->CR &= ~DAC_CR_EN2;

  rcc_clk_disable(RCC_ADC1);
  rcc_clk_disable(RCC_ADC2);
  rcc_clk_disable( RCC_ADC3);
  rcc_clk_disable( RCC_AFIO);
  rcc_clk_disable( RCC_BKP);
  rcc_clk_disable( RCC_CRC);
  rcc_clk_disable( RCC_DAC);
  rcc_clk_disable( RCC_DMA1);
  rcc_clk_disable( RCC_DMA2);
  rcc_clk_disable( RCC_FLITF);
  rcc_clk_disable( RCC_FSMC);
  rcc_clk_disable( RCC_GPIOA);
  rcc_clk_disable( RCC_GPIOB);
  rcc_clk_disable( RCC_GPIOC);
  rcc_clk_disable( RCC_GPIOD);
  rcc_clk_disable( RCC_GPIOE);
  rcc_clk_disable( RCC_GPIOF);
  rcc_clk_disable( RCC_GPIOG);
  rcc_clk_disable( RCC_I2C1);
  rcc_clk_disable( RCC_I2C2);
  rcc_clk_disable( RCC_PWR);
  rcc_clk_disable( RCC_SDIO);
  rcc_clk_disable( RCC_SPI1);
  rcc_clk_disable( RCC_SPI2);
  rcc_clk_disable( RCC_SPI3);
  rcc_clk_disable( RCC_SRAM);
  rcc_clk_disable( RCC_TIMER1);
  rcc_clk_disable( RCC_TIMER2);
  rcc_clk_disable( RCC_TIMER3);
  rcc_clk_disable( RCC_TIMER4);
  rcc_clk_disable( RCC_TIMER5);
  rcc_clk_disable( RCC_TIMER6);
  rcc_clk_disable( RCC_TIMER7);
  rcc_clk_disable( RCC_TIMER8);
  rcc_clk_disable( RCC_TIMER9);
  rcc_clk_disable( RCC_TIMER10);
  rcc_clk_disable( RCC_TIMER11);
  rcc_clk_disable( RCC_TIMER12);
  rcc_clk_disable( RCC_TIMER13);
  rcc_clk_disable( RCC_TIMER14);
  rcc_clk_disable( RCC_USART1);
  rcc_clk_disable( RCC_USART2);
  rcc_clk_disable( RCC_USART3);
  rcc_clk_disable( RCC_UART4);
  rcc_clk_disable( RCC_UART5);
  rcc_clk_disable( RCC_USB);

// any pins changed need to be set back to the right stuff when we wake
// need to find out what pinModes are default or how to reset them
/*
  pinMode(PA1, INPUT);  // INPUT_ANALOG was suggested
  pinMode(PA2, INPUT);
  pinMode(PA3, INPUT);
  pinMode(PA4, INPUT);
  pinMode(PA5, INPUT);
  pinMode(PA6, INPUT);
  pinMode(PA7, INPUT);
  pinMode(PA8, INPUT);
  pinMode(PA9, INPUT);
  pinMode(PA10, INPUT);
  pinMode(PA11, INPUT);
  pinMode(PA12, INPUT);
  pinMode(PA13, INPUT);
  pinMode(PA14, INPUT);
  pinMode(PA15, INPUT);
  pinMode(PB1, INPUT);
  pinMode(PB2, INPUT);
  pinMode(PB3, INPUT);
  pinMode(PB4, INPUT);
  pinMode(PB5, INPUT);
  pinMode(PB6, INPUT);
  pinMode(PB7, INPUT);
  pinMode(PB8, INPUT);
  pinMode(PB9, INPUT);
  pinMode(PB10, INPUT);
  pinMode(PB11, INPUT);
  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);
  pinMode(PB14, INPUT);
  pinMode(PB15, INPUT);
  pinMode(PC1, INPUT);
  pinMode(PC2, INPUT);
  pinMode(PC3, INPUT);
  pinMode(PC4, INPUT);
  pinMode(PC5, INPUT);
  pinMode(PC6, INPUT);
  pinMode(PC7, INPUT);
  pinMode(PC8, INPUT);
  pinMode(PC9, INPUT);
  pinMode(PC10, INPUT);
  pinMode(PC11, INPUT);
  pinMode(PC12, INPUT);
  pinMode(PC13, INPUT);
  pinMode(PC14, INPUT);
  pinMode(PC15, INPUT);
*/

}