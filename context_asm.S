.section .text
    .align  3
    .global _os_restore_context

/* _os_restore_context(addr_t sp) */
_os_restore_context:
    mov     sp, x0

    ldr     x0,  [sp, #(0*8)]
    ldr     x1,  [sp, #(1*8)]
    ldr     x2,  [sp, #(2*8)]
    ldr     x3,  [sp, #(3*8)]
    ldr     x4,  [sp, #(4*8)]
    ldr     x5,  [sp, #(5*8)]
    ldr     x6,  [sp, #(6*8)]
    ldr     x7,  [sp, #(7*8)]
    ldr     x8,  [sp, #(8*8)]
    ldr     x9,  [sp, #(9*8)]
    ldr     x10, [sp, #(10*8)]
    ldr     x11, [sp, #(11*8)]
    ldr     x12, [sp, #(12*8)]
    ldr     x13, [sp, #(13*8)]
    ldr     x14, [sp, #(14*8)]
    ldr     x15, [sp, #(15*8)]
    ldr     x16, [sp, #(16*8)]
    ldr     x17, [sp, #(17*8)]
    ldr     x18, [sp, #(18*8)]
    ldr     x19, [sp, #(19*8)]
    ldr     x20, [sp, #(20*8)]
    ldr     x21, [sp, #(21*8)]
    ldr     x22, [sp, #(22*8)]
    ldr     x23, [sp, #(23*8)]
    ldr     x24, [sp, #(24*8)]
    ldr     x25, [sp, #(25*8)]
    ldr     x26, [sp, #(26*8)]
    ldr     x27, [sp, #(27*8)]
    ldr     x28, [sp, #(28*8)]
    ldr     x29, [sp, #(29*8)]
    ldr     x30, [sp, #(30*8)]

    ldr     x1, [sp, #(31*8)]    // SP_EL0
    ldr     x2, [sp, #(32*8)]    // ELR_EL1
    ldr     x3, [sp, #(33*8)]    // SPSR_EL1

    msr     sp_el0, x1
    msr     elr_el1, x2
    msr     spsr_el1, x3
    isb
    eret



.global _os_save_context
_os_save_context:
    // x0: context 구조체 포인터

    str     x0,  [x0, #(0*8)]
    str     x1,  [x0, #(1*8)]
    str     x2,  [x0, #(2*8)]
    str     x3,  [x0, #(3*8)]
    str     x4,  [x0, #(4*8)]
    str     x5,  [x0, #(5*8)]
    str     x6,  [x0, #(6*8)]
    str     x7,  [x0, #(7*8)]
    str     x8,  [x0, #(8*8)]
    str     x9,  [x0, #(9*8)]
    str     x10, [x0, #(10*8)]
    str     x11, [x0, #(11*8)]
    str     x12, [x0, #(12*8)]
    str     x13, [x0, #(13*8)]
    str     x14, [x0, #(14*8)]
    str     x15, [x0, #(15*8)]
    str     x16, [x0, #(16*8)]
    str     x17, [x0, #(17*8)]
    str     x18, [x0, #(18*8)]
    str     x19, [x0, #(19*8)]
    str     x20, [x0, #(20*8)]
    str     x21, [x0, #(21*8)]
    str     x22, [x0, #(22*8)]
    str     x23, [x0, #(23*8)]
    str     x24, [x0, #(24*8)]
    str     x25, [x0, #(25*8)]
    str     x26, [x0, #(26*8)]
    str     x27, [x0, #(27*8)]
    str     x28, [x0, #(28*8)]
    str     x29, [x0, #(29*8)]
    str     x30, [x0, #(30*8)]

    mrs     x1, sp_el0
    str     x1, [x0, #(31*8)]
    mrs     x2, elr_el1
    str     x2, [x0, #(32*8)]
    mrs     x3, spsr_el1
    str     x3, [x0, #(33*8)]

    mov     x0, x0
    ret
