#ifndef TUTORIALS_BAZEL_TINY_HAL_H_
#define TUTORIALS_BAZEL_TINY_HAL_H_

#include <stdint.h>

#define RCC_IOPENR 0x40021034
#define GPIOC_MODER 0x50000800
#define GPIOC_ODR 0x50000814

void DelayIterations(uint32_t iterations);

void InitLed();
void SetLed(bool enabled);

#endif  // TUTORIALS_BAZEL_TINY_HAL_H_