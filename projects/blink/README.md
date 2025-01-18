# Blink Demo

This is just a simple blink LED demo for the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) development board to demonstrate the basics of startup code and a linkerscript from scratch.

## MCU Start Up Process

**What happens before `main`?**

Well, it depends on the particular MCU, but in general something like the following:

1. MCU hardware boots up: power supply(ies) finish ramping up, internal PLL(s) lock / minimum clock configuration is initialized, etc...

2. Some MCUs (including our STM32G0xx parts) have internal ROM bootloaders that they will jump to depending on external pin strapping (like the BOOT0 & BOOT1 pins on STM32 MCUs). For example, if the BOOT0 pin is high, the MCU will set the PC register to an internal ROM address corresponding to a bootloader and start executing from there.

3. But assuming a normal boot sequence to our FW, the MCU will start executing from a specific, fixed address in flash memory. The MCU hardware will set the PC register to this fixed address upon reset. *This is where "startup" code is run*.

4. Next, the SP (stack pointer) register needs to point to the start of the stack, which is typically the end of the RAM region - the stack grows down. Some MCUs do this in HW automatically (just like they do for the PC register), but others require doing this manually with a load register instruction. Once the stack pointer is set, we can start calling functions :).

5. Then the C (& C++ if we use it) runtime environment needs to get initialized. There are a few steps to this:

    a. Copy global and static variables to RAM so they can be modified during runtime.

    b. Initialize uninitialized variables to zero. Upon reset, RAM may not be initialized to 0, so we cannot assume our uninitialized variables are default initialized to 0 or null like we expect unless we do this manually in our startup code.

    > **c. Only for C++ programs, or C programs with functions marked with `__attribute__((constructor))`**: Call `__libc_init_array`. This function calls static/global variable constructors and otherwise initializes more complex static/global variables that can't simply be initialized with assignment. How this works: linker magic...
    > * The linker collects a list of function pointers to constructors/initialization functions that need to be called. This list gets stored in a special memory section typically defined in the linkerscript as `.init_array`.
    > * The startup code loops through the function pointers in the `.init_array` section and calls each one.

6. Finally, we can call `main`!

7. For MCU embedded systems, we typically expect that the `main` function finishes initializing HW peripherals and then enters an infinite while loop, thus never returning. But just in case `main` returns for some reason, it is usually a good idea to include an infinite while loop or call some kind of hard fault handler after `main` is called.

## Vector Table

ARM MCUs, like the STM32G0xx series, have what's called a "Vector Table" at fixed memory address (typically the start of flash memory). The Vector Table is essentially a look-up table of addresses in memory, mostly for event/interrupt handler functions. The first entry in the vector table is the address that the stack pointer should be initialized to, and the second entry is the address of the reset handler. This is the code that the MCU starts executing immediately after power-on or reset. It typically initializes the system and sets up the MCU for operation by doing the things listed above. Basically, the second entry in the vector table should point to the startup code. The first 16 entries are reserved by ARM and are common for all ARM MCUs; the remainder of the vector table entries are for interrupt/event handlers that are specific to a particular MCU.

## References

The official "template" startup code from ST/ARM for gcc tools, [startup_stm32g031xx.s](https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Source/Templates/gcc/startup_stm32g031xx.s) contains everything discussed above. See that as a more official/polished example. You can also find an example linkerscript from ST [here](https://github.com/STMicroelectronics/STM32CubeG0/blob/master/Projects/NUCLEO-G031K8/Templates/STM32CubeIDE/STM32G031K8TX_FLASH.ld). Note that the [startup code](https://github.com/STMicroelectronics/STM32CubeG0/blob/master/Projects/NUCLEO-G031K8/Templates/STM32CubeIDE/Application/Startup/startup_stm32g031k8tx.s) is also the same in this ST repository.

## Blink From Scratch

Ok, now that we've summarized what we need to do in startup code before `main` behind the scenes, let's write a basic LED blink demo for the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) in a single .c file from scratch. We'll also need a linkerscript .ld file, and a Makefile for easy and repeatable builds.

### Install Toolchain

First, follow the prerequisites section in this repository's top level [README.md](../../README.md). You can skip the part about installing `bazelisk` / `bazel`. We'll just use a Makefile for this project.

### Makefile

Before writing any code, let's start with a super basic `Makefile`. Specify the GCC compiler and linker, then list command line flags for each of them.

```
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-ld
```

