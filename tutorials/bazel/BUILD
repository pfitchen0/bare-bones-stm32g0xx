# Load the stm32g0xx binary and library rules.
load("//:rules.bzl", "stm32g0xx_binary", "stm32g0xx_library")

package(
    default_visibility = ["//visibility:public"]
)

# Build a binary like so:
stm32g0xx_binary(
    name = "main",
    srcs = ["main.c"],
    # hdrs = [],  # Could specify headers, if there were any.
    deps = ["//tutorials/bazel:tiny_hal"],  # This binary can depend on any number of libraries.
    ldscript = "link.ld",
)

# Build a library like so:
stm32g0xx_library(
    name = "tiny_hal",
    srcs = ["tiny_hal.c"],
    hdrs = ["tiny_hal.h"],
    # deps = [],  # And this library could depend on other libraries!
)
