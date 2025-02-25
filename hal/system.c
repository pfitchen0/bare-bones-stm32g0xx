#include <stdint.h>
#include <stddef.h>

extern int main();

void* _sbrk(ptrdiff_t incr) {
    // Linkerscript symbols
    extern uint8_t heap_start, initial_stack_ptr, min_stack_size;
    static uint8_t *current_heap_end = &heap_start;

    const uint32_t stack_limit = (uint32_t)&initial_stack_ptr - (uint32_t)&min_stack_size;
    const uint8_t *max_heap = (uint8_t *)stack_limit;
    uint8_t *previous_heap_end = current_heap_end;

    if (current_heap_end + incr > max_heap) {
        return NULL;  // Heap exhausted
    }

    current_heap_end += incr;
    return (void *)previous_heap_end;
}

__attribute__((optimize("O0")))
void ResetHandler() {
    // Linkerscript symbols
    extern uint32_t flash_data_start, ram_data_start, ram_data_end, bss_start, bss_end;

    // Copy .data from flash to RAM
    uint32_t *flash_data_src = &flash_data_start;
    uint32_t *ram_data_dst = &ram_data_start;
    while (ram_data_dst < &ram_data_end) {
        *ram_data_dst++ = *flash_data_src++;
    }

    // Zero-initialize .bss section
    for (uint32_t *bss_idx = &bss_start; bss_idx < &bss_end; bss_idx++) {
        *bss_idx = 0;
    }

    // If we needed to use .init_array (or similar) sections, do it here.
    // Don't forget `extern void __libc_init_array();`
    // __libc_init_array();

    main();
    while(1);
}

// Defined in linkerscript
extern void InitialStackPtr();

// Define the vector table, which is an array of 16 + 32 constant function pointers.
// There are 16 interrupt/event handlers reserved by ARM, and 32 specific to this STM32G0xx MCU.
// Make sure this vector table array gets placed in the .vector_table section.
__attribute__((section(".vector_table")))
void (*const vector_table[16 + 32])() = {
    InitialStackPtr,
    ResetHandler,
    // Other interrupt/event handler function pointers would go here.
};
