# Bare-Bones-STM32G0xx

This is a "bare-bones" STM32G0xx project repository. I plan to build out a set of libraries (startup, HAL, common data structures, etc...), build systems (Make, Bazel, ... maybe CMake), tutorials (progressively build on a blinky LED example), and more complex example projects (HW & FW, TBD).

For now, there are only a few tutorials. Start with: [`blink`](tutorials/blink/README.md) and [`systick`](tutorials/systick/README.md) to get familiar with the startup process, linkerscripts, makefiles, the vector table, and bare-metal register poking. Then check out the series of timer tutorials, which progressively introduce topics like the NVIC controller, GPIO alt modes, and - of course - timers.

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

## Bazel Toolchain

This repo includes a minimalist Bazel based toolchain for building more complex STM32G0xx FW binaries and libraries. All of the Bazel rules are placed in a single `rules.bzl` file with no external dependencies.

I did hardcode compiler flags to match the STM32G0xx family, but it should be easy to port this example to other embedded/MCU targets though! Just change the compiler flags and linkerscript. Or, improve these Bazel rules to support multiple MCU architectures/targets.

Bazel rules and Starlark code take heavy inspiration from Jay Conrod's [fantastic blog](https://jayconrod.com/posts/106/writing-bazel-rules--simple-binary-rule) and [YouTube talk](https://youtu.be/2KUunGBZiiM?si=fHOEdGWAu-3cPlai), but for embedded C projects instead of go.

There are some **caveats**:

1. Include directories are still passed in the with _C_FLAGS variable
 in the rules.bzl file. Prefer specifying absolute paths from the workspace top when
 including header files - this is the preferred bazel design pattern anyways.
2. Only `.c` and `.s` source files are supported. Can't use C++ or compile/link with
 external libraries (like a `.so`) (*yet*).

## TODO

Only .c and .s files are accepted as sources at this time - expand that to include .cc/.cpp, .h, etc...

Toolchain is tested on MacOS so far - make sure it works on Windows and Linux too.