int main() {
    return 0;
}

__attribute__((naked, noreturn))
void ResetHandler() {
    // Normally we'd need to set the stack pointer sp register here, but the STM32G0xx parts do this
    // automatically. They use the first entry in the vector table as the initial stack pointer, so
    // we do need to make sure the target initial stack pointer is specified there.

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

__attribute__((section(".vector_table")))
void (*const vector_table[16 + 32])() = {
    initial_stack_ptr,
    ResetHandler,
    // Other event/interrupt handler function pointers would go here.
};
