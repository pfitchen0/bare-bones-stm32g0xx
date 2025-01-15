# Blink Demo

This is just a simple blink LED demo for the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) development board to demonstrate the basics of startup code and a linkerscript from scratch.

## MCU Start Up Process

**What happens before `main`?**

Well, it depends on the particular MCU, but in general something like the following:

1. MCU hardware boots up: power supply(ies) finish ramping up, internal PLL(s) lock / minimum clock configuration is initialized, etc...

2. Some MCUs (including our STM32G0xx parts) have internal ROM bootloaders that they will jump to depending on external pin strapping (like the BOOT0 & BOOT1 pins on STM32 MCUs). For example, if the BOOT0 pin is high, the MCU will set the PC register to an internal ROM address corresponding to a bootloader and start executing from there.

3. But assuming a normal boot sequence to our FW, the MCU will start executing from a specific, fixed address in flash memory. The MCU hardware will set the PC register to this fixed address upon reset.

4. Next, the SP (stack pointer) register needs to point to the start of the stack, which is typically the end of the RAM region - the stack grows down. Some MCUs do this in HW automatically (just like they do for the PC register), but others require doing this manually with a load register instruction. Once the stack pointer is set, we can start calling functions :).

5. Then the C (& C++) runtime environment needs to get initialized. There are a few steps to this:

    a. Copy global and static variables to RAM so they can be modified during runtime.

    b. Initialize uninitialized variables to zero. Upon reset, RAM may not be initialized to 0, so we cannot assume our uninitialized variables are default initialized to 0 or null like we expect unless we do this manually in our startup code.

    c. Call `__libc_init_array`. This function calls static/global variable constructors and otherwise initializes more complex static/global variables that can't simply be initialized with assignment. How this works: linker magic...

        i. The linker collects a list of function pointers to constructors/initialization functions that need to be called. This list gets stored in a special memory section typically defined in the linkerscript as `.init_array`.

        ii. The startup code loops through the function pointers in the `.init_array` section and calls each one.

6. Finally, we can call `main`!

7. For MCU embedded systems, we typically expect that the `main` function finishes initializing HW peripherals and then enters an infinite while loop, thus never returning. But just in case `main` returns for some reason, it is usually a good idea to include an infinite while loop or call some kind of hard fault handler after `main` is called.

## Vector Table

ARM MCUs, like the STM32G0xx series, have what's called a "Vector Table" at fixed memory address (typically the start of flash memory). The Vector Table is essentially a look-up table of address in memory, mostly for event/interrupt handler function addresses. The first entry in the vector table is the address that the stack pointer should be initialized to. And the second entry is the address of the Reset Handler. This is the code that the MCU starts executing immediately after power-on or reset. It typically initializes the system and sets up the MCU for operation by doing the things listed above. The first 16 entries are reserved by ARM and are common for all ARM MCUs; the remainder of the vector table entries are for interrupt/event handlers that are specific to a particular MCU.

## References

The official "template" startup code from ST/ARM for gcc, [startup_stm32g031xx.s](https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Source/Templates/gcc/startup_stm32g031xx.s) contains everything discussed above. See that as a more official/polished example. You can also find an example linkerscript from ST [here](https://github.com/STMicroelectronics/STM32CubeG0/blob/master/Projects/NUCLEO-G031K8/Templates/STM32CubeIDE/STM32G031K8TX_FLASH.ld). Note that the [startup code](https://github.com/STMicroelectronics/STM32CubeG0/blob/master/Projects/NUCLEO-G031K8/Templates/STM32CubeIDE/Application/Startup/startup_stm32g031k8tx.s) is also the same in this ST repository.
