.text

.equ CTX_SIZE, 272
.equ CTX_OFF_SP, 248
.equ CTX_OFF_ELR, 256
.equ CTX_OFF_SPSR, 264

// Save caller context into a stack frame and return its base in x0
.global _os_save_context
_os_save_context:
    // Allocate frame
    sub     sp, sp, #CTX_SIZE

    // Save GPRs x0-x30
    stp     x0,  x1,  [sp, #(0*16)]
    stp     x2,  x3,  [sp, #(1*16)]
    stp     x4,  x5,  [sp, #(2*16)]
    stp     x6,  x7,  [sp, #(3*16)]
    stp     x8,  x9,  [sp, #(4*16)]
    stp     x10, x11, [sp, #(5*16)]
    stp     x12, x13, [sp, #(6*16)]
    stp     x14, x15, [sp, #(7*16)]
    stp     x16, x17, [sp, #(8*16)]
    stp     x18, x19, [sp, #(9*16)]
    stp     x20, x21, [sp, #(10*16)]
    stp     x22, x23, [sp, #(11*16)]
    stp     x24, x25, [sp, #(12*16)]
    stp     x26, x27, [sp, #(13*16)]
    stp     x28, x29, [sp, #(14*16)]
    str     x30,      [sp, #(15*16)]     // LR

    // Save original SP, ELR_EL1, SPSR_EL1
    add     x0, sp, #CTX_SIZE            // x0 = original SP before sub
    str     x0,      [sp, #CTX_OFF_SP]
    mrs     x0, ELR_EL1
    str     x0,      [sp, #CTX_OFF_ELR]
    mrs     x0, SPSR_EL1
    str     x0,      [sp, #CTX_OFF_SPSR]

    // Return context base in x0
    mov     x0, sp
    ret

// Restore from context pointer in x0 and eret
.global _os_restore_and_eret
_os_restore_and_eret:
    mov     x9, x0                       // x9 = context_ptr

    // Restore system registers first
    ldr     x10, [x9, #CTX_OFF_ELR]
    ldr     x11, [x9, #CTX_OFF_SPSR]
    msr     ELR_EL1, x10
    msr     SPSR_EL1, x11

    // Restore GPRs (keep x9 as base until last)
    ldp     x0,  x1,  [x9, #(0*16)]
    ldp     x2,  x3,  [x9, #(1*16)]
    ldp     x4,  x5,  [x9, #(2*16)]
    ldp     x6,  x7,  [x9, #(3*16)]
    // x8, x9 later since x9 is the base
    ldp     x10, x11, [x9, #(5*16)]
    ldp     x12, x13, [x9, #(6*16)]
    ldp     x14, x15, [x9, #(7*16)]
    ldp     x16, x17, [x9, #(8*16)]
    ldp     x18, x19, [x9, #(9*16)]
    ldp     x20, x21, [x9, #(10*16)]
    ldp     x22, x23, [x9, #(11*16)]
    ldp     x24, x25, [x9, #(12*16)]
    ldp     x26, x27, [x9, #(13*16)]
    ldp     x28, x29, [x9, #(14*16)]
    ldr     x30,      [x9, #(15*16)]

    // Restore SP, x8, x9 last
    ldr     x8,  [x9, #(4*16)]
    ldr     x10, [x9, #CTX_OFF_SP]       // x10 = SP to restore
    ldr     x9,  [x9, #(4*16 + 8)]
    mov     sp, x10

    // Exit from exception
    isb
    eret