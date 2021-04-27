#ifndef WATERBEAR_WATCHDOG
#define WATERBEAR_WATCHDOG

#include <libmaple/iwdg.h>
#include <libmaple/libmaple.h>

class WaterBear_IWDG
{
  private:
    #define minTimeout 0x000
    #define maxTimeout 0xFFF

  public:
    //static bool waterBearWatchdog(short burstCount, short burstLength);
};

bool waterBearIWDG(short burstCount, short burstLength);

class WaterBear_WWDG
{
  private:
    #define minTimeout
    #define maxTimeout

  public:

};

// NOTE: not able to edit due to the lack of DBGMCU
/* Window Watchdog register map  pg 504 of CD00171190*/
typedef struct wwdg_reg_map {
    __IO uint64 CR;        /* Control Register */
    __IO uint64 CFR;       /* Configuration Register */
    __IO uint64 SR;        /* Status Register */
} wwdg_reg_map;

/* Window Watchdog register map base pointer */
#define WWDG_BASE                        ((struct wwdg_reg_map*)0x0000007F)

/*
 * Register bit definitions
 */

/* Control Register */
/* Bits 31:8 Reserved */
#define WWDG_CR_WDGA_BIT             7 /* Activation bit, set by software and only cleared after reset */
#define WWDG_CR_T6_BIT               6
/* Bits 6:0 7-bit downcounter */

#define WWDG_CR_WDGA                     (1U << WWDG_CR_WDGA_BIT)
#define WWDG_CR_T6                       (1U << WWDG_CR_T6_BIT)

/* Configuration Register */
/* Bits 31:10 Reserved */
#define WWDG_CFR_EWI_BIT            9 /* Early wakeup interrupt - when set, interrupt when counter reached 0x40, only cleared after reset */
#define WWDG_WDGTB_BIT              8 /* Watchdog Timer base prescaler*/
/* Bits 8:7 WDGTB: Timer base */

/* Configuration register WDGTB*/

#define WWDG_CFR_DIV_1                   0x0
#define WWDG_CFR_DIV_2                   0x1
#define WWDG_CFR_DIV_4                   0x2
#define WWDG_CFR_DIV_8                   0x3

/**
 * @brief Window watchdog prescalers.
 *
 * These divide the ~36 kHz fPCLK1 (APB1).
 */
typedef enum wwdg_prescaler {
    WWDG_PRE_1 = WWDG_CFR_DIV_1,     /**< Divide by 1 */
    WWDG_PRE_2 = WWDG_CFR_DIV_2,     /**< Divide by 2 */
    WWDG_PRE_4 = WWDG_CFR_DIV_4,    /**< Divide by 4 */
    WWDG_PRE_8 = WWDG_CFR_DIV_8,    /**< Divide by 8 */
} wwdg_prescaler;

#endif
