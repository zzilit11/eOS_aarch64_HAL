.text
.align 2

.equ CTX_SIZE, 272
.equ CTX_OFF_SP, 248
.equ CTX_OFF_ELR, 256
.equ CTX_OFF_SPSR, 264

.macro SAVE_PAIR reg1, reg2, slot
    stp     \reg1, \reg2, [sp, #((\slot) * 16)]
.endm

.macro LOAD_PAIR reg1, reg2, slot
    ldp     \reg1, \reg2, [x0, #((\slot) * 16)]
.endm

// Save caller context into a stack frame and return its base in x0
.global _os_save_context
.type _os_save_context, %function
_os_save_context:
    // Allocate frame
    sub     sp, sp, #CTX_SIZE           // 커널 전용 stack 사용 (일반적 설정)

    // Save GPRs x0-x30 in ABI order
    SAVE_PAIR x0,  x1,  0
    SAVE_PAIR x2,  x3,  1
    SAVE_PAIR x4,  x5,  2
    SAVE_PAIR x6,  x7,  3
    SAVE_PAIR x8,  x9,  4
    SAVE_PAIR x10, x11, 5
    SAVE_PAIR x12, x13, 6
    SAVE_PAIR x14, x15, 7
    SAVE_PAIR x16, x17, 8
    SAVE_PAIR x18, x19, 9
    SAVE_PAIR x20, x21, 10
    SAVE_PAIR x22, x23, 11
    SAVE_PAIR x24, x25, 12
    SAVE_PAIR x26, x27, 13
    SAVE_PAIR x28, x29, 14
    str     x30,      [sp, #(15 * 16)]    // LR

    // Save original SP and exception context
    add     x1, sp, #CTX_SIZE            // x1 = original SP before sub
    str     x1,      [sp, #CTX_OFF_SP]
    mrs     x1, ELR_EL1
    str     x1,      [sp, #CTX_OFF_ELR]
    mrs     x1, SPSR_EL1
    str     x1,      [sp, #CTX_OFF_SPSR]

    // Return context base in x0
    mov     x0, sp
    ret
.size _os_save_context, . - _os_save_context

// Restore from context pointer in x0 and eret
.global _os_restore_and_eret
.type _os_restore_and_eret, %function
_os_restore_and_eret:
    // Restore system registers first (x0 holds context pointer)
    ldr     x1, [x0, #CTX_OFF_ELR]
    ldr     x2, [x0, #CTX_OFF_SPSR]
    msr     ELR_EL1, x1
    msr     SPSR_EL1, x2

    // Restore general-purpose registers except x0/x1 (used as base)
    LOAD_PAIR x2,  x3,  1
    LOAD_PAIR x4,  x5,  2
    LOAD_PAIR x6,  x7,  3
    LOAD_PAIR x8,  x9,  4
    LOAD_PAIR x10, x11, 5
    LOAD_PAIR x12, x13, 6
    LOAD_PAIR x14, x15, 7
    LOAD_PAIR x16, x17, 8
    LOAD_PAIR x18, x19, 9
    LOAD_PAIR x20, x21, 10
    LOAD_PAIR x22, x23, 11
    LOAD_PAIR x24, x25, 12
    LOAD_PAIR x26, x27, 13
    LOAD_PAIR x28, x29, 14
    ldr     x30,      [x0, #(15 * 16)]

    // Load stack pointer to be restored after fetching x0/x1
    ldr     x3, [x0, #CTX_OFF_SP]

    // Finally restore x1 and x0 (preserving context pointer until done)
    ldr     x1, [x0, #8]
    ldr     x0, [x0, #0]

    // Switch to task stack and return from exception level 1
    mov     sp, x3
    isb
    eret
.size _os_restore_and_eret, . - _os_restore_and_eret
