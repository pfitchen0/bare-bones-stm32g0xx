PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-ld

# Specify the target MCU architecture and allow the compiler to use ARM's "thumb" instructions,
# which is basically a set of 16bit instructions that can be used to help reduce code size in
# memory constrained embedded systems.
CFLAGS = -mcpu=cortex-m0plus -mthumb

# Include debugging info in the output file and turn off compiler optimizations (for now).
CFLAGS += -g -O0

# Enable compiler warnings.
CFLAGS += -Wall -Wextra

SOURCES = blink.c
LDSCRIPT = link.ld
OBJECTS = blink.o
ELF = blink.elf

all: $(ELF)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf $(OBJECTS) $(ELF)

flash: $(ELF)
	st-flash --reset write $< 0x8000000

.PHONY: all clean flash
