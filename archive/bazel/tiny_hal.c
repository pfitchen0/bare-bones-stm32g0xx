#include <stdint.h>
#include <stdbool.h>

#include "tutorials/bazel/tiny_hal.h"

void DelayIterations(uint32_t iterations) {
    while (iterations != 0) {
        iterations--;
    }
}

void InitLed() {
    *(uint32_t *)(RCC_IOPENR) |= (1 << 2);
    *(uint32_t *)(GPIOC_MODER) = (*(uint32_t *)(GPIOC_MODER) & ~(0b11 << 12)) |
                                 (0b01 << 12);
}

void SetLed(bool enabled) {
    if (enabled)
        *(uint32_t *)(GPIOC_ODR) |= (1 << 6);
    else
        *(uint32_t *)(GPIOC_ODR) &= ~(1 << 6);
}