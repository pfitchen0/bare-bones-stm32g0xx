# Building Bare-Bones-STM32G0xx From Scratch

*"If you can't explain it simply, you don't understand it well enough"* -Albert Einstein (I think).

This tutorial is my attempt to understand microcontroller-based embedded systems programming more deeply by explaining it here, simply.

## Overview

In this tutorial we'll build up some example STM32G0xx firmware completely from scratch. We'll start by discussing how MCUs boot, writing startup code, and setting up a minimal C runtime environment. After that we'll be able to boot into a `main` function and write/run C programs that might look more familiar to someone who hasn't gone so low level before. We'll continue to build a few simple examples to demonstrate key MCU fundamentals like memory-mapped peripherals, interrupts, and debugging.

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
9. [Timers](#timers)
10. [UART](#uart)
11. [Printf](#printf)
12. [Debugging](#debugging)

## Prerequisites

## MCU Startup Overview

## Makefile

## Linkerscript

## Startup Code

## Heap and Dynamic Memory Allocation

## LED Blink

## Flashing FW

## SysTick

## Timers

## UART

## Printf

## Debugging