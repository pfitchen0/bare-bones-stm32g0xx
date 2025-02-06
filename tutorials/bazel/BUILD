# Load the stm32g0xx binary and library rules.
load("//:rules.bzl", "stm32g0xx_binary", "stm32g0xx_library")

# Build a binary like so:
stm32g0xx_binary(
    name = "main",
    srcs = ["main.c"],
    # hdrs = [],  # Could specify headers, if there were any.
    # deps = [":hal"],  # This binary can depend on any number of libraries.
    ldscript = "link.ld",
)