At a minimum, the compiler needs to know the target cpu architecture. We specify this with `-mcpu=cortex-m0plus`. We also tell the compiler it can use ARM's smaller 16bit "Thumb" instructions were possible with `-mthumb` - this is common practice for resource constrained embedded systems, using thumb instructions reduces code size. Next, tell the compiler to include debugging information and *not* do any optimizations with `-g` and `-O0` respectively - this will make it easier to see what is going on under the hood after the program is compiled. Finally, enable compiler warnings with `-Wall` and `-Wextra`.

```
CFLAGS = -mcpu=cortex-m0plus -mthumb
CFLAGS += -g -O0
CFLAGS += -Wall -Wextra
```

The linker flags are a bit more interesting. First, we want to tell the linker *not* to include the standard startup code provided by the toolchain using the `-nostartfiles` flag. We want to write our own (plus, the standard startup code usually won't fit your MCU as some details (like the initial stack pointer) vary from one MCU to another).

We also tell the linker not to link against any of the standard C libraries with the `-nostdlib` flag. This is an even broader directive than `-nolibc` which is also commonly used. `-nostdlib` also excludes libraries like  `libgcc` (the GCC support library) or `libm` (the math library). We exclude these standard libraries because they are often unneeded, or larger than we can really afford for our resource constrained microcontroller-based embedded system. Instead, we allow the linker to link against a smaller and more efficient set of libraries with the `--specs=nano.specs` flag. Specifically, this tells the linker it can use the [newlib](https://en.wikipedia.org/wiki/Newlib) library. We also tell the linker to replace the standard C library functions that rely on OS syscalls with stubs using the `--specs=nosys.specs` flag. This declares a set of function stubs ("syscalls" like `_close` or `_sbrk`) that we can define if we need to emulate the behavior of a system call (like `printf`). We also add `-lc` and `-lgcc` flags to allow linking against just the bare minimum set of C standard functions like `strcmp`. While `-nostdlib` and `-lc`/`-lgcc` might seem contradictory, they can be used together to achieve fine-grained control over what parts of the standard library are available to the linker.

Altogether, the `-nostdlib`, `--specs=nano.specs`, `--specs=nosys.specs`, `-lc`, and `-lgcc` linker directives ensure we have a minimal C runtime environment for our STM32G0xx projects, but allow us to use functions like `strcmp` or override `printf` if we want to.

Finally, we add `-Wl,--gc-section` to the linker flags to tell the linker to remove any unused code or data after everything is linked together (sort of like garbage collection at the end of linking).

```
LDFLAGS = -nostartfiles
LDFLAGS += -nostdlib --specs=nano.specs --specs=nosys.specs -lc -lgcc
LDFLAGS += -Wl,--gc-section
```

Great! Let's get the linkerscript in order next...

### Linkerscript

Keep the STM32G031xx [datasheet](https://www.st.com/resource/en/datasheet/stm32g031c6.pdf) and [reference manual](https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) handy for this section. Feel free to reference [this template linkerscript](https://github.com/STMicroelectronics/STM32CubeG0/blob/master/Projects/NUCLEO-G031K8/Templates/STM32CubeIDE/STM32G031K8TX_FLASH.ld) from ST as well.

The linkerscript tells the linker how to arrange everything in memory in the final binary. Therefore, the linkerscript must first describe the flash and RAM memory partitions of the target MCU. From pg. 61 of the [reference manual](https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf):

![STM32G031xx Memory Map](../../assets/stm32g031_memory_map.png)

Flash memory starts at 0x08000000 and is 64KB long. RAM starts at 0x20000000 and is 8KB long. Specify this in `link.ld` with the following:

```
MEMORY {
    FLASH (RX) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (RWX) : ORIGIN = 0x20000000, LENGTH = 8K
}
```

Next, specify the entry point function for the firmware. This is ultimately where our startup code - which sets the stack pointer, copies global/static variables from flash to RAM, etc... - is implemented. We'll get to this in a moment. For now, call it `ResetHandler` or something similar.

It's also convenient to calculate and store the desired initial stack pointer value in the linkerscript so that our startup code can use it without needing to know the final RAM address. Again, this will be more clear when we get to the startup code.

```
ENTRY(ResetHandler)
initial_stack_pointer = ORIGIN(RAM) + LENGTH(RAM);
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

These are just the bare-minimum set of sections. More complicated systems might have separate sections for an A/B FW bootloader, with FW partiion A and FW partition B, for example.

### Build & Flash

Use the `Makefile` to build, flash, or clean the FW:

```
make clean
make flash
```

You should see the onboard LED blinking.
