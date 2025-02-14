load("//:rules.bzl", "stm32g0xx_binary", "stm32g0xx_library")

package(
    default_visibility = ["//visibility:public"]
)

stm32g0xx_binary(
    name = "hal_demo",
    srcs = [
        "main.c",
        "//hal:system.c",
    ],
    deps = ["//hal:gpio"],
    ldscript = "//hal:stm32g031x8xx.ld",
)
