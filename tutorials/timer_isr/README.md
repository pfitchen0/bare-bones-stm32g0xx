# Timer Update ISR Blink

This tutorial builds on the previous [`blink`](../blink/README.md) and [`systick`](../systick/README.md) tutorials to continue improving our blinking LED demo. Yes... blinking an LED is probably getting boring, but there is still more we can do to make this blink demo a bit better.

We'll setup a timer to trigger an interrupt every 500ms. We'll then toggle the LED in that timer interrupt service routine (ISR). This will let our LED blink precisely at 500ms without consuming CPU cycles to delay between blinks. This will let us do other things in FW while keeping the LED blinking precisely.

## Background

Most MCUs include a few timer peripherals. These timer peripherals can vary in complexity, but most of them work in a fashion similar to the SysTick peripheral: they have a counter that is driven by an internal clock, and they have a set of registers that can be used to trigger interrupts based on the counter value. They also often have multiple "channels" per timer. Each channel is basically an individually configurable set of registers to trigger events based on a shared underlying counter for the timer. More complex timers can be configured to do things like:

* Generate precise PWM outputs with variable duty cycle, frequency, etc...

    * Dim LEDs, control a switching supply, etc... In general, PWM signals are used as a control mechanism by varying the duty cycle.

    * Precise time measurement or sequencing between events.

    * Motor control applications - driving H-bridges, FETs, or even more advanced BLDC motor control.

    * Simulate an analog output from the MCU - if you pass a PWM waveform through a low pass filter, you can emulate an analog output. This is what Arduinos do on their analog output pins. But note that some MCUs have Digital to Analog Converter peripherals, which are much more accurate and less noisy than using a PWM output passed through a low pass filter - don't confuse these :).

    * Watchdog timers. These are timers that must be reset every so often by the firmware, otherwise they will trigger an interrupt. We design the firmware, watchdog timer configuration, and associated interrupt to detect when the firmware might be stalled in some way. Watchdog timers act like a safety net, automatically resetting the MCU if the firmware doesn't periodically "pet" the watchdog within a predefined time interval. We'll look at this more in a future tutorial.

    * Precise delays or task scheduling, without consuming CPU cycles (what we'll do for now!).

> *Note:* There are many more examples... These are just a few.

In this tutorial example, we'll setup a timer to behave just like our SysTick peripheral in the [`systick`](../systick/README.md) tutorial, except that the associated interrupt will be triggered at 500ms intervals instead of 1ms intervals. And we'll toggle the LED directly in the ISR.

## Implementation

Ok, diving into the implementation... Starting by looking at the [datasheet](https://www.st.com/resource/en/datasheet/stm32g031c6.pdf) for our STM32G031 part, we see that there are: 11 timers (one 128 MHz capable): *"16-bit for advanced motor control, one 32-bit and four 16-bit general-purpose, two low-power 16-bit, two watchdogs, SysTick timer."*

How do we pick which timer to use? Table 7 on page 25 of the [datasheet](https://www.st.com/resource/en/datasheet/stm32g031c6.pdf) contains a comparison of each timer. Well, for our basic LED blink usage in this tutorial, any of these will do. However, we'll pick Timer 3, channel 1 (TIM3 CH1) because one of the output capture/compare channels of this timer can be directly tied to GPIOC Pin 6 (PC6) (see Table 12 on page 38 of the [datasheet](https://www.st.com/resource/en/datasheet/stm32g031c6.pdf)). This will let us dim the LED with PWM in a future tutorial.

Ok, great. So what do we need to do to configure and enable Timer 3 to generate an interrupt every 500ms?

It's pretty simple, but first we need to talk about the Nested Vector Interrupt Controller (NVIC) a little bit more.

The NVIC is a crucial component within ARM Cortex-M microcontrollers. It's the heart of interrupt management. Think of it as the traffic controller for all the interrupt requests your microcontroller might receive. These requests can come from various peripherals (like timers, UARTs, ADCs, GPIOs, etc.) or even software.

The NVIC is responsible for the following main things:

* Manages Interrupt Priorities: The NVIC allows you to assign different priority levels to various interrupts. This ensures that more critical interrupts can preempt (interrupt and pause) less critical interrupts, ensuring the system responds to the most urgent events first.

* Controls Interrupt Enabling/Disabling: The NVIC is used to control when a particular interrupt is enabled (allowed to happen) or disabled.

* Interrupt "Vectoring": When an interrupt occurs, the NVIC determines the correct interrupt service routine (ISR) to execute. It does this by using the interrupt vector table discussed previously. This process is called vectoring.

Here's an example of the NVIC in action for our TIM3 CH1 blink example that we're working on:

1. Peripheral Interrupt Request: A peripheral (e.g., TIM3) generates an interrupt request.

2. Enabled Check: The NVIC checks if that specific interrupt is enabled.

3. Interrupt Priority Check: The NVIC compares the priority of the pending interrupt with the priority of any other currently running or pending interrupt.

4. Interrupt Vectoring (if enabled and high enough priority): If the interrupt is enabled and its priority is high enough, the NVIC looks up the corresponding ISR address in the vector table.

5. ISR Execution: The CPU jumps to the ISR and executes the code.

6. Interrupt Acknowledgment (in ISR): Usually, within the ISR, you clear the interrupt condition in the peripheral (e.g., clear the timer interrupt flag).

One last note about the NVIC and interrupts. The ARM-reserved interrupts in the vector table (i.e. the SysTick interrupt) are usually enabled by default, but the non-ARM-reserved interrupts that are MCU specific, like our TIM3 interrupt, are disabled by default. We'll need to specifically enable the TIM3 interrupt, which we didn't need to do for the SysTick interrupt in the previous tutorial.

Now we can define the steps to enable TIM3 CH1 to generate a TIM3 interrupt every 500ms.

1. Enable the TIM3 source clock that drives the TIM3 internal counter. This is done via one of the RCC registers again - specifically the `RCC_APBENR` register.

2. Enable the TIM3 interrupt in the `NVIC_ISER` register as discussed above.

3. Like we did in the [`systick` tutorial](../systick/README.md), configure the timer's counter to increment every millisecond. But this time, we'll use the prescalar register (`TIM3_PSC`). Set the Auto-Reload Register (`TIM3_ARR`) to 500 to generate an interrupt every 500ms instead of 1ms.

4. Enable generation of an update event, and allow an update event on TIM3 CH1 to generate a TIM3 interrupt. This requires the `TIM3_EGR` and `TIM3_DIER` registers.

5. Finally, enable TIM3 in the `TIM3_CR1` register.

After that, we just need to define a `Timer3Handler` and insert it into our vector table. Inside that handler, we'll toggle the `PC6` pin attached to the LED.

```
void Timer3Handler() {
    // Clear the pending interrupt flag.
    *(uint32_t *)(TIM3_SR) &= ~1;

    // Toggle the PC6 pin.
    *(uint32_t *)(GPIOC_ODR) ^= (1 << 6);
}
```

We just need to do two things in the handler:

1. Clear the associated "update event" pending TIM3 interrupt flag in the `TIM3_SR` register to acknowledge that the interrupt has been handled (otherwise our code would jump right back into the interrupt routine).

2. Toggle the `PC6` pin using the `GPIOC_ODR` register like we have in previous tutorials.

And from Table 61 in the [STM32G0x1 regerence manual](https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf), we can see that the TIM3 global interrupt is the 16th (0-indexed) non-ARM reserved vector table entry.

Putting it altogether, our `main.c` file looks something like the following:

```
#include <stdint.h>

#define RCC_IOPENR 0x40021034
#define GPIOC_MODER 0x50000800
#define GPIOC_ODR 0x50000814
#define NVIC_ISER 0xE000E100
#define CPU_FREQ_HZ 16000000  // 16 MHz
#define TIM3_CR1 0x40000400
#define TIM3_PSC 0x40000428
#define TIM3_ARR 0x4000042C
#define TIM3_DIER 0x4000040C
#define TIM3_EGR 0x40000414
#define TIM3_SR 0x40000410
#define RCC_APBENR1 0x4002103C

int main() {
    // Enable PC6 as a push-pull output. This pin is attached to the LED.
    *(uint32_t *)(RCC_IOPENR) |= (1 << 2);
    *(uint32_t *)(GPIOC_MODER) = (*(uint32_t *)(GPIOC_MODER) & ~(0b11 << 12)) |
                                 (0b01 << 12);
    

    // Configure and enable TIM3 CH1 to generate an update event interrupt every 500ms.
    *(uint32_t *)(RCC_APBENR1) |= (1 << 1);
    *(uint32_t *)(NVIC_ISER) |= (1 << 16);
    *(uint32_t *)(TIM3_PSC) = (CPU_FREQ_HZ / 1000) - 1;
    *(uint32_t *)(TIM3_ARR) = 500;
    *(uint32_t *)(TIM3_EGR) |= 1;
    *(uint32_t *)(TIM3_DIER) |= 1;
    *(uint32_t *)(TIM3_CR1) |= 1;

    while(1);

    return 0;
}

void Timer3Handler() {
    // Clear the pending interrupt flag and toggle the LED GPIO pin.
    *(uint32_t *)(TIM3_SR) &= ~1;
    *(uint32_t *)(GPIOC_ODR) ^= (1 << 6);
}

void ResetHandler() {
    // Same startup code as before.
    extern uint32_t flash_data_start, ram_data_start, ram_data_end, bss_start, bss_end;
    uint32_t *flash_data_src = &flash_data_start;
    uint32_t *ram_data_dst = &ram_data_start;
    while (ram_data_dst < &ram_data_end) {
        *ram_data_dst++ = *flash_data_src++;
    }
    for (uint32_t *bss_idx = &bss_start; bss_idx < &bss_end; bss_idx++) {
        *bss_idx = 0;
    }
    main();
    while(1);
}

// Defined in the linkerscript.
extern void initial_stack_ptr();

// Define the vector table, which is an array of 16 + 32 constant function pointers.
// There are 16 interrupt/event handlers reserved by ARM, and 32 specific to this STM32G0xx MCU.
// Make sure this vector table array gets placed in the .vector_table section.
__attribute__((section(".vector_table")))
void (*const vector_table[16 + 32])() = {
    initial_stack_ptr,
    ResetHandler,
    // Other ARM reserved interrupt/event handlers would replace these 0s.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Start of non ARM reserved entries.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    Timer3Handler,
    // Other interrupt/event handler function pointers would go here.
};
```

A few things to highlight:

1. Our main while loop is now totally empty. The LED is being toggled in the timer interrupt routine outside of main.

2. We could do things in our main while loop (run some algorithm, computation, delay, etc...) and our LED would still blink at 500ms.

3. Even though the LED is toggled outside of main, it does require a few of our CPU cycles (albeit only a few). The `Timer3Handler` still consumes a few cycles whenever it is triggered.

Wouldn't it be extra cool if we could allow our LED to blink without consuming *any* of our CPU cycles? It turns out we can do this using a timer feature called Output Capture Compare, which will let the timer drive the GPIO pin directly. We just need some one-time setup code at the start of main, and then we can even get rid of our `Timer3Handler` interrupt! We'll do that in the next tutorial...

As an aside though, I often like to use a timer interrupt to blink an LED in my projects. The Output Capture Compare feature isn't available on all timers or all MCUs, but there's another reason I prefer to use an interrupt specifically.

We can configure the timer interrupt to have the lowest priority of all our interrupts. This means that *any* other interrupt can preempt our timer interrupt. If our processor somehow gets stuck in another interrupt or thread, our LED won't be able to toggle because the associated timer interrupt never gets to run. This provides a quick visual way to tell if the processor is locked up from higher priority interrupts or not. Of course there are other ways to detect this, but I find this to be a convenient trick.