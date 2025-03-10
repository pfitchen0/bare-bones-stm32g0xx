/* Specify memory partitions from pg 61 of the reference manual */
/* Modify these values for your STM32G0xx MCU */
MEMORY {
    FLASH (RX) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (RWX) : ORIGIN = 0x20000000, LENGTH = 8K
}

/* Specify the entry point function, which is defined in our startup code */
ENTRY(ResetHandler)

/* Store the desired initial stack pointer in a symbol for our startup code to use */
/* Pick the end of the RAM region - the stack grows down! */
initial_stack_ptr = ORIGIN(RAM) + LENGTH(RAM);
InitialStackPtr = ORIGIN(RAM) + LENGTH(RAM);

/* Specify minimum sizes for the heap and stack. */
min_heap_size = 0x200;
min_stack_size = 0x400;

/* Specify how sections are arranged in memory, see README for more info */
/* Follow best practice of explicitly word aligning the beginning and end of each section, even */
/* when it is redudant */
SECTIONS {
    /* Vector table first! */
    .vector_table : {
        . = ALIGN(4);
        /* Use KEEP to tell the linker to keep the vector_table, even if it appears unused. */
        KEEP(*(.vector_table))
        . = ALIGN(4);
    } > FLASH

    /* Program code next, making sure we are word aligned */
    .text : {
        . = ALIGN(4);
        *(.text*)
        . = ALIGN(4);
    } > FLASH

    /* Read-only data (constants, string literals, etc...) next, again word aligned */
    .rodata : {
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    
    /* We aren't supporting C/C++ constructors or anything that would require .init_array */
    /* (and similar) sections, but if we were, they would go here */

    /* Initialized data next */
    /* Specify that this section should be placed in FLASH, and copied to RAM: "> RAM AT >FLASH" */
    /* Also save start and end symbols for use in our startup code. We need a start address in */
    /* FLASH and a start address in RAM to copy data from one to the other. */
    flash_data_start = LOADADDR(.data);
    .data : {
        . = ALIGN(4);
        ram_data_start = .;
        *(.data*)
        . = ALIGN(4);
        ram_data_end = .;
    } > RAM AT > FLASH

    /* Finally, uninitialized (aka zero-initialized) data */
    /* This gets placed in RAM only, and our startup code needs start and end symbols to loop */
    /* through this region and zero it out. */
    .bss : {
        . = ALIGN(4);
        bss_start = .;
        *(.bss*)
        . = ALIGN(4);
        bss_end = .;
    } > RAM

    /* Heap "section". Check that there is enough space for our min heap and stack sizes, */
    /* and save heap_start symbol. Linker will complain if this section doesn't fit, thus */
    /* warning us that there is not enough RAM for our minimum stack & heap sizes. */
    .heap : {
        /* Align to 8 bytes instead of just 4 (one word) */
        /* Some datatypes (i.e. double) might have more strict alignment requirements */
        . = ALIGN(8);
        heap_start = .;
        . = . + min_heap_size;
        . = . + min_stack_size;
        . = ALIGN(8);
    } > RAM
}
