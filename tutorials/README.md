# Building Bare-Bones-STM32G0xx From Scratch

*"If you can't explain it simply, you don't understand it well enough"* -Albert Einstein (I think).

This tutorial is my attempt to understand microcontroller-based embedded systems programming more deeply by explaining them here, simply.

## Overview

In this tutorial we'll build up some example STM32G0xx firmware ***completely from scratch***. We'll start by discussing how MCUs boot, writing startup code, and setting up a minimal C runtime environment. After that we'll be able to boot into a `main` function and write/run C programs that might look more familiar to someone who hasn't gone so low level before. We'll continue to build a few simple examples to demonstrate key MCU fundamentals like memory-mapped peripherals, interrupts, and debugging.

Here's a rough table of contents:

0. [Prerequisites](#prerequisites)
1. [MCU Startup Overview](#mcu-startup-overview)
2. [Makefile](#makefile)
3. [Linkerscript](#linkerscript)
4. [Startup Code](#startup-code)
5. [Heap and Dynamic Memory Allocation](#heap-and-dynamic-memory-allocation)
6. [LED Blink](#led-blink)
7. [Flashing FW](#flashing-fw)
8. [SysTick](#systick)
9. [Clock Configuration](#clock-configuration)
10. [Timers](#timers)
11. [UART](#uart)
12. [Printf](#printf)
13. [Debugging](#debugging)

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

## Makefile

It might be helpful to keep the official `make` [documentation](https://www.gnu.org/software/make/) on hand.

## Linkerscript

## Startup Code

## Heap and Dynamic Memory Allocation

## LED Blink

## Flashing FW

## SysTick

## Clock Configuration

## Timers

## UART

## Printf

## Debugging