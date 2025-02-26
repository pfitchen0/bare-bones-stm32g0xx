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