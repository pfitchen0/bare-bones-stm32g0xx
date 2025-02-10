#ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2], BRR;
} GpioRegisters;

typedef enum {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF
} GpioPort;

void InitGpio(GpioPort port, uint8_t pin);

void SetGpio(GpioPort port, uint8_t pin, bool state);

#endif  // HAL_GPIO_H_