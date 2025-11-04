.text
.align 2

.equ CTX_SIZE, 272
.equ CTX_OFF_SP, 248
.equ CTX_OFF_ELR, 256
.equ CTX_OFF_SPSR, 264

// Save caller context into a stack frame and return its base in x0
.global _os_save_context
.type _os_save_context, %function
_os_save_context:
    // Allocate frame
    sub     sp, sp, #CTX_SIZE           // 커널 전용 stack 사용 (일반적 설정)

    // Save GPRs x0-x30 in ABI order
    stp     x0,  x1,  [sp, #(0 * 16)]
    stp     x2,  x3,  [sp, #(1 * 16)]
    stp     x4,  x5,  [sp, #(2 * 16)]
    stp     x6,  x7,  [sp, #(3 * 16)]
    stp     x8,  x9,  [sp, #(4 * 16)]
    stp     x10, x11, [sp, #(5 * 16)]
    stp     x12, x13, [sp, #(6 * 16)]
    stp     x14, x15, [sp, #(7 * 16)]
    stp     x16, x17, [sp, #(8 * 16)]
    stp     x18, x19, [sp, #(9 * 16)]
    stp     x20, x21, [sp, #(10 * 16)]
    stp     x22, x23, [sp, #(11 * 16)]
    stp     x24, x25, [sp, #(12 * 16)]
    stp     x26, x27, [sp, #(13 * 16)]
    stp     x28, x29, [sp, #(14 * 16)]
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
    ldp     x2,  x3,  [x0, #(1 * 16)]
    ldp     x4,  x5,  [x0, #(2 * 16)]
    ldp     x6,  x7,  [x0, #(3 * 16)]
    ldp     x8,  x9,  [x0, #(4 * 16)]
    ldp     x10, x11, [x0, #(5 * 16)]
    ldp     x12, x13, [x0, #(6 * 16)]
    ldp     x14, x15, [x0, #(7 * 16)]
    ldp     x16, x17, [x0, #(8 * 16)]
    ldp     x18, x19, [x0, #(9 * 16)]
    ldp     x20, x21, [x0, #(10 * 16)]
    ldp     x22, x23, [x0, #(11 * 16)]
    ldp     x24, x25, [x0, #(12 * 16)]
    ldp     x26, x27, [x0, #(13 * 16)]
    ldp     x28, x29, [x0, #(14 * 16)]
    ldr     x30,      [x0, #(15 * 16)]

    // Load stack pointer to be restored after fetching x0/x1
    ldr     x3, [x0, #CTX_OFF_SP]

    // Finally restore x1 and x0 (preserving context pointer until done)
    ldr     x1, [x0, #8]
    ldr     x0, [x0, #0]

    // Switch to task stack and return from exception level 1
    mov     sp, x3  // sp is SP_EL1
    isb
    eret // ret - excetion level 안바꾸는 명령어
.size _os_restore_and_eret, . - _os_restore_and_eret
