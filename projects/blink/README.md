# Blink Demo

This is just a simple blink LED demo for the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) development board to demonstrate the basics of startup code and a linkerscript from scratch.

## Start Up Code

What happens before `main()`?

Well, it depends on the particular MCU, but in general something like the following:

1. MCU hardware boots up: power supply(ies) finish ramping up, internal PLLs lock / clocks start, etc...

2. Most MCUs start executing from a specific, fixed address in memory. The MCU hardware will set the PC register to this fixed address upon reset.

3. Next, the SP (stack pointer) register needs to point to the start of the stack, which is typically the end of the RAM region - the stack grows down. Some MCUs do this in HW automatically (just like they do for the PC register), but others require doing this manually with a load register instruction.

TODO: finish writing this; i am tired and want to go to bed.
4. ... more stuff, MCU-specific things like jumping to ROM bootloader based on boot pins, etc...

5. Initialize the C (& C++) Runtime environment... copy global and static variables to RAM so they can be modified during runtime, initialize uninitialized variables to zero, etc... Call constructors...

ARM MCUs, like the STM32G0xx series, have what's called a "Vector Table" at fixed memory address (typically the start of flash memory). The Vector Table is essentially a look-up table of address in memory, mostly for event/interrupt handler function addresses. The first entry in the vector table is the address that the stack pointer should be initialized to. And the second entry is the address of the Reset Handler. This is the code that the MCU starts executing immediately after power-on or reset. It typically initializes the system and sets up the MCU for operation by doing the things listed above.

The official "template" startup code from ST/ARM for gcc, [startup_stm32g031xx.s](https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Source/Templates/gcc/startup_stm32g031xx.s) contains everything discussed above. See that as a more official/polished example.