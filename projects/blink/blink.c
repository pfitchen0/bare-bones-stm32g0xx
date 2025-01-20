#define RCC_IOPENR 0x40021034
#define GPIOC_MODER 0x50000800
#define GPIOC_ODR 0x50000814

static const unsigned int kDelayIterations = 1000000;

void delay(volatile unsigned int iterations) {
    while (iterations != 0) {
        iterations--;
    }
}

int main() {
    // The user LED is on PC6 (i.e. port GPIOC pin 6)

    // Enable GPIOC clock by setting the GPIOCEN bit in RCC_IOPENR
    *(unsigned int *)(RCC_IOPENR) |= (1 << 2);

    // Set the PC6 pin mode to output
    *(unsigned int *)(GPIOC_MODER) &= ~(1 << 13);
    *(unsigned int *)(GPIOC_MODER) |= (1 << 12);

    while(1) {
        // Set the PC6 pin high
        *(unsigned int *)(GPIOC_ODR) |= (1 << 6);

        delay(kDelayIterations);

        // Set the PC6 pin low
        *(unsigned int *)(GPIOC_ODR) &= ~(1 << 6);

        delay(kDelayIterations);
    }

    return 0;
}

void ResetHandler() {
    // Normally we'd need to set the stack pointer sp register here, but the STM32G0xx parts do this
    // automatically. They use the first entry in the vector table as the initial stack pointer, so
    // we do need to make sure the target initial stack pointer is specified there.

    // ST example startup script calls SystemInit to allow changing the default clock configuration
    // at this stage. However, I think it is OK to skip that here and configure clocks as desired
    // at the start of main().

    // Defined in the linkerscript:
    extern unsigned int flash_data_start, ram_data_start, ram_data_end, bss_start, bss_end;

    // We basically want to memcpy the .data section from flash to RAM, and memset the .bss section
    // to zero, but we can't use memcpy and memset because we are still setting up the C runtime
    // environment. Use loops and pointers instead.

    // Copy .data from flash to RAM.
    unsigned int *flash_data_src = &flash_data_start;
    unsigned int *ram_data_dst = &ram_data_start;
    while (ram_data_dst < &ram_data_end) {
        *ram_data_dst++ = *flash_data_src++;
    }

    // Zero-initialize .bss section.
    for (unsigned int *bss_idx = &bss_start; bss_idx < &bss_end; bss_idx++) {
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
    // Other interrupt/event handler function pointers would go here.
};
