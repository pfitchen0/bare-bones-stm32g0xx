#ifndef HAL_RCC_H_
#define HAL_RCC_H_

#include <stdint.h>

typedef struct {
    volatile uint32_t cr, icscr, cfgr, pllcfgr, reserved, crrcr, cier, cifr,
                      cicr, ioprstr, ahbrstr, apbrstr1, apbrstr2, iopenr,
                      ahbenr, apbenr1, apbenr2, iopsmenr, ahbsmenr, apbsmenr1,
                      apbsmenr2, ccipr, ccipr2, bdcr, csr;
} RccRegisters;
#define RCC_BASE 0x40021000
#define RCC_REGS ((RccRegisters *)(RCC_BASE))

#endif  // HAL_RCC_H_