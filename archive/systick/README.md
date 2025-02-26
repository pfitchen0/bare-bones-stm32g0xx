# SysTick

This tutorial builds on the previous [`blink`](../blink/README.md) tutorial by adding support for the SysTick interrupt. The SysTick is a very simple 24 bit down-counting timer that is common to all ARM MCUs. When the SysTick peripheral is enabled, the SysTick timer will count down to 0. Everytime it gets to 0, the SysTick interrupt is called.

We can use this SysTick mechanism to improve our timekeeping in the blink demo previously built. We'll configure the SysTick to generate an interrupt every 1ms, and then use that to implement a `DelayMilliseconds` function that lets our processor delay for a specified number of milliseconds. With this, we can set out LED to blink more accurately at 1 second intervals.

## Background

The SysTick peripheral is technically an optional add-on to ARM Cortex M0+ (and all other ARM MCU architectures that I am aware of), but it is almost always included. Some sort of main periodic interrupt is necessary for many firmware applications. Examples include:

* Simple delay function (what we'll use it for, for now)

* Scheduling a periodic task

* Similar to above, RTOSes rely on a systick of some sort to run their more complicated scheduling algorithms

* Timekeeping (i.e. for a function timeout)

* etc...

Because these use cases are so common, ARM defined a standard SysTick peripheral with code portability, ease of use, and low overhead in mind. Since the SysTick peripheral is defined by ARM, there isn't a lot of info about it in the STM32G0xx reference manual that we've grown familiar with. Instead, we must look to ARM's Cortex M0+ [datasheet](https://developer.arm.com/documentation/102835/latest/) and [reference manual](https://developer.arm.com/documentation/ddi0484/c/?lang=en) directly. Furthermore, page 35 of the reference manual further directs us to the [Armv6-M Architecture reference manual](https://developer.arm.com/documentation/ddi0419/latest/) as well.

The SysTick peripheral is very simple. See section B3.3 on page 237 of the [Armv6-M Architecture reference manual](https://developer.arm.com/documentation/ddi0419/latest/). It only has 4 registers:

* `SYST_CSR`: SysTick Control and Status Register, which allows us to select the source clock for the SysTick timer, enable the counter, enable the SysTick interrupt, etc...

* `SYST_RVR`: SysTick Reload Register, which specifies the value that the counter gets reset to after hitting 0.

* `SYST_CVR`: SysTick Current Value Register, which (as the name suggests) contains the current value of the 24bit down counter.

* `SYST_CALIB`: SysTick Calibration Register, which can be used to calibrate the SysTick timer for variations in the SysTick clock source. We won't use this.

## Using SysTick In Code!

Using the SysTick peripheral to generate 1ms ticks is easy. We just need to do the following:

1. Define macros for the SysTick register addresses for easy use.

2. Set the `SYST_RVR` register based on our current system clock so that the SysTick counter expires every millisecond.

3. Initialize the `SYST_CVR` to 0 (unclear if this is done automatically after reset).

4. Enable the SysTick timer with the `SYST_CSR` register.

5. Enable the MCU's clock for the SysTick peripheral using the `RCC_APBENR2` register.

6. Define a global `volatile` variable to count the number of ticks. Think of this variable as a millisecond counter since reset for our MCU, sort of like Unix/epoch time (seconds since 1970) for PCs. More on the `volatile` part later.

7. Define and implement a `SysTickHandler` interrupt service routine (ISR) that manages the ticks variable.

8. Insert a function pointer to our `SysTickHandler` ISR into the 16th entry in the vector table (15th position, 0 indexed). This is the last of the ARM Cortex M0+ reserved entries at the start of the table.

To maintain code portability between ARM MCUs, the 4 SysTick registers are at the same memory addresses in all ARM Cortex M0+ MCUs (and maybe more ARM MCUs). Let's define macros to these addresses, which can be found in section B3.3.2 on page 238 of the [Armv6-M Architecture reference manual](https://developer.arm.com/documentation/ddi0419/latest/):

```
#define SYST_CSR 0xE000E010
#define SYST_RVR 0xE000E014
#define SYST_CVR 0xE000E018
```

We'll also need to use the RCC_APBENR2 register to enable the SysTick peripheral, and we'll want a macro to specify the CPU frequency, which is 16MHz by default.

```
#define RCC_APBENR2 0x40021040
#define CPU_FREQ_HZ 16000000  // 16 MHz
```

> *Note:* We won't worry about SysTick calibration, so we don't need the `SYST_CALIB` register in this tutorial.

Next, let's initialize the SysTick peripheral during our setup in `main`. Specifically, we need to handle steps 2 - 5 above. We can do this with just a few lines:

```
// Set the SysTick Reload Value Register to 1 millisecond based on the CPU_FREQ_HZ.
// Subtract 1 because the 0th counter value is included.
*(uint32_t *)(SYST_RVR) = (CPU_FREQ_HZ / 1000) - 1;

// Initialize the SysTick Current Value Register.
*(uint32_t *)(SYST_CVR) = 0;

// Set the ENABLE, TICKINT, and CLKSOURCE bits to 1 in the SysTick Control and Status Register.
// This enables the SysTick counter, allows the counter reaching 0 to generate an interrupt,
// and sets the SysTick clock source to the internal system clock, respectively.
*(uint32_t *)(SYST_CSR) |= 0b111;

// Finally, enable the SYSCFG clock by setting the 0th bit in the RCC_APBENR2 register.
*(uint32_t *)(RCC_APBENR2) |= 1;
```

Now let's define a static `volatile`(!!!) variable called `systick` that counts the number of systick interrupts (configured to happen every millisecond in our case) since reset. We need to mark this variable with the `volatile` keyword to tell the compiler not to make any assumptions or optimizations around this variable. This is because we are updating this variable in an interrupt handler, which can be called at anytime and therefore could happen between successive accesses to the `systick` variable. The compiler may assume that the `systick` variable is unchanged between successive calls and could make optimizations based on that, which we don't want. Initialize it to 0.

```
// Current systick count since reset.
static volatile uint32_t systick = 0;
```

Let's also define the `SysTickHandler` interrupt service routine that will get called every SysTick interrupt. We'll increment the `systick` variable in this handler.

```
void SysTickHandler() {
    systick++;
}
```

Now insert the `SysTickHandler` to the 16th (or 15th zero indexed) position in the vector table:

```
__attribute__((section(".vector_table")))
void (*const vector_table[16 + 32])() = {
    initial_stack_ptr,
    ResetHandler,
    // Other ARM reserved interrupt/event handlers would replace these 0s.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SysTickHandler,
    // Other interrupt/event handler function pointers would go here.
};
```

And the last thing we need to do for this demo is update our `DelayIterations` function to become a `DelayMilliseconds` function:

```
static const uint32_t kBlinkDelayMilliseconds = 500;

void DelayMilliseconds(uint32_t milliseconds) {
    uint32_t end_tick = systick + milliseconds;
    while (systick < end_tick);
}
```

## Updared main.c

With all of these changes, here's what our `main.c` should look like now:

```
#include <stdint.h>

#define RCC_IOPENR 0x40021034
#define GPIOC_MODER 0x50000800
#define GPIOC_ODR 0x50000814

#define SYST_CSR 0xE000E010
#define SYST_RVR 0xE000E014
#define SYST_CVR 0xE000E018

#define RCC_APBENR2 0x40021040
#define CPU_FREQ_HZ 16000000  // 16 MHz

static volatile uint32_t systick = 0;

static const uint32_t kBlinkDelayMilliseconds = 500;

void DelayMilliseconds(uint32_t milliseconds) {
    uint32_t end_tick = systick + milliseconds;
    while (systick < end_tick);
}

int main() {
    *(uint32_t *)(SYST_RVR) = (CPU_FREQ_HZ / 1000) - 1;
    *(uint32_t *)(SYST_CVR) = 0;
    *(uint32_t *)(SYST_CSR) |= 0b111;
    *(uint32_t *)(RCC_APBENR2) |= 1;

    *(uint32_t *)(RCC_IOPENR) |= (1 << 2);
    *(uint32_t *)(GPIOC_MODER) = (*(uint32_t *)(GPIOC_MODER) & ~(0b11 << 12)) |
                                 (0b01 << 12);

    while(1) {
        *(uint32_t *)(GPIOC_ODR) |= (1 << 6);
        DelayMilliseconds(kBlinkDelayMilliseconds);
        *(uint32_t *)(GPIOC_ODR) &= ~(1 << 6);
        DelayMilliseconds(kBlinkDelayMilliseconds);
    }

    return 0;
}


void SysTickHandler() {
    systick++;
}

void ResetHandler() {
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

extern void initial_stack_ptr();

__attribute__((section(".vector_table")))
void (*const vector_table[16 + 32])() = {
    initial_stack_ptr,
    ResetHandler,
    // Other ARM reserved interrupt/event handlers would replace these 0s.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SysTickHandler,
    // Other interrupt/event handler function pointers would go here.
};
```