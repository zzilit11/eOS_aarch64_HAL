/*
    Linker script
    - What sections go where in memory
    - Memory mapping
    - Entry point 지정
*/

/* ARM Cortex-A76 */

/* Entry point 지정 */
ENTRY(_start)

/* Memory mapping */
SECTIONS {
    . = 0x40080000; /* Start address of RAM memory */

    .text : {
        KEEP(*(.text.boot))
        *(.text*)
    }
    
    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    /* Bss section */
    /* BSS 8바이트 정렬: entry.S의 8바이트 클리어 루프와 일치 */
    . = ALIGN(8);
    __bss_start = .;
    .bss : {
        *(.bss*)
        *(COMMON)
    }
    . = ALIGN(8);
    __bss_end = .;


    /* HW exception vector table (2KiB 정렬 필수) */
    . = ALIGN(2048);
    __vectors_start = .;
    .vectors : {
        KEEP(*(.vectors))
    }
    __vectors_end = .;


    /* Stack section */
    /* AArch64 ABI requires 16-byte stack alignment */
    . = ALIGN(16);
    __stack_bottom = .;
    . += 0x4000;
    __stack_top = .;  /* Stack top address (16KB stack) */
}
