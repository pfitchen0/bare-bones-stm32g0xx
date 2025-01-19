PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc

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

# Linker flags for minimal runtime environment. Exclude all standard libraries for full control.
# Use the much smaller newlib C runtime optimized for size and performance on microcontrollers.
# Replace OS-dependent syscalls with stubs or minimal implementations. Allow linking against
# specific parts of libc and libgcc if needed (e.g., for custom implementation of printf).
LDFLAGS += -nostdlib --specs=nano.specs --specs=nosys.specs -lc -lgcc

# Remove unused data and code after everything is linked together to reduce binary size.
LDFLAGS += -Wl,--gc-section

SOURCES = blink.c
LDSCRIPT = link.ld
ELF = blink.elf

all: $(ELF)

$(ELF): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -T ${LDSCRIPT} $(SOURCES) -o $@

clean:
	rm -rf $(ELF)

flash: $(ELF)
	st-flash --reset write $< 0x8000000

.PHONY: all clean flash
