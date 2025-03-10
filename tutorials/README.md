# Building Bare-Bones-STM32G0xx From Scratch

*"If you can't explain it simply, you don't understand it well enough"* -Albert Einstein (I think).

This tutorial is my attempt to understand microcontroller-based embedded systems programming more deeply by explaining them here, simply.

## Overview

In this tutorial we'll build up some example STM32G0xx firmware ***completely from scratch***. We'll start by discussing how MCUs boot, writing startup code, and setting up a minimal C runtime environment. After that we'll be able to boot into a `main` function and write/run C programs that might look more familiar to someone who hasn't gone so low level before. We'll continue to build a few simple examples to demonstrate key MCU fundamentals like memory-mapped peripherals, interrupts, and debugging.

Here's a rough table of contents:

0. [Prerequisites](#prerequisites)
1. [MCU Startup Overview](#mcu-startup-overview)
2. [Vector Table](#vector-table)
3. [Linkerscript](#linkerscript)
4. [Startup Code](#startup-code)
5. [Makefile](#makefile)
6. [Heap and Dynamic Memory Allocation](#heap-and-dynamic-memory-allocation)
7. [LED Blink](#led-blink)
8. [Flashing FW](#flashing-fw)
9. [SysTick](#systick)
10. [Clock Configuration](#clock-configuration)
11. [Timers](#timers)
12. [UART](#uart)
13. [Printf](#printf)
14. [Debugging](#debugging)

## Prerequisites

### Prior Knowledge

Since this is a "from scratch" tutorial, I don't want to assume too much prior knowledge. However, I will assume some prior knowledge of **C programming basics** and the **compilation and linking process**. If these are unfamiliar topics to you, I'd recommend checking out the following to get familiar with these topics:
* Harvard's [CS50 Lecture 1: C](https://www.youtube.com/watch?v=89cbCbWrM4U).
* Jacob Sorber's [YouTube channel](https://www.youtube.com/@JacobSorber).
* The famous ["The C Programming Language"](https://www.amazon.com/Programming-Language-Brian-W-Kernighan/dp/B001SGWKXA/ref=asc_df_B001SGWKXA?mcid=433e9116bbf9385699fcd969ec2698c3&tag=hyprod-20&linkCode=df0&hvadid=709938295378&hvpos=&hvnetw=g&hvrand=7505471217313001427&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=1026991&hvtargid=pla-1379061097587&psc=1) textbook by the orginal creators of the C language, Brian Kernighan and Dennis Ritchie.

### Hardware

I'll be using a [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) eval kit to build this tutorial, but you should be able to follow along with your own board and microcontroller. I'll try to clarify exactly what is specific to the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) and the [STM32G031K8T6](https://www.st.com/en/microcontrollers-microprocessors/stm32g031k8.html) MCU on it so that you can make your own modifications for your hardware.

Ultmately, any ARM based MCU should work for this tutorial. It doesn't even need to be a Cortex M0+, though I'd recommend one of those for beginners; they're the simplest modern ARM MCU. You'll just need to use slightly different compiler and linker flags than we do in this tutorial to specify your MCU's architecture. Some portions of this tutorial will be ARM specific, but you could try following along with a RISC-V MCU as well - just be prepared to do a bit of Googling on your own too.

### Software

We'll need to be able to compile and link our code for our ARM MCU target instead of our PC. This is called cross-compilation; we'll compile code for one platform (MCU) on another platform (our PCs). You might already be familiar with the GNU `gcc`, `objcopy`, `ld`, etc... tools. Luckily equivalents of those are available for building code for our embedded ARM MCU target. We'll also want to install versions of the standard C libraries that are optimized for ARM MCUs.

We'll also want to use `make` to make repeated builds at each step of this tutorial easier and repeatable. `make` automates software builds by tracking file dependencies and executing only necessary compilation steps, saving time and reducing errors. It simplifies complex build processes by using a `Makefile` to define rules, making projects easier to manage and maintain. We'll write out own `Makefile` to use throughout this tutorial momentarily.

And finally, we need a tool to flash FW into our MCU. There are a bunch of options out there for this, but I think the easiest one to start with (assuming you are also using an STM32) is the open source `stlink-tools`/`stlink` package, which includes command line utilities for working with STM32s. Another great option is `openocd`, which can be used with other MCUs as well as STM32s, and it is useful for debugging. We'll start with `stlink-tools`/`stlink` and then shift to `openocd` once we start learning about in circuit debugging on our MCU.

#### MacOS

Note that the `arm-none-eabi-gcc` homebrew package includes only the ARM compiler toolchain and not the C/C++ standard libraries. Therefore, `gcc-arm-embedded` is preferred as it includes the C/C++ standard libraries as well. Install `make` if you don't have it already, `stlink`, and `openocd` as well.

```
brew install --cask gcc-arm-embedded
brew install make stlink
```

#### Linux

Most Linux distros host the embedded toolchain and libraries that we need to install. Try the following:

```
sudo apt update
sudo apt install gcc-arm-none-eabi
```

> One thing to be aware of: this install might not include some of the C libraries that we want for some distros.

 Install `make` if you don't have it already, `stlink-tools`, and `openocd` as well. `stlink-tools` on Linux is pretty much the same think as `stlink` on Mac; the package managers used different names. `stlink-tools` might be inclusive of additional tools, but we just need `stflash`, which is included.

 ```
 sudo apt install make stlink-tools
 ```

 Check out the `stlink` [github page](https://github.com/stlink-org/stlink) if you are having issues, or if `stlink-tools` isn't availabe in your distro's package manager.

#### Windows

I'd recommend following the [instructions from ARM](https://learn.arm.com/install-guides/gcc/arm-gnu/) directly. I have not been able to test this out myself unfortunately.

I think `make` can be installed for Windows [here](https://gnuwin32.sourceforge.net/packages/make.htm), but again, I can't confirm this.

And you can follow the instructions on the `stlink` [github page](https://github.com/stlink-org/stlink) for instructions on how to install that on Windows.

## MCU Startup Overview

Before we can start writing some code, let's review how an MCU boots up. In other words, **_what happens before `main`?_**

Well, it depends on the particular MCU, but in general something like the following:

1. MCU hardware boots up: power supply(ies) finish ramping up, internal PLL(s) lock / minimum clock configuration is initialized, etc...

2. Some MCUs (including our STM32G0xx parts) have internal ROM bootloaders that they will jump to depending on external pin strapping (like the BOOT0 & BOOT1 pins on STM32 MCUs). For example, if the BOOT0 pin is high, the MCU will set the PC register to an internal ROM address corresponding to a bootloader and start executing from there.

3. But assuming a normal boot sequence to our firmware, the MCU will start executing from a specific, fixed address in flash memory. The MCU hardware will set the PC register to this fixed address upon reset. *This is where "startup" code is run*.

4. Next, the SP (stack pointer) register needs to point to the start of the stack, which is typically the end of the RAM region - the stack grows down. Some MCUs do this in HW automatically (just like they do for the PC register), but others require doing this manually with a load register instruction. Once the stack pointer is set, we can start calling functions :).

5. Then the C (& C++ if we use it) runtime environment needs to get initialized. There are a few steps to this:

    a. Copy global and static variables to RAM so they can be modified during runtime.

    b. Initialize uninitialized variables to zero. Upon reset, RAM may not be initialized to 0, so we cannot assume our uninitialized variables are default initialized to 0 or null like we expect unless we do this manually in our startup code.

    > **c. Only for C++ programs, or C programs with functions marked with `__attribute__((constructor))`**: Call `__libc_init_array`. This function calls static/global variable constructors and otherwise initializes more complex static/global variables that can't simply be initialized with assignment. How this works: linker magic...
    > * The linker collects a list of function pointers to constructors/initialization functions that need to be called. This list gets stored in a special memory section typically defined in the linkerscript as `.init_array`.
    > * The startup code loops through the function pointers in the `.init_array` section and calls each one.

6. Finally, we can call `main`!

7. For MCU embedded systems, we typically expect that the `main` function finishes initializing HW peripherals and then enters an infinite while loop, thus never returning. But just in case `main` returns for some reason, it is usually a good idea to include an infinite while loop or call some kind of hard fault handler after `main` is called.

8. This isn't strictly part of the startup process, but if we want to use the Heap and Dynamic Memory allocation, we need to implement the `_sbrk` function. This is what the C standard library uses under the hood for `malloc`, `calloc`, and `realloc`. The `_sbrk` function is one of what's typically called a system call. There are a bunch of these, which are used for standard C functions that interact with the runtime environment in some way (think `printf` and `scanf` for example, which would use stdout and stdin in a normal operating system environment, or think file operations). We'll need to define a few symbols in our linkerscript for the `_sbrk` function to use, but we won't add support for the heap and dynamic memory allocation until later in this tutorial.

## Vector Table

ARM MCUs, like the STM32G0xx series we're using, have what's called a "Vector Table" at fixed memory address (typically the start of flash memory). The Vector Table is essentially a look-up table of addresses in memory, mostly for event/interrupt handler functions. The first entry in the vector table is the address that the stack pointer should be initialized to, and the second entry is the address of the reset handler. This is the code that the MCU starts executing immediately after power-on or reset. It typically initializes the system and sets up the MCU for operation by doing the things listed above. Basically, the second entry in the vector table should point to the startup code. The first 16 entries are reserved by ARM and are common for all ARM MCUs; the remainder of the vector table entries are for interrupt/event handlers that are specific to a particular MCU.

## Linkerscript

Before we can compile our firmware into a final binary for the STM32G0 MCU, we'll need to define a linkerscript that tells the linker where to put various things in memory. We'll write and then use the same linkerscript from scratch for all of these tutorials, so let's create a file `link.ld` in the tutorial top level directory. Now let's get into the linkerscript implementation...

Keep the STM32G031xx [datasheet](https://www.st.com/resource/en/datasheet/stm32g031c6.pdf) and [reference manual](https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) handy for this section. Feel free to reference [this template linkerscript](https://github.com/STMicroelectronics/STM32CubeG0/blob/master/Projects/NUCLEO-G031K8/Templates/STM32CubeIDE/STM32G031K8TX_FLASH.ld) from ST as well.

The linkerscript tells the linker how to arrange everything in memory in the final binary. Therefore, the linkerscript must first describe the flash and RAM memory partitions of the target MCU. From pg. 61 of the [reference manual](https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf):

![STM32G031xx Memory Map](assets/stm32g031_memory_map.png)

Flash memory starts at 0x08000000 and is 64KB long. RAM starts at 0x20000000 and is 8KB long. Specify this in `link.ld` with the following:

```
MEMORY {
    FLASH (RX) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (RWX) : ORIGIN = 0x20000000, LENGTH = 8K
}
```

> **NOTE:** If you are following along with a different MCU, all you need to do is modify these section definitions to match your MCU.

Next, specify the entry point function for the firmware. This is ultimately where our startup code is implemented. We'll get to this in a moment. Call it `ResetHandler` or something similar. We need a symbol for the initial stack pointer that we can put in the vector table. Define that as well; I named it like a function since we'll use it as a function pointer in the vector table.

```
ENTRY(ResetHandler)
InitialStackPtr = ORIGIN(RAM) + LENGTH(RAM);
```

We also should define a max heap size symbol for use in the `_sbrk` syscall stub for memory allocation.

```
max_heap_size = 0x200;
```

Now the linkerscript just needs to outline how each section should be placed in memory. These are the minimum set of sections that need to be placed:
* `.vector_table`:
    * This is the Vector Table describer earlier. It's basically a look up take for interrupt/event handlers, starting with the ResetHandler (our startup code). It also stores the initial stack, which is actually placed first in the LUT (then the ResetHandler and all other handler function pointers follow). The Vector Table must be placed right at the start of flash memory (you can technically move the vector table, but you must remap each entry accordingly - it's good practice to avoid doing this unless absolutely necessary). We explicitly tell the linker not to rearrange or move the vector table with the `KEEP` macro. Also, we can name this section whatever we'd like, so long as it matches our startup code; I chose "vector_table", but the standard CMSIS template from ST uses "isr_vectors".
* `.text`:
    * This section contains our program code. We place this section in flash memory.
* `.rodata`:
    * This section contains read only data like constants or string literals. It gets placed in flash memory as well, typically right after the `.text` section (and some linker scripts place it within the `.text` section).
* `.data`:
    * This section contains our initialized variables that need to be copied into RAM by our startup code.

> * `.init_array` and similar: In C programs with functions marked as `__attribute__((constructor))`, or C++ programs with classes, there might be sections called `.preinit_array`, `.init_array`, and `.fini_array`. These sections are used to provide a well-defined order of initialization of global/static variables that might need to have constructors called during startup. This is handled in our startup code by calling `__libc_init_array` as discussed earlier. To keep things simple, we will not include these sections in the linkerscript and therefore will avoid using constructors.

* `.bss`:
    * This section contains our *un*initialized (and zero initialized) variables that need to be placed into RAM and *zero initialized* by our startup code.

* `.heap`:
    * This section represents where the heap would start. We use it to store a `heap_start` symbol for our `_sbrk` system call implementation. That's technically all we need. We align `heap_start` to an 8 byte value because some types might have strict alignment requirements and could be larger than a single word in memory (I believe this is true for doubles, for example).

These are just the bare-minimum set of sections. More complicated systems might have separate sections for an A/B firmware bootloader, with firmware partiion A and firmware partition B, for example.

Here's what the sections look like in our `link.ld`:

```
SECTIONS {
    .vector_table : {
        . = ALIGN(4);
        KEEP(*(.vector_table))
        . = ALIGN(4);
    } > FLASH

    .text : {
        . = ALIGN(4);
        *(.text*)
        . = ALIGN(4);
    } > FLASH

    .rodata : {
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* We aren't supporting C/C++ constructors or anything that would require .init_data */
    /* (and similar) sections, but if we were, they would go here */

    flash_data_start = LOADADDR(.data);
    .data : {
        . = ALIGN(4);
        ram_data_start = .;
        *(.data*)
        . = ALIGN(4);
        ram_data_end = .;
    } > RAM AT > FLASH

    .bss : {
        . = ALIGN(4);
        bss_start = .;
        *(.bss*)
        . = ALIGN(4);
        bss_end = .;
    } > RAM

    .heap : {
        . = ALIGN(8);
        heap_start = .;
    } > RAM
}
```

A few notes:
* We use `ALIGN(4)` at the beginning and end of each section to ensure each section is aligned to a word boundary. It isn't strictly necessary to specify this alignment, since the vector table, for example, is already placed at a word aligned address, but it's good practice to specify alignment explicitly.
* We define symbols that our startup code can use to copy initialized data to RAM, and zero initialize the bss section in RAM as well. We need: `flash_data_start`, `ram_data_start`, `ram_data_end`, `bss_start`, and `bss_end`.

## Startup Code

## Makefile

It might be helpful to keep the official `make` [documentation](https://www.gnu.org/software/make/) on hand.

## Heap and Dynamic Memory Allocation

## LED Blink

## Flashing FW

## SysTick

## Clock Configuration

## Timers

## UART

## Printf

## Debugging