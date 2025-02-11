# Load the stm32g0xx binary and library rules.
load("//:rules.bzl", "stm32g0xx_binary", "stm32g0xx_library")

package(
    default_visibility = ["//visibility:public"]
)

stm32g0xx_library(
    name = "gpio",
    srcs = ["gpio.c"],
    hdrs = ["gpio.h", "macros.h"]
)
