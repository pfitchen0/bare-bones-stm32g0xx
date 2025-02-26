#include <stdint.h>

#define RCC_IOPENR 0x40021034
#define GPIOA_MODER 0x50000000
#define GPIOA_AFRL 0x50000020

#define SYST_CSR 0xE000E010
#define SYST_RVR 0xE000E014
#define SYST_CVR 0xE000E018

#define NVIC_ISER 0xE000E100

#define RCC_APBENR2 0x40021040
#define CPU_FREQ_HZ 16000000  // 16 MHz

#define RCC_APBENR1 0x4002103C
#define USART2_CR1 0x40004400
#define USART2_BRR 0x4000440C
#define USART2_TDR 0x40004428
#define USART_ISR 0x4000441C

// Current systick count since reset.
static volatile uint32_t systick = 0;

void DelayMilliseconds(uint32_t milliseconds) {
    uint32_t end_tick = systick + milliseconds;
    while (systick < end_tick);
}

void Usart2Tx(uint8_t byte) {
    *(uint32_t *)(USART2_TDR) = byte;
    while (!(*(uint32_t *)(USART_ISR) & (1 << 6)));
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

    // Enable GPIOA clock by setting the GPIOAEN bit in RCC_IOPENR.
    *(uint32_t *)(RCC_IOPENR) |= (1 << 0);

    // Set the PA2 (TX) and PA3 (RX) pin mode to alternate function mode
    *(uint32_t *)(GPIOA_MODER) = (*(uint32_t *)(GPIOA_MODER) & ~(0b11 << 4)) |
                                 (0b10 << 4);
    *(uint32_t *)(GPIOA_MODER) = (*(uint32_t *)(GPIOA_MODER) & ~(0b11 << 6)) |
                                 (0b10 << 6);

    // Set the alternate function for PA2 and PA3 to USART2. See datasheet pg. 40 Table 13.
    *(uint32_t *)(GPIOA_AFRL) = (*(uint32_t *)(GPIOA_AFRL) & ~(0xF << 8)) |
                                 (0x1 << 8);
    *(uint32_t *)(GPIOA_AFRL) = (*(uint32_t *)(GPIOA_AFRL) & ~(0xF << 12)) |
                                 (0x1 << 12);

    // Enable USART2 clock.
    *(uint32_t *)(RCC_APBENR1) |= (1 << 17);

    // Configure USART2 baud rate to 115200
    *(uint32_t *)(USART2_BRR) = CPU_FREQ_HZ / 115200;

    // Configure USART2: 8 data bits, no parity, 1 stop bit
    *(uint32_t *)(USART2_CR1) |= (1 << 3) | (1 << 2); // Enable TE and RE

    // Enable USART2
    *(uint32_t *)(USART2_CR1) |= (1 << 0);

    // Transmit and receive characters
    while (1) {
        // Transmit 'A'
        Usart2Tx('A');

        // Receive a character
        // char receivedChar = USART2_ReceiveChar();

        // Do something with the received character
        //...

        DelayMilliseconds(1000);
    }

    return 0;
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

    // We basically want to memcpy the.data section from flash to RAM, and memset the.bss section
    // to zero, but we can't use memcpy and memset because we are still setting up the C runtime
    // environment. Use loops and pointers instead.

    // Copy.data from flash to RAM.
    uint32_t *flash_data_src = &flash_data_start;
    uint32_t *ram_data_dst = &ram_data_start;
    while (ram_data_dst < &ram_data_end) {
        *ram_data_dst++ = *flash_data_src++;
    }

    // Zero-initialize.bss section.
    for (uint32_t *bss_idx = &bss_start; bss_idx < &bss_end; bss_idx++) {
        *bss_idx = 0;
    }

    // If we needed to use.init_array (or similar) sections, do it here.
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
// Make sure this vector table array gets placed in the.vector_table section.
__attribute__((section(".vector_table")))
void (*const vector_table[16 + 32])() = {
    initial_stack_ptr,
    ResetHandler,
    // Other ARM reserved interrupt/event handlers would replace these 0s.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SysTickHandler,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,
    // Other interrupt/event handler function pointers would go here.
};