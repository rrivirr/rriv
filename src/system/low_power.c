#include "low_power.h"

#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <libmaple/rcc.h>


void enterStopMode(){
    // Clear PDDS and LPDS bits
    PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

    // Set PDDS and LPDS bits for standby mode, and set Clear WUF flag (required per datasheet):
    PWR_BASE->CR |= PWR_CR_CWUF;
    PWR_BASE->CR |= PWR_CR_PDDS; // Enter stop/standby mode when cpu goes into deep sleep

    // PWR_BASE->CR |=  PWR_CSR_EWUP;   // Enable wakeup pin bit.  This is for wake from the WKUP pin specifically

    //  Unset Power down deepsleep bit.
    PWR_BASE->CR &= ~PWR_CR_PDDS; // Also have to unset this to get into STOP mode
    // set Low-power deepsleep.
    PWR_BASE->CR |= PWR_CR_LPDS; // Puts voltage regulator in low power mode.  This seems to cause problems

    SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
    //SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;

    SCB_BASE->SCR &= ~SCB_SCR_SLEEPONEXIT;

    rcc_switch_sysclk(RCC_CLKSRC_HSI);
    rcc_turn_off_clk(RCC_CLK_PLL);

    __asm volatile( "dsb" );
    systick_disable();
    __asm volatile( "wfi" );
    systick_enable();
    __asm volatile( "isb" );

    rcc_turn_on_clk(RCC_CLK_PLL);
    while(!rcc_is_clk_ready(RCC_CLK_PLL));

    // Finally, switch to the now-ready PLL as the main clock source.
    rcc_switch_sysclk(RCC_CLKSRC_PLL);

}


void enterSleepMode(){
    __asm volatile( "dsb" );
    systick_disable();
    __asm volatile( "wfi" );
    systick_enable();
    //__asm volatile( "isb" );
}