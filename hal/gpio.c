#include "hal/gpio.h"

#define RCC_IOPENR 0x40021034
#define GPIO_BASE 0x50000000
#define GPIO(port) ((GpioRegisters *)(GPIO_BASE + 0x400*(port)))

void InitGpio(GpioPort port, uint8_t pin) {
    *(uint32_t *)(RCC_IOPENR) |= (1 << port);
    GPIO(port)->MODER = ((GPIO(port)->MODER) & ~(0b11 << 2*pin)) | (0b01 << 2*pin);
}

void SetGpio(GpioPort port, uint8_t pin, bool state) {
    if (state)
        GPIO(port)->ODR |= (1 << pin);
    else
        GPIO(port)->ODR &= ~(1 << pin);
}