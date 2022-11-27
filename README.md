# STM32L1 Application

Optimized for STM32L152RCT6

## STM32L1-Discovery-baremetal-toolchain
STM32L1 Discovery baremetal toolchain using `arm-none-eabi-` GCC tools and `OpenOCD` for flashing

## Supported peripherals
Partially supported:
- LCD
- GPIO
- DMA
- Clocks and power

## Building and running
See commands in `Makefile`.

For developing, following packages needed at Fedora 36:
```
dnf install openocd arm-none-eabi-gcc-cs-c++ clang-tools-extra
```

