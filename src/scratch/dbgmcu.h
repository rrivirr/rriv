#ifndef WATERBEAR_DBGMCU
#define WATERBEAR_DBGMCU

#include <libmaple/libmaple.h>

// NOTE: from the register, this is not present in stm32f103rb

/* Debug MCU register map */
typedef struct dbgmcu_reg_map {
    __IO uint32 CR;      /*< Control register */
} dbgmcu_reg_map;

/* Debug MCU register map base pointer */
#define DBGMCU_BASE                        ((struct dbgmcu_reg_map*)0xE0042004)

/*
#define DBGMCU_CR_IWDG_STOP_BIT              8
#define DBGMCU_CR_IWDG_STOP                (1U << DBGMCU_CR_IWDG_STOP_BIT) // The watchdog counter clock is stopped when the core is halted
*/

/*
 * Register bit definitions
 */

/* Control Register */

#define DBGMCU_CR_TIM11_STOP_BIT             30
#define DBGMCU_CR_TIM10_STOP_BIT             29
#define DBGMCU_CR_TIM09_STOP_BIT             28
#define DBGMCU_CR_TIM14_STOP_BIT             27
#define DBGMCU_CR_TIM13_STOP_BIT             26
#define DBGMCU_CR_TIM12_STOP_BIT             25

#define DBGMCU_CR_CAN2_STOP_BIT              21
#define DBGMCU_CR_TIM7_STOP_BIT              20
#define DBGMCU_CR_TIM6_STOP_BIT              19
#define DBGMCU_CR_TIM5_STOP_BIT              18
#define DBGMCU_CR_TIM8_STOP_BIT              17
#define DBGMCU_CR_I2C2_SMBUS_TIMEOUT_BIT     16
#define DBGMCU_CR_I2C1_SMBUS_TIMEOUT_BIT     15
#define DBGMCU_CR_CAN1_STOP_BIT              14
#define DBGMCU_CR_TIM4_STOP_BIT              13
#define DBGMCU_CR_TIM3_STOP_BIT              12
#define DBGMCU_CR_TIM2_STOP_BIT              11
#define DBGMCU_CR_TIM1_STOP_BIT              10
#define DBGMCU_CR_WWDG_STOP_BIT              9
#define DBGMCU_CR_IWDG_STOP_BIT              8
#define DBGMCU_CR_TRACE_MODE_BIT1            7
#define DBGMCU_CR_TRACE_MODE_BIT2            6
#define DBGMCU_CR_TRACE_IOEN_BIT             5

#define DBGMCU_CR_STANDBY_BIT                2
#define DBGMCU_CR_STOP_BIT                   1
#define DBGMCU_CR_SLEEP_BIT                  0

#define DBGMCU_CR_WWDG_STOP                  (1U << DBGMCU_CR_IWDG_STOP_BIT)
#define DBGMCU_CR_IWDG_STOP                  (1U << DBGMCU_CR_IWDG_STOP_BIT) // The watchdog counter clock is stopped when the core is halted

#define DBGMCU_CR_STANDBY                    (1U << DBGMCU_CR_STANDBY_BIT)
#define DBGMCU_CR_STOP                       (1U << DBGMCU_CR_STOP_BIT)
#define DBGMCU_CR_SLEEP                      (1U << DBGMCU_CR_SLEEP_BIT)

#endif