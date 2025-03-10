# Toolchain definitions.
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
OBJCOPY = $(PREFIX)-objcopy

# Specify the target MCU architecture and allow the compiler to use ARM's "thumb" instructions,
# which is basically a set of 16bit instructions that can be used to help reduce code size in
# memory constrained embedded systems.
CFLAGS = -mcpu=cortex-m0plus -mthumb

# Include debugging info in the output file and turn off compiler optimizations (for now).
CFLAGS += -g -O0

# Enable compiler warnings.
CFLAGS += -Wall -Wextra

# Do not try to use the standard startup code included in the arm-none-eabi toolchain. Use our own
# startup code. Plus, the default startup code doesn't handle MCU-specific things like setting the
# proper initial stack pointer address.
LDFLAGS = -nostartfiles

# Linker flags for minimal runtime environment.
# Use the much smaller newlib C runtime optimized for size and performance on microcontrollers.
# Replace OS-dependent syscalls with stubs or minimal implementations. Allow linking against
# specific parts of libc and libgcc if needed (e.g., for custom implementation of printf).
LDFLAGS += --specs=nano.specs --specs=nosys.specs -lc -lgcc

# Remove unused data and code after everything is linked together to reduce binary size.
LDFLAGS += -Wl,--gc-sections

# Print firmware image size after linking.
LDFLAGS += -Wl,--print-memory-usage

# Output files.
ELF = firmware.elf
BIN = firmware.bin

# Compile and link sources into a .elf.
$(ELF): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -T ${LDSCRIPT} $(SOURCES) -o $@

# Convert the .elf to a simple binary .bin for flashing.
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Remove intermediate and output build files for a fresh build.
clean:
	rm -rf $(ELF) $(BIN)

# Flash the final .bin to the STM32G031 Nucleo board via the onboard ST-Link programmer.
flash: $(BIN)
	st-flash --reset write $< 0x8000000

all: $(ELF)

.PHONY: all clean flash
