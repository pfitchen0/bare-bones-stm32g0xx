# Load the stm32g0xx binary and library rules.
load("//:rules.bzl", "stm32g0xx_binary", "stm32g0xx_library")

package(
    default_visibility = ["//visibility:public"]
)

exports_files(["system.c", "stm32g031x8xx.ld"])

stm32g0xx_library(
    name = "gpio",
    srcs = ["gpio.c"],
    hdrs = ["gpio.h"],
    deps = [":rcc", ":macros"],
)

stm32g0xx_library(
    name = "rcc",
    hdrs = ["rcc.h"],
)

stm32g0xx_library(
    name = "macros",
    hdrs = ["macros.h"],
)
