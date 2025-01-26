# Bare-Bones-STM32G0xx

This is a "bare-bones" STM32G0xx project repository. I plan to build out a set of libraries (startup, HAL, common data structures, etc...), build systems (Make, Bazel, ... maybe CMake), tutorials (progressively build on a blinky LED example), and more complex example projects (HW & FW, TBD).

For now, there are two tutorials to start: [`blink`](tutorials/blink/README.md) and [`systick`](tutorials/systick/README.md).

## Prerequisites

Install the ARM compiler toolchain and standard libraries. Note that the `arm-none-eabi-gcc` homebrew package includes only the ARM compiler toolchain and not the C/C++ standard libraries. Therefore, `gcc-arm-embedded` is preferred as it includes the C/C++ standard libraries as well.

```
brew install --cask gcc-arm-embedded
```

Install `make` if you don't have it already. Some of the simpler projects just use `make` (like the [blink](tutorials/blink/README.md) example project for understanding MCU startup code and linkerscripts).

```
brew install make
```

> Note: More complex projects use a `bazel` based build system instead of `make`. Install `bazelisk` for that. These projects haven't been publicly added yet though :)
> ```
> brew install bazelisk
> ```

Install open source ST-Link drivers for flashing FW to your board.

```
brew install stlink
```

Order (or build!) an STM32G0xx board like the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) development board.
