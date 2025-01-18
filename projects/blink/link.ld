/* Specify memory partitions from pg 61 of the reference manual */
MEMORY {
    FLASH (RX) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM (RWX) : ORIGIN = 0x20000000, LENGTH = 8K
}

/* Specify the entry point function, which is defined in our startup code */
ENTRY(ResetHandler)

/* Store the desired initial stack pointer in a variable for our startup code to use */
/* Pick the end of the RAM region - the stack grows down! */
initial_sp = ORIGIN(RAM) + LENGTH(RAM);