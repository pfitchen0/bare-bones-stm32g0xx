# Timer Output Capture Compare Blink

As alluded to in the previous [timer_isr](../timer_isr/README.md) tutorial, we can utilize the Output Capture Compare mode of TIM3 CH1 to toggle the LED pin automatically every 500ms without using an interrupt (and therefore without consuming CPU cycles in our while loop). Let's see how we would modify our previous example to do this. This will also help us build up to using TIM3 CH1 for PWM output.

## Reconfigure TIM3 for Output Capture Compare

As in the previous tutorials, we mostly use the [STM32G0x1 reference manual](https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) and [STM32G031 datasheet](https://www.st.com/resource/en/datasheet/stm32g031c6.pdf) to identify which bits in which registers we need to change, and in what order.

First, we no longer need a TIM3 interrupt, so we can remove the associated interrupt setup and enable steps. We can remove the `NVIC_ISER`, `TIM3_EGR`, and `TIM3_DIER` register modifications. We can also remove the `Timer3Handler` function and entry from the vector table. We don't need the `TIM3_SR` definition anymore either.

Our TIM3 prescalar and auto-reload register values remain the same (`TIM3_PSC` and `TIM3_ARR` register settings remain the same). We also still need to enable the TIM3 clock in the `RCC_APBENR1` register as before. Finally at the end of our TIM3 initialization, we still need to enable the timer in the `TIM3_CR1` register.

So what do we need to change or add from the previous tutorial? Two main things:

1. We need to put the PC6 GPIO pin in "alternate mode", and specify the TIM3 alternate mode in particular. What is alternate mode? Alternate mode basically means control of the GPIO pin is handed off to a specific HW peripheral, allowing that peripheral to use the pin as needed. We can put the PC6 pin in alt mode in the `GPIOC_MODER` register, and then set the specific alt mode function to TIM3 CH1 in the `GPIOC_AFRL` register.

2. We need to enable Output Capture Compare mode with pin toggling on each capture compare for TIM3 CH1 using the `TIM3_CCMR1` and `TIM3_CCER` registers.

Putting it all together, our `main.c` file looks something like this:

```
#include <stdint.h>

#define RCC_IOPENR 0x40021034
#define GPIOC_MODER 0x50000800
#define GPIOC_AFRL 0x50000820
#define CPU_FREQ_HZ 16000000  // 16 MHz
#define TIM3_CR1 0x40000400
#define TIM3_PSC 0x40000428
#define TIM3_ARR 0x4000042C
#define TIM3_CCMR1 0x40000418
#define TIM3_CCER 0x40000420
#define RCC_APBENR1 0x4002103C

int main() {
    // Enable PC6 and put it in TIM3 CH1 alt mode. This pin is attached to the LED.
    *(uint32_t *)(RCC_IOPENR) |= (1 << 2);
    *(uint32_t *)(GPIOC_MODER) = (*(uint32_t *)(GPIOC_MODER) & ~(0b11 << 12)) |
                                 (0b10 << 12);
    *(uint32_t *)(GPIOC_AFRL) = (*(uint32_t *)(GPIOC_AFRL) & ~(0xF << 24)) |
                                 (0x1 << 24);

    // Configure and enable TIM3 CH1 in Output Capture Compare mode and have it toggle PC6 every 500ms.
    *(uint32_t *)(RCC_APBENR1) |= (1 << 1);
    *(uint32_t *)(TIM3_PSC) = (CPU_FREQ_HZ / 1000) - 1;
    *(uint32_t *)(TIM3_ARR) = 500;

    // Configure Timer 3 Channel 1 for Output Compare mode with toggle.
    // Set the OC1M bits in TIM3_CCMR1 to '011' (toggle mode).
    *(uint32_t *)(TIM3_CCMR1) |= (0b011 << 4);

    // Enable the output by setting the CC1E bit in TIM3_CCER.
    *(uint32_t *)(TIM3_CCER) |= (1 << 0);

    *(uint32_t *)(TIM3_CR1) |= 1;

    while (1);

    return 0;
}

// Same startup / ResetHandler code as before, but with Timer3Handler no longer needed.
...
```

Now note that our main while loop is completely empty *and* we no longer periodically consume CPU cycles with an interrupt to toggle the LED.

We also learned about GPIO alt mode and giving control of a GPIO directly to a timer! Let's extend this in the next tutorial by enabling PWM output on PC6 from TIM3 CH1.