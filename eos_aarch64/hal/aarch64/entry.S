/*
    entry_new_context.S
    AArch64 startup + Inlined IRQ vector slot
*/

.globl _start
.section .text.boot, "ax"
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
 * EL1 Exception Vector Table (2KiB align)
 * ===========================*/
.section .vectors, "ax"
.align  11
.global vectors_el1
.global __vectors_start
__vectors_start:
vectors_el1:

    /* ===== Group A: EL1 using SP0 (SPSel=0) =====
     * Context: EL1 code running on SP_EL0 (special stack).
     * Typical use: low-level trampolines, special paths.
     */
    b   el1_sync_sp0                // +0x000  Synchronous exception at EL1 with SP0
    .org vectors_el1 + 0x080
    b   el1_irq_sp0                 // +0x080  IRQ at EL1 with SP0
    .org vectors_el1 + 0x100
    b   el1_fiq_sp0                 // +0x100  FIQ at EL1 with SP0
    .org vectors_el1 + 0x180
    b   el1_serr_sp0                // +0x180  SError at EL1 with SP0


    /* ===== Group B: EL1 using SPx (SPSel=1) =====
     * Context: EL1 code running on SP_EL1 (normal kernel path).
     * Typical use: most kernel exceptions and interrupts.
     */
    .org vectors_el1 + 0x200
    b   el1_sync_spx                // +0x200  Synchronous exception at EL1 with SPx
    .org vectors_el1 + 0x280
    b   el1_irq_spx_handler         // +0x280  IRQ at EL1 with SPx (implemented)
    .org vectors_el1 + 0x300
    b   el1_fiq_spx                 // +0x300  FIQ at EL1 with SPx
    .org vectors_el1 + 0x380
    b   el1_serr_spx                // +0x380  SError at EL1 with SPx


    /* ===== Group C: Exceptions taken from EL0 =====
     * Context: EL0 -> EL1 exception entry.
     * Typical use: syscall, page fault, user IRQ.
     */
    .org vectors_el1 + 0x400
    b   el1_sync_el0                // +0x400  Synchronous exception from EL0
    .org vectors_el1 + 0x480
    b   el1_irq_el0                 // +0x480  IRQ from EL0
    .org vectors_el1 + 0x500
    b   el1_fiq_el0                 // +0x500  FIQ from EL0
    .org vectors_el1 + 0x580
    b   el1_serr_el0                // +0x580  SError from EL0


    /* ===== Group D: AArch32 compatibility at EL1 =====
     * Context: EL1 executing in AArch32 state.
     * Typical use: 32-bit guests or compat layers.
     */
    .org vectors_el1 + 0x600
    b   el1_sync_a32                // +0x600  AArch32 synchronous exception at EL1
    .org vectors_el1 + 0x680
    b   el1_irq_a32                 // +0x680  AArch32 IRQ at EL1
    .org vectors_el1 + 0x700
    b   el1_fiq_a32                 // +0x700  AArch32 FIQ at EL1
    .org vectors_el1 + 0x780
    b   el1_serr_a32                // +0x780  AArch32 SError at EL1


/* =============================================
 * EL1 IRQ Handler with Full Context Save/Restore
 * ============================================= */
.global el1_irq_spx_handler
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
