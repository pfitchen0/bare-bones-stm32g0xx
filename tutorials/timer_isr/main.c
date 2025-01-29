#include <stdint.h>

#define RCC_IOPENR 0x40021034
#define GPIOC_MODER 0x50000800
#define GPIOC_ODR 0x50000814

#define SYST_CSR 0xE000E010
#define SYST_RVR 0xE000E014
#define SYST_CVR 0xE000E018

#define NVIC_ISER 0xE000E100

#define RCC_APBENR2 0x40021040
#define CPU_FREQ_HZ 16000000  // 16 MHz

#define TIM3_CR1 0x40000400
#define TIM3_PSC 0x40000428
#define TIM3_CCR1 0x40000434
#define TIM3_ARR 0x4000042C
#define TIM3_DIER 0x4000040C
#define TIM3_EGR 0x40000414
#define TIM3_SR 0x40000410

#define RCC_APBENR1 0x4002103C

// Current systick count since reset.
static volatile uint32_t systick = 0;

void DelayMilliseconds(uint32_t milliseconds) {
    uint32_t end_tick = systick + milliseconds;
    while (systick < end_tick);
}

int main() {
    // Enable the SysTick peripheral first.

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

    // The user LED is on PC6 (i.e. port GPIOC pin 6). Enable it.

    // Enable GPIOC clock by setting the GPIOCEN bit in RCC_IOPENR.
    *(uint32_t *)(RCC_IOPENR) |= (1 << 2);

    // Set the PC6 pin mode to output
    *(uint32_t *)(GPIOC_MODER) = (*(uint32_t *)(GPIOC_MODER) & ~(0b11 << 12)) |
                                 (0b01 << 12);
    

    // Enable the TIM3 clock.
    *(uint32_t *)(RCC_APBENR1) |= (1 << 1);

    // Enable the TIM3 interrupt in the Nested Vector Interrupt Controller (NVIC).
    // The TIM3 interrupt handler is the 16th (0-indexed) entry in the non-ARM part of the table.
    *(uint32_t *)(NVIC_ISER) |= (1 << 16);

    // Configure TIM3 prescaler to update the counter every 1 millisecond.
    *(uint32_t *)(TIM3_PSC) = (CPU_FREQ_HZ / 1000) - 1;

    // Configure TIM3 auto-reload register to trigger every 500 counts.
    *(uint32_t *)(TIM3_ARR) = 500;

    // Enable generation of an update event to reload the prescaler.
    *(uint32_t *)(TIM3_EGR) |= 1;

    // Initialize the TIM3 Channel 1 compare register to 0.
    *(uint32_t *)(TIM3_CCR1) = 0;

    // Enable the TIM3 Channel 1 update event interrupt.
    *(uint32_t *)(TIM3_DIER) |= 1;

    // Enable TIM3.
    *(uint32_t *)(TIM3_CR1) |= 1;

    while(1);

    return 0;
}

void Timer3Handler() {
    // Clear the pending interrupt flag.
    *(uint32_t *)(TIM3_SR) &= ~1;

    // Toggle the PC6 pin.
    *(uint32_t *)(GPIOC_ODR) ^= (1 << 6);
}

void SysTickHandler() {
    // Increment the systick count every time this handler gets called.
    systick++;
}

void ResetHandler() {
    // Normally we'd need to set the stack pointer sp register here, but the STM32G0xx parts do this
    // automatically. They use the first entry in the vector table as the initial stack pointer, so
    // we do need to make sure the target initial stack pointer is specified there.

    // ST example startup script calls SystemInit to allow changing the default clock configuration
    // at this stage. However, I think it is OK to skip that here and configure clocks as desired
    // at the start of main().

    // Defined in the linkerscript:
    extern uint32_t flash_data_start, ram_data_start, ram_data_end, bss_start, bss_end;

    // We basically want to memcpy the .data section from flash to RAM, and memset the .bss section
    // to zero, but we can't use memcpy and memset because we are still setting up the C runtime
    // environment. Use loops and pointers instead.

    // Copy .data from flash to RAM.
    uint32_t *flash_data_src = &flash_data_start;
    uint32_t *ram_data_dst = &ram_data_start;
    while (ram_data_dst < &ram_data_end) {
        *ram_data_dst++ = *flash_data_src++;
    }

    // Zero-initialize .bss section.
    for (uint32_t *bss_idx = &bss_start; bss_idx < &bss_end; bss_idx++) {
        *bss_idx = 0;
    }

    // If we needed to use .init_array (or similar) sections, do it here.
    // Don't forget `extern void __libc_init_array();`
    // __libc_init_array();

    // Now call main!
    main();

    // Infinite loop in case main returns for some reason.
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SysTickHandler,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    Timer3Handler,
    // Other interrupt/event handler function pointers would go here.
};
