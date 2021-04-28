#ifndef WATERBEAR_M3MPU
#define WATERBEAR_M3MPU

#include <libmaple/libmaple.h>

//NOTE: reading the TYPE register indicates that the MPU is not present on stm32f103rb
/* Memory Protection Unit register map */
typedef struct mpu_reg_map {
    __IO uint64 TYPE;       /* read-only register used to detect the MPU presence */
    __IO uint64 CTRL;       /* control register */
    __IO uint64 RNR;        /* region number register, used to determine which region operations are applied to */
    __IO uint64 RBAR;       /* region base address register */
    __IO uint64 RASR;       /* region attributes and size register*/
    __IO uint64 RBAR_A1;    /* alias 1 of MPU_RBAR */
    __IO uint64 RASR_A1;    /* alias 1 of MPU_RASR */
    __IO uint64 RBAR_A2;    /* alias 2 of MPU_RBAR */
    __IO uint64 RASR_A2;    /* alias 2 of MPU_RASR */
    __IO uint64 RBAR_A3;    /* alias 3 of MPU_RBAR */
    __IO uint64 RASR_A3;    /* alias 3 of MPU_RASR */
} mpu_reg_map;

/* Memory Protection Unit register map base pointer */
#define MPU_BASE                        ((struct mpu_reg_map*)0xE000ED90)

#endif