#include "hal/gpio.h"

#include <stdbool.h>
#include <stdint.h>

#include "hal/macros.h"

#define RCC_IOPENR *(uint32_t *)(0x40021034)
#define GPIO_BASE 0x50000000
#define GPIO_REGS(port) ((GpioRegisters *)(GPIO_BASE + (0x400 * (port))))

void ConfigureGpio(Gpio gpio, GpioSettings settings) {
    SET_BIT(RCC_IOPENR, (1 << gpio.port));
    MODIFY_REG(GPIO_REGS(gpio.port)->moder, (0b11 << (2 * gpio.pin)), (settings.mode << (2 * gpio.pin)));
    if (settings.mode == kOutput) {
        MODIFY_REG(GPIO_REGS(gpio.port)->otyper, (1 << gpio.pin), (settings.otype << gpio.pin));
        MODIFY_REG(GPIO_REGS(gpio.port)->ospeedr, (0b11 << (2 * gpio.pin)), (settings.ospeed << (2 * gpio.pin)));
    }
    MODIFY_REG(GPIO_REGS(gpio.port)->pupdr, (0b11 << (2 * gpio.pin)), (settings.pupd << (2 * gpio.pin)));
    if (settings.mode == kAlternateFunction) {
        MODIFY_REG(GPIO_REGS(gpio.port)->afrl, ((0b0111) << (4 * gpio.pin)), ((settings.afsel) << (4 * gpio.pin)));
    }
}

void SetGpio(Gpio gpio, bool state) {
    if (state) {
        WRITE_REG(GPIO_REGS(gpio.port)->bsrr, (1 << gpio.pin));
    } else {
        WRITE_REG(GPIO_REGS(gpio.port)->bsrr, (1 << (gpio.pin + 16)));
    }
}

bool GetGpio(Gpio gpio) {
    return READ_BIT(GPIO_REGS(gpio.port)->idr, (1 << gpio.pin));
}