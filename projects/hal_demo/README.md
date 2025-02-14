# HAL Demo

This project is a simple example of how to use the [hal](../../hal/) libraries on a [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) board.

## Build and Run

You can build the code like so:

```
bazel build projects/hal_demo:hal_demo
```

And flash it to the [Nucleo-G031K8](https://www.digikey.com/en/products/detail/stmicroelectronics/NUCLEO-G031K8/10321671) board:

```
st-flash --reset write bazel-bin/projects/hal_demo/hal_demo.bin 0x8000000
```