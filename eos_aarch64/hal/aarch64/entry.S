/*
    entry_new_context.S
    AArch64 startup + Inlined IRQ vector slot
*/

.section .text, "ax"
.global _start
_start:
    // Mask all exceptions at entry
    msr     DAIFSet, #0xf           

    // Detect current EL and branch if not EL2
    mrs     x0, CurrentEL           
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    el1_entry

    // Force EL1 to AArch64 (HCR_EL2.RW=1)
    mov     x1, #(1 << 31)
    msr     HCR_EL2, x1

    // Program SPSR_EL2 for return to EL1h with DAIF masked
    mov     x1, #0x3C5
    msr     SPSR_EL2, x1

    // Set return address to el1_entry (aligned) and sync
    adr     x1, el1_entry
    bic     x1, x1, #0b11
    isb                             // Instruction Synchronization Barrier
    msr     ELR_EL2, x1

    // Initialize SP_EL1 with 16-byte alignment
    ldr     x1, =__stack_top
    and     x1, x1, #-16
    msr     SP_EL1, x1

    // Enable FP/SIMD at EL1 (CPACR_EL1.FPEN=0b11)
    mrs     x0, CPACR_EL1
    orr     x0, x0, #(3 << 20)
    msr     CPACR_EL1, x0
    isb

    // Allow EL1 access to Generic Timer (EL1PCTEN, EL1PCEN)
    mrs     x0, CNTHCTL_EL2
    orr     x0, x0, #(1 << 0)
    orr     x0, x0, #(1 << 1)
    msr     CNTHCTL_EL2, x0

    // Disable EL0 timer access for now
    msr     CNTKCTL_EL1, xzr

    // Return to EL1h at el1_entry using programmed state
    eret

el1_entry:
    // Set up SP for EL1 execution
    ldr     x0, =__stack_top
    and     x0, x0, #-16            // Ensure 16-byte alignment
    mov     sp, x0

    // Clear BSS section (zero-initialize)
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end

bss_clear_loop:  
    cmp     x1, x2
    b.hs    bss_clear_done
    str     xzr, [x1], #8
    b       bss_clear_loop

bss_clear_done:
    // Set vector base address for exception handling
    ldr     x0, =__vectors_start
    msr     VBAR_EL1, x0
    isb
    
os_init: 
    // Call OS initialization routine
    bl      _os_initialization

idle_loop:  
    wfe
    b       idle_loop

/* ===========================
 * 예외 벡터 테이블 (2KiB align)
 * ===========================*/
.section .vectors, "ax"
.align  11
.global vectors_el1
.global __vectors_start
__vectors_start:
vectors_el1:
    /* SP0/EL1 slots (30%는 stub) */
    b   el1_sync_sp0                // +0x000
    .org vectors_el1 + 0x080
    b   el1_irq_sp0                 // +0x080
    .org vectors_el1 + 0x100
    b   el1_fiq_sp0
    .org vectors_el1 + 0x180
    b   el1_serr_sp0
    .org vectors_el1 + 0x200
    b   el1_sync_spx                // +0x200
    .org vectors_el1 + 0x280
    b   el1_irq_spx_handler

    .org vectors_el1 + 0x300
    b   el1_fiq_spx
    .org vectors_el1 + 0x380
    b   el1_serr_spx

    .org vectors_el1 + 0x400
    b   el1_sync_el0
    .org vectors_el1 + 0x480
    b   el1_irq_el0
    .org vectors_el1 + 0x500
    b   el1_fiq_el0
    .org vectors_el1 + 0x580
    b   el1_serr_el0

    .org vectors_el1 + 0x600
    b   el1_sync_a32
    .org vectors_el1 + 0x680
    b   el1_irq_a32
    .org vectors_el1 + 0x700
    b   el1_fiq_a32
    .org vectors_el1 + 0x780
    b   el1_serr_a32

/* =============================================
 * 실제 예외 핸들러 구현 (벡터 테이블 바깥)
 * ============================================= */

.equ CTX_SIZE, 272
.equ CTX_OFF_SP, 248
.equ CTX_OFF_ELR, 256
.equ CTX_OFF_SPSR, 264

.global el1_irq_spx_handler
.global _os_context_save
.global _os_restore_and_eret

el1_irq_spx_handler:
    // Save full EL1 context to a frame and get its pointer in x0
    bl      _os_context_save         // x0 = context_ptr

    // Read IRQ ID then call common C handler
    mov     x1, x0                   // x1 = context_ptr (C arg2)
    ldr     x2, =0x0801000C          // x2 = GIC_IAR address
    ldr     w0, [x2]                 // w0 = irq number (C arg1)
    bl      _os_common_interrupt_handler

    // Tail-call into restore-and-exit
    b       _os_restore_and_eret


// Save caller context into a stack frame and return its base in x0
_os_context_save:
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

/* EL1 vector stubs: park CPU until implemented */
el1_sync_sp0:  
    wfe
    b el1_sync_sp0
el1_irq_sp0:   
    wfe
    b el1_irq_sp0
el1_fiq_sp0:
    wfe
    b el1_fiq_sp0
el1_serr_sp0:  
    wfe
    b el1_serr_sp0
el1_sync_spx:  
    wfe
    b el1_sync_spx
el1_fiq_spx:   
    wfe
    b el1_fiq_spx
el1_serr_spx:  
    wfe
    b el1_serr_spx
el1_sync_el0:  
    wfe
    b el1_sync_el0
el1_irq_el0:
    wfe
    b el1_irq_el0
el1_fiq_el0:
    wfe
    b el1_fiq_el0
el1_serr_el0:
    wfe
    b el1_serr_el0
el1_sync_a32:
    wfe
    b el1_sync_a32
el1_irq_a32:   
    wfe
    b el1_irq_a32
el1_fiq_a32:   
    wfe
    b el1_fiq_a32
el1_serr_a32:  
    wfe
    b el1_serr_a32

/* Hook points for real handlers:
 * - Replace each stub with context save, source decode, and dispatch.
 * - For IRQ paths, typically read GIC_IAR, call _os_common_interrupt_handler(w0=irq, x1=context).
 * - Ensure vector entries in __vectors_start point to these labels. */
.extern _os_common_interrupt_handler
