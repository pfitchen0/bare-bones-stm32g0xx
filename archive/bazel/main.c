#include <stdint.h>
#include <stdbool.h>

#include "tutorials/bazel/tiny_hal.h"

static const uint32_t kBlinkDelayIterations = 1000000;

int main() {
    InitLed();

    while(true) {
        SetLed(true);
        DelayIterations(kBlinkDelayIterations);
        SetLed(false);
        DelayIterations(kBlinkDelayIterations);
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
    while(true);
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
